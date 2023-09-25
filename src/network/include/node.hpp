#include "reliableLinks.hpp"
#include <memory>

class Node {
    public:
    Node(std::string nodeId, std::string networkInt);
    void showConnectionData();

    void sendMessage(std::string toAddress, std::string data);
    void closeConnection();

    // Structure will be updated
    std::vector<RecvMessage> deliveredMessages;
    void deliverMessage(RecvMessage message);

    private:

    std::string nodeId;
    std::string nodeAddress;
    std::shared_ptr<ReliableLink> connection;

    // For synchronisation
    std::vector<std::pair<std::string, int>> vectorClocks;
    void updateVectorClock();

    // Lock for delivery of messages
    std::mutex deliveryLock;
};