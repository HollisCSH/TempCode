//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: DA213.c
//创建人  	: Handry
//创建日期	: 
//描述	    : DA213加速度传感器头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "DA213.h"
//#include "BSPSysTick.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "BSPTimer.h"
//#include "BSPFlash.h"
#include "ParaCfg.h"
//#include "interrupt_manager.h"
#include "pt.h"
#include "IOCheck.h"

//=============================================================================================
//全局变量定义
//=============================================================================================
u8	gDA213Readbuff[DA213_READBUFF_LEN] = {0};  		//DA213接收缓冲数组
u8	gDA213Writebuff[DA213_WRITEBUFF_LEN] = {0};  	//DA213发送缓冲数组
u8  gACCWritebuff[DA213_WRITEBUFF_LEN] = {0};  	    //IIC通信缓冲数组

u8	gDA213ChipID = 0;           //DA213芯片ID
u8  gMotionFlag = 0;
u8  gActiveHappen = 0;			//加速度剧烈变化标志。0：没发生；1：已经发生

t_DA213_Var gAccVar;            //加速度计控制/值结构体
pt gPTDA213;					//DA213任务pt线程控制变量
pt gPTDA213Init;				//DA213任务初始化任务pt线程控制变量
pt gPTDA213Norm;				//DA213任务正常任务pt线程控制变量
pt gPTDA213OffLine;				//DA213任务掉线任务pt线程控制变量

#ifdef BMS_ACC_CALI_OFFSET_ENABLE

//校准offse使用变量
static short yzcross = 0;

#define YZ_CROSS_TALK_ENABLE True

#define MIR3DA_OFF_SECT_LEN         9
#define STABLE_CHECK_SAMPLE_NUM     10
#define STABLE_CHECK_THRESHOLD      50000
#define AUTO_CALI_THRESHOLD_XY      200
#define AUTO_CALI_THRESHOLD_Z       200
#define abs(x) ({ long __x = (x); (__x < 0) ? -__x : __x; })

s16 gRawX,gRawY,gRawZ; //加速度计原始数据
//static 
    int is_cali = 0;
//static 
    unsigned char cali_ok_flag = 0;
//static 
    unsigned char original_offset[9];
static unsigned char    stable_sample_cnt = 0;
static int              stable_sample_pow_sum[STABLE_CHECK_SAMPLE_NUM] = {0};
static int              stable_sample_sum[3] = {0};
unsigned char off_reg[9] = 
{
DA213_REG_COARSE_OFFSET_TRIM_X,DA213_REG_COARSE_OFFSET_TRIM_Y,DA213_REG_COARSE_OFFSET_TRIM_Z,
DA213_REG_FINE_OFFSET_TRIM_X,DA213_REG_FINE_OFFSET_TRIM_Y,DA213_REG_FINE_OFFSET_TRIM_Z,
DA213_REG_CUSTOM_OFFSET_X,DA213_REG_CUSTOM_OFFSET_Y,DA213_REG_CUSTOM_OFFSET_Z
};

//NSA_NTO_cali_step_calc
unsigned int       total_gain[3] = {0}; 
unsigned char      coarse_gain = 0;
unsigned char      fine_gain[3] = {0};
unsigned int       const coarse_gain_map[] = {1000, 1125, 1250, 1375, 500, 625, 750, 875};   /* *1000  */
unsigned char      const fine_dig_gain_map[] = {1, 2, 4, 8};   

//NSA_NTO_auto_calibrate
int  xyz[3] = {0};

int     coarse_step[3];
int     fine_step[3];
int     custom_step[3] = {0};
unsigned char     swap_plarity_old = 0;
int     temp=0;          
        
//NSA_once_calibrate
int     coarse[3] = {0};
int     coarse_delta[3] = {0};
int     fine[3] = {0};
int     fine_delta[3] = {0};
int     target[3] = {0};
int     i;
unsigned char   offset_data[9] = {0}; 

#endif


//const pin_settings_config_t cPinMuxAccConfigArr[2] =
//{
//    /* ACC INT2 */
//    {
//        .base          = ACC_CHECK_INT2_GPIOBASE,
//        .pinPortIdx    = ACC_CHECK_INT2_PN,
//        .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
//        .passiveFilter = false,
//        .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
//        .mux           = PORT_MUX_AS_GPIO,
//        .pinLock       = false,
//        .intConfig     = PORT_INT_RISING_EDGE,
//        .clearIntFlag  = false,
//        .gpioBase      = ACC_CHECK_INT2_BASE,
//        .direction     = GPIO_INPUT_DIRECTION,
//        .digitalFilter = false,
//    },      
//    /* ACC INT1 */
//    {
//        .base          = ACC_CHECK_INT1_GPIOBASE,
//        .pinPortIdx    = ACC_CHECK_INT1_PN,
//        .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
//        .passiveFilter = false,
//        .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
//        .mux           = PORT_MUX_AS_GPIO,
//        .pinLock       = false,
//        .intConfig     = PORT_INT_RISING_EDGE,
//        .clearIntFlag  = false,
//        .gpioBase      = ACC_CHECK_INT1_BASE,
//        .direction     = GPIO_INPUT_DIRECTION,
//        .digitalFilter = false,
//    },  
//};

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: u8 DA213Init(void)
//输入参数	: void
//输出参数	: 是否初始化成功，0：成功；1：失败
//静态变量	: void
//功    能	: DA213初始化函数
//注    意	:
//=============================================================================================
u8 DA213Init(void)
{
	u8 result = 0;
    
    PT_INIT(&gPTDA213);
    PT_INIT(&gPTDA213Init);
    PT_INIT(&gPTDA213Norm);    
    PT_INIT(&gPTDA213OffLine);        
    gAccVar.RunStat = ACC_Sta_Init;
    gAccVar.Ctrl = 0;
    //获取IIC通道
//    gAccVar.accchn = BSPIICChannalReq(IIC_ROUTE_ACC);
    
	return result;
}

//=============================================================================================
//函数名称	: u8 DA213ReadIntInit(void)
//输入参数	: void
//输出参数	: 是否初始化成功，0：成功；1：失败
//静态变量	: void
//功    能	: DA213读取中断初始化函数
//注    意	:
//=============================================================================================
u8 DA213ReadIntInit(void)
{
	u8 result = 0;
    
    PT_INIT(&gPTDA213Norm);    
    gAccVar.RunStat = ACC_Sta_Norm;
    
	return result;
}

