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

    public:
        Scheduler(std::vector<Core*>* cores) {
            this->cores = cores;
        }

        void start() {
            t = std::thread(run, this);
        }

        void run() {
            bool assigned = false;
            while(true) {
                Process* p = ready_queue.pop(); //Blocks until there's something to take
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
};
#endif