#pragma once
#include <thread>
#include <atomic>

class Tester 
{
    private: 
        std::thread t;
        long long testerClock;
        std::atomic<long long>* currentSystemClock;
    public:
        
};