//=============================================================================================
//函数名称	: u8 DA213WriteRegister(e_IICNUM num, u8 regaddr,u8* p_buffer, u8 bufflen)
//输入参数	: num:eIIC0/eIIC1	regaddr:DA213寄存器地址      *p_buffer:发送缓冲区		bufflen:发送的数据个数
//输出参数	: void
//静态变量	: void
//功    能	: DA213写n个寄存器函数
//注    意	:
//=============================================================================================
u8 DA213WriteNRegisters(I2C_TypeDef *hi2cx, u8 regaddr,u8* p_buffer, u8 bufflen)
{
	u8 errstat = 0;									//IIC通信错误信息
	u8 regaddress = 0;
	u8 deviceaddr = 0;
	// pwritebuf[DA213_READBUFF_LEN] = {0};    
	u8 i = 0;

	//if((num > eIIC1) || (bufflen > (DA213_WRITEBUFF_LEN - 2)))  //参数有效性判断
	if((hi2cx != DA213_IIC_CHANNEL) || (bufflen > (DA213_WRITEBUFF_LEN - 2)))  //参数有效性判断
	{
		return DA213_IIC_FAULT_STAT;
	}

	deviceaddr = DA213_WRITE_ADDR;
	regaddress = (u8)(regaddr);

	gACCWritebuff[0] = deviceaddr;
	gACCWritebuff[1] = regaddress;
    
    for(i = 0;i < bufflen;i++)
    {
        gACCWritebuff[2 + i] = *p_buffer;
        p_buffer++;
    }

    //errstat = BSPIICSendMsg(num,pwritebuf,bufflen + 2,1);
		//BSPIICWrite(gAccVar.accchn,gACCWritebuff[0],&gACCWritebuff[1],bufflen + 1);
    errstat = BSP_I2C_MEM_WRITE(DA213_IIC_CHANNEL , gACCWritebuff[1] , &gACCWritebuff[2] , bufflen);
//	 errstat = BSP_I2C_MEM_WRITE(DA213_IIC_CHANNEL , gACCWritebuff[0] , &gACCWritebuff[1] , bufflen + 1);
	 return errstat;
}

//=============================================================================================
//函数名称	: u8 DA213ReadNRegisters(e_IICNUM num,u16 regaddr,u8* p_buffer, u8 bufflen)
//输入参数	: num:eIIC0/eIIC1	regaddr:DA213寄存器地址		*p_buffer:接收缓冲区		bufflen:接收的数据个数
//输出参数	: IIC通信错误信息
//静态变量	: void
//功    能	: DA213读取信息函数
//注    意	:
//=============================================================================================
u8 DA213ReadNRegisters(I2C_TypeDef *hi2cx,u16 regaddr,u8* p_buffer, u8 bufflen)
{
	 u8 errstat = 0;	//IIC通信错误信息
	 u8 regaddress = 0;

	 //if((num > eIIC1) || (bufflen > DA213_READBUFF_LEN) || (p_buffer == NULL))  //参数有效性判断
	 if((hi2cx != DA213_IIC_CHANNEL) || (bufflen > DA213_READBUFF_LEN) || (p_buffer == NULL))  //参数有效性判断
	 {
		 return DA213_IIC_FAULT_STAT;
	 }

	 regaddress = (u8)regaddr;
   gACCWritebuff[0] = regaddress;
	 //errstat = BSPIICRcvMsg(num,DA213_WRITE_ADDR,regaddress,p_buffer,bufflen,1);	//发送读取报文，标准地址
     
   //BSPIICWriteAndRead(gAccVar.accchn,DA213_WRITE_ADDR,gACCWritebuff,1,p_buffer,bufflen);
	 errstat = BSP_I2C_MEM_REAR(DA213_IIC_CHANNEL , gACCWritebuff[0] , p_buffer, bufflen);
     
	 return errstat; 
}

//=============================================================================================
//函数名称	: u8 DA213WriteRegAndCompareValue(u8 regaddr,u8* p_buffer, u8 bufflen)
//输入参数	: regaddr:DA213寄存器地址      *p_buffer:写入缓冲区		bufflen:写入的数据个数
//输出参数	: 0：写入正常；1：写入异常
//静态变量	: void
//功    能	: DA213写寄存器函数并比较返回的值是否正确
//注    意	:
//=============================================================================================
u8 DA213WriteRegAndCompareValue(u8 regaddr,u8* p_buffer, u8 bufflen)
{
	u8 result = 0;
	u8 preadbuf[DA213_READBUFF_LEN] = {0};
	u8 i = 0;
	u8 errcnt = 0;

	result = DA213WriteNRegisters(DA213_IIC_CHANNEL,regaddr,p_buffer,bufflen);

	if(DA213_IIC_NOFAULT_STAT == result)
	{
		result = DA213ReadNRegisters(DA213_IIC_CHANNEL,regaddr,preadbuf,bufflen);

		if(DA213_IIC_NOFAULT_STAT == result)
		{
			for(i = 0; i < bufflen;i++)
			{
				if(preadbuf[i] != p_buffer[i])
				{
					errcnt++;
				}
			}
		}
	}

	if(0 < errcnt)
	{
		result = DA213_IIC_FAULT_STAT;
	}

	return result;
}

//=============================================================================================
//函数名称	: s16 DA213CalculateData(u8 * str)
//输入参数	: *str:读取的加速度数据
//输出参数	: s16:转化后的加速度数据
//静态变量	: void
//功    能	: DA213转化加速度数据
//注    意	:
//=============================================================================================
s16 DA213CalculateData(u8 *str)
{
    s16 tmp;
    long tmp1;
    
    tmp = ((s16)*str) + (((s16)*(str + 1)) << 8);
    tmp = tmp >> 2;
    tmp1 = (long)tmp * 125 / 64;
    
    return (s16)tmp1;
}

//=============================================================================================
//函数名称	: void DA213SetCtrl(u8 ctrl)
//输入参数	: ctrl:控制位，详细定义见头文件宏定义
//输出参数	: void
//静态变量	: void
//功	能	: DA213设置控制标志位函数
//注	意	:
//=============================================================================================
void DA213SetCtrl(u8 ctrl)
{
    gAccVar.Ctrl |= ctrl;
}

