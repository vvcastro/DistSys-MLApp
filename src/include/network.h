#ifndef NETWORK_H

#ifdef _WIN32

#include <winsock2.h>
#define INET_ADDRSTRLEN 16

#elif defined(__unix__) || defined(__APPLE__)

#include <arpa/inet.h>

#endif

#include <pthread.h>

// Define the IP address to work in
#define IP_ADDRESS inet_addr("10.0.0.255")
#define PORT 4000

// Define the relevant structs for the netwokr application
#define MESSAGE_TYPE_NORMAL 0
#define MESSAGE_TYPE_ACK 1

#define MAX_CONTENT_LEN 256
#define MAX_MESSAGE_SIZE 512

typedef struct {
    int seq_number;
    int message_type;
    char content[MAX_CONTENT_LEN];

} Message;

typedef struct {

    // Identification and synchronisation
    int pid;
    int seq_number;

    // Communication ( send and receive, both broadcast )
    pthread_t listener_thread;
    int listener;
    int sender;

} ReliableNode;

// Creates a new Message struct with all the relevan information
Message* create_new_message(ReliableNode* sender, int type, char* content);

// To (de)serialise messages
char* encode_message(Message* message);
Message* decode_message(char* encoded_message);

// Init the node binding the sockets to the port
ReliableNode* create_node();

// Binds the receiving socket and listens the network
void start_network_listening(ReliableNode* receiver);

// Uses the node to send a message in a broadcast way.
void send_message(ReliableNode* node, Message* message);
void* receive_messages(void* arg);

// Clean up resources
void close_node(ReliableNode* node);
void delete_message(Message* message);

#endif