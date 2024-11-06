#pragma once

#include <thread>
#include <atomic>
#include "../DataTypes/Process.h"
#include "../DataTypes/SchedAlgo.h"
#include "MemoryInterface.h"

class Core
{
private:
    int coreId;                 // core id number
    long long coreClock;        // core's internal clock
    long long quantumCycles;    // number of cycles before rr preempts process
    long long coreQuantumCountdown; // processFreqCounter for when to preempt process
    long long delayPerExec;     // delay per execution
    long long delayCounter;     // delay counter
    std::thread t;
    Process* currentProcess;
    TSQueue* readyQueue;
    std::atomic<long long>* currentSystemClock;
    std::atomic<bool> isCoreActive;
    std::atomic<bool> isCoreOn;
    std::atomic<bool> isPreempted;
    std::atomic<bool> canProceed;
    std::string (*getCurrentTimestamp)();
    SchedAlgo algorithm;
    MemoryInterface* memory;

    void removeFromCore() {
        currentProcess->setCore(-1);
        currentProcess = nullptr;
        isCoreActive.store(false);
        this->coreQuantumCountdown = quantumCycles;
    }

public:
    Core(int coreId, long long quantumCycles, std::atomic<long long>* currentSystemClock, std::string (*getCurrentTimestamp)(), SchedAlgo algorithm, long long delayPerExec, MemoryInterface* memory) {
        this->coreId = coreId;
        this->coreClock = 0;
        this->quantumCycles = quantumCycles;
        this->coreQuantumCountdown = quantumCycles;
        this->algorithm = algorithm;
        currentProcess = nullptr;
        isCoreActive.store(false);
        isCoreOn.store(false);
        isPreempted.store(false);
        canProceed.store(false);

        this->currentSystemClock = currentSystemClock;
        this->getCurrentTimestamp = getCurrentTimestamp;
        this->delayPerExec = delayPerExec;
        this->delayCounter = 0;
        this->memory = memory;
    }

    void assignReadyQueue(TSQueue* queue_ptr) {
        this->readyQueue = queue_ptr;
    }

    void start() {
        isCoreOn.store(true);
        t = std::thread(run, this);
    }

    void run() {
        bool processCompleted = false;
        while(isCoreOn.load()) {
            while(currentSystemClock->load() == this->coreClock && isCoreOn.load()) {} //Halt if at latest time step
            while(!canProceed.load() && isCoreOn.load()) {} // Wait for scheduler

            if(isCoreActive.load()){
                if(delayCounter == delayPerExec) {
                    processCompleted = currentProcess->executeLine(getCurrentTimestamp(), this->coreId);
                    coreQuantumCountdown--;

                    if(coreQuantumCountdown == 0 && algorithm == RR) {
                        isPreempted.store(true);
                    }

                    if(processCompleted || isPreempted.load()) {
                        memory->free(currentProcess->memoryFrames);

                        if(!processCompleted) {
                            readyQueue->push(currentProcess);
                        }

                        removeFromCore();
                        isPreempted.store(false);
                    }

                    delayCounter = -1;
                }
                delayCounter++;
            }
            coreClock = (coreClock + 1) % LLONG_MAX;
            lock(); // Lock self for next iteration
        }
    }

    bool isActive() {
        return this->isCoreActive.load();
    }

    void assignProcess(Process* p) {
        this->currentProcess = p;
        p->setCore(this->coreId);
        this->isCoreActive.store(true);
    }

    bool isOn() {
        return isCoreOn.load();
    }

    void turnOff() {
        isCoreOn.store(false);
        join();
    }

    void join() {
        if (this->t.joinable()) {
            this->t.join();
        }
    }

    void unlock() {
        this->canProceed.store(true);
    }

    void lock() {
        this->canProceed.store(false);
    }

    long long getTime() {
        return this->coreClock;
    }
};