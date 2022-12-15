#ifndef CORE_BACKEND_CONFIG
#define CORE_BACKEND_CONFIG

#include <string>
#include <vector>
#include <limits>

#include "backend.h"

// #define DEFAULT_MEM_SIZE_CPU        ((unsigned long long)((unsigned long long)3*(unsigned long long)1024*(unsigned long long)1024*(unsigned long long)1024))
// #define DEFAULT_MEM_SIZE_CPU        INT_MAX
// #define DEFAULT_NUM_PE_CPU          1
// #define DEFAULT_NUM_MAC_PER_PE_CPU  1

#define DEFAULT_MEM_SIZE_CPU        (128*1024)
#define DEFAULT_NUM_PE_CPU          8
#define DEFAULT_NUM_MAC_PER_PE_CPU  32

#define DEFAULT_MEM_SIZE_SMV        (32*1024)
#define DEFAULT_NUM_PE_SMV          8
#define DEFAULT_NUM_MAC_PER_PE_SMV  32

struct BackEndConfig {
    unsigned long long memSize;
    int numPEs;
    int numMaccsPerPE;
};

class BackEndConfigurator {
    public:
        BackEndConfigurator(int num_cpu, int num_smv);
        ~BackEndConfigurator();
        BackEndConfig * getBackEndConfig(smaug::BackEndName_t beType, int beNum);
        void parseConfigFile(std::string config_file_path);
        void printConfigs();
    private:
        BackEndConfig * cpuConfs;
        BackEndConfig * smvConfs;
        int numCpu;
        int numSmv;
};



#endif /* CORE_BACKEND_CONFIG */
