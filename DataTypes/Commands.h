#ifndef COMMANDS
#define COMMANDS

#ifndef STDSTRING
#define STDSTRING
#include<string>
#endif

#ifndef STDIOSTREAM
#define STDIOSTREAM
#include<iostream>
#endif

#ifndef STDFUNCTIONAL
#define STDFUNCTIONAL
#include<functional>
#endif

#ifndef STDMAP
#define STDMAP
#include<map>
#endif

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

int cmd_initialize() {
    std::cout << "Initialize called\n";
    return 0;
}

int cmd_screen() {
    std::cout << "Screen called\n";
    return 0;
}

int cmd_scheduler_test() {
    std::cout << "Scheduler-Test called\n";
    return 0;
}

int cmd_scheduler_stop() {
    std::cout << "Scheduler-Stop called\n";
    return 0;
}

int cmd_report_util() {
    std::cout << "Report-Util called\n";
    return 0;
}

int cmd_clear() {
    system("cls");
    printHeader();
    return 0;
}

int cmd_exit() {
    return 0;
}

using commands = std::function<int()>;
const std::map<std::string, commands> commandMap {
    { "initialize", cmd_initialize },
    { "screen", cmd_screen },
    { "scheduler-test", cmd_scheduler_test },
    { "scheduler-stop", cmd_scheduler_stop },
    { "report-util", cmd_report_util },
    { "clear", cmd_clear },
    { "exit", cmd_exit },
};

#endif