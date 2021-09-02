/****************************************************************************/
/* 	File    	BSP_FLASH.h 			 
 * 	Author		Hollis
 *	Notes		//0x08007800 - 0x08007F7E 为自定义用户数据区
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

#define ADDR_MCU_IDL			((uint32_t)0x1FFF7590)  //产品唯一身份标识寄存器	r0	-	r31 
#define ADDR_MCU_IDH			((uint32_t)0x1FFF7598)  //产品唯一身份标识寄存器	r64	-	r95 
#define DATA_64                 ((uint64_t)0x1234567812345678)
#define FLASH_WEITE_BYTE_SIZE   8	//一次需要写8个字节 64bit
#define FLASH_USE_VAR_NUM      	3	//只使用3个变量
#define ADDR_FLASH_TEST    		((uint32_t)0x08007A00) //0x08007800 - 0x08007F7E 为自定义用户数据区
#define ADDR_FLASH_PAGE_15    	((uint32_t)0x08007800) /* Base @ of Page 15, 2 Kbytes */

#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_15   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (ADDR_FLASH_PAGE_15 + FLASH_PAGE_SIZE - 1)   /* End @ of user Flash area */
//数据类型
typedef enum 
{        
	 MEM_FLAG_8BIT  = 1,         //8个字节的标志位
	 MEM_FLAG_16BIT  = 2,        //16个字节的标志位
	 MEM_FLAG_32BIT  = 4,        //32个字节的标志位
	 MEM_FLAG_64BIT  = 8,        //64个字节的标志位
}MEM_FLAG_LEN;

/*****************************************************
	* 函数功能:	Flash写入标志
    * 输入参数: 写入地址 Address , 写入数据 Data , 写入长度 Size
	* 返 回 值: 返回值为0表示写成功
	* 说    明：
******************************************************/
uint8_t WriteFlagToFlash(uint32_t Address , uint64_t Data , uint8_t Size);

/*****************************************************
	* 函数功能:	Flash写入数据
    * 输入参数: 写入地址 Address , 写入数据指针 *Data , 写入长度 Size
	* 返 回 值: 返回值为0表示写成功
	* 说    明：
******************************************************/
uint8_t WriteDataToFlash(uint32_t Address , uint8_t *Data , uint8_t DataLen);	//写数据为固定长度 

/*****************************************************
	* 函数功能:	获取flash地址的数据 1/2/4/8字节
    * 输入参数: 数据地址
	* 返 回 值: 
	* 说    明：
******************************************************/
uint8_t  ReadFlash_8bit(uint32_t Address);
uint16_t ReadFlash_16bit(uint32_t Address);
uint32_t ReadFlash_32bit(uint32_t Address);
uint64_t ReadFlash_64bit(uint32_t Address);

/*****************************************************
	* 函数功能:	擦除页数据
    * 输入参数: 页地址 
	* 返 回 值: 
	* 说    明：
******************************************************/
void EraseFlash(uint32_t StartPage);

/*****************************************************
	* 函数功能: 获取MCU唯一ID
	* 输入参数: void
	* 返 回 值: ID值
	* 说    明：产品唯一身份标识寄存器共96位，目前使用高32/低32位，不同MCU的ID地址不同
******************************************************/
uint32_t GetMcu_IDL(void);
uint32_t GetMcu_IDH(void);
//测试函数
void MX_FLASH_TEST(void);
#endif /* __BSP_FLASH_H__ */
