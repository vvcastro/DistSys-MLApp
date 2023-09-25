#include "network/include/node.hpp"
#include "app/include/config.hpp"
#include <iostream>
#include <csignal>
#include <string>

std::shared_ptr<Node> node;
std::string peerAddress("192.168.56.4");

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

    std::string userInput;
    while (true) {
        std::cout << "Enter a command (or 'exit' to quit).\n> ";

        // capture input from user
        std::cin >> userInput;
        if (userInput == "exit") {
            break;
        }

        // construct message
        node->sendMessage(peerAddress, userInput);
    }

    node->closeConnection();
    return 0;
}