#include"DataTypes\Commands.h"

int main() {
    bool running = true;
    std::string input;

    printHeader();
    while(running) {
        std::cout << "Enter a command: ";
        std::cin >> input;
        
        if(input.compare("exit") == 0) {
            running = false;
        } else if(commandMap[input]) {
            commandMap[input]();
        } else {
            std::cout << "Error! Unrecognized command\n";
        }

        return 0;
    }
}