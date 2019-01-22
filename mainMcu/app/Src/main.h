#ifndef __MAIN_H__
#define __MAIN_H__
/*****************************************************************************
** @Author: quqian  
** @Date: 2018-12-05 14:20:14 
** @File: main.c
** @MCU: GD32f303VET6   
** @MCU max Speed: 120M
** @MCU Flash: 512K
** @MCU RAM: 64K
** @MCU Package: LQFP100
** @Last Modified by: quqian
** @Last Modified time: 2018-12-05 14:20:14 
*****************************************************************************/


//0x08000000
#define BOOT_SIZE	                	((uint32_t)0x3000)                  //12K
#define AppFlashAddr	                (FLASH_BASE + BOOT_SIZE)		    //App������ڵ�ַ (100k) 
#define AppUpBkpAddr	                (AppFlashAddr + (uint32_t)0x19000)	//app�������ݴ洢����ַ(132k ����+����) 801C000
#define SystemInfoAddr                  (AppUpBkpAddr + (uint32_t)0x21000)  //ϵͳ������Ϣ(2K)  
#define SysUpInfoAddr	                (SystemInfoAddr + (uint32_t)0x800)  //��������ͷ��Ϣ(2K)
#define OtherInfoAddr                  	(SysUpInfoAddr + (uint32_t)0x800)   //����(8K)

#define APP_FW_SIZE                     (AppUpBkpAddr - AppFlashAddr)



#endif