//=============================================================================================
//函数名称	: void DA213ClearCtrl(u8 ctrl)
//输入参数	: ctrl:控制位，详细定义见头文件宏定义
//输出参数	: void
//静态变量	: void
//功	能	: DA213清除控制标志位函数
//注	意	:
//=============================================================================================
void DA213ClearCtrl(u8 ctrl)
{
    gAccVar.Ctrl &= (~ctrl);
}

//=============================================================================================
//函数名称	: void DA213Enable(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: DA213使能
//注	意	: 休眠唤醒中调用
//=============================================================================================
void DA213Enable(void)
{
    if(IOCheckWakeupCondGet() & WAKE_COND_ACC_INT_MASK)
    {
        IOCheckWakeupCondClr(WAKE_COND_ACC_INT_MASK);
        DA213ReadIntInit();
//        gAccVar.accchn = BSPIICChannalReq(IIC_ROUTE_ACC);
        BSPTaskStart(TASK_ID_ACCELERO_TASK, 1); 
    }
    else
    {
        DA213Init();
        BSPTaskStart(TASK_ID_ACCELERO_TASK, 10); 
    }
    //BSPTaskCreate(TASK_ID_ACCELERO_TASK,DA213MainTask,(void *)0);
	       
}

//=============================================================================================
//函数名称	: void DA213Disable(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: DA213失能
//注	意	: 休眠唤醒中调用
//=============================================================================================
void DA213Disable(void)
{
    BSPTaskStop(TASK_ID_ACCELERO_TASK);
//    BSPIICChannalDel(gAccVar.accchn);
}

//=============================================================================================
//函数名称	: void DA213FaultTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: DA213初始化任务函数
//注    意	:
//=============================================================================================
void DA213InitTask(void)
{
    static u8 result = 0;
    
    #ifdef BMS_ACC_CALI_OFFSET_ENABLE
    static u8 i = 0;
    #endif
    
    PT_BEGIN(&gPTDA213Init);
     
    //INT_SYS_DisableIRQ(PORT_IRQn); 
		HAL_NVIC_DisableIRQ( EXTI4_15_IRQn );	//加速度计的中断为于 PB8 和 PC10
    /****************************************复位**********************************************/    
    //避免复位引起的中断
    gDA213Writebuff[0] = 0x24;
    result |= DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_CONFIG,gDA213Writebuff,1);    
    
    PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
    if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
    {
        gAccVar.IICErr++;
    }     
    gAccVar.timer = gTimer1ms;
    PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (gTimer1ms - gAccVar.timer > 5));	    //延时5ms 
   
    //读取芯片id
    result |= DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_WHO_AM_I,&(gDA213Readbuff[0]),1);
    
    PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
    if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
    {
        gAccVar.IICErr++;
    }         
    else
    {
        gDA213ChipID = gDA213Readbuff[0];
    }
    
    //读取芯片id
    if(DA213_CHIP_ID == gDA213ChipID)
    {
        result |= DA213_IIC_NOFAULT_STAT;
    }  

    /****************************************配置**********************************************/   
    if(0 == gConfig.accpara.TapEn)
    {
        gDA213Writebuff[0] = 0x01;     // +-4g量程，14bit精度    
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_G_RANGE,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }   
        
        result |= DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_G_RANGE,&(gDA213Readbuff[0]),1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        } 
        else
        {
            if(gDA213Readbuff[0] != 0x01)
                gAccVar.IICErr++;
        }
        /****************************************************************************************************************/
        gDA213Writebuff[0] = 0x08;//0x06;     // enable XYZ axis 0x06:odr=62.5Hz,0x8:odr=250Hz
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_ODR_AXIS,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }     
        
        result |= DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_ODR_AXIS,&(gDA213Readbuff[0]),1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        } 
        else
        {
            if(gDA213Readbuff[0] != 0x08)
                gAccVar.IICErr++;
        }        
        /****************************************************************************************************************/
        gDA213Writebuff[0] = 0x5e;     // low power模式 low_power_bw:500HZ带宽
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_POWERMODE_BW,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }
        
        result |= DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_POWERMODE_BW,&(gDA213Readbuff[0]),1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        } 
        else
        {
            if(gDA213Readbuff[0] != 0x5e)
                gAccVar.IICErr++;
        }     
        /****************************************************************************************************************/
        
        //ACTIVE中断配置
        //gDA213Writebuff[0] = 0x00;     // Active duration time  = 4ms
        gDA213Writebuff[0] = (u8)gConfig.accpara.ActiveDur;
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_ACTIVE_DURATION,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }         
        
        //gDA213Writebuff[0] = 0xff;     // Threshold of active interrupt=Active_th*K(mg) 255* 31.25 ≈ 8g
        gDA213Writebuff[0] = (u8)gConfig.accpara.ActiveTh;
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_ACTIVE_THRESHOLD,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }         
        
        //自由落体中断配置
        // Delay time for freefall
        //delay_time = ( freefall_dur + 1 ) * 2ms
        //gDA213Writebuff[0] = 9;    //20ms
        gDA213Writebuff[0] = (u8)gConfig.accpara.FreeFallDur;
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_FREEFALL_DURATION,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }       
        //freefall threshold = freefall_th * 7.81mg
        //gDA213Writebuff[0] = 0x30;    //默认值 375mg
        gDA213Writebuff[0] = (u8)gConfig.accpara.FreeFallTh;    
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_FREEFALL_THRESHOLD,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }   
        
        gDA213Writebuff[0] = 0x05;    //sum mode,125mg
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_FREEFALL_HYST,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }         
        
        //中断配置
        gDA213Writebuff[0] = 0x07;     // 使能active中断，enable the active interrupt for the x/y/z axis.
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INTERRUPT_SETTINGS1,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }  
        
        gDA213Writebuff[0] = 0x08;     // 使能打开自由落体中断
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INTERRUPT_SETTINGS2,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }      

        /*****************************************中断引脚配置******************************************/  
        gDA213Writebuff[0] = 0x04;     // active中断，映射到INT1
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INTERRUPT_MAPPING1,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }  
        
        gDA213Writebuff[0] = 0x01;     // 自由落体中断，映射到INT2
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INTERRUPT_MAPPING3,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }  
        
        #if defined(USE_DA217)
        gDA213Writebuff[0] = 0x00;     // 配置INT1端口 active high for pin INT1 and INT2 上降沿触发
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INT_PIN_CONFIG,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIICChannalStateGet(gAccVar.accchn) <= 0));
        if((result > 0) || (BSPIICChannalStateGet(gAccVar.accchn) < 0))      
        {
            gAccVar.IICErr++;
        }        
        #else
        gDA213Writebuff[0] = 0x05;     // 配置INT1端口 active high for pin INT1 and INT2 上降沿触发
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INT_PIN_CONFIG,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }     
        #endif
        //gDA213Writebuff[0] = 0xE0;     // 锁存100ms
        gDA213Writebuff[0] = 0x66;     // 锁存8s
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INT_LATCH,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }             
        
    }
    else
    {
        /****************************************************************************************************************/  
	    gDA213Writebuff[0] = 0x03;     // +-16g量程，14bit精度    
	    result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_G_RANGE,gDA213Writebuff,1);
	    PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
	    if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
	    {
	        gAccVar.IICErr++;
	    }  
        
        result |= DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_G_RANGE,&(gDA213Readbuff[0]),1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        } 
        else
        {
            if(gDA213Readbuff[0] != 0x03)
                gAccVar.IICErr++;
        }     
        /****************************************************************************************************************/   

	    gDA213Writebuff[0] = 0x06;     // enable XYZ axis 0x06:odr=62.5Hz,0x8:odr=250Hz
	    result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_ODR_AXIS,gDA213Writebuff,1);
	    PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
	    if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
	    {
	        gAccVar.IICErr++;
	    }   
        
        result |= DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_ODR_AXIS,&(gDA213Readbuff[0]),1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        } 
        else
        {
            if(gDA213Readbuff[0] != 0x06)
                gAccVar.IICErr++;
        }        
        /****************************************************************************************************************/  
    
	    gDA213Writebuff[0] = 0x5e;     // low power模式 low_power_bw:500HZ
	    result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_POWERMODE_BW,gDA213Writebuff,1);
	    PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
	    if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
	    {
	        gAccVar.IICErr++;
	    }     
        
        result |= DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_POWERMODE_BW,&(gDA213Readbuff[0]),1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        } 
        else
        {
            if(gDA213Readbuff[0] != 0x5e)
                gAccVar.IICErr++;
        }     
        /****************************************************************************************************************/
	
        gDA213Writebuff[0] = 0x1e;     // n*0.5g; 配置stap阈值为0x1e = 15g
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA217_REG_TAP_THRESHOLD,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }     

        gDA213Writebuff[0] = 0x05;     // 配置INT1端口 active high for pin INT1 and INT2 下降沿触发
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INT_PIN_CONFIG,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }     

        gDA213Writebuff[0] = 0x20;     // stap中断映射到 INT1 mapping single tap interrupt to INT1
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INTERRUPT_MAPPING1,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }     
        
        gDA213Writebuff[0] = 0x20;     // 使能stap中断  enable the single tap interrupt.    
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INTERRUPT_SETTINGS1,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }         
    }

