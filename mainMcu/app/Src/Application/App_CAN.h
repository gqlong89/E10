#ifndef __APP_CAN_H__
#define __APP_CAN_H__


#include "includes.h"
#include "CAN_Config.h"
#include "BswSrv_CAN.h"


enum {
    CAN_CMD_HAND 				= 1,				//握手请求
	CAN_CMD_REMOTE_CONTROL 		= 2,                //远程控制
	CAN_CMD_EVENT_NOTICE 		= 3,                //事件通知
	CAN_CMD_REQUES_UPGRADE		= 4,                //请求固件升级
	CAN_CMD_DOWN_FW_INFO		= 5,                //固件下发
};


#pragma pack(1)

typedef struct {
    uint8_t  five;                                    //0.
    uint8_t  aa;                                  	//1.
	uint16_t len;								    //2.长度
    uint8_t  sn;                                    //4. 如果序号不变表示报文重发
    uint8_t  cmd;                                   //6.命令代码
} APP_CAN_HEAD_STR;

typedef struct {
    APP_CAN_HEAD_STR head;
    uint8_t  data[256 - sizeof(APP_CAN_HEAD_STR)];
} APP_CAN_STR_t;


typedef struct{
    uint8_t gun;						//充电枪口 从1开始
    uint32_t systemTime;				// 系统时间
    uint8_t SourceEdgeVeb;				//主控flash中缓存的源边固件版本号
    uint8_t SubEdgeVeb;					//主控flash中缓存的副边固件版本号
}APP_CAN_SHAKE_HAND_STR;

typedef struct{
    uint8_t gun;						//充电枪口 从1开始
    uint8_t SourceEdgeVeb;				//源边正在运行的固件版本号
    uint8_t SubEdgeVeb;					//主控flash中缓存的副边固件版本号
    uint8_t SourceEdgeStatus;			//1:空闲；2:占用；3:充电中；4:故障；5:固件升级中
    uint8_t SourceEdgeErrorCode;		//
    uint8_t SubEdgeStatus;			//0:未检测；1:空闲；3:充电中；4:故障；5:固件升级中
    uint8_t SubEdgeErrorCode;		//
}APP_CAN_SHAKE_HAND_ACK_STR;

#pragma pack()




extern void AppCan_CallBack(uint8_t *data, uint16_t len, uint8_t cmd);

#if USER_ANOTHER_THREAD
extern CAN_BSW_FIFO_STR CanBswSrvAppFifo[CAN_NODE_NUMB];	
extern CAN_BSW_CACHE_STR CanApp_Cache[CAN_NODE_NUMB];	
extern int AppCan_GetOneData(int portIndex, uint8_t *pData);
extern void AppCan_FifoInit(void);
#endif



#endif




