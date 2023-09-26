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
        std::vector<std::string> nodesGroup
    );

    // Networking utilities
    void broadcastMessage(Message message);
    void deliverMessage(RecvMessage message);
    void closeConnection();

    private:

    // Basics for communication
    std::string nodeAddress;
    std::shared_ptr<ReliableBroadcast> relBroadcast;

    // For delivery of messages
    std::set<std::pair<std::string, Message>> pendingMessages;
    std::vector<Message> deliveredMessages;
    std::mutex deliverLock;

    // Vector Clocks for causal order
    std::map<std::string, int> vectorClock;
    bool isPreviousClock(std::map<std::string, int> otherClock);


};