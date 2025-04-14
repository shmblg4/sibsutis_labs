#include "client.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <random>
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
        return DhcpMessageType::DHCPDISCOVER;
    }
    ptr += 4;
    while (*ptr != 0xFF) {
        uint8_t opt = *ptr++;
        uint8_t len = *ptr++;
        if (opt == 53) {
            return static_cast<DhcpMessageType>(*ptr);
        }
        ptr += len;
    }
    return DhcpMessageType::DHCPDISCOVER;
}

void DhcpPacket::setMessageType(DhcpMessageType type) {
    options[0] = 0x63;
    options[1] = 0x82;
    options[2] = 0x53;
    options[3] = 0x63;
    options[4] = 53;
    options[5] = 1;
    options[6] = static_cast<uint8_t>(type);
    options[7] = 0xFF;
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

DHCPClient::DHCPClient() : running(true) {
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
    tv.tv_sec = RESPONSE_TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        close(sockfd);
        throw std::runtime_error("Failed to set socket timeout");
    }

    std::memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = INADDR_ANY;
    clientAddr.sin_port = htons(CLIENT_PORT);

    if (bind(sockfd, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) < 0) {
        close(sockfd);
        throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
    }

    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    serverAddr.sin_port = htons(SERVER_PORT);

    mac = generateMac();
    xid = rand();

    log("DHCP Client initialized with MAC: " + mac);
}

DHCPClient::~DHCPClient() {
    stop();
}

void DHCPClient::start() {
    if (!running) {
        log("Client is not running");
        return;
    }

    sendDiscover();

    DhcpPacket offer;
    if (handleOffer(offer)) {
        sendRequest(offer);

        DhcpPacket ack;
        if (handleAck(ack)) {
            log("Successfully acquired IP: " + ack.getIp());
        } else {
            log("Failed to receive valid DHCPACK");
        }
    } else {
        log("Failed to receive valid DHCPOFFER");
    }
}

void DHCPClient::stop() {
    running = false;
    if (sockfd >= 0) {
        close(sockfd);
        sockfd = -1;
        log("DHCP Client socket closed");
    }
}

bool DHCPClient::shouldStop() const {
    return !running;
}

void DHCPClient::sendDiscover() {
    DhcpPacket packet = {};
    packet.op = 1; // BOOTREQUEST
    packet.htype = 1;
    packet.hlen = 6;
    packet.xid = xid;
    packet.setMac(mac);
    packet.setMessageType(DhcpMessageType::DHCPDISCOVER);

    uint8_t buffer[MAX_MSG_SIZE];
    size_t len = 0;
    packet.serialize(buffer, len);

    ssize_t bytesSent = sendto(sockfd, buffer, len, 0,
                               (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (bytesSent < 0) {
        log("Error sending DHCPDISCOVER: " + std::string(strerror(errno)));
    } else {
        log("Sent DHCPDISCOVER");
    }
}

bool DHCPClient::handleOffer(DhcpPacket& offer) {
    struct sockaddr_in fromAddr;
    if (!receivePacket(offer, fromAddr)) {
        return false;
    }

    if (offer.getMessageType() != DhcpMessageType::DHCPOFFER || offer.getMac() != mac || offer.xid != xid) {
        log("Invalid DHCPOFFER received");
        return false;
    }

    log("Received DHCPOFFER with IP: " + offer.getIp());
    return true;
}

void DHCPClient::sendRequest(const DhcpPacket& offer) {
    DhcpPacket packet = {};
    packet.op = 1;
    packet.htype = 1;
    packet.hlen = 6;
    packet.xid = xid;
    packet.setMac(mac);
    packet.setIp(offer.getIp());
    packet.setMessageType(DhcpMessageType::DHCPREQUEST);

    uint8_t buffer[MAX_MSG_SIZE];
    size_t len = 0;
    packet.serialize(buffer, len);

    ssize_t bytesSent = sendto(sockfd, buffer, len, 0,
                               (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (bytesSent < 0) {
        log("Error sending DHCPREQUEST: " + std::string(strerror(errno)));
    } else {
        log("Sent DHCPREQUEST for IP: " + offer.getIp());
    }
}

bool DHCPClient::handleAck(DhcpPacket& ack) {
    struct sockaddr_in fromAddr;
    if (!receivePacket(ack, fromAddr)) {
        return false;
    }

    if (ack.getMessageType() != DhcpMessageType::DHCPACK || ack.getMac() != mac || ack.xid != xid) {
        log("Invalid DHCPACK received");
        return false;
    }

    log("Received DHCPACK for IP: " + ack.getIp());
    clientAddr.sin_addr.s_addr = inet_addr(ack.getIp().c_str());
    log("Client IP set to: " + std::string(inet_ntoa(clientAddr.sin_addr)));
    return true;
}

bool DHCPClient::receivePacket(DhcpPacket& packet, struct sockaddr_in& fromAddr) {
    uint8_t buffer[MAX_MSG_SIZE];
    socklen_t addrLen = sizeof(fromAddr);

    std::memset(buffer, 0, MAX_MSG_SIZE);
    ssize_t bytesReceived = recvfrom(sockfd, buffer, MAX_MSG_SIZE, 0,
                                    (struct sockaddr*)&fromAddr, &addrLen);
    if (bytesReceived < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            log("Timeout waiting for packet");
        } else {
            log("Error receiving packet: " + std::string(strerror(errno)));
        }
        return false;
    }

    bool success = false;
    packet = DhcpPacket::deserialize(buffer, bytesReceived, success);
    if (!success) {
        log("Invalid packet received from " + std::string(inet_ntoa(fromAddr.sin_addr)));
        return false;
    }

    return true;
}

std::string DHCPClient::generateMac() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    std::stringstream ss;
    for (int i = 0; i < 6; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
        if (i < 5) {
            ss << ":";
        }
    }
    return ss.str();
}

void DHCPClient::log(const std::string& message) const {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S] ") << message;
    std::cout << oss.str() << std::endl;
}

} // namespace dhcp