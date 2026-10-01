#include "FileManager.h"
#include <filesystem>
#include <fstream>
#include <map>

namespace fs = std::filesystem;

namespace {

std::vector<std::string> readLines(const std::string& fullPath) {
    std::ifstream in(fullPath);
    if (!in) throw FileException("Cannot open file: " + fullPath);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();   // Windows line endings
        lines.push_back(line);
    }
    return lines;
}

// Writes to a temp file first, then swaps it in, so a crash mid-save
// cannot destroy the old data.
void writeLines(const std::string& fullPath, const std::vector<std::string>& lines) {
    const std::string tmp = fullPath + ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out) throw FileException("Cannot write file: " + fullPath);
        for (const auto& l : lines) out << l << "\n";
        out.flush();
        if (!out) throw FileException("Write failed: " + fullPath);
    }
    std::error_code ec;
    fs::rename(tmp, fullPath, ec);
    if (ec) throw FileException("Cannot replace file " + fullPath + ": " + ec.message());
}

[[noreturn]] void fail(const std::string& file, int lineNo, const std::string& why) {
    throw FileException(file + " (line " + std::to_string(lineNo) + "): " + why);
}

double toDouble(const std::string& s) {
    std::size_t pos = 0;
    double v = std::stod(s, &pos);          // throws std::invalid_argument on junk
    if (pos != s.size()) throw FileException("Bad number: " + s);
    return v;
}

int toInt(const std::string& s) {
    std::size_t pos = 0;
    int v = std::stoi(s, &pos);
    if (pos != s.size()) throw FileException("Bad whole number: " + s);
    return v;
}

void needColumns(const std::vector<std::string>& cols, std::size_t n, const std::string& what) {
    if (cols.size() != n)
        throw FileException(what + " needs " + std::to_string(n) + " columns but has " +
                            std::to_string(cols.size()) + ".");
}

} // namespace

// ---------------------------------------------------------------------
FileManager::FileManager(std::string dataFolder) : dataFolder(std::move(dataFolder)) {
    std::error_code ec;
    fs::create_directories(this->dataFolder, ec);
    if (ec) throw FileException("Cannot create data folder: " + this->dataFolder);
}

// Keeps empty fields (e.g. "a,,b" -> 3 columns).
std::vector<std::string> FileManager::split(const std::string& line, char sep) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : line) {
        if (c == sep) { out.push_back(cur); cur.clear(); }
        else cur += c;
    }
    out.push_back(cur);
    return out;
}

void FileManager::ensureFileExists(const std::string& fullPath) {
    if (fs::exists(fullPath)) return;
    std::ofstream create(fullPath);
    if (!create) throw FileException("Cannot create file: " + fullPath);
}

// ------------------------------ factories ------------------------------
// Add a new `if` here when a new Furniture subclass is added.
std::unique_ptr<Furniture> FileManager::createFurniture(const std::vector<std::string>& cols) {
    if (cols.empty()) throw FileException("Empty furniture record.");
    const std::string& type = cols[0];
    if (type == "Chair") {
        needColumns(cols, 7, "Chair");
        return std::make_unique<Chair>(cols[1], cols[2], cols[3], cols[4],
                                       toDouble(cols[5]), toInt(cols[6]));
    }
    if (type == "Sofa") {
        needColumns(cols, 8, "Sofa");
        return std::make_unique<Sofa>(cols[1], cols[2], cols[3], cols[4],
                                      toDouble(cols[5]), toInt(cols[6]), toInt(cols[7]));
    }
    throw FileException("Unknown furniture type: " + type);
}

// Status is NOT applied here: items must be added while the order is still
// Pending, so loadOrders() sets the status afterwards.
std::unique_ptr<Order> FileManager::createOrder(const std::vector<std::string>& cols) {
    if (cols.empty()) throw FileException("Empty order record.");
    const std::string& type = cols[0];
    if (type == "Online") {
        needColumns(cols, 7, "Online order");
        return std::make_unique<OnlineOrder>(cols[1], cols[2], cols[3], cols[5], toDouble(cols[6]));
    }
    if (type == "Phone") {
        needColumns(cols, 7, "Phone order");
        return std::make_unique<PhoneOrder>(cols[1], cols[2], cols[3], cols[5], cols[6]);
    }
    if (type == "InStore") {
        needColumns(cols, 6, "In-store order");
        return std::make_unique<InStoreOrder>(cols[1], cols[2], cols[3], toInt(cols[5]));
    }
    throw FileException("Unknown order channel: " + type);
}

