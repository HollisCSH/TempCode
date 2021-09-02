/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __SWTIMER_H_
#define __SWTIMER_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "BSPTypeDef.h"

extern volatile unsigned int system_ms_tick;

typedef struct _SwTimer
{
	Bool	m_isStart;
	uint32_t 	m_InitTicks;
	uint32_t 	m_TimeOutTicks;
	uint32_t 	m_Id;
}SwTimer;

void SwTimer_Init(SwTimer* pTimer, uint32_t timeOutMs, uint32_t nId);
void SwTimer_ReStart(SwTimer* pTimer);
void SwTimer_Start(SwTimer* pTimer, uint32_t timeOutMs, uint32_t nId);
void SwTimer_Stop(SwTimer* pTimer);
void SwTimer_StopEx(SwTimer* pTimer, uint32 timeId)	;
Bool SwTimer_isTimerOut(SwTimer* pTimer);
Bool SwTimer_isTimerOutEx(uint32_t initTicks, uint32_t timeOutTicks);
Bool SwTimer_isTimerOutId(SwTimer* pTimer, uint32 timeId);
Bool SwTimer_IsStart(SwTimer* pTimer);
Bool SwTimer_isTimerOut_onId(SwTimer* pTimer, uint32_t timeId);


#ifdef __cplusplus
}
#endif

#endif

