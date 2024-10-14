/*
    This file defines all code related to processes
*/
#ifndef PROCESS
#define PROCESS
#include <string>
#include <iostream>

class Process {
    public:
        std::string name;
        int current_instruction;
        int total_instructions;
        std::string timestamp;
        bool completed;

        Process() {}

        Process(std::string name, int total_instructions, std::string timestamp) {
            this->name = name;
            this->current_instruction = 1;
            this->total_instructions = total_instructions;
            this->timestamp = timestamp;
            this->completed = false;
        }

        bool executeLine() {
            current_instruction++;
            this->completed = current_instruction > total_instructions;
            return completed;
        }
};
#endif