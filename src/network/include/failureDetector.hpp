#include "messages.hpp"
#include <functional>
#include <thread>
#include <mutex>
#include <set>
#include <map>

// A FailureDetector uses timing assumptions to provide
// suspicious about crashed processes
class FailureDetector {
    public:
    FailureDetector(
        std::set<std::string> nodesGroup,
        std::function<void(std::string)> suspectCallback,
        std::function<void(Message)> broadcastMessageCall
    );

    // Handle the timers when a BEAT is received
    void handleBeatMessage(RecvMessage recvMessage);

    // Status management
    void stopDetector();
    bool isRunning();

    // Members management
    void addNewMember(std::string memberAddress);
    void removeMember(std::string memberAddress);

    private:
    bool status;

    // Handle the group of processes
    std::mutex membersLock;
    std::set<std::string> correctNodes;

    // Define the timers for each process
    std::map<std::string, float> membersMaxWait;
    std::map<std::string, float> membersTimers;

    // Manage when a new BEAT message arrives
    std::function<void(std::string)> handleCrash;
    void inspectBeatings();

    // Defines the primitives for sending beats.
    std::function<void(Message)> broadcastMessage;
    void transmitHeartBeat();

    // relevant locks
    std::mutex statusLock;

};