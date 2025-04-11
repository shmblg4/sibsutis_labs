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

std::string Message::serialize() const {
    std::stringstream ss;
    switch (type) {
        case MessageType::DISCOVER:
            ss << "DISCOVER|" << mac;
            break;
        case MessageType::OFFER:
            ss << "OFFER|" << ip << "|" << mac;
            break;
        case MessageType::REQUEST:
            ss << "REQUEST|" << ip << "|" << mac;
            break;
        case MessageType::ACK:
            ss << "ACK|" << ip << "|" << mac;
            break;
    }
    return ss.str();
}

Message Message::deserialize(const std::string& data, bool& success) {
    Message msg;
    success = false;

    if (data.size() > MAX_MSG_SIZE) {
        return msg;
    }

    std::stringstream ss(data);
    std::string typeStr, mac, ip;

    std::getline(ss, typeStr, '|');
    if (typeStr == "DISCOVER") {
        msg.type = MessageType::DISCOVER;
        std::getline(ss, mac);
        msg.mac = mac;
        if (isValidMac(mac)) {
            success = true;
        }
    } else if (typeStr == "OFFER" || typeStr == "REQUEST" || typeStr == "ACK") {
        msg.type = (typeStr == "OFFER") ? MessageType::OFFER :
                   (typeStr == "REQUEST") ? MessageType::REQUEST : MessageType::ACK;
        std::getline(ss, ip, '|');
        std::getline(ss, mac);
        msg.ip = ip;
        msg.mac = mac;
        // Проверяем MAC и IP (IP может быть пустым для OFFER/ACK на клиенте)
        if (isValidMac(mac) && (ip.empty() || std::regex_match(ip, std::regex("\\d+\\.\\d+\\.\\d+\\.\\d+")))) {
            success = true;
        }
    }
    return msg;
}

bool Message::isValidMac(const std::string& mac) {
    // Проверяем формат xx:xx:xx:xx:xx:xx
    std::regex macRegex("^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$");
    return std::regex_match(mac, macRegex);
}

IpPool::IpPool() {
    for (int i = IP_RANGE_START; i <= IP_RANGE_END; ++i) {
        ipPool.push_back(std::string(BASE_IP) + std::to_string(i));
    }
}

std::string IpPool::allocateIp(const std::string& mac) {
    // Проверяем, не выделен ли уже IP для этого MAC
    auto it = allocatedIps.find(mac);
    if (it != allocatedIps.end()) {
        return it->second; // Возвращаем существующий IP
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
    // Создаем UDP-сокет
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    // Включаем широковещательные сообщения
    int broadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        close(sockfd);
        throw std::runtime_error("Failed to set broadcast option");
    }

    // Устанавливаем таймаут
    struct timeval tv;
    tv.tv_sec = SOCKET_TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        close(sockfd);
        throw std::runtime_error("Failed to set socket timeout");
    }

    // Настраиваем адрес сервера
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    // Привязываем сокет
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
    char buffer[MAX_MSG_SIZE];
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    while (running) {
        // Получаем сообщение
        std::memset(buffer, 0, MAX_MSG_SIZE);
        ssize_t bytesReceived = recvfrom(sockfd, buffer, MAX_MSG_SIZE - 1, 0,
                                        (struct sockaddr*)&clientAddr, &addrLen);
        if (bytesReceived < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue; // Таймаут, проверяем running
            }
            log("Error receiving message: " + std::string(strerror(errno)));
            continue;
        }

        buffer[bytesReceived] = '\0';
        bool success = false;
        Message msg = Message::deserialize(buffer, success);
        if (!success) {
            log("Invalid message received from " + std::string(inet_ntoa(clientAddr.sin_addr)));
            continue;
        }

        log("Received " + std::string(msg.type == MessageType::DISCOVER ? "DISCOVER" : "REQUEST") +
            " from MAC: " + msg.mac + " at " + inet_ntoa(clientAddr.sin_addr));
        handleMessage(msg, clientAddr);
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

void DHCPServer::handleMessage(const Message& msg, const struct sockaddr_in& clientAddr) {
    Message response;
    if (msg.type == MessageType::DISCOVER) {
        response = handleDiscover(msg);
    } else if (msg.type == MessageType::REQUEST) {
        response = handleRequest(msg);
    } else {
        log("Ignoring unknown message type from MAC: " + msg.mac);
        return;
    }

    if (!response.ip.empty()) {
        sendMessage(response, clientAddr);
    } else {
        log("No response sent for MAC: " + msg.mac + " (no IP available)");
    }
}

Message DHCPServer::handleDiscover(const Message& msg) {
    Message response;
    response.type = MessageType::OFFER;
    response.mac = msg.mac;
    response.ip = ipPool.allocateIp(msg.mac);

    if (response.ip.empty()) {
        log("No available IPs for MAC: " + msg.mac);
    } else {
        log("Offering IP: " + response.ip + " to MAC: " + msg.mac);
    }

    return response;
}

Message DHCPServer::handleRequest(const Message& msg) {
    Message response;
    response.type = MessageType::ACK;
    response.mac = msg.mac;
    response.ip = msg.ip;

    if (!ipPool.isIpInPool(msg.ip)) {
        log("Requested IP " + msg.ip + " is not in pool for MAC: " + msg.mac);
        response.ip = "";
        return response;
    }

    if (ipPool.confirmIp(msg.mac, msg.ip)) {
        log("Confirmed IP: " + msg.ip + " for MAC: " + msg.mac);
    } else {
        log("Invalid REQUEST for IP: " + msg.ip + ", MAC: " + msg.mac);
        response.ip = "";
    }

    return response;
}

void DHCPServer::sendMessage(const Message& msg, const struct sockaddr_in& clientAddr) {
    std::string data = msg.serialize();
    struct sockaddr_in destAddr = clientAddr;
    destAddr.sin_port = htons(CLIENT_PORT);

    ssize_t bytesSent = sendto(sockfd, data.c_str(), data.size(), 0,
                               (struct sockaddr*)&destAddr, sizeof(destAddr));
    if (bytesSent < 0) {
        log("Error sending message to MAC: " + msg.mac + ": " + strerror(errno));
    } else {
        log("Sent " + std::string(msg.type == MessageType::OFFER ? "OFFER" : "ACK") +
            " with IP: " + msg.ip + " to MAC: " + msg.mac);
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