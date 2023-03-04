#include "user.hpp"

User::User(int id, const std::string& username, const std::string& password, Role role,
           float balance, const std::string& phoneNumber, const std::string& address)
    : id_(id),
      username_(username),
      password_(password),
      role_(role),
      balance_(balance),
      phoneNumber_(phoneNumber),
      address_(address) {}

void User::editInfo(const std::string& newPassword, const std::string& newPhoneNumber, const std::string& newAddress) {
    password_ = newPassword;
    phoneNumber_ = newPhoneNumber;
    address_ = newAddress;
}

bool User::isPasswordCorrect(const std::string& hashedPassword) const {
    return password_ == hashedPassword;
}

User::Role User::getRole() const {
    return role_;
}

int User::getId() const {
    return id_;
}

std::string User::getUsername() const {
    return username_;
}
