#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "includes.h"
#include "BswDrv_CAN.h"

#define ICCID_LEN                       20
#define CHARGER_SN_LEN			        8

enum{
	NETTYPE_GPRS = 1,
	NETTYPE_485,
	NETTYPE_24G,
	NETTYPE_WIFI,
};

typedef struct {
    uint32_t size;
    uint16_t checkSum;
    uint8_t  ver;
    uint8_t  sum;
}TERMINAL_FW_INFO_STR;

//sizeof(SYSTEM_INFO_T) = 256
typedef struct
{
	uint32_t magic_number;						//
    uint8_t stationId[8];                     	// 设备号bcd, SN=10, 10 byte  0001123456789
	uint8_t idCode[8];							// 注册码
	char WifiName[32];							// ssid
	char WifiPasswd[32];						// passwd
	uint8_t reserved0[16];
    uint8_t blue_connectstate;					//蓝牙链接状态
    volatile uint8_t is_socket_0_ok;
    TERMINAL_FW_INFO_STR localFwInfo;           //本地固件信息
    uint8_t  tcp_tx_error_times;
	uint8_t  iccid[22];                         // ASCII
	uint8_t  netType;                           //网络接入类型 1:本地2g 2:485拉远 3蓝牙2.4G: 4:尝试本地 5:尝试拉远
	uint32_t mqtt_sn;
	uint8_t  isRecvStartUpAck;                  //0:未登录后台 1:登录后台
}SYSTEM_INFO_T;


typedef struct
{
	uint8_t isRegister;						//0-未注册  1-已注册
	uint8_t isLogin;						//0-未登录	1-已登陆
	// uint8_t current_usr_card_id[16];     //刷卡获取的卡号 是否有必要保存？
	uint8_t card_state;                     //读卡器状态   0: 表示初始化出错  1 :表示初始化ok 
	uint8_t GprsInitOK;						//Gprs是否初始化成功 0；未初始化 1-初始化OK
	uint8_t WG215InitOK;					//是否检测到WG215模块 0-未检测到 1-检测OK
	// uint8_t WifiInitOK;						//WiFi是否初始化 0：WiFi模块初始化错误 1：初始化ok
	uint8_t BlueInitOk;						//蓝牙是否初始化 0：未初始化	1：已经初始化
	uint8_t CBInitOK;						//刷卡版是否初始化成功 0未初始化 1-初始化成功
	uint8_t netType;						//上网方式	1-本地2G/4G 2-485  3-2.4G 4-wifi
	uint8_t modeType;						//模块型号
	uint8_t gSimStatus;						//sim卡状态
	uint8_t simSignal;						//sim卡信号值
	uint8_t iccid[ICCID_LEN+1];				//sim卡的iccid
	uint8_t doorState;						//门禁状态 0--关闭  1--打开
	uint8_t is_socket_0_ok;					//socket是否建立连接 0-未连接 1-已连接
	char BlueName[16];						//蓝牙名称
	uint8_t CBVerson;						//刷卡版版本
	uint8_t isRecvServerData;				//是否开始接收服务器数据
	uint32_t lastRecvCBTime;				//上次接收到刷卡版的心跳时间	
	uint8_t isBlueConnect;					//是否有蓝牙连接
	uint8_t isBlueLogin;					//蓝牙是否登陆
	char phonesSn[20];						//蓝牙连接手机号码
	uint32_t blueLastOnlineTime;			//蓝牙在线时间
	uint8_t upgradeFlag;					//1-系统正在升级  0-为升级
	void (*readCard_Callback)(uint8_t,uint8_t *);
	void (*AppCan_HandleCallBack)(uint8_t *data, uint16_t len, uint8_t cmd, uint8_t SourceAddr);
	QueueHandle_t CanMessageQueue;			//信息队列句柄
	CAN_DRV_MESSAGE_QUEUE_STR CanSendMessage;
	CAN_DRV_MESSAGE_QUEUE_STR CanReceiveMessage;
	CAN_READ_WRITE_INDEX_STR pReadWriteIndex;
	uint8_t SerialNum;
	uint8_t DestAddr;	//Can 通信的目的地址, 开机时GPIO读取
	struct
	{
		uint32_t PF_Bit0ToBit7:7;
	} BitFieldFlag;
}GLOBAL_INFO_T;



#pragma pack(1)
typedef struct{
	uint16_t updateFlag;
	uint16_t checkSum;
	uint32_t fsize;
}SYS_UPDATE_INFO_T;
#pragma pack()



void BswSrv_LoadSystemInfo(void);
void BswSrv_SystemResetRecord(void);
int BswSrv_GetHWId(uint8_t id[]);
int BswSrv_GetCpuTemp(void);
void BswSrv_SystemReboot(void);
void BswSrv_RF433AddrToChar(uint32_t addr,uint8_t ch[]);
uint32_t BswSrv_CharToRF433Addr(uint8_t ch[]);

uint16_t BswSrv_Tool_CheckSum(uint8_t *data,uint16_t len);
int BswSrv_Tool_isArraryEmpty(uint8_t *array,int len);
int BswSrv_Tool_StringToBCD(unsigned char *BCD, const char *str) ;
char *BswSrv_Tool_BCDToString(char *dest, unsigned char *BCD, int bytes) ;
extern int BswSrv_Tool_StringToBCD(unsigned char *BCD, const char *str);
extern char *BswSrv_Tool_BCDToString(char *dest, unsigned char *BCD, int bytes);

extern SYSTEM_INFO_T	SystemInfo;
extern GLOBAL_INFO_T	GlobalInfo;
extern SYS_UPDATE_INFO_T updateInfo;


#endif


