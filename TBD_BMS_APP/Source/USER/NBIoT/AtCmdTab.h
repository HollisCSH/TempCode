//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: AtCmdTab.h
//创建人  	: 
//创建日期	:
//描述	    : Sim模组AT命令表头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================
//=============================================================================================
//包含头文件
//=============================================================================================
#include "AtCmdCtrl.h"
#include "SimCommon.h"

//=============================================================================================
//定义数据类型
//=============================================================================================
enum
{
    CMD_RESET    = 0
	,CMD_ATE
    ,CMD_CFUN_CLOSE
    ,CMD_QCSEARFCN
    ,CMD_CFUN_OPEN
    ,CMD_SET_FORMAT //设置发送/接收的数据格式
    
	,CMD_CGNSPWR  
	,CMD_CPIN  
	,CMD_CSQ    
	,CMD_CREG   
    ,CMD_CGPADDR    //找网成功后，可通过此命令获取模块 IP 地址。
	,CMD_CGATT   
	,CMD_CCID   
	,CMD_SOCKCFG
    
    ,CMD_ATS0       //设置响铃自动接听
	
	,CMD_CIICR
	,CMD_CIFSR
	
	,CMD_IS_CIPOPEN
	,CMD_CIPOPEN
	
	,CMD_CIPSEND
	,CMD_CIPSENDDATA
	
	,CMD_NETCLOSE
	,CMD_SHUTDOWN
	,CMD_GATT_SET
	
	,CMD_SIM_CSCLK
	
	,CMD_AT_PASSTHROUGH

	//Gps cmd
	,GPS_CMD_CGPSINFOCFG
    ,GPS_CMD_SENDNEMA       //GPS打印NEMA信息
	,GPS_CGNSCMD
	,GPS_CGNSAID 
	
#ifdef CFG_SIM_SLEEP
	//SMS cmd
	,CMD_CMGS
	,CMD_CSCA
	,CMD_CMGD
	,CMD_GSM
	,CMD_CMGF
    //测试
    //,CMU_CSMP
	,GPS_CMD_CMGL
	,CMD_SMS_TEXT
#endif

	,CMD_SAPBR1
	,CMD_SAPBR2
	,CMD_SAPBR3
	,CMD_SAPBR4	
    
    ,CMD_CNTPCID    //设置 GPRS 承载场景 ID
    ,CMD_SETCNTP    //设置 NTP 服务 URL
    ,CMD_CHECKCNTP  //检查 NTP 服务 URL
    ,CMD_CNTP       //同步网络时间
    ,CMD_CCLK       //查询网络时间
    
    ,CMD_CLBS1
    ,CMD_CLBS2
    
    ,CMU_FSLS       //查询是否存在EPO文件
    ,CMU_CGNSCHK    //查询EPO文件是否有效   
    
    ,CMD_CGNSSAV    //设置 HTTP 下载模式并将 EPO 保存到 FS
	,CMD_SAPBR_CLOSE
	,CMD_HTTPINIT
	,CMD_HTTPPARA1
	,CMD_HTTPPARA2
	,CMD_HTTPACTION    
	,CMD_HTTPTERM
    
	,CMD_MAX
};

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
//AT命令及其响应处理
extern const AtCmdItem g_GprsCmdTbl[];

//服务器的请求或接受的URC处理
extern const AtReqItem g_SimReq[];


/*****************************************end of AtCmdTab.c*****************************************/
