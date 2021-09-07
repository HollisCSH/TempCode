//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: AtCmdTab.c
//创建人  	: 
//创建日期	:
//描述	    : Sim模组AT命令表文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SimCommon.h"
#include "AtCmdTab.h"
#include "Fsm.h"
#include "sim.h"
#include "Sms.h"
#include "Gprs868.h"

#ifdef GPRS_FW_UPGRADE
#include "GprsUpg.h"
#endif

//=============================================================================================
//定义全局变量
//=============================================================================================

//AT命令及其响应处理
const AtCmdItem g_GprsCmdTbl[] = 
{
//	 {CMD_RESET 	, "AT+QRST=1" 	, AT_OK, Gprs_RspReset}             //复位
	 {CMD_ATE 		, "ATE0" 		, AT_OK		    , Gprs_RspATE0}     //关闭回显
	,{CMD_CFUN_CLOSE, "AT+CFUN=0" 		, AT_OK		, Gprs_RspCfun}     //关闭注册网络
	,{CMD_QCSEARFCN , "AT+QCSEARFCN" 	, AT_OK		, Null}     //清除记忆频点，清除后，需要至少15秒，才能重新联网
	,{CMD_CFUN_OPEN , "AT+CFUN=1" 		, AT_OK		, Null}     //打开注册网络
    ,{CMD_SET_FORMAT, "AT+QICFG=\"dataformat\",1,1" , AT_OK		, Gprs_Rsp_SET_FORMAT} //配置发送的数据格式为十六进制字符串(<send_data_format>=0)
    
    //SIM 短信SMS
//	,{CMD_CMGF 	 	, "AT+CMGF=1"		 , AT_OK , Null}                //设置短消息格式，TEXT模式
//	,{CMD_GSM		, "AT+CSCS=\"GSM\""	 , AT_OK , Null}                //选择TE字符集，GSM7位默认字符 
//	,{CMU_CSMP		, "AT+CSMP=17,11,0,0"	, AT_OK , Null}             //设置短信 TEXT
//  ,{CMD_GSM		, "AT+CSCS=\"IRA\""	 , AT_OK , Null}                //选择TE字符集，GSM7位默认字符 
#ifdef INTERNATIONAL_CARD   //是否使用国际SIM卡
#else
//    //国际sim卡若此处设置了无法发送短信
//	,{CMD_CSCA 	 	, "AT+CSCA=\"13800100569\"" , AT_OK , Sms_CmdGSM}   //设置短信中心地址
#endif
    
    //查询网络状态
	,{CMD_CREG 		, "AT+CEREG?" 	, AT_OK		, Gprs_RspCREG}         //查询网络注册情况
	,{CMD_CGATT		, "AT+CGATT?"	, AT_OK		, Gprs_RspCGATT}        //查询是否附着网络
    ,{CMD_CGPADDR 	, "AT+CGPADDR?" , AT_OK		, Null}                 //找网成功后，可通过此命令获取模块 IP 地址
	,{CMD_CCID 		, "AT+QCCID"	, "+QCCID: ", Gprs_RspCCID}         //获取SIM卡CCID
	
#ifdef INTERNATIONAL_CARD   //是否使用国际SIM卡
	,{CMD_SOCKCFG	, "AT+CSTT=\"globaldata\"", AT_OK	, Null}
#else
//	,{CMD_SOCKCFG	, "AT+CSTT=\"CMNET\"", AT_OK	, Null}             //设置 APN，移动的 APN 为 “CMNET”
#endif
    
//	,{CMD_CIICR		, "AT+CIICR"	, AT_OK		, Null}                 //建立无线链路（激活移动场景）
//	,{CMD_CIFSR		, "AT+CIFSR"	, "."		, Null}                 //获取本地 IP 地址
    
	,{CMD_CIPOPEN 	, "AT+QIOPEN=0,0,\"TCP\",\"%s\",%d,0,1"	, AT_OK, Gprs_RspCIPOPEN}  	//建立 TCP 连接
	,{CMD_CIPSEND 	, "AT+QISEND=0,%d,\"%s\""	, "\r\nSEND OK\r\n"	, Gprs_RspCIPSENDDATA}      //发送数据到服务器
//	,{CMD_CIPSENDDATA, ""			, "\r\nSEND OK\r\n"	, Gprs_RspCIPSENDDATA}  //代表数据发送成功
	
	,{CMD_IS_CIPOPEN, "AT+QISTATE=1,0"	, "\r\n+QISTATE:"		, Gprs_RspCIPSTATUS}    //查询当前连接状态
	,{CMD_NETCLOSE 	, "AT+QICLOSE=0"		, "\r\nCLOSE OK\r\n"	, Gprs_RspNETCLOSE}	//关闭 TCP 或 UDP 连接
//	,{CMD_SHUTDOWN 	, "AT+CIPSHUT"		, "\r\nSHUT OK\r\n"	, Gprs_RspSHUTDOWN}		//关闭移动场景
	
	,{CMD_CSQ  		, "AT+CSQ"   		, AT_OK		, Gprs_RspCSQ}          //查询信号质量
	,{CMD_GATT_SET 	, "AT+CGATT=1"		, AT_OK		, Gprs_RspCGATT_SET}    //设置GPRS附着网络
	
#ifdef CFG_SIM_SLEEP
    ,{CMD_SIM_CSCLK,  "AT+QSCLK=%d"		, AT_OK		, Dock_RspCSCLK}        //设置休眠唤醒1：接收到短消息（SMS）唤醒
    
//	,{CMD_CMGD		, "AT+CMGDA=\"DEL READ\""		, AT_OK	, Sms_RspCMGD}  //删除已读短消息
//	,{GPS_CMD_CMGL	, "AT+CMGL=\"REC UNREAD\""		, AT_OK	, Sms_RspCMGL}  //列举短消息
//	,{CMD_CMGS		, "AT+CMGS=\"0064899102515\""	, "!>"	, Gprs_RspSMGS} //发送短信  
//	,{CMD_SMS_TEXT	, "Test\x1A"		, AT_OK		, Gprs_RspSMGSTEXT}     //输入短信数据
#endif		

    #if 0
	//Gps命令
	,{CMD_CGNSPWR 	, "AT+CGNSPWR=%d"	, AT_OK		, Gps_RspCGNSPWR}       //打开或关闭GPS
	,{GPS_CGNSCMD	, "AT+CGNSCMD=0,\"$PMTK353,1,0,0,0,1*2B\"", AT_OK, Bunker_RspCGNSCMD}	//给 GNSS 发送控制命令，GPS+北斗
//	,{GPS_CGNSCMD	, "AT+CGNSCMD=0,\"$PMTK353,1,1,0,0,0*2B\"", AT_OK, Bunker_RspCGNSCMD}	//给 GNSS 发送控制命令，GPS+GLONASS
//	,{GPS_CGNSCMD	, "AT+CGNSCMD=0,\"$PMTK353,0,1,0,0,0*2A\"", AT_OK, Bunker_RspCGNSCMD}	//给 GNSS 发送控制命令，GLONASS
//	,{GPS_CGNSCMD	, "AT+CGNSCMD=0,\"$PMTK353,0,0,0,0,1*2A\"", AT_OK, Bunker_RspCGNSCMD}	//给 GNSS 发送控制命令，BD
//	,{GPS_CGNSCMD	, "AT+CGNSCMD=0,\"$PMTK353,1,0,0,0,0*2A\"", AT_OK, Bunker_RspCGNSCMD}	//给 GNSS 发送控制命令，GPS

    ,{GPS_CMD_CGPSINFOCFG, "AT+CGNSINF"	, AT_OK, Gps_RspCGNSINF}             //获取GPS信息
    ,{CMD_ATS0,"ATS0=1", AT_OK, NULL}                                        //设置来电自动接听,一声后自动接听
    ,{GPS_CMD_SENDNEMA,    "AT+CGNSTST=1", AT_OK, Gps_RspSendNEMA}           //打印GPS NEMA信息	
    
    //使用基站进行定位
	,{CMD_SAPBR1	, "AT+SAPBR=3,1,\"Contype\",\"GPRS\"", AT_OK, Null}
	,{CMD_SAPBR2	, "AT+SAPBR=3,1,\"APN\",\"CMNET\""	 , AT_OK, Null}
	,{CMD_SAPBR3	, "AT+SAPBR=1,1"	 				 , AT_OK, Null}
	,{CMD_SAPBR4	, "AT+SAPBR=2,1"	 				 , AT_OK, Null}   
    //基站定位命令
	,{CMD_CLBS2	        , "AT+CLBS=4,1 "	 			, AT_OK, Lbs_RspCLBS}           
	,{CMD_SAPBR_CLOSE	, "AT+SAPBR=0,1"	 			, AT_OK, Lbs_RspCloseIP}
	#endif
	,{Null}
};

