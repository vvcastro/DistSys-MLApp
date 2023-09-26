#include "include/relCausalBroadcast.hpp"

// Init the class structure. Gets the IP from execution.
ReliableCausalBroadcast::ReliableCausalBroadcast(
    std::string nodeAddress,
    std::vector<std::string> nodesGroup,
    std::function<void(RecvMessage)> deliverCallback
) {
    this->nodeAddress = nodeAddress;
    this->deliverCallback = deliverCallback;

    // () Init correct groups as a Set
    std::set<std::string> groupAddrs;
    for (size_t i = 0; i < nodesGroup.size(); ++i) {
        groupAddrs.insert(nodesGroup[i]);
    }

    // Init the vector clock with zeros
    std::map<std::string, int> zeroClocks;
    for (size_t i = 0; i < nodesGroup.size(); ++i) {
        zeroClocks.insert(std::pair<std::string, int>(nodesGroup[i], 0));
    }
    this->vectorClock = zeroClocks;

    // Define the Broadcasting node
    std::function<void(RecvMessage)> rbCallback = [this](RecvMessage msg) {this->deliverRbMessage(msg);};
    std::function<void(std::string)> crashCallbak = [this](std::string crashed) {this->manageCrashedNode(crashed);};
    this->relBroadcast = std::make_shared<ReliableBroadcast>(nodeAddress, groupAddrs, rbCallback, crashCallbak);
}

// Stops the connection at the networking part of the class
void ReliableCausalBroadcast::closeConnection() {
    relBroadcast->closeConnection();
}

// Creates a message and uses the underlying connection to transmit it
void ReliableCausalBroadcast::broadcastMessage(Message message) {

    // (0) Add locks to the message
    clocksLock.lock();
    message.setClock(this->vectorClock);
    clocksLock.unlock();

    // (1) Deliver the message(except when sending BEAT)
    RecvMessage recvMsg(this->nodeAddress, message);
    this->deliverCallback(recvMsg);

    // (2) Broadcast message
    this->relBroadcast->broadcastMessage(message);

    // (3) Update node's clock
    clocksLock.lock();
    this->vectorClock[this->nodeAddress] += 1;
    clocksLock.unlock();
}

// This is just the delivery to a bigger structure of the message
void ReliableCausalBroadcast::deliverRbMessage(RecvMessage recvMessage) {
    std::string senderAddress = recvMessage.fromAddress;
    Message sentMessage = recvMessage.message;
    if (senderAddress == this->nodeAddress) { return; };

    // (1) Add to pending
    bool isPresent = false;
    for (size_t i = 0; i < pendingMessages.size(); ++i) {
        std::pair<std::string, Message> currentPair = pendingMessages[i];
        if ((currentPair.first == senderAddress) && (currentPair.second == sentMessage)) {
            isPresent = true;
            break;
        };
    }
    if (!isPresent) {
        std::pair<std::string, Message> toAdd(senderAddress, recvMessage.message);
        this->pendingMessages.push_back(toAdd);
    }

    // (2) Perform deliver-pending procedure
    this->deliverPendingProc();
}

void ReliableCausalBroadcast::deliverPendingProc() {
    bool stillGoing = true;
    while (stillGoing) {
        stillGoing = false;

        // Look if there is a pair with the conditions
        std::vector<std::pair<std::string, Message>>::iterator it;
        for (it = pendingMessages.begin(); it != pendingMessages.end();) {
            std::pair<std::string, Message> currentPair = *it;
            std::map<std::string, int> clockToCompare = currentPair.second.getClock();

            // Compare the clocks
            clocksLock.lock();
            bool before = this->isPreviousClock(clockToCompare);
            clocksLock.unlock();

            if (before) {
                stillGoing = true;

                // (1) Deliver the message
                RecvMessage recvMsg(currentPair.first, currentPair.second);
                this->deliverCallback(recvMsg);

                // (2) Update vectorClock of the sender
                clocksLock.lock();
                this->vectorClock[currentPair.first] += 1;
                clocksLock.unlock();

                // (3) Delete value from pending
                it = pendingMessages.erase(it);
                continue;
            }
            ++it;
        }
    }
}

void ReliableCausalBroadcast::manageCrashedNode(std::string crashedAddress) {

    // (1) Remove from view and all relevant structures
    this->relBroadcast->manageCrashedNode(crashedAddress);

}

// Just to have more modularisation in the managing of the classes
bool ReliableCausalBroadcast::isPreviousClock(std::map<std::string, int> otherClock) {
    for (const auto& pair1 : this->vectorClock) {
        const std::string& key = pair1.first;
        int value1 = pair1.second;

        // Find the corresponding key in otherClocks
        auto it2 = otherClock.find(key);
        if (it2 == otherClock.end() || value1 < it2->second) {
            return false;
        }
    }
    return true;
}
