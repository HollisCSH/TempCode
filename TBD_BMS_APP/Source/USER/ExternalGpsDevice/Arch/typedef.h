/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __TYPEDEF__H_
#define __TYPEDEF__H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>

#ifdef WIN32
typedef unsigned char 	uint8_t;
typedef char 			int8_t;
typedef unsigned short 	uint16_t;
typedef short 			int16_t;
typedef unsigned int 	uint32_t;
typedef volatile unsigned int vuint32_t;
typedef int 			int32_t;
typedef uint16_t 		wchar_t;
typedef unsigned long long uint64_t;
typedef long long 		int64_t;
typedef float 			float32_t;
typedef double 			float64_t;
typedef unsigned char	bool_t;
#else
#include "stdint.h"
#endif
typedef unsigned char 	uint8;
typedef char 			int8;
typedef unsigned short 	uint16;
typedef short 			int16;
typedef unsigned int 	uint32;
typedef volatile unsigned int vuint32;
typedef int 			int32;
//typedef short   		wchar;
typedef unsigned long long uint64;
typedef long long 		int64;
typedef float 			float32;
typedef double 			float64;
typedef bool 			BOOL;
//typedef unsigned char	bool;

#ifdef WIN32
	#ifdef Bool
	#undef Bool
	#endif
	#define Bool uint8_t
#else
    #ifndef Bool
	#define Bool uint8_t
    #endif
#endif

#define True 	1
#define False 	0
#define true 	1
#define false 	0
#define Null	0

#ifdef WIN32
#else

	#ifndef TRUE
	#define TRUE	1
	#endif

	#ifndef FALSE
	#define FALSE	0
	#endif

	#ifndef NULL
	#define NULL	0
	#endif
#endif


#ifdef __cplusplus
}
#endif

#endif

