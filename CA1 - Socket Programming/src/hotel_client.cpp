#include "hotel_client.hpp"

#include <json.hpp>
#include <sstream>
#include <stdexcept>

#include "crypto.hpp"
#include "status_code.hpp"

HotelClient::HotelClient(net::IpAddr host, net::Port port)
    : host_(host),
      port_(port),
      socket_(net::Socket::Type::stream) {}

bool HotelClient::connect() {
    return socket_.connect(host_, port_);
}

nlohmann::json HotelClient::requestJson(const std::string& request) const {
    nlohmann::json j;
    j["command"] = request;
    j["arguments"] = nullptr;
    if (isLoggedIn()) {
        j["token"] = token_;
    }
    else {
        j["token"] = nullptr;
    }
    return j;
}

nlohmann::json HotelClient::getResponse(const nlohmann::json& req) {
    if (!socket_.send(req.dump())) {
        throw std::runtime_error("Could not send request to the server.");
    }
    std::string resStr;
    if (!socket_.receive(resStr)) {
        throw std::runtime_error("Could not receive response from the server.");
    }
    auto res = nlohmann::json::parse(resStr);
    if (res["status"] == StatusCode::Unauthorized) {
        userId_.clear();
    }
    return res;
}

std::string HotelClient::statusMsg(const nlohmann::json& res) const {
    std::ostringstream sstr;
    sstr << '[' << res["status"] << ']';
    if (res["message"].is_null() || res["message"].empty()) {
        return sstr.str();
    }
    sstr << ": " << res["message"];
    return sstr.str();
}

bool HotelClient::doesUserExist(const std::string& username) {
    auto req = requestJson("checkUsername");
    req["arguments"] = {
        {"username", username},
    };
    auto res = getResponse(req);
    if (res["status"] != StatusCode::UsernameExists &&
        res["status"] != StatusCode::UsernameDoesNotExist) {
        throw std::runtime_error("Unexpected status code.");
    }
    return !res["response"]["checkUsername"];
}

std::string HotelClient::signin(const std::string& username, const std::string& password) {
    auto req = requestJson("signin");
    req["arguments"] = {
        {"username", username},
        {"password", crypto::base64Encode(password)},
    };
    auto res = getResponse(req);
    if (res["status"] == StatusCode::SignedIn) {
        userId_ = res["userId"];
        token_ = res["response"]["token"];
    }
    return statusMsg(res);
}

std::string HotelClient::signup(const std::string& username, const std::string& password, int balance, const std::string& phone, const std::string& address) {
    auto req = requestJson("signup");
    req["arguments"] = {
        {"username", username},
        {"password", crypto::base64Encode(password)},
        {"balance", balance},
        {"phone", phone},
        {"address", address},
    };
    auto res = getResponse(req);
    return statusMsg(res);
}

std::string HotelClient::formatUserInfo(const nlohmann::json& user) const {
    std::ostringstream sstr;
    if (user["admin"]) {
        sstr << "-- User #" << user["id"] << " (admin):\n";
        sstr << "| Username: " << user["username"].get<std::string>();
    }
    else {
        sstr << "-- User #" << user["id"] << " (normal)\n";
        sstr << "| Username: " << user["username"].get<std::string>() << '\n';
        sstr << "| Balance:  " << user["balance"] << '\n';
        sstr << "| Phone:    " << user["phone"].get<std::string>() << '\n';
        sstr << "| Address:  " << user["address"].get<std::string>();
    }
    return sstr.str();
}

std::string HotelClient::userInfo() {
    auto req = requestJson("userInfo");
    auto res = getResponse(req);
    if (res["status"] != StatusCode::OK) {
        return statusMsg(res);
    }
    return formatUserInfo(res["response"]);
}

std::string HotelClient::allUsers() {
    auto req = requestJson("allUsers");
    auto res = getResponse(req);
    if (res["status"] != StatusCode::OK) {
        return statusMsg(res);
    }
    std::ostringstream sstr;
    for (const auto& user : res["response"]) {
        sstr << formatUserInfo(user) << '\n';
    }
    auto ret = sstr.str();
    ret.pop_back();
    return ret;
}

