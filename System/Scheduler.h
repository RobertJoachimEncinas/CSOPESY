#ifndef SCHEDULER
#define SCHEDULER
#include "../DataTypes/TSQueue.h"
#include "./Core.h"
#include <vector>

class Scheduler {
    private:
        TSQueue ready_queue;
        std::vector<Core*>* cores;
        std::thread t;
        std::atomic<bool> active;

    public:
        Scheduler(std::vector<Core*>* cores) {
            this->cores = cores;
            this->active.store(false);
        }

        void start() {
            this->active.store(true);
            t = std::thread(run, this);
        }

        void run() {
            bool assigned = false;
            Process* p;

            while(active.load()) {
                p = nullptr;

                while(p == nullptr) {
                    p = ready_queue.pop(); //Blocks until there's something to take
                
                    if(!active.load()) {
                        return;
                    }
                }

                assigned = false; //Set default unassigned

                //Ping all cores to see who can take
                while(!assigned) {
                    for(int i = 0; i < cores->size(); i++) {
                        if(!((*cores->at(i)).isActive())) {
                            (*cores->at(i)).assignProcess(p);
                            assigned = true;
                            break;
                        }       
                    }
                }
            }
        }

        void enqueue(Process* p) {
            ready_queue.push(p);
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