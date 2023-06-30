#include "tcp_connection.hpp"

#include "utils.hpp"

TcpConnection::TcpConnection(const std::vector<int>& data) {
    packetize(data);
}

TcpConnection::TcpConnection(const std::vector<int>& data, int cwnd, int thold)
    : TcpConnection(data) {
    cwnd_ = cwnd;
    ssthresh_ = thold;
}

int TcpConnection::getWindowSize() const {
    return std::min(cwnd_, awnd_);
}

void TcpConnection::packetize(const std::vector<int>& data) {
    packets_.reserve(data.size());
    for (int d : data) {
        Packet p{};
        p.data = d;
        p.acked = false;
        packets_.push_back(p);
    }
}

bool TcpConnection::packetLost() const {
    int p = static_cast<int>(awnd_ * utils::expProb(cwnd_, 1, awnd_));
    int rand = utils::randInt(1, awnd_);
    return rand <= p;
}

void TcpConnection::run(std::ostream& os) {
    while (sw_.lastAck != static_cast<int>(packets_.size() - 1)) {
        log(os);
        sendData();
        if (!onPacketLoss()) {
            onRttUpdate();
        }
        ++rtt_;
    }
}

void TcpConnection::log(std::ostream& os) const {
    os << '#' << rtt_ << " c-" << cwnd_ << " t-" << ssthresh_ << '\n';
}
