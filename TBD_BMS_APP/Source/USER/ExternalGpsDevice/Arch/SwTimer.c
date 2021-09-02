/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "SwTimer.h"

//system_ms_tick must be modify in MS timer of system
//volatile unsigned int system_ms_tick;
extern volatile unsigned int system_ms_tick ;
void SwTimer_Init(SwTimer* pTimer, uint32_t timeOutMs, uint32_t nId)
{
	memset(pTimer, 0, sizeof(SwTimer));
	
	pTimer->m_TimeOutTicks  = timeOutMs;
	pTimer->m_Id 			= nId;
}

void SwTimer_Start(SwTimer* pTimer, uint32_t timeOutMs, uint32_t nId)
{
	pTimer->m_Id 			= nId;
	pTimer->m_TimeOutTicks 	= timeOutMs;
	
	pTimer->m_InitTicks = GET_TICKS();
	pTimer->m_isStart 	= True;
}

Bool SwTimer_isTimerOutEx(uint32_t initTicks, uint32_t timeOutTicks)
{
	uint32_t totalTicks = 0;
	uint32_t newTicks = GET_TICKS();
	
	if(newTicks < initTicks)
	{
		//tick counter overflow
		totalTicks = 0xFFFFFFFF - initTicks + newTicks;
	}
	else
	{
		totalTicks = newTicks - initTicks;
	}

	return (totalTicks >= timeOutTicks);
}

Bool SwTimer_isTimerOutId(SwTimer* pTimer, uint32 timeId)
{
	return (timeId == pTimer->m_Id && SwTimer_isTimerOut(pTimer));
}

Bool SwTimer_isTimerOut(SwTimer* pTimer)
{
	if(!pTimer->m_isStart) 
		return False;
	
	if(SwTimer_isTimerOutEx(pTimer->m_InitTicks, pTimer->m_TimeOutTicks))
	{
		pTimer->m_isStart = False;
		return True;
	}

	return False;
}

Bool SwTimer_isTimerOut_onId(SwTimer* pTimer, uint32_t timeId)
{
	if(pTimer->m_Id == timeId)
	{
		return SwTimer_isTimerOut(pTimer);
	}
	else
	{
		return False;
	}
	
}
Bool SwTimer_IsStart(SwTimer* pTimer)
{
	return pTimer->m_isStart;
}

void SwTimer_Stop(SwTimer* pTimer)	
{
	pTimer->m_isStart = False ;
}

//根据timeid控制定时器计数停止
void SwTimer_StopEx(SwTimer* pTimer, uint32 timeId)	
{
	if(timeId == pTimer->m_Id)
	{
		pTimer->m_isStart = False ;
	}
}

void SwTimer_ReStart(SwTimer* pTimer)
{
	if(pTimer->m_TimeOutTicks == 0) 
		return;
	pTimer->m_InitTicks = GET_TICKS(); 
	pTimer->m_isStart = True;
}

