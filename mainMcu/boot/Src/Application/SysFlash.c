#include "SysFlash.h"
#include "includes.h"


static int FlashWriteWord(uint32_t address,uint8_t *pBuffer,uint16_t writeNum);



void BswDrv_SysFlashInit(void)
{

}



/**********************************************************************************************************
�� �� ��: FlashWriteWord
����˵��: д���ݵ�CPU �ڲ�Flash��ʹ�øú�����Ҫ�Ȳ�����Ӧ��flash
��    ��: address : Flash��ַ
           pBuffer : ���ݻ�����
            writeNum : ���ݴ�С����λ��4�ֽڣ�
�� �� ֵ: 0-�ɹ���-1����
**********************************************************************************************************/
int FlashWriteWord(uint32_t address,uint8_t *pBuffer,uint16_t writeNum)
{
    uint16_t i;
    uint32_t data = 0;
    uint32_t readAddr = address;
	
	for(i = 0;i < writeNum;i++)
    {
        memcpy(&data,pBuffer + i*4,4);
        fmc_word_program(readAddr, data);
        readAddr += 4;
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR); 
    }
	
     return CL_OK;
}

/**********************************************************************************************************
�� �� ��: FlashRead
����˵��: ��flash��ȡ����
��    ��: address : Flash��ַ
           pBuffer : ���ݻ�����
            readNum : ���ݴ�С
�� �� ֵ: 0-�ɹ���-1����
**********************************************************************************************************/
void BswDrv_SysFlashRead(uint32_t address, uint8_t *pBuffer, uint16_t readNum)   	
{
	uint16_t i;
    uint32_t readAddr = address;
	
	for(i = 0; i < readNum; i++)
	{
		pBuffer[i] = REG8(readAddr);
        readAddr += 1;
	}
}

int BswDrv_SysFlashErase(uint32_t address)
{	
	uint32_t offaddr;   //
	uint32_t secpos;	//

	if(address < FLASH_BASE || ((address+FLASH_PAGE_SIZE) >= (FLASH_BASE+FLASH_SIZE)))  //
	{
		return CL_FAIL;
	}

    fmc_unlock();

	offaddr = address - FLASH_BASE;		        //ʵ��ƫ�Ƶ�ַ.
	secpos = offaddr/FLASH_PAGE_SIZE;			//������ַ  


	fmc_page_erase(FLASH_BASE + secpos*FLASH_PAGE_SIZE);//�����������

	fmc_flag_clear(FMC_FLAG_BANK0_END);                 //������б�־λ
	fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
	fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

	fmc_lock();

	return CL_OK;
}

/**
 * 
 */ 
//int BswDrv_SysFlashErase(uint32_t address,uint16_t sectorNum)
//{	
//	uint32_t offaddr;   //ȥ��0X08000000��ĵ�ַ
//	uint32_t secpos;	//����ƫ�ƺ�

//	if(address < FLASH_BASE || ((address+sectorNum*FLASH_PAGE_SIZE) >= (FLASH_BASE+FLASH_SIZE)))  //�Ƿ���ַ
//	{
//		return CL_FAIL;
//	}

//    fmc_unlock();

//	offaddr = address - FLASH_BASE;		        //ʵ��ƫ�Ƶ�ַ.
//	secpos = offaddr/FLASH_PAGE_SIZE;			//������ַ  
//	//
//	for(uint16_t i = 0; i<sectorNum; i++)
//	{
//		secpos += i;
//		fmc_page_erase(FLASH_BASE + secpos*FLASH_PAGE_SIZE);//�����������
//	}

//	fmc_flag_clear(FMC_FLAG_BANK0_END);                 //������б�־λ
//	fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
//	fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

//	fmc_lock();

//	return CL_OK;
//}