#ifdef BMS_ACC_CALI_OFFSET_ENABLE    
    gDA213Writebuff[0] = 0x83;     //打开工程模式 步骤1   
    result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_ENGINEERING_MODE,gDA213Writebuff,1);
    PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
    if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
    {
        gAccVar.IICErr++;
    }             
    gDA213Writebuff[0] = 0x69;     //打开工程模式 步骤2
    result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_ENGINEERING_MODE,gDA213Writebuff,1);
    PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
    if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
    {
        gAccVar.IICErr++;
    }  
    gDA213Writebuff[0] = 0xBD;     //打开工程模式 步骤3 
    result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_ENGINEERING_MODE,gDA213Writebuff,1);
    PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
    if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
    {
        gAccVar.IICErr++;
    }      
    

    #ifdef YZ_CROSS_TALK_ENABLE    
    //Y轴的补偿
    result = DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_CHIP_INFO_SECOND,gDA213Readbuff,1);
    PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
    if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
    {
        gAccVar.IICErr++;
    }             
    else
    {
        if(gDA213Readbuff[0] & 0x10)
            yzcross = -(gDA213Readbuff[0] & 0x0f);
        else
            yzcross = (gDA213Readbuff[0] & 0x0f);            
    }         
    #endif
    
    //读取offset
    result = DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_COARSE_OFFSET_TRIM_X,gDA213Readbuff,6);
    PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
    if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
    {
        gAccVar.IICErr++;
    }             
    else
    {       
        for(i = 0;i < 6;i++)
        {
            original_offset[i] = gDA213Readbuff[i];
        }
    }    
    
    result = DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_CUSTOM_OFFSET_X,gDA213Readbuff,3);
    PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
    if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
    {
        gAccVar.IICErr++;
    }             
    else
    {       
        for(i = 0;i < 3;i++)
        {
            original_offset[i + 6] = gDA213Readbuff[i];
        }
    }     

    if(ACC_CONFIG_FLAG != BSPEEPReadOneWord(e_ACC_Config_Flag))
    {
        BSPEEPWriteOneWord(e_ACC_Config_Flag,ACC_CONFIG_FLAG);  //写入标志
        
        gDA213Writebuff[0] = False;
        BSPEEPWriteByte(e_ACC_Is_Cali_OK,gDA213Writebuff,1);
        
        for(i = 0;i < MIR3DA_OFF_SECT_LEN;i++)
        {
            BSPEEPWriteByte(e_ACC_Offset_Start + i,gDA213Writebuff,1);
        }
        cali_ok_flag = 0;
    }
    else
    {
        BSPEEPReadByte(e_ACC_Is_Cali_OK,gDA213Readbuff,1);
        if(True == gDA213Readbuff[0])
        {
            BSPEEPReadByte(e_ACC_Offset_Start,offset_data,MIR3DA_OFF_SECT_LEN);
            
            for(i = 0;i < MIR3DA_OFF_SECT_LEN;i++)
            {
                if(original_offset[i] != offset_data[i])
                {
                    cali_ok_flag = 0;
                }
            }
            
            if(0 == cali_ok_flag)
            {
                //写offset
                for(i = 0;i < MIR3DA_OFF_SECT_LEN;i++) 
                {
                    gDA213Writebuff[i] = offset_data[i];
                }
                result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_COARSE_OFFSET_TRIM_X,gDA213Writebuff,6);
                
                PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
                if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
                {
                    gAccVar.IICErr++;
                }            
                
                result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_CUSTOM_OFFSET_X,&gDA213Writebuff[6],3);
                
                PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
                if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
                {
                    gAccVar.IICErr++;
                }  
                else
                {
                    cali_ok_flag = 1;
                }    
            }            
        }
    }
