# CN_CHomeworks_1

- [CN\_CHomeworks\_1](#cn_chomeworks_1)
  - [Introduction](#introduction)
  - [Libraries](#libraries)
  - [Project Structure](#project-structure)
    - [Logger](#logger)
    - [Socket Wrapper](#socket-wrapper)
      - [endian](#endian)
      - [ntoh/hton](#ntohhton)
      - [IpAddr](#ipaddr)
      - [Socket](#socket)
      - [Select](#select)
    - [DateTime](#datetime)
    - [Crypto](#crypto)
    - [JSON](#json)
    - [Client](#client)
      - [CLI](#cli)
      - [Requests](#requests)
    - [Server](#server)
      - [Authentication](#authentication)
      - [User](#user)
      - [Room](#room)
      - [Reservations](#reservations)
      - [Responses](#responses)

## Introduction

This is a project for the Computer Networks course at the University of Tehran, Electrical and Computer Engineering department in Spring 2023.  
The project focuses on *Socket Programming* and is written in C++.  
The main goal of the project is to create a server for a hotel management system and a client for the users to interact with the server. We have chosen the name *Misasha Hotel* as it is the combination of [Misagh](https://github.com/MisaghM) and [Pasha](https://github.com/PashaBarahimi) which are the two members of the team.  
The project structure consists of several parts, each of which is explained in the following sections.

## Libraries

- [Crypto++](https://www.cryptopp.com/)  
  
  To install Crypto++ on Ubuntu, you can run the following command:  

  ```bash
  sudo apt install libcrypto++8 libcrypto++-dev
  ```

  You can also build the library from the [source](https://github.com/weidai11/cryptopp), put the *crypto++* folder in `lib`, and add the following line to the Makefile:

  ```makefile
  LDFLAGS += -L $(PATH_LIB)/crypto++
  ```

The other libraries used in the project are included in the project files (in the `lib` folder) and are as follows:

> - [nlohmann/json](https://github.com/nlohmann/json)
> - [HowardHinnant/date](https://github.com/HowardHinnant/date)
> - [daniele77/cli](https://github.com/daniele77/cli)

## Project Structure

### Logger

The logger class is used to log messages to the console or to a file.  
The following API is provided for the logger class:

```cpp
class Logger {
public:
    enum class Level {
        Info,
        Warning,
        Error
    };

    Logger(Level level);
    Logger(Level level, std::ofstream& file);

    void info(const std::string& message,
              const std::string& action = "",
              int messageCode = -1,
              const std::unordered_map<std::string, std::string>& details = {});

    void warn(const std::string& message,
              const std::string& action = "",
              int messageCode = -1,
              const std::unordered_map<std::string, std::string>& details = {});

    void error(const std::string& message,
               const std::string& action = "",
               int messageCode = -1,
               const std::unordered_map<std::string, std::string>& details = {});
};
```

The logger is able to differentiate between files and the console. So even if `std::cout` is used to log to the console, but the output is redirected to a file, the logger will be able to detect that it is ultimately a file (using the `isatty()` function). This is useful because the logger outputs JSON to a file, and a human-readable and colored format to the console.  
The logger is able to log the following information:

- **Level**: The level of the log message. It can be one of the following: `Info`, `Warning`, `Error`.
- **Message**: The message that is logged.
- **Action**: The action that is performed when the message is logged (e.g. the function that it happened in).
- **Message Code**: A code that is used to identify the message.
- **Details**: A map of key-value pairs that can be used to provide additional information about the message.

Furthermore, the logger always includes the following information in the log messages:

- **Timestamp**: The timestamp of the log message.
- **Server Date**: The date of the server.

> It is important to note that the logger will log the above information only when the logger is used to log to a file.  
> When the logger is used to log to the console, only the following information will be logged so that it stays compact:
>
> - **Level**
> - **Message**
> - **Message Code**

The following is an example of a log message that could be logged to a file:

```json
{
    "action": "setupServer",
    "level": "info",
    "message": "Starting server",
    "serverDate": "2023-03-14",
    "timestamp": "2023-03-14 22:11:06"
}
{
    "action": "setupServer",
    "level": "warn",
    "message": "Server is running in debug mode",
    "serverDate": "2023-03-14",
    "timestamp": "2023-03-14 22:11:06"
}
{
    "action": "setupServer",
    "level": "error",
    "message": "Server failed to start",
    "messageCode": 500,
    "serverDate": "2023-03-14",
    "timestamp": "2023-03-14 22:11:06"
}
```

The following is an example of a log message that is logged to the console:

![Logger](./assets/log-console.png)

Logging to a file in JSON format is useful because we can use third-party tools such as [jq](https://stedolan.github.io/jq/) to parse the log file and extract the information that we need. For example, the following command can be used to extract all the log messages that have the level `Error`:

```bash
# add comma after each closing curly bracket
sed -i 's/}/},/g' log.json
# remove the last comma
sed -i '$ s/.$//' log.json
# add square brackets to the beginning and the end of the file
sed -i '1s/^/[/' log.json
sed -i '$s/$/]/' log.json

# extract all the log messages that have the level "Error"
jq '.[] | select(.level == "error")' log.json
```

![jq](./assets/jq-analyze-logs.png)

### Socket Wrapper

The socket wrapper consists of `net.hpp/cpp` and the helper endian header.

#### endian

```cpp
namespace byte {

enum class Endian {
    little,
    big
};

inline const Endian endian;

template <class T>
inline T swapOrder(T val);

} // namespace byte
```

There is an `Endian` enum and the `endian` variable which says which endianness the architecture runs on.  
The `swapOrder` function performs a byte swap for conversion between endiannesses.

The `net` namespace contains 4 main parts:

- `ntoh/hton`: These templated functions use the `swapOrder` function to convert the endianness from host to network and vice versa.
- `IpAddr`: This class stores an IPv4 address and has methods to convert to and from strings and ints.
- `Socket`: This class is a wrapper for the POSIX socket API.
- `Select`: This class is a wrapper for the `select` system call.

The mentioned parts will now be explained in more detail:

#### ntoh/hton

```cpp
template <class T>
inline T hton(T in) {
    if (byte::endian == byte::Endian::big) {
        return in;
    }
    else {
        return byte::swapOrder(in);
    }
}
```

The code is self-explanatory. If the running architecture is in big endian, there is no need for a conversion.

#### IpAddr

```cpp
class IpAddr {
public:
    IpAddr() = default;
    IpAddr(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d);
    IpAddr(const std::string& addr);
    IpAddr(const char* addr);
    explicit IpAddr(std::uint32_t addr);

    std::uint8_t& operator[](int i);
    std::uint8_t operator[](int i) const;

    bool operator==(IpAddr rhs) const;
    bool operator!=(IpAddr lhs) const;

    std::string toStr() const;
    std::uint32_t toInt() const;
    bool fromStr(const std::string& addr);
    void fromInt(std::uint32_t addr);

    static IpAddr any() { return IpAddr(0, 0, 0, 0); }
    static IpAddr loopback() { return IpAddr(127, 0, 0, 1); }
};
```

The IPv4 address is stored as an array of 4 unsigned 8-bit integers.  
There are different ways of construction, by means of passing the 4 values, converting from a string, and even using the full integer value.  
Indexing operators access the 4 parts of the IP, and `toStr` and `toInt` convert it to the desired type.  
The methods `fromStr` and `fromInt` change the current instance and can report errors.  
Two helper static methods `any()` and `loopback()` (localhost) return an instance of `IpAddr` with the standard IPs.

This class is used as input for the POSIX wrappers.

#### Socket

```cpp
class Socket {
public:
    enum class Type {
        unused = -1,
        stream = SOCK_STREAM,
        datagram = SOCK_DGRAM
    };

    enum class Status {
        unused,
        bound,
        listening,
        connected
    };

    Socket() = default;
    explicit Socket(Type type);
    Socket(const Socket& other) = delete;
    Socket(Socket&& other);
    Socket& operator=(const Socket& other) = delete;
    Socket& operator=(Socket&& other);
    ~Socket();

    bool bind(IpAddr addr, Port port);
    bool listen(int queueSize);
    bool connect(IpAddr addr, Port port);
    bool accept(Socket& outSocket);

    bool send(const std::string& data);
    bool receive(std::string& data);

    bool operator==(const Socket& rhs) const;
    bool operator!=(const Socket& rhs) const;
};
```

This class is the main part of the wrapper facilities.  
A socket is constructed by giving the desired socket `Type`, and its status is stored.

At the server side, a socket needs to be bound to an ip:port, and listened on. And when a connection comes, it should be able to accept it.  
After accepting, a new socket is created which can be used to send or receive data to and from both sides.  
This is easily accomplished using the wrapper:

```cpp
net::Socket server(net::Socket::Type::stream);
server.bind("1.2.3.4", 8282);
server.listen(4);

net::Socket client;
server.accept(client);

client.send("data");
std::string data;
client.receive(data);
```

Error checking can be done using the boolean return value of the methods.  
For example, if `bind` returns false, the *ip:port* may be already in use.

At the client side, a socket should connect to a listening server, and then send and receive data.  
This can be done using the wrapper like so:

```cpp
net::Socket s(net::Socket::Type::stream);
s.connect("1.2.3.4", 8282);
s.send("test");
```

As we can see, the wrapper abstracts away the complications of using the socket API directly.

#### Select

```cpp
class Select {
public:
    Select();
    ~Select();

    void addRead(Socket* socket, bool own = true);
    void addWrite(Socket* socket, bool own = true);
    void addExcept(Socket* socket, bool own = true);

    void removeRead(Socket* socket);
    void removeWrite(Socket* socket);
    void removeExcept(Socket* socket);

    bool isInRead(const Socket* socket) const;
    bool isInWrite(const Socket* socket) const;
    bool isInExcept(const Socket* socket) const;

    int select(int timeoutMs = -1);

    bool isReadyRead(const Socket* socket) const;
    bool isReadyWrite(const Socket* socket) const;
    bool isReadyExcept(const Socket* socket) const;

    std::vector<Socket*> getReadyRead() const;
    std::vector<Socket*> getReadyWrite() const;
    std::vector<Socket*> getReadyExcept() const;
};
```

The select system call gets a list of the file descriptors we want to listen on for read, write, or special conditions. When selecting, the program is blocked until one of the file descriptors is ready for the requested operation.  
This wrapper provides a simple interface to use the select system call.  
The sockets can be added or removed, and can be checked if they are ready for the requested operation.  
After a select call, the ready sockets can be retrieved using the `getReady` methods.

Using the wrappers, a simple server can be implemented as follows:

```cpp
int main() {
    net::Socket server(net::Socket::Type::stream);
    server.bind(net::IpAddr::any(), 8282);
    server.listen(4);

    net::Select sel;
    sel.addRead(&server, false);

    while (true) {
        sel.select();
        auto sockets = sel.getReadyRead();

        for (auto s : sockets) {
            if (server == *s) {
                auto newClient = new net::Socket();
                if (!server.accept(*newClient)) {
                    continue;
                }
                sel.addRead(newClient, true);
            }
            else {
                std::string out;
                if (s->receive(out)) {
                    if (out.empty()) { // client closed
                        sel.removeRead(s);
                    }
                }
                std::cout << out << '\n';
            }
        }
    }
    return 0;
}
```

### DateTime

DateTime is a static class that is used to get the current date and time.  
The following API is provided for the DateTime class:

```cpp
class DateTime {
public:
    static date::year_month_day getDate();
    static date::year_month_day getServerDate();
    static std::chrono::system_clock::time_point getDateTime();

    static std::string toStr(date::year_month_day date);
    static std::string toStr(std::chrono::system_clock::time_point dateTime);

    static void setServerDate(date::year_month_day date);
    static void increaseServerDate(int days = 1);

    static bool isValid(const std::string& date);
    static bool parse(const std::string& date, date::year_month_day& res);

    static int compare(const std::string& lhs, const std::string& rhs);
};
```

This class uses the [HowardHinnant/date](https://github.com/HowardHinnant/date) library (which was added to the standard library in C++20) to store the server date in a `date::year_month_day` struct.  
It is important to note that all of the functions provided by the DateTime class require the date to be in the format `YYYY-MM-DD` which is the ISO 8601 format. This is differs from the project requirements which requires the date to be in the format `DD-MM-YYYY`.

The class can get the real current date or date-time, as well as the server special date.  
The `toStr` methods allow the conversion from either a date or a date-time to a string. The `parse` method is the opposite and converts from a string to a date.

### Crypto

The `crypto` namespace contains the following functions which are wrappers for the functions provided by the [Crypto++](https://www.cryptopp.com/) library:

```cpp
namespace crypto {

std::string SHA256(const std::string& input);
std::string base64Encode(const std::string& input);
std::string base64Decode(const std::string& encoded);

} // namespace crypto
```

The use cases for these functions are:

- **SHA256**: Used to hash the password of the user. The hashed password is stored in the JSON file instead of the plain text password to prevent the password from being stolen.
- **base64Encode**: Used to encode the password of the user before sending it to the server.
- **base64Decode**: Used to decode the password of the user after receiving it in the server.

### JSON

The project requires the use of JSON files in the following cases:

- **Users**: The users information is stored in a JSON file.
- **Rooms**: The rooms information is stored in a JSON file.
- **Config**: The configuration of the server is stored in a JSON file.
- **Logs**: The logs are stored in a JSON file.
- **Request**: The requests sent by the client are in JSON format.
- **Response**: The responses sent by the server are in JSON format.

All of the JSON data and files are formatted and parsed using the [nlohmann/json](https://github.com/nlohmann/json) library.

### Client

The client uses a command-line interface to connect to the hotel server.

#### CLI

The [daniele77/cli](https://github.com/daniele77/cli) library is used to handle the command-line interface.  
Because the library used a thread to read the input, `std::cin` could not be directly used inside of a command handler. The project required some of the commands (such as `signup`) to first get the username, and then the rest of the information.  
The library code was slightly modified so that a custom concurrent queue can be passed to the handler. (*lib/cli/detail/concurrentqueue.h*) This makes the input reading thread post the characters to the queue instead of the library's scheduler. The queue is used for getting input inside of command handlers.  
The menus are created in the `ClientCLI` class. An example of a command is shown below:

```cpp
loginMenu_.push_back(mainMenu->Insert(
    "signup", [this](std::ostream& out, const std::string& username) {
        bool userExists = client_.doesUserExist(username);
        if (userExists) {
            out << "Username already exists." << std::endl;
            return;
        }
        else {
            out << "Username is available." << std::endl;
        }
        std::string password, phone, address;
        int balance;
        if (!inputPassword(out, password)) {
            return;
        }
        if (!getIntInput(out, "Enter your balance: ", balance)) {
            return;
        }
        phone = getInput(out, "Enter your phone number: ");
        address = getInput(out, "Enter your address: ");
        out << client_.signup(username, password, balance, phone, address) << std::endl;
        checkMainMenuItems();
    },
    "Signup (usage: signup <username>)"));
```

This creates the `signup` command which takes a username as an argument. The command will check if the username is available and then ask the user to enter the password, balance, phone number and address. The command will then send a request to the server to create a new user with the given information. The response message is printed and the menu is checked if the use has been signed out due to inactivity.

#### Requests

In the `HotelClient` class, we connect to the server and can interact by sending requests.

```cpp
class HotelClient {
public:
    HotelClient(net::IpAddr host, net::Port port);
    bool connect();

    bool doesUserExist(const std::string& username);
    std::string signin(const std::string& username, const std::string& password);
    std::string signup(const std::string& username, const std::string& password, int balance, const std::string& phone, const std::string& address);
    std::string userInfo();
    std::string allUsers();
    std::string roomsInfo(bool onlyAvailable = false);
    std::string book(const std::string& roomNum, int numOfBeds, const std::string& checkInDate, const std::string& checkOutDate);
    std::string cancel(const std::string& roomNum, int numOfBeds);
    std::string passDay(int numOfDays);
    std::string editInfo(const std::string& password, const std::string& phone, const std::string& address);
    std::string leaveRoom(const std::string& roomNum);
    std::string addRoom(const std::string& roomNum, int maxCapacity, int price);
    std::string modifyRoom(const std::string& roomNum, int newMaxCapacity, int newPrice);
    std::string removeRoom(const std::string& roomNum);
    std::string logout();

    bool isLoggedIn() const;
};
```

Each of the CLI commands have a function here which sends the actual request and returns the response message.  
An example method:

```cpp
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
```

The requests share a common format:

```json
{
    "command": "commandName",
    "arguments": {
        "arg1": "value1",
    },
    "token": "token"
}
```

The server reads the command, uses the token to authorize the user, and uses the optional arguments to generate a response.

### Server

The server reads the configuration from a JSON file and listens for client connections and requests.

#### Authentication

When a user signs up, the server will hash the password using the `SHA256` algorithm and store the hashed password in the JSON file. This process is done to prevent the misuse of the password in case the JSON file is accessed by an unauthorized person.  
When a user signs in, the server will hash the password using the `SHA256` algorithm and compare it with the hashed password that is stored in the JSON file. If the two passwords match, the user will be authenticated and the server will generate a token for the user. This token will be used to authenticate the user in the future requests. The token length is 32 characters and can contain any character from the following set: `[a-zA-Z0-9]-_` (URL base64 characters).

The token should be stored in the client side and sent in the `token` field of the request. Otherwise, the server will consider the request as an unauthorized request and will return an error response if the request requires authentication.  
The token will expire after 30 minutes. However, the expiration time is reset every time the user sends a request to the server. This means that the user will have to sign in again only if the user did not have any activity for 30 minutes.

#### User

The user class simply stores a user from the JSON file (*userinfo.json*).  
There methods for editing info, checking the password, converting to JSON, and getters.

```cpp
class User {
public:
    enum class Role {
        Admin,
        User
    };

    User(int id, std::string username, std::string password, Role role,
         int balance = 0, std::string phone = "", std::string address = "");

    void editInfo(const std::string& newPassword, const std::string& newPhone, const std::string& newAddress);
    void increaseBalance(int amount);
    void decreaseBalance(int amount);

    bool isPasswordCorrect(const std::string& hashedPassword) const;

    Role getRole() const;
    int getId() const;
    std::string getUsername() const;
    int getBalance() const;

    nlohmann::json toJson(bool includePassword = true) const;
};
```

#### Room

A room is also simply a holder of the JSON data (*roomsinfo.json*).

```cpp
class Room {
public:
    Room() = default;
    Room(std::string num, int price, int maxCapacity);

    void modify(int newPrice, int newMaxCapacity);

    std::string getNumber() const;
    int getPrice() const;
    int getMaxCapacity() const;

    nlohmann::json toJson() const;
};
```

#### Reservations

Reservations are read from *roomsinfo.json*.  
There are methods to check if it is expired, has a conflict with another date, or can be cancelled.

```cpp
class Reservation {
public:
    Reservation(int userId, int numOfBeds, date::year_month_day checkIn, date::year_month_day checkOut);

    void modify(int numOfBeds);

    int getUserId() const;
    int getNumOfBeds() const;
    date::year_month_day getCheckOut() const;

    bool hasConflict(date::year_month_day date) const;
    bool isExpired(date::year_month_day date) const;
    bool canBeCancelled(date::year_month_day date) const;

    bool operator==(const Reservation& other) const;
    bool operator!=(const Reservation& other) const;

    nlohmann::json toJson() const;
};
```

#### Responses

The server will send a response to the client after receiving a request. The responses share the following common format:

```json
{
    "status": "status code",
    "message": "status message",
    "userId": "user id",
    "response": {
        "key1": "value1",
    }
}
```

The status codes are defined in the `StatusCode` enum (*status_code.hpp*). The status message is a human-readable message that describes the status code. The user id is the id of the user that sent the request. The response is an optional field that contains the response data.

The status codes include:

```cpp
struct StatusCode {
    enum type {
        RoomNotFound = 101,
        ReservationNotFound = 102,
        RoomAdded = 104,
        RoomModified = 105,
        RoomDeleted = 106,
        BalanceNotEnough = 108,
        RoomCapacityFull = 109,
        CancelOK = 110,
        RoomExists = 111,
        LoggedOut = 201,
        SignedIn = 230,
        SignedUp = 231,
        UsernameDoesNotExist = 311,
        UserInfoChanged = 312,
        InvalidValue = 401,
        AccessDenied = 403,
        InvalidCapacity = 412,
        UserLeftRoom = 413,
        WrongUserPassword = 430,
        UsernameExists = 451,
        BadCommand = 503,
        OK = 1200,
        BadRequest = 1400,
        Unauthorized = 1401,
    };
};
```

An example request handler in the server for the `allUsers` command:

```cpp
nlohmann::json HotelManager::handleAllUsers(const nlohmann::json& request) {
    std::string token;
    if (!getRequestToken(request, token)) {
        return {
            {"status", StatusCode::BadRequest},
            {"message", "Token not provided"},
            {"userId", ""},
            {"response", nullptr},
        };
    }
    int userId = getUser(token);
    if (userId == -1) {
        return {
            {"status", StatusCode::Unauthorized},
            {"message", "Invalid token"},
            {"userId", ""},
            {"response", nullptr},
        };
    }
    refreshTokenAccessTime(token);
    if (!isAdministrator(userId)) {
        return {
            {"status", StatusCode::AccessDenied},
            {"message", "Access denied"},
            {"userId", std::to_string(userId)},
            {"response", nullptr},
        };
    }
    return {
        {"status", StatusCode::OK},
        {"message", "All users"},
        {"userId", std::to_string(userId)},
        {"response", getAllUsers()},
    };
}
```

The handler first checks if the request contains a token. If not, it will return a `BadRequest` response because this command requires authentication.  
Next, the handler will check if the token is valid. If not, it will return an `Unauthorized` response.  
Now, the handler will check if the user is an administrator. If not, it will return an `AccessDenied` response since this command is for admins only.  
Finally, the handler will return an `OK` response with the list of all users.
