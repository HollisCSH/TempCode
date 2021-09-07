//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Gprs868.h
//创建人  	: 
//创建日期	:
//描述	    : Sim模组的SIM868 2G/4G处理头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================
#ifndef __GPRS_868__H_
#define __GPRS_868__H_

#ifdef __cplusplus
extern "C"{
#endif

//=============================================================================================
//包含头文件
//=============================================================================================
#include "AtCmdCtrl.h"

//=============================================================================================
//声明接口函数
//=============================================================================================

//关闭回显响应处理
ATCMD_RC Gprs_RspATE0(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//Cfun关闭注册网络处理
ATCMD_RC Gprs_RspCfun(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//设置发送接收格式
ATCMD_RC Gprs_Rsp_SET_FORMAT(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//暂不使用
ATCMD_RC Gprs_RspCPIN(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//查询信号质量响应处理函数
ATCMD_RC Gprs_RspCSQ(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//查询是否附着网络响应函数
ATCMD_RC Gprs_RspCGATT(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//设置网络附着
ATCMD_RC Gprs_RspCGATT_SET(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//发送CIPSEND后，等待输入数据的处理函数
ATCMD_RC Gprs_RspCIPSEND(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//TCP数据发送成功完毕后的，响应处理函数
ATCMD_RC Gprs_RspCIPSENDDATA(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//查询网络注册情况的响应处理函数
ATCMD_RC Gprs_RspCREG(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//查询 SIM 卡 ICCID 号码响应回复处理
ATCMD_RC Gprs_RspCCID(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//建立TCP连接，响应处理函数
ATCMD_RC Gprs_RspCIPOPEN(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//关闭移动场景响应处理函数
ATCMD_RC Gprs_RspSHUTDOWN(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//关闭 TCP 或 UDP 连接响应处理函数
ATCMD_RC Gprs_RspNETCLOSE(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//暂不使用
void Gprs_CmdState(const char* pReq, int reqLen);

//查询当前连接状态响应处理函数
ATCMD_RC Gprs_RspCIPSTATUS(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//SIM卡相关事件URC响应处理函数
void Gprs_CmdSimCardEvent(const char* pReq, int reqLen);

//收到连接服务器CONNECT OK的URC，响应处理函数
void Gprs_CmdConnect(const char* pReq, int reqLen);

//标识 GPRS 被网络释放处理， 此时仍然需要执行 AT+CIPSHUT 来改变状态。
void Gprs_CmdPdpDeact(const char* pReq, int reqLen);

//TCP连接关闭，需要重新连接服务器的处理
void Gprs_CmdClosed(const char* pReq, int reqLen);

//设置休眠唤醒响应处理函数
ATCMD_RC Dock_RspCSCLK(AtCmdState state, char* pRsp, AtCmdItem** pNextAtCmd);

//TCP数据处理函数，包括服务器的请求及响应处理
void Gprs_CmdIpData(const char* pReq, int reqLen);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************end of Gprs868.h*****************************************/
