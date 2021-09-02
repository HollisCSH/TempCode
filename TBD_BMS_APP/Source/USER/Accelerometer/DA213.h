//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: DA213.h
//创建人  	: Handry
//创建日期	: 
//描述	    : DA213头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef __DA213_H
#define __DA213_H

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTypeDef.h"
//#include "BSPGPIO.h"
//#include "BSPIIC.h"
#include "BSP_GPIO.h"
#include "BSP_IIC.h"

typedef struct AccData_tag{
   s16 ax;                                   //加速度计原始数据结构体  数据格式 0 0 1024
   s16 ay;
   s16 az;
   u8 MotionFlg;
   u8 AccPinSta1;
   u8 AccPinSta2;
}t_AccData;

typedef enum
{
    ACC_Sta_Init = 0,
    ACC_Sta_Norm = 1,
    ACC_Sta_Sleep = 2,
    ACC_Sta_Offline = 3,
}t_ACC_STA;

//时间日期结构体
typedef struct _DA213_DATA
{
   u8 accchn;               //IIC所用通道
   u32 timer;               //定时器
   t_ACC_STA RunStat;       //运行状态
   u8  Ctrl;                //控制
   u8  IICErr;              //IIC通信错误
   u8  InitErr;             //初始化错误
   t_AccData AccData;       //加速度计3轴的加速度
   s8  IICState; 			//IIC 的通讯状态
}t_DA213_Var;

//#define     BMS_ACC_CALI_OFFSET_ENABLE  (True)
#define     ACC_CONFIG_FLAG             (u16)(0x5AA5)    //存在SOC信息标志
//加速度计offset 保存EEPROM地址
typedef enum _ACC_EEPROM_ADDR
{
    e_ACC_Config_Flag = 100,     //加速度计配置标志
    
    e_ACC_Is_Cali_OK = e_ACC_Config_Flag + 2,   //是否已经校准
    
    e_ACC_Offset_Start = e_ACC_Is_Cali_OK + 1,  //offset数据
    e_ACC_Offset_End = e_ACC_Offset_Start + 9,
    
}e_ACC_EEP_ADDR;

#define mir3da_abs(x)          (((x) > 0) ? (x) : (-(x)))

//=============================================================================================
//宏定义
//=============================================================================================
//=============================================================================================
//#define		DA213_IIC_CHANNEL			eIIC1	//DA213使用IIC0通道
#define		DA213_IIC_CHANNEL			I2C2	//DA213使用IIC0通道
#define		DA213_TASK_TICK				500		//DA213任务周期

#define		DA213_IIC_NOFAULT_STAT		0		//IIC通信无错误状态
#define		DA213_IIC_FAULT_STAT		1		//IIC通信错误状态

#define		DA213_READBUFF_LEN			10		//读数组缓存长度
#define		DA213_WRITEBUFF_LEN			10		//写数组缓存长度

#define		DA213_WRITE_ADDR			0X4e	//读取8位地址
#define		DA213_READ_ADDR				DA213_WRITE_ADDR | 0x01

#define		DA213_COMM_FAULT_CNT		100
#define		DA213_MAX_TIMEOUT			20000	//最大等待时间，超过这个时间，直接回复错误信号

#define		DA213_CHIP_ID				0X13

//加速度计事件MASK
#define     DA213_INIT                  0x01
#define     DA213_GET_DATA              0x02
#define     DA213_SLEEP                 0x04
#define     DA213_WAKEUP                0x08
#define     DA213_TEST_INT              0x10
#define     DA213_SUSPEND_MODE          0x20
#define     DA213_CALI_OFFSET           0x40    //校准加速度计offset
#define     DA213_GET_INT               0x80    //读取加速度计中断

//=============================================================================================
//DA213 IC寄存器地址
#define 	DA213_REG_CONFIG                  0x00
#define 	DA213_REG_WHO_AM_I                0x01
#define 	DA213_REG_ACC_X_LSB               0x02
#define 	DA213_REG_ACC_X_MSB               0x03
#define 	DA213_REG_ACC_Y_LSB               0x04
#define 	DA213_REG_ACC_Y_MSB               0x05
#define 	DA213_REG_ACC_Z_LSB               0x06
#define 	DA213_REG_ACC_Z_MSB               0x07
#define 	DA213_REG_MOTION_FLAG             0x09
#define 	DA213_REG_NEWDATA_FLAG            0x0A
#define 	DA213_REG_ACTIVE_STATUS           0x0B
#define 	DA213_REG_G_RANGE                 0x0f
#define 	DA213_REG_ODR_AXIS                0x10
#define		DA213_REG_POWERMODE_BW            0x11
#define 	DA213_REG_SWAP_POLARITY           0x12
#define 	DA213_REG_FIFO_CTRL               0x14
#define	 	DA213_REG_INTERRUPT_SETTINGS1     0x16
#define 	DA213_REG_INTERRUPT_SETTINGS2     0x17
#define 	DA213_REG_INTERRUPT_MAPPING1      0x19
#define 	DA213_REG_INTERRUPT_MAPPING2      0x1a
#define 	DA213_REG_INTERRUPT_MAPPING3      0x1b
#define	 	DA213_REG_INT_PIN_CONFIG          0x20
#define 	DA213_REG_INT_LATCH               0x21
#define 	DA213_REG_FREEFALL_DURATION       0x22
#define 	DA213_REG_FREEFALL_THRESHOLD      0x23
#define 	DA213_REG_FREEFALL_HYST           0x24
#define 	DA213_REG_ACTIVE_DURATION         0x27
#define 	DA213_REG_ACTIVE_THRESHOLD        0x28
#define 	DA217_REG_TAP_DURATION            0x2A
#define 	DA217_REG_TAP_THRESHOLD           0x2B

