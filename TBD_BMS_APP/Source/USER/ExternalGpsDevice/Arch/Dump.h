/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef  _DUMP_H_
#define  _DUMP_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "BSPTypeDef.h"

#ifdef XDEBUG	
#define DUMP_BYTE(address,len)  DumpByte((uint8_t*)address, len, 32)
#define DUMP_BYTE_STR(str,address,len)  {Printf str; DumpByte((uint8_t*)address, len, 32);}
#define DUMP_BYTE_LEVEL(level, address, len){\
		if(((uint32_t)(level)) & g_dwDebugLevel) \
		{	\
    		DUMP_BYTE(address,len);	\
		}	\
	}
#else
	#define DUMP_BYTE(...)
	#define DUMP_DWORD(...)

	#define DUMP_BYTE_EX(...)
	#define DUMP_BYTE_LEVEL(...)
	#define DUMP_DWORD_LEVEL(...)
	#define DUMP_BYTE_LEVEL_EX(...)
#endif
	
	void DumpByte(const uint8_t* pData, uint16_t len, uint8_t cols);
	void DumpDword(const uint32_t* pData, uint16_t len, uint8_t cols);

#ifdef __cplusplus
}
#endif

#endif

