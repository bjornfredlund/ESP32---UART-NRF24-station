#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "protocol.h"
#include "mirf.h"

#define WIFI_CONNECTED_BIT 		(1 << 0)
#define WIFI_DISCONNECTED_BIT 	(1 << 1)
#define RADIO_CONNECTED_BIT 	(1 << 2)
#define RADIO_DISCONNECTED_BIT 	(1 << 3)

#define PAYLOAD_SIZE 32
#define CHANNEL 90
#define MAX_RECIEVE_TRIES 500

typedef union states_u{ 

	struct states_t { 
			double longitude, latitude, velocity;
	} states_t;
	double states_a[3];
} states_u;

void init_radio(NRF24_t* t);
void add_code(unsigned char*, enum Protocol);
void append_double(unsigned char*, double);
int reply(int, unsigned char*);
int receive(int, unsigned char*);
int send_int(int,int);
int get_latitude(states_u*);
int get_states(int, char*);
int get_longitude(states_u*);
int handle_receive(int, char*, char*);
int get_pos(states_u*);
int send_states(int, states_u*);
int transmitt(NRF24_t*,unsigned char*);
int receive_radio(NRF24_t*, unsigned char*,enum Protocol);
int reply_needed(unsigned char*);
#endif
