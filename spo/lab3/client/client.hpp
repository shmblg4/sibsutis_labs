#ifndef DHCP_CLIENT_HPP
#define DHCP_CLIENT_HPP

#include <string>
#include <netinet/in.h>
#include <cstdint>

namespace dhcp {

constexpr int SERVER_PORT = 67;
constexpr int CLIENT_PORT = 68;
constexpr int MAX_MSG_SIZE = 548;
constexpr int RESPONSE_TIMEOUT_SEC = 5;

enum class DhcpMessageType {
    DHCPDISCOVER = 1,
    DHCPOFFER = 2,
    DHCPREQUEST = 3,
    DHCPACK = 5
};

struct DhcpPacket {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint32_t ciaddr;
    uint32_t yiaddr;
    uint32_t siaddr;
    uint32_t giaddr;
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint8_t options[312];

    std::string getMac() const;
    void setMac(const std::string& mac);
    std::string getIp() const;
    void setIp(const std::string& ip);
    DhcpMessageType getMessageType() const;
    void setMessageType(DhcpMessageType type);
    static bool isValidMac(const std::string& mac);
    void serialize(uint8_t* buffer, size_t& len) const;
    static DhcpPacket deserialize(const uint8_t* buffer, size_t len, bool& success);
};

class DHCPClient {
public:
    DHCPClient();
    ~DHCPClient();

    void start();
    void stop();
    bool shouldStop() const;

private:
    int sockfd;
    std::string mac;
    uint32_t xid;
    volatile bool running;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;

    void sendDiscover();
    bool handleOffer(DhcpPacket& offer);
    void sendRequest(const DhcpPacket& offer);
    bool handleAck(DhcpPacket& ack);
    bool receivePacket(DhcpPacket& packet, struct sockaddr_in& fromAddr);
    std::string generateMac();
    void log(const std::string& message) const;
};

} // namespace dhcp

#endif // DHCP_CLIENT_HPP