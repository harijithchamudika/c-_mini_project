// Console user interface for the Furniture Store Management System.
// Build (instead of main.cpp, which is the test program):
//   g++ -std=c++17 -Wall -Wextra app.cpp Models/Order.cpp Models/OrderItem.cpp
//       Services/Inventory.cpp Services/CustomerManager.cpp Services/FileManager.cpp -o furniture_app
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "Services/Inventory.h"
#include "Services/CustomerManager.h"
#include "Services/FileManager.h"
#include "Models/Order.h"

using std::cout;
using std::string;

// ============================ input helpers ============================
static string readLine(const string& prompt) {
    cout << prompt;
    string s;
    if (!std::getline(std::cin, s)) { cout << "\nInput closed. Exiting.\n"; std::exit(0); }
    return s;
}

static int readInt(const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        try {
            std::size_t pos = 0;
            int v = std::stoi(s, &pos);
            if (pos == s.size()) return v;
        } catch (...) {}
        cout << "  Please enter a whole number.\n";
    }
}

static double readDouble(const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        try {
            std::size_t pos = 0;
            double v = std::stod(s, &pos);
            if (pos == s.size()) return v;
        } catch (...) {}
        cout << "  Please enter a number.\n";
    }
}

static string todayDate() {
    std::time_t t = std::time(nullptr);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
    return buf;
}

static string money(double v) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(2) << v;
    return os.str();
}

// ============================ the application ============================
class ConsoleApp {
    Inventory inv;
    CustomerManager cm;
    std::vector<std::unique_ptr<Order>> orders;
    FileManager fm;

public:
    ConsoleApp() : fm("Data") {}

    void run() {
        load();
        int choice = -1;
        do {
            cout << "\n=========== FURNITURE STORE ===========\n"
                    " 1. Furniture & inventory\n"
                    " 2. Customers\n"
                    " 3. Orders\n"
                    " 0. Save and exit\n"
                    "=======================================\n";
            choice = readInt("Choice: ");
            switch (choice) {
                case 1: furnitureMenu(); break;
                case 2: customerMenu();  break;
                case 3: orderMenu();     break;
                case 0: break;
                default: cout << "  Invalid choice.\n";
            }
        } while (choice != 0);
        save();
        cout << "Data saved. Goodbye!\n";
    }

private:
    // ---------------------------- load / save ----------------------------
    void load() {
        try {
            fm.loadFurniture(inv);
            fm.loadCustomers(cm);
            fm.loadOrders(orders);
            cout << "Loaded " << inv.size() << " furniture items, " << cm.size()
                 << " customers, " << orders.size() << " orders.\n";
        } catch (const std::exception& e) {
            cout << "Could not load data: " << e.what() << "\n";
        }
    }

    void save() {
        try {
            fm.saveFurniture(inv);
            fm.saveCustomers(cm);
            fm.saveOrders(orders);
        } catch (const std::exception& e) {
            cout << "Could not save data: " << e.what() << "\n";
        }
    }

    // Runs an action; any exception from the model classes becomes a friendly message.
    template <typename F>
    void guarded(F action) {
        try { action(); }
        catch (const std::exception& e) { cout << "  Error: " << e.what() << "\n"; }
    }

    // ============================ FURNITURE ============================
    static void printFurnitureHeader() {
        cout << std::left << std::setw(8) << "ID" << std::setw(22) << "Name"
             << std::setw(10) << "Category" << std::right << std::setw(10) << "Price"
             << std::setw(8) << "Qty" << "\n"
             << string(58, '-') << "\n";
    }
    static void printFurniture(const Furniture& f) {
        cout << std::left << std::setw(8) << f.getId() << std::setw(22) << f.getName().substr(0, 21)
             << std::setw(10) << f.getCategory() << std::right << std::setw(10)
             << money(f.calculatePrice()) << std::setw(8) << f.getQuantity() << "\n";
    }
    static void printFurnitureList(const std::vector<Furniture*>& list) {
        if (list.empty()) { cout << "  (nothing found)\n"; return; }
        printFurnitureHeader();
        for (const Furniture* f : list) printFurniture(*f);
    }

    void furnitureMenu() {
        int c = -1;
        do {
            cout << "\n--- Furniture & Inventory ---\n"
                    " 1. List all\n 2. Search by name\n 3. Filter by category\n"
                    " 4. Low-stock items\n 5. Add furniture\n 6. Update price\n"
                    " 7. Restock\n 8. Remove furniture\n 0. Back\n";
            c = readInt("Choice: ");
            guarded([&] {
                switch (c) {
                    case 1: printFurnitureList(inv.getAll()); break;
                    case 2: printFurnitureList(inv.searchByName(readLine("Name contains: "))); break;
                    case 3: printFurnitureList(inv.filterByCategory(readLine("Category (Chair/Sofa): "))); break;
                    case 4: {
                        int th = readInt("Show items with quantity at or below: ");
                        printFurnitureList(inv.getLowStockItems(th));
                        break;
                    }
                    case 5: addFurniture(); break;
                    case 6: {
                        string id = readLine("Furniture ID: ");
                        inv.updatePrice(id, readDouble("New base price: "));
                        save(); cout << "  Price updated.\n";
                        break;
                    }
                    case 7: {
                        string id = readLine("Furniture ID: ");
                        inv.restock(id, readInt("Amount to add: "));
                        save(); cout << "  Restocked.\n";
                        break;
                    }
                    case 8: {
                        string id = readLine("Furniture ID to remove: ");
                        inv.removeItem(id);
                        save(); cout << "  Removed.\n";
                        break;
                    }
                    case 0: break;
                    default: cout << "  Invalid choice.\n";
                }
            });
        } while (c != 0);
    }

