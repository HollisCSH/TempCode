//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: GSMTask.h
//创建人  	: Handry
//创建日期	:
//描述	    : Sim868模块任务定义头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef _GSMTASK_H
#define _GSMTASK_H

//=============================================================================================
//包含头文件
//=============================================================================================
//#include "BSPGPIO.h"
#include "BSP_GPIO.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
#define		UART_WAKEUP_INT_PORT		eIOPortC			//串口唤醒
#define		UART_WAKEUP_INT_PIN		    eIOPin6
#define		UART_WAKEUP_INT_PT		    PORTC			    //用于清除中断
#define		UART_WAKEUP_INT_PN		    6                   //用于清除中断
#define		UART_WAKEUP_PIN_MASK		((u32)1 << UART_WAKEUP_INT_PN)

#define     NB_IOT_VCC_ON_DELAY     (2000 / 50) //2秒延时
//#define     NB_IOT_VCC_ON   BSPGPIOSetPin(VCC_CTL_4G_PORT,VCC_CTL_4G_PIN);
//#define     NB_IOT_VCC_OFF  BSPGPIOClrPin(VCC_CTL_4G_PORT,VCC_CTL_4G_PIN);
#define     NB_IOT_VCC_ON   HAL_GPIO_WritePin(VCC_4G_CTL_PORT, VCC_4G_CTL_PIN, GPIO_PIN_SET);
#define     NB_IOT_VCC_OFF  HAL_GPIO_WritePin(VCC_4G_CTL_PORT, VCC_4G_CTL_PIN, GPIO_PIN_RESET);
//sim pin define
#define     SIM_PWR_KEY_PORT	//eIOPortA
#define     SIM_PWR_KEY_PIN		//eIOPin1
#define     SIM_PWR_KEY_IO		////PA1
#define     SIM_PWR_KEY_ON()	//BSPGPIOSetPin(SIM_PWR_KEY_PORT,SIM_PWR_KEY_PIN);//PA1 = 1	//{PA1 = 1; Printf("POWR_KEY=ON\n");}
#define     SIM_PWR_KEY_OFF()	//BSPGPIOClrPin(SIM_PWR_KEY_PORT,SIM_PWR_KEY_PIN);//PA1 = 0	//{PA1 = 0; Printf("POWR_KEY=OFF\n");}

#define     SIM_PWR_ON_PORT		//eIOPortA//PA    4.1V供电
#define     SIM_PWR_ON_PIN		//eIOPin0//BIT0
#define     SIM_PWR_ON()		NB_IOT_VCC_ON//BSPGPIOSetPin(SIM_PWR_ON_PORT,SIM_PWR_ON_PIN);//PA10 = 1
#define     SIM_PWR_OFF()		NB_IOT_VCC_OFF//BSPGPIOClrPin(SIM_PWR_ON_PORT,SIM_PWR_ON_PIN);//PA10 = 0

#define     SIM_ANT_PWR_PORT	//eIOPortA//PA    GPS天线3.3V供电
#define     SIM_ANT_PWR_PIN		//eIOPin11//BIT11
#define     SIM_ANT_PWR_IO		//PA11
#define     SIM_ANT_PWR_ON()    //BSPGPIOClrPin(SIM_ANT_PWR_PORT,SIM_ANT_PWR_PIN);//(PA11 = 0)
#define     SIM_ANT_PWR_OFF()   //BSPGPIOSetPin(SIM_ANT_PWR_PORT,SIM_ANT_PWR_PIN);//(PA11 = 1)

#define     SIM_WAKEUP_PORT 	//eIOPortA//PA
#define     SIM_WAKEUP_PIN		//eIOPin13//BIT13
#define     SIM_WAKEUP()  		//BSPGPIOClrPin(SIM_WAKEUP_PORT,SIM_WAKEUP_PIN);//(PE5 = 0)
#define     SIM_SLEEP()   		//BSPGPIOSetPin(SIM_WAKEUP_PORT,SIM_WAKEUP_PIN);//(PE5 = 1)

