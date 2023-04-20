# CN_CHomeworks_2

- [CN\_CHomeworks\_2](#cn_chomeworks_2)
  - [Introduction](#introduction)
  - [Project Description](#project-description)
  - [Definitions](#definitions)
    - [IEEE 802.11](#ieee-80211)
    - [ns-3](#ns-3)
  - [Project Structure](#project-structure)
    - [header.hpp](#headerhpp)
    - [network.hpp](#networkhpp)
    - [constants.hpp](#constantshpp)
    - [client.hpp](#clienthpp)
    - [master.hpp](#masterhpp)
    - [mapper.hpp](#mapperhpp)
    - [utils.hpp](#utilshpp)
  - [ns-3 Simulation](#ns-3-simulation)
    - [Sockets](#sockets)
    - [Accept Callback](#accept-callback)
    - [Receive Callback](#receive-callback)

## Introduction

This is a project for the Computer Networks course at the University of Tehran, Electrical and Computer Engineering department in Spring 2023.  
The project focuses on network simulation and is written in C++.  
The [ns-3](https://www.nsnam.org/) library is used for simulation.

## Project Description

In this project, there are three types of nodes:

- Client Node
- Central Node
- Mapper Node

![network](assets/network.png)

The **client node** sends data *(which is a number between 0 and 25)* to the central node using a UDP connection.  
The **central node** sends the data to all 3 mappers using a TCP connection.  
The **mapper nodes** each have a map from a number to a character. The 26 numbers are partitioned into the 3 mappers and the numbers that each map are different from the other ones. After mapping, they send the character back to the client using UDP.

## Definitions

### IEEE 802.11

The IEEE 802.11 is a standard for wireless local area networks (WLAN) more commonly known as **Wi-Fi**.  
Some parts of the specification include:

- **MAC** *(Media Access Control)*: Each device is assigned a unique MAC address which is used to identify the device on the network.  
- **PHY** *(Physical Layer)*: The PHY layer of Wi-Fi operates in frequency bands of 2.4GHz or 5GHz. The data transfer rate, channel width, and other physical aspects are also specified.
- **AP** *(Access Points)*: APs are devices that create the wireless network and provide connectivity to other devices.
- **NICs** *(Wireless Network Interface Cards)*: These are the components in a device (such as a PC) that allow communication between a device and an access point.
- **Security**: Some protocols such as WPA2 are used for AP connection authorization and secure data transfer.

Wi-Fi is the most widespread wireless networking standard and many devices support it.

### ns-3

**ns-3** is an open-source discrete-event network simulator.  
ns-3 allows users to build and model network entities (links, nodes, devices, applications), use network protocols (IP, TCP, UDP, Wi-Fi) and simulate them in a customized environment.

## Project Structure

The given sample file has been split into 7 header files:

```text
header.hpp
network.hpp
constants.hpp
client.hpp
master.hpp
mapper.hpp
utils.hpp
```

### header.hpp

Two headers are used.

- `ClientHeader`: This header contains the client ip:port and the 16-bit data.
- `MapperHeader`: This header contains the mapped result of the client's data in *char* format.

### network.hpp

This file contains the `Network` class which specifies and runs the simulation.  
A `NodeContainer` represents a node on the network. There are 3 node types and therefore three of these variables in the class field.  
The `NodeContainer.Create(NUM)` method creates *NUM* instances of the node type.  
Using various helper classes such as `YansWifiChannelHelper`, `YansWifiPhyHelper`, and `WifiMacHelper`, the MAC types are set (`StaWifiMac` for client and mapper, `ApWifiMac` for the server) and then is installed into the nodes' respective `NetDeviceContainer` instances.

An `Ipv4AddressHelper` is used to set the network and host portions of the wanted IP address (this is explained further in **constants.hpp**).  
An `Ipv4InterfaceContainer` represents an IP address. Each node is assigned an IP address using the AddressHelper.  

Now instances of applications (1 client, 1 server, and 3 mappers) are made and ran.  
The simulation begins with all applications starting from the first second and ending after 10 seconds.

### constants.hpp

This file contains constant variables.  
They are separated to different categories with comments.  
Some of the constants are as follows:

- `CLIENT/MASTER_HEADER_LENGTH`: This is the length of the packet headers sent by the client and master.
- `CLIENT/MASTER_PORT`: This is the port used by the client and master.
- `MAPPERS_COUNT` & `MAPPERS_PORTS`: These are the count of mapper nodes and their respective ports.
- `BASE_ADDRESS` & `NET_MASK`: This is used to assign IP addresses to the nodes.
- `RANDOM_DATA`: Sets whether the client should sent random data or `consts::MESSAGE`.
- `SHUFFLE_MAPPINGS`: Use a random mapping.
- `BURSTY_DATA`: Makes the client not wait until `consts::TIMEOUT` to send the next data.
- The options for the grid.

An IP address is made of the network portion and the host portion.  
The network portion determines which network the device belongs to.  
The host portion identifies a specific device on the network.  
The net mask determines the size of the hosts that can be on a network.  
For example, with a net mask of `255.255.255.0`, the first 8 bits is the host portion.

### client.hpp

The client is a simulated application which creates two UDP sockets on start.  
One socket connects to the master and one receives data from mappers.

Data is generated either randomly *(RANDOM_DATA)* or by circularly iterating the given input vector.  
The data is packed into a `ClientHeader` and is sent to the master.

For sending the next data, two modes can be used: bursty or waiting.  
In bursty mode *(BURSTY_DATA)*, the client will send data each `BURSTY_DATA_SEND_INTERVAL` seconds.  
In waiting mode, the client will wait `TIMEOUT` seconds until it receives the mapping result and then sends the next data.

Upon receiving a map result, the character is printed and appended to a string which is printed at the end of the simulation.

### master.hpp

The master application creates 4 sockets:

- 1 UDP socket for receiving the data from the client.
- 3 TCP sockets for sending the data to each mapper.

The TCP sockets forward the same received packet from the UDP socket.

### mapper.hpp

The mapper uses 2 sockets.

- A TCP socket which is listened on to accept incoming master connections and receiving the data.
- A UDP socket to send the mapped data result to the client.

The mapper uses the given map instance to map the received data. If no such entry exists in the map, no action is taken. Else, a UDP socket is created which connects to the ip:port received in the packet header and sends the result.

### utils.hpp

This file contains some helper functions such as `shuffle` and `partitionMappings` which takes a mapping (which is a string with each index being mapped to the character) and partitions it between the number of mappers.

## ns-3 Simulation

Here, some parts of the library are explained:

### Sockets

```cpp
Ptr<Socket> socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId()); // or UdpSocketFactory
```

This creates a UDP/TCP socket.  
The sockets need to be bound to an ip:port. This is done using the `Bind` method:

```cpp
InetSocketAddress addr(ip.GetAddress(0), port);
socket->Bind(addr);
```

We can now `socket->Listen()` (TCP only) or `socket->Connect(addr)` depending on the use case.

The `SetRecvCallback` method is used to handle when something is sent (`socket->Send(packet)`) to the socket:

```cpp
socket->SetRecvCallback(MakeCallback(&Class::HandleRead, this));
```

The application class' `HandleRead` method is called whenever something is received on `socket`.

The `SetAcceptCallback` method is used on a listening socket (TCP) to accept connections and handle them:

```cpp
socket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address&>(),
                          MakeCallback(&Class::HandleAccept, this));
```

The application class' `HandleAccept` is called whenever someone connects to us.

### Accept Callback

```cpp
void Class::HandleAccept(Ptr<Socket> socket, const Address& from) {
    socket->SetRecvCallback(MakeCallback(&Class::HandleRead, this));
}
```

The accept handler takes the socket to the connector and the address of it as input parameters.  
Here, the receive callback from the accepted socket is set.

### Receive Callback

```cpp
void Class::HandleRead(Ptr<Socket> socket) {
    Ptr<Packet> packet;

    while (packet = socket->Recv()) {
        if (packet->GetSize() == 0) {
            break;
        }

        Header header;
        packet->RemoveHeader(header);

        auto x = header.GetX();
        // ...
    }
}
```

This is how received packets are handled. Wanted data should be in the header and it is extracted from the packet.
