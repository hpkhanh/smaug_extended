#ifndef CORE_LAYER_CONFIG
#define CORE_LAYER_CONFIG

#include <string>
#include <vector>

#include "backend.h"
#include "smaug/core/types.pb.h"

struct Layer_Config{
    std::string layer_name;
    smaug::OpType op_type;
    smaug::BackEndName_t backend;
    uint no_cores;
};

struct Layer_Config_Raw{
    std::string layer_name;
    int op_type;
    std::string backend;
    int no_cores;
};

class Layer_Configurator
{
    public:
        Layer_Configurator();
        Layer_Config * get_layer_config(std::string layer_name);
        static smaug::OpType check_optype(int type);
        static smaug::BackEndName_t check_backend(std::string backend);
        void parse_config_file(std::string config_file_path);
        void print_configs();
    private:
        std::vector<Layer_Config> layer_confs;
};

#endif /* CORE_LAYER_CONFIG */
