#include <stdio.h>
#include <stdlib.h>
#include "quanser_board.h"
#include "quanser_status.h"
#include "hil.h"

static char error_log_buffer[ERROR_LOG_BUFFER_SIZE];
static char card_identifier[] = "0";


t_card g_quanser_card = 0;
t_uint32 g_encoder_channels[NR_CHANNEL_ENCODERS]={0,1};
t_uint32 g_analog_channels[NR_ANALOG_CHANNELS];
t_int32	g_counts_encoders[NR_CHANNEL_ENCODERS];

t_double g_analog_input[NR_ANALOG_INPUT_CHANNELS];
t_uint32 g_analog_input_channels[NR_ANALOG_INPUT_CHANNELS];



//
//  Initializes the quanser board with type CARD_TYPE_Q4 and NR_CHANNEL_ENCODERS
//
int
init_quanser_board()
{
    t_error status = 0;

    //	get card handle to our quanser board, PRIMARY indicates there's only one board in the system
    status = hil_open(CARD_TYPE_Q4, card_identifier, &g_quanser_card);

    if (!HIL_SUCCESS(status))
    {
        printf("Creating board handle failed!\n");

        msg_get_error_messageA(NULL, status, error_log_buffer, ERROR_LOG_BUFFER_SIZE);

        printf("[ERROR] %s\n", error_log_buffer);


        //		QUARC_ERROR_LOG(status);
        exit(-1);
    }

    //	initialize encoders count to 0, first opening of the board should initialize the encoders to 0
    status = hil_set_encoder_counts(g_quanser_card, g_encoder_channels, NR_CHANNEL_ENCODERS, g_counts_encoders);

	g_analog_input_channels[0] = 1;
    if (!HIL_SUCCESS(status))
    {
        printf("Encoder counts initialize failed!\n");
        exit(-1);
    }


    return 0;
}

int
clean_quanser_board()
{
    hil_close(g_quanser_card);

    return 0;
}
