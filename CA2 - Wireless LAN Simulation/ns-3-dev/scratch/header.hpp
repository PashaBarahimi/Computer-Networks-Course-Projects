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
    void SetData(uint16_t data);
    uint16_t GetData(void) const;
    static TypeId GetTypeId(void);
    TypeId GetInstanceTypeId(void) const override;
    void Print(std::ostream& os) const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    uint32_t GetSerializedSize(void) const override;

private:
    uint16_t mData_;
};

TypeId ClientHeader::GetTypeId(void) {
    static TypeId tid =
        TypeId("ns3::ClientHeader").SetParent<Header>().AddConstructor<ClientHeader>();
    return tid;
}

TypeId ClientHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void ClientHeader::Print(std::ostream& os) const {
    os << "data = " << mData_ << std::endl;
}

uint32_t ClientHeader::GetSerializedSize(void) const {
    return consts::HEADER_LENGTH;
}

void ClientHeader::Serialize(Buffer::Iterator start) const {
    start.WriteHtonU16(mData_);
}

uint32_t ClientHeader::Deserialize(Buffer::Iterator start) {
    mData_ = start.ReadNtohU16();

    return consts::HEADER_LENGTH;
}

void ClientHeader::SetData(uint16_t data) {
    mData_ = data;
}

uint16_t ClientHeader::GetData(void) const {
    return mData_;
}

#endif // HEADER_HPP_INCLUDE
