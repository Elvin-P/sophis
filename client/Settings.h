#pragma once
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <exception>

struct Settings {
    std::string ip = "127.0.0.1";
    std::string mode = "";
    std::vector<double> weights = { 1, 0, 1, 0.05, 0.7 };
    int budget = 20000;
    int numThreads = 8;
    int samplesPerStep = 1;
    double ts = 0.05;
    int samples = 150;
};

namespace po = boost::program_options;

Settings parseSettings(int argc, const char* argv[]) {
    po::options_description desc("Configurable settings");
    std::vector<double> weights;
    desc.add_options()
        ("ip", po::value<std::string>(), "Server ip")
        ("mode", po::value<std::string>(), "sim or real")
        ("budget", po::value<int>(), "Budget for Sophis")
        ("numThreads", po::value<int>(), "Number of threads to run Sophis on")
        ("samplesPerStep", po::value<int>(), "Number of prediction steps")
        ("ts", po::value<double>(), "Sampling period")
        ("weights", po::value<std::vector<double>>(&weights)->multitoken(), "Reward weights")
        ("samples", po::value<int>(), "Number of samples to run")
        ;
    std::cout << desc << std::endl;
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch (std::exception& e) {
        std::cerr << e.what();
        throw e;
    }
        


    Settings settings;

    if (vm.count("ip")) {
        settings.ip = vm["ip"].as<std::string>();
    }
    if (vm.count("mode")) {
        std::string mode = vm["mode"].as<std::string>();
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
        settings.samplesPerStep = vm["samplesPerStep"].as<int>();
    }
    if (vm.count("ts")) {
        settings.ts = vm["ts"].as<double>();
    }
    if (vm.count("weights")) {
        settings.weights = weights;
        std::cout << settings.weights[0] << std::endl;
    }
    if (vm.count("samples")) {
        settings.samples = vm["samples"].as<int>();
    }

    return settings;
}


