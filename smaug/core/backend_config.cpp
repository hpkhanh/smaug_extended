#include "backend_config.h"
#include "globals.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>

using namespace smaug;

BackEndConfigurator::BackEndConfigurator(int num_cpu, int num_smv){
    numCpu = num_cpu;
    numSmv = num_smv;

    cpuConfs = new BackEndConfig[numCpu];
    smvConfs = new BackEndConfig[numSmv];

    for (int i = 0; i < numCpu; i++) {
        cpuConfs[i].memSize = DEFAULT_MEM_SIZE_CPU;
        cpuConfs[i].numPEs = DEFAULT_NUM_PE_CPU;
        cpuConfs[i].numMaccsPerPE = DEFAULT_NUM_MAC_PER_PE_CPU;
    }

    for (int i = 0; i < numSmv; i++) {
        smvConfs[i].memSize = (long int)DEFAULT_MEM_SIZE_SMV;
        smvConfs[i].numPEs = DEFAULT_NUM_PE_SMV;
        smvConfs[i].numMaccsPerPE = DEFAULT_NUM_MAC_PER_PE_SMV;
    }
}

BackEndConfigurator::~BackEndConfigurator(){
    delete[] cpuConfs;
    delete[] smvConfs;
}

BackEndConfig * BackEndConfigurator::getBackEndConfig(smaug::BackEndName_t beType, int beNum){
    if (beType == Cpu){
        if ((beNum < numCpu) && (beNum >= 0)){
            return &cpuConfs[beNum];
        } else {
            // Out of range, return the config of the first CPU
            return &cpuConfs[0];
        }
    } else {
        if ((beNum < numSmv) && (beNum >= 0)){
            return &smvConfs[beNum];
        } else {
            // Out of range, return the config of the first SMV accelerator
            return &smvConfs[0];
        }
    }
}

void BackEndConfigurator::parseConfigFile(std::string config_file_path){
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(config_file_path, pt);

    int mem_size = pt.get<int>("cpu.memSize");
    for (int i = 0; i < numCpu; i++) {
        cpuConfs[i].memSize = mem_size;
    }
};
