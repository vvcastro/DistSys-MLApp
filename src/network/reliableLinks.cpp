#include "include/reliableLinks.hpp"
#include "comms.hpp"
#include "functional"

// Standard constructor for the class
ReliableLink::ReliableLink(
    std::function<void(RecvMessage)> deliveryMethod,
    std::function<void(RecvMessage)> beatManagement
) {
    this->deliveryMethod = deliveryMethod;
    this->beatManagement = beatManagement;

    // define sockets to use
    sendSocket = defineSenderSocket();
    recvSocket = defineReceiverSocket();

    // start listening thread
    this->status = true;
    std::thread listeningThread(std::bind(&ReliableLink::receivingChannel, this));
    std::thread stubbornThread(std::bind(&ReliableLink::stubbornResending, this));
    listeningThread.detach();
    stubbornThread.detach();
}

// Returns True is the Link is currently executing
bool ReliableLink::isRunning() {
    std::lock_guard<std::mutex> lock(statusLock);
    return status;
}

// Set the status to false, stoping the execution. Note the
// use of Locks to avoid concurrent reading while writing.
void ReliableLink::stopLink() {
    std::lock_guard<std::mutex> lock(statusLock);
    status = false;
    close(sendSocket);
    close(recvSocket);
}

// Send a message over the network to an specific toAddr;
void ReliableLink::sendMessage(std::string toAddress, Message message, bool resending) {

    // (1) Send the encoded message
    std::string encodedMessage = message.encodeToString();
    sendUDPMessage(sendSocket, toAddress, encodedMessage);

    if (resending) { return; }
    SentMessage waitingMessage(toAddress, message);
    waitingMessage.displayMessage();

    // (2) If we are sending a DATA message, add it to waiting messages
    if ((message.getType() == DATA) && (!resending)) {
        std::lock_guard<std::mutex> lock(waitingLock);
        waitingMessages.push_back(waitingMessage);
    }
}

// Creates the stubborn behaviour. Every X seconds re-sends all
// the messages that haven't received and ACK message in return.
void ReliableLink::stubbornResending() {
    while (isRunning()) {
        std::this_thread::sleep_for(std::chrono::duration<float>(RESEND_TIME_S));

        // Lock the waitingList to copy the messages to send
        waitingLock.lock();
        for (size_t i = 0; i < waitingMessages.size(); ++i) {
            waitingMessages[i].addCounter();
        }
        std::vector<SentMessage> toResendMessages(waitingMessages);
        waitingLock.unlock();

        // For each element on the list we sent the message inside
        for (size_t i = 0; i < toResendMessages.size(); ++i) {
            SentMessage msgToResend = toResendMessages[i];
            sendMessage(msgToResend.toAddress, msgToResend.message, true);
        }
    }
}

// Start the listening for messages over the network. It assures that
// messages are received only once.
void ReliableLink::receivingChannel() {

    // Bind the socket to start listening
    setupReceiverSocket(recvSocket);

    while (isRunning()) {
        struct sockaddr_in sourceAddr;
        socklen_t sourceLen = sizeof(sourceAddr);

        // Read the message if available
        char senderIP[INET_ADDRSTRLEN];
        char msgBuffer[MAX_MSG_SIZE];
        int bytesReceived = recvfrom(recvSocket, msgBuffer, sizeof(msgBuffer), 0, (struct sockaddr*)&sourceAddr, &sourceLen);
        if (bytesReceived == -1) {
            close(recvSocket);
            throw std::runtime_error("Could not read @RL-listener");
        };

        // Get the delivery info into strings
        inet_ntop(AF_INET, &(sourceAddr.sin_addr), senderIP, INET_ADDRSTRLEN);
        std::string encodedMessage(msgBuffer, bytesReceived);
        std::string fromAddress(senderIP);

        // Decode it into a message class structure
        Message decodedMessage = Message::decodeToMessage(encodedMessage);
        RecvMessage recvMessage = RecvMessage(fromAddress, decodedMessage);
        handleMessage(recvMessage);
    }
}

// Performs the preprocessing of the message depending on the type
void ReliableLink::handleMessage(RecvMessage recvMessage) {
    switch (recvMessage.message.getType()) {
        case DATA:
            handleDataMessage(recvMessage);
            return;

        case ACK:
            handleACKMessage(recvMessage);
            return;

        case BEAT:
            beatManagement(recvMessage);
            return;

        default:
            // Any other type of message is not manage by this class
            return;
    }
}

// A "new" data message was received: 
void ReliableLink::handleDataMessage(RecvMessage recvMesage) {
    recvMesage.displayMessage();

    // (1) Send ACK (even on repetition)
    Message respondData = Message::getRespondMessage(recvMesage.message);
    sendMessage(recvMesage.fromAddress, respondData, false);

    // (2) Check if it should be received
    bool wasReceived = false;
    for (size_t i = 0; i < recvMessages.size(); i++) {
        if (recvMessages[i] == recvMesage) {
            wasReceived = true;
            return;
        }
    }

    // Adds the message to the list of received and runs the delivery
    if (!wasReceived) {
        std::string messageStr = recvMesage.message.encodeToString();
        this->recvMessages.push_back(recvMesage);
        this->deliveryMethod(recvMesage);
    }
}

// When receive an ACK message remove the message from the waiting list.
void ReliableLink::handleACKMessage(RecvMessage recvMesage) {
    recvMesage.displayMessage();

    // (1) Lock the waitingList to copy the messages to check
    waitingLock.lock();
    std::vector<SentMessage> toCheckMessages(waitingMessages);
    waitingLock.unlock();

    // (2) Iterate and check if the message is waiting
    int indexToRemove;
    bool isWaiting = false;
    for (size_t i = 0; i < waitingMessages.size(); i++) {
        SentMessage currentMessage = waitingMessages[i];
        if (currentMessage.isResponse(recvMesage)) {
            isWaiting = true;
            indexToRemove = i;
            break;
        }
    }
    if (!isWaiting) { return; }

    // (3) Remove the message from the current list using the lock
    std::string messageStr = recvMesage.message.encodeToString();
    std::lock_guard<std::mutex> lock(waitingLock);
    waitingMessages.erase(waitingMessages.begin() + indexToRemove);
}
