#ifndef NETWORK_HPP_INCLUDE
#define NETWORK_HPP_INCLUDE

#include <array>
#include <ctime>
#include <string>
#include <unordered_map>
#include <vector>

#include "client.hpp"
#include "constants.hpp"
#include "mapper.hpp"
#include "master.hpp"
#include "ns3/core-module.h"
#include "ns3/error-model.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ssid.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiTopology");

class Network {
public:
    Network(bool verbose, const std::vector<uint16_t>& clientSendData, const std::vector<std::unordered_map<uint16_t, char>>& mappings);
    ~Network() = default;

    void Simulate();

private:
    std::vector<uint16_t> clientSendData_;
    std::vector<std::unordered_map<uint16_t, char>> mappings_;
    NodeContainer wifiStaNodeClient_, wifiStaNodeMaster_, wifiStaNodeMapper_;
    YansWifiChannelHelper channel_;
    YansWifiPhyHelper phy_;
    WifiHelper wifi_;
    WifiMacHelper mac_;
    Ssid ssid_;
    NetDeviceContainer staDeviceClient_, staDeviceMaster_, staDeviceMapper_;
    MobilityHelper mobility_;
    InternetStackHelper stack_;
    Ipv4AddressHelper address_;
    Ipv4InterfaceContainer staNodeClientInterface_, staNodeMasterInterface_, staNodeMapperInterface_;
    Ptr<Client> clientApp_;
    Ptr<Master> masterApp_;
    std::array<Ptr<Mapper>, consts::MAPPERS_COUNT> mapperApps_;

    void Setup();
    void SetupWifi();
    void SetupNodes();

    static void DelayAndThroughputMonitor(FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> flowMon, double em);
};

Network::Network(bool verbose, const std::vector<uint16_t>& clientSendData, const std::vector<std::unordered_map<uint16_t, char>>& mappings)
    : clientSendData_(clientSendData),
      mappings_(mappings),
      channel_(YansWifiChannelHelper::Default()),
      ssid_(Ssid(consts::SSID)) {
    std::srand(time(nullptr));
    if (verbose) {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
    Setup();
}

void Network::Setup() {
    SetupWifi();
    SetupNodes();
}

void Network::SetupWifi() {
    wifiStaNodeClient_.Create(1);
    wifiStaNodeMaster_.Create(1);
    wifiStaNodeMapper_.Create(consts::MAPPERS_COUNT);

    phy_.SetChannel(channel_.Create());
    wifi_.SetRemoteStationManager("ns3::AarfWifiManager");

    mac_.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid_), "ActiveProbing", BooleanValue(false));
    staDeviceClient_ = wifi_.Install(phy_, mac_, wifiStaNodeClient_);
    mac_.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid_), "ActiveProbing", BooleanValue(false));
    staDeviceMapper_ = wifi_.Install(phy_, mac_, wifiStaNodeMapper_);
    mac_.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid_));
    staDeviceMaster_ = wifi_.Install(phy_, mac_, wifiStaNodeMaster_);

    phy_.SetErrorRateModel("ns3::YansErrorRateModel");
    mobility_.SetPositionAllocator("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue(0.0),
                                   "MinY", DoubleValue(0.0),
                                   "DeltaX", DoubleValue(consts::DELTA_X),
                                   "DeltaY", DoubleValue(consts::DELTA_Y),
                                   "GridWidth", UintegerValue(consts::GRID_WIDTH),
                                   "LayoutType", StringValue("RowFirst"));

    mobility_.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds",
                               RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility_.Install(wifiStaNodeClient_);
    mobility_.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility_.Install(wifiStaNodeMapper_);
    mobility_.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility_.Install(wifiStaNodeMaster_);

    stack_.Install(wifiStaNodeClient_);
    stack_.Install(wifiStaNodeMaster_);
    stack_.Install(wifiStaNodeMapper_);
}

