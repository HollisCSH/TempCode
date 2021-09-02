/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "Tlv.h"

#if 0
void Tlv_dump(uint32 tag, uint8 tagLen, int len, const uint8* val, DType dt)
{
	struct
	{
		DType dt;
		const char* fmt;
		DtConvertFn Convert;
	}
	static const disp[] =
	{
		{DT_UINT8 , "0x%02X(%d)", (DtConvertFn)Dt_convertToU8 },
		{DT_UINT16, "0x%04X(%d)", (DtConvertFn)Dt_convertToU16},
		{DT_UINT32, "0x%08X(%d)", (DtConvertFn)Dt_convertToU32},
		{DT_INT8  , "0x%02X(%d)", (DtConvertFn)Dt_convertToU8 },
		{DT_INT16 , "0x%02X(%d)", (DtConvertFn)Dt_convertToU16},
		{DT_INT32 , "0x%02X(%d)", (DtConvertFn)Dt_convertToU32},

		{DT_UINT16 , "0x%04X(%d)", (DtConvertFn)Dt_convertToU16},
		{DT_UINT32 , "0x%08X(%d)", (DtConvertFn)Dt_convertToU32},
		{DT_INT16  , "0x%04X(%d)", (DtConvertFn)Dt_convertToU16},
		{DT_INT32  , "0x%08X(%d)", (DtConvertFn)Dt_convertToU32},

		{DT_FLOAT32, "%.2f", (DtConvertFn)Dt_convertToU32},
	};

	const char* fmt = Null;
	if (tagLen == 1)
	{
		fmt = "[%02X-%02d]:";
	}
	else if (tagLen == 2)
	{
		fmt = "[%04X-%02d]:";
	}

	//Sample: [120A-1]:2B(43)

	Printf(fmt, tag, len);
	fmt = Null;

	DtConvertFn convet;
	for (int i = 0; i < GET_ELEMENT_COUNT(disp); i++)
	{
		if (dt == disp[i].dt)
		{
			fmt = disp[i].fmt;
			convet = disp[i].Convert;
		}
	}

	if (fmt)
	{
		Printf(fmt, convet(val, dt));
	}
	else if (dt == DT_STRING)
	{
		Printf((char*)val);
	}
	else
	{
		DUMP_BYTE(val, len);
	}
}


void Tlvs_dump(const uint8* tlvs, uint8 tlvsLen, int tagLen)
{
	uint32 tag = 0;
	uint8 len = tlvs[tagLen];
	for (int i = 0; i < tlvsLen; i += tagLen + 1 + len)
	{
		memcpy(&tag, tlvs, tagLen);
		len = tlvs[tagLen];

		Tlv_dump(tag, tagLen, len , &tlvs[tagLen + 1], DT_BYTES);

		tlvs += tagLen + 1 + len;
	}
}
#endif
//=======================================Copyright(c)===========================================
// 								  ���������Ƽ����޹�˾
//---------------------------------------�ļ���Ϣ----------------------------------------------
//�ļ���   	: Tlv.c
//������  	: 
//��������	:
//����	    : Simģ��TlvЭ�鴦���ļ�
//-----------------------------------------------��ǰ�汾�޶�----------------------------------
//�޸���   	:
//�汾	    :
//�޸�����	:
//����	    : 
//=============================================================================================

//=============================================================================================
//����ͷ�ļ�
//=============================================================================================
#include "common.h"
#include "Tlv.h"

//=============================================================================================
//����ӿں���
//=============================================================================================

//TLV���ݰ���ʼ����tlv����������ȣ�������
void Tlv_Init(Tlv* pTlv, uint8 tag, uint8 len, const uint8* pData)
{
	pTlv->tag = tag;
	pTlv->len = len;
	memcpy(pTlv->data, pData, len);
}

//��TLV���ݰ�������pBytes����len���ȵĺ��棬�����Сmaxlen
int Tlv_Append(uint8* pBytes, int len, int maxLen, const Tlv* pNewTlv)
{
	if(len + pNewTlv->len + TLV_HEAD_SIZE <= maxLen)
	{
		memcpy(&pBytes[len], pNewTlv, pNewTlv->len + TLV_HEAD_SIZE);
	}

	return len + pNewTlv->len + TLV_HEAD_SIZE;
}

//��һ�������У�����Tag��������Ѱ��tlv�ĵ�ַָ��
Tlv* Tlv_GetByTag(const uint8* pByte, int len, uint8 tag)
{
	Tlv* pTlv;
	for(int i = 0; i < len; i += (pTlv->len + TLV_HEAD_SIZE))
	{
		pTlv = (Tlv*)&pByte[i];
		
		if(tag == pTlv->tag) 
			return pTlv;
	}
	return Null;
}

/*****************************************end of Tlv.c*****************************************/
