#ifndef SYNCCLOCK
#define SYNCCLOCK

#include<atomic>

class SynchronizedClock {
    private:
        std::atomic<int> currentClockCycle;
        int clockMod;

    public:
        SynchronizedClock(int mod) {
            currentClockCycle.store(0);
            this->clockMod = mod;
        }

        int getCurrentCycle() {
            return currentClockCycle.load();
        }

        void incrementClock() {
            currentClockCycle.store((currentClockCycle.load() + 1) % clockMod);
        }
};

#endif