void Network::SetupNodes() {
    address_.SetBase(consts::BASE_ADDRESS, consts::NET_MASK);
    staNodeClientInterface_ = address_.Assign(staDeviceClient_);
    staNodeMasterInterface_ = address_.Assign(staDeviceMaster_);
    staNodeMapperInterface_ = address_.Assign(staDeviceMapper_);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    clientApp_ = CreateObject<Client>(consts::CLIENT_PORT, staNodeClientInterface_, consts::MASTER_PORT, staNodeMasterInterface_, clientSendData_);
    wifiStaNodeClient_.Get(0)->AddApplication(clientApp_);

    masterApp_ = CreateObject<Master>(consts::MASTER_PORT, staNodeMasterInterface_, consts::MAPPER_PORTS, staNodeMapperInterface_);
    wifiStaNodeMaster_.Get(0)->AddApplication(masterApp_);

    for (unsigned i = 0; i < consts::MAPPERS_COUNT; ++i) {
        mapperApps_[i] = CreateObject<Mapper>(consts::MAPPER_PORTS[i], staNodeMapperInterface_, i, mappings_[i]);
        wifiStaNodeMapper_.Get(i)->AddApplication(mapperApps_[i]);
    }
}

void Network::Simulate() {
    clientApp_->SetStartTime(Seconds(0.0));
    clientApp_->SetStopTime(Seconds(consts::DURATION));

    masterApp_->SetStartTime(Seconds(0.0));
    masterApp_->SetStopTime(Seconds(consts::DURATION));

    for (unsigned i = 0; i < consts::MAPPERS_COUNT; ++i) {
        mapperApps_[i]->SetStartTime(Seconds(0.0));
        mapperApps_[i]->SetStopTime(Seconds(consts::DURATION));
    }

    NS_LOG_INFO("Run Simulation");

    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();
    DelayAndThroughputMonitor(&flowHelper, flowMonitor, consts::ERROR);

    Simulator::Stop(Seconds(consts::SIMULATION_DURATION));
    Simulator::Run();
}

void Network::DelayAndThroughputMonitor(FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> flowMon, double em) {
    uint16_t i = 0;
    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
    Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier>(fmhelper->GetClassifier());

    for (auto stats = flowStats.begin(); stats != flowStats.end(); ++stats) {
        std::cout << "---------------------------------------------------------------------------"
                  << std::endl;

        Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow(stats->first);

        std::cout << "Flow ID               : " << stats->first << " ; "
                  << fiveTuple.sourceAddress << " -----> " << fiveTuple.destinationAddress
                  << std::endl;
        std::cout << "Tx Packets            : " << stats->second.txPackets << std::endl;
        std::cout << "Rx Packets            : " << stats->second.rxPackets << std::endl;
        std::cout << "Duration              : "
                  << (stats->second.timeLastRxPacket.GetSeconds() -
                      stats->second.timeFirstTxPacket.GetSeconds())
                  << std::endl;
        std::cout << "Last Received Packet  : " << stats->second.timeLastRxPacket.GetSeconds()
                  << " Seconds" << std::endl;
        std::cout << "Throughput            : "
                  << stats->second.rxBytes * 8.0 /
                         (stats->second.timeLastRxPacket.GetSeconds() -
                          stats->second.timeFirstTxPacket.GetSeconds()) /
                         1024 / 1024
                  << " Mbps" << std::endl;
        std::cout << "Sum of e2e Delay      : " << stats->second.delaySum.GetSeconds() << "s"
                  << std::endl;
        std::cout << "Average of e2e Delay  : "
                  << stats->second.delaySum.GetSeconds() / stats->second.rxPackets << "s"
                  << std::endl;

        i++;

        std::cout << "---------------------------------------------------------------------------"
                  << std::endl;
    }

    Simulator::Schedule(Seconds(consts::MONITOR_TIME_INTERVAL), &DelayAndThroughputMonitor, fmhelper, flowMon, em);
}

#endif // NETWORK_HPP_INCLUDE
