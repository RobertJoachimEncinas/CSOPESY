#ifndef BACKINGSTORE
#define BACKINGSTORE
#include<map>
#include<string>
#include"..\DataTypes\Memory.h"

class BackingStore {
    private:
        std::map<std::string, std::string> storeDirectory;
        const std::string backingStoreDirectory = "..\\BackingStore\\"; 

    public:
        BackingStore() {

        }

        AllocatedMemory* retrieve(std::string process_name) {

        }

        void store(AllocatedMemory* memory) {

        }
};

#endif