/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __SECTOR_MGR__H_
#define __SECTOR_MGR__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "BSPTypeDef.h"
/******************************************************************/
#define MAX_ITEM_SIZE 	128		//最大元素字节数

	typedef Bool(*ItemVerifyFn)(const void* pItem, int len);
	typedef Bool(*FmcReadfun)(uint32 addr, void* buf, int len);
	typedef Bool(*FmcWritefun)(uint32 addr, const void* pData, int len);
	typedef void(*FmcErasefun)(uint32 addr, int len);

/******************************************************************/

typedef struct _SectorCfg
{
	uint32 startAddr;		//来自初始化函数，写扇区的起始地址
	uint16 sectorSize;		//来自初始化函数，扇区大小，

	void*  storage;			//来自初始化函数，保存从Flash读取的最新数据，和写入到Flash中的数据。
	uint16 storageSize;		//来自初始化函数，storage大小

	void* exchangeBuf;		//storage的交换缓冲区，长度必须等于storageSize
	uint16 exchangeBufSize;	//交换缓冲区长度

	//元素有效性检验函数，在扇区初始化时从Flash读取数据，判定是否符合当前规则（Flash存储的数据可能时旧版本写入的）
	//如果检验失败，则所有的存储内容被认为是无效的，将会被擦除掉。
	ItemVerifyFn Verify;	

	FmcReadfun  Read;	//Read data from flash
	FmcWritefun Write;	//Write data into flash
	FmcErasefun	Erase;	//Erase data in flash
}SectorCfg;

typedef struct _Sector
{
	uint32 writeOffset;	//扇区内的写偏移地址
	uint32 readOffset;	//扇区内的读偏移地址
	uint32 itemCount;		//有效的Item数量

	const SectorCfg* cfg;
}SectorMgr;

Bool EpproMgr_init(SectorMgr* pSector, const SectorCfg* cfg);
Bool EpproMgr_WriteEx(SectorMgr* pSector, const void* pData);
Bool EpproMgr_Write(SectorMgr* pSector);
int EpproMgr_Read(SectorMgr* pSector, void* pBuff);
void EpproMgr_CalcOffset(SectorMgr* pSector);
Bool EpproMgr_isFull(SectorMgr* pSector);
Bool EpproMgr_isEmpty(SectorMgr* pSector);
Bool EpproMgr_ReadItem(SectorMgr* pSector, int itemInd, void* pBuff);
void EpproMgr_Erase(SectorMgr* pSector);
Bool EpproMgr_Copy(SectorMgr* pDst, const SectorMgr* pSrc, uint8* pageBuf, int bufLen);
void EpproMgr_Dump(const SectorMgr* pSectorMgr);
void EpproMgr_Reset(SectorMgr* pSector);
Bool EpproMgr_IsChanged(const SectorMgr* pSector);

#ifdef __cplusplus
}
#endif

#endif

