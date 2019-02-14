/*proto.h
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __PROTO_H__
#define __PROTO_H__


#include "includes.h"
#include "tfs.h"
#include "BswSrv_System.h"

#define SERVER_PKT_MAX_LEN              200

#define COST_POWER                      1  //分功率
#define COST_UNIFY                      2   //统一计费

#define COST_TEMP_CNT                   4

#define NO_LOAD_TIME                    (45) //分钟

#define START_NOTICE_REPEAT_TIMERS					4
#define STOP_NOTICE_REPEAT_TIMERS					5

//1扫码 2蓝牙 3鉴权卡 4实体月卡 5电子月卡 6钱包卡
enum {
    START_TYPE_SCAN_CODE    = 1,
    START_TYPE_BLUE = 2,
    START_TYPE_AUTH_CARD = 3,
    START_TYPE_MONTH_CARD = 4,
    START_TYPE_ELE_MONTH_CARD = 5,
    START_TYPE_WALLET_CARD = 6,
};


typedef enum {
	WECHAT_GONGZONGHAO_BT = 1,                  //微信公众号蓝牙
    WECHAT_XIAOCHENGXV_BT = 2,                  //微信小程序蓝牙
    ZHIFUBAO_XIAOCHENGXV_BT = 3,                //支付宝小程序蓝牙
    ZHIFUBAO_SHENGHUOHAO_BT = 4,                //支付宝生活号蓝牙
    ANDROID_BLUETOOTH = 5,                      //安卓蓝牙
    IOS_BLUETOOTH = 6,                          //IOS蓝牙
    ORDER_SOURCE_RESERVE1 = 7,
    ORDER_SOURCE_RESERVE2 = 8,
    ORDER_SOURCE_RESERVE3 = 9,
    ORDER_SOURCE_RESERVE4 = 10,
    WECHAT_GONGZONGHAO_REMOTE_START = 11,       //微信公众号远程开启
    WECHAT_XIAOCHENGXV_REMOTE_START = 12,       //微信小程序远程开启
    ZHIFUBAO_XIAOCHENGXV_REMOTE_START = 13,     //支付宝小程序远程开启
    ZHIFUBAO_SHENGHUOHAO_REMOTE_START = 14,     //支付宝生活号远程开启
    ANDROID_REMOTE_START = 15,                  //安卓远程开启
    IOS_REMOTE_START = 16,                      //IOS远程开启


    ORDER_SOURCE_INSERT_COINS = 20,             //投币
    ORDER_SOURCE_AUTH_CARD = 30,                //鉴权卡
    ORDER_SOURCE_PAYMENT_CARD = 31,             //钱包卡
    ORDER_SOURCE_START_OR_STOP_CARD = 32,       //启停卡
}ENUM_ORDER_SOURCE_TYPE;

enum {
    REASON_UNKNOW = 0,
    REASON_COMM_ERR = 1,                //1.记录芯片通信故障
    REASON_RESET_TIME_ENOUGH,           //2.复位时间充够
    REASON_MONEY_ENOUGH,                //3.金额充够
    REASON_TIME_ENOUGH,                 //4.时间充够
    REASON_OVER_23H,                    //5.超过12小时
    REASON_BLUE,                        //6.蓝牙结束
    REASON_TOTAL_POWER_LARGE,           //7.整桩功率过大
    REASON_AVERAGE_POWER_LOW,           //8.平均功率过低
};

enum {
    STOP_NORMAL = 0,                    //0.按照服务类型（按电量充电，按时长充电，按金额充电等）正常停止充电
    STOP_SWIPE_CARD,                    //1.用户本地刷卡停止充电
    STOP_INPUT_CODE,                    //2.用户本地输入校验码停止充电
    STOP_PHONE,                         //3.用户远程结束
    STOP_ADMINI,                        //4.管理员远程结束
    STOP_EMERG_STOP,                    //5.急停按下，停止充电
    STOP_PULL_OUT,                      //6.检测到枪头断开，停止充电
    STOP_SYS_REBOOT,                    //7.系统重启停止充电
    STOP_NET_BREAK,                     //8.充电桩断线，停止充电
    STOP_POWER_OFF,                     //9.充电桩掉电，停止充电
    STOP_CHARGER_FAULT,                 //10.充电桩故障，停止充电
    STOP_CAR_FAULT,                     //11.电动车故障，停止充电
    STOP_OTHER,                         //12.其它原因，停止充电 未知原因
    STOP_METER_FAULT,                   //13.电表故障
    STOP_CARD_FAULT,                    //14.刷卡器故障
    STOP_LCD_FAULT,                     //15.LCD故障
    STOP_CHARGING_FULL,                 //16.soc 充满
    STOP_BALANCE_NOT_ENOUG,             //17.余额不足(耗尽)
    STOP_OVER_LOAD,                     //18.过载停止
    STOP_OVER_VOLTAGE,                  //19.过压停止
    STOP_LOW_VOLTAGE,                   //20.欠压停止
    STOP_POWER_TOO_BIG,                 //21.整桩实时功率过大停止充电；
    STOP_BLUE,                          //22.手机蓝牙停止
    STOP_GND_FAULT,                     //23.设备接地故障
    STOP_TEMP_HIGHT,                    //24.设备温度过高
};

#define STOP_UNKNOW STOP_OTHER

/*
1：开机
2：关机
3：掉电
4：网络连接断开
5：网络连接断开后重连
6：开启充电
7：结束充电
8：充满
9：拔插头
10：读卡器通信故障
11：保险丝熔断
12：负载超限
13：多次开启充电失败
14：环境高温预警
15：芯片高温预警
16：订单补传
17：校准异常
18: 器件故障
*/
enum {
	EVENT_START = 1,                        //1.
	EVENT_SHUT_DOWN,                        //2.
	EVENT_POWER_DROP,                       //3.
	EVENT_NET_BREAK,                        //4.
	EVENT_NET_RECONNECT,                    //5.
	EVENT_START_CHARGING,                   //6
	EVENT_STOP_CHARGING,                    //7
	EVENT_CHARGING_FULL,                    //8
	EVENT_PLUG_PULL_OUT,                    //9
	EVENT_CARD_FAULT,                       //10
	EVENT_FUSE_BREAK,                       //11 保险丝熔断
	EVENT_OVER_LOAD,                        //12 负载超限
    EVENT_MULTI_OPEN_FAIL,                  //13
    EVENT_ENV_TEMP_HIGH,                    //14
    EVENT_CHIP_TEMP_HIGH,                   //15
    EVENT_ORDER_RESEND,                     //16
    EVENT_ADJUST_ERR,                       //17
    EVENT_CHIP_FAULT,                       //18 器件故障
	EVENT_GUN_FAULT,                        //0x13 19 设备接地故障
    EVENT_PSW_UPDATE,                       //0x14 20 密钥更新结果 0：更新成功，1：ID2解密失败，2：Token错误，3：Hash校验失败，4：内部错误
    EVENT_SMOKE_ALARM,                      //0x15 21 烟感报警
    EVENT_DOOR_F_OPEN,                      //0x16 22 门闸强开报警
};


