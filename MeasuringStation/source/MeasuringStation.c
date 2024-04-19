#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "LPC55S69_cm33_core0.h"
#include "fsl_debug_console.h"
#include "wlan_mwm.h"


#include "bmp280.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*-----------------------------------------------------------------------------
AP_SECURITY_MODE:
0 - Open
1 - WEP (Open mode)
2 - WEP (Shared mode)
3 - WPA-PSK
4 - WPA2-PSK
9 - WPA3-SAE
 ----------------------------------------------------------------------------*/
#define AP_SSID "Orange_Swiatlowod_AFA0"
#define AP_PASSPHRASE "7WbysP9dLYw92tLvpb"
#define AP_SECURITY_MODE "4"
/*---------------------------------------------------------------------------*/
#define STR_BUFFER_LEN 128
#define CDE_BUFFER_LEN 64
/*---------------------------------------------------------------------------*/
#define TEMP_MIN -40
#define TEMP_MAX 85
#define PRES_MIN 900
#define PRES_MAX 1100
#define HUMI_MIN 10
#define HUMI_MAX 100
//-----------------------------------------

char g_bufferRX[RXD_BUFFER_LEN]={0}; // HTTP RX Buffer
char g_bufferTX[TXD_BUFFER_LEN]={0}; // HTTP TX Buffer
char g_sbuffer[STR_BUFFER_LEN]; // Text Buffer
BMP280_HandleTypedef bmp280;
//------------------------------------------------
#ifndef MSEC_TO_TICK
#define MSEC_TO_TICK(msec) ((uint32_t)(msec) * (uint32_t)configTICK_RATE_HZ / 1000uL)
#endif
//------------------------------------------

void main_task(void *pvParameters) {


	wlan_init(AP_SSID, AP_PASSPHRASE , AP_SECURITY_MODE);

	sprintf(g_sbuffer, "WiFi: %s", AP_SSID);

	vTaskDelay(MSEC_TO_TICK(1000));
	char codebuffer[CDE_BUFFER_LEN]={0};
	uint8_t counter=0;
	float pressure=0, temperature=0, humidity=0;
	while (1) {
		bmp280_read_float(&bmp280, &temperature, &pressure, &humidity);

		if(counter==0) {
			sprintf(g_bufferTX,
					"api.thingspeak.com/update?api_key=3DFSEC6JSAAFZTUU&field1=%.2f&field2=%.2f&field3=%.2f",
					temperature,
					pressure/100,
					humidity);
			http_GET(g_bufferTX, g_bufferRX);
			http_head_parser(g_bufferRX, codebuffer, "HTTP");

		}
		counter++;
		if(counter >= 15) {
			counter=0;
		}

		vTaskDelay(MSEC_TO_TICK(1000));
	}
}
/*
 * @brief Application entry point.
 */
int main(void) {
	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();
#endif

	bmp280_init_default_params(&bmp280.params);
	bmp280.addr = BMP280_I2C_ADDRESS_0;
	bmp280.i2c = FLEXCOMM1_PERIPHERAL;
	if (!bmp280_init(&bmp280, &bmp280.params)) {

		while(1)
			;
	}
	if (xTaskCreate(main_task, "main_task", 350, NULL, MAIN_TASK_PRIORITY, NULL) != pdPASS) {
		PRINTF("Task creation failed!.\r\n");
		while (1)
			;
	}
	vTaskStartScheduler();
	while(1) {
	}
	return 0 ;
}
