/*
    This file defines all code related to displayed outputs
*/
#ifndef DISPLAY
#define DISPLAY

#include<string>
#include<iostream>
#include<vector>
#include"./Styles.h"
#include"../DataTypes/Process.h"

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
    printColored("Type 'exit' to quit, 'clear' to clear the screen\n", YELLOW);
}

void printLine() {
    std::string str(50, '-');
    std::cout << str;
}

void printProcesses(std::vector<Process> runningProcesses, std::vector<Process> completedProcesses) {
    printColored("-----------------------------------------\n", BLUE);
    std::cout << "Running Processes:\n";
    // TODO: change according to the proper format
    for (const auto process : runningProcesses) {
        std::string inCore = (process.core == -1) ? "N/A" : std::to_string(process.core);
        printf("%-11s %-30s Core: %s      %d / %d\n", process.name.c_str(), ("(" + process.timestamp + ")").c_str(), inCore.c_str(), process.current_instruction, process.total_instructions);
    }
    
    std::cout << "\nFinished Processes:\n";
    for (const auto& process : completedProcesses) {
        printf("%-11s %-30s Finished      %d / %d\n", process.name.c_str(), ("(" + process.timestamp + ")").c_str(), process.current_instruction, process.total_instructions);
    }
    printColored("-----------------------------------------\n", BLUE);
}
#endif  