#ifndef BACKINGSTORE
#define BACKINGSTORE
#include<map>
#include<string>
#include<filesystem>
#include"..\DataTypes\Memory.h"
#include"..\DataTypes\Process.h"

class BackingStore {
    private:
        std::map<std::string, std::string> bsDirectory;
        const std::string dirPrefix = ".\\BackingStore\\"; 
        std::filesystem::path directory = ".\\BackingStore\\";
        uint64_t pagedInCount;
        uint64_t pagedOutCount;  
        bool isPagingAllocator;
        uint64_t pageSize; 
        std::mutex mtx;
        
        bool isIn(std::string name) {
            return bsDirectory.find(name) != bsDirectory.end();
        }

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
            std::lock_guard<std::mutex> l(mtx);

            if(!isIn(process_name)) {
                return 0;
            }

            std::string backingStorePath = bsDirectory[process_name];
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

            bsDirectory.erase(process_name);

            return size;
        }

        void store(Process* p) {
            std::lock_guard<std::mutex> l(mtx);
            std::string backingStorePath = dirPrefix + p->name + ".txt";
            FILE* f = fopen(backingStorePath.c_str(), "w");
            fprintf(f, "%lld", p->memoryRequired);
            fclose(f);

            if(this->isPagingAllocator) {
                this->pagedOutCount += (p->memoryRequired + pageSize - 1) / pageSize;
            } else {
                this->pagedOutCount += 1;
            }

            bsDirectory.insert({p->name, backingStorePath});
        }

        uint64_t getPagedIn() {
            return this->pagedInCount;
        }

        uint64_t getPagedOut() {
            return this->pagedOutCount;
        }
};

#endif