//定时唤醒的2档电压
#define     WAKE_UP_INTIME_VOLT1        3200
#define     WAKE_UP_INTIME_VOLT2        2900
//定时唤醒的2档电压对应的时长
#define     WAKE_UP_INTIME_INTERNAL1    100
#define     WAKE_UP_INTIME_INTERNAL2    200

//定时唤醒的S0C档位
#define     WAKE_UP_INTIME_SOC1        1000
#define     WAKE_UP_INTIME_SOC2        600
#define     WAKE_UP_INTIME_SOC3        200
#define     WAKE_UP_INTIME_SOC4        50
#define     WAKE_UP_INTIME_SOC5        10
#define     WAKE_UP_INTIME_SOC6        0

//定时唤醒的2档SOC对应的时长
#define     WAKE_UP_INTIME_SOC_INTERNAL1    1800
//#define     WAKE_UP_INTIME_SOC_INTERNAL1    10

#define     WAKE_UP_INTIME_SOC_INTERNAL2    7200
//#define     WAKE_UP_INTIME_SOC_INTERNAL2    10//60

#define     WAKE_UP_INTIME_SOC_INTERNAL3    86400
//#define     WAKE_UP_INTIME_SOC_INTERNAL3    10//18000

//=============================================================================================
//定义数据类型
//=============================================================================================
//SIM模组相关变量
//#pragma pack(1) 
typedef struct
{
    u8 NeedWifiFlag;         //需要进行WIFI辅助定位，暂时不可以进入休眠
    
    u8 keepupdate;          //需要保持更新，直到刷新的点数耗尽
    u8 needtlvflag;         //需要更新TLV包到服务器
    u8 tlvupdflg;           //更新到服务器的标志；0：已更新/更新超时，1：请求更新
    u16 tlvfault[4];	    //tlv记录的故障信息
    u8 tlvRemoveSta;        //tlv记录的拆开状态

    //bit0-1：GPS是否成功定位；00：未定位成功；01：定位成功；10：定位超时；11：其他；
    //bit2:SIM卡是否插入；0：SIM卡已插入；SIM卡未插入
    //bit3:SIM卡是否注册失败，停机；0：SIM卡未停机；1：SIM卡已停机
    //bit4-bit15：预留    
    u16 gsmteststat;        //GSM模组测试状态    
    
    u32 RefreshNum;         //刷新点数    
    u32 WakeInternal;       //唤醒间隔
    u8 ParaWriteFlg;        //参数更改标志，标志需要使用这个参数来进行唤醒间隔
    
    u8  gsmcsq;             //GSM模组的CSQ值
    u16 gsmfixtime;         //首次定位时长
    u32 gsmstartsec;        //GSM模组打开GPS定位的秒数
    u32 LbsSec;             //用于查看是否切换基站的定位秒数
    u8  gpssignal;          //GPS信号强度
    u8  sateinview;         //GPS可见卫星数
    
    u8 NeedShdn;            //需要关机
    u8 IsPCBTest;           //PCB测试
    //u8 EnterTestEnvir;      //是否进入测试环境
}t_GSM_COND;
//#pragma pack() 
//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern t_GSM_COND gGSMCond;            //SIM模组相关变量

//=============================================================================================
//声明接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void GSMBIDInit(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SIM868 BID初始化
//注    意	:
//=============================================================================================
void GSMBIDInit(void);

//=============================================================================================
//函数名称	: void GSMInit(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SIM868初始化函数
//注    意	:
//=============================================================================================
void GSMInit(void);

#ifdef BMS_ENABLE_NB_IOT
//=============================================================================================
//函数名称	: void IOCheckNBIoTDetectTask(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: NB模块检测任务
//注    意	: 50ms周期
//=============================================================================================
void IOCheckNBIoTDetectTask(void);
#endif

//=============================================================================================
//函数名称	: void GSMMainTask(void *p)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SIM868主任务函数
//注    意	:
//=============================================================================================
void GSMMainTask(void *p);

#endif

/*****************************************end of GSMTask.h*****************************************/
