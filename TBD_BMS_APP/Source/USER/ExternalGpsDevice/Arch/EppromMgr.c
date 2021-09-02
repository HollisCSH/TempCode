/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "Archdef.h"
#include "EpproMgr.h"
#if 0
void EpproMgr_Dump(const SectorMgr* pSectorMgr)
{
	Printf("SectorMgr(@%x) Dump:\n", pSectorMgr);
	
//	Printf("\t cfg->Version=%d.\n"		, pSectorMgr->cfg->Version);
	Printf("\t cfg->startAddr=%d.\n"	, pSectorMgr->cfg->startAddr);
	Printf("\t writeOffset=%d.\n"	, pSectorMgr->writeOffset);
	Printf("\t readOffset=%d.\n"	, pSectorMgr->readOffset);
	
	Printf("\t cfg->pItem=%x.\n"	, pSectorMgr->cfg->storage);
	Printf("\t cfg->itemSize=%d.\n"	, pSectorMgr->cfg->storageSize);
	Printf("\t itemCount=%d.\n"	, pSectorMgr->itemCount);
}
#endif
Bool EpproMgr_Test()
{
//#define BUF_SIZE 512
//	int i = 0;
//	Bool bRet = False;
//	static uint8 tempBuf[BUF_SIZE];
//	static uint8 buf1[BUF_SIZE];
//	SectorMgr sector[2];
//
//	const SectorCfg cfg =
//	{
//		.startAddr = buf1,
//		.sectorSize = 512,
//		.pItem = 512,
//	}
//
//	for(i = 0; i < sizeof(buf1); i++)
//	{
//		buf1[i]= i;
//	}
//
//	SectorMgr_init(sector, 512, 0, tempBuf, sizeof(buf1));
//
//	for(i = 0; i < (sector->cfg->sectorSize / BUF_SIZE); i++)
//	{
//		SectorMgr_WriteEx(sector, buf1);
//	}

	
	return False;
}

//数据块是否被写过
Bool EpproMgr_isWritten(uint8* pByte, int len)
{
	int i = 0;

	for(i = 0; i < len; i++)
	{
		//数据中有一个非0xFF值，说明该区块已经被写过
		if(pByte[i] != 0xFF)
		{
			return True;
		}
	}

	return False;
}

void EpproMgr_CalcOffset(SectorMgr* pSector)
{
	uint16 offset = 0;
	uint8* readBuf = pSector->cfg->exchangeBuf;
	uint32 itemsPerSector = pSector->cfg->sectorSize / pSector->cfg->storageSize;

	//先检测最后一个Item是否已经被写过
	offset = (itemsPerSector - 1) * pSector->cfg->storageSize;
	pSector->cfg->Read(pSector->cfg->startAddr + offset, readBuf, pSector->cfg->storageSize);
	if (SectorMgr_isWritten(readBuf, pSector->cfg->storageSize))
	{
		pSector->writeOffset = 0;
		pSector->readOffset = offset;
		pSector->itemCount = itemsPerSector;
		return;
	}

	//检测第一个位置是否已经被写过
	pSector->cfg->Read(pSector->cfg->startAddr, readBuf, pSector->cfg->storageSize);
	if(!SectorMgr_isWritten(readBuf, pSector->cfg->storageSize))
	{
		pSector->writeOffset = 0;
		pSector->readOffset = 0;
		pSector->itemCount = 0;
		return;
	}

	//定位可写范围
	int writeInd = 0;
	int unWriteInd = 0;
	int mid;
	int minInd = 0;
	int maxInd = itemsPerSector;
	for (; (maxInd - minInd) != 1;)
	{
		mid = (maxInd + minInd) >> 1;//mid是n/2；
		pSector->cfg->Read(pSector->cfg->startAddr + mid * pSector->cfg->storageSize, readBuf, pSector->cfg->storageSize);
		if (SectorMgr_isWritten(readBuf, pSector->cfg->storageSize))
		{
			minInd = mid;
			writeInd = mid;
		}
		else
		{
			maxInd = mid;
			unWriteInd = mid;
		}
	}

	//找出可写位置
	writeInd *= pSector->cfg->storageSize;
	unWriteInd *= pSector->cfg->storageSize;
	for (int i = writeInd; i <= unWriteInd; i += pSector->cfg->storageSize)
	{
		pSector->cfg->Read(pSector->cfg->startAddr + i, readBuf, pSector->cfg->storageSize);
		if (!SectorMgr_isWritten(readBuf, pSector->cfg->storageSize))
		{
			pSector->readOffset = i - pSector->cfg->storageSize;
			pSector->writeOffset = i;
			pSector->itemCount = i / pSector->cfg->storageSize;
			break;
		}
	}
}

