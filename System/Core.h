#pragma once

#include <thread>
#include <atomic>
#include "../DataTypes/Process.h"
#include "../DataTypes/SchedAlgo.h"

struct TickData {
    long long total;
    long long active;
    long long idle;
};

class Core
{
private:
    int coreId;                 // core id number
    long long coreClock;        // core's internal clock
    long long quantumCycles;    // number of cycles before rr preempts process
    long long coreQuantumCountdown; // processFreqCounter for when to preempt process
    long long delayPerExec;     // delay per execution
    long long delayCounter;     // delay counter
    long long activeTicks;
    std::thread t;
    Process* currentProcess;
    TSQueue* readyQueue;
    std::atomic<long long>* currentSystemClock;
    std::atomic<bool> isCoreActive;
    std::atomic<bool> isCoreOn;
    std::atomic<bool> shouldPreempt;
    std::atomic<bool> canProceed;
    std::atomic<bool> processCompleted;
    std::string (*getCurrentTimestamp)();
    std::mutex mtx;
    SchedAlgo algorithm;

    Process* removeFromCore() {
        Process* finished = currentProcess;
        currentProcess->setCore(-1);
        currentProcess = nullptr;
        isCoreActive.store(false);
        this->coreQuantumCountdown = quantumCycles;
        return finished;
    }

public:
    Core(int coreId, long long quantumCycles, std::atomic<long long>* currentSystemClock, std::string (*getCurrentTimestamp)(), SchedAlgo algorithm, long long delayPerExec) {
        this->coreId = coreId;
        this->coreClock = 0;
        this->quantumCycles = quantumCycles;
        this->coreQuantumCountdown = quantumCycles;
        this->algorithm = algorithm;
        this->activeTicks = 0;
        currentProcess = nullptr;
        isCoreActive.store(false);
        isCoreOn.store(false);
        shouldPreempt.store(false);
        canProceed.store(false);
        processCompleted.store(false);

        this->currentSystemClock = currentSystemClock;
        this->getCurrentTimestamp = getCurrentTimestamp;
        this->delayPerExec = delayPerExec;
        this->delayCounter = 0;
    }

    void assignReadyQueue(TSQueue* queue_ptr) {
        this->readyQueue = queue_ptr;
    }

    void start() {
        isCoreOn.store(true);
        t = std::thread(run, this);
    }

    void run() {
        while(isCoreOn.load()) {
            while(currentSystemClock->load() == this->coreClock && isCoreOn.load()) {} //Halt if at latest time step
            while(!canProceed.load() && isCoreOn.load()) {} // Wait for scheduler
            while((processCompleted.load() || shouldPreempt.load()) && isCoreOn.load()) {}

            if(isCoreActive.load()){
                if(delayCounter == delayPerExec) {
                    processCompleted.store(currentProcess->executeLine(getCurrentTimestamp(), this->coreId));

                    if(!processCompleted.load()) {
                        coreQuantumCountdown--;

                        if(coreQuantumCountdown == 0 && algorithm == RR) {
                            shouldPreempt.store(true);
                        }

                    }

                    delayCounter = -1;

                }
                activeTicks++;
                delayCounter++;
            }
            std::unique_lock<std::mutex> l(mtx);
            coreClock = (coreClock + 1) % LLONG_MAX;
            l.unlock();
            lock(); // Lock self for next iteration
        }
    }

    TickData getTickData() {
        std::unique_lock<std::mutex> lock(mtx);
        TickData data = { coreClock, activeTicks, coreClock - activeTicks };
        lock.unlock();
        return data;
    }

    void preempt() {
        std::unique_lock<std::mutex> lock(mtx);
        readyQueue->push(currentProcess);
        removeFromCore();
        coreQuantumCountdown = quantumCycles;
        processCompleted.store(false);
        shouldPreempt.store(false);
        lock.unlock();
    }

    Process* finish() {
        std::unique_lock<std::mutex> lock(mtx);
        Process* p = removeFromCore();
        coreQuantumCountdown = quantumCycles;
        processCompleted.store(false);
        shouldPreempt.store(false);
        lock.unlock();
        return p;
    }

    bool getShouldPreempt() {
        return shouldPreempt.load();
    }

    bool getProcessCompleted() {
        return processCompleted.load();
    }

    void setShouldPreempt() {
        shouldPreempt.store(true);
    }

    bool isActive() {
        return this->isCoreActive.load();
    }

    void assignProcess(Process* p) {
        std::unique_lock<std::mutex> lock(mtx);
        this->currentProcess = p;
        p->setCore(this->coreId);
        this->isCoreActive.store(true);
        processCompleted.store(false);
        shouldPreempt.store(false);
        lock.unlock();
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
        std::lock_guard<std::mutex> l(mtx);
        return this->coreClock;
    }
};