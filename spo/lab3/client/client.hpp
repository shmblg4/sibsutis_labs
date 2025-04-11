#ifndef DHCP_CLIENT_HPP
#define DHCP_CLIENT_HPP

#include <string>
#include <netinet/in.h>
#include <ctime>

namespace dhcp {

constexpr int SERVER_PORT = 67;
constexpr int CLIENT_PORT = 68;
constexpr int MAX_MSG_SIZE = 256;
constexpr int RESPONSE_TIMEOUT_SEC = 5;

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

    // Сериализация в строку
    std::string serialize() const;
    // Десериализация из строки
    static Message deserialize(const std::string& data, bool& success);
    // Проверка корректности MAC
    static bool isValidMac(const std::string& mac);
};

// Основной класс клиента
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
    volatile bool running; // Флаг для graceful завершения
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;

    void sendDiscover();
    bool handleOffer(Message& offer);
    void sendRequest(const Message& offer);
    bool handleAck(Message& ack);
    bool receiveMessage(Message& msg, struct sockaddr_in& fromAddr);
    std::string generateMac();
    void log(const std::string& message) const;
};

} // namespace dhcp

#endif // DHCP_CLIENT_HPP