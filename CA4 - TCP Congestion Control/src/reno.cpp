#include "reno.hpp"

void Reno::sendData() {
    if (timeout_ != 0) return;
    if (sw_.lastAck == static_cast<int>(packets_.size()) - 1) return;

    lostCount_ = 0;
    ackAfterLoss_ = 0;
    int startAck = sw_.lastAck + 1;
    for (int i = startAck; i < startAck + cwnd_ && i < static_cast<int>(packets_.size()); ++i) {
        if (packetLost()) {
            ++lostCount_;
        }
        else if (lostCount_ == 0) {
            ++sw_.lastAck;
            packets_[i].acked = true;
        }
        else {
            ++ackAfterLoss_;
            if (ackAfterLoss_ == RETRANSMIT_ACKS) {
                break;
            }
        }
    }
}

bool Reno::onPacketLoss() {
    if (timeout_ != 0) {
        --timeout_;
        return true;
    }
    if (ackAfterLoss_ >= RETRANSMIT_ACKS) {
        mode_ = Mode::fastRetransmit;
        ssthresh_ = cwnd_ / 2;
        cwnd_ = fastRecovery_ ? ssthresh_ : 1;
        return true;
    }
    if (lostCount_ != 0) {
        timeout_ = TIMEOUT_WAITTIME;
        mode_ = Mode::slowStart;
        ssthresh_ = cwnd_ / 2;
        cwnd_ = 1;
        return true;
    }
    return false;
}

void Reno::onRttUpdate() {
    switch (mode_) {
    case Mode::fastRetransmit:
        mode_ = Mode::addIncMultDec;
    case Mode::addIncMultDec:
        ++cwnd_;
        break;
    case Mode::slowStart:
        if (2 * cwnd_ > ssthresh_) {
            cwnd_ = ssthresh_;
        }
        else if (cwnd_ < ssthresh_) {
            cwnd_ *= 2;
        }
        if (cwnd_ == ssthresh_) {
            mode_ = Mode::addIncMultDec;
        }
        break;
    default:;
    }
}
