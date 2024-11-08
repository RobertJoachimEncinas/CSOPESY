#ifndef MEMORYCHUNK
#define MEMORYCHUNK

#include<cstdint>
#include<memory>
#include<string>
#include<sstream>

class MemoryFrame {
    public:
        uint64_t size;
        uint64_t startAddress;
        uint64_t endAddress;
        bool isInUse;
        MemoryFrame* next;
        MemoryFrame* prev;
        std::string owningProcess;

        MemoryFrame() {}

        MemoryFrame(uint64_t size, uint64_t startAddress, MemoryFrame* next, MemoryFrame* prev, std::string owningProcess, bool isInUse = false) {
            this->size = size;
            this->startAddress = startAddress;
            this->endAddress = startAddress + size - 1;
            this->next = next;
            this->prev = prev;
            this->isInUse = isInUse;
            this->owningProcess = owningProcess;
        }

        MemoryFrame* getPartition(uint64_t partitionSize) {
            if(partitionSize > this->size) {
                return nullptr;
            }

            MemoryFrame* partitionChunk = new MemoryFrame(partitionSize, this->startAddress, this, this->prev, "", true);
            MemoryFrame* previousChunk = partitionChunk->prev;

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

#endif