#ifndef __APP_CAN_H__
#define __APP_CAN_H__


#include "includes.h"
#include "CAN_Config.h"
#include "BswSrv_CAN.h"


#define CAN_AA_55_SRC_DEST_LEN             			(4)
#define CAN_R_N_LEN             					(2)
#define CAN_SUM_LEN             					(1)
#define CAN_SN_CMD_LEN             					(2)

enum {
    CAN_CMD_HAND 				= 1,				//握手请求
	CAN_CMD_REMOTE_CONTROL 		= 2,                //远程控制
	CAN_CMD_EVENT_NOTICE 		= 3,                //事件通知
	CAN_CMD_REQ_UPGRADE			= 4,                //请求固件升级
	CAN_CMD_DOWN_FW_INFO		= 5,                //固件下发
};


#pragma pack(1)

typedef struct {
    uint8_t  five;                                  //0.
    uint8_t  aa;                                  	//1.
    uint8_t  src_SA;                            	//2. 源地址
    uint8_t  dest_PS;                           	//3. 目的地址
	uint16_t len;								    //2. 长度 从sn到sum包括sum
    uint8_t  sn;                                    //4. 如果序号不变表示报文重发
    uint8_t  cmd;                                   //6. 命令代码
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
    uint8_t Preserve;					//保留
}APP_CAN_SHAKE_HAND_STR;				//握手请求(主控?源边)

typedef struct{
    uint8_t gun;						//充电枪口 从1开始
    uint8_t SourceEdgeVeb;				//源边正在运行的固件版本号
    uint8_t SubEdgeVeb;					//主控flash中缓存的副边固件版本号
    uint8_t SourceEdgeStatus;			//1:空闲；2:占用；3:充电中；4:故障；5:固件升级中
    uint8_t SourceEdgeErrorCode;		//
    uint8_t SubEdgeStatus;				//0:未检测；1:空闲；3:充电中；4:故障；5:固件升级中
    uint8_t SubEdgeErrorCode;			//
    uint8_t Preserve;					//保留
}APP_CAN_SHAKE_HAND_ACK_STR;			//握手应答(源边?主控)

typedef struct{
    uint8_t gun;						//充电枪口 从1开始
    uint8_t ControlCmd;					//	1、	系统重启
										//	2、	系统自检
										//	3、	上报电池BMS静态信息
										//	4、	上报电池BMS充电中遥测信息
										//	5、	开始充电
										//	6、	结束充电
    uint8_t Preserve[6];				//保留
}APP_CAN_REMOTE_CONTROL_STR;			//远程控制请求(主控?源边)

typedef struct{
    uint8_t gun;						//充电枪口 从1开始
    uint8_t ControlCmd;
	uint8_t result;						//0：成功；1：失败
    uint8_t Preserve;					//保留
}APP_CAN_REMOTE_CONTROL_ACK_STR;			//远程控制应答(源边?主控)


typedef struct{
    uint8_t gun;						//充电枪口 从1开始
    uint8_t EventCode;					//1、系统启动完成
										//	2、系统自检完成
										//	3、开始充电完成通知
										//	4、结束充电完成通知
										//	5、系统过温
										//	6、过压
										//	7、过流
										//	8、欠压
										//	9、源边与副边通信异常
										//	10、检测到副边状态变化
										//	11、检测到副边发生故障
    uint8_t Para1;						//
    uint16_t Para2;						//
    uint8_t EventState;					//1：产生；2：恢复 有些事件有产生和恢复的状态
    uint8_t EventLevel;					//1提示   2告警   3严重故障
}APP_CAN_EVENT_NOTIFY_STR;				//事件通知(源边?主控)

typedef struct{
    uint8_t gun;						//充电枪口 从1开始
    uint8_t EventCode;					//
    uint8_t Para1;						//
    uint16_t Para2;						//
    uint8_t EventState;					//1：产生；2：恢复 有些事件有产生和恢复的状态
    uint8_t EventLevel;					//1提示   2告警   3严重故障
}APP_CAN_EVENT_NOTIFY_ACK_STR;			//事件通知应答(主控?源边)



typedef struct{
    uint8_t gun;						//充电枪口 从1开始
    uint8_t FirmwareType;				//固件类型 1:源边固件；2:副边固件
    uint8_t FirmwareVeb;				//固件版本
    uint32_t FirmwareOffset;			//固件下发的起始地址偏移量，为了支持断点续传
}APP_CAN_UPGRADE_STR;					//固件升级请求(主控?源边) 

typedef struct{
    uint8_t gun;						//充电枪口 从1开始
    uint8_t FirmwareType;				//固件类型 1:源边固件；2:副边固件
    uint8_t Result;						//0：可以升级；1：无须升级；2：不能升级
    uint32_t FirmwareOffset;			//
}APP_CAN_UPGRADE_ACK_STR;				//固件升级应答 (源边?主控)


typedef struct{
    uint8_t PackNum;					//报文序号
    uint8_t sum;						//固件分片字节累加和
    uint8_t len;						//固件分片长度
    uint8_t data[64];					//固件分片静荷
}APP_CAN_FIRMWARE_DOWN_STR;				//固件下发请求(主控?源边) 

typedef struct{
    uint8_t result;						//结果 0：接收成功；1：接收失败；2：停止升级
    uint8_t PackNum;					//报文序号
}APP_CAN_FIRMWARE_DOWN_ACK_STR;			//固件下发应答 (源边?主控)

#pragma pack()




extern void AppCan_CallBack(uint8_t *data, uint16_t len, uint8_t cmd, uint8_t SourceAddr);

#if USER_ANOTHER_THREAD
extern CAN_BSW_FIFO_STR CanBswSrvAppFifo[CAN_NODE_NUMB];	
extern CAN_BSW_CACHE_STR CanApp_Cache[CAN_NODE_NUMB];	
extern int AppCan_GetOneData(int portIndex, uint8_t *pData);
extern void AppCan_FifoInit(void);
#endif



#endif




