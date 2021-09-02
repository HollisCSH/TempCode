//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: max17205.h
//创建人  	: Handry
//创建日期	: 
//描述	    : max17205驱动头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef __MAX17205_H
#define __MAX17205_H

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTypeDef.h"
//#include "BSPIIC.h"
#include "BSP_IIC.h"

//=============================================================================================
//类型定义
//=============================================================================================

//电池参数配置结构体
struct t_BatteryConfig
{
	u16 Max17205_Regaddr;		//配置的寄存器地址
	u16 RegValue;				//配置的寄存器值
};

typedef	enum
{ 
	eMax17205Read = 0, 
	eMax17205Write = 1
}e_Max17205WR;

//电池数据读取
typedef struct
{
    u8 socchn;    
    u16 DfVer;                  //电量计版本号
    s8  iicres;                 //IIC通信结果
	u8  RunStat;				//运行状态
    u16 Ctrl;					//采集控制信号
    u16 IICErr;				    //IIC通信错误结果
    u8  InitErr;				//初始化发生故障
    u16 Flags;
    u32 Timer;

    u16 RepSOC;					//Report SOC	单位：1%
    u16 SOH;					//电池健康度SOH	单位：1%
    u16 FullChgCap;             //满充容量		单位：1mAH
    u16 RepRemainCap;	        //剩余容量		单位：1mAH
    u16 RemainPower;			//剩余电能		单位：0.1Wh = 0.1 mV*mAH/100000
    u16 FullChgTime;		    //满充时间		单位：1min
    u16 EmptyDchgTime;	        //放空时间		单位：1min
    u16 CycleCount;			    //循坏次数		单位：次
    s16 Averagecurrent;			//平均电流		单位：1mA
    u16 MaxCurr;				//电流最大值		单位：1mA
    s16 Temperature;			//采集的温度		单位：℃

    u16 Voltage;				//电池包电压		单位：1mv
    //附加信息，可选择是否需要
    u16 CellVolt;				//最低单体电压	 单位：mV

    u8 NumOfDates;              //剩余的memory更新次数
    u8 AtLesRem;                //至少剩余的更新次数
    u8 ShdnFlag;                //电量计关机标志
}t_MAX1720X_VAR;

//电量计标定数据
typedef struct
{
    u16 rsense;     //分流器阻值	从MAX17205寄存器中读取	单位：mΩ
    s16 calibcurr;	//标定电流    
    u16 wrtcycle;   //写入的循环次数
}
t_MAX1720X_CAL;

typedef enum
{
    eFuel_Sta_Init = 0,
    eFuel_Sta_Disable = 1,
    eFuel_Sta_Norm = 2,
    eFuel_Sta_Offline = 3,
    eFuel_Sta_Idle = 4,
}e_FuelSTA;


//=============================================================================================
//宏定义
//=============================================================================================
#define		MAX1720X_IIC_CHANNEL		eIIC1	//电量计使用IIC1通道

#define		MAX1720X_READBUFF_LEN		10		//读数组缓存长度
#define		MAX1720X_WRITEBUFF_LEN		10		//写数组缓存长度

#define		MAX1720X_IIC_NOFAULT_STAT	0		//IIC通信无错误状态
#define		MAX1720X_IIC_FAULT_STAT		1		//IIC通信错误状态

#define		MAX1720X_COMM_FAULT_CNT		100
#define     MAX1720X_CURR_CALIB_CNT     5       //平均电流校准个数

#define 	MAX1720X_TEMP_OFF 			0x7FFE
#define 	MAX1720X_TEMP_SHORT 		0x7FFD

#define		MAX1720X_MAX_REGADDR		0X1ff	//最大寄存器地址范围
#define		MAX1720X_MAX_WAIT_TIMEOUT	20000	//最大等待时间，超过这个时间，直接回复错误信号
#define		MAX1720X_MAX_TIMEOUT		3000	//最大等待时间，超过这个时间，直接回复错误信号，单位ms
#define		MAX1720X_PT_MAX_TIMEOUT		300		//最大等待时间，超过这个时间，直接回复错误信号，单位ms
#define		MAX1720X_WAIT_DELAY 		30		//等待间隔时间，单位ms


#define 	MAX1720X_ADDR   			0x36
#define		MAX1720X_WRITE_ADDR			0X6c	//读取8位地址
#define		MAX1720X_READ_ADDR			MAX1720X_WRITE_ADDR | 0x01
#define		MAX1720X_WRITE_EXPADDR		0X16	//读取拓展地址
#define		MAX1720X_READ_EXPADDR		MAX1720X_WRITE_EXPADDR | 0x01

//=============================================================================================
//MAX17205寄存器地址
#define		MAX1720X_STATUS_ADDR		0x00	// Contains alert status and chip status 状态寄存器
#define		MAX1720X_STATUS2_ADDR		0xB0	// Contains alert status and chip status2 状态寄存器2
#define 	MAX1720X_COMMAND_ADDR   	0x60 	// Command register	命令寄存器
#define		MAX1720X_COMMAND_STAT_ADDR	0x61	// Command state register 命令状态寄存器
#define 	MAX1720X_CONFIG_ADDR   		0x1D 	//The Config Register	配置寄存器
#define 	MAX1720X_CONFIG2_ADDR   	0xBB 	// Command register		配置寄存器2

