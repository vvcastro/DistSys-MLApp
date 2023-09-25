#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <chrono>

// Initial interaction with processes
#define PROCESS_POOL {"192.168.56.4", "192.168.56.1", "192.168.56.5"}

// Default PORT for communications
#define PORT 4000

// Time to wait before resending a message
#define RESEND_TIME_S 1.5

// Time between heartbeats
#define HEARTBEAT_PERIOD 0.5

// Define a max size for transmited messages
#define MAX_MSG_SIZE 1024

// --------- Define basic communication functions
int defineSenderSocket();
int defineReceiverSocket();

void sendUDPMessage(int socket, std::string toAddress, std::string encodedMessage);
void setupReceiverSocket(int recvSocket);

std::string getIPv4Address(const std::string& interfaceName);


