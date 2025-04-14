#include "server.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <regex>

namespace dhcp {

std::string DhcpPacket::getMac() const {
    std::stringstream ss;
    for (int i = 0; i < 6; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)chaddr[i];
        if (i < 5) ss << ":";
    }
    return ss.str();
}

void DhcpPacket::setMac(const std::string& mac) {
    std::stringstream ss(mac);
    for (int i = 0; i < 6; ++i) {
        std::string byte;
        std::getline(ss, byte, ':');
        chaddr[i] = std::stoi(byte, nullptr, 16);
    }
}

std::string DhcpPacket::getIp() const {
    struct in_addr addr;
    addr.s_addr = yiaddr;
    return inet_ntoa(addr);
}

void DhcpPacket::setIp(const std::string& ip) {
    yiaddr = inet_addr(ip.c_str());
}

DhcpMessageType DhcpPacket::getMessageType() const {
    const uint8_t* ptr = options;
    if (ptr[0] != 0x63 || ptr[1] != 0x82 || ptr[2] != 0x53 || ptr[3] != 0x63) {
        return DhcpMessageType::DHCPDISCOVER; // По умолчанию
    }
    ptr += 4; // Пропускаем magic cookie
    while (*ptr != 0xFF) { // 0xFF - конец опций
        uint8_t opt = *ptr++;
        uint8_t len = *ptr++;
        if (opt == 53) { // DHCP Message Type
            return static_cast<DhcpMessageType>(*ptr);
        }
        ptr += len;
    }
    return DhcpMessageType::DHCPDISCOVER;
}

void DhcpPacket::setMessageType(DhcpMessageType type) {
    // Magic cookie
    options[0] = 0x63;
    options[1] = 0x82;
    options[2] = 0x53;
    options[3] = 0x63;
    // Опция 53: DHCP Message Type
    options[4] = 53;
    options[5] = 1; // Длина
    options[6] = static_cast<uint8_t>(type);
    // Конец опций
    options[7] = 0xFF;
}

void DhcpPacket::setLeaseTime(uint32_t leaseTime) {
    uint8_t* ptr = options + 7; // После Message Type
    ptr[0] = 51; // IP Address Lease Time
    ptr[1] = 4;  // Длина
    ptr[2] = (leaseTime >> 24) & 0xFF;
    ptr[3] = (leaseTime >> 16) & 0xFF;
    ptr[4] = (leaseTime >> 8) & 0xFF;
    ptr[5] = leaseTime & 0xFF;
    ptr[6] = 0xFF; // Конец опций
}

bool DhcpPacket::isValidMac(const std::string& mac) {
    std::regex macRegex("^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$");
    return std::regex_match(mac, macRegex);
}

void DhcpPacket::serialize(uint8_t* buffer, size_t& len) const {
    std::memcpy(buffer, this, sizeof(DhcpPacket));
    len = sizeof(DhcpPacket);
}

DhcpPacket DhcpPacket::deserialize(const uint8_t* buffer, size_t len, bool& success) {
    success = false;
    DhcpPacket packet = {};
    if (len < sizeof(DhcpPacket)) return packet;
    std::memcpy(&packet, buffer, sizeof(DhcpPacket));
    if (packet.htype != 1 || packet.hlen != 6) return packet;
    if (packet.op != 1 && packet.op != 2) return packet;
    success = true;
    return packet;
}

IpPool::IpPool() {
    for (int i = IP_RANGE_START; i <= IP_RANGE_END; ++i) {
        ipPool.push_back(std::string(BASE_IP) + std::to_string(i));
    }
}

std::string IpPool::allocateIp(const std::string& mac) {
    auto it = allocatedIps.find(mac);
    if (it != allocatedIps.end()) {
        return it->second;
    }

    for (const auto& ip : ipPool) {
        if (isIpAvailable(ip)) {
            allocatedIps[mac] = ip;
            return ip;
        }
    }
    return "";
}

bool IpPool::confirmIp(const std::string& mac, const std::string& ip) {
    auto it = allocatedIps.find(mac);
    if (it != allocatedIps.end() && it->second == ip && isIpInPool(ip)) {
        return true;
    }
    return false;
}

bool IpPool::isIpAvailable(const std::string& ip) const {
    for (const auto& pair : allocatedIps) {
        if (pair.second == ip) {
            return false;
        }
    }
    return isIpInPool(ip);
}

bool IpPool::isIpInPool(const std::string& ip) const {
    for (const auto& poolIp : ipPool) {
        if (poolIp == ip) {
            return true;
        }
    }
    return false;
}

DHCPServer::DHCPServer() : running(true) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int broadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        close(sockfd);
        throw std::runtime_error("Failed to set broadcast option");
    }

    struct timeval tv;
    tv.tv_sec = SOCKET_TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        close(sockfd);
        throw std::runtime_error("Failed to set socket timeout");
    }

    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(sockfd);
        throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
    }

    log("DHCP Server initialized on port " + std::to_string(SERVER_PORT));
}

