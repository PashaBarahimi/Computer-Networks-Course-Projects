#ifndef HOTEL_HPP_INCLUDE
#define HOTEL_HPP_INCLUDE

#include <future>
#include <thread>
#include <unordered_map>
#include <vector>

#include "crypto.hpp"
#include "user.hpp"

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
    std::thread tokenCleaner_;
    std::promise<void> tokenCleanerCancel_;

    std::string generateToken(int userId);
    void removeExistingToken(int userId);
    void cleanTokens();
    int getUser(const std::string& token) const;
    int findUser(const std::string& username) const;
    bool isPasswordCorrect(int userId, const std::string& password) const;
};

#endif // HOTEL_HPP_INCLUDE
