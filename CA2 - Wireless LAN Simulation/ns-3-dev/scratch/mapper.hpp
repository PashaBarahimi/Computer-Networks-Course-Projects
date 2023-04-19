#ifndef MAPPER_HPP_INCLUDE
#define MAPPER_HPP_INCLUDE

#include <unordered_map>

#include "header.hpp"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

class Mapper : public Application {
public:
    Mapper(uint16_t port, Ipv4InterfaceContainer& ip, unsigned idx, const std::unordered_map<uint16_t, char>& map);
    virtual ~Mapper() = default;

private:
    void StartApplication(void) override;
    void SendMappedData(char data, Ipv4Address ip, uint16_t port);
    char Map(uint16_t data) const;
    void HandleRead(Ptr<Socket> socket);
    void HandleAccept(Ptr<Socket> socket, const Address& from);

    uint16_t port_;
    Ipv4InterfaceContainer ip_;
    Ptr<Socket> socket_;
    unsigned idx_;
    std::unordered_map<uint16_t, char> map_;
};

Mapper::Mapper(uint16_t port, Ipv4InterfaceContainer& ip, unsigned idx, const std::unordered_map<uint16_t, char>& map)
    : port_(port),
      ip_(ip),
      idx_(idx),
      map_(map) {}

void Mapper::StartApplication(void) {
    socket_ = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
    InetSocketAddress local = InetSocketAddress(ip_.GetAddress(idx_), port_);
    socket_->Bind(local);
    socket_->Listen();

    socket_->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address&>(),
                               MakeCallback(&Mapper::HandleAccept, this));
}

void Mapper::SendMappedData(char data, Ipv4Address ip, uint16_t port) {
    MapperHeader header;
    header.SetData(data);

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(header);

    Ptr<Socket> socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress destination = InetSocketAddress(ip, port);
    socket->Connect(destination);
    socket->Send(packet);
    socket->Close();
}

char Mapper::Map(uint16_t data) const {
    auto it = map_.find(data);
    if (it != map_.end())
        return it->second;
    return 0;
}

void Mapper::HandleRead(Ptr<Socket> socket) {
    Ptr<Packet> packet;

    while ((packet = socket->Recv())) {
        if (packet->GetSize() == 0) {
            break;
        }

        ClientHeader header;
        packet->RemoveHeader(header);

        Ipv4Address src = header.GetSourceIP();
        uint16_t srcPort = header.GetSourcePort();
        uint16_t data = header.GetData();

        char mappedData = Map(data);
        if (mappedData != 0)
            SendMappedData(mappedData, src, srcPort);
    }
}

void Mapper::HandleAccept(Ptr<Socket> socket, const Address& from) {
    socket->SetRecvCallback(MakeCallback(&Mapper::HandleRead, this));
}

#endif // MAPPER_HPP_INCLUDE
