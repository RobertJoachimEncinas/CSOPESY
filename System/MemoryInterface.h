#pragma once
#include<cstdint>
#include<memory>
#include<iostream>
#include<mutex>
#include<set>
#include<vector>
#include<condition_variable>
#include<sstream>
#include "../DataTypes/Memory.h"
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

class AbstractMemoryInterface {
    protected:
        uint64_t startAddress;
        uint64_t endAddress;
        uint64_t memorySize;
        FreeList* freeList;
        std::mutex mtx;
        std::condition_variable cv;
        std::string (*getCurrentTimestamp)();

        virtual MemoryStats computeMemoryStats() { return {0, 0, {}}; };
    public:
        AbstractMemoryInterface() {}

        AbstractMemoryInterface(uint64_t memorySize, std::string (*getCurrentTimestamp)()) {
            this->memorySize = memorySize;
            this->startAddress = 0;
            this->endAddress = memorySize;
            this->freeList = nullptr;
            this->getCurrentTimestamp = getCurrentTimestamp;
        }

        virtual ~AbstractMemoryInterface() {};
        virtual AllocatedMemory* allocate(uint64_t size, std::string owningProcess) { return nullptr; };
        virtual void free(AllocatedMemory* allocated) {};
        virtual void printMemory(long long quantum_cycle) {};
};

class FlatMemoryInterface: public AbstractMemoryInterface {
    private:
        MemoryChunk* memoryStart;

        MemoryStats computeMemoryStats() override {
            std::unique_lock<std::mutex> lock(mtx);
            MemoryStats stats = {0, 0, {}};

            MemoryChunk *temp;
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
        ~FlatMemoryInterface() {
            delete freeList;
            MemoryChunk *next, *temp;
            temp = memoryStart;
            do {
                next = temp->next;
                delete temp;
                temp = next;
            } while(temp != nullptr);
        }

        FlatMemoryInterface() {}

        FlatMemoryInterface(uint64_t memorySize, std::string (*getCurrentTimestamp)()) {
            this->memorySize = memorySize;
            this->startAddress = 0;
            this->endAddress = memorySize;
            this->memoryStart = new MemoryChunk(memorySize, 0, nullptr, nullptr, "", false);
            this->freeList = new FirstFitFreeList();
            this->freeList->push(memoryStart);
            this->getCurrentTimestamp = getCurrentTimestamp;
        }

        MemoryChunk* allocate(uint64_t size, std::string owningProcess) override {
            std::unique_lock<std::mutex> lock(mtx);
            MemoryChunk* allocated = (MemoryChunk*) freeList->pop(size);

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

        void free(AllocatedMemory* allocated) override {
            std::unique_lock<std::mutex> lock(mtx);
            MemoryChunk* chunk = (MemoryChunk*) allocated;
            MemoryChunk* previousChunk = (chunk)->prev;
            MemoryChunk* nextChunk = chunk->next;

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
        
        void printMemory(long long quantum_cycle) override {
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

class PagingMemoryInterface: public AbstractMemoryInterface {
    private:
        MemoryStats computeMemoryStats() override {
            std::unique_lock<std::mutex> lock(mtx);
            MemoryStats stats = {0, 0, {}};

            

            lock.unlock();
            return stats;
        }

    public:
        ~PagingMemoryInterface() {
            
        }

        PagingMemoryInterface() {}

        PagingMemoryInterface(uint64_t memorySize, std::string (*getCurrentTimestamp)()) {
            this->memorySize = memorySize;
            this->startAddress = 0;
            this->endAddress = memorySize;
            this->freeList = new FirstFitFreeList();
            this->getCurrentTimestamp = getCurrentTimestamp;
        }

        MemoryFrame* allocate(uint64_t size, std::string owningProcess) override {
            return nullptr;
        }

        void free(AllocatedMemory* allocated) override {
            std::unique_lock<std::mutex> lock(mtx);
            
            lock.unlock();
        }
        
        void printMemory(long long quantum_cycle) override {
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