enum {
    CHIP_EMU = 0,                           //0.计量芯片
    CHIP_FLASH,                             //1.flash
    CHIP_E2PROM,                            //2.e2prom 0系统信息；1枪头信息
    CHIP_BLUE,                              //3.蓝牙
};


enum {
    FIRST_START_CHARGING = 1,
    NET_RECOVER,
    POWER_RECOVER,
};


enum {
    EVENT_PROMPT = 1,
    EVENT_ALARM,
    EVENT_FAULT,
};

enum {
    EVENT_OCCUR = 1,
    EVENT_RECOVER,
};

enum {
	MESSAGE_VER_NOENCRYPT = 1,
	MESSAGE_VER_ENCRYPT = 2,
};

enum {
    MQTT_CMD_REGISTER = 0,
	MQTT_CMD_START_UP = 1,                  //1.
	MQTT_CMD_CARD_ID_REQ,                   //2.
	MQTT_CMD_REMOTE_SET_POWER_ON,           //3.
	MQTT_CMD_REMOTE_SET_POWER_OFF,          //4.
	MQTT_CMD_REPORT_POWER_ON,               //5.充电开始通知
	MQTT_CMD_REPORT_POWER_OFF,              //6.充电结束通知
	MQTT_CMD_HEART_BEAT,                    //7
	MQTT_CMD_COST_DOWN,                     //8
	MQTT_CMD_COST_REQ,                      //9
	MQTT_CMD_DFU_DOWN_FW_INFO,              //10
	MQTT_CMD_REMOTE_CTRL,                   //11
	MQTT_CMD_SYS_LOG,                       //12
    MQTT_CMD_EVENT_NOTICE,                  //13
	MQTT_CMD_AES_REQ=0x12,					//设备请求秘钥
	MQTT_CMD_UPDATE_AES_NOTICE=0x13,		//后台更新秘钥通知
};

