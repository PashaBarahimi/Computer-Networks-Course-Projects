#include "net.hpp"

#include <netinet/in.h>
#include <unistd.h>

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

} // namespace net
