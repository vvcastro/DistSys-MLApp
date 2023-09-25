#include "include/messages.hpp"

// Standard constructor for the class
Message::Message(const std::string sender, const MessageType type, const std::string data) {
    this->sender = sender;
    this->type = type;
    this->data = data;
}

// Construction by copying another instance
Message::Message(const Message& other) {
    sender = other.sender;
    type = other.type;
    data = other.data;
    vclock = other.vclock;
}

// This final type of construction is the decoding from a string
// transmited over the network.
Message::Message(std::string encodedMessage) {
    sender = "";
    type = DATA;
    data = "";
}

// Define the create of the capsule for sent messages
SentMessage::SentMessage(std::string toAddress, Message message) {
    this->toAddress = toAddress;
    this->message = message;
}

// Define the create of the capsule for received messages
RecvMessage::RecvMessage(std::string fromAddress, Message message) {
    this->fromAddress = fromAddress;
    this->message = message;
}

// Set the VectorClocks from the sender. Note: will be done under a lock.
void Message::setClock(std::vector<std::pair<std::string, int> > vclock) {
    this->vclock = vclock;
}

// Return the data type of the message. It is just a wrapper
// to protect the assignment of the value
MessageType Message::getType() {
    return type;
}

// Sets the message type to the required (usually ACK)
void Message::setType(MessageType type) {
    this->type = type;
}


// Encodes the data into a string
std::string Message::encode() {
    return "TODO";
}

// Overrides the default == operator
bool Message::operator==(const Message& other) {
    bool sender_eq = (sender == other.sender);
    bool data_eq = (data == other.data);
    bool clock_eq = (vclock == other.vclock);
    return sender_eq && data_eq && clock_eq;
}

// For the RecvMessage class, check if two messages
// are the same.
bool RecvMessage::operator==(const RecvMessage& other) {
    return (fromAddress == other.fromAddress) && (message == other.message);
}

// Check if a message is a response if the data is the same
// but sender and receiver are swaped.
bool SentMessage::isResponse(RecvMessage other) {
    bool msg_eq = (message == other.message);
    return (toAddress == other.fromAddress) && msg_eq;
}
