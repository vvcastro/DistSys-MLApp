#include "include/comms.hpp"
#include "include/node.hpp"
#include <algorithm>

// Init the class structure. Gets the IP from execution.
Node::Node(std::string networkInt) {
    this->nodeAddress = getIPv4Address(networkInt);
    this->correctNodes = PROCESS_POOL;

    // Start the connection
    std::function<void(RecvMessage)> rcoCallback = [this](RecvMessage msg) {this->deliverMessage(msg);};
    this->connection = std::make_shared<ReliableCausalBroadcast>(this->nodeAddress, this->correctNodes, rcoCallback);
}

// Stops the connection at the networking part of the class
void Node::close() {
    connection->closeConnection();
}

// Creates a message and uses the underlying connection to transmit it
void Node::broadcastMessage(std::string data) {
    Message toSendMessage(this->nodeAddress, DATA, data);
    connection->broadcastMessage(toSendMessage);
}

void Node::deliverMessage(RecvMessage recvMessage) {
    recvMessage.displayMessage();
}

// -------- JUST SOME UTIL FUNCTIONS

// Prints the available connection data used by the Node
void Node::showConnectionData() {
    std::string sep(30, '=');
    std::cout << sep << std::endl;
    std::cout << "Node information:" << std::endl;
    std::cout << " - Address: " << nodeAddress << std::endl;
    std::cout << sep << std::endl;
}
