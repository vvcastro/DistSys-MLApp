#include "include/network.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {

    ReliableNode* node = create_node();
    start_network_listening(node);

    char input[MAX_CONTENT_LEN];
    while (1) {
        printf("Enter message: ");
        fgets(input, MAX_CONTENT_LEN, stdin);
        input[strlen(input) - 1] = '\0';  // Remove newline character

        Message* message = create_new_message(node, MESSAGE_TYPE_NORMAL, input);
        send_message(node, message);
        delete_message(message);
    }

    close_node(node);
    return 0;
}
