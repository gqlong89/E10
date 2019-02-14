/*
 * @Author: quqian 
 * @Date: 2019-02-14 18:05:36 
 * @Last Modified by:   quqian 
 * @Last Modified time: 2019-02-14 18:05:36
 */


#include "includes.h"
#include "BswDrv_HC595.h"
#include "BswDrv_Delay.h"



void HC595SendData(unsigned char SendVal)
{
	unsigned char i;

	for(i = 0;  i < 8; i++)
	{
		if((SendVal << i) & 0x80)
		{
			MOSIO_HIGH();
		}
		else
		{
			MOSIO_LOW();
		}
		SRCLK_LOW();
		BswDrv_SoftDelay_us(8);
		SRCLK_HIGH();
	}

	RCLK_LOW();
	BswDrv_SoftDelay_us(8);
	RCLK_HIGH();
}












