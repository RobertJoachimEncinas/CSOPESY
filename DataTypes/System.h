/*
    This file defines all code related to the OS environment
*/
#include"Process.h"
#include<vector>
#include <sstream>
#include <ctime>
#include <iomanip>

class System
{
    private:
        std::vector<Process> processes;
        bool commandsValid = true; // Flag to track if commands are valid
    public:
        //Constructor
        System() {}

        //Methods
        void cmd_initialize() {
            std::cout << "initialize command recognized. Doing something.\n";
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

        void cmd_screen(const Process& process) {
            commandsValid = false; // Set flag to false
            system("cls");
            std::cout << "Process Name: " << process.name << "\n";
            std::cout << "Current Line: "
                << process.current_instruction << " / "
                << process.total_instructions << "\n";
            std::cout << "Timestamp: " << process.timestamp << "\n";
        }

        void cmd_screen_r(const std::string& process_name) {
            for (const auto& process : processes) {
                if (process.name == process_name) {
                    commandsValid = false; // Set flag to false
                    system("cls");
                    std::cout << "Process Name: " << process_name << "\n";
                    std::cout << "Current Line: "
                        << process.current_instruction << " / "
                        << process.total_instructions << "\n";
                    std::cout << "Timestamp: " << process.timestamp << "\n";
                    return;
                }
            }
            std::cout << "Error! Process " << process_name << " not found.\n";
        }

        void cmd_screen_add(const std::string& process_name) {
            // Check if the process already exists
            for (const auto& process : processes) {
                if (process.name == process_name) {
                    std::cout << "Error! Process " << process_name << " already exists.\n";
                    return;  // Exit the function if a duplicate is found
                }
            }

            // If no duplicates, create and add the new process
            Process newProcess;
            newProcess.name = process_name;
            newProcess.current_instruction = 1;   // Start at instruction 1
            newProcess.total_instructions = 10;   // Example total instructions
            newProcess.timestamp = getCurrentTimestamp();
            processes.push_back(newProcess);
            cmd_screen(newProcess);  // Display the new process info
        }

        std::string getCurrentTimestamp() {
            std::time_t t = std::time(&t);
            std::tm now = *localtime(&t);

            std::ostringstream oss;
            oss << std::put_time(&now, "%m/%d/%Y, %I:%M:%S %p");
            return oss.str();
        }


        // Helper function to split input into tokens
        std::vector<std::string> tokenizeInput(const std::string& input) {
            std::vector<std::string> tokens;
            std::istringstream stream(input);
            std::string token;

            while (stream >> token) {
                tokens.push_back(token);
            }
            return tokens;
        }

        void parseCommand(const std::string& input) {
            std::vector<std::string> tokens = tokenizeInput(input);

            if (tokens.empty()) {
                std::cout << "Error! Empty input.\n";
                return;
            }

            std::string command = tokens[0];  // First token is the command

            if (!commandsValid && command != "exit") {
                std::cout << "Error! Invalid command.\n";
                return;
            }

            if (command == "exit") {
                if (!commandsValid) {
                    commandsValid = true;
                    cmd_clear();
                }
                else {
                    std::exit(0);
                }
            }
            else if (command == "initialize") {
                cmd_initialize();
            }
            else if (command == "screen") {
                if (tokens.size() == 3 && tokens[1] == "-s") {
                    std::string process_name = tokens[2];
                    cmd_screen_add(process_name);  // Add new process
                }
                else if (tokens.size() == 3 && tokens[1] == "-r") {
                    std::string process_name = tokens[2];
                    cmd_screen_r(process_name);
                }
                else {
                    std::cout << "Error! Correct usage: screen -s <process_name> or screen -r <process_name>\n";
                }
            }
            else if (command == "scheduler-test") {
                cmd_scheduler_test();
            }
            else if (command == "scheduler-stop") {
                cmd_scheduler_stop();
            }
            else if (command == "report-util") {
                cmd_report_util();
            }
            else if (command == "clear") {
                cmd_clear();
            }
            else {
                std::cout << "Error! Unrecognized command\n";
            }
        }
};

