/*
    This file defines all code related to the OS environment
*/
#include "../DataTypes/TSQueue.h"
#include "../DataTypes/SchedAlgo.h"
#include "../System/Scheduler.h"
#include "../System/Core.h"
#include "../System/SynchronizedClock.h"
#include "../UI/Display.h"
#include <vector>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <memory>

class System
{
    public:
        std::vector<std::shared_ptr<Process>> processes;
        bool commandsValid = true; // Flag to track if commands are valid
        bool initialized = false;
        std::vector<Core*> cores;
        int clockMod = 1000;
        int totalCores = 0;

        std::string current_process; // Global variable to store the current process
        std::map<std::string, std::vector<std::string>> processHistory; // Map to hold history for each process


        SynchronizedClock synchronizer = SynchronizedClock(std::addressof(cores), clockMod);
        Scheduler scheduler = Scheduler(std::addressof(cores), synchronizer.getSyncClock());
        
        //Constructor
        System() {}

        //Methods
        void boot() {
            synchronizer.start();
            scheduler.start();
            for(int i = 0; i < cores.size(); i++) {
                (*(cores.at(i))).start();
            }
            std::cout << "System booted successfully.\n";
        }

        void terminate() {
            synchronizer.turnOff();
            scheduler.turnOff();
            for(int i = 0; i < 4; i++) {
                (*cores[i]).turnOff();
            }
        }

        void cmd_initialize() {
            if (initialized) {
                std::cout << "Error! System already initialized.\n";
                return;
            }

            FILE* f = fopen("config.txt", "r");
            int num_cpu;
            SchedAlgo algorithm;
            long long quantum_cycles;
            long long process_freq;
            long long min_ins;
            long long max_ins;
            long long delay_per_exec;
            long long limit = (long long)1 << 32;

            for (int i = 1; i <= 7; i++) {
                char buffer[256];
                fgets(buffer, 256, f);
                std::vector<std::string> tokens = tokenizeInput(buffer);

                if (tokens.size() != 2) {
                    std::cout << "Error! Invalid config file. Line " << i << "\n";
                    return;
                }

                switch (i)
                {
                case 1:
                    if (tokens[0] != "num-cpu") {
                        std::cout << "Error! Invalid config file. Line " << i << "\n";
                        return;
                    }
                    
                    num_cpu = std::stoi(tokens[1]);
                    if (num_cpu < 1 || num_cpu > 128) {
                        std::cout << "Error! Invalid number of CPUs.\n";
                        return;
                    }
                    break;
                case 2:
                    if (tokens[0] != "scheduler") {
                        std::cout << "Error! Invalid config file. Line " << i << "\n";
                        return;
                    }

                    algorithm = (SchedAlgo) parseSchedAlgo(tokens[1]);
                    if (algorithm == -1) {
                        std::cout << "Error! Invalid scheduling algorithm.\n";
                        return;
                    }
                    break;
                case 3:
                    if (tokens[0] != "quantum-cycles") {
                        std::cout << "Error! Invalid config file. Line " << i << "\n";
                        return;
                    }
                    quantum_cycles = std::stoll(tokens[1]);
                    if (quantum_cycles < 1 || quantum_cycles > limit) {
                        std::cout << "Error! Invalid quantum cycles.\n";
                        return;
                    }
                    break;
                case 4:
                    if (tokens[0] != "batch-process-freq") {
                        std::cout << "Error! Invalid config file. Line " << i << "\n";
                        return;
                    }
                    process_freq = std::stoll(tokens[1]);
                    if (process_freq < 1 || process_freq > limit) {
                        std::cout << "Error! Invalid batch process frequency.\n";
                        return;
                    }
                    break;
                case 5:
                    if (tokens[0] != "min-ins") {
                        std::cout << "Error! Invalid config file. Line " << i << "\n";
                        return;
                    }
                    min_ins = std::stoll(tokens[1]);
                    if (min_ins < 1 || min_ins > limit) {
                        std::cout << "Error! Invalid minimum instructions.\n";
                        return;
                    }
                    break;
                case 6:
                    if (tokens[0] != "max-ins") {
                        std::cout << "Error! Invalid config file. Line " << i << "\n";
                        return;
                    }
                    max_ins = std::stoll(tokens[1]);
                    if (max_ins < 1 || max_ins > limit) {
                        std::cout << "Error! Invalid maximum instructions.\n";
                        return;
                    }
                    break;
                case 7:
                    if (tokens[0] != "delays-per-exec") {
                        std::cout << "Error! Invalid config file. Line " << i << "\n";
                        return;
                    }
                    delay_per_exec = std::stoll(tokens[1]);
                    if (delay_per_exec < 0 || delay_per_exec > limit) {
                        std::cout << "Error! Invalid delay per execution.\n";
                        return;
                    }
                    break;
                }
            }
            totalCores = num_cpu;
            for(int i = 0; i < num_cpu; i++) {
                cores.push_back(new Core(i, quantum_cycles, clockMod, synchronizer.getSyncClock(), this->getCurrentTimestamp));
            }
            scheduler.assignReadyQueueToCores();
            boot();
            initialized = true;
        }

