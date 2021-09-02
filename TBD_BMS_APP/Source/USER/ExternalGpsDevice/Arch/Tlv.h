/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __TLV__H_
#define __TLV__H_

#ifdef __cplusplus
extern "C"{
#endif

#include "BSPTypeDef.h"
#include "DataType.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
#define TLV_HEAD_SIZE 2 //头包括 tag和len

//=============================================================================================
//定义数据类型
//=============================================================================================
//tlv数据包类型
typedef struct _Tlv
{
	uint8 tag;
	uint8 len;
	uint8 data[1];
}Tlv;

//=============================================================================================
//声明接口函数
//=============================================================================================

//TLV数据包初始化，tlv数据命令，长度，数据域
void Tlv_Init(Tlv* pTlv, uint8 tag, uint8 len, const uint8* pData);

//将TLV数据包附加在pBytes数组len长度的后面，数组大小maxlen
int Tlv_Append(uint8* pBytes, int len, int maxLen, const Tlv* pNewTlv);

//在一个数组中，根据Tag命令索引寻找tlv的地址指针
Tlv* Tlv_GetByTag(const uint8* pByte, int len, uint8 tag);

	void Tlv_dump(uint32 tag, uint8 tagLen, int len, const uint8* val, DType dt);
	void Tlvs_dump(const uint8* tlvs, uint8 tlvsLen, int tagLen);

#ifdef __cplusplus
}
#endif

#endif
