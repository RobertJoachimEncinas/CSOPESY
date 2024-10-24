#ifndef SCHEDULER
#define SCHEDULER
#include "../DataTypes/TSQueue.h"
#include "./Core.h"
#include <vector>
#include <atomic>

class Scheduler {
    private:
        TSQueue readyQueue;
        std::vector<Core*>* cores;
        std::thread t;
        std::atomic<bool> active;
        std::atomic<int>* currentSystemClock;
    public:
        Scheduler(std::vector<Core*>* cores) {
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
            bool processIsAssigned;
            Process* process;

            while(active.load()) {
                do {
                    process = readyQueue.pop(); //Blocks until there's something to take

                    if (!active.load()) {
                        return;
                    }
                } while(process == nullptr);

                processIsAssigned = false; //Set default unassigned

                //Ping all cores to see who can take
                while(!processIsAssigned) {
                    for(int i = 0; i < cores->size(); i++) {
                        if(!((*cores->at(i)).isActive())) {
                            (*cores->at(i)).assignProcess(process);
                            processIsAssigned = true;
                            break;
                        }       
                    }
                }
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
};
#endif