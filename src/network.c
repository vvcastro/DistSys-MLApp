#include "include/network.h"

#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ----------> Messages functions

Message* create_new_message(ReliableNode* sender, int type, char* content) {
    // Create a message object with all the relevant information
    // from the sender.
    Message* message = (Message*)malloc(sizeof(Message));
    if (message == NULL) {
        perror("Memory allocation error");
        exit(1);
    }

    // Message contents
    message->seq_number = sender->seq_number;
    message->message_type = type;
    strncpy(message->content, content, MAX_CONTENT_LEN);
    return message;
};

char* encode_message(Message* message) {
    // Encode the message as a string with an specific format
    static char encoded[MAX_MESSAGE_SIZE];
    snprintf(
        encoded, sizeof(encoded), "%d|%d|%s", message->seq_number,
        message->message_type, message->content
    );
    return encoded;
}

Message* decode_message(char* encoded_message) {
    // Decode the string message sent over the network
    int seq_number, message_type;
    char content[MAX_CONTENT_LEN];
    sscanf(encoded_message, "%d|%d|%s", &seq_number, &message_type, content);

    Message* message = (Message*)malloc(sizeof(Message));
    if (message == NULL) {
        perror("Memory allocation error");
        exit(1);
    }

    // Message contents
    message->seq_number = seq_number;
    message->message_type = message_type;
    strncpy(message->content, content, MAX_CONTENT_LEN);
    return message;
}


// ----------> Node functions

ReliableNode* create_node() {
    // Initiates the ReliableNode struct
    ReliableNode* node = (ReliableNode*)malloc(sizeof(ReliableNode));
    if (node == NULL) {
        perror("Memory allocation error");
        exit(1);
    }

    // Set-up relevant data
    node->pid = -1;
    node->seq_number = 0;

    // SetUp connection in Windows application
    #ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        perror("WSAStartup failed");
        exit(1);
    }
    #endif

    // Set Up of the sender socket (with broadcasting)
    node->sender = socket(AF_INET, SOCK_DGRAM, 0);
    if (node->sender < 0) {
        perror("Sender creation error");
        close_node(node);
        exit(1);
    }
    int bcastEnabled = 1;
    setsockopt(node->sender, SOL_SOCKET, SO_BROADCAST, (const char*)&bcastEnabled, sizeof(bcastEnabled));

    // Set Up of the listener ( with address reuse )
    node->listener = socket(AF_INET, SOCK_DGRAM, 0);
    if (node->listener < 0) {
        perror("Listener creation error");
        close_node(node);
        exit(1);
    }
    return node;
}

void start_network_listening(ReliableNode* receiver) {
    // Binds the receiver socket and start listening for network
    // messages in an asynchronous way.
    printf("Listening for incoming messages\n");

    struct sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(PORT);
    listen_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(receiver->listener, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) < 0) {
        perror("Binding error");
        close(receiver->listener);
        exit(1);
    }

    // Create a thread for receiving messages
    if (pthread_create(&(receiver->listener_thread), NULL, receive_messages, (void*)receiver) != 0) {
        perror("Thread creation error");
        close(receiver->listener);
        exit(1);
    }
}

void* receive_messages(void* arg) {
    // Here we read a message from the socket, later this will
    // include the processing steps for the specific type of message.
    ReliableNode* receiver = (ReliableNode*)arg;
    int listener = receiver->listener;

    // Define the receiving message
    char encoded_msg[MAX_MESSAGE_SIZE];
    size_t msize = sizeof(encoded_msg) - 1;

    while (1) {
        struct sockaddr_in saddr;
        #ifdef _WIN32
        int slen = sizeof(saddr);
        #else
        socklen_t slen = sizeof(saddr);
        #endif

        // Receive the message
        ssize_t received_bytes = recvfrom(listener, encoded_msg, msize, 0, (struct sockaddr*)&saddr, &slen);
        if (received_bytes < 0) {
            perror("Message receiving error");
            exit(1);
        }
        encoded_msg[received_bytes] = '\0';
        Message* message = decode_message(encoded_msg);

        // Get the sender IP address
        char sender[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(saddr.sin_addr), sender, INET_ADDRSTRLEN);

        printf("-------------------\n");
        printf("From: %s\n", sender);
        printf(" - Message (%d): %s\n", message->message_type, message->content);
        printf("-------------------\n");
    }
    return NULL;
}

void send_message(ReliableNode* sender, Message* message) {

    // Sets up the broadcast address to send the message
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = IP_ADDRESS;

    // Serialises and sends the message
    char* encoded_message = encode_message(message);
    int msize = strlen(encoded_message);

    ssize_t sent_bytes = sendto(sender->sender, encoded_message, msize, 0, (struct sockaddr*)&saddr, sizeof(saddr));
    if (sent_bytes < 0) {
        perror("Message sending error");
        close(sender->sender);
        exit(1);
    }
    printf("(To %d) Message sent: %s\n", PORT, encoded_message);
}

void close_node(ReliableNode* node) {

    // Terminate the receiving thread
    pthread_cancel(node->listener_thread);
    pthread_join(node->listener_thread, NULL);

    // For windows
    #ifdef _WIN32
    WSACleanup();
    #endif

    // Close the sockets connections
    if (node->listener) {
        close(node->listener);
    };
    if (node->sender) {
        close(node->sender);
    }
    free(node);
}

void delete_message(Message* message) {
    free(message);
}