enum {
    START_FAIL_FAULT = 1,
    START_FAIL_NO_TEMPLATE,
    START_FAIL_CHARGING,
    START_FAIL_NO_ADJUST,
    START_FAIL_PARA_ERR,
    START_FAIL_NET_ERR,                     //6.端口组网离线
    START_FAIL_POWER_LARGE,                 //7.整桩功率过高
    START_FAIL_NO_PULL_GUN,                 //8.未插枪
    START_FAIL_NO_MEMORY,                   //9.缓存不足
};

//1：系统立即重启 2：开启枪头 3：关闭枪头 4：进入维护状态，关闭充电服务 5：开启充电服务 6：设定最大输出功率
//7: 设定充满功率阀值	8:设定充满时间阀值	9: 设定插枪时间阀值		10: 拔枪是否停止充电订单	11: 设置刷卡板工作模式
enum {
    SYSTEM_REBOOT = 1,
    CTRL_OPEN_GUN,                                  //0x02
    CTRL_CLOSE_GUN,                                 //0x03
    CTRL_SET_SERVER_STATUS,                         //0x04
    CTRL_SET_OUT_POWER,                             //0x05
    CTRL_SET_FULL_POWER,                            //0x06
    CTRL_SET_FULL_TIME,                             //0x07
    CTRL_SET_PULL_GUN_TIME,                         //0x08
    CTRL_SET_PULL_GUN_STOP,                         //0x09
    CTRL_SET_CARD_WORK,                             //0x0a
    CTRL_CLEAR_DATA,                                //0x0b
    CTRL_SET_PRINT_SWITCH,                          //0x0c
	CTRL_SET_DISTURBING_TIME,						//0x0D
	CTRL_SET_CHARGING_FULL_STOP = 0x11,             //0x11
	CTRL_OPEN_DOOR,
	CTRL_SET_CHARGE_CHANGE_POWER = 0x13,
};

enum {
    CARD_AUTH = 0,
    CARD_CHECK_MONEY,
	CARD_MONTH,
};


#pragma pack(1)

typedef struct {
    uint8_t  aa;                                //0.
    uint8_t  five;                              //1.
    uint8_t  type;                              //2：x9 2：x10
    uint8_t  chargerSn[CHARGER_SN_LEN];         //3.
    uint16_t len;                               //4. 包含从 版本域 到 校验和域 的所有字节数
    uint8_t  ver;                               //5. b0:为1就是不支持加密;0支持加密；b1:本报文是否加密；b2~3:加密算法类型0:AES ECB
    uint16_t sn;                                //6.
    uint8_t  cmd;                               //7.
}PKT_HEAD_STR;

typedef struct {
    PKT_HEAD_STR head;
    uint8_t  data[OUT_NET_PKT_LEN];
}PKT_STR;

typedef struct {
	uint16_t dataLen;
	uint32_t time;
	uint8_t  data[OUT_NET_PKT_LEN];
}ENCRYPT_PKT_STR;

typedef struct {
    uint8_t  device_type[20];                   //1.设备类型
    uint8_t  register_code[16];                 //2.注册码
	uint8_t  hwId[20];                          //3.硬件设备ID
}REGISTER_REQ_STR;

typedef struct {
    uint8_t  result;
    uint8_t  idcode[8];                         //中心平台分配的设备识别码 bcd
}REGISTER_ACK_STR;


