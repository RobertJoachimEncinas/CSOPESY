#include <string>
#include <map>

enum SchedAlgo {
    FCFS,
    RR
};

int parseSchedAlgo(std::string algo) {
    std::map<std::string, SchedAlgo> algoMap = {
        {"\"fcfs\"", FCFS},
        {"\"rr\"", RR}
    };

    if (algoMap.find(algo) == algoMap.end()) {
        return -1;
    }

    return algoMap[algo];
}