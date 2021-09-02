/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __RECORD_H_
#define __RECORD_H_

#ifdef __cplusplus
extern "C"{
#endif

#ifdef CFG_LOG

#include "SectorMgr.h"
	
#define RECORD_INVALID_ADD 0xFFFFFFFF

typedef Bool(*RecordFilterFn)(void* pItem, uint32 pParam);

typedef struct _RecordCfg
{
	SectorCfg base;

	uint16 sectorCount;
}RecordCfg;

typedef struct _Record
{
	uint32  itemCount;		//有效的元素总数
	uint32 	maxItems;		//最多元素总数，根据扇区大小和元素大小计算出来
	int 	itemsPerSec;	//每个扇区元素数量，根据扇区大小和元素大小计算出来

	uint16 writeSectorInd;		//写扇区号
	uint16 readStartSectorInd;	//读扇区号

	uint32 	readPointer;	//读记录游标指针	

	const RecordCfg* cfg;

	//sector的扇区配置参数
	SectorCfg sectorCfg;
	SectorMgr sector;
}Record;

void Record_Seek(Record* pRecord, uint32 pos);
void Record_Init(Record* pRecord, const RecordCfg* cfg);

Bool Record_Write(Record* pRecord, void* pRecData);
int Record_Read(Record* pRecord, void* pBuf, int buflen);
void Record_Dump(Record* pRecord);
void Record_RemoveAll(Record* pRecord);
uint32 Record_CalcuReadSecAddr(Record* pRecord, uint32 itemInd, uint16* sec, uint32* addrOfSecOffset);
Bool Record_isValid(Record* pRecord);
void Record_ConvertSector(Record* pRecord, uint16* sec, int* ind);

void Record_DumpByCount(Record* pRecord, int count, uint8 moduleId);
void Record_DumpByInd(Record* pRecord, int ind, int count, uint8 moduleId);
void Record_DumpByTimeStr(Record* pRecord, const char* pDataTime, int count);
int Record_Find(Record* pRecord, int startInd, RecordFilterFn verify, uint32 param);
void Record_DumpByTime(Record* pRecord, uint32 seconds, int count);

#endif

#ifdef __cplusplus
}
#endif

#endif