#define 	MAX1720X_REPSOC_ADDR  		0x06 	// Reported state of charge SOC
#define 	MAX1720X_REPSOH_ADDR  		0x07 	// Reported health of charge SOH
#define 	MAX1720X_REPCAP_ADDR   		0x05 	// Reported remaining capacity 剩余容量
#define 	MAX1720X_FULLCAP_ADDR  		0x10 	// Full capacity estimation  满充容量
#define 	MAX1720X_REPFULLCAP_ADDR  	0x35 	// Reported Full capacity estimation  满充容量
#define 	MAX1720X_TTE_ADDR   		0x11 	// Time to empty 放空时长
#define 	MAX1720X_TTF_ADDR   		0x20 	// Time to full  充满时长
#define 	MAX1720X_BATCYCLE_ADDR   	0x17 	// Battery cycles  循环次数

#define 	MAX1720X_VCELL_ADDR   		0x09 	// Lowest cell voltage of a pack, or the cell voltage for a single cell  最低单体电压
#define 	MAX1720X_VCELLX_ADDR   		0xD9 	// cellX voltage of a pack CELLX的单体电压
#define 	MAX1720X_VBAT_ADDR   		0xDA 	// Battery pack voltage  电池包电压

#define 	MAX1720X_CURENT_ADDR   		0x0A 	// Battery current	实时电流
#define 	MAX1720X_AVGCURENT_ADDR   	0x0B 	// Average Battery current 平均电流
#define 	MAX1720X_MAXMINCURENT_ADDR  0x1C 	// MaxMin current 最大最小电流，高8位为最大电流，低8位为最小电流

#define 	MAX1720X_TEMP_ADDR   		0x08	// Temperature  温度
#define 	MAX1720X_TEMP1_ADDR   		0x134	// Temperature  温度1
#define 	MAX1720X_TEMP2_ADDR   		0x13B	// Temperature  温度2

#define		MAX1720X_FSTAT_ADDR			0X3D	// The Fstat register monitors the status of the ModelGauge algorithm
#define 	MAX1720X_Timer				0x3E 	// The Timer register LSb is equal to 175.8ms giving a full-scale range of 0 to 3.2 hours.
#define 	MAX1720X_TimerH				0xBE 	// An LSb of 3.2 hours gives a full-scale range for the register of up to 23.94 years.
#define 	MAX1720X_Shadow_ROM_addr 	0x0B 	// Shadow ROM addr need to write message to NonVolatile memory. Addr changes 180h–1FFh->80h–FFh. Example,0x019D->0x009D(0x9D)
#define 	MAX1720X_QH					0x4D 	// register displays the raw coulomb count generated by the device in mah

#define		MAX1720X_NRSENSE_ADDR		0x1CF	//The sense resistor value register 分流器阻值
#define		MAX1720X_NDESIGNCAP_ADDR	0x1B3	//The nDesignCap Register	N容量配置
#define		MAX1720X_NVEMPTY_ADDR		0x19E	//The nVEmpty Register		N空电压配置
#define		MAX1720X_NPACKCFG_ADDR		0x1B5	//The nPackCfg Register		N电池包配置
#define		MAX1720X_NFULLSOCTHR_ADDR	0x1C6	//The nFullSOCTHr Register	N满充SOC门限值配置
#define		MAX1720X_NFILTERCFG_ADDR	0x19D	//The nFilterCfg Register	N平均值时间值配置

#define     MAX1720X_NUM_OF_UPDATES		0x1ED	//获取Memory更新数次的地址

#define		MAX1720X_DESIGNCAP_ADDR	    0x018	//The nesignCap Register	容量配置
#define		MAX1720X_VEMPTY_ADDR		0x03A	//The VEmpty Register		空电压配置
#define		MAX1720X_PACKCFG_ADDR		0x0BD	//The PackCfg Register		电池包配置
#define		MAX1720X_FULLSOCTHR_ADDR	0x013	//The FullSOCTHr Register	满充SOC门限值配置
#define		MAX1720X_FILTERCFG_ADDR		0x029	//The FilterCfg Register	平均值时间值配置
#define 	MAX1720X_HIBCFG_ADDR		0x0BA	//The HibCfg Register       休眠配置

#define		MAX1720X_COFF_ADDR		    0x02F	//COFF 寄存器
#define 	MAX1720X_CGAIN_ADDR		    0x02E	//CGAIN 寄存器

//=============================================================================================
//配置位参数
#define		MAX1720X_START_SAMPLE		0X01        //开始采样位置
#define		MAX1720X_START_CALIBCURR	0X02        //开始校准电流位置
#define		MAX1720X_WRITE_CYCLES	    0X04        //开始写电量计循环次数

#define		MAX1720X_FULLCHG_FLAG		0X0080      //满充标志位置

//=============================================================================================
//配置参数
#define		BATTERY_CONFIG_LEN			10			//配置参数的数组长度

