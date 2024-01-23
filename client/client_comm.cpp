#include <iostream>
#include <array>
#include <chrono>
#include "client_comm.h"
#include "quanser_status.h"
#include "quanser_board.h"
#include "Pendulum.h"



// PACKET FORMAT DESCRIBED IN client_comm.h

#define PI 3.141599265359


std::array <double, 4> read_pend() {
	static std::array<double, 4> previousStates = { 0, 0, -M_PI, 0 };
	static auto lastRead = std::chrono::high_resolution_clock::now();
	auto ts = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - lastRead).count() / 1000.0 / 1000.0 / 1000.0;
	int status = 0;
	t_error quanser_status = 0;
	t_int32 encoder_buffer[NR_CHANNEL_ENCODERS];

	if (!HIL_SUCCESS(quanser_status))
	{
		std::cout<<"Failed to create an encoder reader!\n";
		exit(-1);
	}


	quanser_status = hil_read_encoder(g_quanser_card, g_encoder_channels, NR_CHANNEL_ENCODERS, encoder_buffer);

	lastRead = std::chrono::high_resolution_clock::now();

	if (!HIL_SUCCESS(quanser_status))
	{
		std::cout << "Something went wrong!\n";
		exit(-1);
	}

	std::array<double, 4> states;


	const double alpha_angle = encoder_buffer[1] / 4096.0 * 2.0 * PI - PI;
	const double alpha_dot = (alpha_angle - previousStates[2]) / ts;

	const double theta_angle = encoder_buffer[0] / 4096.0 * 2.0 * PI;
	const double theta_dot = (theta_angle - previousStates[0]) / ts;

	states[0] = theta_angle;
	states[1] = theta_dot;
	states[2] = alpha_angle;
	states[3] = alpha_dot;

	previousStates = states;
	return states;
}


void write_pend(double cmd) {
	int status = 0;

	t_error quarc_status = 0;
	t_double analog_buffer[NR_ANALOG_CHANNELS];

	analog_buffer[0] = cmd;
	quarc_status = hil_write_analog(g_quanser_card, g_analog_channels, NR_ANALOG_CHANNELS, analog_buffer);
}



