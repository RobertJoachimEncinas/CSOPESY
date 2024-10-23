#ifndef CORE
#define CORE

#include <thread>
#include <atomic>
#include "../DataTypes/Process.h"

class Core
{
private:
    int coreId;                 // core id number
    long long coreClock;        // core's internal clock
    long long quantumCycles;    // number of cycles before rr preempts process
    long long clockMod;         // modulo for when clock exceeds max value of long long
    long long coreQuantumCountdown; // countdown for when to preempt process
    std::thread t;
    Process* currentProcess;
    TSQueue* readyQueue;
    std::atomic<int>* currentSystemClock;
    std::atomic<bool> activeFlag;
    std::atomic<bool> coreOn;
    std::atomic<bool> preemptedFlag;
    std::string (*getCurrentTimestamp)();

    void removeFromCore() {
        currentProcess->setCore(-1);
        currentProcess = nullptr;
        activeFlag.store(false);
        this->coreQuantumCountdown = quantumCycles;
    }

public:
    Core(int coreId, long long quantumCycles, int clockMod, std::atomic<int>* currentSystemClock, std::string (*getCurrentTimestamp)()) {
        this->coreId = coreId;
        this->coreClock = 0;
        this->quantumCycles = quantumCycles;
        this->coreQuantumCountdown = quantumCycles;
        currentProcess = nullptr;
        activeFlag.store(false);
        coreOn.store(false);
        preemptedFlag.store(false);

        this->currentSystemClock = currentSystemClock;
        this->getCurrentTimestamp = getCurrentTimestamp;
        this->clockMod = clockMod;
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
                coreQuantumCountdown = (coreQuantumCountdown - 1) % clockMod;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                if(coreQuantumCountdown == 0) {
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
            coreClock = (coreClock + 1) % clockMod;
        }
    }

    bool isActive() {
        return this->activeFlag.load();
    }

    void assignProcess(Process* p) {
        this->currentProcess = p;
        p->setCore(this->coreId);
        this->coreClock = 0;
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

    int getTime() {
        return this->coreClock;
    }
};
#endif