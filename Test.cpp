#include "./System/System.h"

std::atomic<int> control(0);

class tControl {
public:
    int i;

    tControl() {
        i = 0;
    }
};

void runner(int ms, tControl* threadC) {
    while(threadC->i < 5) {
        while(control.load() == threadC->i) {}
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        threadC->i++;
        std::cout << "PRINT " << threadC->i << std::endl;
    }
}

void synchronizer(tControl* c1, tControl* c2) {
    int i = 0;
    while(i < 5) {
        while(c1->i != control.load() || c2->i != control.load()) {}
        control.store(control.load() + 1);
        i++;
    }
}

int main() {
    tControl c1, c2;

    std::thread t1(runner, 50, std::addressof(c1));
    std::thread t2(runner, 250, std::addressof(c2));
    std::thread sync(synchronizer,std::addressof(c1), std::addressof(c2));
    
    t1.join();
    t2.join();
    sync.join();
    return 0;
}
