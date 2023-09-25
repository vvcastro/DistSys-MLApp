#include "network/include/node.hpp"
#include "app/include/config.hpp"
#include <iostream>
#include <csignal>
#include <string>

std::shared_ptr<Node> node;

// Exit code and close active instances
void handleCtrlC(int signal) {
    std::cout << "\nExiting main code!\n" << std::endl;
    node->closeConnection();
    exit(signal);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, handleCtrlC);
    std::vector<std::string> execArgs = getArgs(argc, argv);

    // Get the nodeId and networkInterface to connect to.
    node = std::make_shared<Node>(execArgs[0], execArgs[1]);
    node->showConnectionData();


    node->closeConnection();
    return 0;
}