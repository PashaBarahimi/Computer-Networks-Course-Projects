#include "bbr.hpp"

#include "utils.hpp"

void Bbr::sendData() {
    if (timeout_ != 0) return;
    if (sw_.lastAck == static_cast<int>(packets_.size()) - 1) return;

    lostAny_ = false;
    int startAck = sw_.lastAck + 1;
    for (int i = startAck; i < startAck + cwnd_ && i < static_cast<int>(packets_.size()); ++i) {
        if (packetLost()) {
            lostAny_ = true;
        }
        else if (!lostAny_) {
            ++sw_.lastAck;
            packets_[i].acked = true;
        }
        else {
            break;
        }
    }

    float estRtt = static_cast<float>(utils::randInt(20, 200)) + utils::randInt(1, 100) / 100;
    if (mode_ == Mode::probeRtt) {
        minRtt_ = std::min(minRtt_, estRtt);
    }
    else if (mode_ == Mode::probeBw) {
        // float p = bandwidth_ * estRtt / minRtt_;
        bwInc_ = !bwInc_;
    }
}

bool Bbr::onPacketLoss() {
    if (lostAny_) {
        if (mode_ == Mode::startup) {
            mode_ = Mode::drain;
        }
    }
    else if (mode_ == Mode::drain) {
        mode_ = Mode::probeBw;
    }
    return false;
}

void Bbr::onRttUpdate() {
    if (mode_ != Mode::startup && mode_ != Mode::drain) {
        --probeCounter_;
        if (probeCounter_ == minprobing::INTERVAL) {
            mode_ = Mode::probeBw;
            cwnd_ = saveCwnd_;
            return;
        }
        if (probeCounter_ == 0) {
            probeCounter_ = minprobing::INTERVAL + minprobing::RTTS;
            mode_ = Mode::probeRtt;
            minRtt_ = minprobing::BASE_RTT;
            saveCwnd_ = cwnd_;
        }
    }

    switch (mode_) {
    case Mode::startup:
        cwnd_ *= 2;
        break;
    case Mode::drain:
        cwnd_ *= 1 - drainage_;
        drainage_ -= 0.01;
        saveCwnd_ = cwnd_;
        break;
    case Mode::probeBw:
        if (bwInc_) {
            cwnd_ = saveCwnd_;
            cwnd_ *= 1.05;
        }
        else {
            cwnd_ = cwnd_ * 0.95;
        }
        break;
    case Mode::probeRtt:
        cwnd_ = minprobing::CWND;
        break;
    default:;
    }
}
