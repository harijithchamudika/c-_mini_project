#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Inventory.h"
#include "CustomerManager.h"
#include "../Models/Order.h"

// All file I/O for the project. No GUI code here.
// Missing file on load -> an empty file is created (no crash).
// Corrupt line         -> FileException with file name and line number.
class FileManager {
    std::string dataFolder;

    std::string path(const std::string& file) const { return dataFolder + "/" + file; }
    static std::vector<std::string> split(const std::string& line, char sep = ',');
    static void ensureFileExists(const std::string& fullPath);

public:
    explicit FileManager(std::string dataFolder = "Data");

    // furniture.csv
    void saveFurniture(const Inventory& inv) const;
    void loadFurniture(Inventory& inv) const;

    // customers.csv
    void saveCustomers(const CustomerManager& cm) const;
    void loadCustomers(CustomerManager& cm) const;

    // orders.csv + orderitems.csv
    void saveOrders(const std::vector<std::unique_ptr<Order>>& orders) const;
    void loadOrders(std::vector<std::unique_ptr<Order>>& orders) const;

    // Factories: pick the subclass from the first column (the "type").
    // throw FileException on unknown type or wrong column count.
    static std::unique_ptr<Furniture> createFurniture(const std::vector<std::string>& cols);
    static std::unique_ptr<Order> createOrder(const std::vector<std::string>& cols);
};
