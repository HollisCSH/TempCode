//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Queue_old.c
//创建人  	: 
//创建日期	:
//描述	    : Sim模组队列操作文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 常见的入队列，出队列
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SimCommon.h"
#include "queue_old.h"

//=============================================================================================
//定义接口函数
//=============================================================================================

//打印队列信息
void QUEUE_Dump(Queue_old* queue)
{
//	Printf("Queue Dump(@0x%x)\n"	, queue);
//	Printf("\t m_ReadPointer=%d\n"	, queue->m_ReadPointer);
//	Printf("\t m_WritePointer=%d\n"	, queue->m_WritePointer);
//	Printf("\t m_nElementSize=%d\n"	, queue->m_nElementSize);
//	Printf("\t m_isEmpty=%d\n"		, queue->m_isEmpty);
//	Printf("\t m_isFull=%d\n"		, queue->m_isFull);
//	Printf("\t m_nBufferSize=%d\n"	, queue->m_nBufferSize);
//	Printf("\t m_pBuffer=0x%x\n"	, queue->m_pBuffer);
}

//获取队列中，直到队列尾，未出队列的元素个数
int QUEUE_getContinuousEleCount(Queue_old* queue)
{
	int count = 0;
	if(queue->m_isEmpty)
	{
		return 0;
	}
	
	if(queue->m_WritePointer > queue->m_ReadPointer)
	{
		count = (queue->m_WritePointer - queue->m_ReadPointer);
	}
	else
	{
		count = (queue->m_nBufferSize - queue->m_ReadPointer);
	}

	if(1 != queue->m_nElementSize)
	{
		count /= queue->m_nElementSize;
	}	

	return count;
}

//获取队列中实际未出队列的元素个数
int QUEUE_getElementCount(Queue_old* queue)
{
	int count = 0;
	if(queue->m_isEmpty)
	{
		return 0;
	}
	
	if(queue->m_isFull)
	{
		count = queue->m_nBufferSize;
	}
	else if(queue->m_WritePointer > queue->m_ReadPointer)
	{
		count = queue->m_WritePointer - queue->m_ReadPointer;
	}
	else
	{
        //已经写满并溢出覆盖到队列头的情况
		count = queue->m_nBufferSize - queue->m_ReadPointer + queue->m_WritePointer;
	}
	
	if(1 != queue->m_nElementSize)
	{
		count /= queue->m_nElementSize;
	}	
	return count;
}

//Return the element count of push out queue
//返回实际出队列的元素个数
int QUEUE_PushOutEleArray(Queue_old* queue, void* pEleArrayBuf, int nMaxEleCount)
{
	int nLen = 0;
	int nCount = 0;
	int i = 0;
	uint8* pData = (uint8*)pEleArrayBuf;
	int nSpace = 0;

	nSpace = QUEUE_getContinuousEleCount(queue);

	while(nSpace && nMaxEleCount)
	{
		if(nSpace >= nMaxEleCount)
		{
			nLen = nMaxEleCount * queue->m_nElementSize;
			if(pData)
			{
				memcpy(&pData[i], &queue->m_pBuffer[queue->m_ReadPointer], nLen);
			}
			nCount += nMaxEleCount;
			nMaxEleCount -= nSpace; //此处是为了退出while循环
			
			queue->m_ReadPointer += nLen;
			if(queue->m_ReadPointer >= queue->m_nBufferSize)
			{
				queue->m_ReadPointer = 0;
			}
			queue->m_isFull = False;
			if(queue->m_WritePointer == queue->m_ReadPointer)
			{
				queue->m_isEmpty = True;
			}	
			break;
		}
		else
		{
            //先出队列，直到队列尾部
			nLen = nSpace * queue->m_nElementSize;
			if(pData)
			{
				memcpy(&pData[i], &queue->m_pBuffer[queue->m_ReadPointer], nLen);
			}
			i += nLen;
			nCount += nSpace;
			nMaxEleCount -= nSpace;
			
			queue->m_ReadPointer += nLen;
			if(queue->m_ReadPointer >= queue->m_nBufferSize)
			{
				queue->m_ReadPointer = 0;
			}
			queue->m_isFull = False;
			if(queue->m_WritePointer == queue->m_ReadPointer)
			{
				queue->m_isEmpty = True;
			}	
		}
		nSpace = QUEUE_getContinuousEleCount(queue);
	}
	
	return nCount;
}

