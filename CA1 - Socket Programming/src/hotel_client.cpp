#include "hotel_client.hpp"

HotelClient::HotelClient(std::string host, int port)
    : host_(host),
      port_(port),
      isLoggedIn_(false) {}

std::string HotelClient::signin(const std::string& username, const std::string& password) {
    isLoggedIn_ = true;
    return "Signin";
}

bool HotelClient::doesUserExist(const std::string& username) {
    return false;
}

std::string HotelClient::signup(const std::string& username, const std::string& password, int balance, const std::string& phone, const std::string& email) {
    return "Signup";
}

std::string HotelClient::userInfo() {
    return "User Info";
}

std::string HotelClient::allUsers() {
    return "All Users";
}

std::string HotelClient::roomsInfo() {
    return "Rooms Info";
}

std::string HotelClient::book(const std::string& roomNum, int numOfBeds, const std::string& checkInDate, const std::string& checkOutDate) {
    return "Book";
}

std::string HotelClient::cancel(const std::string& roomNum, int numOfBeds) {
    return "Cancel";
}

std::string HotelClient::passDay(int numOfDays) {
    return "Pass Day";
}

std::string HotelClient::editInfo(const std::string& password, const std::string& phone, const std::string& email) {
    return "Edit Info";
}

std::string HotelClient::leaveRoom(const std::string& roomNum) {
    return "Leave Room";
}

std::string HotelClient::addRoom(const std::string& roomNum, int maxCapacity, int price) {
    return "Add Room";
}

std::string HotelClient::modifyRoom(const std::string& roomNum, int newMaxCapacity, int newPrice) {
    return "Modify Room";
}

std::string HotelClient::removeRoom(const std::string& roomNum) {
    return "Remove Room";
}

std::string HotelClient::logout() {
    isLoggedIn_ = false;
    return "Logout";
}

bool HotelClient::isLoggedIn() const {
    return isLoggedIn_;
}
