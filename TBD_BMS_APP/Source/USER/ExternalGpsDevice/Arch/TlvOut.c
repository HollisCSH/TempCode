/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "TlvOut.h"
#include "SwTimer.h"
#include "bsptypedef.h"


//#define		_TLVOUT_DEBUGMSG		printf
#define		_TLVOUT_DEBUGMSG

void TlvOut_dump(const TlvOut* pItem, int tagLen, DType dt)
{
//	Printf("%s ", pItem->name);
//	Tlv_dump(pItem->tag, tagLen, pItem->len, pItem->storage, dt);
//	Printf("\n");
}

void TlvOut_setFlag(const TlvOut* pItem, TlvOutFlag flag)
{
	pItem->pEx->flag |= flag;
}

void TlvOutMgr_setFlag(TlvOutMgr* mgr, uint32 tag, TlvOutFlag flag)
{
	const TlvOut* p = TlvOutMgr_find(mgr->itemArray, mgr->itemCount, tag);
	if (p)
	{
		TlvOut_setFlag(p, flag);
	}
}

Bool TlvOutMgr_isChanged(TlvOutMgr* mgr, const TlvOut* pItem)
{
	Bool isChanged = (memcmp(pItem->storage, pItem->mirror, pItem->len) != 0);

	if(pItem->pEx)
	{
		isChanged |= (pItem->pEx->flag & TF_OUT_CHANGED); 
		pItem->pEx->flag &= ~TF_OUT_CHANGED;
		//????????????
		if(!SwTimer_isTimerOutEx(pItem->pEx->ticks, 5000))		// 10s
		{
			return False;
		}
		pItem->pEx->timeOutSec++;
		pItem->pEx->ticks = GET_TICKS();
	}
	
	if(isChanged)
	{
		isChanged = pItem->IsChanged ? pItem->IsChanged(mgr, pItem, TE_CHANGED) : isChanged;
	}
	else 
	{
		isChanged = pItem->IsChanged ? pItem->IsChanged(mgr, pItem, TE_UNCHANGED) : isChanged;
	}
	
	if(isChanged && pItem->pEx) 
	{
		pItem->pEx->timeOutSec = 0;
	}
	
	return isChanged;
}

const TlvOut* TlvOutMgr_find(const TlvOut* pItems, int count, uint32 tag)
{
	for(int i = 0; i < count; i++, pItems++)
	{
		if(pItems->tag == tag) return pItems;
	}
	
	return Null;	
}

void TlvOutMgr_resetAll(TlvOutMgr* mgr)
{
	const TlvOut* p = mgr->itemArray;
	for(int i = 0; i < mgr->itemCount; i++, p++)
	{
		memset(p->storage, 0, p->len);
		memset(p->mirror , 0, p->len);
	}
}

/******************************************
函数功能：获取发生变化的TLV，
函数参数：
	mgr：Tlv管理对象指针。
	pBuf：输出缓冲区。
	bufSize：输出缓冲区大小。
	tlvCount：输入输出参数，
		作为输入参数：> 0,表示获取指定数量的TLV，=0表示不指定TLV的个数。
		作为输出参数：获取到的TLV的个数。
返回值：总的有效的TLV数据长度
******************************************/
int TlvOutMgr_getChanged(TlvOutMgr* mgr, uint8* pBuf, int bufSize, uint8* tlvCount)
{
	Bool isChanged = False;
	int offset = 0;
	uint16_t tlv_l = 0 ;
	uint8 remain = (tlvCount) ? *tlvCount : 0;
	if (remain == 0)
	{
		remain = 0xFF; //赋一个足够大的值，相当于忽略remain条件
	}

	int count = 0;
	const TlvOut* p = mgr->itemArray;
	uint8 val[8];
	for(int i = 0; i < mgr->itemCount && remain > 0; i++, p++, remain--)
	{
		//_TLVOUT_DEBUGMSG("Tag:%X-%s-%d\r\n",p->tag , p->name , p->len );
		isChanged = TlvOutMgr_isChanged(mgr, p);
		if(isChanged)
		{
			// 不能直接这么判断，有字符串
			if( p->dt == DT_STRING )
			{
				tlv_l = strlen( p->storage );
			}
			else
			{
				tlv_l = p->len ;
			}
			if((offset + tlv_l/*p->len*/ + mgr->tagLen + 1) > bufSize) break;

			memcpy(&pBuf[offset], &p->tag, mgr->tagLen);
			offset += mgr->tagLen;

			pBuf[offset++] = tlv_l;//p->len;	
			
			//只有小于8个字节的数据才可能是整数，需要大小端转换
			if (mgr->isSwap && tlv_l/*p->len*/ <= sizeof(val))
			{
				memcpy(val, p->storage, tlv_l/*p->len*/);		// 获取的是 storage 不是 miro
				Dt_swap(val, p->dt);
				memcpy(&pBuf[offset], val, tlv_l/*p->len*/);
			}
			else
			{
				memcpy(&pBuf[offset], p->storage, tlv_l/*p->len*/);
			}
			offset += tlv_l/*p->len*/;

			count++;

		}
	}
	
	if (tlvCount) *tlvCount = count;

	return offset;
}