DHCPServer::~DHCPServer() {
    stop();
}

void DHCPServer::start() {
    uint8_t buffer[MAX_MSG_SIZE];
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    while (running) {
        std::memset(buffer, 0, MAX_MSG_SIZE);
        ssize_t bytesReceived = recvfrom(sockfd, buffer, MAX_MSG_SIZE, 0,
                                        (struct sockaddr*)&clientAddr, &addrLen);
        if (bytesReceived < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            log("Error receiving packet: " + std::string(strerror(errno)));
            continue;
        }

        bool success = false;
        DhcpPacket packet = DhcpPacket::deserialize(buffer, bytesReceived, success);
        if (!success) {
            log("Invalid packet received from " + std::string(inet_ntoa(clientAddr.sin_addr)));
            continue;
        }

        std::string mac = packet.getMac();
        auto msgType = packet.getMessageType();
        log("Received " + std::string(msgType == DhcpMessageType::DHCPDISCOVER ? "DHCPDISCOVER" : "DHCPREQUEST") +
            " from MAC: " + mac + " at " + inet_ntoa(clientAddr.sin_addr));
        handlePacket(packet, clientAddr);
    }

    log("DHCP Server shutting down");
}

void DHCPServer::stop() {
    running = false;
    if (sockfd >= 0) {
        close(sockfd);
        sockfd = -1;
        log("DHCP Server socket closed");
    }
}

bool DHCPServer::shouldStop() const {
    return !running;
}

void DHCPServer::handlePacket(const DhcpPacket& packet, const struct sockaddr_in& clientAddr) {
    DhcpPacket response;
    auto msgType = packet.getMessageType();
    if (msgType == DhcpMessageType::DHCPDISCOVER) {
        response = handleDiscover(packet);
    } else if (msgType == DhcpMessageType::DHCPREQUEST) {
        response = handleRequest(packet);
    } else {
        log("Ignoring unknown message type from MAC: " + packet.getMac());
        return;
    }

    if (!response.getIp().empty()) {
        sendPacket(response, clientAddr);
    } else {
        log("No response sent for MAC: " + packet.getMac() + " (no IP available)");
    }
}

DhcpPacket DHCPServer::handleDiscover(const DhcpPacket& packet) {
    DhcpPacket response = {};
    response.op = 2; // BOOTREPLY
    response.htype = 1;
    response.hlen = 6;
    response.xid = packet.xid;
    response.setMac(packet.getMac());
    response.setMessageType(DhcpMessageType::DHCPOFFER);
    response.setIp(ipPool.allocateIp(packet.getMac()));
    response.setLeaseTime(DEFAULT_LEASE_TIME);

    if (response.getIp().empty()) {
        log("No available IPs for MAC: " + packet.getMac());
    } else {
        log("Offering IP: " + response.getIp() + " to MAC: " + packet.getMac());
    }

    return response;
}

DhcpPacket DHCPServer::handleRequest(const DhcpPacket& packet) {
    DhcpPacket response = {};
    response.op = 2; // BOOTREPLY
    response.htype = 1;
    response.hlen = 6;
    response.xid = packet.xid;
    response.setMac(packet.getMac());
    response.setMessageType(DhcpMessageType::DHCPACK);
    response.setIp(packet.getIp());

    if (!ipPool.isIpInPool(packet.getIp())) {
        log("Requested IP " + packet.getIp() + " is not in pool for MAC: " + packet.getMac());
        response.setIp("");
        return response;
    }

    if (ipPool.confirmIp(packet.getMac(), packet.getIp())) {
        log("Confirmed IP: " + packet.getIp() + " for MAC: " + packet.getMac());
        response.setLeaseTime(DEFAULT_LEASE_TIME);
    } else {
        log("Invalid REQUEST for IP: " + packet.getIp() + ", MAC: " + packet.getMac());
        response.setIp("");
    }

    return response;
}

void DHCPServer::sendPacket(const DhcpPacket& packet, const struct sockaddr_in& clientAddr) {
    uint8_t buffer[MAX_MSG_SIZE];
    size_t len = 0;
    packet.serialize(buffer, len);

    struct sockaddr_in destAddr = clientAddr;
    destAddr.sin_port = htons(CLIENT_PORT);

    ssize_t bytesSent = sendto(sockfd, buffer, len, 0,
                               (struct sockaddr*)&destAddr, sizeof(destAddr));
    if (bytesSent < 0) {
        log("Error sending packet to MAC: " + packet.getMac() + ": " + strerror(errno));
    } else {
        log("Sent " + std::string(packet.getMessageType() == DhcpMessageType::DHCPOFFER ? "DHCPOFFER" : "DHCPACK") +
            " with IP: " + packet.getIp() + " to MAC: " + packet.getMac());
    }
}

void DHCPServer::log(const std::string& message) const {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S] ") << message;
    std::cout << oss.str() << std::endl;
}

} // namespace dhcp