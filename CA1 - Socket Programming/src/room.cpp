#include "room.hpp"

Room::Room(std::string num, int price, int maxCapacity)
    : number_(std::move(num)),
      price_(price),
      maxCapacity_(maxCapacity) {}

std::string Room::getNumber() const {
    return number_;
}

int Room::getPrice() const {
    return price_;
}

int Room::getMaxCapacity() const {
    return maxCapacity_;
}

nlohmann::json Room::toJson() const {
    return {
        {"number", number_},
        {"price", price_},
        {"maxCapacity", maxCapacity_},
    };
}

void Room::modify(int newPrice, int newMaxCapacity) {
    price_ = newPrice;
    maxCapacity_ = newMaxCapacity;
}
