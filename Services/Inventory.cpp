#include "Inventory.h"
#include <algorithm>
#include <cctype>
#include <utility>

namespace {
std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
}

int Inventory::indexOf(const std::string& id) const {
    for (std::size_t i = 0; i < items.size(); ++i)
        if (items[i]->getId() == id) return static_cast<int>(i);
    return -1;
}

void Inventory::addItem(std::unique_ptr<Furniture> item) {
    if (!item || item->getId().empty())
        throw InvalidInputException("Furniture item is missing or has an empty ID.");
    if (exists(item->getId()))
        throw DuplicateIDException("Furniture ID " + item->getId() + " already exists.");
    items.push_back(std::move(item));
}

void Inventory::removeItem(const std::string& id) {
    int i = indexOf(id);
    if (i < 0) throw ItemNotFoundException("Furniture " + id + " not found.");
    items.erase(items.begin() + i);
}

Furniture& Inventory::findById(const std::string& id) {
    int i = indexOf(id);
    if (i < 0) throw ItemNotFoundException("Furniture " + id + " not found.");
    return *items[i];
}

const Furniture& Inventory::findById(const std::string& id) const {
    int i = indexOf(id);
    if (i < 0) throw ItemNotFoundException("Furniture " + id + " not found.");
    return *items[i];
}

bool Inventory::exists(const std::string& id) const { return indexOf(id) >= 0; }

std::vector<Furniture*> Inventory::getAll() const {
    std::vector<Furniture*> out;
    for (const auto& p : items) out.push_back(p.get());
    return out;
}

std::vector<Furniture*> Inventory::searchByName(const std::string& keyword) const {
    std::vector<Furniture*> out;
    const std::string key = lower(keyword);
    for (const auto& p : items)
        if (lower(p->getName()).find(key) != std::string::npos) out.push_back(p.get());
    return out;
}

std::vector<Furniture*> Inventory::filterByCategory(const std::string& category) const {
    std::vector<Furniture*> out;
    const std::string key = lower(category);
    for (const auto& p : items)
        if (lower(p->getCategory()) == key) out.push_back(p.get());
    return out;
}

std::vector<Furniture*> Inventory::getLowStockItems(int threshold) const {
    std::vector<Furniture*> out;
    for (const auto& p : items)
        if (p->getQuantity() <= threshold) out.push_back(p.get());
    return out;
}

void Inventory::updatePrice(const std::string& id, double newPrice) {
    Furniture& f = findById(id);
    if (newPrice < 0) throw InvalidInputException("Price cannot be negative.");
    f.setBasePrice(newPrice);
}

void Inventory::restock(const std::string& id, int amount) {
    Furniture& f = findById(id);
    if (amount <= 0) throw InvalidInputException("Restock amount must be greater than zero.");
    f.setQuantity(f.getQuantity() + amount);
}

void Inventory::reduceStock(const std::string& id, int amount) {
    Furniture& f = findById(id);
    if (amount <= 0) throw InvalidInputException("Amount must be greater than zero.");
    if (f.getQuantity() < amount)
        throw OutOfStockException("Only " + std::to_string(f.getQuantity()) + " of " + f.getName() + " in stock.");
    f.setQuantity(f.getQuantity() - amount);
}

bool Inventory::hasStock(const std::string& id, int amount) const {
    int i = indexOf(id);
    return i >= 0 && items[i]->getQuantity() >= amount;
}
