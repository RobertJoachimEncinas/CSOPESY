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
            while(true) {
                Process* p = ready_queue.pop(); //Blocks until there's something to take

                //Ping all cores to see who can take
                for(int i = 0; i < cores->size(); i++) {
                    if(!((*cores->at(i)).isActive())) {
                        std::cout << "Assigning to core: " << i << "\n";
                        (*cores->at(i)).assignProcess(p);
                        break;
                    }            
                }
            }
        }

        void enqueue(Process* p) {
            std::cout << "Enqueueing: " << p << "\n";
            ready_queue.push(p);
        }
};
#endif