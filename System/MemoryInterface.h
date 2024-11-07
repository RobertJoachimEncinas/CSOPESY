#pragma once
#include<cstdint>
#include<memory>
#include<iostream>
#include<mutex>
#include<set>
#include<vector>
#include<condition_variable>
#include<sstream>
#include "../DataTypes/MemoryChunk.h"

struct ProcessMemory {
    uint64_t startAddress;
    uint64_t endAddress;
    std::string process_name;
};

struct MemoryStats {
    uint64_t processes_in_memory;
    uint64_t totalFragmentation;
    std::vector<ProcessMemory> processMemoryRegions;
};

class MemoryInterface {
    private:
        uint64_t startAddress;
        uint64_t endAddress;
        uint64_t frameSize;
        uint64_t memorySize;
        std::vector<std::shared_ptr<MemoryFrame>> memoryFrames;
        std::mutex mtx;
        std::condition_variable cv;
        std::string (*getCurrentTimestamp)();

        void createChunks() {
            uint64_t num_frames = memorySize / frameSize;
            uint64_t start_addr = 0;

            for(uint64_t i = 0; i < num_frames; i++) {
                this->memoryFrames.push_back(std::make_shared<MemoryFrame>(frameSize, start_addr));
                start_addr += frameSize;
            }
        }

        MemoryStats computeMemoryStats() {
            std::unique_lock<std::mutex> lock(mtx);
            std::set<std::string> unique_processes;
            MemoryStats stats = {0, 0, {}};
            ProcessMemory currentProcess = {0, 0, ""};

            for(auto& frame: memoryFrames) {
                if((frame.get())->isInUse) {
                    if(currentProcess.process_name != frame.get()->owningProcess) {
                        if(currentProcess.process_name != "") { 
                            stats.processMemoryRegions.push_back(currentProcess);
                        }

                        currentProcess.process_name = frame.get()->owningProcess;
                        currentProcess.startAddress = frame.get()->startAddress;
                        currentProcess.endAddress = frame.get()->endAddress;
                    } else {
                        currentProcess.endAddress = frame.get()->endAddress; //Update end address only as memoryFrames is in order
                    }

                    unique_processes.insert(frame.get()->owningProcess); //This wont insert if the process is already in the set
                } else {
                    //Frame not in use with a non-default current process signals end of that process' memory region
                    if(currentProcess.process_name != "") { 
                        stats.processMemoryRegions.push_back(currentProcess);
                    }

                    //Reset the current process to default as the contiguous block for that process is done
                    currentProcess.process_name = "";
                    currentProcess.startAddress = 0;
                    currentProcess.endAddress = 0;

                    stats.totalFragmentation += frame.get()->size;
                }
            }

            if(currentProcess.process_name != "") { 
                stats.processMemoryRegions.push_back(currentProcess);
            }

            stats.processes_in_memory = unique_processes.size();

            lock.unlock();
            return stats;
        }

    public:
        MemoryInterface() {}
        MemoryInterface(uint64_t frameSize, uint64_t memorySize, std::string (*getCurrentTimestamp)()) {
            this->frameSize = frameSize;
            this->memorySize = memorySize;
            this->startAddress = 0;
            this->endAddress = 0;  
            this->getCurrentTimestamp = getCurrentTimestamp;
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

        std::vector<std::shared_ptr<MemoryFrame>> allocate(uint64_t process_size, std::string owningProcess) {
            std::unique_lock<std::mutex> lock(mtx);
            std::vector<std::shared_ptr<MemoryFrame>> allocatedFrames;
            uint64_t allocatedSize = 0;
            for(auto& frame: memoryFrames) {
                if(!frame->isInUse) {
                    allocatedFrames.push_back(frame);
                    allocatedSize += frame->size;
                } else if(allocatedFrames.size() > 0) {
                    allocatedFrames.clear();
                    allocatedSize = 0;
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
                frame->owningProcess = owningProcess;
            }

            return allocatedFrames;
        }

        
        void printMemory(long long quantum_cycle) {
            MemoryStats stats = computeMemoryStats();
            std::ostringstream oss;

            std::string fileMemoryPath = "./Logs/memory_stamp_" + std::to_string(quantum_cycle) + ".txt";
            FILE* f = fopen(fileMemoryPath.c_str(), "a");
            fprintf(f, "Timestamp: (%s)\n", getCurrentTimestamp().c_str());
            fprintf(f, "Number of process in memory: %llu\n", stats.processes_in_memory);
            fprintf(f, "Total external fragmentation in KB: %llu\n", stats.totalFragmentation);
            fprintf(f, "----end---- = %llu\n\n", endAddress);
            for (auto memoryRegion = stats.processMemoryRegions.rbegin(); memoryRegion != stats.processMemoryRegions.rend(); ++memoryRegion) {
                oss << memoryRegion->endAddress << "\n" << memoryRegion->process_name << "\n" << memoryRegion->startAddress << "\n\n";                
            }
            fprintf(f, "%s", oss.str().c_str());
            fprintf(f, "----start---- = %llu\n\n", startAddress);
            fclose(f);
        }
};  