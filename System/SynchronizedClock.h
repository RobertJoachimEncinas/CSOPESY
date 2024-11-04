#pragma once

#include "../System/Core.h"
#include "../System/Scheduler.h"
#include "../System/Tester.h"
#include <atomic>
#include <thread>
#include <vector>

class SynchronizedClock {
    private:
        std::atomic<long long> currentSystemClock;
        std::atomic<bool> active;
        std::atomic<bool> testerShouldStart;
        std::thread t;
        std::vector<Core*>* cores;
        Tester* tester;
        Scheduler* scheduler;

        bool coresSynced() {
            for(int i = 0; i < cores->size(); i++) {
                if(cores->at(i)->getTime() != currentSystemClock && cores->at(i)->isOn()) {
                    return false;
                }
            }

            return true;
        }

        bool testerSynced() {
            if (tester->getTime() != currentSystemClock && tester->isActive()) {
                return false;
            }

            return true;
        }

        bool schedulerSynced() {
            if (scheduler->getTime() != currentSystemClock && scheduler->isActive()) {
                return false;
            }

            return true;
        }

    public:
        SynchronizedClock(std::vector<Core*>* cores, Tester* tester, Scheduler* scheduler) {
            active.store(false);
            testerShouldStart.store(false);
            currentSystemClock.store(0);
            this->cores = cores;
            this->tester = tester;
            this->scheduler = scheduler;
        }

        void start() {
            active.store(true);
            t = std::thread(run, this);
        }

        void run(){
            while(active.load()) {
                while(!schedulerSynced() && active.load()) {} //Halt to wait for scheduler to dispatch

                if(!active.load() || schedulerSynced()) { //Unlock all cores after scheduler completes or system is being shutoff
                    for(int i = 0; i < cores->size(); i++) {
                        cores->at(i)->unlock();
                    }
                }

                while(!coresSynced() && active.load()) {} //Halt to wait for core execution
                
                if((!active.load() || schedulerSynced()) && tester->isActive()) { //Unlock tester after cores execute or system is being shutoff and the tester is active
                    tester->setCanProceed(); //Allow tester to proceed
                }

                while((!testerSynced() && tester->isActive()) && active.load()) {} //Halt to wait for tester execution if it is active
                
                if(testerShouldStart.load() && active.load()) {
                    tester->start();

                    while(!tester->isActive() && testerShouldStart.load() && active.load()) {} //Halt for tester thread to turn on

                    testerShouldStart.store(false); //Reset to false as tester has already started  
                } 

                incrementClock();
            }
        }

        std::atomic<long long>* getSyncClock() {
            return std::addressof(currentSystemClock);
        }

        void incrementClock() {
            currentSystemClock.store((currentSystemClock.load() + 1) % LLONG_MAX);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
        
        void startTester() {
            testerShouldStart.store(true);
        }
};