#ifndef CLIENT_HPP_INCLUDE
#define CLIENT_HPP_INCLUDE

#include <ctime>

#include "constants.hpp"
#include "header.hpp"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

class Client : public Application {
public:
    Client(uint16_t masterPort, Ipv4InterfaceContainer& ip);
    virtual ~Client() = default;

private:
    static void GenerateTraffic(Ptr<Socket> socket, uint16_t data);
    void StartApplication(void) override;

    uint16_t masterPort_;
    Ptr<Socket> socket_;
    Ipv4InterfaceContainer masterIp_;
};

Client::Client(uint16_t masterPort, Ipv4InterfaceContainer& ip) : masterPort_(masterPort), masterIp_(ip) {
    std::srand(time(nullptr));
}

void Client::GenerateTraffic(Ptr<Socket> socket, uint16_t data) {
    Ptr<Packet> packet = new Packet();
    ClientHeader m;
    m.SetData(data);

    packet->AddHeader(m);
    packet->Print(std::cout);
    socket->Send(packet);

    Simulator::Schedule(Seconds(consts::TRAFFIC_GENERATION_TIME_INTERVAL), &GenerateTraffic, socket, rand() % 26);
}

void Client::StartApplication(void) {
    socket_ = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress sockAddr(masterIp_.GetAddress(0), masterPort_);
    socket_->Connect(sockAddr);

    GenerateTraffic(socket_, 0);
}

#endif // CLIENT_HPP_INCLUDE
