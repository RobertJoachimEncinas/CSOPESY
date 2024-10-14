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
        std::atomic<bool> coreOn;
        std::string (*getCurrentTimestamp)();

    public:
        Core(int core_num, std::string (*getCurrentTimestamp)()) {
            this->core_num = core_num;
            current_process = nullptr;
            activeFlag.store(false);
            coreOn.store(false);
            this->getCurrentTimestamp = getCurrentTimestamp;
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
                
                    if(programCompleted) {
                        current_process->assign(-1);
                        current_process = nullptr;
                        activeFlag.store(false);
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

        void turnOff() {
            coreOn.store(false);
            join();
        }

        void join() {
            if(this->t.joinable()) {
                this->t.join();
            }
        }
};
#endif