#ifndef USER_HPP_INCLUDE
#define USER_HPP_INCLUDE

#include <string>

class User {
public:
    enum class Role {
        Admin,
        User
    };

    User(int id, const std::string& username, const std::string& password, Role role,
         float balance = 0, const std::string& phoneNumber = "", const std::string& address = "");

    void editInfo(const std::string& newPassword, const std::string& newPhoneNumber, const std::string& newAddress);

    bool isPasswordCorrect(const std::string& hashedPassword) const;
    Role getRole() const;
    int getId() const;
    std::string getUsername() const;

private:
    int id_;
    std::string username_;
    std::string password_;
    Role role_;
    float balance_;
    std::string phoneNumber_;
    std::string address_;
};

#endif // USER_HPP_INCLUDE
