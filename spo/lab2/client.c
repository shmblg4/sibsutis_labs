#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024
#define TIMEOUT_SEC 3
#define MAX_ATTEMPTS 5

struct Packet {
    int seq_num;
    int total_packets;
    char data[BUFFER_SIZE];
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <server_ip> <port> <filename>\n", argv[0]);
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return 1;
    }

    FILE* fp = fopen(argv[3], "r");
    if (!fp) {
        perror("File opening failed");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);
    int total_packets = (file_size + BUFFER_SIZE - 1) / BUFFER_SIZE;

    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct Packet packet;
    packet.total_packets = total_packets;
    char ack[10];
    socklen_t addr_len = sizeof(server_addr);

    for (int i = 0; i < total_packets; i++) {
        packet.seq_num = i;
        size_t bytes_read = fread(packet.data, 1, BUFFER_SIZE - 1, fp);
        packet.data[bytes_read] = '\0';

        int attempts = 0;
        int ack_received = 0;

        while (attempts < MAX_ATTEMPTS && !ack_received) {
            sendto(sockfd, &packet, sizeof(packet), 0, 
                  (struct sockaddr*)&server_addr, sizeof(server_addr));

            int n = recvfrom(sockfd, ack, sizeof(ack), 0, 
                           (struct sockaddr*)&server_addr, &addr_len);
            
            if (n > 0) {
                ack[n] = '\0';
                if (strncmp(ack, "ACK", 3) == 0 && atoi(ack + 3) == i) {
                    ack_received = 1;
                    printf("Packet %d/%d sent successfully\n", i, total_packets);
                }
            }
            
            if (!ack_received) {
                attempts++;
                printf("Timeout for packet %d, attempt %d/%d\n", 
                       i, attempts, MAX_ATTEMPTS);
            }
        }

        if (!ack_received) {
            printf("Failed to send packet %d after %d attempts\n", i, MAX_ATTEMPTS);
            fclose(fp);
            close(sockfd);
            return 1;
        }
    }

    printf("File sent successfully\n");
    fclose(fp);
    close(sockfd);
    return 0;
}