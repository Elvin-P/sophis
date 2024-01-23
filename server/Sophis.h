#pragma once
#include <vector>
#include <array>

std::vector<double> sophis(std::array<double, 4>, int, double, double);
std::vector<double> parallelSophis(std::array<double, 4>, std::array<double, 5>, int, double, double, short);
std::vector<double> parallelSophis2(std::array<double, 4>, int, double, double, short);
