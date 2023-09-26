#include "relCausalBroadcast.hpp"
#include <memory>

class Node {
    public:
    Node(std::string networkInt);

    // Connection managements
    void showConnectionData();
    void close();

    // Networking utilities
    void broadcastMessage(std::string data);

    private:
    std::string nodeAddress;
    std::vector<std::string> correctNodes;
    std::shared_ptr<ReliableCausalBroadcast> connection;

};