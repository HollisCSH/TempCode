
/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __CHECK_SUM__H_
#define __CHECK_SUM__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "BSPTypeDef.h"

uint16_t CheckSum_Get(uint16_t* pCheckSum, const void* pData, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif


