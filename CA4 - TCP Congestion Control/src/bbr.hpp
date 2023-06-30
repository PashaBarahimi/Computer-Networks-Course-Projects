#ifndef BBR_HPP_INCLUDE
#define BBR_HPP_INCLUDE

#include "tcp_connection.hpp"

namespace minprobing {

constexpr int INTERVAL = 20;
constexpr int RTTS = 3;
constexpr int CWND = 20;
constexpr float BASE_RTT = 1000.0f;

} // namespace minprobing

constexpr float INITIAL_DRAINAGE = 0.2;

class Bbr : public TcpConnection {
public:
    enum class Mode {
        startup,
        drain,
        probeBw,
        probeRtt,
    };

    using TcpConnection::TcpConnection;

    void sendData() override;
    bool onPacketLoss() override;
    void onRttUpdate() override;

private:
    Mode mode_ = Mode::startup;
    bool lostAny_ = false;
    float drainage_ = INITIAL_DRAINAGE;

    bool bwInc_ = true;
    int saveCwnd_ = 1;
    int probeCounter_ = minprobing::INTERVAL - 1;
    float minRtt_ = minprobing::BASE_RTT;
    const float bandwidth_ = 1000.0f;
};

#endif // BBR_HPP_INCLUDE
