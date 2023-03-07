#include "user.hpp"

User::User(int id, std::string username, std::string password, Role role,
           int balance, std::string phoneNumber, std::string address)
    : id_(id),
      username_(std::move(username)),
      password_(std::move(password)),
      role_(role),
      balance_(balance),
      phoneNumber_(std::move(phoneNumber)),
      address_(std::move(address)) {}

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
