//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SH367309.h
//创建人  	: Handry
//创建日期	: 
//描述	    : SH367309驱动头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef _SINGLE_SH367309_H_
#define _SINGLE_SH367309_H_

//=============================================================================================
//头文件
//=============================================================================================
#include "BSPIIC.h"
#include "BatteryInfo.h"
#include "SH367309Def.h"
#include "BSPGPIO.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
#define		SH367309_WRITE_EEP_ON		BSPGPIOSetPin(SH367309_VPRO_PORT,SH367309_VPRO_PIN);
#define		SH367309_WRITE_EEP_OFF      BSPGPIOClrPin(SH367309_VPRO_PORT,SH367309_VPRO_PIN);

#define		SH367309_SHIP_ON		    BSPGPIOClrPin(SH367309_SHIP_PORT,SH367309_SHIP_PIN);
#define		SH367309_SHIP_OFF           BSPGPIOSetPin(SH367309_SHIP_PORT,SH367309_SHIP_PIN);

//----------------------------SH367309 IIC通信相关---------------------------------------------------
#define		SH367309_IIC_CHANNEL		eIIC0	//RTC使用IIC0通道

#define		SH367309_WRITE_ADDR			0x34	//读取8位地址
#define		SH367309_READ_ADDR			SH367309_WRITE_ADDR | 0x01

#define		SH367309_IIC_NOFAULT_STAT	0		//IIC通信无错误状态
#define		SH367309_IIC_FAULT_STAT		1		//IIC通信错误状态
#define		SH367309_IIC_CRCFAULT_STAT	2		//IIC通信CRC校验错误状态

#define		SH367309_READBUFF_LEN		107		//读数组缓存长度
#define		SH367309_WRITEBUFF_LEN		107		//写数组缓存长度

#define		SH367309_COMM_FAULT_CNT		100
#define		SH367309_MAX_WAIT_TIMEOUT	50000	//最大等待时间，超过这个时间，直接回复错误信号
#define		SH367309_MAX_TIMEOUT		10000	//最大等待时间，超过这个时间，直接回复错误信号
#define		SH367309_PT_MAX_TIMEOUT		400		//最大等待时间，超过这个时间，直接回复错误信号

#define		SH367309_REG_NUM			54		//SH367309寄存器个数
#ifdef BMS_BAT_16S
#define 	SH367309_VOLT_NUM    		16		//IC读取单体电压信息最大节数 
#elif defined(BMS_BAT_15S)
#define 	SH367309_VOLT_NUM    		15		//IC读取单体电压信息最大节数
#elif defined(BMS_BAT_14S)
#define 	SH367309_VOLT_NUM    		14		//IC读取单体电压信息最大节数
#endif
#define 	SH367309_TEMP_NUM    		3		//IC读取温度信息最大节数

//判断是13串还是16串的电池
#if 		(13 == BAT_CELL_NUM)
#define		SH367309_VALID_BAT			0X5EFF	//采样有效位，位为1则代表有效
#elif defined(BMS_BAT_14S)
#define		SH367309_VALID_BAT			0X3FFF
#elif defined(BMS_BAT_15S)
#define		SH367309_VALID_BAT			0X7FFF
#elif defined(BMS_BAT_16S)
#define		SH367309_VALID_BAT			0XFFFF
#elif defined(BMS_BAT_20S)
#define		SH367309_VALID_BAT			0X000FFFFF
#endif

//----------------------------SH367309 MOS管控制---------------------------------------------------
#define 	SH367309_CHG_MOS_SEL          	0	//充电MOS管
#define 	SH367309_DCHG_MOS_SEL         	1	//放电MOS管

#define 	SH367309_MOS_ON          		1	//MOS管闭合状态
#define 	SH367309_MOS_OFF        			0	//MOS管断开状态

#define 	SH367309_CC_READY_MASK    		    0x80
#define 	SH367309_SAMPLE_RES 				1250  //uR
#define 	SH367309_SAMPLE_RES_CAL_MAX 		1300  //uR
#define 	SH367309_SAMPLE_RES_CAL_MIN 		900

//----------------------------SH367309 任务事件标志---------------------------------------------------
#define 	SH367309_EVE_CTRL_EN 			0x0001	//使能
#define 	SH367309_EVE_CTRL_BAL 			0x0002	//均衡
#define 	SH367309_EVE_CTRL_MOS 			0x0004	//mos管控制
#define 	SH367309_EVE_CTRL_SLP 			0x0008	//睡眠
#define 	SH367309_EVE_CTRL_OSCD 			0x0010	//OCD、SCD配置
#define 	SH367309_EVE_CTRL_OUVD 			0x0020	//OV、UV配置
#define 	SH367309_EVE_CTRL_CAL 			0x0040	//电流标定
#define 	SH367309_EVE_CTRL_SMP 			0x0080	//采样
#define 	SH367309_EVE_CTRL_DELAYCTRL 	0x0200	//保护延时

