#include "include/comms.hpp"
#include "include/node.hpp"
#include <algorithm>

// Init the class structure. Gets the IP from execution.
Node::Node(std::string networkInt) {
    this->nodeAddress = getIPv4Address(networkInt);
    this->correctNodes = PROCESS_POOL;

    // Start Failure detector
    std::function<void(std::string)> suspectCallback = [this](std::string address) {this->alertSuspicious(address);};
    this->detector = std::make_shared<FailureDetector>(nodeAddress, this->correctNodes, suspectCallback);

    // Start the connection
    std::function<void(RecvMessage)> deliverCallback = [this](RecvMessage msg) {this->deliverMessage(msg);};
    std::function<void(RecvMessage)> beatCallback = [this](RecvMessage msg) {this->detector->handleBeatMessage(msg);};
    this->connection = std::make_shared<ReliableLink>(deliverCallback, beatCallback);
}

// Stops the connection at the networking part of the class
void Node::closeConnection() {
    connection->stopLink();
    detector->stopDetector();
}

// Creates a message and uses the underlying connection to transmit it
void Node::broadcastMessage(std::string data) {
    Message toSendMessage(this->nodeAddress, DATA, data);
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

// This is the manangement of (suspected) crashed elements
void Node::alertSuspicious(std::string memberAddress) {
    std::cout << RED << "<< SYS: " << memberAddress << " has crashed! >>";
    std::cout << RESET << std::endl;

    // Remove from detector
    std::lock_guard<std::mutex> lock(groupLock);
    this->detector->removeMember(memberAddress);

    // Remove from set of correct nodes
    std::vector<std::string>::iterator pos = std::find(
        correctNodes.begin(), correctNodes.end(), memberAddress
    );
    correctNodes.erase(pos);
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
