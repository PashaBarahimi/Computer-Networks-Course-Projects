#include "reservation.hpp"

Reservation::Reservation(int userId, int numOfBeds, date::year_month_day checkIn, date::year_month_day checkOut)
    : userId_(userId),
      numOfBeds_(numOfBeds),
      checkIn_(checkIn),
      checkOut_(checkOut) {}

void Reservation::modify(int numOfBeds) {
    numOfBeds_ = numOfBeds;
}

int Reservation::getNumOfBeds() const { return numOfBeds_; }
int Reservation::getUserId() const { return userId_; }
date::year_month_day Reservation::getCheckOut() const { return checkOut_; }

bool Reservation::hasConflict(date::year_month_day date) const {
    return date >= checkIn_ && date < checkOut_;
}

bool Reservation::isExpired(date::year_month_day date) const {
    return checkOut_ <= date;
}

bool Reservation::canBeCancelled(date::year_month_day date) const {
    return date < checkIn_;
}

bool Reservation::operator==(const Reservation& rhs) const {
    return userId_ == rhs.userId_ &&
           numOfBeds_ == rhs.numOfBeds_ &&
           checkIn_ == rhs.checkIn_ &&
           checkOut_ == rhs.checkOut_;
}

bool Reservation::operator!=(const Reservation& rhs) const {
    return !(*this == rhs);
}

nlohmann::json Reservation::toJson() const {
    return {
        {"id", userId_},
        {"numOfBeds", numOfBeds_},
        {"checkInDate", DateTime::toStr(checkIn_)},
        {"checkOutDate", DateTime::toStr(checkOut_)},
    };
}
