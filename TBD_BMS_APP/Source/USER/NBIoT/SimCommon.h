//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Common.h
//创建人  	: Handry
//创建日期	:
//描述	    : Sim模组公共文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef __SIM_COMMON_H_
#define __SIM_COMMON_H_

#ifdef __cplusplus
extern "C"{
#endif

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTypeDef.h"	
#include "String.h"
#include <stdio.h>
#include <stdlib.h>
#include "GSMTask.h"
#include "version.h"
#include "TestMode.h"
#include "BSPTask.h"
//#include "BSPSCI.h"
#include "BSP_UART.h"
#include "SwTimer.h"
#include "math.h"
#include "Queue.h"
#include "queue_old.h"
#include "CheckSum.h"
#include "Datetime.h"
#include "Debug.h"
#include "Sign.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
#define     GSM_UART_CHANNNEL       USART1

#define     OPEN_GPS_BEFORE_GSM     1   //开启GPS→设置GPS→登陆服务器流程

#define     SIM868
#define     X_MODEM
//#define   XDEBUG
#define     CONFIG_CMDLINE
#define     HW_BUNKER      

#define     USERID_LEN              16
#define     SID_LEN		            12
#define     CCID_LEN  	            20
#define     MD5_KEY_LEN             16

#define     MAX_MSG_COUNT		    60	//Message queue deepth
#define     MAX_TIMER_COUNT		    4
#define     MS_STICKS 			    1	//Hot many ticks per ms, it is for timer
#define     TIMER_TASK_INTERVAL     1
#define     MAX_TEMP_BUF_SIZE 	    512

#define     COM_BUF_SIZE            512			//必须要 > 1024,该值用于
#define     LEARN_MODE_MAX_SPEED    6		    //学习模式的最大限速
#define     LEARN_MODE_MAX_TRIP     1000	    //学习模式的最大里程 

#define     PI                      3.1415926
#define     EARTH_RADIUS            6378.137        //地球近似半径

#if defined HW_BUNKER		//电池仓
//	#define GPRS_FW_UPGRADE
	#define CFG_LOGIN_TIME
	#define CFG_CYC_GET_BAT
	#define CFG_GET_PORT
	#define CFG_BLE_SLEEP
	#define CFG_GYRO 
	#define CFG_SIM_SLEEP
    
    #if defined(SY_PB_32000MAH_17S)
//    #define IP_ADDR_TEST 	 "30810e1m92.wicp.vip"
//	#define IP_PORT_TEST 	 41191
	#define IP_ADDR_TEST 	 "test-ehd-battery-netty.ehuandian.net"
	#define IP_PORT_TEST 	 9005
    #elif defined(LFP_TB_20000MAH_20S)
    #define IP_ADDR_TEST 	 "test-ehd-battery-netty.ehuandian.net"
	#define IP_PORT_TEST 	 9005
    #else
	#define IP_ADDR_TEST 	 "test-ehd-battery-netty.ehuandian.net"
	#define IP_PORT_TEST 	 9005  
    #endif

#ifdef TEST_SRV
    #define IP_ADDR 		 IP_ADDR_TEST
    #define IP_PORT 		 IP_PORT_TEST
#else
    #define IP_ADDR 		 "ehd-battery-netty.ehuandian.net"
    #define IP_PORT 		 9005
#endif

	#define BLE_MF_STR 		 "30"
	#define IMMOTOR_KEY_WORD "IMT60"
	#define TIME_UPDATE_GPS_TO_SERVER 	(5*60*1000)

#else
	#error Must define a target above
#endif

#define MDELAY(mS) TIMER_Delay(TIMER0, (mS)*1000)
#define UDELAY(uS) TIMER_Delay(TIMER0, (uS))

extern volatile unsigned int system_ms_tick;
#define GET_TICKS( ) system_ms_tick

#define     strtoken(strDst, separator, strArray, count) str_tok(strDst, separator, 0, strArray, count)
#define     PostMsgEx(...);
#define     IS_FOUND(_str1, _str2) strstr((char*)(_str1), (char*)(_str2))

#define     ASRT_TURE(parenExpr) if(!(parenExpr))	\
            {                                   \
                Printf( "Assertion Failed! %s,%s,%s,line=%d\n", #parenExpr,__FILE__,__FUNCTION__,_LINE_);	\
                while(1){;}	\
            }
//#define     Assert ASRT
#define     GET_ELEMENT_COUNT(array) (sizeof(array)/sizeof(array[1]))
#define     READ_REG32(reg) (uint32)(*(vuint32*)(reg))
#define     WRITE_REG32(reg, vvalue) (*(vuint32*)(reg)) = vvalue
#define     MODIFY_REG32(reg, clear_mask, set_mask) ((*(vuint32*)(reg)) = ((*(vuint32*)(reg)) &~ clear_mask) | set_mask)
#define     MIN(v1, v2) ((v1) > (v2) ? (v2) : (v1))
#define     MAX(v1, v2) ((v1) < (v2) ? (v2) : (v1))

//=============================================================================================
//定义数据类型
//=============================================================================================
typedef enum _MSG_ID
{
	 MSG_TIMEOUT = 0
	 	
	,MSG_SIM_FAILED
	
	,MSG_GPS_UPDATE	
	,MSG_GPRS_UPDATE
	,MSG_WIFI_MAC_UPDATE
	
	,MSG_PMS_WAKEUP
	,MSG_PMS_SLEEP
	
 	,MSG_BATTERY_PLUG_IN
 	,MSG_BATTERY_PLUG_OUT
 	
 	,MSG_CHARGER_PLUG_IN
 	,MSG_CHARGER_PLUG_OUT
 	
 	,MSG_RTC_TIMEOUT
 	,MSG_FW_UPGRADE
	,MSG_SIM_WAKEUP_IN_TIME //定时唤醒
	,MSG_SIM_ACTIVE_WAKEUP  //激活唤醒
	
	,MSG_LOCK_KEY
	
	,MSG_GPS_POWERON
	,MSG_GPS_POWEROFF
	,MSG_SIM_POWERON	
	,MSG_SIM_POWEROFF	
 	,MSG_FW_UPGRADE_DONE
	
	,MSG_SIM_COMM		
	,MSG_SIM_CARD_READY			
	,MSG_BLE_WAKEUP 	
 	,MSG_BATTERY_FAULT
 	
	,MSG_GPS_MOVE
	,MSG_GPS_STOP
	,MSG_ADC_ISR
	,MSG_SPEED_15
	,MSG_SPEED_25
	,MSG_BEACON_UPDATE
	
	,MSG_CABIN_LOCK_CHANGED
    ,MSG_TLV_HAPPEN     //TLV包发生
    ,MSG_TLV_UPDATE     //TLV包已更新到服务器
    ,MSG_PCBA_TEST_START      //PCB测试开始
    ,MSG_PCBA_TEST_END        //PCB测试结束
 	,MSG_MAX

}MSG_ID;

#define TC_GPS_INIT_FAIL				30
#define TC_GPS_START_FAIL				31
#define TC_GPS_ANTENNA_OPEN				32
#define TC_GPS_ANTENNA_SHORT			33
#define TC_GPS_NOT_FIX					34

#define TC_GPRS_INIT_FAIL				40
#define TC_GPRS_SIMCARD_INVALID			41
#define TC_GPRS_NO_SINGAL				42
#define TC_GPRS_NETWORK_REG_FAIL		43
#define TC_GPRS_CONNECT_SERVER_FAIL		44		
#define TC_GPRS_REG_DENIED				46			//过期

enum
{
	ERR_TYPE_SYSTEM = 0x00,
	ERR_TYPE_MOTOR1,
	ERR_TYPE_MOTOR2,
	ERR_TYPE_BATTERY1,
	ERR_TYPE_BATTERY2,
	ERR_TYPE_GPRS,
	ERR_TYPE_GPS,
	ERR_TYPE_BT,
	ERR_TYPE_GROY,
	ERR_TYPE_MAX
};

typedef enum _RetCode
{
	 RC_SUCCESS 		= 0
	,RC_FAILED 			= 1
	,RC_PENDING 		= 2
	,RC_BAD_PARAMETER 	= 3
	,RC_UNKNOWN 		= 4
	,RC_DONOTHING 		= 5
	,RC_ABORTED 		= 6
	,RC_WRONG_PHASE 	= 7
	,RC_NO_RESOURCE 	= 8
	,RC_BUSY 			= 9
	,RC_DONE 			= 10
}RetCode;

//活动标志定义，当该标志被置位时，设备不能进入睡眠。
typedef enum _ActiveFlag
{
	AF_BLE = BIT_0	//蓝牙处于连接状态
   ,AF_MCU_RESET = BIT_1	//MCU处于延时复位状态
   ,AF_NVC = BIT_2			//NVC正在播放语音
   ,AF_SIM = BIT_8			//SIM模组有命令没有发完
   ,AF_CARBIN_LOCK = BIT_9	//正在操作座舱锁
		
   ,AF_PMS = BIT_16			//PMS
   ,AF_BEACON = BIT_17		//BEACON是否连接状态
   ,AF_FW_UPGRADE = BIT_24	//正在启动升级操作
}ActiveFlag;

//复位原因
typedef enum _MCURST
{
	 MCURST_PWR				//
	,MCURST_MODIFY_FW_VER	//修改固件版本号
	,MCURST_MODIFY_HW_VER	//修改硬件版本号
	,MCURST_MODIFY_SVR_ADDR	//修改服务器地址
	,MCURST_FWUPG_ERROR		//固件升级失败
	,MCURST_GPRS			//网络远程复位
	,MCURST_UPG_OK			//升级固件成功

	,MCURST_BLE_CMD			//收到蓝牙复位命令
	,MCURST_FACTORY_SETTING	//恢复出厂设置
	,MCURST_18650_POWER_OFF	//切断18650供电
	,MCURST_DEV_ACTIVE		//设备激活
	,MCURST_ALL_BAT_PLUG_OUT//拔出所有电池
	,MCURST_SMS				//短息复位
	,MCURST_PMS_FW_UPGRADE	//pms固件升级成功
	 
	,MCURST_ADC_ISR_INVALID	//ADC中断故障，不能触发
	,MCURST_QUEUE_FULL		//消息队列满
}MCURST;

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern uint8 g_CommonBuf[COM_BUF_SIZE];
extern uint8 g_FirmwareVer[];

extern uint8 g_isGprsRun;
extern uint8 g_isGpsRun;
extern uint8 g_isLbsRun;
extern uint8 g_isMpuRun;
extern Bool g_isMcuLowPower;
extern uint16 g_AdcForBat;
extern uint16 g_speed;

//=============================================================================================
//声明外部接口函数
//=============================================================================================
extern char _getc(void);
#define GET_CHAR _getc

extern void _putc (const char c);
#define PUT_CHAR _putc

//=============================================================================================
//声明接口函数
//=============================================================================================
// 求弧度
double radian(double d);

//计算距离单位KM
double get_distance(double lat1, double lng1, double lat2, double lng2);
    
//判断当前字节是否可打印
Bool IsPrintChar(uint8 byte);

//将pDst字符串的startStr与endStr之间的字符串复制到pSrc中
char* strcpyEx(char* pSrc, const char* pDst, const char* startStr, const char* endStr);

//根据目标子字符串寻找源字符串对应的指针位置，并返回当前的位置
uint8* bytesSearch(const uint8* pSrc, int len, const char* pDst);

//根据目标子字符串寻找源字符串对应的指针位置，并返回目标子字符后面的位置
uint8* bytesSearchEx(const uint8* pSrc, int len, const char* pDst);

//查找子字符串pDst在pSrc的位置，如果没有找到，则返回0，如果找到，则返回pSrc的pDst位置后后面
char* strstrex(const char* pSrc, const char* pDst);

//找到目标字符串之后，用空格替换它，使之下次找不到(仅处理一次)。
char* strstr_remove(char* pSrc, const char* pDst, char**pValue);

//功能:把一个字符串根据separator分割成为多个字符串。和strtok不同的是，每一个分隔符都会被分割成为字符串。
int str_tok(char* strDst, const char* separator, int startInd, char* strArray[], int count);

//根据分隔符查询字符串后的索引值
int strtokenValueByIndex(char* strDst, const char* separator, int index);

//字节转化为二进制的字符串
char* ByteToBinStr(uint8 byte, char* str);

//多个字节转化为二进制的字符串，中间用空格隔开
char* ToBinStr(void* pData, int len, char* str);

//将16进制字符转为16进制数，例如"0x12"转为数字18
int str_htoi(const char *s);

//"A1"->0xA1
Bool hexToByte(const char* s, uint8* val);

//"1234A2FF" =>0x1234A2FF,并放在buff中
Bool hexStrToByteArray(const char* s, int len, uint8* pBuf);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************end of Common.h*****************************************/
