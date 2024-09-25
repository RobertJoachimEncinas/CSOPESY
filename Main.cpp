#include"Utils/Display.h"
#include"DataTypes/System.h"

int main() {
    std::string input;
    System system = System();
    printHeader();

    while (true) {
        std::cout << "Enter a command: ";
        std::getline(std::cin, input);
        system.parseCommand(input);
    }

    return 0;
}
