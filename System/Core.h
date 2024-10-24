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
    long long coreQuantumCountdown; // countdown for when to preempt process
    std::thread t;
    Process* currentProcess;
    TSQueue* readyQueue;
    std::atomic<long long>* currentSystemClock;
    std::atomic<bool> activeFlag;
    std::atomic<bool> coreOn;
    std::atomic<bool> preemptedFlag;
    std::string (*getCurrentTimestamp)();
    SchedAlgo algorithm;

    void removeFromCore() {
        currentProcess->setCore(-1);
        currentProcess = nullptr;
        activeFlag.store(false);
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
        activeFlag.store(false);
        coreOn.store(false);
        preemptedFlag.store(false);

        this->currentSystemClock = currentSystemClock;
        this->getCurrentTimestamp = getCurrentTimestamp;
    }

    void assignReadyQueue(TSQueue* queue_ptr) {
        this->readyQueue = queue_ptr;
    }

    void start() {
        coreOn.store(true);
        t = std::thread(run, this);
    }

    void run() {
        bool processCompleted = false;
        while(coreOn.load()) {
            while(currentSystemClock->load() == this->coreClock && coreOn.load()) {} //Halt if at latest time step

            if(activeFlag.load()) {
                processCompleted = currentProcess->executeLine(getCurrentTimestamp(), this->coreId);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                coreQuantumCountdown = (coreQuantumCountdown - 1) % LLONG_MAX;

                if(coreQuantumCountdown == 0 && algorithm == RR) {
                    preemptedFlag.store(true);
                }

                if(processCompleted || preemptedFlag.load()) {
                    if(!processCompleted) {
                        readyQueue->push(currentProcess);
                    }

                    removeFromCore();
                    preemptedFlag.store(false);
                }
            }
            coreClock = (coreClock + 1) % LLONG_MAX;
        }
    }

    bool isActive() {
        return this->activeFlag.load();
    }

    void assignProcess(Process* p) {
        this->currentProcess = p;
        p->setCore(this->coreId);
        this->activeFlag.store(true);
    }

    bool isCoreOn() {
        return coreOn.load();
    }

    void turnOff() {
        coreOn.store(false);
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