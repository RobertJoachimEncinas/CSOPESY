/*
    This file defines all code related to processes
*/
#ifndef PROCESS
#define PROCESS
#include <string>
#include <iostream>
#include <chrono>

class Process {
    public:
        std::string name;
        int current_instruction;
        int total_instructions;
        std::string timestamp;
        bool completed;
        int core;
        std::string logFilePath;

        Process() {}

        Process(std::string name, int total_instructions, std::string timestamp) {
            this->name = name;
            this->current_instruction = 0;
            this->total_instructions = total_instructions;
            this->timestamp = timestamp;
            this->completed = false;
            this->core = -1;
            this->logFilePath = "./Logs/" + name + ".txt";

            FILE* f = fopen(logFilePath.c_str(), "w");

            fprintf(f, "Process name: %s", name.c_str());
            fprintf(f, "Logs:\n\n");

            fclose(f);
        }

        bool executeLine(std::string exec_timestamp) {
            //Execution
            current_instruction++;
            log(exec_timestamp);
            this->completed = current_instruction >= total_instructions;
            return this->completed;
        }

        void log(std::string exec_timestamp) {
            FILE* f = fopen(logFilePath.c_str(), "a");
            fprintf(f, "(%s) Core:%d \"Hello world from %s\"\n", exec_timestamp.c_str(), core, name.c_str());
            fclose(f);
        }

        void assign(int core) {
            this->core = core;
        }
};
#endif