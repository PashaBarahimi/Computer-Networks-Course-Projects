#ifndef NETWORK_HPP_INCLUDE
#define NETWORK_HPP_INCLUDE

#include <ctime>
#include <string>

#include "client.hpp"
#include "constants.hpp"
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
    Network(bool verbose);
    ~Network() = default;

    void Simulate();

private:
    NodeContainer wifiStaNodeClient_;
    NodeContainer wifiStaNodeMaster_;
    YansWifiChannelHelper channel_;
    YansWifiPhyHelper phy_;
    WifiHelper wifi_;
    WifiMacHelper mac_;
    Ssid ssid_;
    NetDeviceContainer staDeviceClient_;
    NetDeviceContainer staDeviceMaster_;
    MobilityHelper mobility_;
    InternetStackHelper stack_;
    Ipv4AddressHelper address_;
    Ipv4InterfaceContainer staNodeClientInterface_;
    Ipv4InterfaceContainer staNodesMasterInterface_;
    Ptr<Client> clientApp_;
    Ptr<Master> masterApp_;

    void Setup();

    static void ThroughputMonitor(FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> flowMon, double em);
    static void AverageDelayMonitor(FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> flowMon, double em);
};

Network::Network(bool verbose)
    : channel_(YansWifiChannelHelper::Default()),
      ssid_(Ssid(consts::SSID)) {
    std::srand(time(nullptr));
    if (verbose) {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
    Setup();
}

void Network::Setup() {
    wifiStaNodeClient_.Create(1);
    wifiStaNodeMaster_.Create(1);
    phy_.SetChannel(channel_.Create());
    wifi_.SetRemoteStationManager("ns3::AarfWifiManager");
    mac_.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid_), "ActiveProbing", BooleanValue(false));
    staDeviceClient_ = wifi_.Install(phy_, mac_, wifiStaNodeClient_);
    mac_.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid_));
    staDeviceMaster_ = wifi_.Install(phy_, mac_, wifiStaNodeMaster_);
    mac_.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid_), "ActiveProbing", BooleanValue(false));
    phy_.SetErrorRateModel("ns3::YansErrorRateModel");
    mobility_.SetPositionAllocator("ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY",
                                   DoubleValue(0.0), "DeltaX", DoubleValue(5.0), "DeltaY",
                                   DoubleValue(10.0), "GridWidth", UintegerValue(3), "LayoutType",
                                   StringValue("RowFirst"));

    mobility_.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds",
                               RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility_.Install(wifiStaNodeClient_);

    mobility_.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility_.Install(wifiStaNodeMaster_);
    stack_.Install(wifiStaNodeClient_);
    stack_.Install(wifiStaNodeMaster_);

    address_.SetBase(consts::BASE_ADDRESS, consts::NET_MASK);
    staNodeClientInterface_ = address_.Assign(staDeviceClient_);
    staNodesMasterInterface_ = address_.Assign(staDeviceMaster_);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    clientApp_ = CreateObject<Client>(consts::CLIENT_PORT, staNodesMasterInterface_); // mind the port and interface
    wifiStaNodeClient_.Get(0)->AddApplication(clientApp_);
    masterApp_ = CreateObject<Master>(consts::MASTER_PORT, staNodesMasterInterface_);
    wifiStaNodeMaster_.Get(0)->AddApplication(masterApp_);
}

void Network::Simulate() {
    clientApp_->SetStartTime(Seconds(0.0));
    clientApp_->SetStopTime(Seconds(consts::DURATION));

    masterApp_->SetStartTime(Seconds(0.0));
    masterApp_->SetStopTime(Seconds(consts::DURATION));

    NS_LOG_INFO("Run Simulation");

    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    ThroughputMonitor(&flowHelper, flowMonitor, consts::ERROR);
    AverageDelayMonitor(&flowHelper, flowMonitor, consts::ERROR);

    Simulator::Stop(Seconds(consts::DURATION));
    Simulator::Run();
}

void Network::ThroughputMonitor(FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> flowMon, double em) {
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

        i++;

        std::cout << "---------------------------------------------------------------------------"
                  << std::endl;
    }

    Simulator::Schedule(Seconds(consts::MONITOR_TIME_INTERVAL), &ThroughputMonitor, fmhelper, flowMon, em);
}

void Network::AverageDelayMonitor(FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> flowMon, double em) {
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
        std::cout << "Sum of e2e Delay      : " << stats->second.delaySum.GetSeconds() << "s"
                  << std::endl;
        std::cout << "Average of e2e Delay  : "
                  << stats->second.delaySum.GetSeconds() / stats->second.rxPackets << "s"
                  << std::endl;

        i++;

        std::cout << "---------------------------------------------------------------------------"
                  << std::endl;
    }

    Simulator::Schedule(Seconds(consts::MONITOR_TIME_INTERVAL), &AverageDelayMonitor, fmhelper, flowMon, em);
}

#endif // NETWORK_HPP_INCLUDE
