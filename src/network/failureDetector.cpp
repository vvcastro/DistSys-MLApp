#include "include/failureDetector.hpp"
#include "comms.hpp"
#include <vector>
#include <map>

FailureDetector::FailureDetector(
    std::string nodeAddress,
    std::vector<std::string> nodesGroup,
    std::function<void(std::string)> suspectCallback
) {
    this->status = true;
    this->nodeAddress = nodeAddress;
    currentGroup = nodesGroup;
    suspectCrash = suspectCallback;

    // Define an initial waitTime for beats
    std::map<std::string, float> initialMaxTimes;
    std::map<std::string, float> initialTimes;
    for (size_t i = 0; i < currentGroup.size(); i++) {
        initialMaxTimes.insert(make_pair(currentGroup[i], HEARTBEAT_PERIOD * 10));
        initialTimes.insert(make_pair(currentGroup[i], -1));
    };
    this->membersMaxWait = initialMaxTimes;
    this->membersTimers = initialTimes;

    // define elements for hearBeating
    sendSocket = defineSenderSocket();
    std::thread beatingThread(std::bind(&FailureDetector::heartBeating, this));
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

// When a new BEAT message arrives, update the timeout
// related to the process
void FailureDetector::handleBeatMessage(RecvMessage recvMessage) {
    std::string mAddress = recvMessage.fromAddress;
    float currentTimeOut;
    float maxTimeOut;

    std::lock_guard<std::mutex> lock(membersLock);

    // Find the related time outs
    std::map<std::string, float>::iterator pos = membersTimers.find(mAddress);
    std::map<std::string, float>::iterator maxPos = membersMaxWait.find(mAddress);
    if (pos == membersTimers.end()) { return; };
    if (maxPos == membersMaxWait.end()) { return; };
    currentTimeOut = pos->second;
    maxTimeOut = pos->second;

    // First BEAT from Node
    if (currentTimeOut == -1) {
        pos->second = maxTimeOut;
        return;
    }

    // Update the maxTimeOut for the node
    float diffTime = maxTimeOut - currentTimeOut;
    maxPos->second = 2 * diffTime;
    pos->second = maxPos->second;
}

// Every X seconds the threads send a BEAT message to all the
// current memebers on the group.
void FailureDetector::heartBeating() {
    while (isRunning()) {
        std::this_thread::sleep_for(std::chrono::duration<float>(HEARTBEAT_PERIOD));

        // Define the BEAT message
        Message beatMessage(nodeAddress, BEAT, "");
        std::string encodedBeat = beatMessage.encodeToString();

        // Lock to get all the current memebers of the group
        for (size_t i = 0; i < currentGroup.size(); ++i) {
            std::string memberAddress = currentGroup[i];
            if (memberAddress != this->nodeAddress) {
                sendUDPMessage(sendSocket, memberAddress, encodedBeat);
            }
        }
    }
}

// Checks periodically for the timeouts of the group processes
void FailureDetector::inspectBeatings() {
    float samplingTime = HEARTBEAT_PERIOD / 10;
    while (isRunning()) {
        std::this_thread::sleep_for(std::chrono::duration<float>(samplingTime));

        // Locks for time-outs between the groupProcesses
        std::lock_guard<std::mutex> lock(membersLock);
        std::map<std::string, float>::iterator pos;
        for (pos = membersTimers.begin(); pos != membersTimers.end(); ++pos) {
            pos->second -= samplingTime;

            // If the timeout hasn't expired we are fine
            if (pos->second > 0) { continue; }

            // Now we check if it crossed the threhsold
            std::string susAddress = pos->first;
            std::map<std::string, float>::iterator maxPos = membersMaxWait.find(susAddress);
            if (maxPos == membersMaxWait.end()) { return; };
            if (pos->second <= (-1.5 * maxPos->second)) {
                suspectCrash(pos->first);
            }
        }
    }
}

// Add a new member to the group so the Detector can also track
// and send BEATS to that node.
void FailureDetector::addNewMember(std::string memberAddress) {
    std::lock_guard<std::mutex> lock(membersLock);
    membersMaxWait.insert(make_pair(memberAddress, HEARTBEAT_PERIOD * 10));
    membersTimers.insert(make_pair(memberAddress, -1));
    currentGroup.push_back(memberAddress);
}

// Deletes a member from the group
void FailureDetector::removeMember(std::string memberAddress) {
    std::lock_guard<std::mutex> lock(membersLock);
    membersMaxWait.erase(memberAddress);
    membersTimers.erase(memberAddress);

    // Find the value at the currentGroup vector
    std::vector<std::string>::iterator pos = std::find(
        currentGroup.begin(), currentGroup.end(), memberAddress
    );
    if (pos == currentGroup.end()) { return; }
    currentGroup.erase(pos);
}