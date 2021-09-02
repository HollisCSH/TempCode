/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "Tlv.h"
#include "TlvIn.h"


void TlvInMgr_dump(const TlvIn* pItem, int tagLen, DType dt)
{
//	Printf("%s ", pItem->name);
//	Tlv_dump(pItem->tag, tagLen, pItem->len, pItem->storage, dt);
//	Printf("\n");
}

const TlvIn* TlvInMgr_find(const TlvIn* pItems, int count, uint32 tag)
{
	for(int i = 0; i < count; i++, pItems++)
	{
		if(pItems->tag == tag) return pItems;
	}
	
	return Null;	
}

void TlvInMgr_resetAll(TlvInMgr* mgr)
{
	const TlvIn* p = mgr->itemArray;
	for(int i = 0; i < mgr->itemCount; i++, p++)
	{
		memset(p->storage, 0, p->len);
	}
}

TlvInEventRc TlvInMgr_event(TlvInMgr* mgr, const TlvIn* p, TlvInEvent ev)
{
	TlvInEventRc rc = TERC_SUCCESS;

	if (mgr->Event) 
		rc = mgr->Event(mgr->pObj, p, ev);

	if (rc == TERC_SUCCESS && p->Event)
		rc = p->Event(mgr->pObj, p, ev);

	return rc;
}

//更新存储指针值
void TlvInMgr_updateStorage(TlvInMgr* mgr, const uint8* pTlvBuf, int bufSize)
{
	uint8 buf[8];
	const TlvIn* p = Null;
	const uint8* pVal = Null;
	uint32 tag = 0;
	for (int i = 0; i + mgr->tagLen < bufSize; )
	{
		memcpy(&tag, pTlvBuf, mgr->tagLen);

		p = TlvInMgr_find(mgr->itemArray, mgr->itemCount, tag);
		
		if (p)
		{
			int len = MIN(p->len, pTlvBuf[mgr->tagLen]);
			if (memcmp(p->storage, &pTlvBuf[mgr->tagLen + 1], len) != 0)
			{
				if (p->len > pTlvBuf[mgr->tagLen] ) memset(p->storage, 0, p->len);

				if (TERC_SUCCESS == TlvInMgr_event(mgr, p, TE_CHANGED_BEFORE))
				{
					pVal = &pTlvBuf[mgr->tagLen + 1];
					//大小端转换
					if (mgr->isSwap && p->len <= 8)
					{
						memcpy(buf, pVal, len);
						pVal = Dt_swap(buf, p->dt);
					}

					memcpy(p->storage, pVal, len);

					TlvInMgr_event(mgr, p, TE_CHANGED_AFTER);
					if(p->mirror)
						memcpy(p->mirror, p->storage, p->len);
				}
			}
			TlvInMgr_event(mgr, p, TE_UPDATE_DONE);			
		}
		//i = i + sizeof(TlvIn);
		i += mgr->tagLen + 1 + pTlvBuf[mgr->tagLen];
		pTlvBuf += mgr->tagLen + 1 + pTlvBuf[mgr->tagLen];
	}
}



void TlvInMgr_init(TlvInMgr* mgr, const TlvIn* items, int itemCount, uint8 tagLen, TlvInEventFn Event, Bool isSwap)
{
	mgr->itemArray = items;
	mgr->itemCount = itemCount;
	mgr->Event = Event;
	mgr->tagLen = tagLen;
	mgr->isSwap = isSwap;
}
