//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: GprsTlv.h
//创建人  	: 
//创建日期	:
//描述	    : Sim模组Tlv电池协议头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================
#ifndef __GPRSTLV__H_
#define __GPRSTLV__H_

#ifdef __cplusplus
extern "C"{
#endif

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTypeDef.h"
#include "Tlv.h"
	
//=============================================================================================
//宏参数设定
//=============================================================================================
#define TAG_ACC 		 0x01	//size = 3，预留
#define TAG_LOCK 		 0x03	//size = 3
#define TAG_BATTERY1_OVP 0xA1	//size = 11
#define TAG_BATTERY1_OTP 0xA2	//size = 11
#define TAG_BATTERY2_OVP 0xB1	//size = 11
#define TAG_BATTERY2_OTP 0xB2	//size = 11
#define TAG_REMOVE       0xA3   //拆开事件

#define TAG_BUF_SIZE 	 50
#define SERIAL_NUM_SIZE	 6

//=============================================================================================
//声明接口函数
//=============================================================================================

//GPRS TLV包初始化，将所有tlv组合成一个数组buffer：g_SrvTlvBuf
void GprsTlv_Init(void);

//根据tag命令，输入单个字节组合成tlv，组合到pTlv数组中
int GprsTlv_AppendByteTlv(uint8* pTlv, int len, int maxLen, uint8 tag, uint8 data);

//根据输入的pNewTlv包，组合到pTlv数组中
int GprsTlv_AppendTlv(uint8* pTlv, int len, int maxLen, const Tlv* pNewTlv);

//更新g_SrvTlvBuf中的数据
void GprsTlv_UpdateSentTlv(void);

//非法充电tlv数据组装
Tlv* GprsTlv_GetOvp(uint8 port);

//电池过热tlv数据组装
Tlv* GprsTlv_GetOtp(uint8 port);

//获取事件变化，若有变化，则打包成tlv包，返回tlv包的长度
uint8 GprsTlv_GetChangedTlv(uint8* pBuf, int size);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************end of GprsTlv.h*****************************************/