std::string HotelClient::roomsInfo(bool onlyAvailable) {
    auto req = requestJson("roomsInfo");
    req["arguments"] = {
        {"onlyAvailable", onlyAvailable},
    };
    auto res = getResponse(req);
    if (res["status"] != StatusCode::OK) {
        return statusMsg(res);
    }
    std::ostringstream sstr;
    for (const auto& room : res["response"]) {
        sstr << "-- Room #" << room["number"].get<std::string>() << ":\n";
        sstr << "| Price: " << room["price"] << '\n';
        sstr << "| Max Capacity: " << room["maxCapacity"] << '\n';
        if (room.contains("reservations")) {
            int resId = 1;
            sstr << "| Reservations:\n";
            for (const auto& reservation : room["reservations"]) {
                sstr << "| -- Reservation #" << resId++ << ":\n";
                sstr << "| User ID: " << reservation["id"] << '\n';
                sstr << "| Number of Beds: " << reservation["numOfBeds"] << '\n';
                sstr << "| Check-in date:  " << reservation["checkInDate"].get<std::string>() << '\n';
                sstr << "| Check-out date: " << reservation["checkOutDate"].get<std::string>() << '\n';
            }
        }
    }
    auto ret = sstr.str();
    ret.pop_back();
    return ret;
}

std::string HotelClient::book(const std::string& roomNum, int numOfBeds, const std::string& checkInDate, const std::string& checkOutDate) {
    auto req = requestJson("book");
    req["arguments"] = {
        {"roomNum", roomNum},
        {"numOfBeds", numOfBeds},
        {"checkInDate", checkInDate},
        {"checkOutDate", checkOutDate},
    };
    auto res = getResponse(req);
    return statusMsg(res);
}

std::string HotelClient::cancel(const std::string& roomNum, int numOfBeds) {
    auto req = requestJson("cancel");
    req["arguments"] = {
        {"roomNum", roomNum},
        {"numOfBeds", numOfBeds},
    };
    auto res = getResponse(req);
    return statusMsg(res);
}

std::string HotelClient::passDay(int numOfDays) {
    auto req = requestJson("passDay");
    req["arguments"] = {
        {"numOfDays", numOfDays},
    };
    auto res = getResponse(req);
    return statusMsg(res);
}

std::string HotelClient::editInfo(const std::string& password, const std::string& phone, const std::string& address) {
    auto req = requestJson("editInfo");
    req["arguments"] = {
        {"password", crypto::base64Encode(password)},
        {"phone", phone},
        {"address", address},
    };
    auto res = getResponse(req);
    return statusMsg(res);
}

std::string HotelClient::leaveRoom(const std::string& roomNum) {
    auto req = requestJson("leaveRoom");
    req["arguments"] = {
        {"roomNum", roomNum},
    };
    auto res = getResponse(req);
    return statusMsg(res);
}

std::string HotelClient::addRoom(const std::string& roomNum, int maxCapacity, int price) {
    auto req = requestJson("addRoom");
    req["arguments"] = {
        {"roomNum", roomNum},
        {"maxCapacity", maxCapacity},
        {"price", price},
    };
    auto res = getResponse(req);
    return statusMsg(res);
}

std::string HotelClient::modifyRoom(const std::string& roomNum, int newMaxCapacity, int newPrice) {
    auto req = requestJson("modifyRoom");
    req["arguments"] = {
        {"roomNum", roomNum},
        {"newMaxCapacity", newMaxCapacity},
        {"newPrice", newPrice},
    };
    auto res = getResponse(req);
    return statusMsg(res);
}

std::string HotelClient::removeRoom(const std::string& roomNum) {
    auto req = requestJson("removeRoom");
    req["arguments"] = {
        {"roomNum", roomNum},
    };
    auto res = getResponse(req);
    return statusMsg(res);
}

std::string HotelClient::logout() {
    auto req = requestJson("logout");
    auto res = getResponse(req);
    userId_.clear();
    return statusMsg(res);
}

bool HotelClient::isLoggedIn() const {
    return !userId_.empty();
}
