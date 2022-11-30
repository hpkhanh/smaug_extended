#ifndef CORE_NETWORK_CONFIG
#define CORE_NETWORK_CONFIG

#include <string>
#include <vector>

#include "backend.h"
#include "smaug/core/types.pb.h"

struct LayerConfig{
    std::string layerName;
    smaug::OpType opType;
    smaug::BackEndName_t backend;
    uint numCores;
};

struct LayerConfigRaw{
    std::string layerName;
    int opType;
    std::string backend;
    int numCores;
};

class NetworkConfigurator
{
    public:
        NetworkConfigurator();
        LayerConfig * getLayerConfig(std::string layerName);
        static smaug::OpType checkOpType(int type);
        static smaug::BackEndName_t checkBackend(std::string backend);
        void parseConfigFile(std::string config_file_path);
        void printConfigs();
        std::vector<LayerConfig> * getNetworkConfig();
    private:
        std::vector<LayerConfig> _networkConfs;
};

#endif /* CORE_NETWORK_CONFIG */
