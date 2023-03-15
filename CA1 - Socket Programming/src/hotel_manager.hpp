#ifndef HOTEL_MANAGER_HPP_INCLUDE
#define HOTEL_MANAGER_HPP_INCLUDE

#include <condition_variable>
#include <fstream>
#include <json.hpp>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "crypto.hpp"
#include "datetime.hpp"
#include "logger.hpp"
#include "net.hpp"
#include "reservation.hpp"
#include "room.hpp"
#include "user.hpp"

constexpr int TOKEN_LENGTH = 32;
constexpr std::chrono::minutes TOKEN_LIFETIME(30);

const std::string LOG_FILE = "misasha.log";
const std::string USERS_FILE = "data/usersinfo.json";
const std::string ROOMS_FILE = "data/roomsinfo.json";

class HotelManager {
public:
    HotelManager(net::IpAddr ip, net::Port port);
    ~HotelManager();

    void run();

private:
    struct UserAccess {
        int userId;
        std::chrono::system_clock::time_point lastAccess;
    };

    net::IpAddr ip_;
    net::Port port_;
    std::ofstream logFile_;
    Logger logger_;
    net::Socket socket_;

    std::vector<User> users_;
    std::unordered_map<std::string, Room> rooms_;
    std::unordered_map<std::string, std::vector<Reservation>> reservations_;

    std::unordered_map<std::string, UserAccess> tokens_;
    std::unordered_map<net::Socket*, std::string> socketToToken_;
    std::mutex tokensMutex_;
    std::thread tokenCleaner_;
    std::condition_variable tokenCleanerCancel_;
    bool tokenCancel_ = false;

    void loadUsers();
    void loadRooms();
    void setupServer();
    void handleConnections();
    void handleRequest(const nlohmann::json& request, net::Socket* client);

    std::string generateTokenForUser(int userId);
    void removeExistingToken(int userId);
    void refreshTokenAccessTime(const std::string& token);
    void cleanTokens();
    void removeToken(const std::string& token);

    void commitChanges();
    void commitUsers();
    void commitRooms();

    bool hasArgument(const nlohmann::json& request, const std::string& argument);
    bool getRequestToken(const nlohmann::json& request, std::string& token);

    nlohmann::json handleSignin(const nlohmann::json& request);
    nlohmann::json handleSignup(const nlohmann::json& request);
    nlohmann::json handleCheckUsername(const nlohmann::json& request);
    nlohmann::json handleUserInfo(const nlohmann::json& request);
    nlohmann::json handleAllUsers(const nlohmann::json& request);
    nlohmann::json handleRoomsInfo(const nlohmann::json& request);
    nlohmann::json handleBook(const nlohmann::json& request);
    nlohmann::json handleCancel(const nlohmann::json& request);
    nlohmann::json handlePassDay(const nlohmann::json& request);
    nlohmann::json handleEditInfo(const nlohmann::json& request);
    nlohmann::json handleLeaveRoom(const nlohmann::json& request);
    nlohmann::json handleAddRoom(const nlohmann::json& request);
    nlohmann::json handleModifyRoom(const nlohmann::json& request);
    nlohmann::json handleRemoveRoom(const nlohmann::json& request);
    nlohmann::json handleLogout(const nlohmann::json& request);

    nlohmann::json getUserInfo(int userId) const;
    nlohmann::json getAllUsers() const;
    nlohmann::json getRoomsInfo(bool onlyAvailable, bool showReservations) const;

    bool isAdministrator(int userId) const;
    bool isPasswordCorrect(int userId, const std::string& password) const;
    bool isResidence(int userId, const std::string& roomNum) const;
    bool hasReservation(int userId, const std::string& roomNum, int numOfBeds) const;
    bool doesRoomExist(const std::string& roomNum) const;
    bool canModifyRoom(const std::string& roomNum, int maxCapacity) const;
    bool canRemoveRoom(const std::string& roomNum) const;
    bool isRoomAvailable(const std::string& roomNum, int numOfBed, date::year_month_day checkIn, date::year_month_day checkOut) const;
    bool hasEnoughBalance(int userId, const std::string& roomNum, int numOfBeds) const;
    int findMaximumUsers(const std::string& roomNum, date::year_month_day start, date::year_month_day end) const;
    int findUser(const std::string& username) const;
    int getUser(const std::string& token);
    int getRoomCapacity(const std::string& roomNum) const;
    void addUser(const std::string& username, const std::string& password, int balance, const std::string& email, const std::string& phone);
    void logoutUser(const std::string& token);
    void editUser(int userId, const std::string& password, const std::string& email, const std::string& phone);
    void checkOutExpiredReservations();
    void leaveRoom(int userId, const std::string& roomNum);
    void addRoom(const std::string& roomNum, int maxCapacity, int price);
    void modifyRoom(const std::string& roomNum, int maxCapacity, int price);
    void removeRoom(const std::string& roomNum);
    void cancelReservation(int userId, const std::string& roomNum, int numOfBeds);
    void bookRoom(int userId, const std::string& roomNum, int numOfBeds, date::year_month_day checkIn, date::year_month_day checkOut);
};

#endif // HOTEL_MANAGER_HPP_INCLUDE
