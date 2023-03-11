#ifndef NETWORK_HPP_INCLUDE
#define NETWORK_HPP_INCLUDE

#include <sys/socket.h>

#include <array>
#include <string>

#include "endian.hpp"

struct sockaddr_in;

namespace net {

template <class T>
inline T hton(T in) {
    if (byte::endian == byte::Endian::big) {
        return in;
    }
    else {
        return byte::swapOrder(in);
    }
}

template <class T>
inline T ntoh(T in) {
    if (byte::endian == byte::Endian::big) {
        return in;
    }
    else {
        return byte::swapOrder(in);
    }
}

using Port = unsigned short;

class IpAddr {
public:
    IpAddr() = default;
    IpAddr(unsigned short a, unsigned short b, unsigned short c, unsigned short d);
    IpAddr(const std::string& addr);
    IpAddr(const char* addr);
    explicit IpAddr(unsigned int addr);

    unsigned short& operator[](int i);
    unsigned short operator[](int i) const;

    bool operator==(IpAddr rhs) const;
    bool operator!=(IpAddr lhs) const;

    std::string toStr() const;
    unsigned int toInt() const;
    bool fromStr(const std::string& addr);
    void fromInt(unsigned int addr);

    static IpAddr any() { return IpAddr(0, 0, 0, 0); }
    static IpAddr loopback() { return IpAddr(127, 0, 0, 1); }

private:
    std::array<unsigned short, 4> addr_{};
};

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
    ~Socket();

    bool bind(IpAddr addr, Port port);
    bool listen(int queueSize);
    bool connect(IpAddr addr, Port port);
    bool accept(Socket& outSocket);

    bool send(const std::string& data);
    bool receive(std::string& data);

    bool operator==(const Socket& rhs) const;
    bool operator!=(const Socket& rhs) const;

private:
    Type type_ = Type::unused;
    IpAddr addr_ = {};
    Port port_ = 0;
    Status status_ = Status::unused;
    int socket_ = -1;

    friend class Select;

    static sockaddr_in ipToSockaddr(IpAddr addr, Port port);
    static void sockaddrToIp(sockaddr_in addrIn, IpAddr& outAddr, Port& outPort);
};

} // namespace net

#endif // NETWORK_HPP_INCLUDE
