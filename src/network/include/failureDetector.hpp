#include "messages.hpp"
#include <functional>
#include <thread>
#include <mutex>
#include <map>

// A FailureDetector uses timing assumptions to provide
// suspicious about crashed processes
class FailureDetector {
    public:
    FailureDetector(
        std::string nodeAddress,
        std::vector<std::string> nodesGroup,
        std::function<void(std::string)> suspectCallback
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
    std::string nodeAddress;

    // Handle the group of processes
    std::mutex membersLock;
    std::vector<std::string> currentGroup;

    // Define the timers for each process
    std::map<std::string, float> membersMaxWait;
    std::map<std::string, float> membersTimers;

    // Manage when a new BEAT message arrives
    std::function<void(std::string)> suspectCrash;
    void inspectBeatings();

    // Defines the primitives for sending beats.
    int sendSocket;
    void heartBeating();

    // relevant locks
    std::mutex statusLock;

};