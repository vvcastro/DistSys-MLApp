#include "network/include/node.hpp"
#include "app/include/config.hpp"
#include <iostream>
#include <csignal>
#include <string>

std::shared_ptr<Node> node;

// Exit code and close active instances
void handleCtrlC(int signal) {
    std::cout << "\nExiting main code!\n" << std::endl;
    node->close();
    exit(signal);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, handleCtrlC);
    std::vector<std::string> execArgs = getArgs(argc, argv);

    // Get the nodeId and networkInterface to connect to.
    node = std::make_shared<Node>(execArgs[0]);
    node->showConnectionData();

    std::string userInput;
    std::cout << "Enter a message to send.\n> ";
    std::cin >> userInput;
    node->broadcastMessage(userInput);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    node->close();
    return 0;
}