#include "basicComms.hpp"
#include "messages.hpp"
#include <thread>
#include <mutex>

// A ReliableLink ensure that messages between CORRECT
// processes are NOT-LOST.
class ReliableLink {
    public:
    ReliableLink(std::string nodeId, std::function<void(RecvMessage)> deliveryMethod);
    void sendMessage(std::string toAddress, Message message, bool resending);
    bool isRunning();
    void stopLink();

    private:
    bool status;
    std::string nodeId;
    std::vector<SentMessage> waitingMessages;
    std::vector<RecvMessage> RecvMessages;

    // This is the upper connection, delivers the message asynchrnously
    std::function<void(RecvMessage)> deliveryMethod;

    // define the sockets for communication
    int sendSocket;
    int recvSocket;

    // relevant locks
    std::mutex statusLock;
    std::mutex waitingLock;

    void handleMessage(RecvMessage RecvMessage);
    void handleDataMessage(RecvMessage RecvMessage);
    void handleACKMessage(RecvMessage RecvMessage);

    void stubbornResending();
    void receivingChannel();
};