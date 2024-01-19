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
#include <condition_variable>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/algorithm/string.hpp>
#include "Pendulum.h"
#include "Utils.h"
#include "quanser_board.h"
#include "io.h"

using boost::asio::ip::tcp;

struct Settings {
    int budget;
    int numThreads;
    int samplesPerStep;
};

int runExp(boost::asio::io_context& io, tcp::socket& socket, PendInterface& pend, const Settings& settings) {
    boost::system::error_code error;
    const int samplesPerStep = settings.samplesPerStep;
    boost::asio::io_context::strand strand(io);
    std::ofstream output("C:\\Users\\quancer\\Desktop\\Elvin\\text.txt");
    std::ofstream simout("C:\\Users\\quancer\\Desktop\\Elvin\\textSim.txt");
    std::ofstream inputs("C:\\Users\\quancer\\Desktop\\Elvin\\inputs.txt");
    constexpr double ts = 0.05;
    std::chrono::milliseconds duration = std::chrono::milliseconds::zero();
    auto start = std::chrono::high_resolution_clock::now();
    boost::asio::steady_timer timer(io);
    boost::asio::steady_timer readTimer(io);

    std::mutex readMutex;


    std::vector<double> u(samplesPerStep);
    for (int i = 0; i < samplesPerStep; i++) {
        u[i] = 0.5;
    }
    auto x0 = pend.readStates();
    auto predX = x0;
    auto simPend = new SimPendulum(x0, ts);
    auto simPend2 = new SimPendulum(x0, ts);
    auto simX = x0;

    std::condition_variable readFlag;
    auto stop = false;

    readTimer.expires_after(std::chrono::milliseconds(0));

    auto read = std::async(std::launch::async, [&]() {
        while (!stop) {
            readTimer.wait();
            {
                std::lock_guard<std::mutex> lock(readMutex);
                x0 = pend.readStates();
            }
            readFlag.notify_all();
            readTimer.expires_at(readTimer.expires_at() + std::chrono::milliseconds(10));
        }
        });
    
    start = std::chrono::high_resolution_clock::now();
    timer.expires_after(std::chrono::milliseconds(0));
    for (int i = 0; i < 150; i += samplesPerStep) {
        {
            std::unique_lock<std::mutex> lock(readMutex);
            readFlag.wait(lock);
            simPend->setStates(x0);
            lock.unlock();
        }
        // Predict X_hat_k+samplesPerStep on separate thread
        auto b = std::async(std::launch::async, [&predX, i, simPend, simPend2, &simX, &u, samplesPerStep, ts, &socket]() {
            for (int j = 0; j < samplesPerStep; j++) {
                double input = u[j];
                simPend->applyInput(input);
                predX = simPend->simStates();
                simPend2->applyInput(input);
                simX = simPend2->simStates();
            }
            return getInputs(socket, predX, samplesPerStep);
            });
        // Apply Uk, Uk+1, ... Uk+samplesPerStep on separate thread
        for (int j = 0; j < samplesPerStep; j++) {
            double input = u[j];
            // Apply u
            boost::asio::post(strand, [&, ts, input, j]() {
                // X_k, U_k
                auto timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
                printStates(output, ts, x0, input, timeElapsed);
                pend.applyInput(input);
                timer.expires_at(timer.expires_at() + std::chrono::milliseconds((int)(ts * 1000)));
                timer.wait();
                });
        }
        auto apply = std::async(std::launch::async, [&]() {
            io.run();
            io.restart();
            return 0;
            });
        // Get next set of U
        u = b.get();
        for (int i = 0; i < u.size(); i++) {
            inputs << u[i] << " ";
        }
        apply.get();
        inputs << std::endl;
        // Wait for all inputs to be finish being applied
      
        // Output X_hat to file
        simout << 50 * (i + samplesPerStep) << " ";
        simout << simX[0] << " " << simX[1] << " " << simX[2] << " " << simX[3] << std::endl;
        {
            std::lock_guard<std::mutex> lock(readMutex);
            std::cout << i * 50 << " " << statesToString(simX) << " - " << statesToString(x0) << std::endl;
        }
        if (error == boost::asio::error::eof)
            break; // Connection closed cleanly by peer.
        else if (error)
            throw boost::system::system_error(error); // Some other error.
    }
    stop = true;
    read.get();
    std::cout << "Average: " << duration.count() / 200 * samplesPerStep << " millisecondss\n";
    output.close();
    simout.close();
    inputs.close();
    for (int j = 0; j < 3; j++) {
        pend.applyInput(0.5);
    }
}

int main(int argc, const char* argv[]) {
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

    const Settings settings = { std::stoi(argv[2]), std::stoi(argv[3]), samplesPerStep };
    boost::asio::io_context io;
    // Connect to server
    tcp::resolver resolver(io);
    tcp::socket socket(io);
    tcp::resolver::results_type endpoints = resolver.resolve(argv[1], "daytime");
    boost::asio::connect(socket, endpoints);
    boost::system::error_code error;
    boost::asio::write(socket, boost::asio::buffer(std::to_string(settings.budget) + " " + std::to_string(settings.numThreads)), error);

  if (argc == 6 && std::string(argv[5]) == "sim") {
    SimPendulum pend({ 0.0, 0.0, -M_PI, 0.0 }, 0.05);
    return runExp(io, socket, pend, settings);
  }
  init_quanser_board();
  RealPend pend(0.04);
  runExp(io, socket, pend, settings);
  clean_quanser_board();
  return 0;
}
