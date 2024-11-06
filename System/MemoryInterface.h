#pragma once
#include<cstdint>
#include<memory>
#include<iostream>
#include<mutex>
#include<vector>
#include<condition_variable>
#include "../DataTypes/MemoryChunk.h"

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
        MemoryInterface() {}
        MemoryInterface(uint64_t frameSize, uint64_t memorySize) {
            this->frameSize = frameSize;
            this->memorySize = memorySize;
            this->startAddress = 0;
            this->endAddress = 0;  
        }

        void initialize(uint64_t frameSize, uint64_t memorySize) {
            this->frameSize = frameSize;
            this->memorySize = memorySize;
            this->endAddress = memorySize - 1;
            createChunks(); 
        }

        void free(std::vector<std::shared_ptr<MemoryFrame>> frames) {
            std::unique_lock<std::mutex> lock(mtx);
            for(auto& frame: frames) {
                frame->isInUse = false;
            }
            
            uint64_t allocatedFramesCount = 0;
            for (auto& frame: memoryFrames) {
                if (frame->isInUse) {
                    allocatedFramesCount++;
                }
            }
        }

        std::vector<std::shared_ptr<MemoryFrame>> allocate(uint64_t process_size) {
            std::unique_lock<std::mutex> lock(mtx);
            std::vector<std::shared_ptr<MemoryFrame>> allocatedFrames;
            uint64_t allocatedSize = 0;
            for(auto& frame: memoryFrames) {
                if(!frame->isInUse) {
                    allocatedFrames.push_back(frame);
                    allocatedSize += frame->size;
                }

                if(allocatedSize >= process_size) {
                    break;
                }
            }

            if(allocatedSize < process_size) {
                return {};
            }

            for(auto& frame: allocatedFrames) {
                frame->isInUse = true;
            }

            return allocatedFrames;
        }
};