// start up  设备登录
typedef struct {
	uint8_t  device_type[20];                   //1.
    uint8_t  chargerSn[CHARGER_SN_LEN];         //2.桩编号 bcd
	uint8_t  fw_version;                        //3. x10版本号
	uint8_t  fw_1_ver;                          //4. X10按键板版本号
    uint8_t  sim_iccid[20];                     //5.
    uint8_t  onNetWay;                          //6.上网方式  1：本地上网；2：485路由上网；3：2.4G路由上网
    uint8_t  modeType;                          //7. 1：2G；2：4G；3：nbIot-I；4：nbIot-II
	uint8_t  login_reason;						//8. 1:上电启动			2:离线恢复重发
	uint8_t  gun_number;                        //9. 充电桩可用充电口数量，最大128个端口
	uint8_t  device_status;                     //10. 指示设备离线时曾经出现过的错误状态
	uint8_t  statistics_info[8];				//11. 统计信息 [0~2]:拨号错误 [3]续冲时长,分钟 [4]拔枪/充满自停参数 [5-6]禁音时间 [7]按键板固件版本
}START_UP_REQ_STR;

typedef struct {
    uint8_t  result;							//1.登录结果
	uint32_t time_utc;							//2.服务器时间
}START_UP_ACK_STR;


// card identify request   刷卡鉴权
typedef struct {
	uint8_t  gun_id;                            //0.
	uint8_t  card_type;                         //1.
    uint8_t  optType;                           //2： 0：刷卡鉴权，开始充电；1：仅查询余额；2：手机用户反向鉴权
	uint8_t  card_id[16];                       //3.
	uint8_t  card_psw[8];                       //4.
    uint8_t  mode;                              //5.充电模式  0：智能充满 1：按金额 2：按时长 3：按电量
	uint32_t chargingPara;                      //6.充电参数  智能充满，为0  按金额，分辨率1分  按时长，分辨率1分钟  按电量，分辨率0.01kwh
}CARD_AUTH_REQ_STR;

typedef struct {
    uint8_t  gun_id;                            //0.
	uint8_t  result;                            //1.鉴权结果
	uint8_t  cardType;                          //2. 1月卡
	uint8_t  rsv;								//3
	uint32_t user_momey;                        //4.账户余额 分
	uint8_t  order[ORDER_SECTION_LEN];          //5.订单号
}CARD_AUTH_ACK_STR;

// remote control power-on/off   远程启动充电
typedef struct {
	uint8_t  gun_id;                            //1.
	uint8_t  ordersource;                         //2.	@2014-4-13 update
	uint8_t  user_account[16];                  //3.用户账号
	uint8_t  userAttribute;                     //用户属性 1:电子月卡
	uint8_t  rsv[3];
	uint32_t money;                             //4.帐号余额 单位：分
	uint8_t  order[ORDER_SECTION_LEN];          //5.订单号
    uint8_t  mode;                              //6.充电模式  0：智能充满 1：按金额 2：按时长 3：按电量
	uint32_t chargingPara;                      //7.充电参数  智能充满，为0  按金额，分辨率1分  按时长，分辨率1分钟  按电量，分辨率0.01kwh
}START_CHARGING_REQ_STR;
typedef struct {
    uint8_t  gun_id;                            //0
	uint8_t  result;                            //1:启动成功  1: 启动失败
    uint8_t  failReason;                        //2：端口故障 2：没有计费模版 3：已经在充电中 4：设备没有校准 5：参数错误
}START_CHARGING_ACK_STR;

//远程结束充电
typedef struct {
	uint8_t  gun_id;
    uint8_t  resv_0; // Note: added by cedar
    uint8_t  resv_1; // Note: added by cedar
    uint8_t  resv_2; // Note: added by cedar
	uint8_t  order[ORDER_SECTION_LEN];          //订单号
}STOP_CHARGING_REQ_STR;
typedef struct {
    uint8_t  gun_id;
	uint8_t  result;
}STOP_CHARGING_ACK_STR;

// report device result power-on/off  充电开始通知
typedef struct {
	uint8_t  gun_id;                            //1.
	uint8_t  ordersource;                         //2.
    uint8_t  optType;                           //3. 1：首次启动充电通知 2：充电中离线恢复重发 3：充电中系统掉电恢复重发
	uint8_t  user_account[20];                  //4.用户账号
	uint8_t  order[ORDER_SECTION_LEN];          //5. 订单号
	uint32_t start_time;                        //6.开始充电时间
	uint32_t start_power;                       //7.开始充电电量
	uint32_t money;                             //8.预消费金额  单位：分，本次充电预计消费的金额，如果为全0xff则不限制
    uint32_t template_id;                       //. 计费模板id@2018-4-18 add
    uint8_t subsidyType;				        //. 补贴类型 @2018-4-18 add
	uint16_t subsidyPararm;				        //. 补贴参数 @2018-4-18 add
}START_CHARGING_NOTICE_STR;

