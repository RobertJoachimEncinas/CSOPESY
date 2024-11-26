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
#include "./BackingStore.h"
#include "./Core.h"

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


struct ProcessAgeComparator
{
    bool operator()(const Process* x, const Process* y) const
    {
        return x->id < y->id;
    }
};

class AbstractMemoryInterface {
    protected:
        uint64_t startAddress;
        uint64_t endAddress;
        uint64_t memorySize;
        uint64_t availableMemory;
        FreeList* freeList;
        std::set<Process*, ProcessAgeComparator> processesList; 
        std::mutex mtx;
        std::condition_variable cv;
        std::string (*getCurrentTimestamp)();
        BackingStore backingStore;
        std::vector<Core*>* cores;

        virtual MemoryStats computeMemoryStats() { return {0, 0, {}}; };

        virtual Process* getFirstWithFreeable() {
            Process* p = nullptr;

            for(const auto& process: processesList) {
                if(process->allocatedMemory.size() > 0) {
                    p = process;
                    break;
                }
            }

            return p;
        }

        virtual void nonLockingFree(AllocatedMemory* allocated) {}
    public:
        AbstractMemoryInterface() {}

        AbstractMemoryInterface(uint64_t memorySize, std::string (*getCurrentTimestamp)(), std::vector<Core*>* cores) {
            this->memorySize = memorySize;
            this->availableMemory = memorySize;
            this->startAddress = 0;
            this->endAddress = memorySize;
            this->freeList = nullptr;
            this->getCurrentTimestamp = getCurrentTimestamp;
            this->cores = cores;
        }

        virtual ~AbstractMemoryInterface() {};

        virtual void addToProcessList(Process* p) {
            std::unique_lock<std::mutex> lock(mtx);
            processesList.insert(p);
            lock.unlock();
        };

        virtual void removeFromProcessList(Process* p) {
            std::unique_lock<std::mutex> lock(mtx);
            processesList.erase(p);
            lock.unlock();
        };

        virtual void reserve(uint64_t size, std::string processName) {
            std::unique_lock<std::mutex> lock(mtx);
            while(size > availableMemory) {
                Process* p = getFirstWithFreeable();

                if(p == nullptr) {
                    break;
                }

                if(p->core != -1 && !p->completed) {
                    cores->at(p->core)->preempt();
                } else if(p->core != -1 && p->completed) {
                    cores->at(p->core)->finish();
                }

                backingStore.store(p);

                for(const auto& memory: p->allocatedMemory) {
                    nonLockingFree(memory);
                }

                p->allocatedMemory = {};

                processesList.erase(p);
            }
            lock.unlock();
        }

        virtual uint64_t fetchFromBackingStore(std::string process_name) {
            return backingStore.retrieve(process_name);
        }

        virtual std::vector<AllocatedMemory*> allocate(uint64_t size, std::string owningProcess) { return {}; };
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

