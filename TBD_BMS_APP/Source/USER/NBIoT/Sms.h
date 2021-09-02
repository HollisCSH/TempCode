//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Sms.h
//创建人  	: 
//创建日期	:
//描述	    : Sim模组SMS短信处理头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================
#ifndef __SMS__H_
#define __SMS__H_

#ifdef __cplusplus
extern "C"{
#endif

//=============================================================================================
//包含头文件
//=============================================================================================
#include "AtCmdCtrl.h"
#include "time.h"

//=============================================================================================
//声明接口函数
//=============================================================================================

//SMS复位时间
void Sms_ResetTime(void);

//接收到上电READY后，随后设置短信格式，短信中心地址等
void Sms_CmdSmsReady(const char* pReq, int reqLen);

//解析收到的短信内容
void Sms_ParserMgl(const char* str, int* ind, struct tm* pDt);

//SMS解析/读取短信，并处理
int Sms_FilterCMGL(const char* pRcv, uint16 len);

//删除短消息后，开始连接TCP服务器
ATCMD_RC Sms_RspCMGD(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//SMS对列举短消息的命令处理
ATCMD_RC Sms_RspCMGL(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//SMS对设置短信中心地址的命令处理
ATCMD_RC Sms_CmdGSM(AtCmdState state, char* pRsp, struct _AtCmdItem** pNextAtCmd);

//SMS对SIM卡收到了短信的处理
void Sms_CmdCMTI(const char* pReq, int reqLen);

//发送短信命令响应函数
ATCMD_RC Gprs_RspSMGS(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//短信发送接收后响应处理函数
ATCMD_RC Gprs_RspSMGSTEXT(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************end of Sms.h*****************************************/
