#ifndef TSQUEUE
#define TSQUEUE

#include <queue>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include "Process.h"

class TSQueue {
    private:
        std::queue<Process*> queue;
        std::mutex mtx;
        std::condition_variable cv;

    public:
        Process* pop() {
            std::unique_lock<std::mutex> l(mtx);
            cv.wait(l, [this] { return !queue.empty();});
            Process* p = queue.front();
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
};

#endif