#endif

#ifdef USE_DA217
        volatile static u8 stmpdata = 0;
        //配置进入工程模式
        gDA213Writebuff[0] = 0x83;
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,0x7F,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIICChannalStateGet(gAccVar.accchn) <= 0));
        if((result > 0) || (BSPIICChannalStateGet(gAccVar.accchn) < 0))
        {
            gAccVar.IICErr++;
        }
        gDA213Writebuff[0] = 0x69;
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,0x7F,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIICChannalStateGet(gAccVar.accchn) <= 0));
        if((result > 0) || (BSPIICChannalStateGet(gAccVar.accchn) < 0))
        {
            gAccVar.IICErr++;
        }        
        gDA213Writebuff[0] = 0xBD;
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,0x7F,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIICChannalStateGet(gAccVar.accchn) <= 0));
        if((result > 0) || (BSPIICChannalStateGet(gAccVar.accchn) < 0))
        {
            gAccVar.IICErr++;
        }
//        //配置关闭 close_sd0_pullup
//        //读&写1
//        result = DA213ReadNRegisters(DA213_IIC_CHANNEL,0x8c,gDA213Readbuff,1);
//        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIICChannalStateGet(gAccVar.accchn) <= 0));    
//        if((result > 0) || (BSPIICChannalStateGet(gAccVar.accchn) < 0))      
//        {
//            gAccVar.IICErr++;
//        }             
//        else
//        {
//            gAccVar.IICErr = 0;
//            stmpdata = gDA213Readbuff[0];
//            stmpdata &= ~0x40; 
//            stmpdata |= (0x00 & 0x40);            
//        }
//        gDA213Writebuff[0] = stmpdata;
//        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,0x8c,gDA213Writebuff,1);
//        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIICChannalStateGet(gAccVar.accchn) <= 0));
//        if((result > 0) || (BSPIICChannalStateGet(gAccVar.accchn) < 0))
//        {
//            gAccVar.IICErr++;
//        }
//        result = DA213ReadNRegisters(DA213_IIC_CHANNEL,0x8c,gDA213Readbuff,1);
//        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIICChannalStateGet(gAccVar.accchn) <= 0));    
//        if((result > 0) || (BSPIICChannalStateGet(gAccVar.accchn) < 0))      
//        {
//            gAccVar.IICErr++;
//        }             
//        else
//        {
//            gAccVar.IICErr = 0;
//            stmpdata = gDA213Readbuff[0];        
//        }        
        //配置关闭 close_iic_pullup
        //读&写2
        result = DA213ReadNRegisters(DA213_IIC_CHANNEL,0x8f,gDA213Readbuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIICChannalStateGet(gAccVar.accchn) <= 0));    
        if((result > 0) || (BSPIICChannalStateGet(gAccVar.accchn) < 0))      
        {
            gAccVar.IICErr++;
        }             
        else
        {
            gAccVar.IICErr = 0;
            stmpdata = gDA213Readbuff[0];
            stmpdata &= ~0x02; 
            stmpdata |= (0x00 & 0x02);            
        }
        gDA213Writebuff[0] = stmpdata;
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,0x8f,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIICChannalStateGet(gAccVar.accchn) <= 0));
        if((result > 0) || (BSPIICChannalStateGet(gAccVar.accchn) < 0))
        {
            gAccVar.IICErr++;
        }
#endif
    
    //PT_ACC_SWRITE(0x2b,0x1e);// n*0.5g; 配置stap阈值为0x1f=15.5g, 0x4 = 2g
    //PT_ACC_SWRITE(0x20,0x05);// 配置INT1端口
    //PT_ACC_SWRITE(0x19,ACC_MAP1_STAP);// stap中断映射到INT1
    //PT_ACC_SWRITE(0x16,ACC_INT1EN_STAP);// 使能stap中断
    //PT_ACC_SWRITE(0x0F,0x03);// +-16g量程，14bit精度
    //PT_ACC_SWRITE(0x10,0x06);// 0x06:odr=62.5Hz,0x8:odr=250Hz
    //PT_ACC_SWRITE(0x11,/*0x80*/0x5e/*0x1e*/);//关闭模式//low power模式//进入正常模式
    
    gAccVar.timer = gTimer1ms;
    PT_WAIT_UNTIL(&gPTDA213Init,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (gTimer1ms - gAccVar.timer > 5));	    //延时5ms  
    
    //PINS_DRV_Init(2, cPinMuxAccConfigArr);    
    //INT_SYS_EnableIRQ(PORT_IRQn); 
    HAL_NVIC_EnableIRQ( EXTI4_15_IRQn );//加速度计的中断为于 PB8 和 PC10
    if(result)
    {
        gAccVar.InitErr = 1;
        gAccVar.RunStat = ACC_Sta_Offline;
        PT_INIT(&gPTDA213OffLine);
    }
    else
    {
        gAccVar.InitErr = 0;
        gAccVar.RunStat = ACC_Sta_Norm;
        PT_INIT(&gPTDA213Norm);        
    }

    PT_INIT(&gPTDA213Init);    
    
    PT_END(&gPTDA213Init);    
}

