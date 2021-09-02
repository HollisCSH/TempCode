/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#include "ArchDef.h"
#include "BSPTypeDef.h"
#include "queue.h"
/*
void Queue_Dump(Queue* queue)
{
	Printf("Queue Dump(@0x%lx)\n"	, (uint32_t)queue);
	Printf("\t readInd=%d\n"		, queue->readInd);
	Printf("\t writeInd=%d\n"		, queue->writeInd);
	Printf("\t elementSize=%d\n"	, queue->elementSize);
	Printf("\t bufSize=%d\n"		, queue->bufSize);
	Printf("\t buf=0x%x\n"			, (uint32_t)queue->buf);
}
*/
void* Queue_getNew(Queue* queue)
{
	void* pElement = Null;
	
	if (queue->writeInd == queue->readInd)
	{
		queue->writeInd = 0;
		queue->readInd  = 0;
	}
	else if (queue->writeInd >= queue->bufSize)
	{
		return Null;	//Full
	}

	pElement = &queue->buf[queue->writeInd];
	queue->writeInd += queue->elementSize;
	return pElement;
}

Bool Queue_write(Queue* queue, const void* element, int len)
{
	if (len <= queue->elementSize)
	{
		void* pData = Queue_getNew(queue);
		if (pData)
		{
			memcpy(pData, element, len);
			return True;
		}
	}
	return False;
}

void* Queue_Read(Queue* queue)
{
	if (queue->readInd == queue->writeInd)
	{
		return Null;
	}
	
	return &queue->buf[queue->readInd];
}

//Get the head element and pop it
void* Queue_pop(Queue* queue)
{
	void* p = &queue->buf[queue->readInd];
	if(queue->readInd == queue->writeInd)
	{
		return Null;
	}
	
	queue->readInd += queue->elementSize;
	return p;
}

//Get multi elements and pop them
void* Queue_popElements(Queue* queue, int elementCount)
{
	void* p = Null;
	if(Queue_GetElements(queue) >= elementCount)
	{
		p = &queue->buf[queue->readInd];
		queue->readInd += elementCount * queue->elementSize;
	}
	return p;
}

/******************************************
 * Function: Search data in queue, dlc may be one or more elements len
 * Param:
 * 	queue: queue pointer.
 *  pElement: elememt pointer for search.
 *  elements: element count for search.
 *  offsetIndex: [IN] start index in queue for search out param, [OUT] next start index for search.
 * Return:
 * 	Not Null: pointer to the head of data in queue.
 *  Null: not found the elememt.
 * ***************************************/
void* Queue_search(Queue* queue, void* pElement, int elements, int* offsetIndex)
{
	void* p = Null;
	*offsetIndex  = (*offsetIndex > queue->readInd) ? *offsetIndex : queue->readInd;
	for(int i = *offsetIndex; i <= queue->writeInd - elements; i += queue->elementSize)
	{
		p = &queue->buf[i];
		if(memcmp(p, pElement, elements) == 0)
		{
			* offsetIndex = i + elements;
			return &queue->buf[queue->readInd];
		}
	}
	return Null;
}


Bool Queue_init(Queue* queue, void* pBuffer, unsigned short itemSize, unsigned int itemCount)
{
	memset(queue, 0, sizeof(Queue));

	queue->buf = pBuffer;	
	queue->elementSize = itemSize;
	queue->bufSize = itemSize * itemCount;

	return True;
}

void Queue_reset(Queue* queue)
{
	queue->readInd = 0;
	queue->writeInd = 0;
}

/******************************************
 * Function: Search byte in queue, only for byte queue
 * Param:
 * 	queue: queue pointer.
 *  byte: byte for search.
 *  offsetIndex: [IN] start index in queue for search out param, [OUT] next start index for search.
 * Return:
 * 	Not Null: pointer to the head of data in queue.
 *  Null: not found the elememt.
 * ***************************************/
void* Queue_searchByte(Queue* queue, uint8_t byte, int* offsetIndex)
{
	//if elementSize != 1, can't search
	if(queue->elementSize != 1) return Null;

	int i  = (*offsetIndex > queue->readInd) ? *offsetIndex : queue->readInd;
	for(; i < queue->writeInd; i ++)
	{
		if(queue->buf[i] == byte)
		{
			*offsetIndex = i;
			return &queue->buf[queue->readInd];
		}
	}

	*offsetIndex = i;
	return Null;
}

Bool Queue_writeByte(Queue* queue, uint8_t byte)
{
	//if elementSize != 1, can't call function Queue_writeByte.
	if(queue->elementSize != 1) return False;

	if (queue->writeInd == queue->readInd)
	{
		queue->writeInd = 0;
		queue->readInd  = 0;
	}
	else if (queue->writeInd >= queue->bufSize)
	{
		return False;	//Full
	}
	queue->buf[queue->writeInd++] = byte;

	return True;
}

Bool Queue_writeBytes(Queue* queue, const uint8_t* bytes, int numOfByte)
{
	//if elementSize != 1, can't call function Queue_writeByte.
	if(queue->elementSize != 1) return False;

	if (queue->writeInd == queue->readInd)
	{
		queue->writeInd = 0;
		queue->readInd  = 0;
	}
	else if (queue->writeInd + numOfByte >= queue->bufSize)
	{
 		return False;	//Full
	}
	memcpy(&queue->buf[queue->writeInd], bytes, numOfByte);
	queue->writeInd += numOfByte;

	return True;
}
