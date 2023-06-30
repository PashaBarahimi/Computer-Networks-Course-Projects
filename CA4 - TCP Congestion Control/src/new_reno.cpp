#include "new_reno.hpp"

void NewReno::sendData() {
    if (timeout_ != 0) return;
    if (sw_.lastAck == static_cast<int>(packets_.size()) - 1) return;

    lostCount_ = 0;
    ackAfterLoss_ = 0;
    int startAck = sw_.lastAck + 1;
    int sackCount = 0;
    for (int i = startAck; i < startAck + sackCount + cwnd_ && i < static_cast<int>(packets_.size()); ++i) {
        if (packets_[i].acked) {
            ++sackCount;
            if (lostCount_ == 0) {
                ++sw_.lastAck;
            }
            continue;
        }
        if (packetLost()) {
            ++lostCount_;
        }
        else if (lostCount_ == 0) {
            ++sw_.lastAck;
            packets_[i].acked = true;
        }
        else {
            ++ackAfterLoss_;
            packets_[i].acked = true;
            // if (ackAfterLoss_ == RETRANSMIT_ACKS) {
            //     break;
            // }
        }
    }
}
