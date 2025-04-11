#ifndef DHCP_SERVER_HPP
#define DHCP_SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <ctime>

namespace dhcp {

constexpr int SERVER_PORT = 67;
constexpr int CLIENT_PORT = 68;
constexpr int MAX_MSG_SIZE = 256; // Уменьшен для DHCP
constexpr const char* BASE_IP = "172.18.0.";
constexpr int IP_RANGE_START = 100;
constexpr int IP_RANGE_END = 200;
constexpr int SOCKET_TIMEOUT_SEC = 1;

enum class MessageType {
    DISCOVER,
    OFFER,
    REQUEST,
    ACK
};

struct Message {
    MessageType type;
    std::string mac;
    std::string ip;

    std::string serialize() const;
    static Message deserialize(const std::string& data, bool& success);
    static bool isValidMac(const std::string& mac);
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
    volatile bool running; // Флаг для graceful завершения

    void handleMessage(const Message& msg, const struct sockaddr_in& clientAddr);
    Message handleDiscover(const Message& msg);
    Message handleRequest(const Message& msg);
    void sendMessage(const Message& msg, const struct sockaddr_in& clientAddr);
    void log(const std::string& message) const;
};

} // namespace dhcp

#endif // DHCP_SERVER_HPP