#include "failureDetector.hpp"
#include "reliableLinks.hpp"
#include "messages.hpp"
#include <functional>
#include <algorithm>
#include <set>
#include <map>


class ReliableBroadcast {
    public:
    ReliableBroadcast(
        std::string nodeAddress,
        std::vector<std::string> nodesGroup
    );

    // Networking utilities
    void bestEffortBroadcast(Message message);
    void broadcastMessage(Message message);
    void closeConnection();

    // Structure will be updated
    void deliverMessage(RecvMessage message);

    private:

    // Basics for communication
    std::string nodeAddress;
    std::set<std::string> correctNodes;

    // For delivery of messages
    std::map<std::string, std::vector<Message>> messagesFrom;
    std::vector<Message> deliveredMessages;
    std::mutex deliverLock;

    // For the management of crashed processes
    std::mutex membersLock;
    void manageCrashedNode(std::string memberAddress);

    // Entities for ReliableBroadcast
    std::shared_ptr<ReliableLink> reliableLink;
    std::shared_ptr<FailureDetector> failureDetector;
};