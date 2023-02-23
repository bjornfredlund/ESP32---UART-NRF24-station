#include "communication.h"
#include <string.h>
#include "include/communication.h"
#include <esp_log.h>
#include "include/mirf.h"
#include "include/protocol.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>

#include "lwip/sockets.h"

static const char* TAG = "Transmitter";

void init_radio(NRF24_t *t)
{ 
	Nrf24_init(t);
	Nrf24_config(t, CHANNEL, (uint8_t)32);

	//Set own address using 5 characters
	esp_err_t ret = Nrf24_setRADDR(t, (uint8_t *)"ABCDE");
	if (ret != ESP_OK) {
		ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
		while(1) { vTaskDelay(1); }
	}

	//Set the receiver address using 5 characters
	ret = Nrf24_setTADDR(t, (uint8_t *)"FGHIJ");
	if (ret != ESP_OK) {
		ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
		while(1) { vTaskDelay(1); }
	}
	/* clear rx buffer */
	char tempbuf[32];
	while(Nrf24_dataReady(t))
	{ 
		Nrf24_getData(t,(u_int8_t*)tempbuf);
		ESP_LOGW(TAG,"Removed %s from rx buffer",tempbuf);
	} 
	/* set faster data rate */
	Nrf24_SetSpeedDataRates(t,1);

} 
int get_latitude(states_u* s)
{


	return -1;
}
int reply_needed(unsigned char* buf)
{ 
	enum Protocol p = buf[0];
	int loops;
	switch (p) {
		case COM_GET_STATES:
			loops = 2;
			break;
		case COM_WRITEOUPUT:
			loops = -1;
			break;
		default:
			loops = -1;
			break;
	}
	return loops;
} 
int receive_radio(NRF24_t* t,unsigned char* sendbuf,int loops)
{ 

	unsigned char temp[32];
	bzero(sendbuf,strlen((char*)sendbuf));

	for(int i = 0; i < loops; i++)
	{ 
		bzero((char*)temp, strlen((char*)temp));
		int waited = 0;
		while(!Nrf24_dataReady(t))// && waited < MAX_RECIEVE_TRIES)
		{ 
			++waited;
		} 
		if(waited == MAX_RECIEVE_TRIES)
		{ 
			ESP_LOGW(TAG, "receive timeout");
			return -1;
		} 

		Nrf24_getData(t, (u_int8_t*)temp);
		strcpy((char*)sendbuf + strlen((char*)sendbuf),(char*)temp);
	} 
	return 0;
} 
int transmitt(NRF24_t* t,unsigned char* msg)
{ 
	Nrf24_send(t, (u_int8_t*)msg);

	if(!Nrf24_isSend(t,1000))
	{ 
		ESP_LOGW(TAG, "transmission error");
		return -1;
	} 

	return 0;
} 
void add_code(unsigned char* buf, enum Protocol p)
{ 
	sprintf((char*)buf + strlen((char*)buf), "%c", p);
} 
void append_double(unsigned char* buf, double num)
{ 
	sprintf((char*)buf + strlen((char*)buf), "%f", num);
	add_code(buf, DELIM);

} 
int reply(int connfd, unsigned char* message){
	int msglen = strlen((char*)message);
	int counter = 0;
	while (counter<=msglen) {
		if(write(connfd, message+counter, 1) < 0){ 
			return -1;
		} 
		counter++;
	}
	return 0;
}

int receive(int connfd,unsigned char* recvbuf){
	int maxmsglen = strlen((char*)recvbuf);
	int counter = 0;
	do { //read once and then until delimiter reached or maximum message length
		if(read(connfd, recvbuf+counter, 1) < 0){ 
			return -1;
		} 
		counter++;

	} while (!(recvbuf[counter -1] == COM_END) && counter>maxmsglen-1);
	return 0;
}
