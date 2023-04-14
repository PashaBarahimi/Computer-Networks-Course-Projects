#ifndef MASTER_HPP_INCLUDE
#define MASTER_HPP_INCLUDE

#include <ctime>

#include "header.hpp"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

class Master : public Application {
public:
    Master(uint16_t port, Ipv4InterfaceContainer& ip);
    virtual ~Master() = default;

private:
    void StartApplication(void) override;
    void HandleRead(Ptr<Socket> socket);

    uint16_t port_;
    Ipv4InterfaceContainer ip_;
    Ptr<Socket> socket_;
};

Master::Master(uint16_t port, Ipv4InterfaceContainer& ip) : port_(port), ip_(ip) {
    std::srand(time(nullptr));
}

void Master::StartApplication(void) {
    socket_ = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress local = InetSocketAddress(ip_.GetAddress(0), port_);
    socket_->Bind(local);

    socket_->SetRecvCallback(MakeCallback(&Master::HandleRead, this));
}

void Master::HandleRead(Ptr<Socket> socket) {
    Ptr<Packet> packet;

    while ((packet = socket->Recv())) {
        if (packet->GetSize() == 0) {
            break;
        }

        ClientHeader destinationHeader;
        packet->RemoveHeader(destinationHeader);
        destinationHeader.Print(std::cout);
    }
}

#endif // MASTER_HPP_INCLUDE
