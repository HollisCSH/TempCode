/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "Bit.h"

int Uint32_t_getMaskBit(uint32_t value)
{
	for (int i = 0; i < 32; i++)
	{
		if (value == (1 << i))
			return i;
	}

	return -1;
}

//在字符串中提取大端模式的16位数据
uint16_t bigendian16_get(uint8_t *msg)
{
    return (((uint16_t)(*msg)<<8) |(*(msg+1)));
}

//向字符串中输出大端模式的16位数据
void bigendian16_put(uint8_t *msg, uint16_t data)
{
    *msg = (uint8_t)(data>>8);
    *(msg+1) = (uint8_t)(data&0xff);
}
