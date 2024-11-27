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
#include "MemoryInterface.h"

class System
{
    private:
        std::map<std::string, std::shared_ptr<Process>> processes;
        bool isInMainConsole = true; // Flag to track if commands are valid
        bool isInitialized = false;
        std::vector<Core*> cores;
        int totalCores = 0;
        long long processMinIns = 100;
        long long processMaxIns = 100;
        long long processFreq = 1;
        long long memoryPerProcess = 0;
        int memAdd = 0;
        MemoryStats computeMemoryStats();


        std::string current_process; // Global variable to store the current process
        std::map<std::string, std::vector<std::pair<std::string, std::string>>> processHistory;

        Scheduler scheduler;
        Tester tester;
        SynchronizedClock synchronizer;
        AbstractMemoryInterface* memory;

    public:    
        //Constructor
        ~System() {
            for(const auto& core: cores) {
                delete core;
            }

            delete memory;
        }

        System(): synchronizer(std::addressof(cores), std::addressof(tester), std::addressof(scheduler)),
        scheduler(std::addressof(cores), synchronizer.getSyncClock()), 
        tester(synchronizer.getSyncClock(), &processFreq, &processes, &processMinIns, &processMaxIns, getCurrentTimestamp, std::addressof(scheduler), &memoryPerProcess)
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
                processHistory["Main"].emplace_back("Error! System already initialized.\n", "RESET");
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
            long long maxMem;
            long long memPerFrame;
            long long memPerProc;
            long long limit = (long long)1 << 32;
            bool isFlatAllocator = false; //CHANGE BASED ON CONFIG

