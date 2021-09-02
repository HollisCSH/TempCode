/****************************************************************************/
/* 	File    	BSP_PWR.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_PWR_H__
#define __BSP_PWR_H__

#include <stdint.h>
#include "main.h"

/*****************************************************
	* 函数功能: 停止模式
	* 输入参数: 
	* 返 回 值: 无
	* 说    明：G0系列单片机要进入低功耗睡眠模式，需先降频进入低功耗运行模式，退出至正常模式也需要一样的流程。
******************************************************/
void MCU_INTO_STOP_MODE(void);

void BSPLowPowInit(void);
#endif /* __BSP_PWR_H__ */