    void addFurniture() {
        cout << "Type: 1. Chair  2. Sofa\n";
        int type = readInt("Choice: ");
        if (type != 1 && type != 2) { cout << "  Invalid type.\n"; return; }
        string id = readLine("ID (e.g. F003): ");
        string name = readLine("Name: ");
        string material = readLine("Material: ");
        string color = readLine("Colour: ");
        double price = readDouble("Base price: ");
        int qty = readInt("Quantity: ");
        if (type == 1) {
            inv.addItem(std::make_unique<Chair>(id, name, material, color, price, qty));
        } else {
            int seats = readInt("Number of seats: ");
            inv.addItem(std::make_unique<Sofa>(id, name, material, color, price, qty, seats));
        }
        save();
        cout << "  Furniture added.\n";
    }

    // ============================ CUSTOMERS ============================
    static void printCustomers(const std::vector<Customer>& list) {
        if (list.empty()) { cout << "  (nothing found)\n"; return; }
        cout << std::left << std::setw(8) << "ID" << std::setw(22) << "Name"
             << std::setw(13) << "Phone" << std::setw(26) << "Email" << "Address\n"
             << string(80, '-') << "\n";
        for (const Customer& c : list)
            cout << std::left << std::setw(8) << c.getId() << std::setw(22) << c.getName().substr(0, 21)
                 << std::setw(13) << c.getPhone() << std::setw(26) << c.getEmail().substr(0, 25)
                 << c.getAddress() << "\n";
    }

    void customerMenu() {
        int c = -1;
        do {
            cout << "\n--- Customers ---\n"
                    " 1. List all\n 2. Search by name\n 3. Add customer\n"
                    " 4. Edit customer\n 5. Remove customer\n 0. Back\n";
            c = readInt("Choice: ");
            guarded([&] {
                switch (c) {
                    case 1: printCustomers(cm.getAll()); break;
                    case 2: printCustomers(cm.searchByName(readLine("Name contains: "))); break;
                    case 3: {
                        string id = cm.generateNextId();
                        cout << "New customer ID: " << id << "\n";
                        string name = readLine("Name: ");
                        string phone = readLine("Phone (10 digits): ");
                        string email = readLine("Email: ");
                        string address = readLine("Address: ");
                        cm.addCustomer(Customer(id, name, phone, email, address));
                        save(); cout << "  Customer added.\n";
                        break;
                    }
                    case 4: {
                        string id = readLine("Customer ID: ");
                        const Customer& cur = cm.findById(id);
                        cout << "(press Enter to keep the current value)\n";
                        string name = readLine("Name [" + cur.getName() + "]: ");
                        string phone = readLine("Phone [" + cur.getPhone() + "]: ");
                        string email = readLine("Email [" + cur.getEmail() + "]: ");
                        string address = readLine("Address [" + cur.getAddress() + "]: ");
                        cm.editCustomer(id, name.empty() ? cur.getName() : name,
                                        phone.empty() ? cur.getPhone() : phone,
                                        email.empty() ? cur.getEmail() : email,
                                        address.empty() ? cur.getAddress() : address);
                        save(); cout << "  Customer updated.\n";
                        break;
                    }
                    case 5: {
                        string id = readLine("Customer ID to remove: ");
                        cm.removeCustomer(id);
                        save(); cout << "  Customer removed.\n";
                        break;
                    }
                    case 0: break;
                    default: cout << "  Invalid choice.\n";
                }
            });
        } while (c != 0);
    }

    // ============================ ORDERS ============================
    string nextOrderId() const {
        int maxNum = 0;
        for (const auto& o : orders) {
            const string& id = o->getOrderId();
            if (id.size() > 1 && id[0] == 'O') {
                try { maxNum = std::max(maxNum, std::stoi(id.substr(1))); } catch (...) {}
            }
        }
        std::ostringstream os;
        os << "O" << std::setw(3) << std::setfill('0') << (maxNum + 1);
        return os.str();
    }

    Order* findOrder(const string& id) {
        for (auto& o : orders)
            if (o->getOrderId() == id) return o.get();
        throw ItemNotFoundException("Order " + id + " not found.");
    }

    static void printOrderSummary(const Order& o) {
        cout << std::left << std::setw(8) << o.getOrderId() << std::setw(10) << o.getChannel()
             << std::setw(8) << o.getCustomerId() << std::setw(12) << o.getDate()
             << std::setw(11) << o.getStatusText() << std::right << std::setw(10)
             << money(o.calculateTotal()) << "\n";
    }

