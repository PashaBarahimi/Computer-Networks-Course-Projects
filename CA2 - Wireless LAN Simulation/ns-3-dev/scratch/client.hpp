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
    Client(uint16_t port, Ipv4InterfaceContainer& ip);
    virtual ~Client() = default;

private:
    virtual void StartApplication(void);

    uint16_t port;
    Ptr<Socket> socket;
    Ipv4InterfaceContainer ip;
};

Client::Client(uint16_t port, Ipv4InterfaceContainer& ip) : port(port), ip(ip) {
    std::srand(time(nullptr));
}

static void GenerateTraffic(Ptr<Socket> socket, uint16_t data) {
    Ptr<Packet> packet = new Packet();
    MisashaHeader m;
    m.SetData(data);

    packet->AddHeader(m);
    packet->Print(std::cout);
    socket->Send(packet);

    Simulator::Schedule(Seconds(consts::TRAFFIC_GENERATION_TIME_INTERVAL), &GenerateTraffic, socket, rand() % 26);
}

void Client::StartApplication(void) {
    Ptr<Socket> sock = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress sockAddr(ip.GetAddress(0), port);
    sock->Connect(sockAddr);

    GenerateTraffic(sock, 0);
}

#endif // CLIENT_HPP_INCLUDE
