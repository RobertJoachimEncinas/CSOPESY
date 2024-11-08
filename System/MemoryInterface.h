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
#include "../DataTypes/Freelist.h"

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
        uint64_t memorySize;
        MemoryFrame* memoryStart;
        FreeList* freeList;
        std::mutex mtx;
        std::condition_variable cv;
        std::string (*getCurrentTimestamp)();

        MemoryStats computeMemoryStats() {
            std::unique_lock<std::mutex> lock(mtx);
            MemoryStats stats = {0, 0, {}};

            MemoryFrame *temp;
            temp = memoryStart;
            do {
                if(temp->isInUse) {
                    stats.processes_in_memory += 1;
                    stats.processMemoryRegions.push_back({temp->startAddress, temp->endAddress, temp->owningProcess});
                } else {
                    stats.totalFragmentation += temp->size;
                }
                temp = temp->next;
            } while(temp != nullptr);

            lock.unlock();
            return stats;
        }

    public:
        ~MemoryInterface() {
            delete freeList;
            MemoryFrame *next, *temp;
            temp = memoryStart;
            do {
                next = temp->next;
                delete temp;
                temp = next;
            } while(temp != nullptr);
        }

        MemoryInterface() {}

        MemoryInterface(uint64_t memorySize, std::string (*getCurrentTimestamp)()) {
            this->memorySize = memorySize;
            this->startAddress = 0;
            this->endAddress = memorySize;
            this->memoryStart = nullptr;
            
            this->freeList = new FirstFitFreeList();

            this->getCurrentTimestamp = getCurrentTimestamp;
        }

        void initialize(uint64_t max_mem) {
            this->memorySize = max_mem;
            this->memoryStart = new MemoryFrame(memorySize, 0, nullptr, nullptr, "", false);
            this->freeList->push(memoryStart);
            this->endAddress = memorySize - 1;
        }

        MemoryFrame* allocate(uint64_t size, std::string owningProcess) {
            std::unique_lock<std::mutex> lock(mtx);
            MemoryFrame* allocated = freeList->pop(size);

            if(allocated == nullptr) {
                return allocated;
            }

            allocated->owningProcess = owningProcess;

            if(allocated->startAddress == 0) {
                this->memoryStart = allocated;
            }

            lock.unlock();
            return allocated;
        }

        void free(MemoryFrame* chunk) {
            std::unique_lock<std::mutex> lock(mtx);
            MemoryFrame* previousChunk = chunk->prev;
            MemoryFrame* nextChunk = chunk->next;

            chunk->owningProcess = "";
            chunk->isInUse = false;

            if(previousChunk != nullptr) {
                if(!previousChunk->isInUse) {
                    //Remove the previous chunk from the freelist
                    freeList->remove(previousChunk);

                    //Merge previousChunk into the chunk being freed
                    chunk->prev = previousChunk->prev;
                    chunk->size += previousChunk->size;
                    chunk->startAddress = previousChunk->startAddress;

                    //Set previousChunk's previous to point to the new merged chunk
                    if(previousChunk->prev != nullptr) {
                        previousChunk->prev->next = chunk;
                    }

                    //Reset head of chunk linked list
                    if(chunk->startAddress == 0) {
                        this->memoryStart = chunk;
                    }

                    //Free the memory of the previous chunk
                    delete previousChunk;
                }
            }

            if(nextChunk != nullptr) {
                if(!nextChunk->isInUse) {
                    //Remove the next chunk from the freelist
                    freeList->remove(nextChunk);

                    //Merge nextChunk into the chunk being freed
                    chunk->next = nextChunk->next;
                    chunk->size += nextChunk->size;
                    chunk->endAddress = nextChunk->endAddress;

                    //Set nextChunk's next to point to the new merged chunk
                    if(nextChunk->next != nullptr) {
                        nextChunk->next->prev = chunk;
                    }

                    //Free the memory of the next chunk
                    delete nextChunk;
                }
            }

            //Add the freed and coalesced chunk into the freelist
            freeList->push(chunk);
            lock.unlock();
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