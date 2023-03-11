#include "net.hpp"

#include <netinet/in.h>
#include <unistd.h>

#include <cstring>
#include <regex>
#include <sstream>

namespace net {

IpAddr::IpAddr(unsigned short a, unsigned short b, unsigned short c, unsigned short d)
    : addr_{a, b, c, d} {}

IpAddr::IpAddr(const std::string& addr) {
    fromStr(addr);
}

IpAddr::IpAddr(const char* addr)
    : IpAddr(std::string(addr)) {}

IpAddr::IpAddr(unsigned int addr) {
    fromInt(addr);
}

unsigned short& IpAddr::operator[](int i) { return addr_[i]; }
unsigned short IpAddr::operator[](int i) const { return addr_[i]; }

bool IpAddr::operator==(IpAddr rhs) const { return addr_ == rhs.addr_; }
bool IpAddr::operator!=(IpAddr lhs) const { return addr_ != lhs.addr_; }

std::string IpAddr::toStr() const {
    std::ostringstream sstr;
    sstr << addr_[0] << '.' << addr_[1] << '.' << addr_[2] << '.' << addr_[3];
    return sstr.str();
}

unsigned int IpAddr::toInt() const {
    unsigned int res = 0;
    for (int i = 0; i < 4; ++i) {
        res |= addr_[i] << ((3 - i) * 8);
    }
    return res;
}

bool IpAddr::fromStr(const std::string& addr) {
    static std::regex re(R"((\d+)\.(\d+)\.(\d+)\.(\d+))");
    std::smatch match;
    if (!std::regex_match(addr, match, re)) {
        return false;
    }
    IpAddr res;
    for (int i = 0; i < 4; ++i) {
        int octet = std::stoi(match[i + 1]);
        if (octet > 255) {
            return false;
        }
        res[i] = octet;
    }
    *this = res;
    return true;
}

void IpAddr::fromInt(unsigned int addr) {
    for (int i = 0; i < 4; ++i) {
        addr_[i] = (addr >> ((3 - i) * 8)) & 0xFF;
    }
}

Socket::Socket(Type type)
    : type_(type) {
    socket_ = socket(PF_INET, static_cast<int>(type_), 0);
    if (type_ == Type::stream) {
        int reuseaddr = 1;
        setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
    }
}

Socket::~Socket() {
    if (socket_ != -1) {
        close(socket_);
    }
}

bool Socket::bind(IpAddr addr, Port port) {
    sockaddr_in addrIn = ipToSockaddr(addr, port);
    int res = ::bind(socket_, reinterpret_cast<sockaddr*>(&addrIn), sizeof(addrIn));
    if (res == -1) {
        return false;
    }
    addr_ = addr;
    port_ = port;
    status_ = Status::bound;
    return true;
}

bool Socket::listen(int queueSize) {
    int res = ::listen(socket_, queueSize);
    if (res == -1) {
        return false;
    }
    status_ = Status::listening;
    return true;
}

bool Socket::connect(IpAddr addr, Port port) {
    sockaddr_in addrIn = ipToSockaddr(addr, port);
    int res = ::connect(socket_, reinterpret_cast<sockaddr*>(&addrIn), sizeof(addrIn));
    if (res == -1) {
        return false;
    }
    addr_ = addr;
    port_ = port;
    status_ = Status::connected;
    return true;
}

bool Socket::accept(Socket& outSocket) {
    sockaddr_in clientAddrIn{};
    socklen_t addrSize = sizeof(clientAddrIn);
    int res = ::accept(socket_, reinterpret_cast<sockaddr*>(&clientAddrIn), &addrSize);
    if (res == -1) {
        return false;
    }
    outSocket.socket_ = res;
    sockaddrToIp(clientAddrIn, outSocket.addr_, outSocket.port_);
    outSocket.status_ = Status::connected;
    return true;
}

bool Socket::send(const std::string& data) {
    int res = ::send(socket_, data.c_str(), data.size(), 0);
    if (res == -1) {
        return false;
    }
    return true;
}

bool Socket::receive(std::string& data) {
    std::array<char, 1024> buf;

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(socket_, &readSet);
    timeval instantTimeout{0, 0};

    while (true) {
        int receivedBytes = ::recv(socket_, &buf, buf.size(), 0);
        if (receivedBytes == -1) {
            return false;
        }
        if (receivedBytes == 0) {
            break;
        }

        data.append(buf.data(), receivedBytes);
        if (receivedBytes != static_cast<int>(buf.size())) {
            break;
        }

        select(socket_ + 1, &readSet, nullptr, nullptr, &instantTimeout);
        if (!FD_ISSET(socket_, &readSet)) {
            break;
        }
    }
    return true;
}

bool Socket::operator==(const Socket& rhs) const { return socket_ == rhs.socket_; }
bool Socket::operator!=(const Socket& rhs) const { return socket_ != rhs.socket_; }

sockaddr_in Socket::ipToSockaddr(IpAddr addr, Port port) {
    sockaddr_in addrIn{};
    addrIn.sin_family = AF_INET;
    addrIn.sin_port = hton(port);
    addrIn.sin_addr.s_addr = hton(addr.toInt());
    return addrIn;
}

void Socket::sockaddrToIp(sockaddr_in addrIn, IpAddr& outAddr, Port& outPort) {
    auto addrInt = ntoh(addrIn.sin_addr.s_addr);
    outAddr = IpAddr(addrInt);
    outPort = ntoh(addrIn.sin_port);
}

Select::Select() {
    FD_ZERO(&readMaster_);
    FD_ZERO(&writeMaster_);
    FD_ZERO(&exceptMaster_);
    FD_ZERO(&readWorking_);
    FD_ZERO(&writeWorking_);
    FD_ZERO(&exceptWorking_);
}

Select::~Select() {
    for (auto& s : socketsMap_) {
        if (s.second.second) {
            delete s.second.first;
        }
    }
}

void Select::addRead(Socket* socket, bool own) {
    add(&readMaster_, socket, own);
}
void Select::addWrite(Socket* socket, bool own) {
    add(&writeMaster_, socket, own);
}
void Select::addExcept(Socket* socket, bool own) {
    add(&exceptMaster_, socket, own);
}

void Select::removeRead(Socket* socket) {
    remove(&readMaster_, socket);
}
void Select::removeWrite(Socket* socket) {
    remove(&writeMaster_, socket);
}
void Select::removeExcept(Socket* socket) {
    remove(&exceptMaster_, socket);
}

bool Select::isInRead(const Socket* socket) const {
    return FD_ISSET(socket->socket_, &readMaster_);
}
bool Select::isInWrite(const Socket* socket) const {
    return FD_ISSET(socket->socket_, &writeMaster_);
}
bool Select::isInExcept(const Socket* socket) const {
    return FD_ISSET(socket->socket_, &exceptMaster_);
}

int Select::select(int timeoutMs) {
    timeval tv;
    timeval* tvPtr = nullptr;
    if (timeoutMs != -1) {
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        tvPtr = &tv;
    }

    readWorking_ = readMaster_;
    writeWorking_ = writeMaster_;
    exceptWorking_ = exceptMaster_;

    fd_set* readWk = nullptr;
    fd_set* writeWk = nullptr;
    fd_set* exceptWk = nullptr;

    if (isAnySet(&readMaster_)) {
        readWk = &readWorking_;
    }
    if (isAnySet(&writeMaster_)) {
        writeWk = &writeWorking_;
    }
    if (isAnySet(&exceptMaster_)) {
        exceptWk = &exceptWorking_;
    }

    return ::select(max_, readWk, writeWk, exceptWk, tvPtr);
}

bool Select::isReadyRead(const Socket* socket) const {
    return FD_ISSET(socket->socket_, &readWorking_);
}
bool Select::isReadyWrite(const Socket* socket) const {
    return FD_ISSET(socket->socket_, &writeWorking_);
}
bool Select::isReadyExcept(const Socket* socket) const {
    return FD_ISSET(socket->socket_, &exceptWorking_);
}

std::vector<Socket*> Select::getReadyRead() const {
    return getReady(&readWorking_);
}
std::vector<Socket*> Select::getReadyWrite() const {
    return getReady(&writeWorking_);
}
std::vector<Socket*> Select::getReadyExcept() const {
    return getReady(&exceptWorking_);
}

std::vector<Socket*> Select::getReady(const fd_set* fdset) const {
    std::vector<Socket*> readySockets;
    for (int i = 0; i < max_; ++i) {
        if (FD_ISSET(i, fdset)) {
            const auto itr = socketsMap_.find(i);
            readySockets.push_back(itr->second.first);
        }
    }
    return readySockets;
}

void Select::add(fd_set* fdset, Socket* socket, bool own) {
    const int fd = socket->socket_;
    socketsMap_[fd] = {socket, own};

    FD_SET(fd, fdset);
    if (fd >= max_) {
        max_ = fd + 1;
    }
}

void Select::remove(fd_set* fdset, Socket* socket) {
    const int fd = socket->socket_;
    auto itr = socketsMap_.find(fd);
    if (itr->second.second) {
        delete itr->second.first;
    }
    socketsMap_.erase(itr);

    FD_CLR(fd, fdset);
    if (fd == max_ - 1) {
        --max_;
    }
}

bool Select::isAnySet(const fd_set* fdset) const {
    fd_set emptySet;
    FD_ZERO(&emptySet);
    return std::memcmp(fdset, &emptySet, sizeof(fd_set));
}

} // namespace net
