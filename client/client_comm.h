#ifndef _CLIENT_COMM_H
#define _CLIENT_COMM_H

#include <array>


//      
//                      ACQUISITION PACKET FORMAT FOR FLEXIBLE LINK
//      ---------------------------------------------------------------------------------------------------------------------------------
//      |   size_of_packet_buffer (32bits)  |   timestamp (32 bits)    |    theta_angle (64 bits)   |   deflection_voltage (64 bits)    |
//      ---------------------------------------------------------------------------------------------------------------------------------
//      Note:
//          -   a packet with this format will be sent to the remote controller


//      
//                          CONTROL PACKET FORMAT FOR FLEXIBLE LINK
//                          --------------------------------------
//                          |   motor_voltage (64bits) (double)  |
//                          --------------------------------------
//      Note:
//          -   a packet with this format will be be received to this plant

int init_client_socket();
void* sender_thread(void* arg);
void* receiver_thread(void* arg);
int start_sender_receiver_threads();
void wait_for_sender_receiver_threads();
std::array<double, 4> read_pend();
void write_pend(double cmd);

#endif  //_CLIENT_COMM_H