#define		BATTERY_DESIGNCAP			12000		//单位：mAH
#define		BATTERY_CELLNUM				15			//串数
#define		BATTERY_VEMPTY_VE			140 		//空状态检测电压	2800mV	单位：20mV
#define		BATTERY_VEMPTY_VR			75			//空状态恢复电压	3000mV	单位：40mV
#define		BATTERY_FULLSOCTHR			95			//满充SOC上限值				单位：1%
#define		BATTERY_DFRENSE_VAL			100			//单位0.01mΩ				单位：1%

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern t_MAX1720X_VAR gMax17205Var;	//max17205读取的数据
extern t_MAX1720X_CAL gMax17205Cal; //max17205标定的数据

//=============================================================================================
//声明接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: u8 Max17205Init(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: MAX17205初始化函数
//注	意	:
//=============================================================================================
u8 Max17205Init(void);

//=============================================================================================
//函数名称	: u8 Max17205ResumeInit(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: MAX17205休眠唤醒初始化函数
//注    意	:
//=============================================================================================
u8 Max17205ResumeInit(void);

//=============================================================================================
//函数名称	: void Max17205DataInit(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: MAX17205数据初始化函数
//注	意	:
//=============================================================================================
void Max17205DataInit(void);

//=============================================================================================
//函数名称	: void Max17205SetCtrl(u16 ctrl)
//输入参数	: ctrl:控制位，详细定义见头文件宏定义
//输出参数	: void
//静态变量	: void
//功	能	: MAX17205设置控制标志位函数
//注	意	:
//=============================================================================================
void Max17205SetCtrl(u16 ctrl);

//=============================================================================================
//函数名称	: void Max17205SetCtrl(u16 ctrl)
//输入参数	: ctrl:控制位，详细定义见头文件宏定义
//输出参数	: void
//静态变量	: void
//功	能	: MAX17205清除控制标志位函数
//注	意	:
//=============================================================================================
void Max17205ClearCtrl(u16 ctrl);

//=============================================================================================
//函数名称  : void MAX17205_SetCalCurr(s16 act_current)
//输入参数  : void
//输出参数  : void
//静态变量  : void
//功		 能 : 设置校准电流
//注		 意 :
//=============================================================================================
void MAX17205SetCalCurr(s16 act_current);

//=============================================================================================
//函数名称	: u8 Max17205FullReset(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: MAX17205芯片复位函数
//注    意	:
//=============================================================================================
u8 Max17205FullReset(void);

//=============================================================================================
//函数名称	: u8 Max17205ShutDown(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: MAX17205关机函数
//注    意	:
//=============================================================================================
u8 Max17205ShutDown(void);

//=============================================================================================
//函数名称	: u8 Max17205WriteWord(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: MAX17205写寄存器函数
//注	意	:
//=============================================================================================
//u8 MAX17205WriteRegister(e_IICNUM num, u16 regaddr,u16 word);

//=============================================================================================
//函数名称	: u8 Max17205WriteWord(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: MAX17205写寄存器函数
//注	意	:
//=============================================================================================
//u8 MAX17205WriteRegisterWait(e_IICNUM num, u16 regaddr,u16 word);

//=============================================================================================
//函数名称	: u8 MAX17205ReadRegister(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr)
//输入参数	: num:eIIC0/eIIC1	*p_buffer:接收缓冲区		bufflen:接收的数据个数		regaddr:MAX17205寄存器地址
//输出参数	: IIC通信错误信息
//静态变量	: void
//功    能	: MAX17205读取信息函数
//注    意	:
//=============================================================================================
//u8 MAX17205ReadRegister(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr);

//=============================================================================================
//函数名称	: u8 MAX17205ReadRegisterWait(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr)
//输入参数	: num:eIIC0/eIIC1	*p_buffer:接收缓冲区		bufflen:接收的数据个数		regaddr:MAX17205寄存器地址
//输出参数	: IIC通信错误信息
//静态变量	: void
//功    能	: MAX17205读取信息函数
//注    意	:
//=============================================================================================
//u8 MAX17205ReadRegisterWait(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr);

//=============================================================================================
//函数名称	: u16 Max17205GetRegisterValue(u16 max1720x_regaddr)
//输入参数	: void
//输出参数	: u16
//静态变量	: 16位寄存器的值
//功	能	: MAX17205读取max17205寄存器的值
//注	意	:
//=============================================================================================
u16 Max17205GetRegisterValue(u16 max1720x_regaddr);

//=============================================================================================
//函数名称	: void Max17205GetBatteryData(void)
//输入参数	: max1720x_regaddr:max17205寄存器地址	max1720x_var:电池数据结构体地址
//输出参数	: void
//静态变量	: void
//功    能	: 获取MAX17205电池数据函数接口
//注    意	:
//=============================================================================================
void Max17205GetBatteryData(u16 max1720x_regaddr,t_MAX1720X_VAR *max1720x_var);

//=============================================================================================
//函数名称	: void Max17205MainTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: MAX17205主任务函数
//注    意	:
//=============================================================================================
void Max17205MainTask(void *p);

#endif

/*****************************************end of MAX17205.h*****************************************/
