/****************************************************************************/
/* 	File    	BSP_FLASH.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_FLASH_H__
#define __BSP_FLASH_H__

#include <stdint.h>
#include "main.h"

#define DATA_64                 ((uint64_t)0x1234567812345678)
#define FLASH_WEITE_BYTE_SIZE   8	//һ����Ҫд8���ֽ� 64bit
#define FLASH_USE_VAR_NUM      	3	//ֻʹ��3������
#define ADDR_FLASH_PAGE_15    	((uint32_t)0x08007800) /* Base @ of Page 15, 2 Kbytes */
#define ADDR_FLASH_PAGE_16    	((uint32_t)0x08008000) /* Base @ of Page 16, 2 Kbytes */

#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_15   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (ADDR_FLASH_PAGE_15 + FLASH_PAGE_SIZE - 1)   /* End @ of user Flash area */

/*****************************************************
	* ��������:	Flashд���־
    * �������: д���ַ Address , д������ Data , д�볤�� Size
	* �� �� ֵ: ����ֵΪ0��ʾд�ɹ�
	* ˵    ����
******************************************************/
uint8_t WriteFlagToFlash(uint32_t Address , uint64_t Data);

/*****************************************************
	* ��������:	����ʱFLASH��������
    * �������: д���ַ Address , д������ָ�� *Data , д�볤�� Size
	* �� �� ֵ: ����ֵΪ0��ʾд�ɹ�
	* ˵    ����ֻΪ����ʹ�ã���ͨ�ú���
******************************************************/
uint8_t UpDataToFlash(uint32_t Address , uint8_t *Data , uint8_t DataLen);

/*****************************************************
	* ��������:	Flashд������
    * �������: д���ַ Address , д������ָ�� *Data , д�볤�� Size
	* �� �� ֵ: ����ֵΪ0��ʾд�ɹ�
	* ˵    ����
******************************************************/
uint8_t WriteDataToFlash(uint32_t Address , uint8_t *Data , uint8_t DataLen);	//д����Ϊ�̶����� 

/*****************************************************
	* ��������:	��ȡflash��ַ������ 8�ֽ�
    * �������: ���ݵ�ַ
	* �� �� ֵ: 
	* ˵    ����
******************************************************/
uint64_t ReadFlagFmFlash(uint32_t Address);

/*****************************************************
	* ��������:	����ҳ����
    * �������: ҳ��ַ 
	* �� �� ֵ: 
	* ˵    ����
******************************************************/
void EraseFlash(uint32_t StartPage);

#endif /* __BSP_FLASH_H__ */
