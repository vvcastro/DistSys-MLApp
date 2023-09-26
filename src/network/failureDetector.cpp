#include "include/failureDetector.hpp"
#include "include/comms.hpp"
#include <algorithm>
#include <vector>
#include <set>
#include <map>

FailureDetector::FailureDetector(
    std::set<std::string> nodesGroup,
    std::function<void(std::string)> suspectCallback,
    std::function<void(Message)> broadcastMessageCall
) {
    this->status = true;
    this->correctNodes = nodesGroup;
    this->handleCrash = suspectCallback;
    this->broadcastMessage = broadcastMessageCall;

    // Define an initial waitTime for beats
    std::map<std::string, float> initialMaxTimes;
    std::map<std::string, float> initialTimes;

    std::set<std::string>::iterator it;
    for (it = correctNodes.begin(); it != correctNodes.end(); ++it) {
        initialMaxTimes.insert(make_pair(*it, HEARTBEAT_PERIOD * 3));
        initialTimes.insert(make_pair(*it, -100));
    };
    this->membersMaxWait = initialMaxTimes;
    this->membersTimers = initialTimes;

    // define elements for hearBeating
    std::thread beatingThread(std::bind(&FailureDetector::transmitHeartBeat, this));
    beatingThread.detach();

    // periodically checks for the timeouts
    std::thread inspectionThread(std::bind(&FailureDetector::inspectBeatings, this));
    inspectionThread.detach();
}

// Return True is the detector is active
bool FailureDetector::isRunning() {
    std::lock_guard<std::mutex> lock(statusLock);
    return status;
}

// Changes safely the status of the detector
void FailureDetector::stopDetector() {
    std::lock_guard<std::mutex> lock(statusLock);
    status = false;
}

// Every X seconds the thread broadcast BEAT message to all the members
void FailureDetector::transmitHeartBeat() {
    while (isRunning()) {
        std::this_thread::sleep_for(std::chrono::duration<float>(HEARTBEAT_PERIOD));
        Message beatMessage("", BEAT, "");
        this->broadcastMessage(beatMessage);
    }
}

// When a new BEAT message arrives, update the related timeout
void FailureDetector::handleBeatMessage(RecvMessage recvMessage) {
    std::string mAddress = recvMessage.fromAddress;
    float currentTimeOut;
    float maxTimeOut;

    // Find the related time outs
    std::lock_guard<std::mutex> lock(membersLock);
    std::map<std::string, float>::iterator pos = membersTimers.find(mAddress);
    std::map<std::string, float>::iterator maxPos = membersMaxWait.find(mAddress);

    // In this case we found a process joining the group
    if (pos == membersTimers.end() && maxPos == membersMaxWait.end()) {
        std::cout << GREEN << "SYS: new process found -> " << mAddress << RESET << std::endl;
        return;
    };
    currentTimeOut = pos->second;
    maxTimeOut = maxPos->second;

    // First BEAT from Node
    if (currentTimeOut == -100) {
        pos->second = maxTimeOut;
        return;
    }

    // Update the maxTimeOut for the node
    float newTime = 2 * (maxTimeOut - currentTimeOut);
    maxPos->second = std::max(newTime, maxTimeOut);
    pos->second = maxPos->second;
}

// Checks periodically for the timeouts of the group processes
void FailureDetector::inspectBeatings() {
    float samplingTime = HEARTBEAT_PERIOD / 2;
    while (isRunning()) {
        std::this_thread::sleep_for(std::chrono::duration<float>(samplingTime));
        std::vector<std::string> susMembers;

        // (1) Iterate to update timer values
        membersLock.lock();
        std::map<std::string, float>::iterator pos;
        for (pos = membersTimers.begin(); pos != membersTimers.end(); ++pos) {
            if (pos->second == -100) { continue; };
            pos->second -= samplingTime;

            // If the timeout hasn't expired we are fine
            std::string susAddress = pos->first;
            if (pos->second > 0) { continue; }

            std::map<std::string, float>::iterator maxPos = membersMaxWait.find(susAddress);
            if (pos->second <= (-1.5 * maxPos->second)) {
                susMembers.push_back(pos->first);
            }
        }
        membersLock.unlock();

        // (2) Now we work with the process to be removed
        for (size_t i = 0; i < susMembers.size(); ++i) {
            std::string susAddress = susMembers[i];
            handleCrash(susAddress);
        }
    }
}

// Add a new member to the group so the Detector can also track
// and send BEATS to that node.
void FailureDetector::addNewMember(std::string memberAddress) {
    std::lock_guard<std::mutex> lock(membersLock);
    membersMaxWait.insert(make_pair(memberAddress, HEARTBEAT_PERIOD * 3));
    membersTimers.insert(make_pair(memberAddress, -100));
    correctNodes.insert(memberAddress);
}

// Deletes a member from the group
void FailureDetector::removeMember(std::string memberAddress) {
    std::lock_guard<std::mutex> lock(membersLock);
    membersMaxWait.erase(memberAddress);
    membersTimers.erase(memberAddress);
    correctNodes.erase(memberAddress);
}