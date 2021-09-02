/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __MESSAGE__H
#define __MESSAGE__H

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
	typedef struct _MsgIdNameMatch
	{
		uint8	id;
		const char* name;
	}MsgIdNameMatch;

typedef struct _tagMessage
{
    void* 	m_pHandler;
	
	uint32 	m_MsgID;
	uint32	m_Param1;
	uint32	m_Param2;
}Message;

struct _tagMsgIf;
typedef Bool (*MsgPostFun)(struct _tagMsgIf* pReceiver, uint32 msgID, uint32 param1, uint32 param2);
typedef int (*MsgProcFun)(struct _tagMsgIf* pHandler, uint32 msgID, uint32 param1, uint32 param2);
typedef int (*MsgMapFun)(void* pHandler, uint32 param1, uint32 param2);
typedef void (*RunFun)(void* pObj);

//消息映射表
typedef struct _tagMsgMap
{
	uint32	    m_MsgID;
	MsgMapFun	MsgHandler;
}MsgMap;

//消息接口定义
typedef struct _tagMsgIf
{
	MsgProcFun	MsgHandler;
	RunFun		Run;
}MsgIf;

void MsgIf_Init(MsgIf* pMsgIf, MsgProcFun MsgHandler, RunFun Run);
int MsgIf_MsgProc(void* pObj, uint8 msgId, uint32 param1, uint32 param2, const MsgMap* pMsgTbl, int nCount);
const char* Msg_GetName(uint8 id);
void MsgName_Init(const MsgIdNameMatch* pTbl, int count);

#ifdef __cplusplus
}
#endif

#endif
