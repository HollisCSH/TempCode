/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-10-20     Jose      first version
 */



#include "ArchDef.h"
//#include "typedef.h"
#include "Cirbuffer.h"



void CirBuffInit( pCirBuff pcirbuff, uint8_t * pbuff, uint16_t size )
{
	if( !pcirbuff || !pbuff || !size ) return ;
	pcirbuff->mpBuff = pbuff ;
	pcirbuff->miHead = 0 ;
	pcirbuff->miTail = 0 ;
	pcirbuff->miSize = size ;
}

void CirBuffReset( pCirBuff pcirbuff )
{
	if( !pcirbuff ) return ;
	pcirbuff->miTail = 0 ;
	pcirbuff->miHead = 0 ;
}


void CirBuffPush( pCirBuff pcirbuff , const uint8_t *pInbuff , uint16_t len )
{
	uint16 i = 0 ;
	if( !pcirbuff || !pInbuff || !len ) return ;
	for( i = 0 ; i < len ; i++ )
	{
		pcirbuff->mpBuff[pcirbuff->miTail] = pInbuff[i];
		_CIR_LOOP_ADD( pcirbuff->miTail, 1, pcirbuff->miSize );
	}
}

uint16_t CirBuffPop( pCirBuff pcirbuff , uint8_t *poutbuff , uint16_t size )
{
	uint16_t len = 0 ;
	if( !pcirbuff || !poutbuff || !size ) return 0;
	while( pcirbuff->miHead != pcirbuff->miTail && len < size )
	{
		poutbuff[len++] = pcirbuff->mpBuff[pcirbuff->miHead];
		_CIR_LOOP_ADD( pcirbuff->miHead, 1, pcirbuff->miSize );
	}
	return len ;
}











