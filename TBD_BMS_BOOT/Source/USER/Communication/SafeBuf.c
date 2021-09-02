//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SafeBuf.c
//创建人  	: 
//创建日期	:
//描述	    : Sim模组串口读写缓存操作文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SafeBuf.h"
#include "Common.h"

//=============================================================================================
//定义接口函数
//=============================================================================================

//buff读写索引复位
void SafeBuf_Reset(SafeBuf* pSafeBuf)
{
	pSafeBuf->m_WriteInd = 0;
	pSafeBuf->m_ReadIndx = 0;
}

//buff初始化
void SafeBuf_Init(SafeBuf* pSafeBuf, void* pBuf, uint16 bufSize)
{
	memset(pSafeBuf, 0, sizeof(SafeBuf));

	//Faking the complier to change the const value
	pSafeBuf->m_pBuf = pBuf;
	pSafeBuf->m_MaxSize = bufSize;	
}

//Push one byte to SafeBuf
//写入1字节到buff中
int SafeBuf_WriteByte(SafeBuf* pSafeBuf, uint8 data)
{
	if(pSafeBuf->m_WriteInd == pSafeBuf->m_ReadIndx)
	{
        //读取完毕，则立刻清除所有索引
		pSafeBuf->m_WriteInd = 0;
		pSafeBuf->m_ReadIndx = 0;
	}
	
	if(pSafeBuf->m_WriteInd < pSafeBuf->m_MaxSize)
	{
		pSafeBuf->m_pBuf[pSafeBuf->m_WriteInd++] = data;
		return 1;
	}
	else
	{
		//Printf("Buf full\n");
		return 0;
	}
}

//Push one or more bytes to SafeBuf
//写入多个字节到buff中
int SafeBuf_Write(SafeBuf* pSafeBuf, const void* pData, uint16 len)
{
	int copyLen = 0;
	int bytes = 0;
	if(pSafeBuf->m_WriteInd == pSafeBuf->m_ReadIndx)
	{
		pSafeBuf->m_WriteInd = 0;
		pSafeBuf->m_ReadIndx = 0;
	}

	bytes = pSafeBuf->m_MaxSize - pSafeBuf->m_WriteInd;
	copyLen = (len > bytes) ? bytes : len;
	memcpy(&pSafeBuf->m_pBuf[pSafeBuf->m_WriteInd], pData, copyLen);
	pSafeBuf->m_WriteInd += copyLen;
	
	return copyLen;
}

//Pop one or more bytes from SafeBuf
//从buff中读取1个或多个字节
int SafeBuf_Read(SafeBuf* pSafeBuf, void* pBuf, int bufSize)
{
	int copyLen = 0;
	int bytes = pSafeBuf->m_WriteInd - pSafeBuf->m_ReadIndx;

	if(bytes)
	{
		copyLen = (bytes > bufSize) ? bufSize : bytes;
		memcpy(pBuf, &pSafeBuf->m_pBuf[pSafeBuf->m_ReadIndx], copyLen);
		pSafeBuf->m_ReadIndx += copyLen;
	}
	
	return copyLen;
}

//获取buff中剩余的未处理字节
int SafeBuf_GetCount(SafeBuf* pSafeBuf)
{
	return (pSafeBuf->m_WriteInd - pSafeBuf->m_ReadIndx);
}

/*****************************************end of SafeBuf.c*****************************************/
