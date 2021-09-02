//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Queue_old.h
//创建人  	: 
//创建日期	:
//描述	    : Sim模组队列操作头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 常见的入队列，出队列
//=============================================================================================
#ifndef __QUEUE_OLD_H__
#define __QUEUE_OLD_H__    

#ifdef __cplusplus
extern "C"{
#endif
    
//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTypeDef.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
#define     INVALID_POS		0xFFFFFFFF
#define     THE_FIRST_POS	0xFFFFFFFE

//=============================================================================================
//定义数据类型
//=============================================================================================
//队列操作定义结构体
typedef struct _Queue_old
{
    int m_ReadPointer;      //队列读指针
    int m_WritePointer;     //队列写指针
	unsigned short	m_nElementSize;		//Element size，一个队列元素的大小

    unsigned char m_isUsed:1;   //是否使用
    unsigned char m_isEmpty:1;  //是否空队列
    unsigned char m_isFull:1;   //是否队列已满
    unsigned char m_isLock:1;   //队列是否锁定
    unsigned char m_count:4;    //队列实时元素个数
	
	uint32	m_nBufferSize;      //分配的队列内存大小
    uint8* m_pBuffer;           //队列buff对应地址指针
}Queue_old;

//=============================================================================================
//声明接口函数
//=============================================================================================

//打印队列信息
void QUEUE_Dump(Queue_old* queue);

//获取队列中，直到队列尾，未出队列的元素个数
int QUEUE_getContinuousEleCount(Queue_old* queue);

//获取队列中实际未出队列的元素个数
int QUEUE_getElementCount(Queue_old* queue);

//Return the element count of push out queue
//返回实际出队列的元素个数
int QUEUE_PushOutEleArray(Queue_old* queue, void* pEleArrayBuf, int nMaxEleCount);

//获取队列中，直到队列尾，剩余的队列空间
int QUEUE_getContinuousSpace(Queue_old* queue);

//获取队列中，实际剩余的队列空间
int QUEUE_getSpace(Queue_old* queue);

//压入数据到队列中，暂不使用
int QUEUE_PushInEleArray(Queue_old* queue, const void* pEleArray, int nEleCount);

//暂不使用
void* QUEUE_getNew(Queue_old* queue);

//队列中添加元素，压入队列
Bool QUEUE_add(Queue_old* queue, const void* element, int len);

//获取读指针的队列头指针
void* QUEUE_getHead(Queue_old* queue);

//暂不使用
void* QUEUE_get(Queue_old* queue, uint32* pNextPos);

//队列移除nElements个元素
void QUEUE_RemoveElements(Queue_old* queue, int nElements);

//抛弃头元素
void QUEUE_removeHead(Queue_old* queue);

//队列删除所有元素
void QUEUE_removeAll(Queue_old* queue);

//获取队列头指针的地址并且删除头指针的第一个元素
void* QUEUE_popGetHead(Queue_old* queue);

//判断是否队列已空
Bool QUEUE_isEmpty(Queue_old* queue);

//判断是否队列已满
Bool QUEUE_isFull(Queue_old* queue);

//初始化队列
Bool QUEUE_init(Queue_old* queue, void* pBuffer, unsigned short itemSize, uint32 itemCount);


//队列复位，清空buff
void QUEUE_reset(Queue_old* queue);

#ifdef __cplusplus
}
#endif

#endif 

/*****************************************end of Queue_old.h*****************************************/

