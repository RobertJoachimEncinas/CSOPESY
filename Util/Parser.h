#ifndef PARSER
#define PARSER

#include"DataTypes/System.h"
#include"DataTypes/Exceptions.h"

#ifndef STDIOSTREAM
#define STDIOSTREAM
#include<iostream>
#endif

#ifndef STDSTRING
#define STDSTRING
#include<string>
#endif

#ifndef STDSSTREAM
#define STDSSTREAM
#include<sstream>
#endif

#ifndef STDFSTREAM
#define STDFSTREAM
#include<fstream>
#endif

#ifndef STDVECTOR
#define STDVECTOR
#include<vector>
#endif

/* Constants for valid values of the config variables */
const std::string NUM_CPU_RANGE = "[1, 128]";
const std::string SCHEDULER_RANGE = "[\"fcfs\", \"rr\"]";
//misc_range is for all the other configs to reduce memory consumption
const std::string MISC_RANGE = "[1, 2^32]";
const std::string DELAYS_RANGE = "[0, 2^32]";

/* Constants for min/max values */
#define MIN_CPU_AND_MISC_VALUE 1
#define MAX_CPU_VALUE 128
#define MIN_DELAYS_VALUE 0
#define MAX_MISC_AND_DELAYS_VALUE 4294967296

System initialize(std::string filePath) {
    int num_cpu;
    Scheduler scheduler;
    long long quantum_cycles;
    long long batch_process_freq;
    long long min_ins;
    long long max_ins;
    long long delays_per_exec;
    //init_flags is a bitstring of flags for each setting. All need to be 1 to have the whole system be considered properly initialized
    char init_flags = 0;
    std::string line;
    std::stringstream strstream;
    std::string token;
    std::vector<std::string> tokens;
    std::ifstream file(filePath);
    int line_number = 0;

    if(file.is_open()) {
        try {
            while(std::getline(file, line)) {
                line_number++;
                strstream << line;
                while(std::getline(strstream, token, ' ')) {
                    tokens.push_back(token);
                }

                if(tokens.size() != 2) {
                    std::cout << "Error at line " << line_number << ". Expected line to be in \'config_variable value\' format. ";
                    std::cout << "Received " << line;
                    return System();
                }

                if(tokens[0].compare("num-cpu") == 0) {
                    num_cpu = std::stoi(tokens[1]);
                    if(num_cpu >= MIN_CPU_AND_MISC_VALUE && num_cpu <= MAX_CPU_VALUE) {
                        throw InvalidConfigValueException(tokens[1], NUM_CPU_RANGE);
                    } 
                } else if(tokens[0].compare("scheduler") == 0) {
                    
                }
            }

            file.close();
        } catch(const std::exception& e) {
            if(file.is_open()) {
                file.close();
            }

            return System();
        }
    } else {
        std::cout << "Error! Config is currently being used by another process.\n";
        return System(); 
    }

} 

#endif