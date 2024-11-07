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
        std::string owningProcess;

        MemoryFrame() {}

        MemoryFrame(uint64_t size, uint64_t startAddress) {
            this->size = size;
            this->startAddress = startAddress;
            this->endAddress = startAddress + size - 1;
            this->isInUse = false;
            this->owningProcess = "";
        }
};

#endif