//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//-----------------------------------------文件信息---------------------------------------------
//文件名   	: BSPTypeDef.h
//创建人  	: Hardry
//创建日期	: 
//描述	    : 公共的头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef _BSPTYPEDEF_H
#define _BSPTYPEDEF_H

//=============================================================================================
//头文件
//=============================================================================================
#include "stdint.h"
#include <stdbool.h>
//============================================================================
//							类型定义
//============================================================================
typedef unsigned char u8;
typedef signed char s8;
//typedef unsigned int u16;
//typedef signed int s16;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned long u32;
typedef unsigned long long u64;
typedef signed long s32;

typedef unsigned char uint8;
typedef char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef volatile unsigned int vuint32;
//typedef int int32;
typedef uint16 wchar;

typedef char BOOLEAN;
typedef char int8;
//typedef int  int16;
typedef short int16;
typedef long  int32;

//#define BOOLEAN u8;

//============================================================================
//								宏参数定义
//============================================================================
#define 	DEBUG_EN    1

#define     True 	    1
#define     False 	    0
#define     Null	    0
#ifndef 	TRUE
#define 	TRUE		1
#endif

#ifndef 	FALSE
#define 	FALSE		0
#endif

#ifndef 	NULL
#define 	NULL        0//((void *) 0)
#endif

/* BITS */
#define BIT_0		0x00000001UL
#define BIT_1		0x00000002UL
#define BIT_2		0x00000004UL
#define BIT_3		0x00000008UL
#define BIT_4		0x00000010UL
#define BIT_5		0x00000020UL
#define BIT_6		0x00000040UL
#define BIT_7		0x00000080UL
#define BIT_8		0x00000100UL
#define BIT_9		0x00000200UL
#define BIT_10		0x00000400UL
#define BIT_11		0x00000800UL
#define BIT_12		0x00001000UL
#define BIT_13		0x00002000UL
#define BIT_14		0x00004000UL
#define BIT_15		0x00008000UL
#define BIT_16		0x00010000UL
#define BIT_17		0x00020000UL
#define BIT_18		0x00040000UL
#define BIT_19		0x00080000UL
#define BIT_20		0x00100000UL
#define BIT_21		0x00200000UL
#define BIT_22		0x00400000UL
#define BIT_23		0x00800000UL
#define BIT_24		0x01000000UL
#define BIT_25		0x02000000UL
#define BIT_26		0x04000000UL
#define BIT_27		0x08000000UL
#define BIT_28		0x10000000UL
#define BIT_29		0x20000000UL
#define BIT_30		0x40000000UL
#define BIT_31		0x80000000UL

#define Bool uint8

//#define inline __inline 
#define _FUNC_ __FUNCTION__ 
#define _FILE_ __FILE__ 
#define _LINE_ __LINE__ 
//============================================================================
//							宏函数定义
//============================================================================
#define 	BITSET(x, y)			((x) |= ((u8)1 << (y)))
#define 	BITCLR(x, y)			((x) &= ~((u8)1 << (y)))
#define 	BITGET(x, y)            ((u8)(((x) & ((u8)1 << (y))) >> (y)))

#define 	BITSET16(x, y)			((x) |= ((u16)1 << (y)))
#define 	BITCLR16(x, y)			((x) &= ~((u16)1 << (y)))
#define 	BITGET16(x, y)	        (((x) & (u16)1 << (y)) >> (y))

#define 	BITSET32(x, y)		    ((x) |= ((u32)1 << (y)))
#define 	BITCLR32(x, y)		    ((x) &= ~((u32)1 << (y)))
#define 	BITGET32(x, y)          ((x & ((u32)1 << (y))) >> (y))
#define 	IsEqual(x, y)           ((x) == (y))
#define 	IsNotEqual(a,b)		    ((a)!=(b))                                  //a不等于b返回1 否则返回0

#define 	Min(v1, v2) 			((v1) > (v2) ? (v2) : (v1))
#define 	Max(v1, v2) 			((v1) < (v2) ? (v2) : (v1))
#define 	Max3(v1, v2, v3) 		Max(Max(v1,v2),v3)

#define 	IsInside(x1, x, x2)     (((x1) <= (x)) && ((x) <= (x2)))
#define 	ABS(x, y)               ((x) > (y) ? ((x) - (y)) : ((y) - (x)))

//大小端转换
#define 	SWAP16(value) 			(uint16)(((value) << 8) | (((value) >> 8) & 0xFF))
#define 	SWAP32(value) 			(uint32)((((uint8*)&(value))[0] << 24) | (((uint8*)&(value))[1] << 16) | (((uint8*)&(value))[2] << 8) | ((uint8*)&(value))[3])



#define UPPER_BYTE(x)       ((x >> 8) & 0xff)
#define LOWER_BYTE(x)       (x & 0xff)
#define UPPER_WORD(x)       ((x >> 16) & 0xffff)
#define LOWER_WORD(x)       (x & 0xffff)
#define UPPER_NIBBLE(x)     ((x >> 4) & 0x0f)
#define LOWER_NIBBLE(x)     (x & 0x0f)

#define     GET_ELEMENT_COUNT(array)(sizeof(array)/sizeof(array[1]))
//调试打印
//extern int  _Printf(const char* lpszFormat, ...);
//#define     cm_printf                _Printf
//#define     printf                  __Printf

#endif

/*****************************************end of BSPTypeDef.h*****************************************/
