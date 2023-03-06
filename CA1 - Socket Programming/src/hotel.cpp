#include "hotel.hpp"

#include <algorithm>
#include <random>

#include "crypto.hpp"

Hotel::Hotel() {
    tokenCleaner_ = std::thread(&Hotel::cleanTokens, this);
}

Hotel::~Hotel() {
    tokenCleanerCancel_.set_value();
    tokenCleaner_.join();
}

std::string Hotel::generateTokenForUser(int userId) {
    removeExistingToken(userId);
    std::string token;

    std::lock_guard<std::mutex> lock(tokensMutex_);
    while (true) {
        token = generateToken();
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

std::string Hotel::generateToken() {
    static auto& chrs =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "!@#$%^&*()_+{}|:<>?,./;'[]\\-=";
    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    std::string token;
    token.reserve(TOKEN_LENGTH);
    while (token.size() < TOKEN_LENGTH) {
        token += chrs[pick(rg)];
    }
    return token;
}

void Hotel::removeExistingToken(int userId) {
    std::lock_guard<std::mutex> lock(tokensMutex_);
    tokens_.erase(std::remove_if(tokens_.begin(), tokens_.end(), [userId](const auto& token) {
                      return token.second.userId == userId;
                  }),
                  tokens_.end());
}

void Hotel::cleanTokens() {
    while (tokenCleanerCancel_.get_future().wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
        tokensMutex_.lock();
        for (auto it = tokens_.begin(); it != tokens_.end();) {
            if (std::chrono::system_clock::now() - it->second.lastAccess > TOKEN_LIFETIME) {
                it = tokens_.erase(it);
            }
            else {
                ++it;
            }
        }
        tokensMutex_.unlock();
        std::this_thread::sleep_for(std::chrono::minutes(10));
    }
}

int Hotel::getUser(const std::string& token) {
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
