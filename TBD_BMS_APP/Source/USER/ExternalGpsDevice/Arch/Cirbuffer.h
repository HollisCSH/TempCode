/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-10-20     Jose      first version
 */


#ifndef __CIR_BUFFER_H__
#define __CIR_BUFFER_H__    

#ifdef __cplusplus
extern "C"{
#endif

//#include "typedef.h"


// z 一定要是2的幂次
//#define			_CIR_LOOP_ADD(x,l,z)		(x) = ( (x) + (l) )& ( (z) - 1 )

#define			_CIR_LOOP_ADD(x,l,z)		(x) = ( (x) + (l) )%( z )


typedef struct _CirBuff
{
	uint16_t 	miTail;
	uint16_t 	miHead;
	uint16_t 	miSize;
	uint8_t 	*mpBuff ;
}CirBuff,*pCirBuff;


void CirBuffInit( pCirBuff , uint8_t * , uint16_t );

void CirBuffReset( pCirBuff );

void CirBuffPush( pCirBuff , const uint8_t * , uint16_t );

uint16_t CirBuffPop( pCirBuff , uint8_t *, uint16_t );




#ifdef __cplusplus
}
#endif



#endif //__CIR_BUFFER_H__



//Cirbuffer
