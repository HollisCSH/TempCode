//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Storage.h
//创建人  	: Handry
//创建日期	: 
//描述	    : 存储任务头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef __STORAGE_H_
#define __STORAGE_H_

//=============================================================================================
//头文件
//=============================================================================================
//#include "BSP_SPI.h"
#include "BSP_SPI.h"
#include "BSPTypeDef.h"

//=============================================================================================
//宏参数设定
//=============================================================================================

//设备配置占用两个块510~511, Block31
//用户数据占用两个块508~509, Block31
//事件记录概要信息占用两个块506~507, Block31
//事件记录占用20*16个块0~319, Block0~Block19

#define 	SECTOR_SIZE 				4096UL					//一个存储块大小为一个sector 4KByte
#define 	BLOCK_SIZE 					(SECTOR_SIZE * 16UL)	//1 block = 16 sectors = 256 pages
#define 	MEM_SIZE 					(BLOCK_SIZE * 32UL)		//总共大小：32 block
#define 	PAGE_SIZE 					(256UL)					//1 page = 256bytes

#define 	RECORD_SIZE 				(128UL)

#define 	CFG_START_SECTOR 			510UL
#define 	CFG_START_ADDR 				(CFG_START_SECTOR*SECTOR_SIZE)

#define 	USER_START_SECTOR 			508UL
#define 	USER_START_ADDR 			(USER_START_SECTOR * SECTOR_SIZE)

#define 	RECORD_INF_START_SECTOR 	506UL
#define 	RECORD_INF_START_ADDR 		(RECORD_INF_START_SECTOR * SECTOR_SIZE)
//////////////////////////////////////////////////////
#define 	SOC_START_SECTOR 			498UL                   //测试SOC算法使用 沿用M4/M6使用8个扇区 498 - 505
#define 	SOC_START_ADDR 				(SOC_START_SECTOR * SECTOR_SIZE)
#define     SOC_END_SECTOR 			    505UL                   //测试SOC算法使
#define     SOC_END_ADDR 			    ((SOC_END_SECTOR+1) * SECTOR_SIZE) //结束地址
//////////////////////////////////////////////////////

#define 	RECORD_START_SECTOR 		0UL
#define 	RECORD_START_ADDR 			(RECORD_START_SECTOR * SECTOR_SIZE)

#define 	RECORD_BLOCK_NUM 			(20UL)
#define 	RECORD_SECTOR_NUM 			(RECORD_BLOCK_NUM * 16UL)
#define 	RECORD_END_ADDR 			(RECORD_START_ADDR + RECORD_SECTOR_NUM*SECTOR_SIZE - 1UL)
#define 	RECORD_MAX_ITEM 			(RECORD_SECTOR_NUM * SECTOR_SIZE / RECORD_SIZE)//可存储的事件最大条目数（10240）
#define	 	RECORD_MAX_NUM 				(0x1fffffffUL - 0x1fffffffUL % RECORD_MAX_ITEM)//事件编号最大数目（536862720）

#define 	FLASH_WR_CFG_MASK 			0x01    //配置参数存储
#define 	FLASH_WR_UD_MASK 			0x02    //写用户数据
#define 	FLASH_WR_EVE_MASK 			0x04    //写事件记录
#define 	FLASH_RD_EVE_MASK 			0x08    //读事件记录
#define 	FLASH_CLR_EVE_MASK 			0x10    //清除事件记录
#define 	FLASH_POW_TEST_MASK 		0x20    //电源测试

#define     FLASH_SOC_WR_MASK           0x40    //SOC数据写入存储,测试soc算法使用，以后可以屏蔽
#define 	FLASH_POW_OFF_MASK 		    0x80    //电源关闭控制
#define     FLASH_SOC_CLR_MASK          0x100   //SOC清除数据

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern u8 gStorageErr;				//存储故障计数
extern u8 gStorageBuff[];
extern u16 gStorageCTRL;
extern u8 gFlashPowSta;            //Flash上电状态：0：未上电；1：已上电
extern u8 gPowOffTim;              //关闭FLASH延时定时器

//=============================================================================================
//声明接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void StorageInit(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 存储任务初始化函数
//注	意	:
//=============================================================================================
void StorageInit(void);

//=============================================================================================
//函数名称	: void StoragePowerOff(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 存储关闭
//注	意	:
//=============================================================================================
void StoragePowerOff(void);

//=============================================================================================
//函数名称	: void StoragePowerOn(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 存储开启，供电
//注	意	:
//=============================================================================================
void StoragePowerOn(void);

//=============================================================================================
//函数名称	: void StorageWriteFlashWait(u32 addr,u8 *writebuf,u32 len)
//函数参数	: addr：写入的地址；*writebuf:写入数据的数组指针；len：写入的长度
//输出参数	: void
//静态变量	: void
//功	能	: 读取flash函数
//注	意	:
//=============================================================================================
void StorageWriteFlashWait(u32 addr,u8 *writebuf,u32 len);

//=============================================================================================
//函数名称	: void StorageReadFlash(u32 addr,u8 *readbuf,u32 len)
//函数参数	: addr：读取的地址；*readbuf:读取数据放置的数组指针；len：读取的长度
//输出参数	: void
//静态变量	: void
//功	能	: 读取flash函数
//注	意	:
//=============================================================================================
void StorageReadFlash(u32 addr,u8 *readbuf,u32 len);

//=============================================================================================
//函数名称	: u8 StorageStringCopy(const u8 *msg1, u8 *msg2, u16 cnt)
//输入参数	: dest：目标字符串地址； *sour：源字符串地址；cnt：复制的长度
//输出参数	: void
//函数功能	: 字符串复制
//注意事项	:
//=============================================================================================
void StorageStringCopy(u8 *dest, u8 *sour, u16 cnt);

//=============================================================================================
//函数名称	: u8 StorageStringComp(const u8 *msg1, const u8 *msg2, u16 cnt)
//输入参数	: msg1：比较字符串1；msg2：比较的字符串2；cnt：比较的长度
//输出参数	: TRUE：比较正确；FALSE：比较错误
//函数功能	: 字符串比较
//注意事项	:
//=============================================================================================
u8 StorageStringComp(const u8 *msg1, const u8 *msg2, u16 cnt);

//=============================================================================================
//函数名称	: void StorageMainTask(void *p)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 存储任务函数
//注	意	:
//=============================================================================================
void StorageMainTask(void *p);

#endif

/*****************************************end of Storage.h*****************************************/