//服务器的请求或接受的URC处理
const AtReqItem g_SimReq[] = 
{
	 {"+QIURC: \"recv\",0,"		, Gprs_CmdIpData}	//NB-IOT 接收服务器的TCP数据处理，+QIURC: "recv",0,3,"012"
	
//	,{"+CPIN: NOT"		, Gprs_CmdSimCardEvent}	    //NB-IOT SIM CARD IS PLUT OUT
	,{"\r\n+QIOPEN:"		, Gprs_CmdConnect}      //NB-IOT 网络连接ok
	,{"+PDP: DEACT"		, Gprs_CmdPdpDeact}         //标识 GPRS 被网络释放， 此时仍然需要执行 AT+CIPSHUT 来改变状态。
	,{"+QIURC: \"closed\"", Gprs_CmdClosed}         //NB-IOT TCP连接关闭，需要重新连接服务器
	
#ifdef CFG_SIM_SLEEP
	,{"\r\n+IP:"	, Sms_CmdSmsReady}	            //NB-IOT  SIM卡上电成功，ready
//	,{"+CMTI: \"SM\","	, Sms_CmdCMTI}	            //+CMTI: "SM",19，sm卡收到了短信
//	,{"+CMTI: \"ME\","	, Sms_CmdCMTI}	            //+CMTI: "SM",19，me设备收到了短信
#endif	
	
	,{Null}
};

/*****************************************end of AtCmdTab.c*****************************************/
