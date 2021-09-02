/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __DATA_TYPE_H_
#define __DATA_TYPE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "BSPTypeDef.h"

	//数据类型定义
	typedef enum _DType
	{
		DT_BYTES = 0,	//字节数组
		DT_UINT8 = 1,   //8位无符号整数 
		DT_UINT16,  //16位无符号整数 
		DT_UINT32,  //32位无符号整数 

		DT_INT8,	  //8位有符号整数 
		DT_INT16,   //16位有符号整数 
		DT_INT32,   //32位有符号整数 

		DT_FLOAT32,	//32位浮点数
		DT_FLOAT64,	//64位浮点数

		DT_STRING,	//字符串
		DT_STRUCT,	//结构体
	}DType;

	typedef uint32 (*DtConvertFn)(const void* val, DType dt);
	uint32 Dt_convertToU32(const void* val, DType dt);
	uint16 Dt_convertToU16(const void* val, DType dt);
	uint8  Dt_convertToU8(const void* val, DType dt);

	//大小端位序转换
	void* Dt_swap(uint8* val, DType dt);

#ifdef __cplusplus
}
#endif

#endif
