#pragma once
#include <string>
#include <vector>
#include "../Stubs/Customer.h"
#include "../Stubs/Exceptions.h"

class CustomerManager {
    std::vector<Customer> customers;
    int indexOf(const std::string& id) const;

public:
    // Validation helpers (public so the GUI can reuse them for live checks)
    static bool isValidPhone(const std::string& phone);   // exactly 10 digits
    static bool isValidEmail(const std::string& email);   // has '@' and '.' after it
    static void validate(const Customer& c);              // throws InvalidInputException

    // throws InvalidInputException, DuplicateIDException
    void addCustomer(const Customer& c);
    // throws ItemNotFoundException, InvalidInputException
    void editCustomer(const std::string& id, const std::string& name, const std::string& phone,
                      const std::string& email, const std::string& address);
    // throws ItemNotFoundException
    void removeCustomer(const std::string& id);
    Customer& findById(const std::string& id);
    bool exists(const std::string& id) const;

    std::vector<Customer> searchByName(const std::string& keyword) const;   // case-insensitive
    const std::vector<Customer>& getAll() const { return customers; }
    std::string generateNextId() const;                   // "C001", "C002", ...
    std::size_t size() const { return customers.size(); }
    void clear() { customers.clear(); }
};
