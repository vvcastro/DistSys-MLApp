#include "include/config.hpp"


// Check args from command execution
std::vector<std::string> getArgs(int argc, char* argv[]) {
    const char* base[] = { "--interface" };
    std::vector<std::string> names(base, std::end(base));

    // Check if the arguments are expected
    std::vector<std::string> args;
    for (int i = 0; i < names.size(); ++i) {
        std::string argName = names[i];
        std::string arg = argv[2 * i + 1];

        if (arg == argName && (2 * (i + 1)) < argc) {
            args.push_back(argv[2 * (i + 1)]);
        }
        else {
            std::cerr << "Invalid or missing arguments: " << arg << std::endl;
        };

    }
    return args;
}
