#ifndef HOTEL_HPP_INCLUDE
#define HOTEL_HPP_INCLUDE

#include <chrono>
#include <future>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "crypto.hpp"
#include "user.hpp"

constexpr int TOKEN_LENGTH = 32;
constexpr std::chrono::minutes TOKEN_LIFETIME(30);

class Hotel {
public:
    Hotel();
    ~Hotel();

private:
    struct UserAccess {
        int userId;
        std::chrono::system_clock::time_point lastAccess;
    };

    std::vector<User> users_;
    std::unordered_map<std::string, UserAccess> tokens_;
    std::mutex tokensMutex_;
    std::thread tokenCleaner_;
    std::promise<void> tokenCleanerCancel_;

    std::string generateTokenForUser(int userId);
    std::string generateToken();
    void removeExistingToken(int userId);
    void cleanTokens();
    int getUser(const std::string& token);
    int findUser(const std::string& username) const;
    bool isPasswordCorrect(int userId, const std::string& password) const;
};

#endif // HOTEL_HPP_INCLUDE