//EEP配置参数
#define 	SH367309_CFG_BALV 	            3000	//平衡开启电压，超过此电压才会开启均衡
#define 	SH367309_CFG_PREV 	            2500	//预充电电压设定值
#define 	SH367309_CFG_L0V 	            1000	//低压禁止充电电压设定值
#define 	SH367309_CFG_PFV 	            3800	//二次过压充电保护电压

#define     SH367309_CFG_OT_OFFSET          0     //高温比BMS软件保护偏移的℃
#define     SH367309_CFG_UT_OFFSET          100   //低温比BMS软件保护偏移的℃

#if defined(SY_PB_32000MAH_14S) || defined(SY_PB_32000MAH_16S)
#define 	SH367309_CFG_OCC	            4000	//充电过流保护配置 40A
#elif defined(LFP_HL_25000MAH_16S)
#define 	SH367309_CFG_OCC	            3000	//充电过流保护配置 20A
#else
#define 	SH367309_CFG_OCC	            2000	//充电过流保护配置 20A
#endif

//----------------------------SH367309 任务事件标志---------------------------------------------------
#define     SH367309_REF_RES_VAL	        680
#define     SH367309_CALICUR                200000

//----------------------------清除SH367309系统状态---------------------------------------------------
#define     SH367309_CLR_WDT                (0x80)
#define     SH367309_CLR_PF                 (0x40)
#define     SH367309_CLR_SC                 (0x20)
#define     SH367309_CLR_OCC                (0x10)
#define     SH367309_CLR_OCD2               (0x08)
#define     SH367309_CLR_OCD1               (0x04)
#define     SH367309_CLR_UV                 (0x02)
#define     SH367309_CLR_OV                 (0x01)

#define     SH367309_STAT_WDT               (0x80)
#define     SH367309_STAT_PF                (0x40)
#define     SH367309_STAT_SC      	        (0x20)
#define     SH367309_STAT_OCC              	(0x10)
#define     SH367309_STAT_OCD2              (0x08)
#define     SH367309_STAT_OCD1             	(0x04)
#define     SH367309_STAT_UV             	(0x02)
#define     SH367309_STAT_OV             	(0x01)


typedef enum
{
    e_SH_AFE_Sta_Init = 0,
    e_SH_AFE_Sta_Disable = 1,
    e_SH_AFE_Sta_Bqinit = 2,
    e_SH_AFE_Sta_Norm = 3,
    e_SH_AFE_Sta_Offline = 4,
}e_SH_AFESTA;

//SH_AFE数据/状态结构体
typedef struct __SH_AFE_DATA
{
    u8 afechn;                          //SH367309当前IIC通道
    u32 currtime;			            //SH367309当前时间
    e_SH_AFESTA runstatus;		        //SH367309运行状态
    u8 ResetFlg;                        //SH367309复位标志
    u8 IntFlg;                          //SH367309发生告警中断标志
    u8 MosUptFlg;                       //SH367309更新MOS标志
    u8  initerr;    	  	            //SH367309初始化异常标志
    u16 iicerr;			                //SH367309通信错误计数器
    s8  iicres;		                	//阻塞时使用变量，用于等待iic通信的结果
    u8  faultcnt;			  			//SH367309故障标志
    u8  devsta;				  			//SH367309 标志
    u8  devfault;				  		//SH367309 设备状态错误，bit0：xready故障；bit1：over alert故障
    u8  protflag;				  		//SH367309 保护标志，1：代表需要保护；0：不需要保护
    u8  mosstatus;			  			//SH367309 mos管状态，bit0：充电mos；bit1：放电mos
    u16	cellvolt[SH367309_VOLT_NUM];	//SH367309 单体电压采样值，1mv
    s16 celltemp[SH367309_TEMP_NUM]; 	//SH367309 单体温度采样值，0.1℃
    s32 curr;							//SH367309 电流采样值，1ma
    s32 curradval;						//SH367309 电流采样ad值
}t_SH_AFE_Data_Status;

//SH_AFE控制相关结构体
typedef struct __SH_AFE_CTRL
{
    u16 eventctrl;			  	        //SH367309 事件控制，参考事件标志宏定义表
    u16 balctrl;			  			//SH367309 均衡控制，每1bit为1代表开均衡
    u8 	mosctrl;			  			//SH367309 MOS管控制，bit0：充电MOS；bit1：放电MOS
}t_SH_AFE_Ctrl;

//AFE配置数据相关结构体
typedef struct _SH_AFE_CFG
{
    u16 adcgain;                        //SH367309的ad增益
    s8  adcoffset;                      //SH367309的ad偏移
    s16 Cadcoffset;                     //SH367309的CADC偏移
    s16 Cadcgain;                       //SH367309的CADC增益
    u16	calres;				  		    //SH367309 分流器电阻
    s16 calcurr;						//SH367309校准的电流值，0.01a/bit
    s16 calv16;							//SH367309校准的第16节单体电压值，1mv
    u16 ovdval;							//SH367309一般过压配置值，1mv
    u16 uvdval;							//SH367309一般欠压配置值，1mv
    u16 scdval;							//SH367309短路电流配置值
    u16 ocd1val;						//SH367309放电过流配置值
    u16 ocd2val;						//SH367309放电过流配置值
}t_SH_AFE_CfgData;

