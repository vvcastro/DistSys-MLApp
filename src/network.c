#include "include/network.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Message* create_new_message(ReliableNode* sender, int type, char* content) {
    // Create a message object with all the relevant information
    // from the sender.
    Message* message = (Message*)malloc(sizeof(Message));
    if (message == NULL) {
        perror("Memory allocation error");
        exit(1);
    }

    // First the sender info
    message->sender_pid = sender->pid;
    message->seq_number = sender->seq_number;

    // Message contents
    message->message_type = type;
    strncpy(message->content, content, MAX_CONTENT_LEN);
    return message;
};

char* encode_message(Message* message) {
    // Encode the message as a string with an specific format
    static char encoded[MAX_MESSAGE_SIZE];
    snprintf(
        encoded, sizeof(encoded), "%d|%d|%d|%s",
        message->sender_pid, message->seq_number,
        message->message_type, message->content
    );
    return encoded;
}

ReliableNode* create_node() {
    // This functions create a ReliableNode class in the most default form
    // the `pid` and `seq_number` or not yet defined.
    ReliableNode* node = (ReliableNode*)malloc(sizeof(ReliableNode));
    if (node == NULL) {
        perror("Memory allocation error");
        exit(1);
    }

    // Set-up relevant data
    node->pid = -1;
    node->seq_number = 0;

    // Set Up of the sender socket (with broadcasting)
    node->sender = socket(AF_INET, SOCK_DGRAM, 0);
    if (node->sender < 0) {
        perror("Sender creation error");
        exit(1);
    }
    int broadcast = 1;
    setsockopt(node->sender, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    // Set Up of the listener ( with address reuse )
    node->listener = socket(AF_INET, SOCK_DGRAM, 0);
    if (node->listener < 0) {
        perror("Listener creation error");
        close(node->listener);
        exit(1);
    }
    int reusePort = 1;
    setsockopt(node->listener, SOL_SOCKET, SO_REUSEPORT, &reusePort, sizeof(reusePort));

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
    char encoded_message[MAX_MESSAGE_SIZE];

    while (1) {
        ssize_t received_bytes = recvfrom(receiver->listener, encoded_message, sizeof(encoded_message), 0, NULL, NULL);
        if (received_bytes < 0) {
            perror("Message receiving error");
            exit(1);
        }
        printf("Received message: %s\n", encoded_message);
    }
    return NULL;
}

void send_message(ReliableNode* sender, Message* message) {

    // Sets up the broadcast address to send the message
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    // Serialises and sends the message
    char* encoded_message = encode_message(message);
    int msize = strlen(encoded_message);

    ssize_t sent_bytes = sendto(sender->sender, encoded_message, msize, 0, (struct sockaddr*)&saddr, sizeof(saddr));
    if (sent_bytes < 0) {
        perror("Message sending error");
        close(sender->sender);
        exit(1);
    }
    printf("Message sent: %s\n", encoded_message);
}

void close_node(ReliableNode* node) {

    // Terminate the receiving thread
    pthread_cancel(node->listener_thread);
    pthread_join(node->listener_thread, NULL);

    // Close the sockets connections
    close(node->listener);
    close(node->sender);
    free(node);
}

void delete_message(Message* message) {
    free(message);
}