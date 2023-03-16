#ifndef HOTEL_CLIENT_HPP_INCLUDE
#define HOTEL_CLIENT_HPP_INCLUDE

#include <json.hpp>
#include <string>

#include "net.hpp"

class HotelClient {
public:
    HotelClient(net::IpAddr host, net::Port port);
    bool connect();

    bool doesUserExist(const std::string& username);
    std::string signin(const std::string& username, const std::string& password);
    std::string signup(const std::string& username, const std::string& password, int balance, const std::string& phone, const std::string& address);
    std::string userInfo();
    std::string allUsers();
    std::string roomsInfo(bool onlyAvailable = false);
    std::string book(const std::string& roomNum, int numOfBeds, const std::string& checkInDate, const std::string& checkOutDate);
    std::string showReservations();
    std::string cancel(const std::string& roomNum, int numOfBeds);
    std::string passDay(int numOfDays);
    std::string editInfo(const std::string& password, const std::string& phone, const std::string& address);
    std::string leaveRoom(const std::string& roomNum);
    std::string addRoom(const std::string& roomNum, int maxCapacity, int price);
    std::string modifyRoom(const std::string& roomNum, int newMaxCapacity, int newPrice);
    std::string removeRoom(const std::string& roomNum);
    std::string logout();

    bool isLoggedIn() const;

private:
    net::IpAddr host_;
    net::Port port_;
    net::Socket socket_;

    std::string token_;
    std::string userId_;

    nlohmann::json requestJson(const std::string& request) const;
    nlohmann::json getResponse(const nlohmann::json& req);
    std::string statusMsg(const nlohmann::json& res) const;

    std::string formatUserInfo(const nlohmann::json& user) const;
};

#endif // HOTEL_CLIENT_HPP_INCLUDE
