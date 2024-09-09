#ifndef SYSTEM
#define SYSTEM

#include"Config.h"

class System {
    private:
        Config config;
        bool configured;

    public:
        System() {
            this->config = Config();
            this->configured = false;
        }

        System(Config config) {
            this->config = config;
            this->configured = true;
        }
};

#endif