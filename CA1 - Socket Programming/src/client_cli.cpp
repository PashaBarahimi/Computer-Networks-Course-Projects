#include "client_cli.hpp"

#include <ostream>
#include <regex>
#include <sstream>

#include "cli/clilocalsession.h"
#include "cli/loopscheduler.h"

ClientCLI::ClientCLI(HotelClient& client) : client_(client) {
    inputQueue_ = new concurrent::Queue<std::pair<cli::detail::KeyType, char>>();
    setupCLI();
}

ClientCLI::~ClientCLI() {
    delete cli_;
    delete session_;
    delete inputQueue_;
}

void ClientCLI::run() {
    cli::LoopScheduler scheduler;
    session_ = new cli::CliLocalSession(*cli_, scheduler, std::cout, 200);
    session_->ExitAction([&scheduler](auto& out) { scheduler.Stop(); });
    session_->SetCustomQueue(inputQueue_);
    scheduler.Run();
}

void ClientCLI::setupCLI() {
    auto mainMenu = createMainMenu();
    cli_ = new cli::Cli(std::move(mainMenu));
    cli::SetColor();
    cli_->EnterAction([](auto& out) { out << "Welcome to Hotel Misasha\n"; });
    cli_->ExitAction([](auto& out) { out << "Thank you for using Hotel Misasha\n"; });
    cli_->StdExceptionHandler([](auto& out, const auto& cmd, const auto& e) {
        out << "Error: " << e.what() << std::endl;
    });
}

std::unique_ptr<cli::Menu> ClientCLI::createMainMenu() {
    std::string menuName = "MainMenu";
    auto mainMenu = std::make_unique<cli::Menu>(menuName);
    loginMenu_.push_back(mainMenu->Insert(
        "signin", [this](std::ostream& out, const std::string& username) {
            std::string password;
            if (!inputPassword(out, password, false))
                return;
            out << client_.signin(username, password) << std::endl;
            checkMainMenuItems();
        },
        "Login (usage: signin <username>)"));
    loginMenu_.push_back(mainMenu->Insert(
        "signup", [this](std::ostream& out, const std::string& username) {
            bool userExists = client_.doesUserExist(username);
            if (userExists) {
                out << "Username already exists." << std::endl;
            }
            else {
                out << "Username is available." << std::endl;
            }
            std::string password, phone, email;
            int balance;
            if (!inputPassword(out, password)) {
                return;
            }
            if (!getIntInput(out, "Enter your balance: ", balance)) {
                return;
            }
            phone = getInput(out, "Enter your phone number: ");
            email = getInput(out, "Enter your email: ");
            out << client_.signup(username, password, balance, phone, email) << std::endl;
            checkMainMenuItems();
        },
        "Signup (usage: signup <username>)"));
    userMenu_.push_back(mainMenu->Insert(
        "UserInfo", [this](std::ostream& out) {
            out << client_.userInfo() << std::endl;
            checkMainMenuItems();
        },
        "View user information (usage: UserInfo)"));
    userMenu_.push_back(mainMenu->Insert(
        "AllUsers", [this](std::ostream& out) {
            out << client_.allUsers() << std::endl;
            checkMainMenuItems();
        },
        "View all users (usage: AllUsers)"));
    userMenu_.push_back(mainMenu->Insert(
        "RoomsInfo", [this](std::ostream& out) {
            out << client_.roomsInfo() << std::endl;
            checkMainMenuItems();
        },
        "View rooms information (usage: RoomsInfo)"));
    userMenu_.push_back(mainMenu->Insert(std::move(createBookMenu(menuName))));
    userMenu_.push_back(mainMenu->Insert(std::move(createCancelMenu(menuName))));
    userMenu_.push_back(mainMenu->Insert(std::move(createPassDayMenu(menuName))));
    userMenu_.push_back(mainMenu->Insert(
        "EditInfo", [this](std::ostream& out) {
            std::string password, phone, email;
            if (!inputPassword(out, password)) {
                return;
            }
            phone = getInput(out, "Enter your phone number: ");
            email = getInput(out, "Enter your email: ");
            out << client_.editInfo(password, phone, email) << std::endl;
            checkMainMenuItems();
        },
        "Edit user information (usage: EditInfo)"));
    userMenu_.push_back(mainMenu->Insert(std::move(createLeaveRoomMenu(menuName))));
    userMenu_.push_back(mainMenu->Insert(std::move(createRoomsMenu(menuName))));
    userMenu_.push_back(mainMenu->Insert(
        "Logout", [this](std::ostream& out) {
            out << client_.logout() << std::endl;
            checkMainMenuItems();
        },
        "Logout (usage: Logout)"));
    checkMainMenuItems();
    return mainMenu;
}

std::unique_ptr<cli::Menu> ClientCLI::createBookMenu(const std::string& parentName) {
    auto bookMenu = std::make_unique<cli::Menu>("Book", "[book]");
    bookMenu->Insert(
        "book", [this, parentName](std::ostream& out, const std::string& roomNum, int numOfBeds, const std::string& checkInDate, const std::string& checkOutDate) {
            if (!isDateValid(checkInDate) || !isDateValid(checkOutDate)) {
                out << "Invalid date format." << std::endl;
                return;
            }
            out << client_.book(roomNum, numOfBeds, checkInDate, checkOutDate) << std::endl;
            session_->Feed(parentName);
            checkMainMenuItems();
        },
        "Book a room (usage: book <room number> <number of beds> <check in date> <check out date>)");
    return bookMenu;
}

