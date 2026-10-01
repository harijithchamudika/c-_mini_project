#include "OrderItem.h"

OrderItem::OrderItem(std::string furnitureId, std::string name, int quantity, double unitPrice)
    : furnitureId(std::move(furnitureId)), name(std::move(name)),
      quantity(quantity), unitPrice(unitPrice) {
    if (this->furnitureId.empty())
        throw InvalidInputException("Order item needs a furniture ID.");
    if (quantity <= 0)
        throw InvalidInputException("Order item quantity must be greater than zero.");
    if (unitPrice < 0)
        throw InvalidInputException("Order item price cannot be negative.");
}

std::string OrderItem::serialize(const std::string& orderId) const {
    return orderId + "," + furnitureId + "," + name + ","
         + std::to_string(quantity) + "," + std::to_string(unitPrice);
}
