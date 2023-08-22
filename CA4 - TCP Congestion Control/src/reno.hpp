#ifndef RENO_HPP_INCLUDE
#define RENO_HPP_INCLUDE

#include "tcp_connection.hpp"

constexpr int RETRANSMIT_ACKS = 3;

class Reno : public TcpConnection {
public:
    enum class Mode {
        addIncMultDec,
        slowStart,
        fastRetransmit
    };

    using TcpConnection::TcpConnection;

    void sendData() override;
    bool onPacketLoss() override;
    void onRttUpdate() override;

protected:
    Mode mode_ = Mode::slowStart;
    const bool fastRecovery_ = true;

    int lostCount_ = 0;
    int ackAfterLoss_ = 0;
};

#endif // RENO_HPP_INCLUDE
