#pragma once

#include <queue>
#include <mutex>
#include <iostream>
#include <chrono>
#include <condition_variable>
#include "Process.h"

class TSQueue {
    private:
        std::queue<Process*> queue;
        std::mutex mtx;
        std::condition_variable cv;

    public:
        Process* pop() {
            Process* p;
            std::unique_lock<std::mutex> l(mtx);
            cv.wait(l, [this] { return !queue.empty();});

            p = queue.front();
            queue.pop();
            
            l.unlock();

            return p;
        }

        void push(Process* p) {
            std::unique_lock<std::mutex> l(mtx);
            queue.push(p);
            l.unlock();
            cv.notify_one();
        }

        bool isEmpty() {
            return queue.empty();
        }
};