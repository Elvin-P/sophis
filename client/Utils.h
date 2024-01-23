#pragma once
#include <string>
#include <array>
#include <sstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "Settings.h"
#include "Pendulum.h"

std::string statesToString(std::array<double, 4> states) {
  std::string statesString = "";

  for (double x : states) {
    statesString += std::to_string(x) + " ";
  }
  boost::trim(statesString);
  return statesString;
}

std::string settingsToString(Settings &settings) {
    return std::to_string(settings.budget) + " " 
                        + std::to_string(settings.numThreads) + " " 
                        + std::to_string(settings.ts) + " " 
                        + statesToString({ settings.weights[0], settings.weights[1], settings.weights[2], settings.weights[3] }) + " " 
                        + std::to_string(settings.weights[4]);
}

std::vector<double> parseInputs(std::string uString) {
  std::vector<double> u;
  std::stringstream ss(uString);
  std::string num;

  for (int i = 0; ss >> num; i++) {
    u.push_back(std::stod(num));
  }
  return u;
}

void printStates(std::ofstream& output, double ts, const std::array<double, 4> x0, double input, long long t) {
  std::cout << input << std::endl;
  output << t << " ";
  double rew = StaticPendulum::rewardFunc(x0, input);
  for (double x : x0) {
    output << x << " ";
  }
  output << input << " ";
  output << rew << "\n";
}
