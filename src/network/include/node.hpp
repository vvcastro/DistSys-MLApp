#include "reliableLinks.hpp"
#include <memory>

class Node {
    public:
    Node(std::string networkInt);
    void showConnectionData();
    void closeConnection();

    // Networking utilities
    void broadcastMessage(std::string data);

    // Structure will be updated
    std::vector<RecvMessage> deliveredMessages;
    void deliverMessage(RecvMessage message);

    private:
    std::string nodeAddress;
    std::shared_ptr<ReliableLink> connection;

    // Aware of the GroupView
    std::vector<std::string> correctNodes;

    // For synchronisation
    std::vector<std::pair<std::string, int>> vectorClocks;
    void updateVectorClock();

    // Lock for delivery of messages
    std::mutex deliveryLock;
};