#include "failureDetector.hpp"
#include "relLinks.hpp"
#include "messages.hpp"
#include <functional>
#include <algorithm>
#include <memory>
#include <set>
#include <map>


class ReliableBroadcast {
    public:
    ReliableBroadcast(
        std::string nodeAddress,
        std::set<std::string> nodesGroup,
        std::function<void(RecvMessage)> deliverCallback,
        std::function<void(std::string)> crashCallback
    );

    // Networking utilities
    void bestEffortBroadcast(Message message);
    void broadcastMessage(Message message);
    void closeConnection();

    // To change structures
    void deliverMessage(RecvMessage message);
    void manageCrashedNode(std::string memberAddress);

    private:

    // Basics for communication
    std::string nodeAddress;
    std::set<std::string> correctNodes;

    // For delivery of messages
    std::map<std::string, std::vector<Message>> messagesFrom;
    std::vector<Message> deliveredMessages;
    std::mutex deliverLock;

    // Callbacks to higher processes
    std::function<void(RecvMessage)> deliveryCallback;

    // For the management of crashed processes
    std::mutex membersLock;

    // Entities for ReliableBroadcast
    std::shared_ptr<ReliableLink> reliableLink;
    std::shared_ptr<FailureDetector> failureDetector;
};