#ifndef NET_HPP_INCLUDE
#define NET_HPP_INCLUDE

#include <sys/select.h>
#include <sys/socket.h>

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

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

using Port = std::uint8_t;

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

private:
    std::array<std::uint8_t, 4> addr_{};
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

private:
    int max_ = 0;
    fd_set readMaster_, readWorking_;
    fd_set writeMaster_, writeWorking_;
    fd_set exceptMaster_, exceptWorking_;

    std::unordered_map<int, std::pair<Socket*, bool>> socketsMap_;

    std::vector<Socket*> getReady(const fd_set* fdset) const;
    void add(fd_set* fdset, Socket* socket, bool own);
    void remove(fd_set* fdset, Socket* socket);
    bool isAnySet(const fd_set* fdset) const;
};

} // namespace net

#endif // NET_HPP_INCLUDE