//获取队列中，直到队列尾，剩余的队列空间
int QUEUE_getContinuousSpace(Queue_old* queue)
{
	if(queue->m_isEmpty)
	{
		//queue->m_WritePointer = 0;
		//queue->m_ReadPointer = 0;
		return (queue->m_nBufferSize - queue->m_WritePointer) / queue->m_nElementSize;
	}
	else if(queue->m_isFull)
	{
		return 0;
	}
	else if(queue->m_WritePointer > queue->m_ReadPointer)
	{
		return (queue->m_nBufferSize - queue->m_WritePointer) / queue->m_nElementSize;
	}
	else
	{
		return (queue->m_ReadPointer - queue->m_WritePointer) / queue->m_nElementSize;
	}
}

//获取队列中，实际剩余的队列空间
int QUEUE_getSpace(Queue_old* queue)
{
	if(queue->m_isFull)
	{
		return 0;
	}
	else if(queue->m_isEmpty)
	{
		return queue->m_nBufferSize/queue->m_nElementSize;
	}
	else if(queue->m_WritePointer > queue->m_ReadPointer)
	{
		return (queue->m_nBufferSize - queue->m_WritePointer + queue->m_ReadPointer) / queue->m_nElementSize;
	}
	else
	{
		return (queue->m_ReadPointer - queue->m_WritePointer) / queue->m_nElementSize;
	}
}

//Return the element count of push in queue 
//压入数据到队列中，暂不使用
int QUEUE_PushInEleArray(Queue_old* queue, const void* pEleArray, int nEleCount)
{
	int nLen = 0;
	int nCount = 0;
	int i = 0;
	const uint8* pData = (uint8*)pEleArray;
	int nSpace = QUEUE_getContinuousSpace(queue);

	while(nSpace && nEleCount)
	{
		if(nSpace >= nEleCount)
		{
			nLen = nEleCount * queue->m_nElementSize;
			if(pData)
			{
				memcpy(&queue->m_pBuffer[queue->m_WritePointer], &pData[i], nLen);
			}
			nCount += nEleCount;
			
			queue->m_WritePointer += nLen;
			if(queue->m_WritePointer >= queue->m_nBufferSize)
			{
				queue->m_WritePointer = 0;
			}
			queue->m_isEmpty = False;
			if(queue->m_WritePointer == queue->m_ReadPointer)
			{
				queue->m_isFull = True;
			}	
			break;
		}
		else
		{
			nLen = nSpace * queue->m_nElementSize;
			if(pData)
			{
				memcpy(&queue->m_pBuffer[queue->m_WritePointer], &pData[i], nLen);
			}
			i += nLen;
			nCount += nSpace;
			nEleCount -= nSpace;
			
			queue->m_WritePointer += nLen;
			if(queue->m_WritePointer >= queue->m_nBufferSize)
			{
				queue->m_WritePointer = 0;
			}
			queue->m_isEmpty = False;
			if(queue->m_WritePointer == queue->m_ReadPointer)
			{
				queue->m_isFull = True;
			}
		}
		nSpace = QUEUE_getContinuousSpace(queue);
	}
	
	return nCount;
}

//暂不使用
void* QUEUE_getNew(Queue_old* queue)
{
	void* pElement = &queue->m_pBuffer[queue->m_WritePointer];
	
	if(queue->m_isFull) return Null;
	
	queue->m_WritePointer += queue->m_nElementSize;
	if(queue->m_WritePointer >= queue->m_nBufferSize)
	{
		queue->m_WritePointer = 0;
	}
	
	queue->m_isEmpty = False;
	if(queue->m_WritePointer == queue->m_ReadPointer)
	{
		queue->m_isFull = True;
	}

	return pElement;
}

