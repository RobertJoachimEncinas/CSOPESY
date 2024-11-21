#ifndef MEMORYCHUNK
#define MEMORYCHUNK

#include<cstdint>
#include<memory>
#include<string>
#include<sstream>

class AllocatedMemory {
    public:
        uint64_t size;
        uint64_t startAddress;
        uint64_t endAddress;
        bool isInUse;
        std::string owningProcess;

        virtual ~AllocatedMemory() {}

        virtual AllocatedMemory* getPartition(uint64_t partitionSize) { return nullptr; }
};

class MemoryChunk: public AllocatedMemory {
    public:
        MemoryChunk* next;
        MemoryChunk* prev;
        
        ~MemoryChunk() {}

        MemoryChunk() {}

        MemoryChunk(uint64_t size, uint64_t startAddress, MemoryChunk* next, MemoryChunk* prev, std::string owningProcess, bool isInUse = false) {
            this->size = size;
            this->startAddress = startAddress;
            this->endAddress = startAddress + size - 1;
            this->next = next;
            this->prev = prev;
            this->isInUse = isInUse;
            this->owningProcess = owningProcess;
        }

        MemoryChunk* getPartition(uint64_t partitionSize) override {
            if(partitionSize > this->size) {
                return nullptr;
            }

            MemoryChunk* partitionChunk = new MemoryChunk(partitionSize, this->startAddress, this, this->prev, "", true);
            MemoryChunk* previousChunk = partitionChunk->prev;

            //Edit the data of the right split of the chunk (represented by "this")
            this->size = this->size - partitionSize;
            this->startAddress = this->startAddress + partitionSize;
            this->prev = partitionChunk;
            
            //Edit the data of the previous chunk
            if(previousChunk != nullptr) {
                previousChunk->next = partitionChunk;
            }

            return partitionChunk;
        }
};

class MemoryFrame: public AllocatedMemory {

};

#endif