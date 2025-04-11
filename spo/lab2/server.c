#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5

struct Packet {
    int seq_num;
    int total_packets;
    char data[BUFFER_SIZE];
};

struct ClientData {
    char* buffer;
    int* received;
    int total_packets;
    int packets_received;
    int active;
};

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = 0;

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(sockfd, (struct sockaddr*)&sin, &len);
    printf("Server started on port: %d\n", ntohs(sin.sin_port));

    struct ClientData clients[MAX_CLIENTS] = {0};

    while (1) {
        struct Packet packet;
        char ack[10];
        
        int n = recvfrom(sockfd, &packet, sizeof(packet), 0, 
                        (struct sockaddr*)&client_addr, &addr_len);
        if (n < 0) {
            perror("Receive failed");
            continue;
        }

        char client_id[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_id, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);
        int client_idx = client_port % MAX_CLIENTS;

        if (!clients[client_idx].active) {
            clients[client_idx].total_packets = packet.total_packets;
            clients[client_idx].buffer = calloc(packet.total_packets, BUFFER_SIZE);
            clients[client_idx].received = calloc(packet.total_packets, sizeof(int));
            clients[client_idx].packets_received = 0;
            clients[client_idx].active = 1;
            printf("New client connected: %s:%d\n", client_id, client_port);
        }

        if (!clients[client_idx].received[packet.seq_num]) {
            memcpy(clients[client_idx].buffer + (packet.seq_num * BUFFER_SIZE), 
                   packet.data, strlen(packet.data));
            clients[client_idx].received[packet.seq_num] = 1;
            clients[client_idx].packets_received++;
            printf("Received packet %d/%d from %s:%d\n", 
                   packet.seq_num, packet.total_packets, client_id, client_port);
        }

        // Отправка ACK
        sprintf(ack, "ACK%d", packet.seq_num);
        sendto(sockfd, ack, strlen(ack), 0, 
               (struct sockaddr*)&client_addr, addr_len);

        // Запись в файл при получении всех пакетов
        if (clients[client_idx].packets_received == clients[client_idx].total_packets) {
            char filename[50];
            sprintf(filename, "received_%d.txt", client_port);
            FILE* fp = fopen(filename, "w");
            if (fp) {
                fwrite(clients[client_idx].buffer, 1, 
                       packet.total_packets * BUFFER_SIZE, fp);
                fclose(fp);
                printf("File from %s:%d successfully received\n", client_id, client_port);
            }
            
            free(clients[client_idx].buffer);
            free(clients[client_idx].received);
            clients[client_idx].active = 0;
        }
    }

    close(sockfd);
    return 0;
}