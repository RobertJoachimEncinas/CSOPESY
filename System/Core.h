#pragma once

#include <thread>
#include <atomic>
#include "../DataTypes/Process.h"
#include "../DataTypes/SchedAlgo.h"

class Core
{
private:
    int coreId;                 // core id number
    long long coreClock;        // core's internal clock
    long long quantumCycles;    // number of cycles before rr preempts process
    long long coreQuantumCountdown; // processFreqCounter for when to preempt process
    std::thread t;
    Process* currentProcess;
    TSQueue* readyQueue;
    std::atomic<long long>* currentSystemClock;
    std::atomic<bool> isCoreActive;
    std::atomic<bool> isCoreOn;
    std::atomic<bool> isPreempted;
    std::string (*getCurrentTimestamp)();
    SchedAlgo algorithm;

    void removeFromCore() {
        currentProcess->setCore(-1);
        currentProcess = nullptr;
        isCoreActive.store(false);
        this->coreQuantumCountdown = quantumCycles;
    }

public:
    Core(int coreId, long long quantumCycles, std::atomic<long long>* currentSystemClock, std::string (*getCurrentTimestamp)(), SchedAlgo algorithm) {
        this->coreId = coreId;
        this->coreClock = 0;
        this->quantumCycles = quantumCycles;
        this->coreQuantumCountdown = quantumCycles;
        this->algorithm = algorithm;
        currentProcess = nullptr;
        isCoreActive.store(false);
        isCoreOn.store(false);
        isPreempted.store(false);

        this->currentSystemClock = currentSystemClock;
        this->getCurrentTimestamp = getCurrentTimestamp;
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

            if(isCoreActive.load()) {
                processCompleted = currentProcess->executeLine(getCurrentTimestamp(), this->coreId);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                coreQuantumCountdown = (coreQuantumCountdown - 1) % LLONG_MAX;

                if(coreQuantumCountdown == 0 && algorithm == RR) {
                    isPreempted.store(true);
                }

                if(processCompleted || isPreempted.load()) {
                    if(!processCompleted) {
                        readyQueue->push(currentProcess);
                    }

                    removeFromCore();
                    isPreempted.store(false);
                }
            }
            coreClock = (coreClock + 1) % LLONG_MAX;
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

    long long getTime() {
        return this->coreClock;
    }
};