#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <map>
#include <random>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <algorithm>

#define MAX_PACKET_SIZE 128
#define MAX_RETRIES 3
#define TIMEOUT_SEC 1
#define LOSS 20
#define PORT 8080

struct Packet {
    uint32_t seq_num;
    uint32_t total_packets;
    char data[MAX_PACKET_SIZE];
    uint32_t data_size;
    uint32_t checksum;
};

uint32_t calculate_checksum(const char* data, size_t size) {
    uint32_t sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += (unsigned char)data[i];
    }
    return sum;
}

// Server code
void server(int packet_loss_count) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Socket creation failed\n";
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed: " << strerror(errno) << "\n";
        close(sockfd);
        return;
    }

    std::cout << "Server running on port: " << PORT << "\n";

    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 99);
    int lost_packets = 0;

    // Хранение данных клиентов для проверки целостности
    std::map<std::string, std::vector<Packet>> client_data;

    while (true) {
        Packet packet;
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // Получение датаграммы
        ssize_t recv_len = recvfrom(sockfd, &packet, sizeof(Packet), 0,
                                  (struct sockaddr*)&client_addr, &client_len);
        if (recv_len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            std::cerr << "Receive error: " << strerror(errno) << "\n";
            continue;
        }

        // Формирование ключа клиента
        std::string client_key = std::string(inet_ntoa(client_addr.sin_addr)) + ":" +
                                std::to_string(ntohs(client_addr.sin_port));

        // Симуляция потери пакета
        if (lost_packets < packet_loss_count && dis(gen) < LOSS) {
            lost_packets++;
            std::cout << "Simulated packet loss for seq: " << packet.seq_num
                      << " from " << client_key << "\n";
            continue;
        }

        // Проверка контрольной суммы
        uint32_t received_checksum = packet.checksum;
        packet.checksum = 0;
        uint32_t calculated_checksum = calculate_checksum((char*)&packet, sizeof(Packet));
        if (received_checksum != calculated_checksum) {
            std::cout << "Checksum error for packet " << packet.seq_num
                      << " from " << client_key << "\n";
            continue;
        }

        // Вывод информации о датаграмме
        std::cout << "Received packet " << packet.seq_num << "/" << packet.total_packets
                  << " from " << client_key
                  << ", size: " << packet.data_size << " bytes"
                  << ", checksum: " << received_checksum << "\n";

        // Сохранение пакета
        if (client_data.find(client_key) == client_data.end()) {
            client_data[client_key] = std::vector<Packet>(packet.total_packets);
        }
        client_data[client_key][packet.seq_num] = packet;

        // Подтверждение получения (ACK)
        uint32_t ack = packet.seq_num;
        sendto(sockfd, &ack, sizeof(ack), 0,
               (struct sockaddr*)&client_addr, client_len);
        std::cout << "Sent ACK for packet " << packet.seq_num << " to " << client_key << "\n";

        // Проверка завершения передачи
        bool complete = true;
        for (uint32_t i = 0; i < packet.total_packets; i++) {
            if (client_data[client_key][i].data_size == 0) {
                complete = false;
                break;
            }
        }

        if (complete) {
            // Сохранение данных в файл
            std::string filename = "received_" + client_key + ".dat";
            std::replace(filename.begin(), filename.end(), ':', '_');
            std::ofstream out(filename, std::ios::binary);
            for (const auto& p : client_data[client_key]) {
                out.write(p.data, p.data_size);
            }
            out.close();

            std::cout << "File transfer complete from " << client_key
                      << ", saved to " << filename << "\n";

            // Проверка целостности
            std::ifstream in(filename, std::ios::binary);
            in.seekg(0, std::ios::end);
            size_t file_size = in.tellg();
            in.close();
            size_t expected_size = 0;
            for (const auto& p : client_data[client_key]) {
                expected_size += p.data_size;
            }
            if (file_size == expected_size) {
                std::cout << "Integrity check passed for " << client_key
                          << ": " << file_size << " bytes\n";
            } else {
                std::cout << "Integrity check failed for " << client_key
                          << ": expected " << expected_size << " bytes, got " << file_size << "\n";
            }

            // Очистка данных клиента
            client_data.erase(client_key);
        }
    }

    close(sockfd);
}

// Client code
void client(const std::string& server_ip, const std::string& filename) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Socket creation failed\n";
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Read file
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << filename << "\n";
        close(sockfd);
        return;
    }

    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(file_size);
    file.read(buffer.data(), file_size);
    file.close();

    // Split into packets
    std::vector<Packet> packets;
    size_t offset = 0;
    uint32_t seq = 0;
    while (offset < file_size) {
        Packet packet;
        packet.seq_num = seq++;
        packet.data_size = std::min<size_t>(MAX_PACKET_SIZE, file_size - offset);
        memcpy(packet.data, buffer.data() + offset, packet.data_size);
        packet.total_packets = (file_size + MAX_PACKET_SIZE - 1) / MAX_PACKET_SIZE;
        packet.checksum = 0;
        packet.checksum = calculate_checksum((char*)&packet, sizeof(Packet));
        packets.push_back(packet);
        offset += packet.data_size;
    }

    // Send packets
    for (const auto& packet : packets) {
        int retries = 0;
        bool sent = false;
        while (retries < MAX_RETRIES && !sent) {
            sendto(sockfd, &packet, sizeof(Packet), 0,
                   (struct sockaddr*)&server_addr, sizeof(server_addr));

            uint32_t ack;
            ssize_t recv_len = recvfrom(sockfd, &ack, sizeof(ack), 0, nullptr, nullptr);
            if (recv_len > 0 && ack == packet.seq_num) {
                sent = true;
                std::cout << "Sent packet " << packet.seq_num << "/" << packet.total_packets
                          << " (" << packet.data_size << " bytes)\n";
            } else {
                retries++;
                std::cout << "Timeout for packet " << packet.seq_num << ", retry " << retries << "\n";
            }
        }
        if (!sent) {
            std::cerr << "Failed to send packet " << packet.seq_num << "\n";
        }
    }

    close(sockfd);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " server [packet_loss_count] | client <server_ip> <file>\n";
        return 1;
    }

    std::string mode = argv[1];
    if (mode == "server") {
        int packet_loss_count = (argc > 2) ? std::atoi(argv[2]) : 0;
        server(packet_loss_count);
    } else if (mode == "client") {
        if (argc != 4) {
            std::cerr << "Usage: " << argv[0] << " client <server_ip> <file>\n";
            return 1;
        }
        client(argv[2], argv[3]);
    } else {
        std::cerr << "Invalid mode\n";
        return 1;
    }

    return 0;
}