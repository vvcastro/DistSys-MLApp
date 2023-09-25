#include "include/comms.hpp"
#include <netinet/in.h>
#include <ifaddrs.h>
#include <string.h>
#include <string>

int defineSenderSocket() {
    int sendSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (sendSocket == -1) {
        perror("Sender socket creation failed!");
        exit(1);
    }
    return sendSocket;
}

int defineReceiverSocket() {
    int recvSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (recvSocket == -1) {
        perror("Receiver socket creation failed!");
        exit(1);
    }
    return recvSocket;
}

// Wraps the logic to send a message using UDP sockets.
void sendUDPMessage(int socket, std::string toAddress, std::string encodedMessage) {

    // Sets up the socket for sending the message
    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_addr.s_addr = inet_addr(toAddress.c_str());
    destAddr.sin_port = htons(PORT);
    destAddr.sin_family = AF_INET;

    // Serialises and sends the message
    int messageSize = encodedMessage.length();
    ssize_t sent_bytes = sendto(socket, encodedMessage.c_str(), messageSize, 0, (struct sockaddr*)&destAddr, sizeof(destAddr));
    if (sent_bytes < 0) {
        perror("Message sending error");
        close(socket);
        exit(1);
    }
}

// Binds the socket to the PORT to start listening
// for messages
void setupReceiverSocket(int recvSocket) {

    struct sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(recvAddr));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(PORT);
    recvAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the broadcast port
    if (bind(recvSocket, (struct sockaddr*)&recvAddr, sizeof(recvAddr)) < 0) {
        perror("Binding error: couldn't setup the socket!");
        close(recvSocket);
        exit(1);
    }
};

// Uses the primivites to get a IP address for the system
std::string getIPv4Address(const std::string& interfaceName) {
    std::string ipAddress;

    struct ifaddrs* ifAddrStruct = nullptr;
    struct ifaddrs* ifa = nullptr;
    void* tmpAddrPtr = nullptr;

    if (getifaddrs(&ifAddrStruct) == -1) {
        perror("Could not resolve IP address!");
        exit(1);
    }

    // Iterate over the network interfaces
    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }

        if (ifa->ifa_addr->sa_family == AF_INET) {
            if (strcmp(ifa->ifa_name, interfaceName.c_str()) == 0) {
                tmpAddrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                ipAddress = addressBuffer;
                break;
            }
        }
    }

    if (ifAddrStruct != nullptr) {
        freeifaddrs(ifAddrStruct);
    }
    return ipAddress;
}
