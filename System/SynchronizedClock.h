#ifndef SYNCCLOCK
#define SYNCCLOCK

#include "../System/Core.h"
#include "../System/Scheduler.h"
#include <atomic>
#include <thread>
#include <vector>

class SynchronizedClock {
    private:
        std::atomic<int> currentClockCycle;
        std::atomic<bool> active;
        std::thread t;
        int clockMod;
        std::vector<Core*>* cores;

        bool systemSynced() {
            int time = cores->at(0)->getTime();

            for(int i = 1; i < 4; i++) {
                if(cores->at(i)->getTime() != time && cores->at(i)->isCoreOn()) {
                    return false;
                }
            }

            return true;
        }

    public:
        SynchronizedClock(std::vector<Core*>* cores, int mod) {
            active.store(false);
            currentClockCycle.store(0);
            this->cores = cores;
            this->clockMod = mod;
        }

        void start() {
            active.store(true);
            t = std::thread(run, this);
        }

        void run(){
            while(active.load()) {
                while(!systemSynced()) {} //Halt to wait for every thread to catch up
                currentClockCycle.store((currentClockCycle.load() + 1) % clockMod);
            }
        }

        std::atomic<int>* getSyncClock() {
            return std::addressof(currentClockCycle);
        }

        void incrementClock() {
            currentClockCycle.store((currentClockCycle.load() + 1) % clockMod);
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
};

#endif