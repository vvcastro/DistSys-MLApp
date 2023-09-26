#include "relBroadcast.hpp"
#include "messages.hpp"
#include <functional>
#include <algorithm>
#include <memory>
#include <set>
#include <map>

// This class implements ReliableBroadcasting with
// View-Synchrnoy consensus.
class ReliableVSBroadcast {
    public:
    ReliableVSBroadcast(
        std::string nodeAddress,
        std::vector<std::string> nodesGroup,
        std::function<void(RecvMessage)> deliverCallback
    );

    // Networking utilities
    void broadcastMessage(Message message);
    void closeConnection();

    // To change structures
    void deliverRbMessage(RecvMessage message);
    bool isBlock();
    bool isFlushing();

    private:

    // Basics for communication
    std::string nodeAddress;
    std::set<std::string> correctNodes;
    std::pair<int, std::set<std::string>> groupView;
    std::shared_ptr<ReliableBroadcast> relBroadcast;

    // For View-Synch
    void triggerBlocking();
    void triggerChangeViewProposal();
    std::mutex statusLock;
    bool flushing;
    bool blocked;

    // For delivery managements
    std::vector<std::pair<std::string, std::vector<Message>>> deliveredSet;
    std::vector<Message> deliveredMessages;
    std::mutex deliverLock;

    // This is the delivery of the whole system
    std::function<void(RecvMessage)> deliverCallback;

    // For the management of crashed processes
    std::mutex membersLock;
    void manageCrashedNode(std::string memberAddress);

    // Some helper functions
    void appendToDSET(std::string srcAddress, std::vector<Message> messages);
};