int TlvOutMgr_getValByTag(TlvOutMgr* mgr,uint8* pBuf ,int bufSize ,uint32 tag)
{
	int offset = 0;
	uint16_t tlv_l = 0 ;
	const TlvOut* p = mgr->itemArray;
	uint8 val[8];
	for(int i = 0; i < mgr->itemCount ; i++, p++ )
	{
		if( p->tag == tag )
		{
			// 不能直接这么判断，有字符串
			if( p->dt == DT_STRING )
			{
				tlv_l = strlen( p->storage );
			}
			else
			{
				tlv_l = p->len ;
			}
			if((offset + tlv_l/*p->len*/ + mgr->tagLen + 1) > bufSize) break;
					memcpy(&pBuf[offset], &p->tag, mgr->tagLen);
			offset += mgr->tagLen;
					pBuf[offset++] = tlv_l;//p->len;	
					//只有小于8个字节的数据才可能是整数，需要大小端转换
			if (mgr->isSwap && tlv_l/*p->len*/ <= sizeof(val))
			{
				memcpy(val, p->storage, tlv_l/*p->len*/);		// 获取的是 storage 不是 miro
				Dt_swap(val, p->dt);
				memcpy(&pBuf[offset], val, tlv_l/*p->len*/);
			}
			else
			{
				memcpy(&pBuf[offset], p->storage, tlv_l/*p->len*/);
			}
			offset += tlv_l/*p->len*/;
			return offset ;
		}	
	}
	return 0;
}

//更新镜像指针值

void TlvOutMgr_updateMirror(TlvOutMgr* mgr, const uint8* pTlvBuf, int bufSize)
{
	const TlvOut* p = Null;
	uint32 tag = 0;
	const uint8* pVal;
	int len = 0;
	for(int i = 0; i + mgr->tagLen < bufSize; )
	{
		pVal = &pTlvBuf[mgr->tagLen + 1];

		memcpy(&tag, pTlvBuf, mgr->tagLen);

		p = TlvOutMgr_find(mgr->itemArray, mgr->itemCount, tag);
		if(p == Null) break;

		len = MIN(pTlvBuf[mgr->tagLen], p->len);
		//如果idLen有效，则比较ID信息是否匹配。
		if (p->idLen)
		{
			uint8_t* storage = (uint8*)p->storage;

			//是否包含ID信息（一个TAG对应多个TLV）
			if(memcmp(&storage[p->idInd], &pVal[p->idInd], p->idLen) == 0)
			{
				memcpy(p->mirror, pVal, len);
				if (mgr->isSwap)
				{
					Dt_swap(p->mirror, p->dt);
				}
			}
		}
		else
		{
			memset( p->mirror , 0 , p->len );
			memcpy(p->mirror, pVal, len);
			if (mgr->isSwap)
			{
				Dt_swap(p->mirror, p->dt);
			}
		}
		i += mgr->tagLen + 1 + pTlvBuf[mgr->tagLen] ;
		pTlvBuf += mgr->tagLen + 1 + pTlvBuf[mgr->tagLen];
	}
}



void TlvOutMgr_init(TlvOutMgr* mgr, const TlvOut* items, int itemCount,  int tagLen, Bool isSwap)
{
	mgr->itemArray = items;
	mgr->itemCount = itemCount;
	mgr->tagLen = tagLen;
	mgr->isSwap = isSwap;
}

