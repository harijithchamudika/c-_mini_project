#include "Order.h"
#include <cctype>
#include <utility>
#include "../Services/Inventory.h"

namespace {
// YYYY-MM-DD shape check (not a full calendar check).
bool looksLikeDate(const std::string& d) {
    if (d.size() != 10 || d[4] != '-' || d[7] != '-') return false;
    for (std::size_t i = 0; i < d.size(); ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(d[i]))) return false;
    }
    return true;
}
}

// ------------------------------ Order ------------------------------
Order::Order(std::string orderId, std::string customerId, std::string date)
    : orderId(std::move(orderId)), customerId(std::move(customerId)),
      date(std::move(date)), status(Status::Pending) {
    if (this->orderId.empty() || this->customerId.empty())
        throw InvalidInputException("Order ID and customer ID cannot be empty.");
    if (!looksLikeDate(this->date))
        throw InvalidInputException("Order date must be in YYYY-MM-DD format.");
}

// Format: channel,orderId,customerId,date,status  (subclasses append their extras)
std::string Order::serialize() const {
    return getChannel() + "," + orderId + "," + customerId + "," + date + "," + getStatusText();
}

void Order::addItem(const OrderItem& item) {
    if (status != Status::Pending)
        throw InvalidInputException("Only pending orders can be changed.");
    for (auto& existing : items) {
        if (existing.getFurnitureId() == item.getFurnitureId()) {
            // merge: keep the price from the first time the item was added
            existing = OrderItem(existing.getFurnitureId(), existing.getName(),
                                 existing.getQuantity() + item.getQuantity(),
                                 existing.getUnitPrice());
            return;
        }
    }
    items.push_back(item);
}

void Order::removeItem(const std::string& furnitureId) {
    if (status != Status::Pending)
        throw InvalidInputException("Only pending orders can be changed.");
    for (auto it = items.begin(); it != items.end(); ++it) {
        if (it->getFurnitureId() == furnitureId) { items.erase(it); return; }
    }
    throw ItemNotFoundException("Item " + furnitureId + " is not in order " + orderId + ".");
}

double Order::calculateSubtotal() const {
    double sum = 0.0;
    for (const auto& it : items) sum += it.getLineTotal();
    return sum;
}

void Order::confirm(Inventory& inventory) {
    if (status != Status::Pending)
        throw InvalidInputException("Only pending orders can be confirmed.");
    if (items.empty())
        throw InvalidInputException("Cannot confirm an empty order.");

    // Pass 1: check everything first, so a failure changes nothing.
    for (const auto& it : items) {
        inventory.findById(it.getFurnitureId());                 // throws ItemNotFoundException
        if (!inventory.hasStock(it.getFurnitureId(), it.getQuantity()))
            throw OutOfStockException("Not enough stock for " + it.getName() + ".");
    }
    // Pass 2: reduce stock.
    for (const auto& it : items)
        inventory.reduceStock(it.getFurnitureId(), it.getQuantity());

    status = Status::Confirmed;
}

void Order::markDelivered() {
    if (status != Status::Confirmed)
        throw InvalidInputException("Only confirmed orders can be delivered.");
    status = Status::Delivered;
}

void Order::cancel() {
    if (status == Status::Delivered || status == Status::Cancelled)
        throw InvalidInputException("This order can no longer be cancelled.");
    status = Status::Cancelled;
}

std::string Order::getStatusText() const { return statusToText(status); }

std::string Order::statusToText(Status s) {
    switch (s) {
        case Status::Pending:   return "Pending";
        case Status::Confirmed: return "Confirmed";
        case Status::Delivered: return "Delivered";
        case Status::Cancelled: return "Cancelled";
    }
    return "Pending";
}

Order::Status Order::textToStatus(const std::string& text) {
    if (text == "Pending")   return Status::Pending;
    if (text == "Confirmed") return Status::Confirmed;
    if (text == "Delivered") return Status::Delivered;
    if (text == "Cancelled") return Status::Cancelled;
    throw InvalidInputException("Unknown order status: " + text);
}

// ---------------------------- OnlineOrder ----------------------------
OnlineOrder::OnlineOrder(std::string orderId, std::string customerId, std::string date,
                         std::string websiteRef, double handlingFee)
    : Order(std::move(orderId), std::move(customerId), std::move(date)),
      websiteRef(std::move(websiteRef)), handlingFee(handlingFee) {
    if (handlingFee < 0) throw InvalidInputException("Handling fee cannot be negative.");
}

std::string OnlineOrder::serialize() const {
    return Order::serialize() + "," + websiteRef + "," + std::to_string(handlingFee);
}

// ----------------------------- PhoneOrder -----------------------------
PhoneOrder::PhoneOrder(std::string orderId, std::string customerId, std::string date,
                       std::string callRef, std::string takenByStaff)
    : Order(std::move(orderId), std::move(customerId), std::move(date)),
      callRef(std::move(callRef)), takenByStaff(std::move(takenByStaff)) {}

std::string PhoneOrder::serialize() const {
    return Order::serialize() + "," + callRef + "," + takenByStaff;
}

// ---------------------------- InStoreOrder ----------------------------
InStoreOrder::InStoreOrder(std::string orderId, std::string customerId, std::string date,
                           int counterNumber)
    : Order(std::move(orderId), std::move(customerId), std::move(date)),
      counterNumber(counterNumber) {
    if (counterNumber <= 0) throw InvalidInputException("Counter number must be positive.");
}

std::string InStoreOrder::serialize() const {
    return Order::serialize() + "," + std::to_string(counterNumber);
}
