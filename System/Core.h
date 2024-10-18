#ifndef CORE
#define CORE

#include <thread>
#include <atomic>
#include "../DataTypes/Process.h"

class Core
{
private:
    int core_num;
    int internalClock;
    int timeQuanta;
    std::thread t;
    Process* current_process;
    TSQueue* ready_queue;
    std::atomic<bool> activeFlag;
    std::atomic<bool> coreOn;
    std::atomic<bool> preemptedFlag;
    std::string (*getCurrentTimestamp)();

    void removeFromCore() {
        current_process->assign(-1);
        current_process = nullptr;
        this->internalClock = 0;
        activeFlag.store(false);
    }

public:
    Core(int core_num, int timeQuanta, std::string (*getCurrentTimestamp)()) {
        this->core_num = core_num;
        this->internalClock = 0;
        this->timeQuanta = timeQuanta;
        current_process = nullptr;
        activeFlag.store(false);
        coreOn.store(false);
        preemptedFlag.store(false);
        this->getCurrentTimestamp = getCurrentTimestamp;
    }

    void assignReadyQueue(TSQueue* queue_ptr) {
        this->ready_queue = queue_ptr;
    }

    void start() {
        coreOn.store(true);
        t = std::thread(run, this);
    }

    void run() {
        bool programCompleted = false;
        while(coreOn.load()) {
            if(activeFlag.load()) {
                programCompleted = current_process->executeLine(getCurrentTimestamp());
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                internalClock++;

                if(internalClock % timeQuanta == 0) {
                    preemptedFlag.store(true);
                }

                if(programCompleted || preemptedFlag.load()) {
                    if(!programCompleted) {
                        ready_queue->push(current_process);
                    }

                    removeFromCore();
                    preemptedFlag.store(false);
                }
            }
        }
    }

    bool isActive() {
        return this->activeFlag.load();
    }

    void assignProcess(Process* p) {
        this->current_process = p;
        p->assign(this->core_num);
        this->internalClock = 0;
        this->activeFlag.store(true);
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
};
#endif