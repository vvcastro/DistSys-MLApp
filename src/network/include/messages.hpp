#ifndef MESSAGES_HPP
#define MESSAGES_HPP

#include <iostream>
#include <sstream>
#include <utility>
#include <vector>
#include <string>
#include <map>

// Codes for text colors
#define RESET   "\x1B[0m"
#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define CYAN    "\x1B[36m"
#define BLUE    "\x1B[34m"
#define MAGENTA "\x1B[35m"

enum MessageType {
    DATA,
    ACK,
    BEAT,
    INVALID,
};

// Contains all the data that will be transmited over the network
class Message {
    public:
    Message() {}
    Message(std::string sender, MessageType type, std::string data);

    // To manage internal data
    MessageType getType() { return type; };
    std::map<std::string, int> getClock() { return clock; }
    void setClock(std::map<std::string, int> clock) { this->clock = clock; };

    // To be able to exchange the message over the network
    std::string encodeToString();
    static Message decodeToMessage(const std::string encodedMessage);
    static Message getRespondMessage(const Message& other);

    // To compare messages
    bool operator==(const Message& other);
    bool operator<(const Message& other) const;

    private:
    std::string sender;
    MessageType type;
    std::string data;
    std::map<std::string, int> clock;
};

// Encaptulation of a received message, it ease the management
// at the receiving node.
class RecvMessage {
    public:
    RecvMessage(std::string fromAddress, Message message);
    bool operator==(const RecvMessage& other);
    void displayMessage();

    std::string fromAddress;
    Message message;
};

// Encaptulation of a sent message, it ease the management
// at the sending node.
class SentMessage {
    public:
    SentMessage(std::string toAddress, Message message);
    bool isResponse(RecvMessage other);
    void displayMessage();

    std::string toAddress;
    Message message;

    // To compute how many times a message is re-sent
    int reCounter;
    void addCounter() { ++reCounter; };

};

// ----------- AUX functions
std::string getTypeString(MessageType type);
MessageType stringToType(const std::string typeName);

std::string mapToString(std::map<std::string, int> baseMap);
std::map<std::string, int> stringToMap(std::string strMap);
#endif