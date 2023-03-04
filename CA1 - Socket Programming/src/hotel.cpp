#include "hotel.hpp"

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

    while (true) {
        token = generateToken();
        tokensMutex_.lock();
        if (tokens_.find(token) == tokens_.end()) {
            tokensMutex_.unlock();
            break;
        }
        tokensMutex_.unlock();
    }
    tokensMutex_.lock();
    tokens_[token] = {
        userId,
        std::chrono::system_clock::now(),
    };
    tokensMutex_.unlock();
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
    tokensMutex_.lock();
    for (auto it = tokens_.begin(); it != tokens_.end(); ++it) {
        if (it->second.userId == userId) {
            tokens_.erase(it);
            break;
        }
    }
    tokensMutex_.unlock();
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
    tokensMutex_.lock();
    auto it = tokens_.find(token);
    tokensMutex_.unlock();
    if (it == tokens_.end()) {
        return -1;
    }
    return it->second.userId;
}

int Hotel::findUser(const std::string& username) const {
    for (int i = 0; i < users_.size(); ++i) {
        if (users_[i].getUsername() == username) {
            return i;
        }
    }
    return -1;
}

bool Hotel::isPasswordCorrect(int userId, const std::string& password) const {
    return users_[userId].isPasswordCorrect(Crypto::SHA256(password));
}