std::unique_ptr<cli::Menu> ClientCLI::createCancelMenu(const std::string& parentName) {
    auto cancelMenu = std::make_unique<cli::Menu>("Cancel", "[cancel]");
    cancelMenu->Insert(
        "cancel", [this, parentName](std::ostream& out, const std::string& roomNum, int numOfBeds) {
            out << client_.cancel(roomNum, numOfBeds) << std::endl;
            session_->Feed(parentName);
            checkMainMenuItems();
        },
        "Cancel a room (usage: cancel <room number> <number of beds>)");
    return cancelMenu;
}

std::unique_ptr<cli::Menu> ClientCLI::createPassDayMenu(const std::string& parentName) {
    auto passDayMenu = std::make_unique<cli::Menu>("PassDay", "[passDay]");
    passDayMenu->Insert(
        "passDay", [this, parentName](std::ostream& out, int numOfDays) {
            out << client_.passDay(numOfDays) << std::endl;
            session_->Feed(parentName);
            checkMainMenuItems();
        },
        "Pass days (usage: passDay <number of days>)");
    return passDayMenu;
}

std::unique_ptr<cli::Menu> ClientCLI::createLeaveRoomMenu(const std::string& parentName) {
    auto leaveRoomMenu = std::make_unique<cli::Menu>("LeaveRoom", "[leaveRoom]");
    leaveRoomMenu->Insert(
        "leaveRoom", [this, parentName](std::ostream& out, const std::string& roomNum) {
            out << client_.leaveRoom(roomNum) << std::endl;
            session_->Feed(parentName);
            checkMainMenuItems();
        },
        "Leave a room (usage: leaveRoom <room number>)");
    return leaveRoomMenu;
}

std::unique_ptr<cli::Menu> ClientCLI::createRoomsMenu(const std::string& parentName) {
    auto roomsMenu = std::make_unique<cli::Menu>("Rooms", "[add] [modify] [remove]");
    roomsMenu->Insert(
        "add", [this, parentName](std::ostream& out, const std::string& roomNum, int maxCapacity, int price) {
            out << client_.addRoom(roomNum, maxCapacity, price) << std::endl;
            session_->Feed(parentName);
            checkMainMenuItems();
        },
        "Add a room (usage: add <room number> <max capacity> <price>)");
    roomsMenu->Insert(
        "modify", [this, parentName](std::ostream& out, const std::string& roomNum, int newMaxCapacity, int newPrice) {
            out << client_.modifyRoom(roomNum, newMaxCapacity, newPrice) << std::endl;
            session_->Feed(parentName);
            checkMainMenuItems();
        },
        "Modify a room (usage: modify <room number> <new max capacity> <new price>)");
    roomsMenu->Insert(
        "remove", [this, parentName](std::ostream& out, const std::string& roomNum) {
            out << client_.removeRoom(roomNum) << std::endl;
            session_->Feed(parentName);
            checkMainMenuItems();
        },
        "Remove a room (usage: remove <room number>)");
    return roomsMenu;
}

bool ClientCLI::getIntInput(std::ostream& out, const std::string& inputMsg, int& input) {
    std::string str = getInput(out, inputMsg);
    try {
        input = std::stoi(str);
        return true;
    }
    catch (const std::invalid_argument&) {
        out << "Invalid input." << std::endl;
        return false;
    }
}

std::string ClientCLI::getInput(std::ostream& out, const std::string& inputMsg, bool hidden, char mask) {
    out << inputMsg << std::flush;
    std::string input;
    inputQueue_->clear();
    session_->PostToCustomQueue();
    cli::detail::Terminal terminal(out);
    while (true) {
        auto c = inputQueue_->pop();
        auto symbol = terminal.Keypressed(c, hidden, mask);
        if (symbol.first == cli::detail::Symbol::command) {
            input = symbol.second;
            break;
        }
    }
    session_->PostToScheduler();
    return input;
}

void ClientCLI::checkMainMenuItems() {
    if (client_.isLoggedIn()) {
        for (auto& item : userMenu_) {
            item.Enable();
        }
        for (auto& item : loginMenu_) {
            item.Disable();
        }
    }
    else {
        for (auto& item : userMenu_) {
            item.Disable();
        }
        for (auto& item : loginMenu_) {
            item.Enable();
        }
    }
}

bool ClientCLI::inputPassword(std::ostream& out, std::string& password, bool check) {
    password = getInput(out, "Enter your password: ", true, '*');
    if (password.empty()) {
        out << "Password cannot be empty." << std::endl;
        return false;
    }
    if (!check) {
        return true;
    }
    std::string password2 = getInput(out, "Enter your password again: ", true, '*');
    if (password != password2) {
        out << "Passwords do not match." << std::endl;
        return false;
    }
    return true;
}

bool ClientCLI::isDateValid(const std::string& date) {
    std::regex dateRegex(R"((\d{4})-(\d{2})-(\d{2}))");
    std::smatch match;
    if (!std::regex_match(date, match, dateRegex)) {
        return false;
    }
    int year = std::stoi(match[1]);
    int month = std::stoi(match[2]);
    int day = std::stoi(match[3]);
    if (year < 1970 || year > 2100) {
        return false;
    }
    if (month < 1 || month > 12) {
        return false;
    }
    if (day < 1 || day > 31) {
        return false;
    }
    if (month == 2 && day > 29) {
        return false;
    }
    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) {
        return false;
    }
    return true;
}