typedef struct {
    uint8_t  gun_id;
	uint8_t  result;
	uint8_t  order[ORDER_SECTION_LEN];          //后台生成的订单号
}START_CHARGING_NOTICE_ACK_STR;


//充电结束通知
typedef struct {
	uint8_t  gun_id;                            //1.
	uint8_t  ordersource;                       //2.	@2018-4-13 update
    uint8_t  stop_reason;                       //3.结束原因
    uint8_t  stopDetal;                         //4.充电结束原因细节
	uint8_t  addChargingTimeCnt;                //进入判满阶段后，由于功率变化，延长充电时间次数
    uint8_t  ctrlFlag;                          //b0~1:拔枪停止充电(1停止/2不停)；b2~3:充满自停(1停止/2不停)
	uint16_t pullGunStopTime;                   //拔枪停止充电时间，秒
    uint8_t  fwVer;                             //6.固件版本
	uint8_t  user_account[20];                  //7.用户账号
	uint8_t  order[ORDER_SECTION_LEN];          //8.订单号
	uint32_t startTime;                         //9.开始充电时间
	uint32_t stop_time;                         //10.结束充电时间
	uint32_t startElec;                         //11.开始充电电量
	uint32_t stop_elec;                         //12.结束充电电量
	uint32_t charger_cost;                      //13.订单费用  分
	uint32_t template_id;                       //14.费率id
	uint16_t chargingPower;                     //15.按功率段计费的功率，单位：w
	uint8_t  subsidyType;                       //. 补贴类型	@2018-4-13 add
	uint16_t subsidyPararm;				        //. 补贴参数	@2018-4-13 add
    uint8_t  chargerMode;                       //. 充电模式
    uint16_t chargerPararm;                     //. 充电参数
	uint32_t chargingTime;                      //20. 实际充电时间 秒
}STOP_CHARGING_NOTICE_REQ_STR;

typedef struct {
    uint8_t  gun_id;
	uint8_t  result;
}STOP_CHARGING_NOTICE_ACK_STR;


//心跳报文最大长度 22*12+7=271
typedef struct {
    uint8_t  gunIndex;                          //1.
    uint8_t  chipTemp;                          //2.
    uint8_t  status;                            //3.充电口状态 0 空闲 1 占用未打开或关闭 2 占用已打开，充电中 3 故障
    uint8_t  faultCode;                         //4. 如果是充电中，就表示剩余充电时长，单位:分钟
    uint8_t  voltage;                           //5. 电压 1V /比特
    uint8_t  current;                           //6.输出电流 分辨率：0.1A/比特
    uint16_t power;                             //7.输出功率，单位瓦特
    uint16_t elec;                              //8.
    uint16_t money;                             //9.
    uint8_t  order[ORDER_SECTION_LEN];          //10.
}GUN_HEART_BEAT_STR;

// MQTT heart-beat  遥信及心跳
typedef struct {
    uint8_t  netSigle;                          //1. Sim卡信号
	uint8_t  envTemp;                           //2. 环境温度 度 -50度偏移  -50~200
	uint8_t  status;                         	//[0]按键板错误次数
	uint8_t  lock_state;						//柜锁状态: B0~B7:柜1到柜8的锁状态，0打开；1关闭
	uint8_t  leftCoverTemp;						//左仓盖温度
	uint8_t	 rightCoverTemp;					//右仓盖温度
    uint8_t  gunCnt;                            //3. 本次报文所包含的充电接口数目
    GUN_HEART_BEAT_STR gunStatus[GUN_NUM_MAX];
}HEART_BEAT_REQ_STR;

typedef struct {
    uint32_t time;
    uint8_t  status;                            //0，	接收成功   1，	接收失败
	uint32_t traffiId;                          //2.计费模板id
}HEART_BEAT_ACK_STR;


typedef struct {
    uint16_t startPower;                        //起始功率 单位w
    uint16_t endPower;                          //结束功率 单位w
    uint16_t price;                             //指定时长充电费用，分辨率分
    uint16_t duration;                          //计费时长,分钟  费率=100,时长=120表示1元充2小时
}segment_str;

