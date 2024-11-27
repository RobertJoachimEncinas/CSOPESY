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
        uint64_t pagedInCount;
        uint64_t pagedOutCount;  
        bool isPagingAllocator;
        uint64_t pageSize; 

    public:
        BackingStore() {
            if (std::filesystem::exists(directory)) {
                std::filesystem::remove_all(directory); 
            } 

            std::filesystem::create_directory(directory); 
            FILE* f = fopen(".\\BackingStore\\.gitkeep", "w");
            fclose(f);

            this->pagedInCount = 0;
            this->pagedOutCount = 0;
        }

        void init(bool isPaging, uint64_t size = 0) {
            this->isPagingAllocator = isPaging;

            if(isPagingAllocator) {
                this->pageSize = size;
            } else {
                this->pageSize = 0;
            }
        }

        uint64_t retrieve(std::string process_name) {
            std::string backingStorePath = dirPrefix + process_name + ".txt";
            std::ifstream inputFile(backingStorePath); 

            if (!inputFile) {
                return 0;
            }

            uint64_t size;
            inputFile >> size;

            if(this->isPagingAllocator) {
                this->pagedInCount += (size + pageSize - 1) / pageSize;
            } else {
                this->pagedInCount += 1;
            }

            inputFile.close();
            remove(backingStorePath.c_str());

            return size;
        }

        void store(Process* p) {
            std::string backingStorePath = dirPrefix + p->name + ".txt";
            FILE* f = fopen(backingStorePath.c_str(), "w");
            fprintf(f, "%d", p->memoryRequired);
            fclose(f);

            if(this->isPagingAllocator) {
                this->pagedOutCount += (p->memoryRequired + pageSize - 1) / pageSize;
            } else {
                this->pagedOutCount += 1;
            }
        }

        uint64_t getPagedIn() {
            return this->pagedInCount;
        }

        uint64_t getPagedOut() {
            return this->pagedOutCount;
        }
};

#endif