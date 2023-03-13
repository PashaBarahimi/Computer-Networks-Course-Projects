#include "reservation.hpp"

Reservation::Reservation(int userId, int numOfBeds, date::year_month_day checkIn, date::year_month_day checkOut)
    : userId_(userId),
      numOfBeds_(numOfBeds),
      checkIn_(checkIn),
      checkOut_(checkOut) {}

void Reservation::modify(int numOfBeds) {
    numOfBeds_ = numOfBeds;
}

int Reservation::getNumOfBeds() const {
    return numOfBeds_;
}

int Reservation::getUserId() const {
    return userId_;
}

date::year_month_day Reservation::getCheckOut() const {
    return checkOut_;
}

bool Reservation::hasConflict(const date::year_month_day& date) const {
    return date >= checkIn_ && date < checkOut_;
}

bool Reservation::isExpired(const date::year_month_day& date) const {
    return checkOut_ <= date;
}

bool Reservation::canBeCancelled(const date::year_month_day& date) const {
    return date < checkIn_;
}

bool Reservation::operator==(const Reservation& other) const {
    return userId_ == other.userId_ && numOfBeds_ == other.numOfBeds_ && checkIn_ == other.checkIn_ && checkOut_ == other.checkOut_;
}

nlohmann::json Reservation::toJson() const {
    return {
        {"id", userId_},
        {"numOfBeds", numOfBeds_},
        {"checkInDate", date::format("%F", checkIn_)},
        {"checkOutDate", date::format("%F", checkOut_)},
    };
}
