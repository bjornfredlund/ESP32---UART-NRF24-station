#include "nvs_flash.h"
#include "freertos/freeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "communication.h"
#include "string.h"

static const char* TAG = "UART";

#define UART_NUM UART_NUM_0
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
#define TX_PIN 14
#define RX_PIN 12


static void uart_task(void *arg)
{
	QueueHandle_t* queue = (QueueHandle_t*)arg;
    uart_event_t event;
    unsigned char recvbuf[BUF_SIZE];
    unsigned char sendbuf[BUF_SIZE];

	NRF24_t t;
	init_radio(&t);

    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(*queue, (void * )&event, (portTickType)portMAX_DELAY)) {

            bzero(recvbuf, RD_BUF_SIZE);
            bzero(sendbuf, RD_BUF_SIZE);

            switch(event.type) {
				//Event of UART receving data
				case UART_DATA:
					uart_read_bytes(UART_NUM, recvbuf, event.size, portMAX_DELAY);
					if(event.size > 0)
					{ 
					ESP_LOGI(TAG, "Got: %s",recvbuf);
					ESP_LOGI(TAG, "sizeof rev: %d",sizeof(recvbuf));
					ESP_LOGI(TAG, "len recv: %d",strlen((char*)recvbuf));
						//uart_write_bytes(UART_NUM_0, (const char*) recvbuf, strlen((char*)recvbuf));
					} 

					if(transmitt(&t, recvbuf) < 0)
					{ 
						ESP_LOGI(TAG, "receive error");
						break;
					} 
					int payload_size = reply_needed(recvbuf);
					if (payload_size < 0)
					{ 
						break;
					} 
					if(receive_radio(&t, sendbuf,payload_size) < 0)
					{ 
						ESP_LOGI(TAG, "receive from car error");
						break;
					} 
					uart_write_bytes(UART_NUM, (const char*) sendbuf, strlen((char*)sendbuf));
					break;
					//Event of HW FIFO overflow detected
				case UART_FIFO_OVF:
					ESP_LOGI(TAG, "hw fifo overflow");
					// If fifo overflow happened, you should consider adding flow control for your application.
					// The ISR has already reset the rx FIFO,
					// As an example, we directly flush the rx buffer here in order to read more data.
					uart_flush_input(UART_NUM);
					xQueueReset(*queue);
					break;
					//Event of UART ring buffer full
				case UART_BUFFER_FULL:
					ESP_LOGI(TAG, "ring buffer full");
					// If buffer full happened, you should consider encreasing your buffer size
					// As an example, we directly flush the rx buffer here in order to read more data.
					uart_flush_input(UART_NUM);
					xQueueReset(*queue);
					break;
					//Event of UART RX break detected
				case UART_BREAK:
					ESP_LOGI(TAG, "uart rx break");
					break;
					//Event of UART parity check error
				case UART_PARITY_ERR:
					ESP_LOGI(TAG, "uart parity error");
					break;
					//Event of UART frame error
				case UART_FRAME_ERR:
					ESP_LOGI(TAG, "uart frame error");
					break;
					//Others
				default:
					ESP_LOGI(TAG, "uart event type: %d", event.type);
					break;
			}
		}
	}
	vTaskDelete(NULL);
}
void init_comm(QueueHandle_t* queue)
{ 
	/* Configure parameters of an UART driver,
	 * communication pins and install the driver */
	uart_config_t uart_config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB,
	};
	//Install UART driver, and get the queue.
	uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE*2, 20, queue, 0);
	uart_param_config(UART_NUM, &uart_config);

	//Set UART log level
	esp_log_level_set(TAG, ESP_LOG_NONE);
	//Set UART pins (using UART0 default pins ie no changes.)
	uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

	// flush buffers
	uart_flush_input(UART_NUM);
	uart_flush(UART_NUM);

} 


void app_main(void)
{

	QueueHandle_t queue;

	init_comm(&queue);

	//Create a task to handle UART event from ISR
	xTaskCreate(uart_task, "Uart", 8096, &queue, 12, NULL);
	for(;;) vTaskDelay(1);
}
