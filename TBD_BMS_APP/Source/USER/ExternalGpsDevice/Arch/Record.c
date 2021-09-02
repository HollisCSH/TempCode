/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "Record.h"
#include "DateTime.h"

#ifdef CFG_LOG

void Record_Dump(Record* pRecord)
{
	Printf("Record Dump:\n");
	
	Printf("\t startAddr=%d.\n"		, pRecord->cfg->base.startAddr);
	Printf("\t sectorCount=%d.\n"	, pRecord->cfg->sectorCount);
	
	Printf("\t total=%d.\n"			, pRecord->itemCount);
	
	Printf("\t writeSectorInd=%d.\n", pRecord->writeSectorInd);
	Printf("\t readPointer=%d.\n"	, pRecord->readPointer);
	
	SectorMgr_Dump(&pRecord->sector);
}

/*功能:转换相对扇区为实际扇区
参数说明:
	sec:扇区值。
	ind:扇区的相对记录位置
*/
void Record_ConvertSector(Record* pRecord, uint16* sec, int* ind)
{
	int logInd = * ind;
	if(pRecord->readStartSectorInd > *sec)
	{
		logInd -= (pRecord->readStartSectorInd - *sec) * pRecord->itemsPerSec;
	}
	else if(pRecord->readStartSectorInd < *sec)
	{
		logInd += (*sec - pRecord->readStartSectorInd) * pRecord->itemsPerSec;
	}
	*sec = pRecord->readStartSectorInd;

	if(logInd < 0) logInd = 0;
	if(logInd > (int)pRecord->itemCount) logInd = pRecord->itemCount;
	
	*ind = logInd;
}

Bool Record_isValid(Record* pRecord)
{
	return pRecord->itemCount > 0 && pRecord->readPointer < pRecord->itemCount;
}

int Record_GetTotal(Record* pRecord)
{
	return pRecord->itemCount;
}

/*函数功能：计算扇区号和扇区内偏移地址
	参数:itemInd,元素索引位置.
	参数:sec,扇区号.
	参数:addrOfSecOffset,扇区内部偏移地址.
	返回值:绝对地址.
*/
uint32 Record_CalcWriteSecAddr(Record* pRecord, uint32 itemInd, uint16* sec, uint32* addrOfSecOffset)
{
	uint32 addr = 0;
	int offsetSec = 0;
//	const RecordCfg* cfg = pRecord->cfg;

	if(itemInd >= pRecord->maxItems) return RECORD_INVALID_ADD;

	//当前所有扇区都有内容
	if(pRecord->itemCount == pRecord->maxItems)
	{
		offsetSec = (pRecord->writeSectorInd + 1) % pRecord->cfg->sectorCount;
	}
	else if(pRecord->itemCount > pRecord->maxItems - pRecord->itemsPerSec)
	{
		offsetSec = pRecord->writeSectorInd;
	}
	else
	{
		offsetSec = itemInd / pRecord->itemsPerSec;
	}
	
	addr = (itemInd % pRecord->itemsPerSec) * pRecord->sector.cfg->storageSize;

	if(sec) *sec = offsetSec;
	if(addrOfSecOffset) *addrOfSecOffset = addr;
	
	return pRecord->cfg->base.startAddr + (offsetSec * pRecord->sector.cfg->sectorSize) + addr;
}

Bool Record_Write(Record* pRecord, void* pRecData)
{
//	const RecordCfg* cfg = pRecord->cfg;
	uint32 addrOfSec = 0;
	memcpy(pRecord->sector.cfg->storage, pRecData, pRecord->sector.cfg->storageSize);

	if(pRecord->cfg->sectorCount == 0) 	//还没有初始化完毕
		return False;
	
	if(SectorMgr_Write(&pRecord->sector))
	{
		pRecord->itemCount++;
	}
	
	uint16 sectorInd = pRecord->writeSectorInd;
	uint32 addr = Record_CalcWriteSecAddr(pRecord, pRecord->itemCount % pRecord->maxItems, &pRecord->writeSectorInd, &addrOfSec);
	if(addr == RECORD_INVALID_ADD) return False;
	if (pRecord->writeSectorInd != sectorInd)	//当前扇区写满,切换到下一个新的扇区
	{
		//切换到下一个新的扇区
		pRecord->sectorCfg.startAddr = addr;
		SectorMgr_Reset(&pRecord->sector);
		if(pRecord->itemCount >= pRecord->maxItems)
		{
			//擦除新扇区内容
			SectorMgr_Erase(&pRecord->sector);
			//更正有效元素总数
			pRecord->itemCount -= pRecord->itemsPerSec;
			//移动读指针游标，如果当前正指向删除的扇区，要移动到第一个可读位置
			pRecord->readPointer = ((int)pRecord->readPointer > pRecord->itemsPerSec) ? (pRecord->readPointer - pRecord->itemsPerSec) : 0;
		}
		
		//更新读起始扇区号
		Record_CalcuReadSecAddr(pRecord, 0, &pRecord->readStartSectorInd, Null);
	}
	return True;
}

