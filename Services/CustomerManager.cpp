#include "CustomerManager.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace {
std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
}

int CustomerManager::indexOf(const std::string& id) const {
    for (std::size_t i = 0; i < customers.size(); ++i)
        if (customers[i].getId() == id) return static_cast<int>(i);
    return -1;
}

bool CustomerManager::isValidPhone(const std::string& phone) {
    if (phone.size() != 10) return false;
    return std::all_of(phone.begin(), phone.end(),
                       [](unsigned char c) { return std::isdigit(c) != 0; });
}

bool CustomerManager::isValidEmail(const std::string& email) {
    std::size_t at = email.find('@');
    if (at == std::string::npos || at == 0) return false;
    std::size_t dot = email.find('.', at + 1);
    return dot != std::string::npos && dot > at + 1 && dot + 1 < email.size();
}

void CustomerManager::validate(const Customer& c) {
    if (c.getId().empty())   throw InvalidInputException("Customer ID cannot be empty.");
    if (c.getName().empty()) throw InvalidInputException("Customer name cannot be empty.");
    if (!isValidPhone(c.getPhone()))
        throw InvalidInputException("Phone number must be exactly 10 digits.");
    if (!isValidEmail(c.getEmail()))
        throw InvalidInputException("Email must contain '@' followed by a '.'.");
}

void CustomerManager::addCustomer(const Customer& c) {
    validate(c);
    if (exists(c.getId()))
        throw DuplicateIDException("Customer ID " + c.getId() + " already exists.");
    customers.push_back(c);
}

void CustomerManager::editCustomer(const std::string& id, const std::string& name,
                                   const std::string& phone, const std::string& email,
                                   const std::string& address) {
    int i = indexOf(id);
    if (i < 0) throw ItemNotFoundException("Customer " + id + " not found.");
    validate(Customer(id, name, phone, email, address));   // check BEFORE changing anything
    customers[i].setName(name);
    customers[i].setPhone(phone);
    customers[i].setEmail(email);
    customers[i].setAddress(address);
}

void CustomerManager::removeCustomer(const std::string& id) {
    int i = indexOf(id);
    if (i < 0) throw ItemNotFoundException("Customer " + id + " not found.");
    customers.erase(customers.begin() + i);
}

Customer& CustomerManager::findById(const std::string& id) {
    int i = indexOf(id);
    if (i < 0) throw ItemNotFoundException("Customer " + id + " not found.");
    return customers[i];
}

bool CustomerManager::exists(const std::string& id) const { return indexOf(id) >= 0; }

std::vector<Customer> CustomerManager::searchByName(const std::string& keyword) const {
    std::vector<Customer> out;
    const std::string key = lower(keyword);
    for (const auto& c : customers)
        if (lower(c.getName()).find(key) != std::string::npos) out.push_back(c);
    return out;
}

std::string CustomerManager::generateNextId() const {
    int maxNum = 0;
    for (const auto& c : customers) {
        const std::string& id = c.getId();
        if (id.size() > 1 && id[0] == 'C' &&
            std::all_of(id.begin() + 1, id.end(), [](unsigned char ch) { return std::isdigit(ch) != 0; }))
            maxNum = std::max(maxNum, std::stoi(id.substr(1)));
    }
    std::ostringstream os;
    os << "C" << std::setw(3) << std::setfill('0') << (maxNum + 1);
    return os.str();
}
