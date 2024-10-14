#ifndef CORE
#define CORE

#include <thread>
#include <atomic>
#include "../DataTypes/Process.h"

class Core {
    private:
        int core_num;
        std::thread t;
        Process* current_process;
        std::atomic<bool> activeFlag;

    public:
        Core(int core_num) {
            this->core_num = core_num;
            current_process = nullptr;
            activeFlag.store(false);
        }

        void start() {
            t = std::thread(run, this);
        }

        void run() {
            bool programCompleted = false;
            while(true) {
                if(activeFlag.load()) {
                    programCompleted = current_process->executeLine();
                
                    if(programCompleted) {
                        std::cout << "completed\n";
                        current_process = nullptr;
                    }
                }
            }
        }

        bool isActive() {
            return this->activeFlag.load();
        }

        void assignProcess(Process* p) {
            this->current_process = p;
            this->activeFlag.store(true);
            std::cout << "Process with pId: " << p << "assigned\n";
        }
};
#endif