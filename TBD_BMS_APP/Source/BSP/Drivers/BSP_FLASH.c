/****************************************************************************/
/* 	File    	BSP_FLASH.c 			 
 * 	Author		Hollis
 *	Notes		使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08		first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_FLASH.h"
#include "stdio.h"
#include "string.h"

/* 私有宏定义 ----------------------------------------------------------------*/
#define SWAP8(value) (uint64_t)(((value& 0x0F) << 4) | ((value & 0xF0) >> 4))
/* 私有变量 ------------------------------------------------------------------*/
static FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t startAddress;
uint32_t endAddress;
uint32_t FirstPage = 0, NbOfPages = 1, BankNumber = 0, PageError = 0;
/* 扩展变量 ------------------------------------------------------------------*/
/* 函数声明 ------------------------------------------------------------------*/

/*****************************************************
	* 函数功能: 获取目标地址Bank
    * 输入参数: 地址uint32_t Addr
	* 返 回 值: Bank值
	* 说    明：STM32G0B1共有两个BANK 每个BANK 128K
******************************************************/
static uint32_t GetBank(uint32_t Addr)
{
  if(Addr < (FLASH_BASE + FLASH_BANK_SIZE) )
  return FLASH_BANK_1;
  else
  return FLASH_BANK_2;  
}

/*****************************************************
	* 函数功能: 获取目标地址Page
    * 输入参数: 地址uint32_t Addr
	* 返 回 值: Page值，此处的page针对每个BANK而言,范围(0-63)
	* 说    明：每个page大小为2K，共128个,
******************************************************/
static uint32_t GetPage(uint32_t Addr)
{
  uint32_t page = 0;

  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }

  return page;
}

/*****************************************************
	* 函数功能:	Flash写入标志
    * 输入参数: 写入地址 Address , 写入数据 Data , 写入长度 Size
	* 返 回 值: 返回值为0表示写成功
	* 说    明：
******************************************************/
uint8_t WriteFlagToFlash(uint32_t Address , uint64_t Data , uint8_t Size)
{
    uint8_t buf[Size];
    for(int i = 0 ; i < Size; i++)
    {
        buf[i] =0xFF & (Data>>(8*i));
    }
    return WriteDataToFlash(Address , buf , Size);
}
 
/*****************************************************
	* 函数功能:	将数据地址后的8个字节转换为双字返回
    * 输入参数: 数据地址
	* 返 回 值: 
	* 说    明：
******************************************************/
uint64_t byte_to_doubleword(uint8_t *Data)
{
	uint64_t temp  =  ((uint64_t)Data[0]) | ((uint64_t)Data[1])<<8 
										| ((uint64_t)Data[2])<<16 | ((uint64_t)Data[3])<<24 
										| ((uint64_t)Data[4])<<32 | ((uint64_t)Data[5])<<40 
										| ((uint64_t)Data[6])<<48 | ((uint64_t)Data[7])<<56 ;
	return temp;
}

/*****************************************************
	* 函数功能:	Flash写入数据
    * 输入参数: 写入地址 Address , 写入数据指针 *Data , 写入长度 Size
	* 返 回 值: 返回值为0表示写成功
	* 说    明：
******************************************************/
uint8_t WriteDataToFlash(uint32_t Address , uint8_t *Data , uint8_t DataLen)	//写数据为固定长度 
{
	uint64_t vl_data[256]	;										   			//一次擦除操作2K字节  每次读出8个字节  需要256个
	uint32_t vl_start_addr;
	uint16_t i ;

	if((Address < 0x08000000) | (Address > 0x08040000) | (DataLen >128 )) 		//地址不符合要求
	return 1;
	vl_start_addr 	= Address / FLASH_PAGE_SIZE * FLASH_PAGE_SIZE;              //取出目标页的首地址
	HAL_FLASH_Unlock();
	for( i = 0; i<256 ; i++)													//读取该页内的原数据
	{
		vl_data[i] = ReadFlash_64bit(vl_start_addr + i*FLASH_WEITE_BYTE_SIZE);  		
	}
	memcpy(((uint8_t *)vl_data)+Address-vl_start_addr,Data,DataLen);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
    FirstPage = GetPage(vl_start_addr);
    NbOfPages =  1;
    BankNumber = GetBank(vl_start_addr);
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks       = BankNumber;
    EraseInitStruct.Page        = FirstPage;
    EraseInitStruct.NbPages     = NbOfPages;
	
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
		assert_param(0);                  //断言测试
        HAL_FLASH_Lock();
        return 2;
	}
  
	for( i = 0; i<256 ; i++)
	{
        //if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, vl_start_addr + i*FLASH_WEITE_BYTE_SIZE, byte_to_doubleword(&vl_data[8*i])) != HAL_OK)
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, vl_start_addr + i*FLASH_WEITE_BYTE_SIZE, vl_data[i]) != HAL_OK)
        {
            assert_param(0);                  //断言 错误提示
            HAL_FLASH_Lock();
            return 3;
        }		
	}
	HAL_FLASH_Lock();
	return 0;
}

/*****************************************************
	* 函数功能:	获取flash地址的数据 1字节
    * 输入参数: 数据地址
	* 返 回 值: 
	* 说    明：
******************************************************/
uint8_t ReadFlash_8bit(uint32_t Address)
{
	uint8_t temp = *(__IO uint8_t*)(Address);
	return temp;
}

