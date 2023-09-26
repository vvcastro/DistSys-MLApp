#include "include/relVSBroadcast.hpp"

// Init the class structure. Gets the IP from execution.
ReliableVSBroadcast::ReliableVSBroadcast(
    std::string nodeAddress,
    std::vector<std::string> nodesGroup,
    std::function<void(RecvMessage)> deliverCallback
) {
    this->nodeAddress = nodeAddress;
    this->deliverCallback = deliverCallback;

    // (0) Init Views
    std::set<std::string> groupAddrs;
    for (size_t i = 0; i < nodesGroup.size(); ++i) {
        groupAddrs.insert(nodesGroup[i]);
    }
    this->correctNodes = groupAddrs;
    this->groupView = std::pair<int, std::set<std::string>>(0, this->correctNodes);

    // Define the Broadcasting node
    std::function<void(std::string)> crashCallbak = [this](std::string crashed) {this->manageCrashedNode(crashed);};
    std::function<void(RecvMessage)> rbCallback = [this](RecvMessage msg) {this->deliverRbMessage(msg);};
    this->relBroadcast = std::make_shared<ReliableBroadcast>(nodeAddress, correctNodes, rbCallback, crashCallbak);
}

// Stops the connection at the networking part of the class
void ReliableVSBroadcast::closeConnection() {
    relBroadcast->closeConnection();
}

// Creates a message and uses the underlying connection to transmit it
void ReliableVSBroadcast::broadcastMessage(Message message) {

    if (!isBlock()) {

        // (1) Add the message to delivered
        message.setGroupView(this->groupView.first);
        deliverLock.lock();
        this->deliveredMessages.push_back(message);
        deliverLock.unlock();

        // (2) Deliver the message to the upper class
        RecvMessage recvMsg(this->nodeAddress, message);
        this->deliverCallback(recvMsg);

        // (3) Broadcast the message
        this->relBroadcast->broadcastMessage(message);
    }
}

// This is just the delivery to a bigger structure of the message
void ReliableVSBroadcast::deliverRbMessage(RecvMessage recvMessage) {
    Message message = recvMessage.message;
    MessageType msgType = message.getType();
    std::string directSender = recvMessage.fromAddress;

    if (msgType == DATA) {
        // (1) Make a safe copy of the delivered messages 
        deliverLock.lock();
        std::vector<Message> copyMessages(this->deliveredMessages);
        deliverLock.unlock();

        // (2) Check (isDelivered) && (sameView) && (notBlocked)
        for (size_t i = 0; i < copyMessages.size(); ++i) {
            if (copyMessages[i] == message) { return; }
        }
        if (this->groupView.first != message.getViewId()) { return; }
        if (this->isBlock()) { return; }

        // (3) Deliver the message
        deliverLock.lock();
        this->deliveredMessages.push_back(message);
        deliverLock.unlock();
        this->deliverCallback(recvMessage);
        return;
    }

    if (msgType == DSET) {

        // (1) Add to DSET if necessary
        this->appendToDSET(directSender, message.getDSET());

        // (2) Check if can propose a new View
        for (auto nodeAddress : this->correctNodes) {
            for (auto pairData : this->deliveredSet) {
                if (pairData.first == nodeAddress) {
                    this->triggerChangeViewProposal();
                    return;
                }
            }
        }
    }

}



// Manages the event of a crashing process ( should also interact 
// with ReliableBroadcast method )
void ReliableVSBroadcast::manageCrashedNode(std::string crashedAddress) {

    // (1) Remove from view and all relevant structures
    membersLock.lock();
    correctNodes.erase(crashedAddress);
    this->relBroadcast->manageCrashedNode(crashedAddress);
    membersLock.unlock();

    // (2) Check flushing
    if (isFlushing()) { return; }
    statusLock.lock();
    flushing = true;
    statusLock.unlock();

    // (3) Trigger the blocking primitive
    this->triggerBlocking();
}

// Triggers the blocking and broadcasts the DSET message
void ReliableVSBroadcast::triggerBlocking() {

    // (1) Blocked the node
    statusLock.lock();
    blocked = true;
    statusLock.unlock();

    // (2) Broadcast the message
    Message dsetMessage(this->nodeAddress, DSET, "");
    dsetMessage.setDSET(this->deliveredMessages);
    dsetMessage.setGroupView(this->groupView.first);
    this->relBroadcast->broadcastMessage(dsetMessage);
}

void ReliableVSBroadcast::triggerChangeViewProposal() {

}

void ReliableVSBroadcast::appendToDSET(
    std::string srcAddress,
    std::vector<Message> messages
) {
    // (1) Make the pair
    std::pair<std::string, std::vector<Message>> toAppend(srcAddress, messages);

    // (2) Check if the tuple is already present
    std::vector<std::pair<std::string, std::vector<Message>>>::iterator it;
    for (it = deliveredSet.begin(); it != deliveredSet.end(); ++it) {
        std::pair<std::string, std::vector<Message>> currPair = *it;
        if (currPair.first != toAppend.first) { continue; }

        // (3) Node to check that all the messages are contained
        std::vector<Message>::iterator itm;
        for (itm = messages.begin(); itm != messages.end(); ++itm) {
            Message currMsg = *itm;
            for (const Message& msg : currPair.second) {
                if (currMsg == msg) { return; }
            }
        }
    }

    // (3) Add the pair to the strucutre
    this->deliveredSet.push_back(toAppend);
}



bool ReliableVSBroadcast::isBlock() {
    std::lock_guard<std::mutex> lock(this->statusLock);
    return blocked;
}

bool ReliableVSBroadcast::isFlushing() {
    std::lock_guard<std::mutex> lock(this->statusLock);
    return flushing;
}