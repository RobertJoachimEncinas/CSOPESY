#ifndef BACKINGSTORE
#define BACKINGSTORE
#include<map>
#include<string>
#include<filesystem>
#include"..\DataTypes\Memory.h"
#include"..\DataTypes\Process.h"

class BackingStore {
    private:
        const std::string dirPrefix = ".\\BackingStore\\"; 
        std::filesystem::path directory = ".\\BackingStore\\";

    public:
        BackingStore() {
            if (std::filesystem::exists(directory)) {
                std::filesystem::remove_all(directory); 
            } 

            std::filesystem::create_directory(directory); 
            FILE* f = fopen(".\\BackingStore\\.gitkeep", "w");
            fclose(f);
        }

        uint64_t retrieve(std::string process_name) {
            std::string backingStorePath = dirPrefix + process_name + ".txt";
            std::ifstream inputFile(backingStorePath); 

            if (!inputFile) {
                return 0;
            }

            uint64_t size;
            inputFile >> size;

            inputFile.close();
            remove(backingStorePath.c_str());

            return size;
        }

        void store(Process* p) {
            std::string backingStorePath = dirPrefix + p->name + ".txt";
            FILE* f = fopen(backingStorePath.c_str(), "w");
            fprintf(f, "%d", p->memoryRequired);
            fclose(f);
        }
};

#endif