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

        Process() {}

        Process(std::string name, int total_instructions, std::string timestamp) {
            this->name = name;
            this->current_instruction = 1;
            this->total_instructions = total_instructions;
            this->timestamp = timestamp;
            this->completed = false;
            this->core = -1;
        }

        bool executeLine() {
            this->completed = current_instruction >= total_instructions;

            if(this->completed) {
                return true;
            }
            current_instruction++;
            return false;
        }

        void assign(int core) {
            this->core = core;
        }
};
#endif