#ifndef __BSWSEV_WIFI_H__
#define __BSWSEV_WIFI_H__

#include "includes.h"

// #define WG215_DBG					1
#define USER_BLUETOOTH					1

#define RECEIVE_AT_LEN 		600
#define WIFI_BLUE_BUF_LEN	600


#define WIFI_POWER_ON()		gpio_bit_set(GPIOA,GPIO_PIN_0)
#define WIFI_POWER_OFF()	gpio_bit_reset(GPIOA,GPIO_PIN_0)



typedef enum {
	/*BASC CMD*/
	WIFI_AT_CMD = 0,
	WIFI_AT_RST_CMD,							//Restarts WG215 module.
	WIFI_AT_ATE,
	// WIFI_AT_VER_CMD,							//Checks version information.
	/*wifi CMD*/
	WIFI_SET_AT_CWMODE_CMD,						//Sets the Wi-Fi mode (STA/AP/STA+AP).
//	WIFI_GET_AT_CWMODE_CMD,						//Gets the Wi-Fi mode (STA/AP/STA+AP).
	WIFI_AT_CWJAP_CMD,							//Connects to an AP
	WIFI_AT_CIPDOMAIN_CMD,						//DNS function
	WIFI_SET_AT_CWDHCP_CMD,						//Enables/disables DHCP.
//	WIFI_GET_AT_CWDHCP_CMD,						//DHCP disabled or enabled now?
//	WIFI_AT_CIFSR_CMD,							//Gets the local IP address.
//	WIFI_AT_PING_CMD,							//Ping packets
	WIFI_AT_CIPSTART_CMD,						//Establishes TCP connection, UDP transmission or SSL connections
//	WIFI_AT_CIPMODE_CMD,						//Configures the transmission mode
#if USER_BLUETOOTH
	/*blue CMD*/
	BLUE_SET_AT_BLEINIT_CMD,					//1: client role, 2: server role
	BLUE_AT_BLENAME_CMD,
	BLUE_AT_BLEADVPARAM_CMD,					//���ù㲥����
	BLUE_AT_BLEADVDATA_CMD,
	BLUE_AT_BLESCANRSPDATA_CMD,					//����ɨ����Ӧ����
	BLUE_AT_BLEGATTSSRVCRE_CMD,					//����BLE GATTS����
	BLUE_AT_BLEGATTSSRVSTART_CMD,				//����GATTS����
	BLUE_AT_BLEADVSTART_CMD,					//��ʼ�㲥
#endif
	WIFI_CMD_END,
} WIFI_MSG_ENUM;


void BswSrv_WIFI_CloseSocket(void);
int BswSrv_Bule_SendData(uint8_t* BlueData, uint8_t len);
int BswSrv_WIFI_SendSocketData(uint8_t* data, uint16_t len);

int BswSrv_WIFI_HttpGet(char *httpurl);
int BswSrv_WG215_StartUp(void);
int BswSrv_WG215_StartBlue(void);
int BswSrv_WG215_StartWifi(void);
int BswSrv_Blue_Disconnent(void);

void BswSrv_WG215_Init(void);

#endif
