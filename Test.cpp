#include"./System/MemoryInterface.h"
#include<thread>
#include<sstream>
#include<vector>
#include<chrono>

MemoryInterface mm = MemoryInterface(64, BESTFIT);

void alloc(int id) {
    MemoryChunk* x;
    x = mm.alloc(4);
    std::ostringstream oss;
    oss << "From thread " << id << " memory assigned: " << x->toString() << "\n";
    std::cout << oss.str();
    mm.free(x);
}

int main() {
    int threads = 10;
    std::vector<std::thread> v;

    for(int i = 0; i < threads; i++) {
        v.emplace_back(alloc, i);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    for (auto& t : v) {
        t.join();
    }

    mm.printFreeList();
    return 0;
}