#define     DA213_REG_Z_BLOCK                 0x2D
#define     DA213_REG_CUSTOM_OFFSET_X         0x38
#define     DA213_REG_CUSTOM_OFFSET_Y         0x39
#define     DA213_REG_CUSTOM_OFFSET_Z         0x3a
#define     DA213_REG_ENGINEERING_MODE        0x7f
#define     DA213_REG_SENSITIVITY_TRIM_X      0x80
#define     DA213_REG_SENSITIVITY_TRIM_Y      0x81
#define     DA213_REG_SENSITIVITY_TRIM_Z      0x82
#define     DA213_REG_COARSE_OFFSET_TRIM_X    0x83
#define     DA213_REG_COARSE_OFFSET_TRIM_Y    0x84
#define     DA213_REG_COARSE_OFFSET_TRIM_Z    0x85
#define     DA213_REG_FINE_OFFSET_TRIM_X      0x86
#define     DA213_REG_FINE_OFFSET_TRIM_Y      0x87
#define     DA213_REG_FINE_OFFSET_TRIM_Z      0x88
#define     DA213_REG_SENS_COMP               0x8c
#define     DA213_REG_CHIP_INFO_SECOND        0xc1
#define     DA213_REG_SENS_COARSE_TRIM        0xd1

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern t_DA213_Var gAccVar;            //加速度计控制/值结构体

//=============================================================================================
//声明接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: u8 DA213Init(void)
//输入参数	: void
//输出参数	: 是否初始化成功，0：成功；1：失败
//静态变量	: void
//功    能	: DA213初始化函数
//注    意	:
//=============================================================================================
u8 DA213Init(void);

//=============================================================================================
//函数名称	: u8 DA213ReadIntInit(void)
//输入参数	: void
//输出参数	: 是否初始化成功，0：成功；1：失败
//静态变量	: void
//功    能	: DA213读取中断初始化函数
//注    意	:
//=============================================================================================
u8 DA213ReadIntInit(void);

//=============================================================================================
//函数名称	: u8 DA213WriteRegister(e_IICNUM num, u8 regaddr,u8* p_buffer, u8 bufflen)
//输入参数	: num:eIIC0/eIIC1	regaddr:DA213寄存器地址      *p_buffer:发送缓冲区		bufflen:发送的数据个数
//输出参数	: void
//静态变量	: void
//功    能	: DA213写n个寄存器函数
//注    意	:
//=============================================================================================
u8 DA213WriteNRegisters(I2C_TypeDef *hi2cx, u8 regaddr,u8* p_buffer, u8 bufflen);

//=============================================================================================
//函数名称	: u8 DA213ReadNRegisters(e_IICNUM num,u16 regaddr,u8* p_buffer, u8 bufflen)
//输入参数	: num:eIIC0/eIIC1	regaddr:DA213寄存器地址		*p_buffer:接收缓冲区		bufflen:接收的数据个数
//输出参数	: IIC通信错误信息
//静态变量	: void
//功    能	: DA213读取信息函数
//注    意	:
//=============================================================================================
u8 DA213ReadNRegisters(I2C_TypeDef *hi2cx,u16 regaddr,u8* p_buffer, u8 bufflen);

//=============================================================================================
//函数名称	: u8 DA213WriteRegAndCompareValue(u8 regaddr,u8* p_buffer, u8 bufflen)
//输入参数	: regaddr:DA213寄存器地址      *p_buffer:写入缓冲区		bufflen:写入的数据个数
//输出参数	: 0：写入正常；1：写入异常
//静态变量	: void
//功    能	: DA213写寄存器函数并比较返回的值是否正确
//注    意	:
//=============================================================================================
u8 DA213WriteRegAndCompareValue(u8 regaddr,u8* p_buffer, u8 bufflen);

//=============================================================================================
//函数名称	: void DA213SetCtrl(u8 ctrl)
//输入参数	: ctrl:控制位，详细定义见头文件宏定义
//输出参数	: void
//静态变量	: void
//功	能	: DA213设置控制标志位函数
//注	意	:
//=============================================================================================
void DA213SetCtrl(u8 ctrl);

//=============================================================================================
//函数名称	: void DA213ClearCtrl(u8 ctrl)
//输入参数	: ctrl:控制位，详细定义见头文件宏定义
//输出参数	: void
//静态变量	: void
//功	能	: DA213清除控制标志位函数
//注	意	:
//=============================================================================================
void DA213ClearCtrl(u8 ctrl);

//=============================================================================================
//函数名称	: void DA213Enable(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: DA213使能
//注	意	: 休眠唤醒中调用
//=============================================================================================
void DA213Enable(void);

//=============================================================================================
//函数名称	: void DA213Disable(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: DA213失能
//注	意	: 休眠唤醒中调用
//=============================================================================================
void DA213Disable(void);

//=============================================================================================
//函数名称	: u8 DA213MainTask(void *p)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: DA213主任务函数
//注    意	:
//=============================================================================================
void DA213MainTask(void *p);

#endif

/*****************************************end of DA213.h*****************************************/