/*函数功能：计算扇区号和扇区内偏移地址
	参数:itemInd,元素索引位置.
	参数:sec,扇区号.
	参数:addrOfSecOffset,扇区内部偏移地址.
	返回值:绝对地址.
*/
uint32 Record_CalcuReadSecAddr(Record* pRecord, uint32 itemInd, uint16* sec, uint32* addrOfSecOffset)
{
	uint32 addr = 0;
	uint8 offsetSec = 0;
//	const RecordCfg* cfg = pRecord->cfg;
	
	if(itemInd >= pRecord->maxItems) return RECORD_INVALID_ADD;

	if(pRecord->itemCount >= pRecord->maxItems - pRecord->itemsPerSec)	//全部扇区的数据都是有效的
	{
		offsetSec = pRecord->writeSectorInd + 1;	//读指针为于写指针的下一个扇区
	}
	else
	{
		offsetSec = 0;
	}
	
	offsetSec = (offsetSec + itemInd / pRecord->itemsPerSec) % pRecord->cfg->sectorCount;
	addr = (itemInd % pRecord->itemsPerSec) * pRecord->sector.cfg->storageSize;

	if(sec) *sec = offsetSec;
	if(addrOfSecOffset) *addrOfSecOffset = addr;

	return pRecord->cfg->base.startAddr + (offsetSec * pRecord->sector.cfg->sectorSize) + addr;
}

void Record_Seek(Record* pRecord, uint32 pos)
{
	pRecord->readPointer = (pos > pRecord->itemCount) ? pRecord->itemCount : pos;
}

int Record_Read(Record* pRecord, void* pBuf, int buflen)
{
	int i = 0;
	int readLen = 0;
	uint8* pByte = (uint8*)pBuf;
	uint32 addr = 0;
	uint16 itemSize = pRecord->sector.cfg->storageSize;

	for(i = pRecord->readPointer
		; i < (int)pRecord->itemCount && buflen >= itemSize
		; i++)
	{
		addr = Record_CalcuReadSecAddr(pRecord, i, Null, Null);
		if(addr == RECORD_INVALID_ADD) break;
		
		pRecord->cfg->base.Read(addr, &pByte[readLen], itemSize);
		readLen += itemSize;
		buflen-= itemSize;
	}

	pRecord->readPointer = i;
	return readLen;
}

void Record_RemoveAll(Record* pRecord)
{
	int i = 0;
	if(pRecord->cfg->sectorCount == 0) 	//还没有初始化完毕
		return;
	uint16 SECTOR_SIZE = pRecord->sector.cfg->sectorSize;
	//擦除所有扇区
	for(i = 0; i < pRecord->cfg->sectorCount; i++)
	{
		pRecord->cfg->base.Erase(pRecord->cfg->base.startAddr + i * SECTOR_SIZE, SECTOR_SIZE);
	}

	//重新初始化扇区管理器
	pRecord->writeSectorInd = 0;
	pRecord->itemCount = 0;
	pRecord->readPointer = 0;
	pRecord->readStartSectorInd = 0;
	SectorMgr_Reset(&pRecord->sector);
}

void Record_Init(Record* pRecord, const RecordCfg* cfg)
{
	int i = 0;
	int writeSecInd = -1;
	SectorCfg* secCfg = &pRecord->sectorCfg;

	memset(pRecord, 0, sizeof(Record));
	pRecord->cfg = cfg;
	memcpy(secCfg, &cfg->base, sizeof(SectorCfg));

	Assert((uint32)cfg->base.startAddr % cfg->base.sectorSize == 0);
	pRecord->itemsPerSec = secCfg->sectorSize / secCfg->storageSize;
	pRecord->maxItems = pRecord->cfg->sectorCount * pRecord->itemsPerSec;

	//遍历所有扇区，统计有效的记录总数, 找出可写扇区。
	SectorMgr* pMgr = &pRecord->sector;
	for(i = 0; i < cfg->sectorCount; i++)
	{
		uint32 addr = secCfg->startAddr + i * secCfg->sectorSize;
		secCfg->startAddr = addr;

		if (!SectorMgr_init(pMgr, secCfg))
		{
			//检验错误
			writeSecInd = -1;
			break;
		}

		//如果中间有一个扇区没有写满，则之后的所有扇区，要不全满，要不全空
		if(!SectorMgr_isFull(pMgr))
		{		
			if(writeSecInd < 0)	//第一个
			{
				writeSecInd = i;
			}
			else if(pMgr->itemCount > 0)	//发生不可预期的错误，删除后面的日志数据。
			{
				cfg->base.Erase(addr, cfg->base.sectorSize);
				continue;
			}
		}
		pRecord->itemCount += pMgr->itemCount;
	}
	
	//找不到可写扇区
	if(writeSecInd == -1) 
	{
		writeSecInd = 0;
		if(pRecord->itemCount)	//发生不可预期的错误
		{
			Record_RemoveAll(pRecord);
		}
	}

	pRecord->writeSectorInd = writeSecInd;
	secCfg->startAddr = secCfg->startAddr + pRecord->writeSectorInd * secCfg->sectorSize;
	SectorMgr_init(&pRecord->sector, &pRecord->sectorCfg);
	Record_CalcuReadSecAddr(pRecord, 0, &pRecord->readStartSectorInd, Null);
}

#endif