            for (int i = 1; i <= 10; i++) {
                char buffer[256];
                fgets(buffer, 256, f);
                std::vector<std::string> tokens = tokenizeInput(buffer);

                if (tokens.size() != 2) {
                    std::cout << "Error! Invalid config file. Line " << i << "\n";
                    processHistory["Main"].emplace_back("Error! Invalid config file. Line " + std::to_string(i) + "\n", "RESET");
                    return;
                }

                switch (i)
                {
                    case 1:
                        if (tokens[0] != "num-cpu") {
                            std::cout << "Error! Invalid config file. Line " << i << "\n";
                            processHistory["Main"].emplace_back("Error! Invalid config file. Line " + std::to_string(i) + "\n", "RESET");
                            return;
                        }
                        
                        num_cpu = std::stoi(tokens[1]);
                        if (num_cpu < 1 || num_cpu > 128) {
                            std::cout << "Error! Invalid number of CPUs.\n";
                            processHistory["Main"].emplace_back("Error! Invalid number of CPUs.\n", "RESET");
                            return;
                        }
                        break;
                    case 2:
                        if (tokens[0] != "scheduler") {
                            std::cout << "Error! Invalid config file. Line " << i << "\n";
                            processHistory["Main"].emplace_back("Error! Invalid config file. Line " + std::to_string(i) + "\n", "RESET");
                            return;
                        }

                        algorithm = (SchedAlgo) parseSchedAlgo(tokens[1]);
                        if (algorithm == -1) {
                            std::cout << "Error! Invalid scheduling algorithm.\n";
                            processHistory["Main"].emplace_back("Error! Invalid scheduling algorithm.\n", "RESET");
                            return;
                        }
                        break;
                    case 3:
                        if (tokens[0] != "quantum-cycles") {
                            std::cout << "Error! Invalid config file. Line " << i << "\n";
                            processHistory["Main"].emplace_back("Error! Invalid config file. Line " + std::to_string(i) + "\n", "RESET");
                            return;
                        }
                        quantum_cycles = std::stoll(tokens[1]);
                        if (quantum_cycles < 1 || quantum_cycles > limit) {
                            std::cout << "Error! Invalid quantum cycles.\n";
                            processHistory["Main"].emplace_back("Error! Invalid quantum cycles.\n", "RESET");
                            return;
                        }
                        break;
                    case 4:
                        if (tokens[0] != "batch-process-freq") {
                            std::cout << "Error! Invalid config file. Line " << i << "\n";
                            processHistory["Main"].emplace_back("Error! Invalid config file. Line " + std::to_string(i) + "\n", "RESET");
                            return;
                        }
                        process_freq = std::stoll(tokens[1]);
                        if (process_freq < 1 || process_freq > limit) {
                            std::cout << "Error! Invalid batch process frequency.\n";
                            processHistory["Main"].emplace_back("Error! Invalid batch process frequency.\n", "RESET");
                            return;
                        }
                        break;
                    case 5:
                        if (tokens[0] != "min-ins") {
                            std::cout << "Error! Invalid config file. Line " << i << "\n";
                            processHistory["Main"].emplace_back("Error! Invalid config file. Line " + std::to_string(i) + "\n", "RESET");
                            return;
                        }
                        min_ins = std::stoll(tokens[1]);
                        if (min_ins < 1 || min_ins > limit) {
                            std::cout << "Error! Invalid minimum instructions.\n";
                            processHistory["Main"].emplace_back("Error! Invalid minimum instructions.\n", "RESET");
                            return;
                        }
                        break;
                    case 6:
                        if (tokens[0] != "max-ins") {
                            std::cout << "Error! Invalid config file. Line " << i << "\n";
                            processHistory["Main"].emplace_back("Error! Invalid config file. Line " + std::to_string(i) + "\n", "RESET");
                            return;
                        }
                        max_ins = std::stoll(tokens[1]);
                        if (max_ins < 1 || max_ins > limit) {
                            std::cout << "Error! Invalid maximum instructions.\n";
                            processHistory["Main"].emplace_back("Error! Invalid maximum instructions.\n", "RESET");
                            return;
                        }
                        break;
                    case 7:
                        if (tokens[0] != "delays-per-exec") {
                            std::cout << "Error! Invalid config file. Line " << i << "\n";
                            processHistory["Main"].emplace_back("Error! Invalid config file. Line " + std::to_string(i) + "\n", "RESET");
                            return;
                        }
                        delay_per_exec = std::stoll(tokens[1]);
                        if (delay_per_exec < 0 || delay_per_exec > limit) {
                            std::cout << "Error! Invalid delay per execution.\n";
                            processHistory["Main"].emplace_back("Error! Invalid delay per execution.\n", "RESET");
                            return;
                        }
                        break;
                    case 8: 
                        if (tokens[0] != "max-overall-mem") {
                            std::cout << "Error! Invalid config file. Line " << i << "\n";
                            processHistory["Main"].emplace_back("Error! Invalid config file. Line " + std::to_string(i) + "\n", "RESET");
                            return;
                        }
                        maxMem = std::stoll(tokens[1]);
                        if (maxMem < 2 || maxMem > limit) {
                            std::cout << "Error! Invalid maximum overall memory.\n";
                            processHistory["Main"].emplace_back("Error! Invalid maximum overall memory.\n", "RESET");
                            return;
                        }
                        break;
                    case 9:
                        if (tokens[0] != "mem-per-frame") {
                            std::cout << "Error! Invalid config file. Line " << i << "\n";
                            processHistory["Main"].emplace_back("Error! Invalid config file. Line " + std::to_string(i) + "\n", "RESET");
                            return;
                        }
                        memPerFrame = std::stoll(tokens[1]);
                        if (memPerFrame < 2 || memPerFrame > limit) {
                            std::cout << "Error! Invalid memory per frame.\n";
                            processHistory["Main"].emplace_back("Error! Invalid memory per frame.\n", "RESET");
                            return;
                        }
                        break;
                    case 10:
                        if (tokens[0] != "mem-per-proc") {
                            std::cout << "Error! Invalid config file. Line " << i << "\n";
                            processHistory["Main"].emplace_back("Error! Invalid config file. Line " + std::to_string(i) + "\n", "RESET");
                            return;
                        }
                        memPerProc = std::stoll(tokens[1]);
                        if (memPerProc < 2 || memPerProc > limit) {
                            std::cout << "Error! Invalid memory per process.\n";
                            processHistory["Main"].emplace_back("Error! Invalid memory per process.\n", "RESET");
                            return;
                        }
                        break;
                }
            }

