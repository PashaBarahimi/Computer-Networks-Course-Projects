#include "hotel.hpp"

#include <algorithm>

#include "crypto.hpp"
#include "strutils.hpp"

Hotel::Hotel() {
    tokenCleaner_ = std::thread(&Hotel::cleanTokens, this);
}

Hotel::~Hotel() {
    tokenCancel_ = true;
    tokenCleanerCancel_.notify_one();
    tokenCleaner_.join();
}

std::string Hotel::generateTokenForUser(int userId) {
    removeExistingToken(userId);
    std::string token;

    std::lock_guard<std::mutex> lock(tokensMutex_);
    while (true) {
        token = strutils::random(TOKEN_LENGTH);
        if (tokens_.find(token) == tokens_.end()) {
            break;
        }
    }
    tokens_[token] = {
        userId,
        std::chrono::system_clock::now(),
    };
    return token;
}

void Hotel::removeExistingToken(int userId) {
    std::lock_guard<std::mutex> lock(tokensMutex_);
    auto it = std::find_if(tokens_.begin(), tokens_.end(), [userId](const auto& token) {
        return token.second.userId == userId;
    });
    if (it != tokens_.end()) {
        tokens_.erase(it);
    }
}

void Hotel::cleanTokens() {
    const std::chrono::minutes waitTime(1);
    while (true) {
        std::unique_lock<std::mutex> guard(tokensMutex_);
        if (tokenCleanerCancel_.wait_for(guard, waitTime, [this]() { return this->tokenCancel_; })) {
            break;
        }

        for (auto it = tokens_.begin(); it != tokens_.end();) {
            if (std::chrono::system_clock::now() - it->second.lastAccess > TOKEN_LIFETIME) {
                it = tokens_.erase(it);
            }
            else {
                ++it;
            }
        }
    }
}

int Hotel::getUser(const std::string& token) {
    std::lock_guard<std::mutex> lock(tokensMutex_);
    auto it = tokens_.find(token);
    if (it == tokens_.end()) {
        return -1;
    }
    return it->second.userId;
}

int Hotel::findUser(const std::string& username) const {
    auto it = std::find_if(users_.begin(), users_.end(), [username](const auto& user) {
        return user.getUsername() == username;
    });
    if (it == users_.end()) {
        return -1;
    }
    return it->getId();
}

bool Hotel::isPasswordCorrect(int userId, const std::string& password) const {
    return users_[userId].isPasswordCorrect(crypto::SHA256(password));
}
