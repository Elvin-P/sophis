#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#include <array>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <future>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include "Pendulum.h"
#include "Utils.h"
#include "io.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using boost::asio::ip::tcp;

int runSimulated(int argc, char* argv[]) {
  try {
    if (argc < 4)
    {
      std::cerr << "Usage: client <host> <budget> <cores> <?samples per step>" << std::endl;
      return 1;
    }
    auto a = [argc, &argv]() {
      if (argc != 5) {
        return 3;
      }
      return std::stoi(std::string(argv[4]));
      };
    const short samplesPerStep = a();

    boost::asio::io_context io;
    std::ofstream output("C:/Users/Ebi/Documents/MATLAB/output.txt");
    std::array<double, 4> x0 = { 0, 0, M_PI, 0 };
    std::array<double, 4> simX = x0;
    constexpr double ts = 0.05;

    // Connect to server
    tcp::resolver resolver(io);
    tcp::socket socket(io);
    tcp::resolver::results_type endpoints = resolver.resolve(argv[1], "daytime");
    boost::asio::connect(socket, endpoints);


    boost::system::error_code error;

    // Send budget and numThreads to server
    boost::asio::write(socket, boost::asio::buffer(std::string(argv[2]) + " " + std::string(argv[3])), error);

    std::chrono::milliseconds duration = std::chrono::milliseconds::zero();
    boost::asio::deadline_timer timer(io);

    bool stop = false;

    std::vector<double> u(samplesPerStep);

    for (int i = 0; i < 200; i += samplesPerStep) {
      simX = x0;
      // Apply previous inputs
      for (int j = 0; j < samplesPerStep; j++) {
        double input = u[j];
        x0 = applyInput(ts, x0, input);
        printStates(output, ts, x0, input, i + j);
      }
      // Read 3 inputs from server into queue
      for (int j = 0; j < samplesPerStep; j++) {
        simX = Pendulum::simulate(u[j], simX, ts);
      }
      u = getInputs(socket, simX, samplesPerStep);
      // Wait for all inputs to be applied
      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.
    }

    std::cout << "Average: " << duration.count() / 200 * samplesPerStep << " millisecondss\n";
    output.close();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}

int run(int argc, char* argv[]) {
  try {
    if (argc < 4)
    {
      std::cerr << "Usage: client <host> <budget> <cores> <?samples per step>" << std::endl;
      return 1;
    }
    auto a = [argc, &argv]() {
      if (argc != 5) {
        return 3;
      }
      return std::stoi(std::string(argv[4]));
      };
    const short samplesPerStep = a();

    boost::asio::io_context io;
    boost::asio::io_context::strand strand(io);
    std::ofstream output("C:/Users/Ebi/Documents/MATLAB/output.txt");
    std::array<double, 4> x0 = { 0, 0, M_PI, 0 };
    std::array<double, 4> simX = x0;
    constexpr double ts = 0.05;

    // Connect to server
    tcp::resolver resolver(io);
    tcp::socket socket(io);
    tcp::resolver::results_type endpoints = resolver.resolve(argv[1], "daytime");
    boost::asio::connect(socket, endpoints);


    boost::system::error_code error;

    // Send budget and numThreads to server
    boost::asio::write(socket, boost::asio::buffer(std::string(argv[2]) + " " + std::string(argv[3])), error);

    std::chrono::milliseconds duration = std::chrono::milliseconds::zero();
    boost::asio::deadline_timer timer(io);

    bool stop = false;

    std::vector<double> u(samplesPerStep);

    for (int i = 0; i < 200; i += samplesPerStep) {
      simX = x0;
      // Apply previous inputs
      for (int j = 0; j < samplesPerStep; j++) {
        double input = u[j];
        boost::asio::post(strand, [&, ts, i, j, input]() {
          timer.expires_from_now(boost::posix_time::milliseconds(50));
          applyPendInput(input);
          printStates(output, ts, x0, input, i + j);
          });
        boost::asio::post(strand, [&]() { timer.wait(); });
      }
      io.restart();
      auto a = std::async(std::launch::async, [&]() {
        io.run();
        });
      // Read 3 inputs from server into queue
      for (int j = 0; j < samplesPerStep; j++) {
        simX = Pendulum::simulate(u[j], simX, ts);
      }
      u = getInputs(socket, simX, samplesPerStep);
      // Wait for all inputs to be applied
      a.get();
      std::cout << statesToString(x0) << std::endl;
      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.
    }

    std::cout << "Average: " << duration.count() / 200 * samplesPerStep << " millisecondss\n";
    output.close();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}

int main(int argc, char* argv[]) {
  if (std::string(argv[5]) == "sim") {
    return runSimulated(argc, argv);
  }
  return run(argc, argv);
}
