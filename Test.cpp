#include "./System/System.h"

int main() {
    System s = System();
    s.cmd_screen_add("process0");
    s.cmd_screen_add("process1");
    
    std::this_thread::sleep_for (std::chrono::seconds(10));
    return 0;
}
