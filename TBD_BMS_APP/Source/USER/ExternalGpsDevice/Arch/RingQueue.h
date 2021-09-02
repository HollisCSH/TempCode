/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */


#ifndef __RING_QUEUE_H__
#define __RING_QUEUE_H__    

#ifdef __cplusplus
extern "C"{
#endif

#include "queue.h"

#define INVALID_POS		0xFFFFFFFF
#define THE_FIRST_POS	0xFFFFFFFE

typedef struct _RingQueue
{
    Queue  base;
    Bool   isFull;
    Bool   isEmpty;
}RingQueue;

void* RingQueue_getNew(RingQueue* ringQueue);
Bool RingQueue_write(RingQueue* ringQueue, const void* element, int len);

//Get the head element of queue
void* RingQueue_read(RingQueue* ringQueue);
//Get the head element and remove it
void* RingQueue_pop(RingQueue* ringQueue);

inline Bool RingQueue_isEmpty(RingQueue* ringQueue){return ringQueue->isEmpty;};
inline Bool RingQueue_isFull(RingQueue* ringQueue){return ringQueue->isFull;};

void RingQueue_reset(RingQueue* ringQueue);

Bool RingQueue_init(RingQueue* ringQueue, void* pBuffer, unsigned short itemSize, unsigned int itemCount);
//void RingQueue_RemoveElements(RingQueue* ringQueue, int nElements);
void RingQueue_Dump(RingQueue* ringQueue);

#ifdef __cplusplus
}
#endif

#endif 

