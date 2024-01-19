#pragma once
#include <string>
#include <array>
#include <sstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "Pendulum.h"

std::string statesToString(std::array<double, 4> states) {
  std::string statesString = "";

  for (double x : states) {
    statesString += std::to_string(x) + " ";
  }
  boost::trim(statesString);
  return statesString;
}

std::string settingsToString(int budget, int numThreads) {
  return std::to_string(budget) + " " + std::to_string(numThreads);
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
