#ifndef HEADER_HPP_INCLUDE
#define HEADER_HPP_INCLUDE

#include "constants.hpp"
#include "ns3/udp-header.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

class ClientHeader : public Header {
public:
    ClientHeader() = default;
    virtual ~ClientHeader() = default;

    static TypeId GetTypeId(void);
    TypeId GetInstanceTypeId(void) const override;

    void SetData(uint16_t data);
    void SetSource(Ipv4Address ip, uint16_t port);

    uint16_t GetData(void) const;
    Ipv4Address GetSourceIP(void) const;
    uint16_t GetSourcePort(void) const;

    void Print(std::ostream& os) const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    uint32_t GetSerializedSize(void) const override;

private:
    uint16_t mData_;

    Ipv4Address mSourceIP_;
    uint16_t mSourcePort_;
};

TypeId ClientHeader::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::ClientHeader").SetParent<Header>().AddConstructor<ClientHeader>();
    return tid;
}

TypeId ClientHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void ClientHeader::SetData(uint16_t data) {
    mData_ = data;
}

void ClientHeader::SetSource(Ipv4Address ip, uint16_t port) {
    mSourceIP_ = ip;
    mSourcePort_ = port;
}

Ipv4Address ClientHeader::GetSourceIP(void) const {
    return mSourceIP_;
}

uint16_t ClientHeader::GetSourcePort(void) const {
    return mSourcePort_;
}

uint16_t ClientHeader::GetData(void) const {
    return mData_;
}

void ClientHeader::Print(std::ostream& os) const {
    os << "data = " << mData_ << std::endl;
}

void ClientHeader::Serialize(Buffer::Iterator start) const {
    start.WriteHtonU16(mData_);
    start.WriteHtonU32(mSourceIP_.Get());
    start.WriteHtonU16(mSourcePort_);
}

uint32_t ClientHeader::Deserialize(Buffer::Iterator start) {
    mData_ = start.ReadNtohU16();
    mSourceIP_.Set(start.ReadNtohU32());
    mSourcePort_ = start.ReadNtohU16();
    return consts::CLIENT_HEADER_LENGTH;
}

uint32_t ClientHeader::GetSerializedSize(void) const {
    return consts::CLIENT_HEADER_LENGTH;
}

#endif // HEADER_HPP_INCLUDE