// ------------------------------ furniture ------------------------------
void FileManager::saveFurniture(const Inventory& inv) const {
    std::vector<std::string> lines;
    for (const Furniture* f : inv.getAll()) lines.push_back(f->serialize());
    writeLines(path("furniture.csv"), lines);
}

void FileManager::loadFurniture(Inventory& inv) const {
    const std::string file = path("furniture.csv");
    ensureFileExists(file);
    Inventory loaded;                       // only replace `inv` if the whole file is OK
    int n = 0;
    for (const auto& line : readLines(file)) {
        ++n;
        if (line.empty()) continue;
        try { loaded.addItem(createFurniture(split(line))); }
        catch (const std::exception& e) { fail("furniture.csv", n, e.what()); }
    }
    inv = std::move(loaded);
}

// ------------------------------ customers ------------------------------
void FileManager::saveCustomers(const CustomerManager& cm) const {
    std::vector<std::string> lines;
    for (const Customer& c : cm.getAll()) lines.push_back(c.serialize());
    writeLines(path("customers.csv"), lines);
}

void FileManager::loadCustomers(CustomerManager& cm) const {
    const std::string file = path("customers.csv");
    ensureFileExists(file);
    CustomerManager loaded;
    int n = 0;
    for (const auto& line : readLines(file)) {
        ++n;
        if (line.empty()) continue;
        try {
            auto cols = split(line);
            needColumns(cols, 5, "Customer");
            loaded.addCustomer(Customer(cols[0], cols[1], cols[2], cols[3], cols[4]));
        } catch (const std::exception& e) { fail("customers.csv", n, e.what()); }
    }
    cm = std::move(loaded);
}

// -------------------------------- orders --------------------------------
void FileManager::saveOrders(const std::vector<std::unique_ptr<Order>>& orders) const {
    std::vector<std::string> orderLines, itemLines;
    for (const auto& o : orders) {
        orderLines.push_back(o->serialize());
        for (const auto& it : o->getItems()) itemLines.push_back(it.serialize(o->getOrderId()));
    }
    writeLines(path("orders.csv"), orderLines);
    writeLines(path("orderitems.csv"), itemLines);
}

void FileManager::loadOrders(std::vector<std::unique_ptr<Order>>& orders) const {
    const std::string orderFile = path("orders.csv");
    const std::string itemFile  = path("orderitems.csv");
    ensureFileExists(orderFile);
    ensureFileExists(itemFile);

    std::vector<std::unique_ptr<Order>> loaded;
    std::map<std::string, Order*> byId;
    std::map<std::string, Order::Status> statusById;

    int n = 0;
    for (const auto& line : readLines(orderFile)) {
        ++n;
        if (line.empty()) continue;
        try {
            auto cols = split(line);
            auto order = createOrder(cols);
            if (byId.count(order->getOrderId()))
                throw FileException("Duplicate order ID " + order->getOrderId());
            statusById[order->getOrderId()] = Order::textToStatus(cols[4]);
            byId[order->getOrderId()] = order.get();
            loaded.push_back(std::move(order));
        } catch (const std::exception& e) { fail("orders.csv", n, e.what()); }
    }

    n = 0;
    for (const auto& line : readLines(itemFile)) {
        ++n;
        if (line.empty()) continue;
        try {
            auto cols = split(line);
            needColumns(cols, 5, "Order item");
            auto it = byId.find(cols[0]);
            if (it == byId.end()) throw FileException("Item refers to unknown order " + cols[0]);
            it->second->addItem(OrderItem(cols[1], cols[2], toInt(cols[3]), toDouble(cols[4])));
        } catch (const std::exception& e) { fail("orderitems.csv", n, e.what()); }
    }

    for (auto& o : loaded) o->setStatus(statusById[o->getOrderId()]);
    orders = std::move(loaded);
}
