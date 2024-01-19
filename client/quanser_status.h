#ifndef _QUANSER_STATUS_H
#define _QUANSER_STATUS_H

#include "quanser_messages.h"

#define ERROR_LOG_BUFFER_SIZE		1024
//char error_log_buffer[ERROR_LOG_BUFFER_SIZE];
//char card_identifier[] = "0";

#define NR_ANALOG_INPUT_CHANNELS	1

#define CARD_TYPE_Q4				"q4"
#define CARD_PRIMARY_IDENTIFIER		0
#define NR_CHANNEL_ENCODERS			2
#define NR_ANALOG_CHANNELS			1
#define HIL_SUCCESS(status)			((status >= 0))
#define NUMBER_OF_SAMPLES_IN_TASK_BUFFER	1
#define QUARC_ERROR_LOG(quarc_error)			{ \
		msg_get_error_messageA(NULL, quarc_error, error_log_buffer, ERROR_LOG_BUFFER_SIZE);		\
		printf("[ERROR] %s", (quarc_error));													\
		}






#endif //_QUANSER_STATUS_H