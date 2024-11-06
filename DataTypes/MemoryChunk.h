#ifndef MEMORYCHUNK
#define MEMORYCHUNK

#include<cstdint>
#include<memory>
#include<iostream>
#include<sstream>

class MemoryChunk {
    public:
        uint64_t size;
        uint64_t startAddress;
        uint64_t endAddress;
        bool isInUse;
        MemoryChunk* next;
        MemoryChunk* prev;

        MemoryChunk() {}

        MemoryChunk(uint64_t size, uint64_t startAddress, MemoryChunk* next, MemoryChunk* prev, bool isInUse = false) {
            this->size = size;
            this->startAddress = startAddress;
            this->endAddress = startAddress + size - 1;
            this->next = next;
            this->prev = prev;
            this->isInUse = isInUse;
        }

        MemoryChunk* getPartition(uint64_t partitionSize) {
            if(partitionSize > this->size) {
                return nullptr;
            }

            MemoryChunk* partitionChunk = new MemoryChunk(partitionSize, this->startAddress, this, this->prev, true);
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

        //FOR DEBUG ONLY REMOVE LATER ON
        std::string toString() const {
            std::ostringstream oss;
            oss << "MemoryChunk @" << this 
                << "[Size: " << size
                << ", Start Address: " << startAddress
                << ", End Address: " << endAddress
                << ", In Use: " << (isInUse ? "Yes" : "No")
                << ", Prev: " << prev
                << ", Next: " << next
                << "]";
            return oss.str();
        }
};

#endif