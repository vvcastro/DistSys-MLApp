#include "basicComms.hpp"
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>
#include <string>

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
    void setType(MessageType type);
    void setClock(std::vector<std::pair<std::string, int> > vclocks);

    // To be able to exchange the message over the network
    std::string encodeToString();
    static Message decodeToMessage(const std::string encodedMessage);
    static Message getRespondMessage(const Message& other);

    // To compare messages
    bool operator==(const Message& other);

    private:
    std::string sender;
    MessageType type;
    std::string data;
    std::vector<std::pair<std::string, int> > vclock;
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
    void addCounter();

    std::string toAddress;
    Message message;
    int reCounter;
};

// ----------- AUX functions
std::string getTypeString(MessageType type);
MessageType stringToType(const std::string typeName);