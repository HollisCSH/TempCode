/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "Message.h"

const MsgIdNameMatch* g_msgNameTbl;
int g_msgNameTblCount = 0;

//获取消息名称
const char* Msg_GetName(uint8 id)
{
	const MsgIdNameMatch* p = g_msgNameTbl;
	for (int i = 0; i < g_msgNameTblCount; i++, p++)
	{
		if (p->id == id) return p->name;
	}
	return "Unknown";
}

void MsgName_Init(const MsgIdNameMatch* pTbl, int count)
{
	g_msgNameTbl = pTbl;
	g_msgNameTblCount = count;
}

void MsgIf_Init(MsgIf* pMsgIf, MsgProcFun MsgHandler, RunFun Run)
{
	pMsgIf->MsgHandler = MsgHandler;
	pMsgIf->Run = Run;
}

int MsgIf_MsgProc(void* pObj, uint8 msgId, uint32 param1, uint32 param2, const MsgMap* pMsgTbl, int nCount)
{
	int i = 0;
	
	for(i = 0; i < nCount; i++, pMsgTbl++)
	{
		if(msgId == pMsgTbl->m_MsgID)
		{
			return pMsgTbl->MsgHandler((MsgIf*)pObj, param1, param2);
		}
	}

	return -1;
}

