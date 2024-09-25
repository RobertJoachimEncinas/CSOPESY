/*
    This file defines all code related to processes
*/

#include <string>

struct Process {
    std::string name;
    int current_instruction;
    int total_instructions;
    std::string timestamp;
};