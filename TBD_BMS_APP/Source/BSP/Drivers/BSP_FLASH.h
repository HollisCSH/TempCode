/****************************************************************************/
/* 	File    	BSP_FLASH.h 			 
 * 	Author		Hollis
 *	Notes		//0x08007800 - 0x08007F7E Ϊ�Զ����û�������
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08		first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_FLASH_H__
#define __BSP_FLASH_H__

#include <stdint.h>
#include "main.h"

#define ADDR_MCU_IDL			((uint32_t)0x1FFF7590)  //��ƷΨһ��ݱ�ʶ�Ĵ���	r0	-	r31 
#define ADDR_MCU_IDH			((uint32_t)0x1FFF7598)  //��ƷΨһ��ݱ�ʶ�Ĵ���	r64	-	r95 
#define DATA_64                 ((uint64_t)0x1234567812345678)
#define FLASH_WEITE_BYTE_SIZE   8	//һ����Ҫд8���ֽ� 64bit
#define FLASH_USE_VAR_NUM      	3	//ֻʹ��3������
#define ADDR_FLASH_TEST    		((uint32_t)0x08007A00) //0x08007800 - 0x08007F7E Ϊ�Զ����û�������
#define ADDR_FLASH_PAGE_15    	((uint32_t)0x08007800) /* Base @ of Page 15, 2 Kbytes */

#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_15   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (ADDR_FLASH_PAGE_15 + FLASH_PAGE_SIZE - 1)   /* End @ of user Flash area */
//��������
typedef enum 
{        
	 MEM_FLAG_8BIT  = 1,         //8���ֽڵı�־λ
	 MEM_FLAG_16BIT  = 2,        //16���ֽڵı�־λ
	 MEM_FLAG_32BIT  = 4,        //32���ֽڵı�־λ
	 MEM_FLAG_64BIT  = 8,        //64���ֽڵı�־λ
}MEM_FLAG_LEN;

/*****************************************************
	* ��������:	Flashд���־
    * �������: д���ַ Address , д������ Data , д�볤�� Size
	* �� �� ֵ: ����ֵΪ0��ʾд�ɹ�
	* ˵    ����
******************************************************/
uint8_t WriteFlagToFlash(uint32_t Address , uint64_t Data , uint8_t Size);

/*****************************************************
	* ��������:	Flashд������
    * �������: д���ַ Address , д������ָ�� *Data , д�볤�� Size
	* �� �� ֵ: ����ֵΪ0��ʾд�ɹ�
	* ˵    ����
******************************************************/
uint8_t WriteDataToFlash(uint32_t Address , uint8_t *Data , uint8_t DataLen);	//д����Ϊ�̶����� 

/*****************************************************
	* ��������:	��ȡflash��ַ������ 1/2/4/8�ֽ�
    * �������: ���ݵ�ַ
	* �� �� ֵ: 
	* ˵    ����
******************************************************/
uint8_t  ReadFlash_8bit(uint32_t Address);
uint16_t ReadFlash_16bit(uint32_t Address);
uint32_t ReadFlash_32bit(uint32_t Address);
uint64_t ReadFlash_64bit(uint32_t Address);

/*****************************************************
	* ��������:	����ҳ����
    * �������: ҳ��ַ 
	* �� �� ֵ: 
	* ˵    ����
******************************************************/
void EraseFlash(uint32_t StartPage);

/*****************************************************
	* ��������: ��ȡMCUΨһID
	* �������: void
	* �� �� ֵ: IDֵ
	* ˵    ������ƷΨһ��ݱ�ʶ�Ĵ�����96λ��Ŀǰʹ�ø�32/��32λ����ͬMCU��ID��ַ��ͬ
******************************************************/
uint32_t GetMcu_IDL(void);
uint32_t GetMcu_IDH(void);
//���Ժ���
void MX_FLASH_TEST(void);
#endif /* __BSP_FLASH_H__ */