    static void printOrderDetails(const Order& o) {
        cout << "\nOrder " << o.getOrderId() << "  |  " << o.getChannel() << "  |  Customer "
             << o.getCustomerId() << "  |  " << o.getDate() << "  |  " << o.getStatusText() << "\n"
             << string(60, '-') << "\n";
        if (o.getItems().empty()) cout << "  (no items yet)\n";
        for (const OrderItem& it : o.getItems())
            cout << "  " << std::left << std::setw(8) << it.getFurnitureId() << std::setw(22)
                 << it.getName().substr(0, 21) << std::right << std::setw(4) << it.getQuantity()
                 << " x " << std::setw(9) << money(it.getUnitPrice()) << " = " << std::setw(10)
                 << money(it.getLineTotal()) << "\n";
        cout << string(60, '-') << "\n"
             << "  Subtotal:       " << std::setw(10) << money(o.calculateSubtotal()) << "\n"
             << "  Channel charge: " << std::setw(10) << money(o.getChannelCharge()) << "\n"
             << "  TOTAL:          " << std::setw(10) << money(o.calculateTotal()) << "\n";
    }

    void orderMenu() {
        int c = -1;
        do {
            cout << "\n--- Orders ---\n"
                    " 1. List all orders\n 2. View order details\n 3. Create new order\n"
                    " 4. Add item to order\n 5. Remove item from order\n 6. Confirm order\n"
                    " 7. Mark delivered\n 8. Cancel order\n 0. Back\n";
            c = readInt("Choice: ");
            guarded([&] {
                switch (c) {
                    case 1:
                        if (orders.empty()) { cout << "  (no orders yet)\n"; break; }
                        cout << std::left << std::setw(8) << "ID" << std::setw(10) << "Channel"
                             << std::setw(8) << "Cust" << std::setw(12) << "Date" << std::setw(11)
                             << "Status" << std::right << std::setw(10) << "Total" << "\n"
                             << string(59, '-') << "\n";
                        for (const auto& o : orders) printOrderSummary(*o);
                        break;
                    case 2: printOrderDetails(*findOrder(readLine("Order ID: "))); break;
                    case 3: createOrder(); break;
                    case 4: addItemToOrder(); break;
                    case 5: {
                        Order* o = findOrder(readLine("Order ID: "));
                        o->removeItem(readLine("Furniture ID to remove: "));
                        save(); cout << "  Item removed.\n";
                        break;
                    }
                    case 6: {
                        Order* o = findOrder(readLine("Order ID: "));
                        o->confirm(inv);
                        save(); cout << "  Order confirmed. Stock reduced.\n";
                        printOrderDetails(*o);
                        break;
                    }
                    case 7: {
                        Order* o = findOrder(readLine("Order ID: "));
                        o->markDelivered();
                        save(); cout << "  Order marked as delivered.\n";
                        break;
                    }
                    case 8: {
                        Order* o = findOrder(readLine("Order ID: "));
                        o->cancel();
                        save(); cout << "  Order cancelled.\n";
                        break;
                    }
                    case 0: break;
                    default: cout << "  Invalid choice.\n";
                }
            });
        } while (c != 0);
    }

    void createOrder() {
        string custId = readLine("Customer ID: ");
        if (!cm.exists(custId)) throw ItemNotFoundException("Customer " + custId + " not found.");

        cout << "Channel: 1. Online  2. Phone  3. In-store\n";
        int ch = readInt("Choice: ");
        string id = nextOrderId();
        string date = readLine("Date YYYY-MM-DD [Enter = " + todayDate() + "]: ");
        if (date.empty()) date = todayDate();

        std::unique_ptr<Order> order;
        if (ch == 1) {
            order = std::make_unique<OnlineOrder>(id, custId, date, readLine("Website reference: "));
        } else if (ch == 2) {
            string call = readLine("Call reference: ");
            string staff = readLine("Taken by (staff name): ");
            order = std::make_unique<PhoneOrder>(id, custId, date, call, staff);
        } else if (ch == 3) {
            order = std::make_unique<InStoreOrder>(id, custId, date, readInt("Counter number: "));
        } else {
            cout << "  Invalid channel.\n";
            return;
        }
        orders.push_back(std::move(order));
        save();
        cout << "  Order " << id << " created (Pending). Use 'Add item to order' next.\n";
    }

    void addItemToOrder() {
        Order* o = findOrder(readLine("Order ID: "));
        printFurnitureList(inv.getAll());
        string fid = readLine("Furniture ID: ");
        const Furniture& f = inv.findById(fid);
        int qty = readInt("Quantity: ");
        // The price is frozen into the order line at the time of sale.
        o->addItem(OrderItem(f.getId(), f.getName(), qty, f.calculatePrice()));
        save();
        cout << "  Item added.\n";
        printOrderDetails(*o);
    }
};

int main() {
    ConsoleApp app;
    app.run();
    return 0;
}