typedef struct {
    uint8_t  segmentCnt;                        //功率段数目，1~4
    segment_str segmet[COST_TEMP_CNT];
}multiPower_t;

typedef struct {
    uint16_t  price;                            //指定时长充电费用，分辨率:分
    uint16_t  duration;                         //计费时长,分钟  费率=100,时长=120表示1元充2小时
}unify_t;

// cost template   计费模版下载
typedef struct {
    uint8_t  gunId;                             //0
	uint32_t template_id;                       //1.
    uint8_t  mode;                              //2. 1：按功率段计费 2按统一收费
    union {
        multiPower_t powerInfo;
        unify_t unifyInfo;
    }Data;
}COST_TEMPLATE_REQ_STR;

//
typedef struct {
    uint8_t  gunId;
	uint32_t template_id;
    uint8_t  mode;                              //1：按功率段计费 2按统一收费
	uint8_t Data[256];
}COST_TEMPLATE_HEAD_STR;


typedef struct {
    uint8_t  gunId;
	uint8_t  result;
}COST_TEMPLATE_ACK_STR;

typedef struct{
    uint8_t  gun_id;                            //1.
}REQ_COST_TEMPLATE_STR;


// DFU - down fw info 固件升级
typedef struct{
	char     url[48];                           //升级服务器地址，不足补0
	char     usrName[4];                        //登录用户名
	char     psw[4];
    char     fileName[8];                       //文件名
	uint32_t checkSum;                          //文件字节累加和
}DOWN_FW_REQ_STR;

typedef struct {
	uint8_t result;                             //0: 升级成功  1: 接收失败 2: 校验失败 3: 写入失败
}DOWN_FW_ACK_STR;


typedef struct {
    uint8_t  optCode;                           //1：系统立即重启 2：开启枪头 3：关闭枪头 4：进入维护状态，关闭充电服务 5：开启充电服务 6：设定最大输出功率
    uint32_t para;                              //控制参数  如果是设定功率，单位：kw
}REMO_CTRL_REQ_STR;

typedef struct {
    uint8_t  optCode;
    uint32_t para;
    uint8_t  result;
}BLUE_REMO_CTRL_ACK_STR;


typedef struct {
    uint8_t  gun_id;
    uint8_t  logType;                           //1：运行日志 2：统计信息
    uint8_t  data[OUT_NET_PKT_LEN];
}SYS_LOG_STR;


//事件通知
typedef struct {
    uint8_t  gun_id;                            //0. 如果是0表示整桩,1~128,插座接口
    uint8_t  code;                              //1. 事件代码
    uint8_t  para1;                             //2.
    uint32_t para2;                             //3.
    uint8_t  status;                            //4.1：产生  2：恢复
    uint8_t  level;                             //5.事件等级 1提示  2告警  3严重故障
    char     discrip[EVEN_DISCRI_LEN];          //6.事件详细描述
}EVENT_NOTICE_STR;

typedef struct {
    uint8_t  result;                            //0、接收成功；1、接收失败
    uint8_t  gun_id;                            //0. 如果是0表示整桩,1~128,插座接口
    uint8_t  code;                              //1. 事件代码
    uint8_t  para1;                             //2.
    uint32_t para2;                             //3.
    uint8_t  status;                            //4.1：产生  2：恢复
    uint8_t  level;                             //5.事件等级 1提示  2告警  3严重故障
}EVENT_NOTICE_ACK_STR;


//设备请求秘钥
typedef struct {
	uint8_t  id2[24];                           //0. ID2
	uint8_t  authCode[100];                     //1. 认证码
	uint8_t  extra[8];                          //2. 扩展字段
	uint8_t  reason;                            //3. 0、主动请求（如注册时）；2、解密消息失败
	uint8_t  token[16];				            //桩端生成的随机值
	uint8_t  method;				            //0:MD5		1:SHA256
}DEVICE_AES_REQ_STR;

typedef struct {
	uint8_t  result;                            //0. 结果
	uint32_t time_utc;                          //1. 系统时间
	uint8_t  aesInfo[128];	//请求token(16byte) + 秘钥(16byte) + hash值(MD5:32byte)/(SHA256:64byte)
}DEVICE_AES_REQ_ACK_STR;


