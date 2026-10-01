// STUB - Vimukthi owns the real Person/Customer classes.
#pragma once
#include <string>

class Customer {
    std::string id, name, phone, email, address;
public:
    Customer(std::string id, std::string name, std::string phone,
             std::string email, std::string address)
        : id(id), name(name), phone(phone), email(email), address(address) {}
    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    const std::string& getPhone() const { return phone; }
    const std::string& getEmail() const { return email; }
    const std::string& getAddress() const { return address; }
    void setName(const std::string& v) { name = v; }
    void setPhone(const std::string& v) { phone = v; }
    void setEmail(const std::string& v) { email = v; }
    void setAddress(const std::string& v) { address = v; }
    // Format: id,name,phone,email,address
    std::string serialize() const { return id + "," + name + "," + phone + "," + email + "," + address; }
};
