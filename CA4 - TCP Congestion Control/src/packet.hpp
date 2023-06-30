#ifndef PACKET_HPP_INCLUDE
#define PACKET_HPP_INCLUDE

#include <cstdint>

struct TCPHeader {
    uint16_t srcPort;
    uint16_t dstPort;
    uint32_t seqNum;
    uint32_t acknowledge;
    uint16_t checksum;
    uint16_t advWindow;
};

struct IPHeader {
    uint16_t version;
    uint16_t checksum;
    uint32_t srcIp;
    uint32_t dstIp;
};

struct Packet {
    TCPHeader tcpHeader;
    IPHeader ipHeader;
    int data;
    bool acked;
};

struct SlidingWindow {
    int lastAck;
    int lastWritten;
    int lastSent;
};

#endif // PACKET_HPP_INCLUDE
