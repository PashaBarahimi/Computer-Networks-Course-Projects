#include "hotel_manager.hpp"

#include <algorithm>
#include <functional>
#include <iomanip>

#include "crypto.hpp"
#include "datetime.hpp"
#include "status_code.hpp"
#include "strutils.hpp"

using namespace std::string_literals;

HotelManager::HotelManager(net::IpAddr ip, net::Port port)
    : ip_(ip),
      port_(port),
      logFile_(LOG_FILE, std::ios::app),
      logger_(Logger::Level::Info, logFile_) {
    loadUsers();
    loadRooms();
    tokenCleaner_ = std::thread(&HotelManager::cleanTokens, this);
}

HotelManager::~HotelManager() {
    tokenCancel_ = true;
    tokenCleanerCancel_.notify_one();
    tokenCleaner_.join();
}

void HotelManager::run() {
    setupServer();
    logger_.info("Server started", __func__);
    handleConnections();
}

void HotelManager::loadUsers() {
    std::ifstream file(USERS_FILE);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open users file");
    }
    nlohmann::json users;
    try {
        file >> users;
    }
    catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("Failed to parse users file: "s + e.what());
    }

    for (const auto& user : users["users"]) {
        int id = user["id"];
        std::string username = user["user"];
        std::string password = user["password"];
        bool isAdmin = user["admin"];
        int balance;
        std::string phone, address;
        if (user.contains("balance")) {
            balance = user["balance"];
        }
        if (user.contains("phone")) {
            phone = user["phone"];
        }
        if (user.contains("address")) {
            address = user["address"];
        }
        users_.emplace_back(id, username, password, isAdmin ? User::Role::Admin : User::Role::User, balance, phone, address);
    }
    logger_.info("Loaded " + std::to_string(users_.size()) + " users", __func__);
}

void HotelManager::loadRooms() {
    std::ifstream file(ROOMS_FILE);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open rooms file");
    }
    nlohmann::json rooms;
    try {
        file >> rooms;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse rooms file: "s + e.what());
    }

    for (const auto& room : rooms["rooms"]) {
        std::string roomNum = room["number"];
        int price = room["price"];
        int maxCapacity = room["maxCapacity"];
        rooms_.emplace(roomNum, Room(roomNum, price, maxCapacity));
        reservations_.emplace(roomNum, std::vector<Reservation>());
        for (const auto& reservation : room["users"]) {
            int userId = reservation["id"];
            int numOfBeds = reservation["numOfBeds"];
            std::string checkIn = reservation["checkInDate"];
            std::string checkOut = reservation["checkOutDate"];
            date::year_month_day checkInDate, checkOutDate;
            DateTime::parse(checkIn, checkInDate);
            DateTime::parse(checkOut, checkOutDate);
            reservations_[roomNum].emplace_back(userId, numOfBeds, checkInDate, checkOutDate);
        }
    }
    logger_.info("Loaded " + std::to_string(rooms_.size()) + " rooms", __func__);
}

void HotelManager::setupServer() {
    socket_ = net::Socket(net::Socket::Type::stream);
    if (!socket_.bind(ip_, port_)) {
        throw std::runtime_error("Failed to bind socket, port already in use");
    }
    if (!socket_.listen(10)) {
        throw std::runtime_error("Failed to listen on socket");
    }
}

void HotelManager::handleConnections() {
    net::Select sel;
    sel.addRead(&socket_, false);

    while (true) {
        sel.select();
        auto socks = sel.getReadyRead();

        for (auto sock : socks) {
            if (*sock == socket_) {
                auto client = new net::Socket();
                if (socket_.accept(*client)) {
                    logger_.info("New client connected", __func__);
                    sel.addRead(client, true);
                }
                else {
                    delete client;
                }
            }
            else {
                nlohmann::json request;
                std::string requestStr;
                if (!sock->receive(requestStr)) {
                    logger_.error("Failed to receive request", __func__);
                    continue;
                }
                if (requestStr.empty()) { // client closed
                    logoutUser(socketToToken_[sock]);
                    socketToToken_.erase(sock);
                    sel.removeRead(sock);
                    continue;
                }

                try {
                    request = nlohmann::json::parse(requestStr);
                }
                catch (const nlohmann::json::parse_error& e) {
                    logger_.error("Failed to parse request: "s + e.what(), __func__);
                    continue;
                }
                handleRequest(request, sock);
            }
        }
    }
}

