#ifndef __BLUE_PROTO_H__
#define __BLUE_PROTO_H__

#include "includes.h"
#include "BswSrv_System.h"


enum {
    B_SHAKE_REQ = 1,			//握手请求
    B_OPPO_SITE_AUTH = 2,		//用户鉴权
    B_START_CHARGING = 3,		//开启充电
    B_STOP_CHARGING = 4,		//结束充电
    B_HEART_BEAT = 5,			//心跳
    B_COST_TEMPLATE_UPLOAD = 6,	//计费模板上传
    B_REQ_COST_TEMPLATE = 7,	//请求计费模板
    B_FW_UPGRADE_NOTICE = 8,	//固件升级开始通知
    B_FW_DOWN_LOAD = 9,			//固件下发
    B_REQ_BREAK = 0x0A,			//请求断开蓝牙链接
	B_DEV_REGISTER	= 0x0B,		//设备注册
	B_DEV_LOGIN	= 0x0C,			//设备登陆
	B_HISTORY_ORDER_UPLOAD=0x0D,//历史订单上传
	B_COST_TEMPLATE_DOWNLOAD=0x0E,//计费模板下发
	B_SET_DEV_SERIALNUM = 0x0F,	//设置充电桩编号
	B_SET_DEV_CODER	= 0x10,		//设置充电桩识别码
	B_SET_GW_ADDR = 0x11,		//设置2.4G网关地址
	B_SET_SERVER_IP	 = 0x12,	//设置服务器IP
	B_SET_SERVER_PORT = 0x13,	//设置服务器端口
	B_SET_TERMINAL_NUM = 0x14,	//设置终端编号信息
	B_REMOTE_CTRL = 0x15,		//远程控制
	B_HISTORY_ORDER_ENSURE = 0x16,//历史订单上报确认
};


//b0:是否发生过心跳接收失败 b1:是否发送过发送报文失败 b2:是否带有蓝牙 b3:是否带有2.4G b4:是否进行蓝牙升级 b5:是否进行拉远升级
enum{
    HB_RECV_FAIL = 0,                           //0.
    HB_SEND_FAIL,                               //1.
    BLUE_EXIT,                                  //2.
    BLUE_24G_EXIT,                              //3.
    BLUE_UPGRADE,                               //4.
    OUT_NET_UPGRADE,                            //5.
};


typedef struct {
    uint8_t  status;                                //0:蓝牙未连接  1:连接
    uint32_t lastRecvHeartBeat;                     //最近一次接收心跳时间
	//FIFO_S_t rxBtBuff;
}BLUE_STATUS_STR;


#pragma pack(1)

typedef struct {
	uint8_t  ab;                                //0.
    uint8_t  cd;                                //1.
    uint8_t  target;                            //2.
    uint16_t len;
}BLUE_RX_HEAD_STR;


typedef struct {
    uint8_t  ab;
    uint8_t  cd;
    uint8_t  type;
    uint16_t len;
}BLUE_BT_HEAD_STR;


typedef struct {
    uint8_t  ab;                                //0.
    uint8_t  cd;                                //1.
    uint8_t  target;                            //2.
    uint8_t  addr[6];                           //3.
    uint16_t len;                               //4.
}BLUE_HEAD_STR;

typedef struct {
    BLUE_HEAD_STR head;
    uint8_t  data[255];
}BLUE_PKT_STR;


typedef struct {
    uint8_t  start;
    uint8_t  cmd;
    uint8_t  len;
}BLUE_PROTO_HEAD_STR;

typedef struct {
    BLUE_PROTO_HEAD_STR head;
    uint8_t  data[255];
}BLUE_PROTO_STR;


/***********手机握手请求**********/
typedef struct {
    uint32_t time;
	char phonesSn[12];
}BLUE_SHAKE_REQ_STR;

typedef struct {
    char     name[4];                               //0. 设备型号
    uint8_t  chargerSn[5];                          //1. 桩编号
    uint8_t  fwVer;                                 //2. 固件版本号
    uint8_t  portCnt;                               //3. 插座数目
    uint8_t  startNo;                               //4. 插座起始编号
}BLUE_SHAKE_ACK_STR;

/***********用户反向鉴权*********/
typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  optType;                               //1. 0：刷卡鉴权，开始充电；1：仅查询余额；2：手机用户反向鉴权
    uint8_t  usrId[11];                             //2. 用户账号（或者卡号）
    uint8_t  mode;                                  //3. 0：智能充满；1：按金额，具体参数见 充电参数；2：按时长；3：按电量
    uint16_t para;                                  //4. 智能充满，为0；按金额，分辨率1分；按时长，分辨率1分钟；按电量，分辨率0.01kwh
}BLUE_OPPO_SITE_AUTH_STR;

typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  result;                                //1. 0:鉴权成功；1:密码错误；2:余额不足/次数用完；3:正在使用；4:账户无效；5:其它原因
    uint8_t  cardType;                              //2. 月卡,  对于手机用户鉴权无效
    uint8_t  rsv;
    uint32_t money;                                 //3. 账户余额，分辨率1分
    uint8_t  order[ORDER_SECTION_LEN];              //4. bcd 订单号
}BLUE_OPPO_SITE_AUTH_ACK_STR;


/***********请求开启充充电*********/
typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  order[ORDER_SECTION_LEN];              //1. bcd 订单号
    uint8_t  mode;                                  //2. 0：智能充满；1：按金额，具体参数见 充电参数；2：按时长；3：按电量
    uint16_t para;                                  //3. 智能充满，为0；按金额，分辨率1分；按时长，分辨率1分钟；按电量，分辨率0.01kwh
	uint8_t orderSource;							//4. 订单来源
	uint8_t subsidyType;							//5. 补贴类型
	uint16_t subsidyPararm;							//6. 补贴参数
}BLUE_START_CHARGING_STR;

typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  result;                                //1. 0:启动成功；1: 启动失败
    uint8_t  reason;                                //2. 1：端口故障；2：没有计费模版；3：已经在充电中；4：设备没有校准；5：参数错误
	uint32_t startTime;								//3. 启动时间
	uint16_t startElec;								//4. 启动电量
}BLUE_START_CHARGING_ACK_STR;


/***********请求结束充电*********/
typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  order[ORDER_SECTION_LEN];              //1. bcd 订单号
}BLUE_STOP_CHARGING_STR;

typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  result;                                //1. 0:成功；1: 失败
	uint32_t stopTime;								//2. 结束时间
	uint16_t stopElec;								//3. 结束电量
	uint8_t stopReason;								//4. 结束原因
}BLUE_STOP_CHARGING_ACK_STR;


/**********遥信及心跳*********/
typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  status;                                //1. 充电口状态：0 空闲；1 占用；2 故障；3 离线
    uint8_t  errCode;                               //2. 1~255  当充电口状态为2故障时有效；1：状态异常；2：计量芯片通信故障
}BLUE_GUN_HEART_BEAT_STR;

typedef struct {
    uint8_t  simSignal;                             //0. Sim卡信号
    uint8_t  temp;                                  //1. 环境温度 度 -50度偏移  -50~200
    uint8_t  portCnt;                               //2. 本次报文所包含的充电接口数目
    BLUE_GUN_HEART_BEAT_STR gunStatus[GUN_NUM_MAX];
}HEART_BEAT_STR;

typedef struct {
    uint32_t time;                                  //0. 系统时间
    uint8_t  result;                                //1. 0接收成功；1接收失败
}BLUE_HEART_BEAT_ACK_STR;


/**********请求计费模板********/
typedef struct{
	uint8_t gun_id;
	uint32_t template_id;
}BLUE_COST_TEMPLATE_REQ;

/**********计费模板下发ACK*********/

typedef struct {
    uint8_t  gunId;
	uint32_t template_id;
    uint8_t  mode;                              //1：按功率段计费 2按统一收费
	uint8_t Data[256];
}BLUE_COST_TEMPLATE_HEAD_STR;

typedef struct {
	uint8_t  result;
}BLUE_COST_TEMPLATE_ACK_STR;


/*********上报历史订单********/
typedef struct {
	uint8_t gun_id;
	uint8_t ordersource;				//订单来源
	uint8_t stopReason;					//结束原因
	uint8_t stopDetails;				//结束详情
	uint8_t chargerMode;				//充电模式
	uint16_t chargerPararm;				//充电参数
	uint8_t  subsidyType;				//补贴类型
	uint16_t subsidyPararm;				//补贴参数
	uint8_t fw_version;					//固件版本
	uint8_t phoneSn[16];
	uint8_t order[ORDER_SECTION_LEN];
	uint32_t startTime;
	uint32_t stopTime;
	uint32_t startElec;
	uint32_t stopElec;
	uint32_t cost_price;			//订单费用
	uint32_t template_id;			//计费模板id
	uint16_t power;
	uint8_t sampleTimes;			//采样次数
	uint8_t sampleCycle;			//采用周期
	uint16_t samplePower[90];		//采样功率
}BLUE_UPLOAD_HISTORY_ORDER_REQ_STR;
typedef struct{
	uint8_t gun_id;
	uint8_t result;
}BLUE_UPLOAD_HISTORY_ORDER_ACK_STR;


/**********固件升级********/
typedef struct{
	uint32_t     fw_size;
	uint32_t     package_num;
	uint16_t     checkSum;
    uint8_t      fw_version;
}BLUE_DOWN_FW_REQ_STR;
typedef struct {
	uint8_t result;                             //0: 升级成功  1: 接收失败 2: 校验失败 3: 写入失败
}BLUE_DOWN_FW_ACK_STR;

typedef struct {
	uint8_t data[64];
}BLUE_FW_DOWNLOAD_REQ_STR;
typedef struct {
	uint8_t result;
    uint8_t index;
}BLUE_FW_DOWNLOAD_ACK_STR;


