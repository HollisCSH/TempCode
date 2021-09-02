/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "Tlv.h"

uint8 Dt_convertToU8(const void* val, DType dt)
{
	uint8 value = *((uint8*)val);
	return value;
}

uint16 Dt_convertToU16(const void* val, DType dt)
{
	uint16 value = *((uint16*)val);
	if (dt == DT_UINT16)
	{
		value = SWAP16(value);
	}

	return value;
}

uint32 Dt_convertToU32(const void* val, DType dt)
{
	uint32 value = *((uint32*)val);
	if (dt == DT_UINT32)
	{
		value = SWAP32(value);
	}
	return value;
}

//大小端位序转换
void* Dt_swap(uint8* val, DType dt)
{
	switch (dt)
	{
	case DT_INT16: 
	case DT_UINT16: {*(uint16*)val = SWAP16(*val); return val; }
	case DT_INT32:  
	case DT_UINT32: 
	case DT_FLOAT32:{*(uint32*)val = SWAP32(*val); return val; }
	default:
		return val;
	}
}
