#pragma once
#include <string>
#include "../Stubs/Exceptions.h"

// One line of an order. Stores the unit price AT THE TIME OF SALE so that
// later price updates never change old orders.
class OrderItem {
    std::string furnitureId;
    std::string name;
    int quantity;
    double unitPrice;
public:
    OrderItem(std::string furnitureId, std::string name, int quantity, double unitPrice);
    // throws InvalidInputException if quantity <= 0 or unitPrice < 0

    const std::string& getFurnitureId() const { return furnitureId; }
    const std::string& getName() const { return name; }
    int getQuantity() const { return quantity; }
    double getUnitPrice() const { return unitPrice; }
    double getLineTotal() const { return quantity * unitPrice; }

    // Format: orderId,furnitureId,name,qty,unitPrice
    std::string serialize(const std::string& orderId) const;
};