//队列中添加元素，压入队列
Bool QUEUE_add(Queue_old* queue, const void* element, int len)
{
	if(queue->m_isFull || queue->m_nElementSize < len) return False;

	memcpy(&queue->m_pBuffer[queue->m_WritePointer], element, len);

	queue->m_WritePointer += queue->m_nElementSize;
	if(queue->m_WritePointer >= queue->m_nBufferSize)
	{
		queue->m_WritePointer = 0;
	}
	
	queue->m_isEmpty = False;
	if(queue->m_WritePointer == queue->m_ReadPointer)
	{
		queue->m_isFull = True;
	}
	
	return True;
}

//获取读指针的队列头指针
void* QUEUE_getHead(Queue_old* queue)
{
	if(queue->m_isEmpty)
	{
		return Null;
	}
	
	return &queue->m_pBuffer[queue->m_ReadPointer];
}

//暂不使用
void* QUEUE_get(Queue_old* queue, uint32* pNextPos)
{
	void* pRet = Null;

	* pNextPos = 0;
	if(queue->m_isEmpty || INVALID_POS == *pNextPos)
	{
		return Null;
	}

	if(THE_FIRST_POS == *pNextPos)
	{
		*pNextPos = queue->m_ReadPointer;
	}

	//Assert((*pNextPos % queue->m_nElementSize) == 0);

	pRet = &queue->m_pBuffer[*pNextPos];
	
	*pNextPos += queue->m_nElementSize;
	if(*pNextPos >= queue->m_nBufferSize)
	{
		*pNextPos = 0;
	}
	
	if(*pNextPos == queue->m_WritePointer)
	{
		*pNextPos = INVALID_POS;
	}
	
	return pRet;
}

//队列移除nElements个元素
void QUEUE_RemoveElements(Queue_old* queue, int nElements)
{
	QUEUE_PushOutEleArray(queue, Null, nElements);
}

//Pop the head element;
//抛弃头元素
void QUEUE_removeHead(Queue_old* queue)
{
	if(queue->m_isEmpty)
	{
		return;
	}
	
	queue->m_ReadPointer += queue->m_nElementSize;
	if(queue->m_ReadPointer >= queue->m_nBufferSize)
	{
		queue->m_ReadPointer = 0;
	}
	
	queue->m_isFull = False;
	if(queue->m_ReadPointer == queue->m_WritePointer)
	{
		queue->m_isEmpty = True;
	}
}

//队列删除所有元素
void QUEUE_removeAll(Queue_old* queue)
{
	queue->m_ReadPointer = 0;
	queue->m_WritePointer = 0;
	queue->m_isEmpty = True;
	queue->m_isFull = False;
}

//Get the head element and pop it
//获取队列头指针的地址并且删除头指针的第一个元素
void* QUEUE_popGetHead(Queue_old* queue)
{
	void* pvalue = QUEUE_getHead(queue);
	QUEUE_removeHead(queue);
	return pvalue;
}

//Is queue empty 
//return: 1=Empty 0=Not Empty
//判断是否队列已空
Bool QUEUE_isEmpty(Queue_old* queue)
{
	return queue->m_isEmpty;
}

//判断是否队列已满
Bool QUEUE_isFull(Queue_old* queue)
{
	return queue->m_isFull;
}

//queue：初始化的队列
//pBuffer：队列buff地址
//itemSize：队列元素大小
//itemCount：元素个数
Bool QUEUE_init(Queue_old* queue, void* pBuffer, unsigned short itemSize, uint32 itemCount)
{
	memset(queue, 0, sizeof(Queue_old));
	queue->m_isUsed = 1;

	queue->m_pBuffer = pBuffer;	
	queue->m_isEmpty = True;
	queue->m_nElementSize = itemSize;
	queue->m_nBufferSize = itemSize * itemCount;

	return True;
}

//队列复位，清空buff
void QUEUE_reset(Queue_old* queue)
{
	queue->m_ReadPointer = 0;
	queue->m_WritePointer = 0;
	queue->m_isEmpty = True;
	queue->m_isFull = False;
	memset(queue->m_pBuffer, 0, queue->m_nBufferSize);
}

/*****************************************end of Queue_old.c*****************************************/
