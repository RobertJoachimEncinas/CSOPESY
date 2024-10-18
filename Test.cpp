#include "./System/System.h"

int main() {
    std::atomic<int> x;

    x.store(1);
    x++;
    
    return 0;
}
