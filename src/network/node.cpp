#include "include/node.hpp"

// Init the class structure. Gets the IP from execution.
Node::Node(std::string nodeId, std::string networkInt) {
    this->nodeId = nodeId;
    this->nodeAddress = getIPv4Address(networkInt);

    // Start the connection
    std::function<void(RecvMessage)> callback = [this](RecvMessage msg) {this->deliverMessage(msg);};
    this->connection = std::make_shared<ReliableLink>(nodeId, callback);
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
    std::cout << " - NodeId: " << nodeId << std::endl;
    std::cout << " - IPv4: " << nodeAddress << std::endl;
    std::cout << sep << std::endl;
}

// Creates a message and uses the underlying connection to transmit it
void Node::sendMessage(std::string toAddress, std::string data) {
    Message toSendMessage = Message(nodeId, DATA, data);
    toSendMessage.setClock(this->vectorClocks);
    connection->sendMessage(toAddress, toSendMessage, false);
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
