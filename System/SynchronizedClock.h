#ifndef SYNCCLOCK
#define SYNCCLOCK

#include "../System/Core.h"
#include "../System/Scheduler.h"
#include <atomic>
#include <thread>
#include <vector>

class SynchronizedClock {
    private:
        std::atomic<long long> currentSystemClock;
        std::atomic<bool> active;
        std::thread t;
        std::vector<Core*>* cores;

        bool systemSynced() {
            for(int i = 0; i < cores->size(); i++) {
                if(cores->at(i)->getTime() != currentSystemClock && cores->at(i)->isCoreOn()) {
                    return false;
                }
            }

            return true;
        }

    public:
        SynchronizedClock(std::vector<Core*>* cores) {
            active.store(false);
            currentSystemClock.store(0);
            this->cores = cores;
        }

        void start() {
            active.store(true);
            t = std::thread(run, this);
        }

        void run(){
            while(active.load()) {
                while(!systemSynced() && active.load()) {} //Halt to wait for every thread to catch up
                incrementClock();
            }
        }

        std::atomic<long long>* getSyncClock() {
            return std::addressof(currentSystemClock);
        }

        void incrementClock() {
            currentSystemClock.store((currentSystemClock.load() + 1) % LLONG_MAX);
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