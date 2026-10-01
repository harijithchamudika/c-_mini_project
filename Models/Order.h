#pragma once
#include <string>
#include <vector>
#include "OrderItem.h"

class Inventory;   // forward declaration (avoids circular includes)

// Abstract base class. Channels differ only in extra data and channel charge.
class Order {
public:
    enum class Status { Pending, Confirmed, Delivered, Cancelled };

protected:
    std::string orderId;
    std::string customerId;
    std::string date;              // YYYY-MM-DD
    Status status;
    std::vector<OrderItem> items;  // composition: items live inside the order

public:
    Order(std::string orderId, std::string customerId, std::string date);
    virtual ~Order() = default;

    // ---- polymorphic ----
    virtual std::string getChannel() const = 0;
    virtual double getChannelCharge() const = 0;
    virtual std::string serialize() const;        // orders.csv line

    // ---- common behaviour ----
    void addItem(const OrderItem& item);          // merges quantity if same furniture
    void removeItem(const std::string& furnitureId);   // throws ItemNotFoundException
    double calculateSubtotal() const;
    double calculateTotal() const { return calculateSubtotal() + getChannelCharge(); }

    // Checks stock for every item, then reduces stock. Only works when Pending.
    // throws OutOfStockException / ItemNotFoundException / InvalidInputException
    void confirm(Inventory& inventory);
    void markDelivered();
    void cancel();

    // ---- getters ----
    const std::string& getOrderId() const { return orderId; }
    const std::string& getCustomerId() const { return customerId; }
    const std::string& getDate() const { return date; }
    const std::vector<OrderItem>& getItems() const { return items; }
    Status getStatus() const { return status; }
    std::string getStatusText() const;
    void setStatus(Status s) { status = s; }      // used by FileManager when loading

    static std::string statusToText(Status s);
    static Status textToStatus(const std::string& text);   // throws InvalidInputException
};

class OnlineOrder : public Order {
    std::string websiteRef;
    double handlingFee;
public:
    OnlineOrder(std::string orderId, std::string customerId, std::string date,
                std::string websiteRef, double handlingFee = 5.0);
    std::string getChannel() const override { return "Online"; }
    double getChannelCharge() const override { return handlingFee; }
    std::string serialize() const override;       // adds websiteRef,handlingFee
};

class PhoneOrder : public Order {
    std::string callRef;
    std::string takenByStaff;
public:
    PhoneOrder(std::string orderId, std::string customerId, std::string date,
               std::string callRef, std::string takenByStaff);
    std::string getChannel() const override { return "Phone"; }
    double getChannelCharge() const override { return 0.0; }
    std::string serialize() const override;       // adds callRef,takenByStaff
};

class InStoreOrder : public Order {
    int counterNumber;
public:
    InStoreOrder(std::string orderId, std::string customerId, std::string date,
                 int counterNumber);
    std::string getChannel() const override { return "InStore"; }
    double getChannelCharge() const override { return 0.0; }
    std::string serialize() const override;       // adds counterNumber
};
