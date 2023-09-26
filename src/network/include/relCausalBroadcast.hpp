#include "relBroadcast.hpp"
#include "messages.hpp"
#include <functional>
#include <memory>
#include <map>
#include <set>


class ReliableCausalBroadcast {
    public:
    ReliableCausalBroadcast(
        std::string nodeAddress,
        std::vector<std::string> nodesGroup,
        std::function<void(RecvMessage)> deliverCallback
    );

    // Networking utilities
    void broadcastMessage(Message message);
    void deliverRbMessage(RecvMessage message);
    void closeConnection();

    private:

    // Basics for communication
    std::string nodeAddress;
    std::shared_ptr<ReliableBroadcast> relBroadcast;
    void manageCrashedNode(std::string crashedAddress);

    // For delivery of messages
    std::function<void(RecvMessage)> deliverCallback;
    std::vector<std::pair<std::string, Message>> pendingMessages;
    void deliverPendingProc();

    // Vector Clocks for causal order
    bool isPreviousClock(std::map<std::string, int> otherClock);
    std::map<std::string, int> vectorClock;
    std::mutex clocksLock;
};