void HotelManager::handleRequest(const nlohmann::json& request, net::Socket* client) {
    if (!request.contains("command") || request["command"].is_null()) {
        logger_.error("Request has no command", __func__);
        return;
    }
    std::string command = request["command"];

    // clang-format off
    static std::unordered_map<std::string, std::function<nlohmann::json(const nlohmann::json&)>> handlers = {
        {"signin", std::bind(&HotelManager::handleSignin, this, std::placeholders::_1)},
        {"signup", std::bind(&HotelManager::handleSignup, this, std::placeholders::_1)},
        {"checkUsername", std::bind(&HotelManager::handleCheckUsername, this, std::placeholders::_1)},
        {"userInfo",   std::bind(&HotelManager::handleUserInfo,   this, std::placeholders::_1)},
        {"allUsers",   std::bind(&HotelManager::handleAllUsers,   this, std::placeholders::_1)},
        {"roomsInfo",  std::bind(&HotelManager::handleRoomsInfo,  this, std::placeholders::_1)},
        {"book",       std::bind(&HotelManager::handleBook,       this, std::placeholders::_1)},
        {"cancel",     std::bind(&HotelManager::handleCancel,     this, std::placeholders::_1)},
        {"passDay",    std::bind(&HotelManager::handlePassDay,    this, std::placeholders::_1)},
        {"editInfo",   std::bind(&HotelManager::handleEditInfo,   this, std::placeholders::_1)},
        {"leaveRoom",  std::bind(&HotelManager::handleLeaveRoom,  this, std::placeholders::_1)},
        {"addRoom",    std::bind(&HotelManager::handleAddRoom,    this, std::placeholders::_1)},
        {"modifyRoom", std::bind(&HotelManager::handleModifyRoom, this, std::placeholders::_1)},
        {"removeRoom", std::bind(&HotelManager::handleRemoveRoom, this, std::placeholders::_1)},
        {"logout",     std::bind(&HotelManager::handleLogout,     this, std::placeholders::_1)},
    };
    // clang-format on

    if (handlers.find(command) == handlers.end()) {
        logger_.error("Unknown command received: " + command, __func__);
        return;
    }

    nlohmann::json response = handlers[command](request);
    response["timestamp"] = DateTime::toStr(DateTime::getServerDate());
    response["command"] = command;

    if (command == "signin" && response["status"] == StatusCode::SignedIn) {
        socketToToken_[client] = response["response"]["token"];
    }
    else if (command == "logout" && response["status"] == StatusCode::LoggedOut) {
        socketToToken_[client] = "";
    }

    std::string responseStr = response.dump();
    if (!client->send(responseStr)) {
        logger_.error("Failed to send response", __func__);
        return;
    }
    logger_.info("Responded to request", __func__,
                 response["status"], {
                                         {"message", response["message"]},
                                         {"user", response["user"]},
                                     });
}

