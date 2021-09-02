/****************************************************************************/
/* 	File    	BSP_IWDG.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_IWDG_H__
#define __BSP_IWDG_H__

#include <stdint.h>
#include "main.h"

void MX_IWDG_Init(uint32_t rlv);

void FeedIDog(void);
void BSPIDogInit(uint32_t rlv);
void BSPFeedWDogTask(void);
void BSPIDogStart(void);
#endif /* __BSP_IWDG_H__ */