/*****************************************************
	* 函数功能:	获取flash地址的数据 2字节
    * 输入参数: 数据地址
	* 返 回 值: 
	* 说    明：确保地址字节对齐，逐一读取合并
******************************************************/
uint16_t ReadFlash_16bit(uint32_t Address)
{
	if(Address%2 == 0)
	{
		uint16_t temp = *(__IO uint16_t*)(Address);
		return temp;
	}
	else
	{
		uint8_t tempL,tempH;
		tempL = ReadFlash_8bit(Address);
		tempH = ReadFlash_8bit(Address + 1);
		return (tempL&0x00FF)|((tempH&0x00FF)<<8);
	}
}

/*****************************************************
	* 函数功能:	获取flash地址的数据 4字节
    * 输入参数: 数据地址
	* 返 回 值: 
	* 说    明：确保地址字节对齐，逐一读取合并
******************************************************/
uint32_t ReadFlash_32bit(uint32_t Address)
{
	if(Address%4 == 0)
	{
		uint32_t temp = *(__IO uint32_t*)(Address);
		return temp;
	}
	else
	{
		uint16_t tempL,tempH;
		tempL = ReadFlash_16bit(Address);
		tempH = ReadFlash_16bit(Address + 2);
		return (tempL&0x0000FFFF)|((tempH&0x0000FFFF)<<16);
	}
}

/*****************************************************
	* 函数功能:	获取flash地址的数据 8字节
    * 输入参数: 数据地址
	* 返 回 值: 
	* 说    明：
******************************************************/
uint64_t ReadFlash_64bit(uint32_t Address)
{
	if(Address%8 == 0)
	{
		uint64_t temp = *(__IO uint64_t*)(Address);
		return temp;
	}
	else
	{
		uint32_t tempL,tempH;
		tempL = ReadFlash_32bit(Address);
		tempH = ReadFlash_32bit(Address + 2);
		return (tempL&0x00000000FFFFFFFF)|((tempH&0x00000000FFFFFFFF)<<16);
	}
}

/*****************************************************
	* 函数功能:	擦除页数据
    * 输入参数: 页地址 
	* 返 回 值: 
	* 说    明：
******************************************************/
void EraseFlash(uint32_t StartPage)
{
	PageError = 0;
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks       = GetBank(FLASH_BASE + (StartPage*FLASH_PAGE_SIZE));
    EraseInitStruct.Page        = GetPage(FLASH_BASE + (StartPage*FLASH_PAGE_SIZE));
    EraseInitStruct.NbPages     = 1;
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    HAL_FLASH_Lock();
}

/*****************************************************
	* 函数功能: 获取MCU唯一ID
	* 输入参数: void
	* 返 回 值: ID值
	* 说    明：产品唯一身份标识寄存器共96位，目前使用高32/低32位，不同MCU的ID地址不同
******************************************************/
uint32_t GetMcu_IDL(void)
{
	return *(__IO uint32_t*)(ADDR_MCU_IDL);
}
uint32_t GetMcu_IDH(void)
{
	return *(__IO uint32_t*)(ADDR_MCU_IDH);
}
///**
//  * @brief  Returns first word of the unique device identifier (UID based on 96 bits)
//  * @retval Device identifier
//  */
//uint32_t HAL_GetUIDw0(void)
//{
//  return (READ_REG(*((uint32_t *)UID_BASE)));
//}

///**
//  * @brief  Returns second word of the unique device identifier (UID based on 96 bits)
//  * @retval Device identifier
//  */
//uint32_t HAL_GetUIDw1(void)
//{
//  return (READ_REG(*((uint32_t *)(UID_BASE + 4U))));
//}

//*data = *((uint16_t*)&(SIM->UIDH) + 1 - mbaddr);	//可以替换为下以一行
//*data = *((uint16_t*)(ADDR_MCU_IDH) + 1 - mbaddr);


/*****************************************************
	* 函数功能: 简易测试
	* 输入参数: void
	* 返 回 值: 
	* 说    明：
******************************************************/
void MX_FLASH_TEST(void)
{
    volatile uint64_t data;
    #define ADDR0 0x08007A00
    #define ADDR1 0x08007A01
    #define ADDR2 0x08007A02
    #define ADDR3 0x08007A03
    #define ADDR4 0x08007A04
    #define ADDR5 0x08007A05
    #define ADDR6 0x08007A06
    #define ADDR7 0x08007A07
    WriteFlagToFlash(ADDR0,0x1234567899012345,8);
    data = ReadFlash_8bit(ADDR0);
    //	data = ReadFlash_16bit(ADDR1);
    data = ReadFlash_8bit(ADDR2);
    //	data = ReadFlash_16bit(ADDR3);
    data = ReadFlash_8bit(ADDR4);
    data = ReadFlash_8bit(ADDR5);
    data = ReadFlash_8bit(ADDR6);
    data = ReadFlash_8bit(ADDR7);

    data = ReadFlash_16bit(ADDR0);
    data = ReadFlash_16bit(ADDR2);

    data = ReadFlash_32bit(ADDR0);
    //	data = ReadFlash_32bit(ADDR1);
    data = ReadFlash_32bit(ADDR2);
    //	data = ReadFlash_32bit(ADDR3);
    data = ReadFlash_32bit(ADDR4);
    data = ReadFlash_32bit(ADDR5);
    data = ReadFlash_32bit(ADDR6);
    data = ReadFlash_32bit(ADDR7);
}