std::string HotelManager::generateTokenForUser(int userId) {
    removeExistingToken(userId);
    std::string token;

    std::lock_guard<std::mutex> lock(tokensMutex_);
    while (true) {
        token = strutils::random(TOKEN_LENGTH);
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

void HotelManager::removeExistingToken(int userId) {
    std::lock_guard<std::mutex> lock(tokensMutex_);
    auto it = std::find_if(tokens_.begin(), tokens_.end(), [userId](const auto& token) {
        return token.second.userId == userId;
    });
    if (it != tokens_.end()) {
        tokens_.erase(it);
    }
}

void HotelManager::cleanTokens() {
    const std::chrono::minutes waitTime(1);
    while (true) {
        std::unique_lock<std::mutex> guard(tokensMutex_);
        if (tokenCleanerCancel_.wait_for(guard, waitTime, [this]() { return this->tokenCancel_; })) {
            break;
        }

        for (auto it = tokens_.begin(); it != tokens_.end();) {
            if (std::chrono::system_clock::now() - it->second.lastAccess > TOKEN_LIFETIME) {
                it = tokens_.erase(it);
            }
            else {
                ++it;
            }
        }
    }
}

void HotelManager::refreshTokenAccessTime(const std::string& token) {
    std::lock_guard<std::mutex> lock(tokensMutex_);
    auto it = tokens_.find(token);
    if (it != tokens_.end()) {
        it->second.lastAccess = std::chrono::system_clock::now();
    }
}

void HotelManager::removeToken(const std::string& token) {
    std::lock_guard<std::mutex> lock(tokensMutex_);
    tokens_.erase(token);
}

void HotelManager::commitChanges() {
    commitUsers();
    commitRooms();
}

void HotelManager::commitUsers() {
    nlohmann::json j;
    j["users"] = nlohmann::json::array();
    auto& users = j["users"];
    for (const auto& user : users_) {
        users.push_back(user.toJson());
    }
    std::ofstream usersFile(USERS_FILE);
    usersFile << std::setw(4) << j;
    logger_.info("Users committed", __func__);
}

void HotelManager::commitRooms() {
    nlohmann::json j;
    j["rooms"] = nlohmann::json::array();
    auto& rooms = j["rooms"];
    for (const auto& room : rooms_) {
        auto roomJson = room.second.toJson();
        roomJson["users"] = nlohmann::json::array();
        int capacity = getRoomCapacity(room.first);
        roomJson["capacity"] = capacity;
        roomJson["isFull"] = (capacity == 0);
        for (const auto& reservation : reservations_[room.first]) {
            roomJson["users"].push_back(reservation.toJson());
        }
        rooms.push_back(roomJson);
    }
    std::ofstream roomsFile(ROOMS_FILE);
    roomsFile << std::setw(4) << j;
    logger_.info("Rooms committed", __func__);
}

bool HotelManager::hasArgument(const nlohmann::json& request, const std::string& argument) {
    if (!request.contains("arguments") || !request["arguments"].is_object()) {
        return false;
    }
    return request["arguments"].contains(argument) && !request["arguments"][argument].is_null();
}

bool HotelManager::getRequestToken(const nlohmann::json& request, std::string& token) {
    if (!request.contains("token") || request["token"].is_null()) {
        return false;
    }
    token = request["token"];
    return true;
}

nlohmann::json HotelManager::handleSignin(const nlohmann::json& request) {
    if (!hasArgument(request, "username") || !hasArgument(request, "password")) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Not enough arguments provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    auto& args = request["arguments"];
    std::string username = args["username"];
    std::string password = args["password"];
    int userId = findUser(username);
    if (userId == -1) {
        return {
            {"status", StatusCode::WrongUserPassword},
            {"message", "Username doesn't exist"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    password = crypto::base64Decode(password);
    if (!isPasswordCorrect(userId, password)) {
        return {
            {"status", StatusCode::WrongUserPassword},
            {"message", "Wrong password"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    std::string token = generateTokenForUser(userId);
    return {
        {"status", StatusCode::SignedIn},
        {"message", "Signed in successfully"},
        {"user", std::to_string(userId)},
        {"response", {
                         {"token", token},
                     }},
    };
}

nlohmann::json HotelManager::handleSignup(const nlohmann::json& request) {
    if (!hasArgument(request, "username") ||
        !hasArgument(request, "password") ||
        !hasArgument(request, "balance") ||
        !hasArgument(request, "phone") ||
        !hasArgument(request, "address")) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Not enough arguments provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    auto& args = request["arguments"];
    std::string username = args["username"];
    std::string password = args["password"];
    std::string phone = args["phone"];
    std::string address = args["address"];
    if (!args["balance"].is_number_integer()) {
        return {
            {"status", StatusCode::BadCommand},
            {"message", "Invalid balance"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    int balance = args["balance"];
    int userId = findUser(username);
    if (userId != -1) {
        return {
            {"status", StatusCode::UsernameExists},
            {"message", "Username already exists"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    password = crypto::base64Decode(password);
    addUser(username, password, balance, phone, address);
    return {
        {"status", StatusCode::SignedUp},
        {"message", "Signed up successfully"},
        {"user", ""},
        {"response", nullptr},
    };
}

nlohmann::json HotelManager::handleCheckUsername(const nlohmann::json& request) {
    if (!hasArgument(request, "username")) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Not enough arguments provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    std::string username = request["arguments"]["username"];
    bool exists = (findUser(username) != -1);
    return {
        {"status", exists ? StatusCode::UsernameExists : StatusCode::UsernameDoesNotExist},
        {"message", exists ? "Username already exists" : "Username is available"},
        {"user", ""},
        {"response", {
                         {"checkUsername", !exists},
                     }},
    };
}

nlohmann::json HotelManager::handleUserInfo(const nlohmann::json& request) {
    std::string token;
    if (!getRequestToken(request, token)) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Token not provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    int userId = getUser(token);
    if (userId == -1) {
        return {
            {"status", StatusCode::Unauthorized},
            {"message", "Invalid token"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    refreshTokenAccessTime(token);
    return {
        {"status", StatusCode::OK},
        {"message", "User info"},
        {"user", std::to_string(userId)},
        {"response", getUserInfo(userId)},
    };
}

nlohmann::json HotelManager::handleAllUsers(const nlohmann::json& request) {
    std::string token;
    if (!getRequestToken(request, token)) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Token not provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    int userId = getUser(token);
    if (userId == -1) {
        return {
            {"status", StatusCode::Unauthorized},
            {"message", "Invalid token"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    refreshTokenAccessTime(token);
    if (!isAdministrator(userId)) {
        return {
            {"status", StatusCode::AccessDenied},
            {"message", "Access denied"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    return {
        {"status", StatusCode::OK},
        {"message", "All users"},
        {"user", std::to_string(userId)},
        {"response", getAllUsers()},
    };
}

nlohmann::json HotelManager::handleRoomsInfo(const nlohmann::json& request) {
    std::string token;
    if (!getRequestToken(request, token)) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Token not provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    int userId = getUser(token);
    if (userId == -1) {
        return {
            {"status", StatusCode::Unauthorized},
            {"message", "Invalid token"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    refreshTokenAccessTime(token);
    bool onlyAvailable = false;
    if (hasArgument(request, "onlyAvailable")) {
        auto& onlyAv = request["arguments"]["onlyAvailable"];
        if (onlyAv.is_boolean()) {
            onlyAvailable = onlyAv;
        }
    }
    bool showReservations = isAdministrator(userId);
    return {
        {"status", StatusCode::OK},
        {"message", "Rooms info"},
        {"user", std::to_string(userId)},
        {"response", getRoomsInfo(onlyAvailable, showReservations)},
    };
}

nlohmann::json HotelManager::handleBook(const nlohmann::json& request) {
    std::string token;
    if (!getRequestToken(request, token)) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Token not provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    int userId = getUser(token);
    if (userId == -1) {
        return {
            {"status", StatusCode::Unauthorized},
            {"message", "Invalid token"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    refreshTokenAccessTime(token);
    if (!hasArgument(request, "roomNum") ||
        !hasArgument(request, "numOfBeds") ||
        !hasArgument(request, "checkInDate") ||
        !hasArgument(request, "checkOutDate")) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Not enough arguments provided"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    auto& args = request["arguments"];
    std::string roomNum = args["roomNum"];
    std::string checkIn = args["checkInDate"];
    std::string checkOut = args["checkOutDate"];
    if (!args["numOfBeds"].is_number_integer()) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Invalid number of beds"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    int numOfBeds = args["numOfBeds"];
    date::year_month_day checkInDate, checkOutDate;
    if (!DateTime::parse(checkIn, checkInDate) || !DateTime::parse(checkOut, checkOutDate)) {
        return {
            {"status", StatusCode::BadCommand},
            {"message", "Invalid date"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    if (checkInDate >= checkOutDate) {
        return {
            {"status", StatusCode::BadCommand},
            {"message", "Invalid date range"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    if (!doesRoomExist(roomNum)) {
        return {
            {"status", StatusCode::RoomNotFound},
            {"message", "Room does not exist"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    if (!isRoomAvailable(roomNum, numOfBeds, checkInDate, checkOutDate)) {
        return {
            {"status", StatusCode::RoomCapacityFull},
            {"message", "Room is full"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    if (!hasEnoughBalance(userId, roomNum, numOfBeds)) {
        return {
            {"status", StatusCode::BalanceNotEnough},
            {"message", "Not enough balance"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    bookRoom(userId, roomNum, numOfBeds, checkInDate, checkOutDate);
    return {
        {"status", StatusCode::OK},
        {"message", "Room booked"},
        {"user", std::to_string(userId)},
        {"response", nullptr},
    };
}

nlohmann::json HotelManager::handleCancel(const nlohmann::json& request) {
    std::string token;
    if (!getRequestToken(request, token)) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Token not provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    int userId = getUser(token);
    if (userId == -1) {
        return {
            {"status", StatusCode::Unauthorized},
            {"message", "Invalid token"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    refreshTokenAccessTime(token);
    if (!hasArgument(request, "roomNum") || !hasArgument(request, "numOfBeds")) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Not enough arguments provided"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    auto& args = request["arguments"];
    std::string roomNum = args["roomNum"];
    if (!args["numOfBeds"].is_number_integer()) {
        return {
            {"status", StatusCode::InvalidValue},
            {"message", "Invalid number of beds"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    int numOfBeds = args["numOfBeds"];
    if (!doesRoomExist(roomNum)) {
        return {
            {"status", StatusCode::RoomNotFound},
            {"message", "Room does not exist"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    if (!hasReservation(userId, roomNum, numOfBeds)) {
        return {
            {"status", StatusCode::ReservationNotFound},
            {"message", "Reservation does not exist"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    cancelReservation(userId, roomNum, numOfBeds);
    return {
        {"status", StatusCode::CancelOK},
        {"message", "Reservation cancelled"},
        {"user", std::to_string(userId)},
        {"response", nullptr},
    };
}

nlohmann::json HotelManager::handlePassDay(const nlohmann::json& request) {
    std::string token;
    if (!getRequestToken(request, token)) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Token not provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    int userId = getUser(token);
    if (userId == -1) {
        return {
            {"status", StatusCode::Unauthorized},
            {"message", "Invalid token"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    refreshTokenAccessTime(token);
    if (!hasArgument(request, "days")) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Not enough arguments provided"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    if (!isAdministrator(userId)) {
        return {
            {"status", StatusCode::AccessDenied},
            {"message", "Access denied"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    if (!request["arguments"]["days"].is_number_integer()) {
        return {
            {"status", StatusCode::InvalidValue},
            {"message", "Invalid days"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    int days = request["arguments"]["days"];
    DateTime::increaseServerDate(days);
    checkOutExpiredReservations();
    return {
        {"status", StatusCode::OK},
        {"message", "Passed days successfully"},
        {"user", std::to_string(userId)},
        {"response", nullptr},
    };
}

nlohmann::json HotelManager::handleEditInfo(const nlohmann::json& request) {
    std::string token;
    if (!getRequestToken(request, token)) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Token not provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    int userId = getUser(token);
    if (userId == -1) {
        return {
            {"status", StatusCode::Unauthorized},
            {"message", "Invalid token"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    refreshTokenAccessTime(token);
    if (!hasArgument(request, "password") ||
        !hasArgument(request, "phone") ||
        !hasArgument(request, "address")) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Not enough arguments provided"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    auto& args = request["arguments"];
    std::string password = args["password"];
    std::string phone = args["phone"];
    std::string address = args["address"];
    password = crypto::base64Decode(password);
    editUser(userId, password, phone, address);
    return {
        {"status", StatusCode::UserInfoChanged},
        {"message", "User info edited successfully"},
        {"user", std::to_string(userId)},
        {"response", nullptr},
    };
}

nlohmann::json HotelManager::handleLeaveRoom(const nlohmann::json& request) {
    std::string token;
    if (!getRequestToken(request, token)) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Token not provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    int userId = getUser(token);
    if (userId == -1) {
        return {
            {"status", StatusCode::Unauthorized},
            {"message", "Invalid token"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    refreshTokenAccessTime(token);
    if (!hasArgument(request, "roomNum")) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Not enough arguments provided"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    std::string roomNum = request["arguments"]["roomNum"];
    if (!doesRoomExist(roomNum)) {
        return {
            {"status", StatusCode::BadCommand},
            {"message", "Room not found"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    if (!isResidence(userId, roomNum)) {
        return {
            {"status", StatusCode::ReservationNotFound},
            {"message", "User is not in this room"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    leaveRoom(userId, roomNum);
    return {
        {"status", StatusCode::UserLeftRoom},
        {"message", "Left room successfully"},
        {"user", std::to_string(userId)},
        {"response", nullptr},
    };
}

nlohmann::json HotelManager::handleAddRoom(const nlohmann::json& request) {
    std::string token;
    if (!getRequestToken(request, token)) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Token not provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    int userId = getUser(token);
    if (userId == -1) {
        return {
            {"status", StatusCode::Unauthorized},
            {"message", "Invalid token"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    refreshTokenAccessTime(token);
    if (!isAdministrator(userId)) {
        return {
            {"status", StatusCode::AccessDenied},
            {"message", "Access denied"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    if (!hasArgument(request, "roomNum") ||
        !hasArgument(request, "maxCapacity") ||
        !hasArgument(request, "price")) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Not enough arguments provided"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    auto& args = request["arguments"];
    std::string roomNum = args["roomNum"];
    if (!args["maxCapacity"].is_number_integer() || !args["price"].is_number_integer()) {
        return {
            {"status", StatusCode::InvalidValue},
            {"message", "Invalid arguments"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    int maxCapacity = args["maxCapacity"];
    int price = args["price"];
    if (doesRoomExist(roomNum)) {
        return {
            {"status", StatusCode::RoomExists},
            {"message", "Room already exists"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    addRoom(roomNum, maxCapacity, price);
    return {
        {"status", StatusCode::RoomAdded},
        {"message", "Room added successfully"},
        {"user", std::to_string(userId)},
        {"response", nullptr},
    };
}

nlohmann::json HotelManager::handleModifyRoom(const nlohmann::json& request) {
    std::string token;
    if (!getRequestToken(request, token)) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Token not provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    int userId = getUser(token);
    if (userId == -1) {
        return {
            {"status", StatusCode::Unauthorized},
            {"message", "Invalid token"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    refreshTokenAccessTime(token);
    if (!isAdministrator(userId)) {
        return {
            {"status", StatusCode::AccessDenied},
            {"message", "Access denied"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    if (!hasArgument(request, "roomNum") ||
        !hasArgument(request, "maxCapacity") ||
        !hasArgument(request, "price")) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Not enough arguments provided"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    auto& args = request["arguments"];
    std::string roomNum = args["roomNum"];
    if (!args["maxCapacity"].is_number_integer() || !args["price"].is_number_integer()) {
        return {
            {"status", StatusCode::InvalidValue},
            {"message", "Invalid arguments"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    int maxCapacity = args["maxCapacity"];
    int price = args["price"];
    if (!doesRoomExist(roomNum)) {
        return {
            {"status", StatusCode::RoomNotFound},
            {"message", "Room not found"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    if (!canModifyRoom(roomNum, maxCapacity)) {
        return {
            {"status", StatusCode::RoomCapacityFull},
            {"message", "Room is full"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    modifyRoom(roomNum, maxCapacity, price);
    return {
        {"status", StatusCode::RoomModified},
        {"message", "Room modified successfully"},
        {"user", std::to_string(userId)},
        {"response", nullptr},
    };
}

nlohmann::json HotelManager::handleRemoveRoom(const nlohmann::json& request) {
    std::string token;
    if (!getRequestToken(request, token)) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Token not provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    int userId = getUser(token);
    if (userId == -1) {
        return {
            {"status", StatusCode::Unauthorized},
            {"message", "Invalid token"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    refreshTokenAccessTime(token);
    if (!isAdministrator(userId)) {
        return {
            {"status", StatusCode::AccessDenied},
            {"message", "Access denied"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    if (!hasArgument(request, "roomNum")) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Not enough arguments provided"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    std::string roomNum = request["arguments"]["roomNum"];
    if (!doesRoomExist(roomNum)) {
        return {
            {"status", StatusCode::RoomNotFound},
            {"message", "Room not found"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    if (!canRemoveRoom(roomNum)) {
        return {
            {"status", StatusCode::RoomCapacityFull},
            {"message", "Room is not empty"},
            {"user", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    removeRoom(roomNum);
    return {
        {"status", StatusCode::RoomDeleted},
        {"message", "Room removed successfully"},
        {"user", std::to_string(userId)},
        {"response", nullptr},
    };
}

nlohmann::json HotelManager::handleLogout(const nlohmann::json& request) {
    std::string token;
    if (!getRequestToken(request, token)) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Token not provided"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    int userId = getUser(token);
    if (userId == -1) {
        return {
            {"status", StatusCode::Unauthorized},
            {"message", "Invalid token"},
            {"user", ""},
            {"response", nullptr},
        };
    }
    logoutUser(token);
    return {
        {"status", StatusCode::LoggedOut},
        {"message", "Logged out successfully"},
        {"user", std::to_string(userId)},
        {"response", nullptr},
    };
}

nlohmann::json HotelManager::getUserInfo(int userId) const {
    return users_[userId].toJson(false);
}

nlohmann::json HotelManager::getAllUsers() const {
    nlohmann::json response = nlohmann::json::array();
    for (const auto& user : users_) {
        response.push_back(user.toJson(false));
    }
    return response;
}

nlohmann::json HotelManager::getRoomsInfo(bool onlyAvailable, bool showReservations) const {
    nlohmann::json response = nlohmann::json::array();
    for (const auto& room : rooms_) {
        if (onlyAvailable && getRoomCapacity(room.first) == 0) {
            continue;
        }
        auto roomJson = room.second.toJson();
        if (showReservations) {
            roomJson["reservations"] = nlohmann::json::array();
            for (const auto& reservation : reservations_.at(room.first)) {
                roomJson["reservations"].push_back(reservation.toJson());
            }
        }
        response.push_back(roomJson);
    }
    return response;
}

bool HotelManager::isAdministrator(int userId) const {
    return users_[userId].getRole() == User::Role::Admin;
}

bool HotelManager::isPasswordCorrect(int userId, const std::string& password) const {
    return users_[userId].isPasswordCorrect(crypto::SHA256(password));
}

bool HotelManager::isResidence(int userId, const std::string& roomNum) const {
    auto serverDate = DateTime::getServerDate();
    for (const auto& reservation : reservations_.at(roomNum)) {
        if (reservation.getUserId() == userId && reservation.hasConflict(serverDate)) {
            return true;
        }
    }
    return false;
}

bool HotelManager::hasReservation(int userId, const std::string& roomNum, int numOfBeds) const {
    for (const auto& reservation : reservations_.at(roomNum)) {
        if (reservation.getUserId() == userId && reservation.getNumOfBeds() >= numOfBeds) {
            return true;
        }
    }
    return false;
}

bool HotelManager::doesRoomExist(const std::string& roomNum) const {
    return rooms_.find(roomNum) != rooms_.end();
}

bool HotelManager::canModifyRoom(const std::string& roomNum, int maxCapacity) const {
    if (maxCapacity >= rooms_.at(roomNum).getMaxCapacity()) {
        return true;
    }
    auto lastDate = DateTime::getServerDate();
    for (const auto& reservation : reservations_.at(roomNum)) {
        lastDate = std::max(lastDate, reservation.getCheckOut());
    }
    return maxCapacity >= findMaximumUsers(roomNum, DateTime::getServerDate(), lastDate);
}

bool HotelManager::canRemoveRoom(const std::string& roomNum) const {
    return reservations_.at(roomNum).empty();
}

bool HotelManager::isRoomAvailable(const std::string& roomNum, int numOfBed, date::year_month_day checkIn, date::year_month_day checkOut) const {
    return rooms_.at(roomNum).getMaxCapacity() - findMaximumUsers(roomNum, checkIn, checkOut) >= numOfBed;
}

bool HotelManager::hasEnoughBalance(int userId, const std::string& roomNum, int numOfBeds) const {
    int balanceNeeded = rooms_.at(roomNum).getPrice() * numOfBeds;
    return users_.at(userId).getBalance() >= balanceNeeded;
}

int HotelManager::findMaximumUsers(const std::string& roomNum, date::year_month_day start, date::year_month_day end) const {
    int maxConflicts = 0;
    for (date::sys_days day = start; day < end; day += date::days(1)) {
        int conflicts = 0;
        for (const auto& reservation : reservations_.at(roomNum)) {
            if (reservation.hasConflict(day)) {
                conflicts += reservation.getNumOfBeds();
            }
        }
        maxConflicts = std::max(maxConflicts, conflicts);
    }
    return maxConflicts;
}

int HotelManager::getUser(const std::string& token) {
    std::lock_guard<std::mutex> lock(tokensMutex_);
    auto it = tokens_.find(token);
    if (it == tokens_.end()) {
        return -1;
    }
    return it->second.userId;
}

int HotelManager::findUser(const std::string& username) const {
    auto it = std::find_if(users_.begin(), users_.end(), [username](const auto& user) {
        return user.getUsername() == username;
    });
    if (it == users_.end()) {
        return -1;
    }
    return it->getId();
}

int HotelManager::getRoomCapacity(const std::string& roomNum) const {
    auto serverDate = DateTime::getServerDate();
    int capacity = rooms_.at(roomNum).getMaxCapacity();
    for (const auto& reservation : reservations_.at(roomNum)) {
        if (reservation.hasConflict(serverDate)) {
            capacity -= reservation.getNumOfBeds();
        }
    }
    return capacity;
}

void HotelManager::addUser(const std::string& username, const std::string& password, int balance, const std::string& address, const std::string& phone) {
    users_.emplace_back(users_.size(), username, password, User::Role::User, balance, phone, address);
    commitChanges();
}

void HotelManager::logoutUser(const std::string& token) {
    removeToken(token);
}

void HotelManager::editUser(int userId, const std::string& password, const std::string& address, const std::string& phone) {
    users_[userId].editInfo(password, phone, address);
    commitChanges();
}

void HotelManager::checkOutExpiredReservations() {
    auto serverDate = DateTime::getServerDate();
    for (auto& room : reservations_) {
        for (auto& reservation : std::vector<Reservation>(room.second)) {
            if (reservation.isExpired(serverDate)) {
                room.second.erase(std::remove(room.second.begin(), room.second.end(), reservation), room.second.end());
            }
        }
    }
    commitChanges();
}

void HotelManager::leaveRoom(int userId, const std::string& roomNum) {
    auto serverDate = DateTime::getServerDate();
    for (auto& reservation : std::vector<Reservation>(reservations_[roomNum])) {
        if (reservation.getUserId() == userId && reservation.hasConflict(serverDate)) {
            reservations_[roomNum].erase(std::remove(reservations_[roomNum].begin(), reservations_[roomNum].end(), reservation), reservations_[roomNum].end());
        }
    }
    commitChanges();
}

void HotelManager::addRoom(const std::string& roomNum, int maxCapacity, int price) {
    rooms_.emplace(roomNum, Room(roomNum, price, maxCapacity));
    reservations_.emplace(roomNum, std::vector<Reservation>());
    commitChanges();
}

void HotelManager::modifyRoom(const std::string& roomNum, int maxCapacity, int price) {
    rooms_[roomNum].modify(price, maxCapacity);
    commitChanges();
}

void HotelManager::removeRoom(const std::string& roomNum) {
    rooms_.erase(roomNum);
    reservations_.erase(roomNum);
    commitChanges();
}

void HotelManager::cancelReservation(int userId, const std::string& roomNum, int numOfBeds) {
    auto serverDate = DateTime::getServerDate();
    for (auto& reservation : std::vector<Reservation>(reservations_[roomNum])) {
        if (reservation.getUserId() == userId && reservation.getNumOfBeds() >= numOfBeds && reservation.canBeCancelled(serverDate)) {
            if (reservation.getNumOfBeds() > numOfBeds) {
                reservation.modify(reservation.getNumOfBeds() - numOfBeds);
            }
            else {
                reservations_[roomNum].erase(std::remove(reservations_[roomNum].begin(), reservations_[roomNum].end(), reservation), reservations_[roomNum].end());
            }
            users_[userId].increaseBalance((numOfBeds * rooms_[roomNum].getPrice()) / 2);
        }
    }
    commitChanges();
}

void HotelManager::bookRoom(int userId, const std::string& roomNum, int numOfBeds, date::year_month_day checkIn, date::year_month_day checkOut) {
    reservations_[roomNum].emplace_back(userId, numOfBeds, checkIn, checkOut);
    users_[userId].decreaseBalance(numOfBeds * rooms_[roomNum].getPrice());
    commitChanges();
}
