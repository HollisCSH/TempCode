/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __BIT__H_
#define __BIT__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "BSPTypeDef.h"

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

//Register operations
#define REG32(addr)                  (*(volatile uint32_t *)(uint32_t)(addr))
#define REG16(addr)                  (*(volatile uint16_t *)(uint32_t)(addr))
#define REG8(addr)                   (*(volatile uint8_t *)(uint32_t)(addr))

/* bit operations */
#define BIT(x)                       ((uint32_t)((uint32_t)0x01U<<(x)))
#define BITS(start, end)             ((0xFFFFFFFFUL << (start)) & (0xFFFFFFFFUL >> (31U - (uint32_t)(end)))) 
#define GET_BITS(regval, start, end) (((regval) & BITS((start),(end))) >> (start))

#define AS_UINT8(_pPointer) 					 (*((uint8_t*)&(_pPointer)))
#define AS_UINT16(_msByte, _lsByte) 			 (uint16_t)((((_msByte) << 8)) | (_lsByte))
#define AS_UINT32(_Byte3, _Byte2,_Byte1, _Byte0) (uint32_t)(((_Byte3) << 24) | ((_Byte2) << 16) | ((_Byte1) << 8) | (_Byte0))

//#define SWAP16(value) (uint16_t)(((value) << 8) | (((value) >> 8) & 0xFF))
//#define SWAP32(value) (uint32_t)((((uint8_t*)&(value))[0] << 24) | (((uint8_t*)&(value))[1] << 16) | (((uint8_t*)&(value))[2] << 8) | ((uint8_t*)&(value))[3])

#define UINT32_MEM_BYTE(asUint32_t, byteIndex) (((uint8_t*)&(asUint32_t))[byteIndex])

#ifdef BIG_ENDIAN
	#define N2H16(value) (value)
	#define N2H32(value) (value)

	#define H2N16(value) (value)
	#define H2N32(value) (value)
#else
	#define N2H16(value) SWAP16(value)
	#define N2H32(value) SWAP32(value)

	#define H2N16(value) SWAP16(value)
	#define H2N32(value) SWAP32(value)
#endif

	int Uint32_t_getMaskBit(uint32_t bitMask);
	uint16_t bigendian16_get(uint8_t *msg);//?ú×?·?′??Dìáè?′ó???￡ê?μ?16??êy?Y
	void bigendian16_put(uint8_t *msg, uint16_t data);//?ò×?·?′??Dê?3?′ó???￡ê?μ?16??êy?Y


#ifdef __cplusplus
}
#endif

#endif

