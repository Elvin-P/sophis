#ifndef _QUANSER_BOARD_H
#define _QUANSER_BOARD_H

#include "quanser_status.h"
#include "hil.h"

extern t_card g_quanser_card;
extern t_uint32 g_encoder_channels[NR_CHANNEL_ENCODERS];
extern t_uint32 g_analog_channels[NR_ANALOG_CHANNELS];
extern t_int32	g_counts_encoders[NR_CHANNEL_ENCODERS];
extern t_double g_analog_input[NR_ANALOG_INPUT_CHANNELS];
extern t_uint32 g_analog_input_channels[NR_ANALOG_INPUT_CHANNELS];

int init_quanser_board();
int clean_quanser_board();

#endif