//后台更新秘钥通知
typedef struct {
	uint32_t time_utc;
	uint8_t  reason;
}UPDATE_AES_NOTICE_STR;

typedef struct {
	uint8_t result;
}UPDATE_AES_ACK_STR;

/*********上报历史订单********/
typedef struct{
//	uint8_t gun_id;
//	uint8_t ordersource;				//订单来源
//	uint8_t stopReason;					//结束原因
//	uint8_t stopDetails;				//结束详情
//	uint8_t  addChargingTimeCnt;        //进入判满阶段后，由于功率变化，延长充电时间次数
//	uint8_t  ctrlFlag;                  //b0~1:拔枪停止充电(1停止/2不停)；b2~3:充满自停(1停止/2不停)
//	uint16_t pullGunStopTime;           //拔枪停止充电时间，秒
//	uint8_t chargerMode;				//充电模式
//	uint16_t chargerPararm;				//充电参数
//	uint8_t  subsidyType;				//补贴类型
//	uint16_t subsidyPararm;				//补贴参数
//	uint8_t fw_version;					//固件版本
//	uint8_t  user_account[20];          //用户账号
//	uint8_t order[10];
//	uint32_t startTime;
//	uint32_t stopTime;
//	uint32_t startElec;
//	uint32_t stopElec;
//	uint32_t cost_price;			    //订单费用
//	uint32_t template_id;			    //计费模板id
//	uint16_t power;
//	uint8_t sampleTimes;			    //采样次数	X10没有功率曲线 填0，保证格式一致
//	uint8_t sampleCycle;			    //采用周期
//	uint32_t realChargingTime;          //实际充电时间 秒
//	uint8_t rsv[49];                    //保证128字节
	uint16_t  checkSum;
}HISTORY_ORDER_STR;

#pragma pack()


int RecvServerData(PKT_STR *pFrame, uint16_t len);
void StopCharging(uint8_t gunId);
//int SendStopChargingNotice(void *order);
int SendTradeRecordNotice(HISTORY_ORDER_STR *order);
int SendStartUpNotice(int flag);
int SendRegister(void);
int SendReqCostTemplate(uint8_t gunId);
int SendStartChargingNotice(int gun_id, uint32_t startTime, int flag);
void CostTemplateReq(void);
void HeartBeatHandle(void);
int SendHeartBeatFun(uint8_t gun_id);
int SendEventNotice(uint8_t gunId, uint8_t event, uint8_t para1, uint32_t para2, uint8_t status, char *pDisc);
//int GetPktSum(uint8_t *pData, uint16_t len);
int SendCardAuthReq(int flag);
int SendSysLog(uint8_t gunId, uint8_t logType, char *pStr);
int CheckStartPara(uint8_t gun_id);
int MakeTradeInfo(HISTORY_ORDER_STR *mqtt_report_power_off_req, int gun_id);
int SendDeviceAesReq(uint32_t time_utc, uint8_t reason);
int TestCloseGunProc(uint8_t gunId);
int CostTempCopy(COST_TEMPLATE_HEAD_STR *pcost);
void StartCharging(uint8_t startMode, uint32_t money, uint8_t *pCardSn, uint8_t *pOrder, uint8_t ordersource);
int GetLeftChargingTime(uint8_t gun_id);
int CheckCostTemplate(void);
int OpenGunEmergency(uint8_t gunId, uint16_t timeLimit, uint8_t openType);
int TestOpenGunProc(uint8_t gunId, uint16_t timeLimit, uint8_t openType);
void SetPullGunStopTimeFuncy(uint32_t time);
void InitChargingCtrlPara(uint8_t gunId);
void SetChargeChangePowerFuncy(uint8_t chargePower);

extern uint8_t req_cnt[GUN_NUM_MAX];
extern uint8_t gCardReqFlag;
extern uint8_t gProtoSendBuff[OUT_NET_PKT_LEN];
extern uint8_t  gSendHearBeatCnt;
extern uint8_t AesKeyUpdateFlag;
extern uint8_t  gID2[TFS_ID2_LEN+1];
extern uint32_t SendTradeRecordNoticeTime;
extern uint32_t SendStartChargingNoticeTime;

#endif //__PROTO_H__


