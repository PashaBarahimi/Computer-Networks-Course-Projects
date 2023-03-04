#include "hotel.hpp"

#include <chrono>
#include <random>

#include "crypto.hpp"

Hotel::Hotel() {
    tokenCleaner_ = std::thread(&Hotel::cleanTokens, this);
}

Hotel::~Hotel() {
    tokenCleanerCancel_.set_value();
    tokenCleaner_.join();
}

std::string Hotel::generateToken(int userId) {
    removeExistingToken(userId);

    static auto& chrs =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "!@#$%^&*()_+{}|:<>?,./;'[]\\-=";
    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    std::string token;
    token.reserve(32);
    while (true) {
        while (token.size() < 32) {
            token += chrs[pick(rg)];
        }
        if (tokens_.find(token) == tokens_.end()) {
            break;
        }
        token.clear();
    }
    tokens_[token] = {
        userId,
        std::chrono::system_clock::now(),
    };
    return token;
}

void Hotel::removeExistingToken(int userId) {
    for (auto it = tokens_.begin(); it != tokens_.end(); ++it) {
        if (it->second.userId == userId) {
            tokens_.erase(it);
            break;
        }
    }
}

void Hotel::cleanTokens() {
    while (tokenCleanerCancel_.get_future().wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
        for (auto it = tokens_.begin(); it != tokens_.end();) {
            if (std::chrono::system_clock::now() - it->second.lastAccess > std::chrono::minutes(30)) {
                it = tokens_.erase(it);
            }
            else {
                ++it;
            }
        }
        std::this_thread::sleep_for(std::chrono::minutes(10));
    }
}

int Hotel::getUser(const std::string& token) const {
    auto it = tokens_.find(token);
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
