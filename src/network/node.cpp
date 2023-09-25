#include "include/node.hpp"

// Init the class structure. Gets the IP from execution.
Node::Node(std::string networkInt) {
    this->nodeAddress = getIPv4Address(networkInt);

    // Define the POOL of processes
    this->correctNodes = PROCESS_POOL;

    // Start the connection
    std::function<void(RecvMessage)> callback = [this](RecvMessage msg) {this->deliverMessage(msg);};
    this->connection = std::make_shared<ReliableLink>(callback);
}

// Stops the connection at the networking part of the class
void Node::closeConnection() {
    connection->stopLink();
}

// Prints the available connection data used by the Node
void Node::showConnectionData() {
    std::string sep(30, '=');
    std::cout << sep << std::endl;
    std::cout << "Node information:" << std::endl;
    std::cout << " - Address: " << nodeAddress << std::endl;
    std::cout << sep << std::endl;
}

// Creates a message and uses the underlying connection to transmit it
void Node::broadcastMessage(std::string data) {
    Message toSendMessage = Message(nodeAddress, DATA, data);
    toSendMessage.setClock(this->vectorClocks);

    // Broadcast the message
    for (size_t i = 0; i < correctNodes.size(); i++) {
        std::string otherAddress = correctNodes[i];
        if (this->nodeAddress != otherAddress) {
            connection->sendMessage(otherAddress, toSendMessage, false);
        }
    }
}

// Constructs the vector clocks object as a vector of tuples
void Node::updateVectorClock() {
    return;
}

// This is just the delivery to a bigger structure of the message
void Node::deliverMessage(RecvMessage recvMessage) {
    std::lock_guard<std::mutex> lock(deliveryLock);
    this->deliveredMessages.push_back(recvMessage);
}
