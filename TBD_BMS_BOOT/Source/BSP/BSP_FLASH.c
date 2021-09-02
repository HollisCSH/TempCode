/****************************************************************************/
/* 	File    	BSP_FLASH.c 			 
 * 	Author		Hollis
 *	Notes		使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_FLASH.h"
#include "BSP_UART.h"
#include "stdio.h"
#include "Comm.h"

/* 私有宏定义 ----------------------------------------------------------------*/
#define SWAP8(value) (uint64_t)(((value& 0x0F) << 4) | ((value & 0xF0) >> 4))
/* 私有变量 ------------------------------------------------------------------*/
static FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t startAddress;
uint32_t endAddress;
uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0, PageError = 0;
/* 扩展变量 ------------------------------------------------------------------*/
/* 函数声明 ------------------------------------------------------------------*/

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
static uint32_t GetBank(uint32_t Addr)
{
    if(Addr < (FLASH_BASE + FLASH_BANK_SIZE) )
    return FLASH_BANK_1;
    else
    return FLASH_BANK_2;  
}

/*****************************************************
	* 函数功能:	Flash写入标志
    * 输入参数: 写入地址 Address , 写入数据 Data , 写入长度 Size
	* 返 回 值: 返回值为0表示写成功
	* 说    明：
******************************************************/
uint8_t WriteFlagToFlash(uint32_t Address , uint64_t Data)
{
    uint8_t buf[8];
    for(int i = 0 ; i < 8; i++)
    {
        buf[i] =0xFF & (Data>>(8*i));
    }
    return WriteDataToFlash(Address , buf , 8);
}

/*****************************************************
	* 函数功能:	Flash写入数据
    * 输入参数: 写入地址 Address , 写入数据指针 *Data , 写入长度 Size
	* 返 回 值: 返回值为0表示写成功
	* 说    明：
******************************************************/
uint8_t WriteDataToFlash(uint32_t Address , uint8_t *Data , uint8_t DataLen)//写数据为固定长度 
{
    uint64_t vl_data[256]	;											    //一次擦除操作2K字节  每次读出8个字节  需要256个
    uint32_t vl_start_addr;
    uint16_t i ;
    #ifdef DEBUG
    _Debug_Printf("FlashW addr:0x%08X len:%d\n",Address,DataLen);
    #endif
    if((Address < 0x08000000) | (Address > 0x08040000) | (DataLen >128 )) 	//地址不符合要求
    return 1;
    vl_start_addr 	= Address / FLASH_PAGE_SIZE * FLASH_PAGE_SIZE;          //取出目标页的首地址
    //vl_offset_addr 	= (Address - vl_start_addr)/8;					    //取出写数据的偏移量

    for( i = 0; i<256 ; i++)										        //读取该页内的原数据
    {
        vl_data[i] = ReadFlagFmFlash(vl_start_addr + i*FLASH_WEITE_BYTE_SIZE);  		
    }
    StringCopy(((uint8_t *)vl_data)+Address-vl_start_addr,Data,DataLen);
    HAL_FLASH_Unlock();
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
        #ifdef DEBUG
        _Debug_Printf("Flash Erase err:0x%08X \n",Address);
        #endif
        HAL_FLASH_Lock();
        return 2;
    }
    for( i = 0; i<256 ; i++)
    {
        //if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, vl_start_addr + i*FLASH_WEITE_BYTE_SIZE, byte_to_doubleword(&vl_data[8*i])) != HAL_OK)
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, vl_start_addr + i*FLASH_WEITE_BYTE_SIZE, vl_data[i]) != HAL_OK)
        {
            //Error_Handler();#ifdef DEBUG
            #ifdef DEBUG
            _Debug_Printf("Flash Prog err:0x%08X \n",Address);
            #endif
            HAL_FLASH_Lock();
            return 3;
        }		
    }
    HAL_FLASH_Lock();
    return 0;
}
/*****************************************************
	* 函数功能:	升级时FLASH更新数据
    * 输入参数: 写入地址 Address , 写入数据指针 *Data , 写入长度 Size
	* 返 回 值: 返回值为0表示写成功
	* 说    明：只为升级使用，非通用函数
******************************************************/
uint8_t UpDataToFlash(uint32_t Address , uint8_t *Data , uint8_t DataLen)//写数据为固定长度 
{
    uint64_t *pData = (uint64_t *)Data;
    uint16_t i ;
//    uint64_t vl_data[16]	;											    //一次擦除操作2K字节  每次读出8个字节  需要256个
    #ifdef DEBUG
    _Debug_Printf("UpData addr:0x%08X len:%d\n",Address,DataLen);
    #endif
    if((Address < 0x08000000) | (Address > 0x08040000) | (DataLen >128 )) 	//地址不符合要求
    return 1;
    
//	for( i = 0; i<DataLen/8 ; i++)							//修改该页目标地址的数据
//	{
//		vl_data[i] =  ((uint64_t)Data[8*i]) | ((uint64_t)Data[8*i+1])<<8 
//                    | ((uint64_t)Data[8*i+2])<<16 | ((uint64_t)Data[8*i+3])<<24 
//                    | ((uint64_t)Data[8*i+4])<<32 | ((uint64_t)Data[8*i+5])<<40 
//                    | ((uint64_t)Data[8*i+6])<<48 | ((uint64_t)Data[8*i+7])<<56 ;
//	}
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR); 
    
//    if(Address == vl_start_addr)                                             //写的地址为页首则擦除整页 
//    {
//        FirstPage = GetPage(vl_start_addr);
//        NbOfPages =  1;
//        BankNumber = GetBank(vl_start_addr);
//        EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
//        EraseInitStruct.Banks       = BankNumber;
//        EraseInitStruct.Page        = FirstPage;
//        EraseInitStruct.NbPages     = NbOfPages;

//        if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
//        {
//            #ifdef DEBUG
//            _Debug_Printf("Flash Erase err:0x%08X \n",Address);
//            #endif
//            HAL_FLASH_Lock();
//            return 2;
//        }
//    }

    for( i = 0; i<DataLen/8 ; i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address + i*FLASH_WEITE_BYTE_SIZE, *pData) != HAL_OK)
        //if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address + i*FLASH_WEITE_BYTE_SIZE, vl_data[i]) != HAL_OK)
        {
            //Error_Handler();#ifdef DEBUG
            #ifdef DEBUG
            _Debug_Printf("Flash Prog err:0x%08X \n",Address);
            #endif
            HAL_FLASH_Lock();
            return 3;
        }		
        pData++;
    }    
    HAL_FLASH_Lock();
    return 0;
}
/*****************************************************
	* 函数功能:	获取flash地址的数据 8字节
    * 输入参数: 数据地址
	* 返 回 值: 
	* 说    明：
******************************************************/
uint64_t ReadFlagFmFlash(uint32_t Address)
{
	uint64_t temp = *(__IO uint64_t*)(Address);
	return temp;
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
        //assert_param(0);    //引发故障源头
    }
    HAL_FLASH_Lock();
}