//=============================================================================================
//函数名称	: void DA213OfflineTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: DA213断线处理任务函数
//注    意	:
//=============================================================================================
void DA213OfflineTask(void)
{
    u8 result = 0;
    
    PT_BEGIN(&gPTDA213OffLine);
    
    gDA213ChipID = 0;
    //读取芯片id
    result |= DA213ReadNRegisters(DA213_IIC_CHANNEL,0,&(gDA213Readbuff[0]),2);
    PT_WAIT_UNTIL(&gPTDA213OffLine,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
    if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
    {
        gAccVar.IICErr++;
    }             
    else
    {
        gDA213ChipID = gDA213Readbuff[1];
    }

    //读取芯片id
    if(DA213_CHIP_ID == gDA213ChipID)
    {
        //复位
        gAccVar.RunStat = ACC_Sta_Init;
        gAccVar.timer = gTimer1ms;
        PT_WAIT_UNTIL(&gPTDA213OffLine,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (gTimer1ms - gAccVar.timer > 5));	    //延时5ms 
        PT_INIT(&gPTDA213Norm);            
    }  
    else
    {
        gAccVar.RunStat = ACC_Sta_Offline;
        gAccVar.timer = gTimer1ms;
        BSPI2C_DeInit(DA213_IIC_CHANNEL);
        PT_WAIT_UNTIL(&gPTDA213OffLine,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (gTimer1ms - gAccVar.timer > 500));
        BSPI2C_Init(DA213_IIC_CHANNEL);	    //延时500ms，进入下一轮检测           
    }
    
    PT_INIT(&gPTDA213OffLine);        

    PT_END(&gPTDA213OffLine);    
}

#ifdef BMS_ACC_CALI_OFFSET_ENABLE
static int mir3da_auto_cali_condition_confirm(int x, int y, int z, int ave_xyz[3])
{
    int    max = 0, min = 0;
    int    i;
    int    x_ok = 0,y_ok = 0,z_ok = 0; 	
    
    stable_sample_pow_sum[stable_sample_cnt] = x*x + y*y + z*z;
    stable_sample_sum[0] += x;
    stable_sample_sum[1] += y;
    stable_sample_sum[2] += z;
    stable_sample_cnt++;
    
    //MI_MSG("---stable_sample_cnt = %d", stable_sample_cnt);
    
    if( stable_sample_cnt < STABLE_CHECK_SAMPLE_NUM )
        return -1;
    stable_sample_cnt = 0;
    
    max = stable_sample_pow_sum[0];
    min = stable_sample_pow_sum[0];
    stable_sample_pow_sum[0] = 0;
    
    for(i = 1; i < STABLE_CHECK_SAMPLE_NUM; i++)
    {
        if( stable_sample_pow_sum[i] > max )
            max = stable_sample_pow_sum[i];
        if( stable_sample_pow_sum[i] < min )
            min = stable_sample_pow_sum[i];
        stable_sample_pow_sum[i] = 0;
    }
    //MI_MSG("---max = %d; min = %d", max, min);

    ave_xyz[0] = stable_sample_sum[0]/STABLE_CHECK_SAMPLE_NUM;
    stable_sample_sum[0] = 0;
    ave_xyz[1] = stable_sample_sum[1]/STABLE_CHECK_SAMPLE_NUM;
    stable_sample_sum[1] = 0;
    ave_xyz[2] = stable_sample_sum[2]/STABLE_CHECK_SAMPLE_NUM;
    stable_sample_sum[2] = 0;
            
    //MI_MSG("ave_x = %d, ave_y = %d, ave_z = %d", ave_xyz[0], ave_xyz[1], ave_xyz[2]);
    x_ok =  (abs(ave_xyz[0]) < AUTO_CALI_THRESHOLD_XY) ? 1:0;
    y_ok =  (abs(ave_xyz[1]) < AUTO_CALI_THRESHOLD_XY) ? 1:0;
    z_ok =  (abs(abs(ave_xyz[2])-1024) < AUTO_CALI_THRESHOLD_Z) ? 1:0;

    if( (abs(max-min) > STABLE_CHECK_THRESHOLD) ||((x_ok + y_ok + z_ok) < 2) ) 
    {
        return -1;
    }

    return 0;
}

static void mir3da_cali_off_to_lsb(int off, int *coarse, int coarse_step, int *fine, int fine_step)
{
    *coarse = off/coarse_step;
    *fine = 100*(off-(*coarse)*coarse_step)/fine_step;
    
    //MI_MSG("off = %d; delta_coarse = %d; delta_fine = %d", off, *coarse, *fine);
}
#endif

//=============================================================================================
//函数名称	: void DA213NormalTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: DA213正常任务函数
//注    意	:
//=============================================================================================
void DA213NormalTask(void)
{
    u8 result = 0;
    
    PT_BEGIN(&gPTDA213Norm);
    
    //获取中断标志
    if(DA213_GET_INT == (gAccVar.Ctrl & DA213_GET_INT))
    {
        static u8 ReadCnt = 0;
        static u8 Recordflg = False;
        
        result = DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_MOTION_FLAG,gDA213Readbuff,1);
        PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }             
        else
        {
            gAccVar.IICErr = 0;
            gAccVar.AccData.MotionFlg = gDA213Readbuff[0];        
        }
        
        if((0x05 == (gAccVar.AccData.MotionFlg & 0x05)) && (False == Recordflg))
        {
            //加速度计中断
            Recordflg = True;
            IOCheckACCIntCallback();
        }
        
        //读取25次
        if(ReadCnt++ > 25)
        {
            ReadCnt = 0;
            Recordflg = False;
            DA213ClearCtrl(DA213_GET_INT);
        }
    }
    
    //读取数据
    if(DA213_GET_DATA == (gAccVar.Ctrl & DA213_GET_DATA))
    {
        //读取x轴，y轴，z轴的加速度
        result = DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_ACC_X_LSB,gDA213Readbuff,6);
        PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }             
        else
        {
            gAccVar.IICErr = 0;
//            gAccVar.AccData.ax = DA213CalculateData(gDA213Readbuff);
//            gAccVar.AccData.ay = DA213CalculateData(gDA213Readbuff + 2);
//            gAccVar.AccData.az = DA213CalculateData(gDA213Readbuff + 4);    
            gAccVar.AccData.ax = ((short)(gDA213Readbuff[1] << 8 | gDA213Readbuff[0]))>> 3;
            gAccVar.AccData.ay = ((short)(gDA213Readbuff[3] << 8 | gDA213Readbuff[2]))>> 3;
            gAccVar.AccData.az = ((short)(gDA213Readbuff[5] << 8 | gDA213Readbuff[4]))>> 3;	            
        }        

        DA213ClearCtrl(DA213_GET_DATA);
    }

    //测试
    if(DA213_TEST_INT == (gAccVar.Ctrl & DA213_TEST_INT))
    {
        if(1 == gConfig.accpara.TapEn)
        {

            gDA213Writebuff[0] = 0x04;      //配置INT1端口中断反向
            result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INT_PIN_CONFIG,gDA213Writebuff,1);
            PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
            if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
            {
                gAccVar.IICErr++;
            }             
            
            
            gDA213Writebuff[0] = 0x05;      //配置INT1端口
            result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INT_PIN_CONFIG,gDA213Writebuff,1);
            PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
            if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
            {
                gAccVar.IICErr++;
            } 
        }  
        else
        {
            #if defined(USE_DA217)
            gDA213Writebuff[0] = 0x05;      //配置INT2端口中断反向
            result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INT_PIN_CONFIG,gDA213Writebuff,1);
            PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIICChannalStateGet(gAccVar.accchn) <= 0));
            if((result > 0) || (BSPIICChannalStateGet(gAccVar.accchn) < 0))      
            {
                gAccVar.IICErr++;
            }             
            
            
            gDA213Writebuff[0] = 0x01;      //配置INT2端口
            result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INT_PIN_CONFIG,gDA213Writebuff,1);
            PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIICChannalStateGet(gAccVar.accchn) <= 0));
            if((result > 0) || (BSPIICChannalStateGet(gAccVar.accchn) < 0))      
            {
                gAccVar.IICErr++;
            }               
            #else
            gDA213Writebuff[0] = 0x01;      //配置INT2端口中断反向
            result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INT_PIN_CONFIG,gDA213Writebuff,1);
            PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
            if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
            {
                gAccVar.IICErr++;
            }             
            
            
            gDA213Writebuff[0] = 0x05;      //配置INT2端口
            result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_INT_PIN_CONFIG,gDA213Writebuff,1);
            PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
            if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
            {
                gAccVar.IICErr++;
            }   
            #endif       
            
        }
        
        DA213ClearCtrl(DA213_TEST_INT);           
    }    

    //挂起模式
    if(DA213_SUSPEND_MODE == (gAccVar.Ctrl & DA213_SUSPEND_MODE))
    {    
        #if defined(USE_DA217)
        gDA213Writebuff[0] = 0x85;      //配置加速度计挂起模式
        #else
        gDA213Writebuff[0] = 0x9E;      //配置加速度计挂起模式
        #endif
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_POWERMODE_BW,gDA213Writebuff,1);
        PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
            DA213ClearCtrl(DA213_SUSPEND_MODE); 
        }                     
        else
        {
            DA213ClearCtrl(DA213_SUSPEND_MODE); 
        }
    }
    
