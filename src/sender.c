#include "include/network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sock;
    char message[MAX_MESSAGE_SIZE];

    // Create socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        exit(1);
    }

    // Set up the address structure for broadcast
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    // Set the socket option to allow broadcast
    int broadcastEnable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    // Simulate sending a message
    snprintf(message, MAX_MESSAGE_SIZE, "Hello from Node A");

    // Send the message to all nodes in the LAN
    ssize_t sent_bytes = sendto(sock, message, strlen(message), 0, (struct sockaddr*)&addr, sizeof(addr));
    if (sent_bytes < 0) {
        perror("Message sending error");
        close(sock);
        exit(1);
    }

    printf("Message sent: %s\n", message);

    close(sock);
    return 0;
}
