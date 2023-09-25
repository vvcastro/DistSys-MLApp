#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <chrono>


// --------- Define relevant variables

// Default PORT for communications
#define PORT 4000

// Time to wait before resending a message
#define RESEND_TIME_S 0.7

// Define a max size for transmited messages
#define MAX_MSG_SIZE 1024

// --------- Define basic communication functions
int defineSenderSocket();
int defineReceiverSocket();

void sendUDPMessage(int socket, std::string toAddress, std::string encodedMessage);
void setupReceiverSocket(int recvSocket);

std::string getIPv4Address(const std::string& interfaceName);
