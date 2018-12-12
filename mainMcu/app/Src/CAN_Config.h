#ifndef __CAN_CONFIG_H__
#define __CAN_CONFIG_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include "gd32f30x.h"
#include "cmsis_os.h"



#define CAN_BUFF_512_BYTE              		(1)			//1: buff��512���ֽ� 	 0: buff��1024���ֽ�
#define CAN_NODE_NUMB                		(9)			//���Լ�ͨ�ŵĽڵ����
#define CAN_WAIT_TIMES             			(100)
#define CAN_WRITE_DATA_LEN             		(256 + 32)


#define USER_ANOTHER_THREAD             	(0)


#if CAN_BUFF_512_BYTE
	#define CAN_BUFF_LEN              		(64)			
#else
	#define CAN_BUFF_LEN              		(128)
#endif



#endif



