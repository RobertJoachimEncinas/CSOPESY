#include<iostream>
#include<string>

typedef enum TextColor {
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37,
    RESET = 0,
} TextColor;

void printColored(std::string text, TextColor color) {
    std::string color_escape = "\033[";
    std::cout << color_escape << color << "m" << text << color_escape << RESET << "m";
}

void printHeader() {
    std::cout << "  ____ ____   ___  ____  _____ ______   __\n";
    std::cout << " / ___/ ___| / _ \\|  _ \\| ____/ ___\\ \\ / /\n";
    std::cout << "| |   \\___ \\| | | | |_) |  _| \\___  \\ V /\n";
    std::cout << "| |___ ___) | |_| |  __/| |___ ___) || |\n";
    std::cout << " \\____|____/ \\___/|_|   |_____|____/ |_|  \n";
    printColored("Hello, Welcome to CSOPESY commandline!\n", GREEN);
    printColored("Type \'exit\' to quit, \'clear\' to clear the screen\n", YELLOW);
}

void cmd_initialize() {
    std::cout << "initialize command recognized. Doing something.\n";
}

void cmd_screen() {
    std::cout << "screen command recognized. Doing something.\n";
}

void cmd_scheduler_test() {
    std::cout << "scheduler-test command recognized. Doing something.\n";
}

void cmd_scheduler_stop() {
    std::cout << "scheduler-stop command recognized. Doing something.\n";
}

void cmd_report_util() {
    std::cout << "report-util command recognized. Doing something.\n";
}

void cmd_clear() {
    system("cls");
    printHeader();
}

int main() {
    bool running = true;
    std::string input;

    printHeader();
    while(running) {
        std::cout << "Enter a command: ";
        std::cin >> input;
        
        if(input.compare("exit") == 0) {
            running = false;
        } else if(input.compare("initialize")) {
            cmd_initialize();
        } else if(input.compare("screen")) {
            cmd_screen();
        } else if(input.compare("scheduler-test")) {
            cmd_scheduler_test();
        } else if(input.compare("scheduler-stop")) {
            cmd_scheduler_stop();
        } else if(input.compare("report-util")) {
            cmd_report_util();
        } else if(input.compare("clear")) {
            cmd_clear();
        } else {
            std::cout << "Error! Unrecognized command\n";
        }
    }

    return 0;
}