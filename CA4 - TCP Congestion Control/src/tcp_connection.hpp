#ifndef TCP_CONNECTION_HPP_INCLUDE
#define TCP_CONNECTION_HPP_INCLUDE

#include <algorithm>
#include <limits>
#include <ostream>
#include <vector>

#include "packet.hpp"

constexpr int TIMEOUT_WAITTIME = 2;

class TcpConnection {
public:
    TcpConnection(const std::vector<int>& data);
    TcpConnection(const std::vector<int>& data, int cwnd, int thold);
    virtual ~TcpConnection() = default;

    virtual void sendData() = 0;
    virtual bool onPacketLoss() = 0;
    virtual void onRttUpdate() = 0;

    void run(std::ostream& os);

protected:
    const int awnd_ = 1000;
    int cwnd_ = 1;
    int ssthresh_ = std::numeric_limits<int>::max();
    int rtt_ = 0;
    int timeout_ = 0;

    std::vector<Packet> packets_;
    SlidingWindow sw_{-1, -1, -1};
    int getWindowSize() const;
    void packetize(const std::vector<int>& data);
    bool packetLost() const;
    void log(std::ostream& os) const;
};

#endif // TCP_CONNECTION_HPP_INCLUDE
