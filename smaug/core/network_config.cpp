#include "network_config.h"
#include "globals.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <fstream>
#include <sstream>
#include <ostream>

using namespace smaug;
using namespace std;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

BOOST_FUSION_ADAPT_STRUCT(
    LayerConfigRaw,
    (string, layerName)
    (int, opType)
    (string, backend)
    (int, numCores)
)

template <typename Iterator>
struct layer_cfg_parser : qi::grammar<Iterator, LayerConfigRaw()>
{
    layer_cfg_parser() : layer_cfg_parser::base_type(start)
    {
        using qi::int_;
        using ascii::char_;
        using qi::lit;
        using qi::lexeme;

        basic_string %= lexeme[+(char_ - ' ')];

        start =
            basic_string >> +(lit(" "))
            >>  int_ >> +(lit(" "))
            >>  basic_string >> +(lit(" "))
            >>  int_
            ;
    }

    qi::rule<Iterator, std::string()> basic_string;
    qi::rule<Iterator, LayerConfigRaw()> start;
};

NetworkConfigurator::NetworkConfigurator(){

}

OpType NetworkConfigurator::checkOpType(int type){
    OpType out_type;
    if ((type > OpType_MAX) || (type < OpType_MIN)){
        out_type = OpType_MIN;
    }
    else{
        out_type = (OpType)type;
    }

    return out_type;
}

BackEndName_t NetworkConfigurator::checkBackend(string backend)
{
    BackEndName_t backend_name;
    if (backend == "REF"){
        backend_name = Reference;
    }
    else if (backend == "SMV"){
        backend_name = Smv;
    }
    else{
        backend_name = Cpu;
    }

    return backend_name;
}

void NetworkConfigurator::parseConfigFile(string config_file_path){
    ifstream fin;

    using boost::spirit::ascii::space;
    typedef std::string::const_iterator iterator_type;
    typedef layer_cfg_parser<iterator_type> layer_cfg_parser;

    layer_cfg_parser g; // Our grammar
    std::string str;
    
    fin.open(config_file_path);

    while (getline(fin, str))
    {
        LayerConfigRaw l_conf_raw;
        LayerConfig l_conf;
        std::string::const_iterator iter = str.begin();
        std::string::const_iterator end = str.end();
        bool r = parse(iter, end, g, l_conf_raw);

        if (r && iter == end)
        {
            l_conf.layerName = l_conf_raw.layerName;
            l_conf.opType = checkOpType(l_conf_raw.opType);
            l_conf.backend = checkBackend(l_conf_raw.backend);
            if ((l_conf_raw.numCores <= 0) || (l_conf_raw.numCores > maxNumAccelerators)){
                l_conf.numCores = 1;
            }
            else {
                l_conf.numCores = (uint)l_conf_raw.numCores;
            }
            _networkConfs.push_back(l_conf);
        }
    }

    fin.close();
}

void NetworkConfigurator::printConfigs() {
    vector<LayerConfig>::iterator it;
    cout << "--------------------\n";
    cout << "Layer Config Lists:\n";
    for (it = _networkConfs.begin(); it != _networkConfs.end(); it++){
        cout << it->layerName << " " << it->opType << " " << it->backend << " " << it->numCores << endl;
    }
    cout << "--------------------\n\n";
}

LayerConfig * NetworkConfigurator::getLayerConfig(std::string layerName){
    LayerConfig * ptr = nullptr;
    vector<LayerConfig>::iterator it;
    for (it = _networkConfs.begin(); it != _networkConfs.end(); it++){
        if (it->layerName == layerName){
            ptr = &(*it);
        }
    }
    
    return ptr;
}

std::vector<LayerConfig> * NetworkConfigurator::getNetworkConfig(){
    return &_networkConfs;
}
