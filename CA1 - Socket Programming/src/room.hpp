#ifndef ROOM_HPP_INCLUDE
#define ROOM_HPP_INCLUDE

#include <json.hpp>
#include <string>

class Room {
public:
    Room(std::string num, int price, int maxCapacity);

    void modify(int newPrice, int newMaxCapacity);

    std::string getNumber() const;
    int getPrice() const;
    int getMaxCapacity() const;

    nlohmann::json toJson() const;

private:
    std::string number_;
    int price_;
    int maxCapacity_;
};

#endif // ROOM_HPP_INCLUDE
