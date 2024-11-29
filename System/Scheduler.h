#pragma once
#include "../DataTypes/TSQueue.h"
#include "./Core.h"
#include "MemoryInterface.h"
#include <vector>
#include <atomic>

class Scheduler {
    private:
        long long schedulerClock;
        TSQueue readyQueue;
        std::vector<Core*>* cores;
        std::thread t;
        std::atomic<bool> active;
        std::atomic<long long>* currentSystemClock;
        std::mutex mtx;
        AbstractMemoryInterface* memory;
        bool isFCFS = false;

    public:
        Scheduler(std::vector<Core*>* cores, std::atomic<long long>* currentSystemClock) {
            this->schedulerClock = 0;
            this->currentSystemClock = currentSystemClock;
            this->cores = cores;
            this->active.store(false);
        }

        void setMemoryInterface(AbstractMemoryInterface* memory) {
            this->memory = memory;
        }
        
        void assignReadyQueueToCores() {
            for(int i = 0; i < cores->size(); i++) {
                cores->at(i)->assignReadyQueue(std::addressof(readyQueue));
            }
        }

        void start() {
            this->active.store(true);
            t = std::thread(run, this);
        }

        void run() {
            Process* process;

            while(active.load()) {
                while(currentSystemClock->load() == this->schedulerClock && active.load()) {} // Block if not synced

                for(int i = 0; i < cores->size(); i++) {
                    if(cores->at(i)->getProcessCompleted()) {
                        Process* p = cores->at(i)->finish();
                        
                        for(const auto& mem: p->allocatedMemory) {
                            memory->free(mem);
                        }

                        p->allocatedMemory = {};
                        memory->removeFromProcessList(process);
                    } else if(cores->at(i)->getShouldPreempt()) {
                        Process* p = cores->at(i)->preempt();
                        memory->addToProcessList(p); // Add back as it is freeable now
                    }
                }

                for(int i = 0; i < cores->size(); i++) {
                    if(readyQueue.isEmpty()) {
                        break; //Ready queue for this time step has all been dispatch already, process anything from screen -s that was not synced in the next timestep
                    } 

                    if(!((*cores->at(i)).isActive())) { //Check if the core is free
                        process = readyQueue.peek();

                        if(process->allocatedMemory.size() == 0) {
                            uint64_t memoryRequirement = memory->fetchFromBackingStore(process->name);

                            if(memoryRequirement == 0) {
                                memoryRequirement = process->memoryRequired;
                            }

                            memory->reserve(memoryRequirement, process->name);
                            process->allocatedMemory = memory->allocate(memoryRequirement, process->name);
                        }

                        if(process->allocatedMemory.size() == 0) {
                            if(!isFCFS) {
                                readyQueue.pop();
                                enqueue(process);
                            }
                        } else {
                            (*cores->at(i)).assignProcess(process);
                            memory->removeFromProcessList(process);
                            readyQueue.pop();
                        }
                    }     
                }

                std::unique_lock<std::mutex> lock(mtx);
                this->schedulerClock++;
                lock.unlock();
            }
        }

        void enqueue(Process* process) {
            readyQueue.push(process);
        }

        void turnOff() {
            active.store(false);
            join();
        }
        
        void join() {
            if(this->t.joinable()) {
                this->t.join();
            }
        }

        bool isActive() {
            return this->active.load();
        }

        long long getTime() {
            std::lock_guard<std::mutex> lock(mtx);
            return this->schedulerClock;
        }

        void setIsFCFS(bool isFCFS) {
            this->isFCFS = isFCFS;
        }
};