#ifdef BMS_ACC_CALI_OFFSET_ENABLE    
    //校准加速度计offset
    if(DA213_CALI_OFFSET == (gAccVar.Ctrl & DA213_CALI_OFFSET))
    {      
        int i;
        
//        gDA213Writebuff[0] = 0x42;     // +-8g量程，14bit精度    
//        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_G_RANGE,gDA213Writebuff,1);
//        PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
//        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
//        {
//            gAccVar.IICErr++;
//        }     
        
        gAccVar.timer = gTimer1ms;
        PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (gTimer1ms - gAccVar.timer > 50));	    //延时5ms  
        
        //读取x轴，y轴，z轴的加速度
        result = DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_ACC_X_LSB,gDA213Readbuff,6);
        PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }             
        else
        {
            gAccVar.IICErr = 0;
            
            gRawX = ((short)(gDA213Readbuff[1] << 8 | gDA213Readbuff[0]))>> 3;
            gRawY = ((short)(gDA213Readbuff[3] << 8 | gDA213Readbuff[2]))>> 3;
            gRawZ = ((short)(gDA213Readbuff[5] << 8 | gDA213Readbuff[4]))>> 3;	
//            gRawX = DA213CalculateData(gDA213Readbuff);
//            gRawY = DA213CalculateData(gDA213Readbuff + 2);
//            gRawZ = DA213CalculateData(gDA213Readbuff + 4);            
            
            #if YZ_CROSS_TALK_ENABLE
            if(yzcross)
                gRawY = gRawY - gRawZ * yzcross / 100;
            #endif	
        }       
        
        if(1 == cali_ok_flag)
        {
            DA213ClearCtrl(DA213_CALI_OFFSET); 
            goto EXIT;
            //printf(cali ok!);             
        }
        else
        {
            if( mir3da_auto_cali_condition_confirm(gRawX, gRawY, gRawZ, xyz) )
            {
                //不可进行校准
                goto EXIT;
            }              
        }
        
        /*
        //写offset
        for(i = 0;i < MIR3DA_OFF_SECT_LEN;i++) 
        {
            gDA213Writebuff[i] = original_offset[i];
        }
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_COARSE_OFFSET_TRIM_X,gDA213Writebuff,6);
        
        PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }            
        
        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_CUSTOM_OFFSET_X,&gDA213Writebuff[6],3);
        
        PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
        {
            gAccVar.IICErr++;
        }         
        */
        
        /* compute step NSA_NTO_cali_step_calc*/
        {
            result = DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_SENSITIVITY_TRIM_X,gDA213Readbuff,3);
            PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
            if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
            {
                gAccVar.IICErr++;
            }             
            else
            {       
                fine_gain[0] = gDA213Readbuff[0];
                fine_gain[1] = gDA213Readbuff[1];
                fine_gain[2] = gDA213Readbuff[2];
            }
            
            result = DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_SENS_COARSE_TRIM,gDA213Readbuff,1);
            PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
            if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
            {
                gAccVar.IICErr++;
            }             
            else
            {       
                coarse_gain = gDA213Readbuff[0];
            }           
            for(i = 0;i < 3;i++) 
            {  
                // *100*1000
                total_gain[i] = ((1000 + (fine_gain[i]&0x1F)*1000/32)/15) * fine_dig_gain_map[((fine_gain[i]>>5)&0x03)] * coarse_gain_map[coarse_gain&0x07]; 
                coarse_step[i] = (int)(total_gain[i] * 500 / 100000);
                fine_step[i] = (int)(total_gain[i] * 293 / 100000);
                custom_step[i] = (int)(total_gain[i] * 390 / 100000);
            }
            //MI_MSG("coarse_step_x = %d, coarse_step_y = %d, coarse_step_z = %d\n", coarse[0], coarse[1], coarse[2]);
            //MI_MSG("fine_step_x = %d, fine_step_y = %d, fine_step_z = %d\n", x100_fine[0], x100_fine[1], x100_fine[2]);
            //MI_MSG("custom_step_x = %d, custom_step_y = %d, custom_step_z = %d\n", x100_cust[0], x100_cust[1], x100_cust[2]);            
        }
        
        // save swap/plarity old setting
        {
            result = DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_SWAP_POLARITY,gDA213Readbuff,1);
            PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
            if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
            {
                gAccVar.IICErr++;
            }             
            else
            {       
                swap_plarity_old = gDA213Readbuff[0];
            }
            
            coarse_step[2] = 2 *coarse_step[2];

            if((swap_plarity_old & (1<<0)))
            {
               temp = xyz[0];
               xyz[0] = ((swap_plarity_old & (1<<2)) == 0) ? (-xyz[1]) :xyz[1];
               xyz[1] = ((swap_plarity_old & (1<<3)) == 0) ? (-temp) :temp;
            }
            else
            {
               xyz[0] = ((swap_plarity_old & (1<<3)) == 0) ? (-xyz[0]) :xyz[0];
               xyz[1] = ((swap_plarity_old & (1<<2)) == 0) ? (-xyz[1]) :xyz[1];
            }
            xyz[2] = ((swap_plarity_old & (1<<1)) == 0) ? (-xyz[2]) :xyz[2];                
            
        }
        
        //NSA_once_calibrate
        {
            //读取offset
            result = DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_COARSE_OFFSET_TRIM_X,gDA213Readbuff,6);
            PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
            if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
            {
                gAccVar.IICErr++;
            }             
            else
            {       
                for(i = 0;i < 6;i++)
                {
                    offset_data[i] = gDA213Readbuff[i];
                }
            }    
            
            result = DA213ReadNRegisters(DA213_IIC_CHANNEL,DA213_REG_CUSTOM_OFFSET_X,gDA213Readbuff,3);
            PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));    
            if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
            {
                gAccVar.IICErr++;
            }             
            else
            {       
                for(i = 0;i < 3;i++)
                {
                    offset_data[i + 6] = gDA213Readbuff[i];
                }
            } 
            coarse[0] = offset_data[0] & 0x3f;
            coarse[1] = offset_data[1] & 0x3f;
            coarse[2] = offset_data[2] & 0x3f;
            fine[0] = (((int)offset_data[0] << 2) & 0x300)|offset_data[3];
            fine[1] = (((int)offset_data[1] << 2) & 0x300)|offset_data[4];
            fine[2] = (((int)offset_data[2] << 2) & 0x300)|offset_data[5];
     
            //MI_MSG("Old coarse_x = %d; coarse_y = %d; coarse_z = %d; fine_x = %d; fine_y = %d; fine_z = %d;", coarse[0], co
                
            /* 0 means auto detect z direction assume z axis is verticle */
            if ((abs(target[0]) + abs(target[1]) + abs(target[2])) == 0)
            {
                target[2] = (xyz[2] > 0) ? 1024 : (-1024);
            }
            
            for(i = 0;i < 3; i++)
            {
                coarse_step[i] *= coarse[i] >= 32 ? (-1) : 1;
                mir3da_cali_off_to_lsb((xyz[i]-target[i]), &coarse_delta[i], coarse_step[i], &fine_delta[i], fine_step[i]);
                
                coarse[i] += coarse_delta[i];
                fine[i] += fine_delta[i];
                offset_data[i] = coarse[i]|((fine[i]>>2)&0xc0);
                offset_data[i+3] = fine[i]&0xFF;
            } 
        
            //写offset
            for(i = 0;i < MIR3DA_OFF_SECT_LEN;i++) 
            {
                gDA213Writebuff[i] = offset_data[i];
            }
            result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_COARSE_OFFSET_TRIM_X,gDA213Writebuff,6);
            
            PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
            if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
            {
                gAccVar.IICErr++;
            }            
            
            result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_CUSTOM_OFFSET_X,&gDA213Writebuff[6],3);
            
            PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
            if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
            {
                gAccVar.IICErr++;
            }  
            else
            {
                BSPEEPWriteByte(e_ACC_Offset_Start,gDA213Writebuff,MIR3DA_OFF_SECT_LEN);
                gDA213Writebuff[9] = True;
                BSPEEPWriteByte(e_ACC_Is_Cali_OK,&gDA213Writebuff[9],1);
                cali_ok_flag = 1;
            }             
        }
        
        DA213ClearCtrl(DA213_CALI_OFFSET); 
        
 EXIT:	  
        is_cali = 0;
