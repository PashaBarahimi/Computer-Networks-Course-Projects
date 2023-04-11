#ifndef HEADER_HPP_INCLUDE
#define HEADER_HPP_INCLUDE

#include "constants.hpp"
#include "ns3/udp-header.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

class MisashaHeader : public Header {
public:
    MisashaHeader() = default;
    virtual ~MisashaHeader() = default;
    void SetData(uint16_t data);
    uint16_t GetData(void) const;
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream& os) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);
    virtual uint32_t GetSerializedSize(void) const;

private:
    uint16_t m_data;
};

TypeId MisashaHeader::GetTypeId(void) {
    static TypeId tid =
        TypeId("ns3::MisashaHeader").SetParent<Header>().AddConstructor<MisashaHeader>();
    return tid;
}

TypeId MisashaHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void MisashaHeader::Print(std::ostream& os) const {
    os << "data = " << m_data << std::endl;
}

uint32_t MisashaHeader::GetSerializedSize(void) const {
    return consts::HEADER_LENGTH;
}

void MisashaHeader::Serialize(Buffer::Iterator start) const {
    start.WriteHtonU16(m_data);
}

uint32_t MisashaHeader::Deserialize(Buffer::Iterator start) {
    m_data = start.ReadNtohU16();

    return consts::HEADER_LENGTH;
}

void MisashaHeader::SetData(uint16_t data) {
    m_data = data;
}

uint16_t MisashaHeader::GetData(void) const {
    return m_data;
}

#endif // HEADER_HPP_INCLUDE
