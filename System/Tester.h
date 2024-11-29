#pragma once
#include "../DataTypes/Process.h"
#include "Core.h"
#include "Scheduler.h"
#include <thread>
#include <atomic>
#include <vector>
#include <cmath>

class Tester 
{
    private: 
        std::thread t;
        std::atomic<bool> active;
        std::atomic<bool> locked;
        std::atomic<bool> canProceed;
        long long testerClock;
        std::atomic<long long>* currentSystemClock;
        long long* processFreq;
        long long processFreqCounter;
        std::map<std::string, std::shared_ptr<Process>>* processes;
        long long processIdCounter;
        long long* processMinIns;
        long long* processMaxIns;
        long long* processMaxMem;
        long long* processMinMem;
        long long* memoryPerProcess;
        std::string (*getCurrentTimestamp)();
        Scheduler* scheduler;
        AbstractMemoryInterface* memory;

    public:    
        Tester(std::atomic<long long>* currentSystemClock, long long* processFreq, std::map<std::string, std::shared_ptr<Process>>* processes, long long *processMinIns, long long *processMaxIns, std::string (*getCurrentTimestamp)(), Scheduler* scheduler, long long* processMinMem, long long* processMaxMem) {
            this->currentSystemClock = currentSystemClock;
            this->testerClock = 0;
            this->active.store(false);
            this->locked.store(false);
            this->canProceed.store(false);
            this->processFreq = processFreq;
            this->processes = processes;
            this->processIdCounter = 0;
            this->processMinIns = processMinIns;
            this->processMaxIns = processMaxIns;
            this->processMinMem = processMinMem;
            this->processMaxMem = processMaxMem;
            this->getCurrentTimestamp = getCurrentTimestamp;
            this->scheduler = scheduler;
        }

        void start() {
            this->active.store(true);
            this->canProceed.store(false);
            testerClock = currentSystemClock->load();
            this->processFreqCounter = *processFreq;
            t = std::thread(run, this);
        }

        void run() {
            while(active.load()) {
                while ((!canProceed.load() || currentSystemClock->load() == testerClock) && active.load()) {}

                if (processFreqCounter == *processFreq) { 
                    processFreqCounter = 0;
                    
                    while(locked.load()) {}
                    locked.store(true); // Lock during write

                    // Check if the process already exists
                    std::string process_name = "Process" + std::to_string(processIdCounter);
                    for (const auto& process : *processes) {
                        if (process.first == process_name) {
                            processIdCounter++;
                            process_name = "Process" + std::to_string(processIdCounter);
                        }
                    }
                    // Set random number of instructions
                    long long instructions = *processMinIns + (rand() % (*processMaxIns - *processMinIns + 1));
                    // Set random memory per process
                    int minPower = 0, maxPower = 0;
                    while ((1LL << minPower) < *processMinMem)
                        minPower++;
                    while ((1LL << maxPower) < *processMaxMem)
                        maxPower++;
                    int randomPower = minPower + rand() % (maxPower - minPower + 1);
                    long long memoryPerProcess = 1LL << randomPower;
                    // Create new Process
                    std::shared_ptr<Process> newProcess = std::make_shared<Process>(process_name, instructions, getCurrentTimestamp(), memoryPerProcess);
                    processes->insert(std::make_pair(process_name, newProcess));
                
                    //Add to scheduler
                    scheduler->enqueue(newProcess.get());

                    locked.store(false); //Unlock after write
                }

                testerClock = (testerClock + 1) % LLONG_MAX;
                processFreqCounter++;      
                clearCanProceed(); //Reset can proceed for next cycle
            }
        }

        void setMemoryInterface(AbstractMemoryInterface* memory) {
            this->memory = memory;
        }

        long long getTime() {
            return testerClock;
        }

        bool isActive() {
            return active.load();
        }

        void turnOff() {
            active.store(false);
            join();
        }

        void join() {
            if (this->t.joinable()) {
                this->t.join();
            }
        }

        std::atomic<bool> isLocked() {
            return locked.load();
        }

        void lock() {
            this->locked.store(true);
        }

        void unlock() {
            this->locked.store(false);
        }

        void setCanProceed() {
            this->canProceed.store(true);
        }

        void clearCanProceed() {
            this->canProceed.store(false);
        }
};