int BswDrv_SysFlashWrite(uint32_t address,uint8_t *writeBuffer,uint16_t writeLen)
{
	uint16_t writeNum;

	if(address < FLASH_BASE || (address >= (FLASH_BASE+FLASH_SIZE)))  //�Ƿ���ַ
	{
		return CL_FAIL;
	}

	if(writeLen == 0)
	{
		return CL_FAIL;
	}


    fmc_unlock();

	if(writeLen%4 == 0)
	{
		writeNum = writeLen/4;
	}
	else
	{
		writeNum = writeLen/4 + 1;
	}

	FlashWriteWord(address,writeBuffer,writeNum);

	fmc_lock();

	return CL_OK;
}


#if 0

/**********************************************************************************************************
�� �� ��:   FlashWrite
����˵��:   д���ݵ�CPU �ڲ�Flash�����б������е���������
��    ��:   address : Flash��ַ����ַ�����4�ı���
            writeBuffer : ���ݻ�����
            writeLen : ���ݴ�С����λ���ֽڣ�
�� �� ֵ:   0-�ɹ���-1����
**********************************************************************************************************/
uint8_t FlashBuffer[FLASH_PAGE_SIZE] = {0};

void FlashWrite(uint32_t address,uint8_t *writeBuffer,uint16_t writeLen)
{
    uint32_t secpos;	   //������ַ
	uint16_t secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	uint16_t secremain; //������ʣ���ַ(16λ�ּ���)	 
 	uint16_t i;    
	uint32_t offaddr;   //ȥ��0X08000000��ĵ�ַ
    
    if(address < FLASH_BASE || (address >= (FLASH_BASE+FLASH_SIZE)))  //�Ƿ���ַ
	{
		return;
	}
    
    fmc_unlock();
    
    offaddr = address - FLASH_BASE;		        //ʵ��ƫ�Ƶ�ַ.
	secpos = offaddr/FLASH_PAGE_SIZE;			//������ַ  
	secoff = (offaddr%FLASH_PAGE_SIZE);			//�������ڵ�ƫ��(4���ֽ�Ϊ������λ.)
	secremain = FLASH_PAGE_SIZE - secoff;		//����ʣ��ռ��С   
	if(writeLen <= secremain)                   //�����ڸ�������Χ
	{
		secremain = writeLen;
	}

    while(1) 
	{	
		FlashRead(FLASH_BASE + secpos*FLASH_PAGE_SIZE,FlashBuffer,FLASH_PAGE_SIZE);//������������������
		for(i = 0;i < secremain;i++)            //У������
		{
			if(FlashBuffer[secoff+i] != 0XFF)   //��Ҫ����  	  
			{
				break;
			}
		}
		if(i < secremain)                       //��Ҫ����
		{
			fmc_page_erase(FLASH_BASE + secpos*FLASH_PAGE_SIZE);//�����������
            fmc_flag_clear(FMC_FLAG_BANK0_END);                 //������б�־λ
            fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
            fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
            
			for(i=0;i < secremain;i++)          //����
			{
				FlashBuffer[i+secoff]=writeBuffer[i];	  
			}

			FlashWriteWord(FLASH_BASE + secpos*FLASH_PAGE_SIZE,FlashBuffer,FLASH_PAGE_SIZE/4);//д����������  
		}
        else 
		{
			FlashWriteWord(address,writeBuffer,secremain/4);//д�Ѿ������˵�,ֱ��д������ʣ������. 	
		}
        
		if(writeLen == secremain)               //д�������
		{
			break;
		}
		else                                    //д��δ����
		{
			secpos++;				            //������ַ��1
			secoff = 0;				            //ƫ��λ��Ϊ0 	 
		   	writeBuffer += secremain;  	        //ָ��ƫ�� 
			address += secremain;	            //д��ַƫ��	   
		   	writeLen -= secremain;	            //�ֽ�(16λ)���ݼ�
			if(writeLen > FLASH_PAGE_SIZE)
			{
				secremain = FLASH_PAGE_SIZE;	//��һ����������д����
			}
			else                                //��һ����������д����
			{
				secremain = writeLen;
			}
		}	 
	}

    fmc_lock();

}


#endif
