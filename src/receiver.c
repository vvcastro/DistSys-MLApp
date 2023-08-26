#include "include/network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    int sock;
    struct sockaddr_in addr;
    char message[MAX_MESSAGE_SIZE];

    // Initialize Winsock for Windows
    #ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        perror("WSAStartup failed");
        exit(1);
    }
    #endif

    // Create socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        #ifdef _WIN32
        WSACleanup();
        #endif
    }
    // int reusePort = 1;
    // setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (const char*)&reusePort, sizeof(reusePort));

    // Set up the address structure to bind to the broadcast port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the broadcast port
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Binding error");
        close(sock);
        #ifdef _WIN32
        WSACleanup();
        #endif
        exit(1);
    }
    printf("Receiver node listening...\n");
    printf(" -IPv4: %u\n", INADDR_ANY);
    printf(" -Port: %d\n", PORT);

    while (1) {
        ssize_t received_bytes = recvfrom(sock, message, sizeof(message), 0, NULL, NULL);
        if (received_bytes < 0) {
            perror("Message receiving error");
            close(sock);
            #ifdef _WIN32
            WSACleanup();
            #endif
        }
        message[received_bytes] = '\0'; // Null-terminate the received message
        printf("Received message: %s\n", message);
    }

    close(sock);
    #ifdef _WIN32
    WSACleanup();
    #endif
    return 0;
}