            if(isFlatAllocator) {
                memAdd = maxMem;
                memory = new FlatMemoryInterface(maxMem, getCurrentTimestamp, std::addressof(cores));
            } else {
                memAdd = maxMem;
                memory = new PagingMemoryInterface(maxMem, memPerFrame, getCurrentTimestamp, std::addressof(cores));
            }

            scheduler.setMemoryInterface(memory);
            synchronizer.setMemoryInterface(memory);
            tester.setMemoryInterface(memory);

            totalCores = num_cpu;
            for(int i = 0; i < num_cpu; i++) {
                cores.push_back(new Core(i, quantum_cycles, synchronizer.getSyncClock(), this->getCurrentTimestamp, algorithm, delay_per_exec));
            }
            scheduler.assignReadyQueueToCores();
            processMaxIns = max_ins;
            processMinIns = min_ins;
            processFreq = process_freq;
            memoryPerProcess = memPerProc;
            
            boot();
            isInitialized = true;

            processHistory["Main"].emplace_back("System booted successfully.\n", "RESET");
        }
        
        void cmd_scheduler_test() {
            if(tester.isActive()) {
                std::cout << "Error! scheduler-test still active!\n";
                processHistory["Main"].emplace_back("Error! scheduler-test still active!\n", "RESET");
                return;
            }

            synchronizer.startTester();

            while(!tester.isActive()) {};
            std::cout << "Scheduler started\n";
            processHistory["Main"].emplace_back("Scheduler started\n", "RESET");
        }

        void cmd_scheduler_stop() {
            if(!tester.isActive()) {
                std::cout << "Error! scheduler-test is not active!\n";
                processHistory["Main"].emplace_back("Error! scheduler-test is not active!\n", "RESET");
                return;
            }

            tester.turnOff();
            std::cout << "Scheduler stopped\n";
            processHistory["Main"].emplace_back("Scheduler stoped\n", "RESET");
        }

        void cmd_report_util() {
            std::vector<Process> runningProcesses;
            std::vector<Process> finishedProcesses;

            for(const auto& process: processes) {
                if(process.second->completed) {
                    finishedProcesses.push_back(*process.second);
                } else {
                    runningProcesses.push_back(*process.second);
                }
            }

            logProcesses(totalCores, runningProcesses, finishedProcesses);

            std::ostringstream output;                
            output << "\"csopesy-log.txt\" report generated successfully.\n";
            std::cout << output.str();
            processHistory["Main"].emplace_back(output.str(), "RESET");
        }

        void cmd_clear() {
            system("cls");
            printHeader();
            cmd_display_history("Main");
        }

        void cmd_screen(Process process) {
            isInMainConsole = false; // Set flag to false
            system("cls");
            cmd_display_history(process.name);

            std::ostringstream output; 
            output << "\nProcess: " << process.name << "\n";
            output << "ID: " << process.id << "\n\n";
            
            if (process.completed) {
                output << "Finished!\n\n";
            } else {
                output << "Current instruction line: " << process.current_instruction << "\n";
                output << "Lines of code: " << process.total_instructions << "\n\n";
            }

            std::cout << output.str();
            processHistory[process.name].emplace_back(output.str(), "RESET");

            current_process = process.name; // Store process
        }


        void cmd_display_history(const std::string& process_name) {
            if (processHistory.find(process_name) != processHistory.end()) {
                const auto& processData = processHistory[process_name];

                for (const auto& entry : processData) {
                    TextColor color = stringToColor(entry.second);
                    printColored(entry.first, color);
                }
            }
        }


        void cmd_screen_r(const std::string& process_name) {
            for (const auto& process : processes) {
                if (process.second->name == process_name) {
                    if(process.second->completed) {
                        break;
                    }

                    cmd_screen(*process.second);
                    return;
                }
            }
            std::cout << "Error! Process " << process_name << " not found.\n";
            processHistory["Main"].emplace_back("Error! Process " + process_name + " not found.\n", "RESET");
        }

        std::shared_ptr<Process> cmd_screen_add(const std::string& process_name) {
            // Check if the process already exists
            while(tester.isActive() && !tester.isLocked()) {}
            tester.lock();

            for (const auto& process : processes) {
                if (process.second->name == process_name) {
                    std::cout << "Error! Process " << process_name << " already exists.\n";
                    processHistory["Main"].emplace_back("Error! Process " + process_name + " already exists.\n", "RESET");
                    tester.unlock();
                    return nullptr;  // Exit the function if a duplicate is found
                }
            }
            // Set random number of instructions
            long long instructions = processMinIns + (rand() % (processMaxIns - processMinIns + 1));
            // If no duplicates, create and add the new process
            std::shared_ptr<Process> newProcess = std::make_shared<Process>(process_name, instructions, getCurrentTimestamp(), memoryPerProcess);
            processes.insert(std::make_pair(process_name, newProcess));

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
                processHistory["Main"].emplace_back("Enter a command: \n", "RESET");
                std::cout << "Error! Empty input.\n";
                processHistory["Main"].emplace_back("Error! Empty input.\n", "RESET");
                return;
            }

            std::string command = tokens[0];  // First token is the command

            
            if (!isInMainConsole) {
                if (command == "exit") {
                    isInMainConsole = true;
                    processHistory[current_process].emplace_back("Enter a command: exit\n\n", "RESET");
                    cmd_clear();
                }
                else if (command == "process-smi"){
                    for(const auto& process: processes) {
                        if (process.second->name == current_process) {
                            std::ostringstream output; 
                            output << "\nProcess: " << process.second->name << "\n";
                            output << "ID: " << process.second->id << "\n\n";
                            
                            if (process.second->completed) {
                                output << "Finished!\n\n";
                            } else {
                                output << "Current instruction line: " << process.second->current_instruction << "\n";
                                output << "Lines of code: " << process.second->total_instructions << "\n\n";
                            }

                            std::cout << output.str();
                            processHistory[process.second->name].emplace_back("Enter a command: process-smi\n", "RESET");
                            processHistory[process.second->name].emplace_back(output.str(), "RESET");

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
                    processHistory[current_process].emplace_back(commandOutput.str(), "RESET");
                    processHistory[current_process].emplace_back(output.str(), "RESET");

                    return;
                } 
                
            }
            else if (command == "clear") {
                processHistory["Main"].clear();
                cmd_clear();
            }
            else if (command == "exit") {
                terminate();
                std::exit(0);
            }
            else if (command == "initialize" || command == "init") {
                processHistory["Main"].emplace_back("Enter a command: initialize\n", "RESET");
                cmd_initialize();
            }
            else if (command == "screen") {
                processHistory["Main"].emplace_back("Enter a command: "+ input +"\n", "RESET");

                if(!isInitialized) {
                    std::cout << "Error! System not initialized.\n";
                    processHistory["Main"].emplace_back("Error! System not initialized.\n", "RESET");
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
                        if(process.second->completed) {
                            finishedProcesses.push_back(*process.second);
                        } else {
                            runningProcesses.push_back(*process.second);
                        }
                    }

                    std::vector<std::pair<std::string, std::string>> processDetails = printProcesses(totalCores, runningProcesses, finishedProcesses);
                    processHistory["Main"].insert(processHistory["Main"].end(), processDetails.begin(), processDetails.end());
                }   
                else {
                    std::cout << "Error! Correct usage: screen -s <process_name> or screen -r <process_name> or screen -ls\n";
                    processHistory["Main"].emplace_back("Error! Correct usage: screen -s <process_name> or screen -r <process_name> or screen -ls\n", "RESET");
                }
            }
            else if (command == "scheduler-test" || command == "st") {
                processHistory["Main"].emplace_back("Enter a command: scheduler-test\n", "RESET");

                if(!isInitialized) {
                    std::cout << "Error! System not initialized.\n";
                    processHistory["Main"].emplace_back("Error! System not initialized.\n", "RESET");
                    return;
                }
                cmd_scheduler_test();
            }
            else if (command == "scheduler-stop" || command == "ss") {
                processHistory["Main"].emplace_back("Enter a command: scheduler-stop\n", "RESET");

                if(!isInitialized) {
                    std::cout << "Error! System not initialized.\n";
                    processHistory["Main"].emplace_back("Error! System not initialized.\n", "RESET");
                    return;
                }
                cmd_scheduler_stop();
            }
            else if (command == "report-util") {
                processHistory["Main"].emplace_back("Enter a command: report-util\n", "RESET");
                if(!isInitialized) {
                    std::cout << "Error! System not initialized.\n";
                    processHistory["Main"].emplace_back("Error! System not initialized.\n", "RESET");
                    return;
                }
                cmd_report_util();
            }
            else if (command == "process-smi"){
                processHistory["Main"].emplace_back("Enter a command: process-smi\n", "RESET");
                if(!isInitialized) {
                    std::cout << "Error! System not initialized.\n";
                    processHistory["Main"].emplace_back("Error! System not initialized.\n", "RESET");
                    return;
                }
                std::vector<Process> runningProcesses;
                int running_ctr = 0; 


                for(const auto& process: processes) {
                    if(!process.second->completed) {
                        runningProcesses.push_back(*process.second);
                        if (process.second->core != -1){
                            running_ctr++;
                        }
                    }
                }

                MemoryStats stats = memory->getMemoryStats();
                int memory_usage = 0;
                for (auto memoryRegion = stats.processMemoryRegions.rbegin(); memoryRegion != stats.processMemoryRegions.rend(); ++memoryRegion) {
                    int mem_usage = (memoryRegion->endAddress - memoryRegion->startAddress)+1;
                    memory_usage += mem_usage;
                }

                int memory_util = static_cast<double>(memory_usage) / memAdd * 100;
                int cpu_util = static_cast<double>(running_ctr) / totalCores * 100;
                cpu_util = cpu_util < 0 ? 0 : cpu_util;   
                printColored("--------------------------------------------------\n", BLUE);
                printColored("|", BLUE);  
                std::cout << " PROCESS-SMI V01";
                printColored(".00 ", YELLOW);
                std::cout << "Driver Version: ";
                printColored("01.00 ", YELLOW);
                printColored("|\n", BLUE);
                printColored("--------------------------------------------------\n", BLUE);
                std::cout << "CPU utilization: ";
                printColored(std::to_string(cpu_util), YELLOW);
                printColored("%\n", BLUE);
                std::cout << "Memory Usage: ";
                printColored(std::to_string(memory_usage), YELLOW);
                printColored("MiB ", YELLOW);
                printColored("/ ", BLUE);
                printColored(std::to_string(memAdd), YELLOW);
                printColored("MiB\n", YELLOW);
                std::cout << "Memory Util: ";
                printColored(std::to_string(memory_util), YELLOW);
                printColored("%\n", BLUE);
                printColored("==================================================\n", BLUE);
                std::cout << "Running processes ";
                printColored("and", BLUE);
                std::cout << " memory usage:\n";
                printColored("--------------------------------------------------\n", BLUE);

                // processHistory
                processHistory["Main"].emplace_back("--------------------------------------------------\n", "BLUE");
                processHistory["Main"].emplace_back("|", "BLUE");
                processHistory["Main"].emplace_back(" PROCESS-SMI V01", "RESET");
                processHistory["Main"].emplace_back(".00 ", "YELLOW");
                processHistory["Main"].emplace_back("Driver Version: ", "RESET");
                processHistory["Main"].emplace_back("01.00 ", "YELLOW");
                processHistory["Main"].emplace_back("|\n", "BLUE");
                processHistory["Main"].emplace_back("--------------------------------------------------\n", "BLUE");
                processHistory["Main"].emplace_back("CPU utilization: ", "RESET");
                processHistory["Main"].emplace_back(std::to_string(cpu_util), "YELLOW");
                processHistory["Main"].emplace_back("%\n", "BLUE");
                processHistory["Main"].emplace_back("Memory Usage: ", "RESET");
                processHistory["Main"].emplace_back(std::to_string(memory_usage), "YELLOW");
                processHistory["Main"].emplace_back("MiB ", "YELLOW");
                processHistory["Main"].emplace_back("/ ", "BLUE");
                processHistory["Main"].emplace_back(std::to_string(memAdd), "YELLOW");
                processHistory["Main"].emplace_back("MiB\n", "YELLOW");
                processHistory["Main"].emplace_back("Memory Util: ", "RESET");
                processHistory["Main"].emplace_back(std::to_string(memory_util), "YELLOW");
                processHistory["Main"].emplace_back("%\n", "BLUE");
                processHistory["Main"].emplace_back("==================================================\n", "BLUE");
                processHistory["Main"].emplace_back("Running processes ", "RESET");
                processHistory["Main"].emplace_back("and", "BLUE");
                processHistory["Main"].emplace_back(" memory usage:\n", "RESET");
                processHistory["Main"].emplace_back("--------------------------------------------------\n", "BLUE");
                

                for (auto memoryRegion = stats.processMemoryRegions.rbegin(); memoryRegion != stats.processMemoryRegions.rend(); ++memoryRegion) {
                    int total_memory = (memoryRegion->endAddress - memoryRegion->startAddress)+1;
                    std::cout << memoryRegion->process_name << " ";
                    printColored(std::to_string(total_memory), YELLOW);
                    printColored("MiB\n", YELLOW);

                    // processHistory
                    processHistory["Main"].emplace_back(std::to_string(total_memory), "YELLOW");
                    processHistory["Main"].emplace_back("MiB\n", "YELLOW");
                    // std::cout << memoryRegion->endAddress << "\n" << memoryRegion->process_name << "\n" << memoryRegion->startAddress << "\n\n";                
                }
                printColored("--------------------------------------------------\n\n", BLUE);
                processHistory["Main"].emplace_back("--------------------------------------------------\n\n", "BLUE");
            }
            else if (command == "vmstat"){
                processHistory["Main"].emplace_back("Enter a command: vmstat\n", "RESET");
                if(!isInitialized) {
                    std::cout << "Error! System not initialized.\n";
                    processHistory["Main"].emplace_back("Error! System not initialized.\n", "RESET");
                    return;
                }

                MemoryStats stats = memory->getMemoryStats();
                int memory_usage = 0;
                for (auto memoryRegion = stats.processMemoryRegions.rbegin(); memoryRegion != stats.processMemoryRegions.rend(); ++memoryRegion) {
                    int mem_usage = (memoryRegion->endAddress - memoryRegion->startAddress)+1;
                    memory_usage += mem_usage;
                }
                int free_memory = memAdd - memory_usage;
                
                printf("%13d %s\n", memAdd, "K total memory");
                printf("%13d %s\n", memory_usage, "K used memory");
                printf("%13d %s\n", free_memory, "K free memory");
                printf("%13s %s\n", "", "idle cpu ticks");
                printf("%13s %s\n", "", "active cpu ticks");
                printf("%13s %s\n", "", "total cpu ticks");
                printf("%13s %s\n", "", "num paged in");
                printf("%13s %s\n\n", "", "num paged out");
            }
            else {
                processHistory["Main"].emplace_back("Enter a command: "+ input +"\n", "RESET");
                std::cout << "Error! Unrecognized command\n";
                processHistory["Main"].emplace_back("Error! Unrecognized command\n", "RESET");
            }
        }
};