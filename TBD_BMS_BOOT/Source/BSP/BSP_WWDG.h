/****************************************************************************/
/* 	File    	BSP_WWDG.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_WWDG_H__
#define __BSP_WWDG_H__

#include <stdint.h>
#include "main.h"

void MX_WWDG_Init(void);
void  FeedWDog(void);
#endif /* __BSP_WWDG_H__ */
