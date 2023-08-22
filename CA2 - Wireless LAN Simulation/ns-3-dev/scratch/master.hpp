#ifndef MASTER_HPP_INCLUDE
#define MASTER_HPP_INCLUDE

#include <array>
#include <ctime>

#include "header.hpp"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

class Master : public Application {
public:
    Master(uint16_t port, Ipv4InterfaceContainer& ip,
           std::array<uint16_t, consts::MAPPERS_COUNT> mapperPorts, Ipv4InterfaceContainer& mapperIps);
    virtual ~Master() = default;

private:
    void StartApplication(void) override;
    void HandleRead(Ptr<Socket> socket);
    void ForwardHeader(ClientHeader header);

    uint16_t port_;
    Ipv4InterfaceContainer ip_;
    std::array<uint16_t, consts::MAPPERS_COUNT> mapperPorts_;
    Ipv4InterfaceContainer mapperIps_;
    Ptr<Socket> socket_;
    std::array<Ptr<Socket>, consts::MAPPERS_COUNT> mapperSockets_;
};

Master::Master(uint16_t port, Ipv4InterfaceContainer& ip,
               std::array<uint16_t, consts::MAPPERS_COUNT> mapperPorts, Ipv4InterfaceContainer& mapperIps)
    : port_(port),
      ip_(ip),
      mapperPorts_(mapperPorts),
      mapperIps_(mapperIps) {
    std::srand(time(nullptr));
}

void Master::StartApplication(void) {
    socket_ = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress local(ip_.GetAddress(0), port_);
    socket_->Bind(local);

    for (unsigned i = 0; i < mapperPorts_.size(); ++i) {
        Ptr<Socket> mapperSocket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        InetSocketAddress mapper(mapperIps_.GetAddress(i), mapperPorts_[i]);
        mapperSocket->Connect(mapper);
        mapperSockets_[i] = mapperSocket;
    }

    socket_->SetRecvCallback(MakeCallback(&Master::HandleRead, this));
}

void Master::HandleRead(Ptr<Socket> socket) {
    Ptr<Packet> packet;

    while (packet = socket->Recv()) {
        if (packet->GetSize() == 0) {
            break;
        }

        ClientHeader destinationHeader;
        packet->RemoveHeader(destinationHeader);

        ForwardHeader(destinationHeader);
    }
}

void Master::ForwardHeader(ClientHeader header) {
    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(header);

    for (auto& socket : mapperSockets_) {
        socket->Send(packet);
    }
}

#endif // MASTER_HPP_INCLUDE
