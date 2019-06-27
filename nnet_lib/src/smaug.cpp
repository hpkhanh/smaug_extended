#include <string>

#include <boost/program_options.hpp>

#include "core/backend.h"
#include "core/globals.h"
#include "core/scheduler.h"
#include "core/network_builder.h"
#include "operators/common.h"
#include "utility/debug_stream.h"

namespace po = boost::program_options;

using namespace smaug;

int main(int argc, char* argv[]) {
    std::string modelTopo;
    std::string modelParams;
    int debugLevel = -1;
    std::string lastOutputFile;
    bool dumpGraph = false;
    runningInSimulation = false;
    SamplingInfo sampling;
    std::string samplingLevel = "no";
    sampling.num_sample_iterations = 1;
    po::options_description options(
            "SMAUG Usage:  ./smaug model_topo.pbtxt model_params.pb [options]");
    // clang-format off
    options.add_options()
        ("help,h", "Display this help message")
        ("debug-level", po::value(&debugLevel)->implicit_value(0),
         "Set the debugging output level. If omitted, all debugging output "
         "is ignored. If specified without a value, the debug level is set "
         "to zero.")
        ("dump-graph", po::value(&dumpGraph)->implicit_value(false),
         "Dump the network in GraphViz format.")
        ("gem5", po::value(&runningInSimulation)->implicit_value(true),
         "Run the network in gem5 simulation.")
        ("print-last-output,p",
         po::value(&lastOutputFile)->implicit_value("stdout"),
         "Dump the output of the last layer to this file. If specified with no "
         "argument, it is printed to stdout.")
        ("sample-level",
          po::value(&samplingLevel)->implicit_value("no"),
         "Set the sampling level. By default, SMAUG doesn't do any sampling. "
         "There are four options of sampling: no, low, medium and high. With "
         "more sampling, the simulation speed can be greatly improved at the "
         "expense of accuracy loss.")
        ("sample-num",
          po::value(&(sampling.num_sample_iterations))->implicit_value(1),
         "Set the number of sample iterations used by every sampling enabled "
         "entity. By default, the global sample number is set to 1. Larger "
         "sample number means less sampling.");
    // clang-format on

    po::options_description hidden;
    hidden.add_options()("model-topo-file", po::value(&modelTopo),
                         "Model topology protobuf file");
    hidden.add_options()("model-params-file", po::value(&modelParams),
                         "Model parameters protobuf file");
    po::options_description all, visible;
    all.add(options).add(hidden);
    visible.add(options);

    po::positional_options_description p;
    p.add("model-topo-file", 1);
    p.add("model-params-file", 1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv)
                      .options(all)
                      .positional(p)
                      .run(),
              vm);
    try {
        po::notify(vm);
    } catch (po::error& e) {
        std::cout << "ERROR: " << e.what() << "\n";
        exit(1);
    }

    if (vm.count("help")) {
        std::cout << visible << "\n";
        return 1;
    }
    if (modelTopo.empty() || modelParams.empty()) {
        std::cout << "The model protobuf files must be specified!\n";
        exit(1);
    }
    initDebugStream(debugLevel);

    std::cout << "Model topology file: " << modelTopo << "\n";
    std::cout << "Model parameters file: " << modelParams << "\n";

    if (samplingLevel == "no") {
        sampling.level = NoSampling;
    } else if (samplingLevel == "low") {
        sampling.level = Low;
    } else if (samplingLevel == "medium") {
        sampling.level = Medium;
    } else if (samplingLevel == "high") {
        sampling.level = High;
    } else {
        std::cout << "Doesn't support the specified sampling option: "
                  << samplingLevel << "\n";
        exit(1);
    }
    if (sampling.level > NoSampling) {
        std::cout << "Sampling level: " << samplingLevel
                  << ", number of sample iterations: "
                  << sampling.num_sample_iterations << "\n";
    }

    Workspace* workspace = new Workspace();
    Network* network =
            buildNetwork(modelTopo, modelParams, sampling, workspace);
    SmvBackend::initGlobals();

    if (dumpGraph)
        network->dumpDataflowGraph();

    if (!network->validate())
        return -1;

    Tensor* output = runNetwork(network, workspace);
    if (!lastOutputFile.empty()) {
        if (lastOutputFile == "stdout") {
            std::cout << "Final network output:\n" << *output << "\n";
        } else {
            std::ofstream outfile(lastOutputFile);
            outfile << "Final network output:\n" << *output << "\n";
        }
    }

    delete network;
    delete workspace;
    SmvBackend::freeGlobals();

    return 0;
}
