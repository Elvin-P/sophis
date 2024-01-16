#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <future>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Pendulum.h"
#include "Utils.h"
#include "io.h"

using boost::asio::ip::tcp;

int run(int argc, const char* argv[]) {
  try {
    if (argc < 4)
    {
      std::cerr << "Usage: client <host> <budget> <cores> <?samples per step>" << std::endl;
      return 1;
    }
    auto a = [argc, &argv]() {
      if (argc < 5) {
        return 3;
      }
      return std::stoi(std::string(argv[4]));
      };
    const short samplesPerStep = a();

    boost::asio::io_context io;

    boost::asio::io_context::strand strand(io);
    std::ofstream output("C:\\Users\\Ebi\\Documents\\MATLAB\\text.txt");
    std::ofstream simout("C:\\Users\\Ebi\\Documents\\MATLAB\\textSim.txt");
    std::mutex statesMutex;
    auto x0 = readPend();
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
    auto start = std::chrono::high_resolution_clock::now();
    boost::asio::deadline_timer timer(io);

    std::vector<double> u(samplesPerStep);
    for (int i = 0; i < samplesPerStep; i++) {
      u[i] = 0.5;
    }

    for (int i = 0; i < 150; i += samplesPerStep) {
      if (i == 0) {
        start = std::chrono::high_resolution_clock::now();
        timer.expires_from_now(boost::posix_time::milliseconds(50));
      }
      x0 = readPend();
      simX = x0;
      // Predict X_hat_k+samplesPerStep on separate thread
      auto b = std::async(std::launch::async, [&simX, &u, samplesPerStep, ts]() {
        for (int j = 0; j < samplesPerStep; j++) {
          u[j] = u[j] * 0.988;
          simX = StaticPendulum::simulate(u[j], simX, ts);
        }
        });
      // Apply Uk, Uk+1, ... Uk+samplesPerStep on separate thread
      for (int j = 0; j < samplesPerStep; j++) {
        double input = u[j];
        // Apply u
        boost::asio::post(strand, [&, ts, input]() {
          applyPendInput(input);
          auto timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
          printStates(output, ts, x0, input, timeElapsed);
          });
        // Wait for next sampling interval
        boost::asio::post(strand, [&]() {
          timer.wait();
          timer.expires_at(timer.expires_at() + boost::posix_time::milliseconds(50));
          });
      }
      auto apply = std::async(std::launch::async, [&]() {
        io.run();
        io.restart();
        return 0;
        });
      // Synchronize X_hat thread
      b.get();
      // Get next set of U
      u = getInputs(socket, simX, samplesPerStep);
      // Wait for all inputs to be finish being applied
      apply.get();
      // Output X_hat to file
      simout << 50 * i << " ";
      simout << simX[0] << " " << simX[1] << " " << simX[2] << " " << simX[3] << std::endl;
      std::cout << i * 50 << " " << statesToString(simX) << " - " << statesToString(x0) << std::endl;
      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.
    }
    std::cout << "Average: " << duration.count() / 200 * samplesPerStep << " millisecondss\n";
    output.close();
    for (int j = 0; j < 3; j++) {
      applyPendInput(0.5);
    }

  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

int runSim(int argc, const char* argv[]) {
  try {
    if (argc < 4)
    {
      std::cerr << "Usage: client <host> <budget> <cores> <?samples per step>" << std::endl;
      return 1;
    }
    auto a = [argc, &argv]() {
      if (argc < 5) {
        return 3;
      }
      return std::stoi(std::string(argv[4]));
      };
    const short samplesPerStep = a();

    auto pend = new SimPendulum({ 0.0, 0.0, -M_PI, 0.0 }, 0.05);


    boost::asio::io_context io;

    boost::asio::io_context::strand strand(io);
    std::ofstream output("C:\\Users\\Ebi\\Documents\\MATLAB\\text.txt");
    std::ofstream simout("C:\\Users\\Ebi\\Documents\\MATLAB\\textSim.txt");
    std::mutex statesMutex;
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
    auto start = std::chrono::high_resolution_clock::now();
    boost::asio::steady_timer timer(io);

    std::vector<double> u(samplesPerStep);
    for (int i = 0; i < samplesPerStep; i++) {
      u[i] = 0.5;
    }

    for (int i = 0; i < 150; i += samplesPerStep) {
      if (i == 0) {
        start = std::chrono::high_resolution_clock::now();
        timer.expires_after(std::chrono::milliseconds(0));
      }
      auto x0 = pend->readStates();
      auto simX = x0;
      // Predict X_hat_k+samplesPerStep on separate thread
      auto b = std::async(std::launch::async, [&simX, &u, samplesPerStep, ts]() {
        for (int j = 0; j < samplesPerStep; j++) {
          simX = StaticPendulum::simulate(u[j], simX, ts);
        }
        });
      // Apply Uk, Uk+1, ... Uk+samplesPerStep on separate thread
      for (int j = 0; j < samplesPerStep; j++) {
        double input = u[j];
        // Apply u
        boost::asio::post(strand, [&, ts, input]() {
          // Wait for next sampling interval
          timer.wait();
          timer.expires_at(timer.expires_at() + std::chrono::milliseconds(50));
          x0 = pend->readStates();
          pend->applyInput(input);
          auto timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
          printStates(output, ts, x0, input, timeElapsed);
          });
      }
      auto apply = std::async(std::launch::async, [&]() {
        io.run();
        io.restart();
        return 0;
        });
      // Synchronize X_hat thread
      b.get();
      // Get next set of U
      u = getInputs(socket, simX, samplesPerStep);
      // Wait for all inputs to be finish being applied
      apply.get();
      // Output X_hat to file
      simout << 50 * (i + samplesPerStep) << " ";
      simout << simX[0] << " " << simX[1] << " " << simX[2] << " " << simX[3] << std::endl;
      std::cout << i * 50 << " " << statesToString(simX) << " - " << statesToString(x0) << std::endl;
      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.
    }
    std::cout << "Average: " << duration.count() / 200 * samplesPerStep << " millisecondss\n";
    output.close();
    for (int j = 0; j < 3; j++) {
      applyPendInput(0.5);
    }

  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

int main(int argc, const char* argv[]) {
  if (argc == 6 && std::string(argv[5]) == "sim") {
    return runSim(argc, argv);
  }
  //init_quanser_board();
  run(argc, argv);
  //clean_quanser_board();
  return 0;
}