//在扇区空闲部位写入数据，如果发现Flash某一位置损坏，则自动跳过去.
Bool EpproMgr_WriteEx(SectorMgr* pSector, const void* pData)
{
	uint8 startWriteoffset = pSector->writeOffset;
	uint8 readBuf[MAX_ITEM_SIZE];
	
	//写到新位置上
	while(True)
	{
		if(0 == pSector->writeOffset)
		{
			//检查该扇区是否被写过，如果是，执行擦除操作
			for (int i = 0; i < pSector->cfg->sectorSize; i += MAX_ITEM_SIZE)
			{
				if (pSector->cfg->Read(pSector->cfg->startAddr + i, readBuf, MAX_ITEM_SIZE)
					&& SectorMgr_isWritten(readBuf, MAX_ITEM_SIZE))
				{
					SectorMgr_Erase(pSector);
					break;
				}
			}
		}
		
		//写数据
		if(!pSector->cfg->Write(pSector->cfg->startAddr + pSector->writeOffset, (uint8*)pData, pSector->cfg->storageSize)) 
		{
			return False;
		}
		pSector->itemCount++;

		//移动读指针
		pSector->readOffset  = pSector->writeOffset;
		
		//移动写指针,检查可写位置的长度是否超过pSector->cfg->sectorSize
		pSector->writeOffset += pSector->cfg->storageSize;
		if((pSector->writeOffset + pSector->cfg->storageSize) > pSector->cfg->sectorSize)
		{
			pSector->writeOffset = 0;
		}

		//读取写入的数据
		if(!SectorMgr_Read(pSector, readBuf))
		{
			return False;
		}
		
		//通过比较数据判断是否写成功,如果失败，说明Flash可能损坏,再写下一个区块。
		if(memcmp(pData, readBuf, pSector->cfg->storageSize) == 0)
		{
			//比较成功
			break;
		}

		//当两者相等时，说明全部区块都写失败，Flash这个Sector全部损坏。
		if(startWriteoffset == pSector->writeOffset)
		{
			return False;
		}		
	}

	return True;
}

Bool EpproMgr_IsChanged(const SectorMgr* pSector)
{
	return memcmp((void*)(pSector->cfg->startAddr + pSector->readOffset), pSector->cfg->storage, pSector->cfg->storageSize) != 0;
}

Bool EpproMgr_Write(SectorMgr* pSector)
{
	return SectorMgr_WriteEx(pSector, pSector->cfg->storage);
}

//读取数据
int EpproMgr_Read(SectorMgr* pSector, void* pBuff)
{
	if (!pSector->cfg->Read(pSector->cfg->startAddr + pSector->readOffset, pBuff, pSector->cfg->storageSize)) return False;

	return pSector->cfg->storageSize;
}

Bool EpproMgr_ReadItem(SectorMgr* pSector, int itemInd, void* pBuff)
{
	//uint32 offset = itemInd * pSector->cfg->itemSize;

	if(itemInd >= (int)pSector->itemCount)
		return False;	//Offset is invalid.

	if (!pSector->cfg->Read(pSector->cfg->startAddr + pSector->readOffset, pBuff, pSector->cfg->storageSize)) return False;

	return True;
}

Bool EpproMgr_isFull(SectorMgr* pSector)
{
	return (pSector->itemCount == pSector->cfg->sectorSize / pSector->cfg->storageSize);
}

Bool EpproMgr_isEmpty(SectorMgr* pSector)
{
	return (pSector->itemCount == 0);
}

void EpproMgr_Erase(SectorMgr* pSector)
{
	pSector->writeOffset = 0;
	pSector->readOffset  = 0;
	pSector->itemCount   = 0;
	pSector->cfg->Erase(pSector->cfg->startAddr, pSector->cfg->sectorSize);
}

void EpproMgr_Reset(SectorMgr* pSector)
{
	pSector->writeOffset = 0;
	pSector->readOffset = 0;
	pSector->itemCount = 0;
}

Bool EpproMgr_Copy(SectorMgr* pDst, const SectorMgr* pSrc, uint8* pageBuf, int bufLen)
{
	uint32 offset = 0;
	
	if(!SectorMgr_isEmpty(pDst))
	{
		SectorMgr_Erase(pDst);
	}

	for(offset = 0; offset < pDst->cfg->sectorSize; offset += bufLen)
	{
		pSrc->cfg->Read (pSrc->cfg->startAddr + offset, pageBuf, bufLen);
		pDst->cfg->Write(pDst->cfg->startAddr + offset, pageBuf, bufLen);
	}

	SectorMgr_CalcOffset(pDst);
	return True;
}

////初始化一个满的扇区
//void SectorMgr_initFullSector(SectorMgr* pSector, uint32 startAddr, uint16 itemSize)
//{
//	pSector->cfg->cfg->startAddr = startAddr;
//	pSector->cfg->cfg->itemSize	 = itemSize;
//	
//	pSector->writeOffset = 0;
//	pSector->itemCount   = pSector->cfg->cfg->sectorSize/itemSize;
//	pSector->readOffset  = (pSector->itemCount - 1) * itemSize ;
//}

Bool EppromMgr_init(SectorMgr* pSector, const SectorCfg* cfg)
{	
	memset(pSector, 0, sizeof(SectorMgr));
	pSector -> cfg = cfg;
	
	if(cfg->startAddr % cfg->sectorSize) return False;	//startAddr必须要被pSector->cfg->sectorSize整除
	if(cfg->storageSize > cfg->sectorSize) return False;	//dataLength必须小于pSector->cfg->sectorSize
	
	SectorMgr_CalcOffset(pSector);
//	Assert(cfg->exchangeBufSize >= cfg->storageSize);

	if(pSector->itemCount > 0 && cfg->storage)
	{
		//读取第一项内容，检验数据是否合法
		if (SectorMgr_Read(pSector, cfg->exchangeBuf))
		{
			if (cfg->Verify && !cfg->Verify(cfg->exchangeBuf, cfg->storageSize))
			{
				SectorMgr_Erase(pSector);
				return False;
			}
			memcpy(cfg->storage, cfg->exchangeBuf, cfg->storageSize);
		}
	}

	return True;
}

