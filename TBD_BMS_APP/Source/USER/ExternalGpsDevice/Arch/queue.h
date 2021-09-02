/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */


#ifndef __Queue_H__
#define __Queue_H__    

#ifdef __cplusplus
extern "C"{
#endif

#include "BSPTypeDef.h"

#define INVALID_POS		0xFFFFFFFF
#define THE_FIRST_POS	0xFFFFFFFE

typedef struct _Queue
{
    int readInd;
    int writeInd;

	unsigned short	elementSize;		//Element size
	int	bufSize;
    uint8_t* buf;
}Queue;

void Queue_Dump(Queue* queue);

inline Bool   Queue_isEmpty(Queue* queue){return queue->writeInd == queue->readInd;}
inline Bool   Queue_isFull(Queue* queue){return queue->writeInd >= queue->bufSize;}
inline int Queue_GetElements(Queue* queue){return queue->writeInd - queue->readInd;}

void* Queue_getNew(Queue* queue);
Bool Queue_write(Queue* queue, const void* element, int len);

//Read the head element of queue, but not pop element.
void* Queue_Read(Queue* queue);

//Pop out the head element from queue
void* Queue_pop(Queue* queue);
//Get multi elements and pop them
void* Queue_popElements(Queue* queue, int elementCount);

/******************************************
 * Function: Search data in queue, dlc may be one or more elements len
 * Param:
 * 	queue: queue pointer.
 *  pElement: elememt pointer for search.
 *  elements: element count for search.
 *  pElementCount: out param, > 0 if search success. = 0 if search failed.
 * Return:
 * 	Not Null: pointer to the head of data in queue.
 *  Null: not found the elememt.
 * ***************************************/
void* Queue_search(Queue* queue, void* pElement, int elements, int* pElementCount);

void Queue_reset(Queue* queue);
Bool Queue_init(Queue* queue, void* pbuffer, unsigned short itemSize, unsigned int itemCount);

/****************************
 * the follow functions only for byte queue.
 * **************************/

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
void* Queue_searchByte(Queue* queue, uint8_t byte, int* offsetIndex);
Bool Queue_writeByte(Queue* queue, uint8_t byte);
Bool Queue_writeBytes(Queue* queue, const uint8_t* bytes, int numOfByte);

#ifdef __cplusplus
}
#endif

#endif 

