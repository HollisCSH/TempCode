/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "Dump.h"

void DumpByte(const uint8_t* pData, uint16_t len, uint8_t cols)
{
	int i;
	int counter = 0;
	//Printf("Dump Data, addr=[0x%04x], Len=%d\n", pData, len); 
	while (counter < len)
	{
		//Printf("[%04x]:",counter); 
		for (i = 0; i < cols; i++)
		{
//			Printf("%02X ", *pData);
			pData++;
			if (++counter >= len) break;
		}
	}
}

void DumpDword(const uint32_t * pData, uint16_t len, uint8_t cols)
{
	int i;
	int counter = 0;
	while (counter < len)
	{
//		Printf("[%04X]:", counter);
		for (i = 0; i < cols; i++)
		{
//			Printf("%08X ", *pData);
			pData++;

			if (++counter >= len) break;
		}
	}
}
/*
void ShowWait(int ticksPerDot, int dotCountPerRow)
{
	static int i = 0;
	static int count = 0;

	if ((i++) % ticksPerDot == 0)
	{
		if (count++ >= dotCountPerRow)
		{
			count = 0;
			i = 0;
			Printf("\n\t");
		}
		else
		{
			Printf(".");
		}
	}
}
*/
