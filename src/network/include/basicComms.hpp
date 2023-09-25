#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <chrono>

// Initial interaction with processes
#define PROCESS_POOL {"192.168.56.4", "192.168.56.1", "192.168.56.5"}

// Default PORT for communications
#define PORT 4000

// Time to wait before resending a message
#define RESEND_TIME_S 2.5

// Define a max size for transmited messages
#define MAX_MSG_SIZE 1024

// ANSI escape codes for text colors
#define RESET   "\x1B[0m"
#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define CYAN    "\x1B[36m"
#define BLUE    "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define WHITE   "\x1B[37m"

// --------- Define basic communication functions
int defineSenderSocket();
int defineReceiverSocket();

void sendUDPMessage(int socket, std::string toAddress, std::string encodedMessage);
void setupReceiverSocket(int recvSocket);

std::string getIPv4Address(const std::string& interfaceName);


