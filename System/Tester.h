#pragma once
#include "../DataTypes/Process.h"
#include "Core.h"
#include "Scheduler.h"
#include <thread>
#include <atomic>
#include <vector>

class Tester 
{
    private: 
        std::thread t;
        std::atomic<bool> active;
        std::atomic<bool> locked;
        long long testerClock;
        std::atomic<long long>* currentSystemClock;
        long long* processFreq;
        long long processFreqCounter;
        std::vector<std::shared_ptr<Process>>* processes;
        long long processIdCounter;
        long long* processMinIns;
        long long* processMaxIns;
        std::string (*getCurrentTimestamp)();
        Scheduler* scheduler;
        std::vector<Core*>* cores;

        bool isCoresDone() {
            for(int i = 0; i < cores->size(); i++) {
                if(cores->at(i)->getTime() != currentSystemClock->load() && cores->at(i)->isOn()) {
                    return false;
                }
            }
            return true;
        }

    public:    
        Tester(std::atomic<long long>* currentSystemClock, long long* processFreq, std::vector<std::shared_ptr<Process>>* processes, long long *processMinIns, long long *processMaxIns, std::string (*getCurrentTimestamp)(), Scheduler* scheduler, std::vector<Core*>* cores) {
            this->currentSystemClock = currentSystemClock;
            this->testerClock = 0;
            this->active.store(false);
            this->locked.store(false);
            this->processFreq = processFreq;
            this->processFreqCounter = 1;
            this->processes = processes;
            this->processIdCounter = 0;
            this->processMinIns = processMinIns;
            this->processMaxIns = processMaxIns;
            this->getCurrentTimestamp = getCurrentTimestamp;
            this->scheduler = scheduler;
            this->cores = cores;
        }

        void start() {
            this->active.store(true);
            testerClock = currentSystemClock->load();
            this->processFreqCounter = 1;
            t = std::thread(run, this);
        }

        void run() {
            //TODO: Implement batch loading of processes
            while(active.load()) {
                while ((!isCoresDone() || currentSystemClock->load() == testerClock) && active.load()) {}
                if (processFreqCounter == *processFreq) { 
                    processFreqCounter = 0;
                    
                    while(locked.load()) {}
                    locked.store(true); // Lock during write

                    std::cout << "TESTER: creating new process...\n";
                    // std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Sleep for 1 second
                    // Check if the process already exists
                    std::string process_name = "Process" + std::to_string(processIdCounter);
                    for (const auto& process : *processes) {
                        if (process->name == process_name) {
                            processIdCounter++;
                            process_name = "Process" + std::to_string(processIdCounter);
                        }
                    }
                    // Set random number of instructions
                    long long instructions = *processMinIns + (rand() % (*processMaxIns - *processMinIns + 1));
                    // If no duplicates, create and add the new process
                    std::shared_ptr<Process> newProcess = std::make_shared<Process>(process_name, instructions, getCurrentTimestamp());
                    processes->push_back(newProcess);

                    //Add to scheduler
                    scheduler->enqueue(newProcess.get());

                    locked.store(false); //Unlock after write
                }

                testerClock = (testerClock + 1) % LLONG_MAX;
                processFreqCounter++;      
            }
        }

        long long getTime() {
            return testerClock;
        }

        bool isActive() {
            return active.load();
        }

        void turnOff() {
            active.store(false);
            join();
        }

        void join() {
            if (this->t.joinable()) {
                this->t.join();
            }
        }

        std::atomic<bool> isLocked() {
            return locked.load();
        }

        void lock() {
            this->locked.store(true);
        }

        void unlock() {
            this->locked.store(false);
        }
};