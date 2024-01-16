#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#define _USE_MATH_DEFINES
#define M_SPLIT 3
#define DISC_ACT_NO 0

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <ctime>
#include <tuple>
#include <chrono>
#include <vector>
#include "Pendulum.h"
#include "Sophis.h"



#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using boost::asio::ip::tcp;

std::array<double, 4> parseStates(std::string statesString) {
  std::array<double, 4> x0;
  std::stringstream ss(statesString);
  std::string num;

  for (int i = 0; ss >> num; i++) {
    x0[i] = std::stod(num);
  }
  return x0;
}

std::tuple<std::array<double, 4>, size_t> readStates(tcp::socket& socket) {
  std::array<char, 512> buff;
  boost::system::error_code ignored_error;
  std::size_t n = socket.read_some(boost::asio::buffer(buff), ignored_error);

  std::string states(buff.data(), n);

  if (n) {
    //std::cout << "Recieved states: " << states << std::endl;
    return { parseStates(states), n };
  }

  return { {0.0, 0.0, 0.0, 0.0 }, n };
}

std::tuple<int, int> readSettings(tcp::socket& socket) {
  std::array<char, 512> buff;
  boost::system::error_code ignored_error;
  std::size_t n = socket.read_some(boost::asio::buffer(buff), ignored_error);

  std::string settings(buff.data(), n);

  if (!n) {
    return { 20000, 32 };
  }
  std::stringstream ss(settings);
  std::string num;
  ss >> num;
  int budget = std::stoi(num);

  ss >> num;
  int threads = std::stoi(num);

  return { budget, threads };
}

std::string inputsToString(std::vector<double> u) {
  std::string uString = "";

  for (double input : u) {
    uString += std::to_string(input) + " ";
  }
  boost::trim(uString);
  return uString;
}

int main() {
  try {
    boost::asio::io_context io;
    tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 13));

    for (;;) {
      tcp::socket socket(io);
      acceptor.accept(socket);

      auto settings = readSettings(socket);
      int budget = std::get<0>(settings);
      int numThreads = std::get<1>(settings);
      double ts = 0.05;


      std::chrono::milliseconds duration = std::chrono::milliseconds::zero();
      int n = 0;

      for (;;) {
        auto readResult = readStates(socket);

        if (!std::get<1>(readResult)) {
          break;
        }
        auto x0 = std::get<0>(readResult);

        auto start = std::chrono::high_resolution_clock::now();
        auto u = parallelSophis(x0, budget, 0.85, ts, numThreads);
        //double u = sophis(x0, 20000, 0.85, 0.05)[0];
        auto stop = std::chrono::high_resolution_clock::now();
        duration += std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        n++;

        boost::system::error_code error;
        boost::asio::write(socket, boost::asio::buffer(inputsToString(u)), error);
        std::cout << "Output: " << inputsToString(u) << std::endl;

        if (error) {
          break;
        }
      }
      std::cout << "Average: " << duration.count() / n << " millisecondss\n";
    }
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
