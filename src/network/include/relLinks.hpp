#include "messages.hpp"
#include <functional>
#include <thread>
#include <mutex>

// A ReliableLink ensure that messages between CORRECT
// processes are NOT-LOST.
class ReliableLink {
    public:
    ReliableLink(
        std::function<void(RecvMessage)> deliveryMethod,
        std::function<void(RecvMessage)> beatManagement
    );
    void sendMessage(std::string toAddress, Message message, bool resending);

    // Status management
    bool isRunning();
    void stopLink();

    private:
    bool status;
    std::vector<SentMessage> waitingMessages;
    std::vector<RecvMessage> recvMessages;

    // These are callbacks to handle events at an upper level
    std::function<void(RecvMessage)> deliveryMethod;
    std::function<void(RecvMessage)> beatManagement;

    // define the sockets for communication
    int sendSocket;
    int recvSocket;

    // relevant locks
    std::mutex statusLock;
    std::mutex waitingLock;

    // handle different types of messages
    void handleMessage(RecvMessage RecvMessage);
    void handleDataMessage(RecvMessage RecvMessage);
    void handleACKMessage(RecvMessage RecvMessage);

    void stubbornResending();
    void receivingChannel();
};