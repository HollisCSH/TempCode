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
#define FLASH_WEITE_BYTE_SIZE   8	//一次需要写8个字节 64bit
#define FLASH_USE_VAR_NUM      	3	//只使用3个变量
#define ADDR_FLASH_PAGE_15    	((uint32_t)0x08007800) /* Base @ of Page 15, 2 Kbytes */
#define ADDR_FLASH_PAGE_16    	((uint32_t)0x08008000) /* Base @ of Page 16, 2 Kbytes */

#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_15   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (ADDR_FLASH_PAGE_15 + FLASH_PAGE_SIZE - 1)   /* End @ of user Flash area */

/*****************************************************
	* 函数功能:	Flash写入标志
    * 输入参数: 写入地址 Address , 写入数据 Data , 写入长度 Size
	* 返 回 值: 返回值为0表示写成功
	* 说    明：
******************************************************/
uint8_t WriteFlagToFlash(uint32_t Address , uint64_t Data);

/*****************************************************
	* 函数功能:	升级时FLASH更新数据
    * 输入参数: 写入地址 Address , 写入数据指针 *Data , 写入长度 Size
	* 返 回 值: 返回值为0表示写成功
	* 说    明：只为升级使用，非通用函数
******************************************************/
uint8_t UpDataToFlash(uint32_t Address , uint8_t *Data , uint8_t DataLen);

/*****************************************************
	* 函数功能:	Flash写入数据
    * 输入参数: 写入地址 Address , 写入数据指针 *Data , 写入长度 Size
	* 返 回 值: 返回值为0表示写成功
	* 说    明：
******************************************************/
uint8_t WriteDataToFlash(uint32_t Address , uint8_t *Data , uint8_t DataLen);	//写数据为固定长度 

/*****************************************************
	* 函数功能:	获取flash地址的数据 8字节
    * 输入参数: 数据地址
	* 返 回 值: 
	* 说    明：
******************************************************/
uint64_t ReadFlagFmFlash(uint32_t Address);

/*****************************************************
	* 函数功能:	擦除页数据
    * 输入参数: 页地址 
	* 返 回 值: 
	* 说    明：
******************************************************/
void EraseFlash(uint32_t StartPage);

#endif /* __BSP_FLASH_H__ */