//        gDA213Writebuff[0] = 0x03;     // +-16g量程，14bit精度    
//        result = DA213WriteNRegisters(DA213_IIC_CHANNEL,DA213_REG_G_RANGE,gDA213Writebuff,1);
//        PT_WAIT_UNTIL(&gPTDA213Norm,BSPTaskStart(TASK_ID_ACCELERO_TASK, 2) && (BSPIIC_StateGet( DA213_IIC_CHANNEL ) <= 0));
//        if((result > 0) || (BSPIIC_StateGet( DA213_IIC_CHANNEL ) < 0))      
//        {
//            gAccVar.IICErr++;
//        }             
    }
#endif    

    
    if(gAccVar.IICErr >= 3 && gAccVar.IICErr < 6)	//出现通信故障
	{
        //IIC解锁
        /*
        BSPGPIOUnLockLpIIC();
        BSPIICReInit(eIIC0);
        */
        BSPI2C_DeInit(DA213_IIC_CHANNEL);
        BSPI2C_Init(DA213_IIC_CHANNEL);
	}    
	else if(gAccVar.IICErr >= 6)	//出现断线故障
	{
		gAccVar.RunStat = ACC_Sta_Offline;
	}
    BSPTaskStart(TASK_ID_ACCELERO_TASK, 200);	//空闲时200ms周期
    
    PT_INIT(&gPTDA213Norm);        

    PT_END(&gPTDA213Norm);    
}

//=============================================================================================
//函数名称	: u8 DA213MainTask(void *p)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: DA213主任务函数
//注    意	:
//=============================================================================================
void DA213MainTask(void *p)
{
	(void)p;
	
	PT_BEGIN(&gPTDA213);   

	for(;;)
    {
		switch(gAccVar.RunStat)
		{
            //初始化任务
			case ACC_Sta_Init:
				DA213InitTask();
				break;
            
            //正常工作任务
			case ACC_Sta_Norm:
                DA213NormalTask();
				break;
            
            //断线任务
			case ACC_Sta_Offline:
				DA213OfflineTask();
				break;

			default:
				;
				break;
		}
        
		PT_NEXT(&gPTDA213);
	}
    PT_END(&gPTDA213);    
}

/*****************************************end of DA213.c*****************************************/
