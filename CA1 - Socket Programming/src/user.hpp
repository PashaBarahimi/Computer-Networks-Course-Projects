#ifndef USER_HPP_INCLUDE
#define USER_HPP_INCLUDE

#include <json.hpp>
#include <string>

class User {
public:
    enum class Role {
        Admin,
        User
    };

    User(int id, std::string username, std::string password, Role role,
         int balance = 0, std::string phoneNumber = "", std::string address = "");

    void editInfo(const std::string& newPassword, const std::string& newPhoneNumber, const std::string& newAddress);
    void increaseBalance(int amount);
    void decreaseBalance(int amount);

    bool isPasswordCorrect(const std::string& hashedPassword) const;
    Role getRole() const;
    int getId() const;
    std::string getUsername() const;
    int getBalance() const;
    nlohmann::json toJson(bool includePassword = true) const;

private:
    int id_;
    std::string username_;
    std::string password_;
    Role role_;
    int balance_;
    std::string phoneNumber_;
    std::string address_;
};

#endif // USER_HPP_INCLUDE
