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
        if (isValidMac(mac) && (ip.empty() || std::regex_match(ip, std::regex("\\d+\\.\\d+\\.\\d+\\.\\d+")))) {
            success = true;
        }
    }
    return msg;
}

bool Message::isValidMac(const std::string& mac) {
    std::regex macRegex("^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$");
    return std::regex_match(mac, macRegex);
}

DHCPClient::DHCPClient() : running(true) {
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
    tv.tv_sec = RESPONSE_TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        close(sockfd);
        throw std::runtime_error("Failed to set socket timeout");
    }

    // Настраиваем адрес клиента
    std::memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = INADDR_ANY;
    clientAddr.sin_port = htons(CLIENT_PORT);

    // Привязываем сокет
    if (bind(sockfd, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) < 0) {
        close(sockfd);
        throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
    }

    // Настраиваем адрес сервера
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    serverAddr.sin_port = htons(SERVER_PORT);

    // Генерируем MAC
    mac = generateMac();

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

    // Отправляем DISCOVER
    sendDiscover();

    // Ожидаем OFFER
    Message offer;
    if (handleOffer(offer)) {
        // Отправляем REQUEST
        sendRequest(offer);

        // Ожидаем ACK
        Message ack;
        if (handleAck(ack)) {
            log("Successfully acquired IP: " + ack.ip);
        } else {
            log("Failed to receive valid ACK");
        }
    } else {
        log("Failed to receive valid OFFER");
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
    Message msg;
    msg.type = MessageType::DISCOVER;
    msg.mac = mac;

    std::string data = msg.serialize();
    ssize_t bytesSent = sendto(sockfd, data.c_str(), data.size(), 0,
                               (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (bytesSent < 0) {
        log("Error sending DISCOVER: " + std::string(strerror(errno)));
    } else {
        log("Sent DISCOVER");
    }
}

bool DHCPClient::handleOffer(Message& offer) {
    struct sockaddr_in fromAddr;
    if (!receiveMessage(offer, fromAddr)) {
        return false;
    }

    if (offer.type != MessageType::OFFER || offer.mac != mac) {
        log("Invalid OFFER received");
        return false;
    }

    log("Received OFFER with IP: " + offer.ip);
    return true;
}

void DHCPClient::sendRequest(const Message& offer) {
    Message msg;
    msg.type = MessageType::REQUEST;
    msg.mac = mac;
    msg.ip = offer.ip;

    std::string data = msg.serialize();
    ssize_t bytesSent = sendto(sockfd, data.c_str(), data.size(), 0,
                               (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (bytesSent < 0) {
        log("Error sending REQUEST: " + std::string(strerror(errno)));
    } else {
        log("Sent REQUEST for IP: " + msg.ip);
    }
}

bool DHCPClient::handleAck(Message& ack) {
    struct sockaddr_in fromAddr;
    if (!receiveMessage(ack, fromAddr)) {
        return false;
    }

    if (ack.type != MessageType::ACK || ack.mac != mac) {
        log("Invalid ACK received");
        return false;
    }

    log("Received ACK for IP: " + ack.ip);
    return true;
}

bool DHCPClient::receiveMessage(Message& msg, struct sockaddr_in& fromAddr) {
    char buffer[MAX_MSG_SIZE];
    socklen_t addrLen = sizeof(fromAddr);

    std::memset(buffer, 0, MAX_MSG_SIZE);
    ssize_t bytesReceived = recvfrom(sockfd, buffer, MAX_MSG_SIZE - 1, 0,
                                    (struct sockaddr*)&fromAddr, &addrLen);
    if (bytesReceived < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            log("Timeout waiting for message");
        } else {
            log("Error receiving message: " + std::string(strerror(errno)));
        }
        return false;
    }

    buffer[bytesReceived] = '\0';
    bool success = false;
    msg = Message::deserialize(buffer, success);
    if (!success) {
        log("Invalid message received from " + std::string(inet_ntoa(fromAddr.sin_addr)));
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