        void nonLockingFree(AllocatedMemory* allocated) override {
            MemoryChunk* chunk = (MemoryChunk*) allocated;
            MemoryChunk* previousChunk = (chunk)->prev;
            MemoryChunk* nextChunk = chunk->next;

            chunk->owningProcess = "";
            chunk->isInUse = false;
            availableMemory += allocated->size;

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

        FlatMemoryInterface(uint64_t memorySize, std::string (*getCurrentTimestamp)(), std::vector<Core*>* cores) {
            this->memorySize = memorySize;
            this->availableMemory = memorySize;
            this->startAddress = 0;
            this->endAddress = memorySize - 1;
            this->memoryStart = new MemoryChunk(memorySize, 0, nullptr, nullptr, "", false);
            this->freeList = new FirstFitFreeList();
            this->freeList->push(memoryStart);
            this->getCurrentTimestamp = getCurrentTimestamp;
            this->cores = cores;
        }

        std::vector<AllocatedMemory *> allocate(uint64_t size, std::string owningProcess) override {
            std::unique_lock<std::mutex> lock(mtx);
            MemoryChunk* allocated = (MemoryChunk*) freeList->pop(size);

            if(allocated == nullptr) {
                return {};
            }

            allocated->owningProcess = owningProcess;

            if(allocated->startAddress == 0) {
                this->memoryStart = allocated;
            }

            availableMemory -= size;

            lock.unlock();
            return { allocated };
        }

        void free(AllocatedMemory* allocated) override {
            std::unique_lock<std::mutex> lock(mtx);
            nonLockingFree(allocated);
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
        uint64_t frameSize;
        std::vector<MemoryFrame*> memoryMap;

        MemoryStats computeMemoryStats() override {
            std::unique_lock<std::mutex> lock(mtx);
            std::set<std::string> unique_processes;
            MemoryStats stats = {0, 0, {}};
            ProcessMemory currentProcess = {0, 0, ""};

            for(const auto& frame: memoryMap) {
                if(frame->isInUse) {
                    if(currentProcess.process_name != frame->owningProcess) {
                        if(currentProcess.process_name != "") { 
                            stats.processMemoryRegions.push_back(currentProcess);
                        }

                        currentProcess.process_name = frame->owningProcess;
                        currentProcess.startAddress = frame->startAddress;
                        currentProcess.endAddress = frame->endAddress;
                    } else {
                        currentProcess.endAddress = frame->endAddress; //Update end address only as memoryFrames is in order
                    }

                    unique_processes.insert(frame->owningProcess); //This wont insert if the process is already in the set

                } else {
                    //Frame not in use with a non-default current process signals end of that process' memory region
                    if(currentProcess.process_name != "") { 
                        stats.processMemoryRegions.push_back(currentProcess);
                    }

                    //Reset the current process to default as the contiguous block for that process is done
                    currentProcess.process_name = "";
                    currentProcess.startAddress = 0;
                    currentProcess.endAddress = 0;

                    stats.totalFragmentation += frame->size;
                }
            }

            if(currentProcess.process_name != "") { 
                stats.processMemoryRegions.push_back(currentProcess);
            }

            stats.processes_in_memory = unique_processes.size();

            lock.unlock();
            return stats;
        }

        void createChunks() {
            uint64_t num_frames = memorySize / frameSize;
            uint64_t start_addr = 0;
            MemoryFrame* addr;

            for(uint64_t frameNum = 0; frameNum < num_frames; frameNum++) {
                addr = new MemoryFrame(frameSize, start_addr, frameNum, "");
                this->memoryMap.push_back(addr);
                this->freeList->push(addr);
                start_addr += frameSize;
            }
        }

        void nonLockingFree(AllocatedMemory* allocated) override {
            allocated->owningProcess = "";
            allocated->isInUse = false;
            freeList->push(allocated);
            availableMemory += allocated->size;
        }

    public:
        ~PagingMemoryInterface() {
            delete freeList;

            for(const auto& frame: memoryMap) {
                delete frame;
            }
        }

        PagingMemoryInterface() {}

        PagingMemoryInterface(uint64_t memorySize, uint64_t frameSize, std::string (*getCurrentTimestamp)(), std::vector<Core*>* cores) {
            this->memorySize = memorySize;
            this->availableMemory = memorySize;
            this->startAddress = 0;
            this->endAddress = memorySize - 1;
            this->frameSize = frameSize;
            this->freeList = new FirstFitPagingFreeList(frameSize);
            this->getCurrentTimestamp = getCurrentTimestamp;
            this->cores = cores;
            
            createChunks();
        }

        std::vector<AllocatedMemory*> allocate(uint64_t size, std::string owningProcess) override {
            std::unique_lock<std::mutex> lock(mtx);
            if(size > ((FirstFitPagingFreeList*) freeList)->getAvailableMemory()) {
                lock.unlock();
                return {};
            }
            
            std::vector<AllocatedMemory*> allocatedMem;
            uint64_t allocatedSize = 0;
            MemoryFrame* temp;

            while(allocatedSize < size) {
                temp = (MemoryFrame*) freeList->pop(frameSize);
                temp->owningProcess = owningProcess;
                temp->isInUse = true;
                allocatedMem.push_back(temp);
                allocatedSize += frameSize;
            }

            availableMemory -= size;
            
            lock.unlock();
            return allocatedMem;
        }

        void free(AllocatedMemory* allocated) override {
            std::unique_lock<std::mutex> lock(mtx);
            nonLockingFree(allocated);
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
