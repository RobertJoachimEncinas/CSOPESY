/*
    This file defines all code related to processes
*/
#pragma once
#include <string>
#include <iostream>
#include <chrono>

class Process {
    public:
        std::string name;
        long long current_instruction;
        long long total_instructions;
        std::string timestamp;
        bool completed;
        int core;
        std::string logFilePath;
        int id;
        static int last_id;

        Process() {}

        Process(std::string name, long long total_instructions, std::string timestamp) {
            this->name = name;
            this->id = last_id++;;
            this->current_instruction = 0;
            this->total_instructions = total_instructions;
            this->timestamp = timestamp;
            this->completed = false;
            this->core = -1;
            this->logFilePath = "./Logs/" + name + ".txt";

            // FILE* f = fopen(logFilePath.c_str(), "w");

            // fprintf(f, "Process name: %s\n", name.c_str());
            // fprintf(f, "Logs:\n\n");

            // fclose(f);
        }

        bool executeLine(std::string exec_timestamp, int coreId) {
            //Execution
            current_instruction++;
            //log(exec_timestamp, coreId);
            this->completed = current_instruction >= total_instructions;
            return this->completed;
        }

        void log(std::string exec_timestamp, int coreId) {
            FILE* f = fopen(logFilePath.c_str(), "a");
            fprintf(f, "(%s) Core:%d \"Hello world from %s\"\n", exec_timestamp.c_str(), coreId, name.c_str());
            fclose(f);
        }

        void setCore(int core) {
            this->core = core;
        }
};
int Process::last_id = 0;