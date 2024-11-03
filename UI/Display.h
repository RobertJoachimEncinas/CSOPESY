/*
    This file defines all code related to displayed outputs
*/
#ifndef DISPLAY
#define DISPLAY

#include<string>
#include<iostream>
#include<vector>
#include <iomanip>
#include <fstream>
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

void printProcesses(int totalCores, std::vector<Process> runningProcesses, std::vector<Process> completedProcesses) {
    int running_ctr = 0; 

    for (const auto& process : runningProcesses) { 
        if (process.core != -1) {
            running_ctr++;
        }
    } 

    int cpu_util = static_cast<double>(running_ctr) / totalCores * 100;
    cpu_util = cpu_util < 0 ? 0 : cpu_util;
    std::cout << "\nCPU utilization: " << cpu_util << "%\n";
    std::cout << "Cores used: " << running_ctr << "\n";
    std::cout << "Cores available: " << totalCores - running_ctr << "\n";
    printColored("-----------------------------------------\n", BLUE);
    std::cout << "Running Processes:\n";
    for (const auto process : runningProcesses) {
        std::string inCore = (process.core == -1) ? "N/A" : std::to_string(process.core);
        printf("%-11s %-30s Core: %-3s      %d / %d\n", process.name.c_str(), ("(" + process.timestamp + ")").c_str(), inCore.c_str(), process.current_instruction, process.total_instructions);
    }
    
    std::cout << "\nFinished Processes:\n";
    for (const auto& process : completedProcesses) {
        printf("%-11s %-30s Finished      %d / %d\n", process.name.c_str(), ("(" + process.timestamp + ")").c_str(), process.current_instruction, process.total_instructions);
    }
    printColored("-----------------------------------------\n", BLUE);
}

void logProcesses(int totalCores, std::vector<Process> runningProcesses, std::vector<Process> completedProcesses) {
    std::ofstream outfile("./Logs/csopesy-log.txt");

    if (!outfile.is_open()) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }

    int running_ctr = 0; 

    for (const auto& process : runningProcesses) { 
        if (process.core != -1) {
            running_ctr++;
        }
    } 

    int cpu_util = static_cast<double>(running_ctr) / totalCores * 100;
    cpu_util = cpu_util < 0 ? 0 : cpu_util;
    outfile << "CPU utilization: " << cpu_util << "%\n";
    outfile << "Cores used: " << running_ctr << "\n";
    outfile << "Cores available: " << totalCores - running_ctr << "\n";
    outfile << "-----------------------------------------\n";
    outfile << "Running Processes:\n";
    for (const auto& process : runningProcesses) {
        std::string inCore = (process.core == -1) ? "N/A" : std::to_string(process.core);
        outfile << process.name << " (" << process.timestamp << ") Core: " << std::left << std::setw(3)<< inCore 
                << "      " << process.current_instruction << " / " << process.total_instructions << "\n";
    }

    outfile << "\nFinished Processes:\n";
    for (const auto& process : completedProcesses) {
        outfile << process.name << " (" << process.timestamp << ") Finished      "
                << process.current_instruction << " / " << process.total_instructions << "\n";
    }

    outfile << "-----------------------------------------\n";
    outfile.close();
}
#endif  