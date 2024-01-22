#pragma once
#include <boost/program_options.hpp>
#include <iostream>

struct Settings {
    const char* ip = "127.0.0.1";
    const char* mode = "";
    int budget = -1;
    int numThreads = -1;
    int samplesPerStep = 1;
    double ts = 0.05;
};

namespace po = boost::program_options;

Settings parseSettings(int argc, const char* argv[]) {
    po::options_description desc("Configurable settings");
    desc.add_options()
        ("budget", "Budget for Sophis")
        ("numThreads", "Number of threads to run Sophis on")
        ("samplesPerStep", "Number of prediction steps")
        ("ts", "Sampling period")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    Settings settings;
    if (vm.count("ip")) {
        settings.ip = vm["ip"].as<char*>();
    }
    if (vm.count("mode")) {
        const char* mode = vm["mode"].as<char*>();
        if (mode == "sim") {
            settings.mode = mode;
        }
    }
    if (vm.count("budget")) {
        settings.budget = vm["budget"].as<int>();
    }
    if (vm.count("numThreads")) {
        settings.numThreads = vm["numThreads"].as<int>();
    }
    if (vm.count("samplesPerStep")) {
        settings.numThreads = vm["samplesPerStep"].as<int>();
    }
    if (vm.count("ts")) {
        settings.ts = vm["ts"].as<double>();
    }

    return settings;
}


