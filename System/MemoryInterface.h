#include<cstdint>
#include<memory>
#include<iostream>
#include<mutex>
#include<vector>
#include<condition_variable>
#include"../DataTypes/Freelist.h"

class MemoryInterface {
    private:
        uint64_t startAddress;
        uint64_t endAddress;
        uint64_t frameSize;
        uint64_t memorySize;
        std::vector<std::shared_ptr<MemoryFrame>> memoryFrames;
        std::mutex mtx;
        std::condition_variable cv;

        void createChunks() {
            uint64_t num_frames = memorySize / frameSize;
            uint64_t start_addr = 0;

            for(uint64_t i = 0; i < num_frames; i++) {
                this->memoryFrames.push_back(std::make_shared<MemoryFrame>(frameSize, start_addr, false));
                start_addr += frameSize;
            }
        }
    public:
        MemoryInterface
        {} )()ecafretnIyrom
};