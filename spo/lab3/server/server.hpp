#ifndef DHCP_SERVER_HPP
#define DHCP_SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <cstdint>

namespace dhcp {

constexpr int SERVER_PORT = 67;
constexpr int CLIENT_PORT = 68;
constexpr int MAX_MSG_SIZE = 548; // Минимальный размер DHCP-пакета
constexpr const char* BASE_IP = "172.18.0.";
constexpr int IP_RANGE_START = 100;
constexpr int IP_RANGE_END = 200;
constexpr int SOCKET_TIMEOUT_SEC = 1;
constexpr uint32_t DEFAULT_LEASE_TIME = 86400; // 24 часа в секундах

enum class DhcpMessageType {
    DHCPDISCOVER = 1,
    DHCPOFFER = 2,
    DHCPREQUEST = 3,
    DHCPACK = 5
};

// Структура для DHCP-пакета
struct DhcpPacket {
    uint8_t op;      // 1 = BOOTREQUEST, 2 = BOOTREPLY
    uint8_t htype;   // 1 = Ethernet
    uint8_t hlen;    // 6 = MAC-адрес
    uint8_t hops;    // 0
    uint32_t xid;    // Transaction ID
    uint16_t secs;   // Seconds elapsed
    uint16_t flags;  // Flags
    uint32_t ciaddr; // Client IP (0 для DISCOVER)
    uint32_t yiaddr; // Your (client) IP (заполняет сервер)
    uint32_t siaddr; // Server IP
    uint32_t giaddr; // Gateway IP
    uint8_t chaddr[16]; // Client hardware address (MAC)
    uint8_t sname[64];  // Server name (не обязательно)
    uint8_t file[128];  // Boot file name (не обязательно)
    uint8_t options[312]; // Опции DHCP (начинаются с magic cookie 0x63825363)

    // Методы для работы с пакетом
    std::string getMac() const;
    void setMac(const std::string& mac);
    std::string getIp() const;
    void setIp(const std::string& ip);
    DhcpMessageType getMessageType() const;
    void setMessageType(DhcpMessageType type);
    void setLeaseTime(uint32_t leaseTime);
    static bool isValidMac(const std::string& mac);
    void serialize(uint8_t* buffer, size_t& len) const;
    static DhcpPacket deserialize(const uint8_t* buffer, size_t len, bool& success);
};

class IpPool {
public:
    IpPool();
    std::string allocateIp(const std::string& mac);
    bool confirmIp(const std::string& mac, const std::string& ip);
    bool isIpAvailable(const std::string& ip) const;
    bool isIpInPool(const std::string& ip) const;

private:
    std::vector<std::string> ipPool;
    std::map<std::string, std::string> allocatedIps; // MAC -> IP
};

class DHCPServer {
public:
    DHCPServer();
    ~DHCPServer();

    void start();
    void stop();
    bool shouldStop() const;

private:
    int sockfd;
    struct sockaddr_in serverAddr;
    IpPool ipPool;
    volatile bool running;

    void handlePacket(const DhcpPacket& packet, const struct sockaddr_in& clientAddr);
    DhcpPacket handleDiscover(const DhcpPacket& packet);
    DhcpPacket handleRequest(const DhcpPacket& packet);
    void sendPacket(const DhcpPacket& packet, const struct sockaddr_in& clientAddr);
    void log(const std::string& message) const;
};

} // namespace dhcp

#endif // DHCP_SERVER_HPP