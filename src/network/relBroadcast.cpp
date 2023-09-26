#include "include/relBroadcast.hpp"


// Init the class structure. Gets the IP from execution.
ReliableBroadcast::ReliableBroadcast(
    std::string nodeAddress,
    std::set<std::string> nodesGroup,
    std::function<void(RecvMessage)> reliableDelivery,
    std::function<void(std::string)> crashCallback
) {
    this->nodeAddress = nodeAddress;
    this->correctNodes = nodesGroup;
    this->deliveryCallback = reliableDelivery;

    // start FailureDetector
    std::function<void(Message)> brodcastCallback = [this](Message msg) {this->bestEffortBroadcast(msg);};
    this->failureDetector = std::make_shared<FailureDetector>(correctNodes, crashCallback, brodcastCallback);

    // Start ReliableLink connection
    std::function<void(RecvMessage)> deliverCallback = [this](RecvMessage msg) {this->deliverMessage(msg);};
    std::function<void(RecvMessage)> beatCallback = [this](RecvMessage msg) {this->failureDetector->handleBeatMessage(msg);};
    this->reliableLink = std::make_shared<ReliableLink>(deliverCallback, beatCallback);
}

// Stops the connection at the networking part of the class
void ReliableBroadcast::closeConnection() {
    reliableLink->stopLink();
    failureDetector->stopDetector();
}

// This is a standard broadcasting to all the nodes
void ReliableBroadcast::bestEffortBroadcast(Message message) {

    // (1) Get a safe copy of all the members
    membersLock.lock();
    std::set<std::string> copyMembers(this->correctNodes);
    membersLock.unlock();

    // (2) Broadcast message to all the other nodes
    std::set<std::string>::iterator it;
    for (it = copyMembers.begin(); it != copyMembers.end(); ++it) {
        std::string memberAddress = *it;
        if (this->nodeAddress != memberAddress) {
            reliableLink->sendMessage(memberAddress, message, false);
        }
    }
}

// Creates a message and uses the underlying connection to transmit it
void ReliableBroadcast::broadcastMessage(Message message) {

    // (1) Add the message to delivered and deliver (except when sending BEAT)
    if (message.getType() != BEAT) {
        deliverLock.lock();
        this->deliveredMessages.push_back(message);
        deliverLock.unlock();

        // (1.5) Deliver the message to the upper class
        RecvMessage recvMsg(this->nodeAddress, message);
        this->deliveryCallback(recvMsg);
    }

    // (2) Broadcast the message to all the other nodes
    this->bestEffortBroadcast(message);
}

// This is just the delivery to a bigger structure of the message
void ReliableBroadcast::deliverMessage(RecvMessage recvMessage) {
    std::string directSender = recvMessage.fromAddress;
    Message message = recvMessage.message;

    // (1) Make a safe copy of the delivered messages 
    deliverLock.lock();
    std::vector<Message> copyMessages(this->deliveredMessages);
    deliverLock.unlock();

    // (2) Check if the messages was previously delivered
    for (size_t i = 0; i < copyMessages.size(); ++i) {
        if (copyMessages[i] == message) { return; }
    }

    // (3) As it was not previously delivered, deliver it
    deliverLock.lock();
    this->deliveredMessages.push_back(message);
    deliverLock.unlock();

    // (3.5) Deliver the message to the upper class
    this->deliveryCallback(recvMessage);

    // (4) Check if the sender is still a correct process
    membersLock.lock();
    bool isCorrect = (correctNodes.find(directSender) != correctNodes.end());
    membersLock.unlock();

    // (5) Manage depending the case
    if (!isCorrect) {
        this->bestEffortBroadcast(message);
        return;
    }
    membersLock.lock();
    this->messagesFrom[directSender].push_back(message);
    membersLock.unlock();
}

// Manage the crashed node removing it from the group
void ReliableBroadcast::manageCrashedNode(std::string memberAddress) {
    std::cout << RED << "<< SYS: " << memberAddress << " has crashed! >>";
    std::cout << RESET << std::endl;

    // (1) Remove the member from correct processes
    membersLock.lock();
    correctNodes.erase(memberAddress);
    this->failureDetector->removeMember(memberAddress);

    // (1.5) Get all the messages received from the crashed process
    std::vector<Message> fromCrashed = messagesFrom[memberAddress];
    membersLock.unlock();

    // (2) Broadcast all the messages
    for (size_t i = 0; i < fromCrashed.size(); ++i) {
        Message toBroadcast = fromCrashed[i];
        this->bestEffortBroadcast(toBroadcast);
    }
}