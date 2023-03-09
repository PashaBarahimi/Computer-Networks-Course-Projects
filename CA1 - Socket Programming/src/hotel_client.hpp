#ifndef HOTEL_CLIENT_HPP_INCLUDE
#define HOTEL_CLIENT_HPP_INCLUDE

#include <string>

class HotelClient {
public:
    HotelClient(std::string host, int port);

    std::string signin(const std::string& username, const std::string& password);
    bool doesUserExist(const std::string& username);
    std::string signup(const std::string& username, const std::string& password, int balance, const std::string& phone, const std::string& email);
    std::string userInfo();
    std::string allUsers();
    std::string roomsInfo();
    std::string book(const std::string& roomNum, int numOfBeds, const std::string& checkInDate, const std::string& checkOutDate);
    std::string cancel(const std::string& roomNum, int numOfBeds);
    std::string passDay(int numOfDays);
    std::string editInfo(const std::string& password, const std::string& phone, const std::string& email);
    std::string leaveRoom(const std::string& roomNum);
    std::string addRoom(const std::string& roomNum, int maxCapacity, int price);
    std::string modifyRoom(const std::string& roomNum, int newMaxCapacity, int newPrice);
    std::string removeRoom(const std::string& roomNum);
    std::string logout();

    bool isLoggedIn() const;

private:
    std::string host_;
    int port_;
    bool isLoggedIn_;
};

#endif // HOTEL_CLIENT_HPP_INCLUDE
