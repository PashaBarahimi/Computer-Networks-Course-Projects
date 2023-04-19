#ifndef CLIENT_HPP_INCLUDE
#define CLIENT_HPP_INCLUDE

#include <ctime>
#include <vector>

#include "constants.hpp"
#include "header.hpp"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

class Client : public Application {
public:
    Client(uint16_t port, Ipv4InterfaceContainer& ip,
           uint16_t masterPort, Ipv4InterfaceContainer& masterIp,
           const std::vector<uint16_t>& data);
    virtual ~Client();

private:
    static void GenerateTraffic(Client* client, uint16_t data);
    static void Timeout(Client* client);
    void HandleRead(Ptr<Socket> socket);
    void StartApplication(void) override;
    // void StopApplication(void) override;
    uint16_t GetData();

    uint16_t port_;
    Ipv4InterfaceContainer ip_;
    uint16_t masterPort_;
    Ipv4InterfaceContainer masterIp_;
    Ptr<Socket> masterSocket_;
    Ptr<Socket> mapperSocket_;
    EventId timeoutEvent_;
    std::string dataReceived_;
    std::vector<uint16_t> data_;
    int dataIdx_ = 0;
};

Client::Client(uint16_t port, Ipv4InterfaceContainer& ip,
               uint16_t masterPort, Ipv4InterfaceContainer& masterIp,
               const std::vector<uint16_t>& data)
    : port_(port),
      ip_(ip),
      masterPort_(masterPort),
      masterIp_(masterIp),
      data_(data) {
    std::srand(time(nullptr));
}

Client::~Client() {
    std::cout << "Data Received: " << std::endl;
    std::cout << dataReceived_ << std::endl;
}

void Client::StartApplication(void) {
    masterSocket_ = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress sockAddr(masterIp_.GetAddress(0), masterPort_);
    masterSocket_->Connect(sockAddr);

    mapperSocket_ = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress local(ip_.GetAddress(0), port_);
    mapperSocket_->Bind(local);
    mapperSocket_->SetRecvCallback(MakeCallback(&Client::HandleRead, this));

    GenerateTraffic(this, GetData());
}

// void Client::StopApplication(void) {
//     std::cout << "Data Received: " << std::endl;
//     std::cout << dataReceived_ << std::endl;
// }

void Client::GenerateTraffic(Client* client, uint16_t data) {
    Ptr<Packet> packet = Create<Packet>();
    ClientHeader header;
    header.SetData(data);
    header.SetSource(client->ip_.GetAddress(0), client->port_);

    packet->AddHeader(header);
    client->masterSocket_->Send(packet);

    if (consts::BURSTY_DATA)
        Simulator::Schedule(Seconds(consts::BURSTY_DATA_SEND_INTERVAL), &Client::GenerateTraffic, client, client->GetData());
    else
        client->timeoutEvent_ = Simulator::Schedule(Seconds(consts::TIMEOUT), &Client::Timeout, client);
}

uint16_t Client::GetData() {
    if (consts::RANDOM_DATA)
        return std::rand() % consts::VALID_CHARACTERS.size();
    uint16_t data = data_[dataIdx_];
    dataIdx_ = (dataIdx_ + 1) % data_.size();
    return data;
}

void Client::HandleRead(Ptr<Socket> socket) {
    Ptr<Packet> packet;

    while (packet = socket->Recv()) {
        if (packet->GetSize() == 0) {
            break;
        }

        MapperHeader header;
        packet->RemoveHeader(header);

        dataReceived_ += header.GetData();
        std::cout << "Received: " << header.GetData() << std::endl;
        // header.Print(std::cout);
    }

    if (!consts::BURSTY_DATA) {
        Simulator::Cancel(timeoutEvent_);
        GenerateTraffic(this, GetData());
    }
}

void Client::Timeout(Client* client) {
    // std::cout << "Timed out waiting for response at " << Simulator::Now().GetSeconds() << std::endl;
    GenerateTraffic(client, client->GetData());
}

#endif // CLIENT_HPP_INCLUDE
