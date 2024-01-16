#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <boost/asio.hpp>
#include "Pendulum.h"
#include "Utils.h"
#include "client_comm.h"

using boost::asio::ip::tcp;

std::vector<double> getInputs(tcp::socket& socket, std::array<double, 4> x0, short samplesPerStep) {
  auto start = std::chrono::high_resolution_clock::now();
  boost::system::error_code error;
  boost::asio::write(socket, boost::asio::buffer(statesToString(x0)), error);
  std::array<char, 4096> buf;
  size_t len = socket.read_some(boost::asio::buffer(buf), error);
  auto stop = std::chrono::high_resolution_clock::now();

  std::string ustring(buf.data(), len);
  auto u = parseInputs(ustring);
  return u;
}

std::array<double, 4> applyInput(double ts, std::array<double, 4>& x0, double input) {
  return Pendulum::simulate(input, x0, ts);
}

void applyPendInput(double input) {
	double voltageCmd = -(input * 18 - 9);
	write_pend(voltageCmd);
	return;
}

std::array<double, 4> readPend() {
	return read_pend();
}