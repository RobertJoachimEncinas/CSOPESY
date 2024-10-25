#pragma once
#include "../DataTypes/TSQueue.h"
#include "./Core.h"
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

    public:
        Scheduler(std::vector<Core*>* cores, std::atomic<long long>* currentSystemClock) {
            this->schedulerClock = 0;
            this->currentSystemClock = currentSystemClock;
            this->cores = cores;
            this->active.store(false);
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
                    if(readyQueue.isEmpty()) {
                        break; //Ready queue for this time step has all been dispatch already, process anything from screen -s that was not synced in the next timestep
                    } 

                    if(!((*cores->at(i)).isActive())) { //Check if the core is free
                        (*cores->at(i)).assignProcess(readyQueue.pop());
                    }     
                }

                this->schedulerClock++;
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
            return this->schedulerClock;
        }
};