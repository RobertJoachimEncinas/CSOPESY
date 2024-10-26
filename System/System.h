/*
    This file defines all code related to the OS environment
*/
#include "../DataTypes/TSQueue.h"
#include "../DataTypes/SchedAlgo.h"
#include "../System/Scheduler.h"
#include "../System/Tester.h"
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
    private:
        std::vector<std::shared_ptr<Process>> processes;
        bool isInMainConsole = true; // Flag to track if commands are valid
        bool isInitialized = false;
        std::vector<Core*> cores;
        int totalCores = 0;
        long long processMinIns = 100;
        long long processMaxIns = 100;
        long long processFreq = 1;

        std::string current_process; // Global variable to store the current process
        std::map<std::string, std::vector<std::string>> processHistory; // Map to hold history for each process

        Scheduler scheduler;
        Tester tester;
        SynchronizedClock synchronizer;

    public:    
        //Constructor
        System(): synchronizer(std::addressof(cores), std::addressof(tester), std::addressof(scheduler)),
        scheduler(std::addressof(cores), synchronizer.getSyncClock()), 
        tester(synchronizer.getSyncClock(), &processFreq, &processes, &processMinIns, &processMaxIns, getCurrentTimestamp, std::addressof(scheduler))
        {}

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
            if (isInitialized) {
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
                cores.push_back(new Core(i, quantum_cycles, synchronizer.getSyncClock(), this->getCurrentTimestamp, algorithm, delay_per_exec));
            }
            scheduler.assignReadyQueueToCores();
            processMaxIns = max_ins;
            processMinIns = min_ins;
            processFreq = process_freq;
            boot();
            isInitialized = true;
        }
        
        void cmd_scheduler_test() {
            if(tester.isActive()) {
                std::cout << "Error! scheduler-test still active!\n";
                return;
            }

            synchronizer.startTester();
            std::cout << "Scheduler started\n";
        }

        void cmd_scheduler_stop() {
            if(!tester.isActive()) {
                std::cout << "Error! scheduler-test is not active!\n";
                return;
            }

            tester.turnOff();
            std::cout << "Scheduler stoped\n";
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
            std::cout << "\"csopesy-log.txt\" report generated successfully.\n";
        }

        void cmd_clear() {
            system("cls");
            printHeader();
        }

        void cmd_screen(Process process) {
            isInMainConsole = false; // Set flag to false
            system("cls");
            cmd_display_history(process.name);

            if (processHistory[process.name].size() == 0) {
                std::ostringstream output;
                output << "Process Name: " << process.name << "\n";
                output << "Current Line: " << process.current_instruction << " / " << process.total_instructions << "\n";
                output << "Timestamp: " << process.timestamp << "\n\n";
                std::cout << output.str();
                processHistory[process.name].push_back(output.str());
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
                    if(process->completed) {
                        break;
                    }

                    cmd_screen(*process);
                    return;
                }
            }
            std::cout << "Error! Process " << process_name << " not found.\n";
        }

        std::shared_ptr<Process> cmd_screen_add(const std::string& process_name) {
            // Check if the process already exists
            while(tester.isActive() && !tester.isLocked()) {}
            tester.lock();

            for (const auto& process : processes) {
                if (process->name == process_name) {
                    std::cout << "Error! Process " << process_name << " already exists.\n";
                    return nullptr;  // Exit the function if a duplicate is found
                }
            }
            // Set random number of instructions
            long long instructions = processMinIns + (rand() % (processMaxIns - processMinIns + 1));
            // If no duplicates, create and add the new process
            std::shared_ptr<Process> newProcess = std::make_shared<Process>(process_name, instructions, getCurrentTimestamp());
            processes.push_back(newProcess);

            //Add to scheduler
            scheduler.enqueue(newProcess.get());
            tester.unlock();
            return newProcess;
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

            
            if (!isInMainConsole) {
                if (command == "exit") {
                    isInMainConsole = true;
                    processHistory[current_process].push_back("Enter a command: exit\n\n");
                    cmd_clear();
                }
                else if (command == "process-smi"){
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
                else {
                    std::ostringstream output;
                    output << "Error! Invalid command.\n\n";
                    std::cout << output.str();

                    std::ostringstream commandOutput;
                    commandOutput << "Enter a command: " << command << "\n";
                    processHistory[current_process].push_back(commandOutput.str());
                    processHistory[current_process].push_back(output.str());

                    return;
                } 
                
            }
            else if (command == "clear") {
                cmd_clear();
            }
            else if (command == "exit") {
                terminate();
                std::exit(0);
            }
            else if (command == "initialize") {
                cmd_initialize();
            }
            else if (command == "screen") {
                if(!isInitialized) {
                    std::cout << "Error! System not initialized.\n";
                    return;
                }
                if (tokens.size() == 3 && tokens[1] == "-s") {
                    std::string process_name = tokens[2];
                    std::shared_ptr<Process> newProcess = cmd_screen_add(process_name);  // Add new process
                    if(newProcess != nullptr)
                        cmd_screen(*newProcess);  // Display the new process info
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
                if(!isInitialized) {
                    std::cout << "Error! System not initialized.\n";
                    return;
                }
                cmd_scheduler_test();
            }
            else if (command == "scheduler-stop") {
                if(!isInitialized) {
                    std::cout << "Error! System not initialized.\n";
                    return;
                }
                cmd_scheduler_stop();
            }
            else if (command == "report-util") {
                if(!isInitialized) {
                    std::cout << "Error! System not initialized.\n";
                    return;
                }
                cmd_report_util();
            }
            else {
                std::cout << "Error! Unrecognized command\n";
            }
        }
};

