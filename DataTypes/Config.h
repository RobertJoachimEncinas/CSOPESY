#ifndef CONFIG
#define CONFIG

typedef enum Scheduler {
    FCFS = 0,
    RR = 1,
    UNDEFINED = -1,
} Scheduler;

class Config {
    private:
        int num_cpu;
        Scheduler scheduler;
        long long quantum_cycles;
        long long batch_process_freq;
        long long min_ins;
        long long max_ins;
        long long delays_per_exec;
    public:
        Config() {
            num_cpu = -1;
            scheduler = UNDEFINED;
            quantum_cycles = -1;
            batch_process_freq = -1;
            min_ins = -1;
            max_ins = -1;
            delays_per_exec = -1;
        }

        Config(int num_cpu, Scheduler scheduler, long long quantum_cycles, 
            long long batch_process_freq, long long min_ins, 
            long long max_ins, long long delays_per_exec
        ) {
            this->num_cpu = num_cpu;
            this->scheduler = scheduler;
            this->quantum_cycles = quantum_cycles;
            this->batch_process_freq = batch_process_freq;
            this->min_ins = min_ins;
            this->max_ins = max_ins;
            this->delays_per_exec = delays_per_exec;
        }
};

#endif