/**********请求断开链接********/
typedef struct {
	uint32_t     timestamp;
}BLUE_DISCONNECT_DEV_REQ_STR;
typedef struct {
	uint8_t     status;
}BLUE_DISCONNECT_DEV_ACK_STR;

/**********设备注册********/
typedef struct {
    uint8_t  device_type[8];                   //1.设备类型
    uint8_t  register_code[16];                 //2.注册码
}BLUE_REGISTER_REQ_STR;
typedef struct {
    uint8_t  result;
    uint8_t  idcode[8];                         //中心平台分配的设备识别码 bcd
}BLUE_REGISTER_ACK_STR;


/**********设备登录*******/
typedef struct {
	uint8_t  device_type[8];                   //1.
    uint8_t  chargerSn[CHARGER_SN_LEN];         //2.桩编号 bcd
	uint8_t  fw_version;                        //3.
	uint8_t  fw_1_ver;                          //4.
	uint8_t  login_reason;						//8. 1:上电启动			2:离线恢复重发
	uint8_t  gun_number;                        //9. 充电桩可用充电口数量，最大128个端口
	uint8_t  statistics_info[8];				//11. 统计信息
}BLUE_START_UP_REQ_STR;
typedef struct {
    uint8_t  result;
	uint32_t time_utc;
}BLUE_START_UP_ACK_STR;


/**********设置充电桩编号*********/
typedef struct {
	uint8_t  chargerSn[CHARGER_SN_LEN];
}BLUE_SET_CHARGERSN_REQ_STR;
typedef struct {
	uint8_t  result;
}BLUE_SET_CHARGERSN_ACK_STR;

/**********设置充电桩识别码*********/
typedef struct {
	uint8_t idcode[8];
}BLUE_SET_CHARGERIDCODE_REQ_STR;
typedef struct {
	uint8_t  result;
}BLUE_SET_CHARGERIDCODE_ACK_STR;

/**********设置2.4G网关地址*********/
typedef struct {
	uint8_t gw_addr[5];
}BLUE_SET_GW_ADDR_REQ_STR;
typedef struct {
	uint8_t  result;
}BLUE_SET_GW_ADDR_ACK_STR;

/**********远程控制*********/
typedef struct {
	uint8_t  cmd;
	uint32_t pararm;
}BLUE_REMOTE_CTRL_REQ_STR;
typedef struct {
	uint8_t  cmd;
	uint32_t pararm;
	uint8_t result;
}BLUE_REMOTE_CTRL_ACK_STR;


/**********历史订单上报确认*********/
typedef struct{
	uint8_t 	gun_id;
	uint32_t 	startTime;
	uint32_t 	stopTime;
}BLUE_Record_ENSURE_REQ_STR;
typedef struct{
	uint8_t 	gun_id;
	uint32_t 	startTime;
	uint8_t 	result;
}BLUE_Record_ENSURE_ACK_STR;


#pragma pack()


typedef struct {
    uint32_t turnOnLcdTime;                     //最近一次打开lcd的时间
    uint32_t lastRecvKbMsgTime;                 //上次接收按键板消息时间
    uint32_t lastOpenTime;                      //复用计时变量 枪头开关计时；拉远升级计时
	uint16_t chargingTotalPower;                //充电总功率 w
    uint8_t  netStatus;                         //网络操作状态: b0:是否发生过心跳接收失败 b1:是否发送过发送报文失败 b2:是否带有蓝牙 b3:心跳状态(1正常/0异常) b4:是否进行蓝牙升级 b5:是否进行拉远升级 b6:密钥协商成功 b7:收到密钥更新通知
    uint8_t  inputCode;                         //按键输入动作
    uint32_t lastInputTime;
    uint32_t blueCheck;
	uint32_t size;                              //复用变量:固件大小；蓝牙升级计时
	uint8_t sendBlueStatus;                     //发送蓝牙状态标志 0未发；1发送状态；0xff 收到应答,或发送多次没有应答，停止发送
	uint8_t  errCode;                           //语音错误代码
}CHG_INFO_STR;


extern void BlueRecvProc(uint8_t *pbuff, uint16_t len);
void BlueRecvProc(uint8_t *pbuff, uint16_t len);

extern void BlueRegister(void);
extern void BlueLogin(uint8_t method);
extern void BlueSendHeartBeat(void);
extern void BlueCostTemplateReq(uint8_t gunid);
extern void BlueCostTemplateUpload(uint8_t gunid);
extern void BlueUpLoadHistoryOrder(BLUE_UPLOAD_HISTORY_ORDER_REQ_STR *order);
extern int SendCKB24Pkt(uint8_t nodeType, BLUE_PKT_STR *pkt, uint16_t len);
extern void SendTest(void);
extern void ProcBtHeartBeat(void);


extern BLUE_STATUS_STR gBlueStatus;
extern CHG_INFO_STR gChgInfo;

#endif

