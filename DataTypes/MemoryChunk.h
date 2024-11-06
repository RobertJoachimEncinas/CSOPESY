#ifndef MEMORYCHUNK
#define MEMORYCHUNK

#include<cstdint>
#include<memory>
#include<iostream>
#include<sstream>

class MemoryFrame {
    public:
        uint64_t size;
        uint64_t startAddress;
        uint64_t endAddress;
        bool isInUse;

        MemoryFrame() {}

        MemoryFrame(uint64_t size, uint64_t startAddress, bool isInUse = false) {
            this->size = size;
            this->startAddress = startAddress;
            this->endAddress = startAddress + size - 1;
            this->isInUse = isInUse;
        }
};

#endif