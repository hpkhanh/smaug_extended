#include "layer_config.h"
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
    Layer_Config_Raw,
    (string, layer_name)
    (int, op_type)
    (string, backend)
    (int, no_cores)
)

template <typename Iterator>
struct layer_cfg_parser : qi::grammar<Iterator, Layer_Config_Raw()>
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
    qi::rule<Iterator, Layer_Config_Raw()> start;
};

Layer_Configurator::Layer_Configurator(){

}

OpType Layer_Configurator::check_optype(int type){
    OpType out_type;
    if ((type > OpType_MAX) || (type < OpType_MIN)){
        out_type = OpType_MIN;
    }
    else{
        out_type = (OpType)type;
    }

    return out_type;
}

BackEndName_t Layer_Configurator::check_backend(string backend)
{
    BackEndName_t backend_name;
    if (backend == "Ref"){
        backend_name = Reference;
    }
    else if (backend == "Smv"){
        backend_name = Smv;
    }
    else{
        backend_name = Cpu;
    }

    return backend_name;
}

void Layer_Configurator::parse_config_file(string config_file_path){
    ifstream fin;

    using boost::spirit::ascii::space;
    typedef std::string::const_iterator iterator_type;
    typedef layer_cfg_parser<iterator_type> layer_cfg_parser;

    layer_cfg_parser g; // Our grammar
    std::string str;
    
    fin.open(config_file_path);

    while (getline(fin, str))
    {
        Layer_Config_Raw l_conf_raw;
        Layer_Config l_conf;
        std::string::const_iterator iter = str.begin();
        std::string::const_iterator end = str.end();
        bool r = parse(iter, end, g, l_conf_raw);

        if (r && iter == end)
        {
            l_conf.layer_name = l_conf_raw.layer_name;
            l_conf.op_type = check_optype(l_conf_raw.op_type);
            l_conf.backend = check_backend(l_conf_raw.backend);
            if ((l_conf_raw.no_cores <= 0) || (l_conf_raw.no_cores > maxNumAccelerators)){
                l_conf.no_cores = 1;
            }
            else {
                l_conf.no_cores = (uint)l_conf_raw.no_cores;
            }
            layer_confs.push_back(l_conf);
        }
    }

    fin.close();
}

void Layer_Configurator::print_configs() {
    vector<Layer_Config>::iterator it;
    cout << "Layer Config Lists:\n";
    for (it = layer_confs.begin(); it != layer_confs.end(); it++){
        cout << it->layer_name << " " << it->op_type << " " << it->backend << " " << it->no_cores << endl;
    }
    cout << "--------------------\n\n";
}