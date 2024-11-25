#ifndef BACKINGSTORE
#define BACKINGSTORE
#include<map>
#include<string>
#include"..\DataTypes\Memory.h"

struct MemoryData {
    uint64_t size; 
    std::string owningProcess;
};

class BackingStore {
    private:
        std::map<std::string, std::string> storeDirectory;
        const std::string backingStoreDirectory = "..\\BackingStore\\"; 

    public:
        BackingStore() {

        }

        MemoryData retrieve(std::string process_name) {
            if (storeDirectory.find(process_name) == storeDirectory.end()) {
                return { 0, "" };
            }

            //READ FILE

            MemoryData retrievedData;
        }

        void store(AllocatedMemory memory) {
            
        }
};

#endif