        void cmd_scheduler_test() {
            std::string process = "process";
            for(int i = 0; i < 10; i++) {
                cmd_screen_add(process + std::to_string(i));
            }
            cmd_clear();
            commandsValid = true;
        }

        void cmd_scheduler_stop() {
            std::cout << "scheduler-stop command recognized. Doing something.\n";
        }

        void cmd_report_util() {
            std::vector<Process> runningProcesses;
            std::vector<Process> finishedProcesses;

            for(const auto& process: processes) {
                if(process->completed) {
                    finishedProcesses.push_back(*process);
                } else {
                    runningProcesses.push_back(*process);
                }
            }

            logProcesses(totalCores, runningProcesses, finishedProcesses);
        }

        void cmd_clear() {
            system("cls");
            printHeader();
        }

        void cmd_screen(Process process) {
            commandsValid = false; // Set flag to false
            system("cls");
            cmd_display_history(process.name);

            if (processHistory[process.name].size() == 0) {
                std::ostringstream output;
                output << "Process Name: " << process.name << "\n";
                output << "Current Line: " << process.current_instruction << " / " << process.total_instructions << "\n";
                output << "Timestamp: " << process.timestamp << "\n\n";
                std::cout << output.str();
                processHistory[process.name].push_back(output.str());

                return;
            }
            current_process = process.name; // Store process
        }


        void cmd_display_history(const std::string& process_name) {
            if (processHistory.find(process_name) != processHistory.end()) {
                for (const auto& entry : processHistory[process_name]) {
                    std::cout << entry; // Print each entry in the history
                }
            }
        }


        void cmd_screen_r(const std::string& process_name) {
            for (const auto& process : processes) {
                if (process->name == process_name) {
                    cmd_screen(*process);
                    return;
                }
            }
            std::cout << "Error! Process " << process_name << " not found.\n";
        }

        void cmd_screen_add(const std::string& process_name) {
            // Check if the process already exists
            for (const auto& process : processes) {
                if (process->name == process_name) {
                    std::cout << "Error! Process " << process_name << " already exists.\n";
                    return;  // Exit the function if a duplicate is found
                }
            }
            // If no duplicates, create and add the new process
            std::shared_ptr<Process> newProcess = std::make_shared<Process>(process_name, 100, getCurrentTimestamp());
            processes.push_back(newProcess);
            cmd_screen(*newProcess);  // Display the new process info

            //Add to scheduler
            scheduler.enqueue(newProcess.get());
        }

        std::string static getCurrentTimestamp() {
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

            if (!commandsValid && command != "exit" && command != "process-smi") {
                std::ostringstream output;
                output << "Error! Invalid command.\n\n";
                std::cout << output.str();

                std::ostringstream commandOutput;
                commandOutput << "Enter a command: " << command << "\n";
                processHistory[current_process].push_back(commandOutput.str());
                processHistory[current_process].push_back(output.str());

                return;
            }

            if (!commandsValid && command == "process-smi") {
                for(const auto& process: processes) {
                    if (process->name == current_process) {
                        std::ostringstream output; 
                        output << "\nProcess: " << process->name << "\n";
                        output << "ID: " << process->id << "\n\n";
                        
                        if (process->completed) {
                            output << "Finished!\n\n";
                        } else {
                            output << "Current instruction line: " << process->current_instruction << "\n";
                            output << "Lines of code: " << process->total_instructions << "\n\n";
                        }

                        std::cout << output.str();
                        processHistory[process->name].push_back("Enter a command: process-smi\n");
                        processHistory[process->name].push_back(output.str());

                        return;
                    }
                }
            }

            if (command == "exit") {
                if (!commandsValid) {
                    commandsValid = true;
                    processHistory[current_process].push_back("Enter a command: exit\n\n");
                    cmd_clear();
                }
                else {
                    terminate();
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
                else if (tokens.size() == 2 && tokens[1] == "-ls") {
                    std::vector<Process> runningProcesses;
                    std::vector<Process> finishedProcesses;

                    for(const auto& process: processes) {
                        if(process->completed) {
                            finishedProcesses.push_back(*process);
                        } else {
                            runningProcesses.push_back(*process);
                        }
                    }

                    printProcesses(totalCores, runningProcesses, finishedProcesses);
                }   
                else {
                    std::cout << "Error! Correct usage: screen -s <process_name> or screen -r <process_name> or screen -ls\n";
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

