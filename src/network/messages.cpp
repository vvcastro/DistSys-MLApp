#include "include/messages.hpp"
#include <map>

// Standard constructor for the class
Message::Message(std::string sender, MessageType type, std::string data) {
    this->sender = sender;
    this->type = type;
    this->data = data;
}

// Encodes the data into a string
std::string Message::encodeToString() {
    std::string encoded;
    encoded += "(" + getTypeString(type) + ")";
    encoded += "|" + sender + "|" + data + "|-|";
    return encoded;
}

// Decodes the encrypted data transmited over the network.
Message Message::decodeToMessage(std::string encodedMessage) {
    std::istringstream stream(encodedMessage);

    // Parse the string
    std::string strType, senderId, msgData, vclock;
    stream.ignore(1);
    std::getline(stream, strType, ')');
    stream.ignore(1);
    std::getline(stream, senderId, '|');
    std::getline(stream, msgData, '|');
    std::getline(stream, vclock, '|');

    // Re-construct the message
    MessageType msgType = stringToType(strType);
    return Message(senderId, msgType, msgData);
}

// Gets the respondData for a given message
Message Message::getRespondMessage(const Message& other) {
    Message respond(other.sender, ACK, other.data);
    respond.setClock(other.vclock);
    return respond;
}

// Define the create of the capsule for sent messages
SentMessage::SentMessage(std::string toAddress, Message message) {
    this->toAddress = toAddress;
    this->message = message;
    this->reCounter = 0;
}

// Define the create of the capsule for received messages
RecvMessage::RecvMessage(std::string fromAddress, Message message) {
    this->fromAddress = fromAddress;
    this->message = message;
}

// Prints the SentMessage
void SentMessage::displayMessage() {
    std::string messageStr = this->message.encodeToString();
    std::cout << MAGENTA << "<To: " << this->toAddress << " {";
    std::cout << messageStr << "}" << RESET << std::endl;
}

// Prints the RecvMessage
void RecvMessage::displayMessage() {
    std::string messageStr = this->message.encodeToString();
    std::cout << GREEN << ">From: " << this->fromAddress << " {";
    std::cout << messageStr << "}" << RESET << std::endl;
}

// Check if a message is a response (==data) and sender and receiver are swaped.
bool SentMessage::isResponse(RecvMessage other) {
    bool msg_eq = (message == other.message);
    return (toAddress == other.fromAddress) && msg_eq;
}

// Set the VectorClocks from the sender. Note: will be done under a lock.
void Message::setClock(std::vector<std::pair<std::string, int> > vclock) {
    this->vclock = vclock;
}

// Add a counter for a reSent message, just to keep track of stats
void SentMessage::addCounter() {
    ++reCounter;
}

// Overrides the default == operator
bool Message::operator==(const Message& other) {
    bool sender_eq = (sender == other.sender);
    bool data_eq = (data == other.data);
    bool clock_eq = (vclock == other.vclock);
    return sender_eq && data_eq && clock_eq;
}

// For the RecvMessage class, check if two messages are the same.
bool RecvMessage::operator==(const RecvMessage& other) {
    return (fromAddress == other.fromAddress) && (message == other.message);
}


// ----------- AUX functions
std::string getTypeString(MessageType type) {
    switch (type) {
        case DATA:
            return "DATA";
        case ACK:
            return "ACK";
        case BEAT:
            return "BEAT";
        default:
            return "";
    }
}

MessageType stringToType(const std::string typeName) {

    // Define an standard mapping
    std::map<std::string, MessageType> typeMaps = {
        {"DATA", DATA},
        {"ACK", ACK},
        {"BEAT", BEAT}
    };

    // Find the matching values
    auto it = typeMaps.find(typeName);
    if (it != typeMaps.end()) {
        return it->second;
    }
    return INVALID;
}
