#include <iostream>
#include <utility>
#include <vector>

enum MessageType {
    DATA,
    ACK,
    BEAT,
};

// Contains all the data that will be transmited over the network
class Message {
    public:
    Message() {}
    Message(const std::string sender, const MessageType type, const std::string data);
    Message(const std::string encodedMessage); // decode string into message
    Message(const Message& other);

    // To manage internal data
    MessageType getType();
    void setType(MessageType type);
    void setClock(std::vector<std::pair<std::string, int> > vclocks);

    // To be able to exchange the message over the network
    std::string encode();

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
    std::string fromAddress;
    Message message;
    bool operator==(const RecvMessage& other);
};

// Encaptulation of a sent message, it ease the management
// at the sending node.
class SentMessage {
    public:
    SentMessage(std::string toAddress, Message message);
    std::string toAddress;
    Message message;
    bool isResponse(RecvMessage other);
};