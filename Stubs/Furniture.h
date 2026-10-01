// STUB - Vimukthi owns the real Furniture hierarchy. Keep these function names
// so your code does not change when his version replaces this one.
#pragma once
#include <string>
#include "Exceptions.h"

class Furniture {
protected:
    std::string id, name, material, color;
    double basePrice;
    int quantity;
public:
    Furniture(std::string id, std::string name, std::string material,
              std::string color, double basePrice, int quantity)
        : id(id), name(name), material(material), color(color),
          basePrice(basePrice), quantity(quantity) {
        if (basePrice < 0 || quantity < 0) throw InvalidInputException("Negative price or quantity.");
    }
    virtual ~Furniture() = default;

    virtual double calculatePrice() const = 0;
    virtual std::string getCategory() const = 0;
    // Format: category,id,name,material,color,basePrice,quantity[,extra]
    virtual std::string serialize() const {
        return getCategory() + "," + id + "," + name + "," + material + "," + color + ","
             + std::to_string(basePrice) + "," + std::to_string(quantity);
    }

    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    double getBasePrice() const { return basePrice; }
    int getQuantity() const { return quantity; }
    void setQuantity(int q) { if (q < 0) throw InvalidInputException("Quantity cannot be negative."); quantity = q; }
    void setBasePrice(double p) { if (p < 0) throw InvalidInputException("Price cannot be negative."); basePrice = p; }
};

class Chair : public Furniture {
public:
    using Furniture::Furniture;
    double calculatePrice() const override { return basePrice; }
    std::string getCategory() const override { return "Chair"; }
};

class Sofa : public Furniture {
    int seats;
public:
    Sofa(std::string id, std::string name, std::string material, std::string color,
         double price, int qty, int seats = 3)
        : Furniture(id, name, material, color, price, qty), seats(seats) {}
    double calculatePrice() const override { return basePrice + 25.0; }  // assembly fee
    std::string getCategory() const override { return "Sofa"; }
    std::string serialize() const override { return Furniture::serialize() + "," + std::to_string(seats); }
};
