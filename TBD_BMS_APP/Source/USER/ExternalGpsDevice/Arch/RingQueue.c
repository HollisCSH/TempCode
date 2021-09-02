/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "RingQueue.h"
/*
void RingQueue_Dump(RingQueue* ringQueue)
{
	Queue* queue = (Queue*)ringQueue;
	Printf("RingQueue Dump(@0x%x)\n", (uint32_t)queue);
	Printf("\t readInd=%d\n"		, queue->readInd);
	Printf("\t writeInd=%d\n"		, queue->writeInd);
	Printf("\t elementSize=%d\n"	, queue->elementSize);
	Printf("\t bufSize=%d\n"		, queue->bufSize);
	Printf("\t buf=0x%x\n"			, (uint32_t)queue->buf);
	Printf("\t m_isEmpty=0x%x\n"	, ringQueue->isFull);
	Printf("\t m_isFull=0x%x\n"		, ringQueue->isEmpty);
}
*/
void* RingQueue_getNew(RingQueue* ringQueue)
{
	Queue* queue = (Queue*)ringQueue;
	void* pElement = Null;
	if(ringQueue->isFull) return Null;

	if (queue->writeInd == queue->readInd)
	{
		queue->writeInd = 0;
		queue->readInd = 0;
	}

	ringQueue->isEmpty = False;
	pElement = &queue->buf[queue->writeInd];

	queue->writeInd += queue->elementSize;
	if(queue->writeInd >= queue->bufSize)
	{
		queue->writeInd = 0;
	}
	
	if(queue->writeInd == queue->readInd)
	{
		ringQueue->isFull = True;
	}

	return pElement;
}

Bool RingQueue_write(RingQueue* ringQueue, const void* element, int len)
{
	Queue* queue = (Queue*)ringQueue;
	if (len <= queue->elementSize)
	{
		void* pData = RingQueue_getNew(ringQueue);
		if (pData)
		{
			memcpy(pData, element, len);
			return True;
		}
	}
	return False;
}

//Get the head element but not pop it, don't remove the readPointer.
void* RingQueue_read(RingQueue* ringQueue)
{
	Queue* queue = (Queue*)ringQueue;
	if(ringQueue->isEmpty) return Null;

	return &queue->buf[queue->readInd];
}

//Get the head element and pop it out from queue.
void* RingQueue_pop(RingQueue* ringQueue)
{
	Queue* queue = (Queue*)ringQueue;
	void* pvalue = &queue->buf[queue->readInd];

	if(ringQueue->isEmpty) return Null;

	queue->readInd += queue->elementSize;

	if(queue->readInd >= queue->bufSize)
	{
		queue->readInd = 0;
	}

	if(queue->readInd == queue->writeInd)
	{
		ringQueue->isEmpty = True;
	}

	ringQueue->isFull = False;
	return pvalue;
}

void RingQueue_reset(RingQueue* ringQueue)
{
	Queue* queue = (Queue*)ringQueue;
	Queue_reset(queue);
	
	ringQueue->isEmpty = True;
	ringQueue->isFull = False;
}

Bool RingQueue_init(RingQueue* ringQueue, void* pBuffer, unsigned short itemSize, unsigned int itemCount)
{
	memset(ringQueue, 0, sizeof(RingQueue));
	Queue_init((Queue*)ringQueue, pBuffer, itemSize, itemCount);
	ringQueue->isEmpty = True;
	return True;
}