typedef struct
{
    uint8_t IsModTemp;	//是否进入测试模式更改温度
    int16_t ModTempVal;	//更改的温度的值
}t_AFE_Test;

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern t_SH_AFE_Data_Status gSHAFEData;			            //SH367309采样数据/状态相关结构体
extern t_SH_AFE_Ctrl gSHAFECtrl;                               //SH367309控制相关结构体
extern t_SH_AFE_CfgData gSHAFECfg;                             //SH367309配置数据相关结构体
extern t_SH367309_Register gSHAFEReg;                          //SH367309寄存器结构体
extern t_AFE_Test g_AFE_Test_VAR;                              //修改温度

//=============================================================================================
//声明接口函数
//=============================================================================================
//=============================================================================
//函数名称: void SH367309Init(void)
//输入参数: void
//输出参数: void
//功能描述: SH367309上电初始化
//注意事项:
//=============================================================================
void SH367309Init(void);

//=============================================================================================
//函数名称	: void SH367309EventFlagSet(u16 eventflag)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SH367309主任务:任务标志置位
//注    意	:
//=============================================================================================
void SH367309EventFlagSet(u16 eventflag);

//=============================================================================================
//函数名称	: void SH367309EventFlagSet(u16 eventflag)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SH367309主任务:任务标志置位
//注    意	:
//=============================================================================================
void SH367309EventFlagClear(u16 eventflag);

//=============================================================================================
//函数名称	: u16 SH367309EventFlagGet(void)
//输入参数	: void
//输出参数	: SH367309任务标志位
//静态变量	: void
//功    能	: 获取SH367309任务标志位
//注    意	:
//=============================================================================================
u16 SH367309EventFlagGet(void);

//=============================================================================================
//函数名称	: void SH367309MosfetSet(uint8_t mos)
//输入参数	: mos:mos管控制状态
//输出参数	: void
//静态变量	: void
//功    能	: SH367309设置MOS控制状态
//注    意	:
//=============================================================================================
void SH367309MosfetSet(u8 mos);

//=============================================================================================
//函数名称	: u8 SH367309MosfetGet(void)
//输入参数	: void
//输出参数	: MOS控制状态
//静态变量	: void
//功    能	: SH367309获取MOS控制状态
//注    意	:
//=============================================================================================
u8 SH367309MosfetGet(void);

//=============================================================================================
//函数名称	: void SH367309DevStatClear(u8 mask)
//输入参数	: mask：需要清除的位
//输出参数	: void
//静态变量	: void
//功    能	:SH367309清除设备状态标志
//注    意	:
//=============================================================================================
void SH367309DevStatClear(u8 mask);

//=============================================================================
//函数名称: u8 SH367309OUVConfig(s16 ov,s16 ovr,s16 uv,s16 uvr)
//输入参数: ov:过压保护值，ovr:过压恢复值，uv：欠压保护值，uvr:欠压恢复值
//输出参数: 配置结果 成功:SH367309_IIC_NOFAULT_STAT;  失败:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//功能描述: SH367309过压、欠压配置
//注意事项:
//=============================================================================
u8 SH367309OUVConfig(s16 ov,s16 ovr,s16 uv,s16 uvr);

//=============================================================================
//函数名称: u8 SH367309OCConfig(u16 ocd1value,u16 ocd2value,u16 scdvalue,u16 occvalue)
//输入参数: ocd1value：放电过流1保护，ocd2value：放电过流2保护，scdvalue:放电短路电流保护值	occvalue：充电过流保护值
//输出参数: 配置结果 成功:SH367309_IIC_NOFAULT_STAT;  失败:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//功能描述: SH367309 配置充放电过流、短路配置
//注意事项:
//=============================================================================
u8 SH367309OCConfig(u16 ocd1value,u16 ocd2value,u16 scdvalue,u16 occvalue);

//=============================================================================
//函数名称: void SH367309CalCurrent(s16 actcurrent)
//输入参数: actcurrent：校准的电流
//输出参数: void
//功能描述: SH367309校准采样电流
//注意事项: void
//=============================================================================
void SH367309CalCurrent(s16 actcurrent);

//=============================================================================================
//函数名称	: u8 SH367309DirectReadData(u16 *volt,s16 *temp,s16 *curr);
//输入参数	: volt:放置的电压数组指针
//输出参数	: 是否有误：0：无错误；1：读取iic错误；2：校验crc错误
//功能描述	: 直接获取AFE采样的数据，不阻塞
//注意事项	:
//=============================================================================================
u8 SH367309DirectReadData(u16 *volt,s16 *temp,s16 *curr);

//=============================================================================================
//函数名称	: void SH367309AlertIntCallback(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: AFE告警引脚中断调用函数
//注    意	:
//=============================================================================================
void SH367309AlertIntCallback(void);

//=============================================================================================
//函数名称	: void SH367309MainTask(void *p)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SH367309主任务函数
//注    意	:
//=============================================================================================
void SH367309MainTask(void *p);

#endif

/*****************************************end of SH367309.h*****************************************/
