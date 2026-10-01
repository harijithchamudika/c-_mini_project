#pragma once
#include <memory>
#include <string>
#include <vector>
#include "../Stubs/Furniture.h"

// Owns all furniture objects (composition via unique_ptr).
class Inventory {
    std::vector<std::unique_ptr<Furniture>> items;

    // returns index or -1
    int indexOf(const std::string& id) const;

public:
    // Takes ownership. throws DuplicateIDException, InvalidInputException (null / empty id)
    void addItem(std::unique_ptr<Furniture> item);

    // throws ItemNotFoundException
    void removeItem(const std::string& id);
    Furniture& findById(const std::string& id);
    const Furniture& findById(const std::string& id) const;
    bool exists(const std::string& id) const;

    // Returns non-owning pointers; do not delete them.
    std::vector<Furniture*> getAll() const;
    std::vector<Furniture*> searchByName(const std::string& keyword) const;   // case-insensitive
    std::vector<Furniture*> filterByCategory(const std::string& category) const;
    std::vector<Furniture*> getLowStockItems(int threshold = 5) const;

    // throws ItemNotFoundException, InvalidInputException (price < 0)
    void updatePrice(const std::string& id, double newPrice);
    // throws ItemNotFoundException, InvalidInputException (amount <= 0)
    void restock(const std::string& id, int amount);
    // throws ItemNotFoundException, InvalidInputException (amount <= 0), OutOfStockException
    void reduceStock(const std::string& id, int amount);
    bool hasStock(const std::string& id, int amount) const;

    std::size_t size() const { return items.size(); }
    void clear() { items.clear(); }
};
