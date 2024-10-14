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
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                    if(programCompleted) {
                        current_process->assign(-1);
                        current_process = nullptr;
                        activeFlag.store(false);
                        break;
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
            this->activeFlag.store(true);
        }
};
#endif