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
//������趨
//=============================================================================================
#define TLV_HEAD_SIZE 2 //ͷ���� tag��len

//=============================================================================================
//������������
//=============================================================================================
//tlv���ݰ�����
typedef struct _Tlv
{
	uint8 tag;
	uint8 len;
	uint8 data[1];
}Tlv;

//=============================================================================================
//�����ӿں���
//=============================================================================================

//TLV���ݰ���ʼ����tlv����������ȣ�������
void Tlv_Init(Tlv* pTlv, uint8 tag, uint8 len, const uint8* pData);

//��TLV���ݰ�������pBytes����len���ȵĺ��棬�����Сmaxlen
int Tlv_Append(uint8* pBytes, int len, int maxLen, const Tlv* pNewTlv);

//��һ�������У�����Tag��������Ѱ��tlv�ĵ�ַָ��
Tlv* Tlv_GetByTag(const uint8* pByte, int len, uint8 tag);

	void Tlv_dump(uint32 tag, uint8 tagLen, int len, const uint8* val, DType dt);
	void Tlvs_dump(const uint8* tlvs, uint8 tlvsLen, int tagLen);

#ifdef __cplusplus
}
#endif

#endif
