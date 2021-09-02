//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: MAX17205.c
//创建人  	: Handry
//创建日期	: 
//描述	    : MAX17205驱动源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "MAX17205.h"
#include "BSPSysTick.h"
#include "BSPSystemCfg.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "BSPTimer.h"
#include "Storage.h"
#include "ParaCfg.h"
#include "DataDeal.h"
#include "Sample.h"
#include "FuelDffsTab.h"
#include "pt.h"

//=============================================================================================
//全局变量
//=============================================================================================
u8	gMAX172XXReadbuff[MAX1720X_READBUFF_LEN] = {0};  	//max17205接收缓冲数组
u8	gMAX172XXWritebuff[MAX1720X_WRITEBUFF_LEN] = {0};  	//max17205接收缓冲数组
u16 Max1720x_ConfigTableRead[BATTERY_CONFIG_LEN] = {0}; //读取参数数组
t_MAX1720X_VAR gMax17205Var = {0};	                    //max17205读取的数据
t_MAX1720X_CAL gMax17205Cal = {100,0};                  //max17205标定的数据
pt gPTMax17205;					                        //max17205任务pt线程控制变量
pt gPTMax17205Init;			                            //max17205初始化任务pt线程控制变量
pt gPTMax17205Norm;					                    //max17205正常任务pt线程控制变量
pt gPTMax17205Update;					                //max17205模型数据更新任务pt线程控制变量
pt gPTMax17205Fault;					                //max17205模型错误任务pt线程控制变量
pt gPTMax17205ShutDown;					                //max17205模型关机任务pt线程控制变量
u8  gSOCWritebuff[50] = {0};                            //IIC通信数据
s16 gAvgCurTab[MAX1720X_CURR_CALIB_CNT] = {0};

unsigned short gDFFSRead[DFFS_NUM] = {0};               //读取的内部电量计划memory数据
    
struct t_BatteryConfig Max1720x_ConfigTable[BATTERY_CONFIG_LEN] =           //配置参数数组
{
    {MAX1720X_PACKCFG_ADDR,BATTERY_CELLNUM | 0x9900},						//串数、均衡使能等配置，使能AIN1和AIN2温度传感器
    {MAX1720X_FILTERCFG_ADDR,0x0EA0},										//平均值时间配置	电流平均时间配置为0.7S
//	{MAX1720X_NRSENSE_ADDR,BATTERY_DFRENSE_VAL},							//分流器阻值配置
};

//=============================================================================================
//静态函数声明
//=============================================================================================
//=============================================================================================
//函数名称	: static void MAX17205ClearRecBuff(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 接收数据缓冲区清零
//注    意	:
//=============================================================================================
static void MAX17205ClearRecBuff(void);

//=============================================================================================
//函数名称	: u16 Max17205GetnRSense(void)
//输入参数	: void
//输出参数	: u16
//静态变量	:
//功    能	: MAX17205读取分流器电阻 单位：mΩ
//注    意	:
//=============================================================================================
//static u16 Max17205GetnRSense(void);

//=============================================================================================
//接口函数定义
//=============================================================================================
//=============================================================================================
//函数名称	: u8 Max17205Init(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: MAX17205初始化函数
//注    意	:
//=============================================================================================
u8 Max17205Init(void)
{
	u8 i2cerr = 0;
	
	PT_INIT(&gPTMax17205);	    //初始化通信任务pt线程控制变量
	PT_INIT(&gPTMax17205Init);	
	PT_INIT(&gPTMax17205Norm);
	PT_INIT(&gPTMax17205Fault);    
	PT_INIT(&gPTMax17205Update);
    
	Max17205DataInit();		//数据初始化

    //获取分流器阻值，以计算电流使用
	gMax17205Cal.rsense = gConfig.calpara.fuelrense;
    //至少剩余的更新次数
    gMax17205Var.AtLesRem = gConfig.calpara.FuelAtLesRem;

    //进入初始化流程
	gMax17205Var.RunStat = eFuel_Sta_Init;
    
    //获取IIC通道
    gMax17205Var.socchn = BSPIICChannalReq(IIC_ROUTE_SOC);
    
    //gMax17205Var.ShdnFlag = True;
    
	return i2cerr;

//	gMax17205Cal.rsense = Max17205GetnRSense();	//获取分流器阻值，以计算电流使用
//
//	if(gMax17205Cal.rsense == 0)	//分流器电阻值作分母，不能用0处理；
//	{
//		gMax17205Cal.rsense = 1;	//1mΩ
//	}
}

//=============================================================================================
//函数名称	: u8 Max17205ResumeInit(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: MAX17205休眠唤醒初始化函数
//注    意	:
//=============================================================================================
u8 Max17205ResumeInit(void)
{
	u8 i2cerr = 0;
	
	PT_INIT(&gPTMax17205);	    //初始化通信任务pt线程控制变量
	PT_INIT(&gPTMax17205Init);	
	PT_INIT(&gPTMax17205Norm);
	PT_INIT(&gPTMax17205Fault);    
	PT_INIT(&gPTMax17205Update);
    
    if(eFuel_Sta_Norm != gMax17205Var.RunStat)
    {
        gMax17205Var.RunStat = eFuel_Sta_Init; //电量计初始化
        gMax17205Var.ShdnFlag = True;
    }
    
    //获取IIC通道
    gMax17205Var.socchn = BSPIICChannalReq(IIC_ROUTE_SOC);
    
	return i2cerr;
}

//=============================================================================================
//函数名称	: void Max17205DataInit(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: MAX17205数据初始化函数
//注	意	:
//=============================================================================================
void Max17205DataInit(void)
{
    gMax17205Var.RepSOC = 0;
    gMax17205Var.SOH = 0;
    gMax17205Var.FullChgCap = 0;
    gMax17205Var.RepRemainCap = 0;
    gMax17205Var.FullChgTime = 0;
    gMax17205Var.EmptyDchgTime = 0;
    gMax17205Var.CycleCount = 0;
    gMax17205Var.MaxCurr = 0;
    gMax17205Var.Averagecurrent = 0;
    gMax17205Var.Temperature = 0;
    gMax17205Var.Voltage = 0;
    gMax17205Var.CellVolt = 0;
    gMax17205Var.RemainPower = 0;
	gMax17205Var.Flags = 0;
}

//=============================================================================================
//函数名称	: void Max17205SetCtrl(u16 ctrl)
//输入参数	: ctrl:控制位，详细定义见头文件宏定义
//输出参数	: void
//静态变量	: void
//功	能	: MAX17205设置控制标志位函数
//注	意	:
//=============================================================================================
void Max17205SetCtrl(u16 ctrl)
{
    gMax17205Var.Ctrl |= ctrl;
}

//=============================================================================================
//函数名称	: void Max17205SetCtrl(u16 ctrl)
//输入参数	: ctrl:控制位，详细定义见头文件宏定义
//输出参数	: void
//静态变量	: void
//功	能	: MAX17205清除控制标志位函数
//注	意	:
//=============================================================================================
void Max17205ClearCtrl(u16 ctrl)
{
    gMax17205Var.Ctrl &= (~ctrl);
}

//=============================================================================================
//函数名称	: static void MAX17205ClearRecBuff(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 接收数据缓冲区清零
//注    意	:
//=============================================================================================
static void MAX17205ClearRecBuff(void)
{
	u8 i = 0;

	for(i = 0;i < MAX1720X_READBUFF_LEN; i++)
	{
		gMAX172XXReadbuff[i] = 0;
	}
}

 //=============================================================================================
 //函数名称	: u8 Max17205WriteWord(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr)
 //输入参数	: void
 //输出参数	: void
 //静态变量	: void
 //功	能	: MAX17205写寄存器函数
 //注	意	:
 //=============================================================================================
u8 MAX17205WriteRegister(e_IICNUM num, u16 regaddr,u16 word)
{
	u8 errstat = 0;									//IIC通信错误信息
	u8 regaddress = 0;
	u8 deviceaddr = 0;
	u16 writeword = 0;

	if((num > eIIC1) ||  (regaddr > MAX1720X_MAX_REGADDR))  //参数有效性判断
	{
		return MAX1720X_IIC_FAULT_STAT;
	}
    
	if(regaddr < 0x100)
	{
		deviceaddr = MAX1720X_WRITE_ADDR;
		regaddress = (u8)regaddr;
	}
	 else
	 {
		 deviceaddr = MAX1720X_WRITE_EXPADDR;
		 regaddress = (u8)(regaddr & 0x00ff);
	 }

	writeword = word;

	gMAX172XXWritebuff[0] = deviceaddr;
	gMAX172XXWritebuff[1] = regaddress;
	gMAX172XXWritebuff[2] = (u8)(writeword & 0x00ff);			//先写低字节，后写高字节
	gMAX172XXWritebuff[3] = (u8)((writeword >> 8) & 0x00ff);

	if(0 == errstat)
	{
		//errstat = BSPIICSendMsg(num,writebuff,4,1);
        errstat = BSPIICWrite(gMax17205Var.socchn,gMAX172XXWritebuff[0],(u8 *)(&(gMAX172XXWritebuff[1])),3);
	}
	
	return errstat;
 }

//=============================================================================================
//函数名称	: u8 Max17205WriteWord(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: MAX17205写寄存器函数
//注	意	:
//=============================================================================================
u8 MAX17205WriteRegisterWait(e_IICNUM num, u16 regaddr,u16 word)
{
	u8 errstat = 0;									//IIC通信错误信息
	u8 regaddress = 0;
	u8 deviceaddr = 0;
	u16 writeword = 0;
	u8 writebuff[4] = {0};

	if((num > eIIC1) ||  (regaddr > MAX1720X_MAX_REGADDR))  //参数有效性判断
	{
		return MAX1720X_IIC_FAULT_STAT;
	}

	if(regaddr < 0x100)
	{
		deviceaddr = MAX1720X_WRITE_ADDR;
		regaddress = (u8)regaddr;
	}
	 else
	 {
		 deviceaddr = MAX1720X_WRITE_EXPADDR;
		 regaddress = (u8)(regaddr & 0x00ff);
	 }

	writeword = word;

	writebuff[0] = deviceaddr;
	writebuff[1] = regaddress;
	writebuff[2] = (u8)(writeword & 0x00ff);			//先写低字节，后写高字节
	writebuff[3] = (u8)((writeword >> 8) & 0x00ff);

	if(0 == errstat)
	{
		errstat = BSPIICSendMsg(num,writebuff,4,1);
	}
    
	return errstat;
}

 //=============================================================================================
 //函数名称	: u8 MAX17205ReadRegister(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr)
 //输入参数	: num:MAX1720X_IIC_CHANNEL/eIIC1	*p_buffer:接收缓冲区		bufflen:接收的数据个数		regaddr:MAX17205寄存器地址
 //输出参数	: IIC通信错误信息
 //静态变量	: void
 //功	能	: MAX17205读取信息函数
 //注	意	:
 //=============================================================================================
 u8 MAX17205ReadRegister(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr)
 {
	 u8 errstat = 0;	//IIC通信错误信息
	 u8 regaddress = 0;

	 if((num > eIIC1) || (bufflen > MAX1720X_READBUFF_LEN)
		|| (p_buffer == NULL) ||  (regaddr > MAX1720X_MAX_REGADDR))  //参数有效性判断
	 {
		 return MAX1720X_IIC_FAULT_STAT;
	 }


	 if(regaddr < 0x100)
	 {
		 regaddress = (u8)regaddr;
         gSOCWritebuff[0] = regaddress;
		 //errstat = BSPIICRcvMsg(num,MAX1720X_WRITE_ADDR,regaddress,p_buffer,bufflen,1);	//发送读取报文，标准地址
         errstat = BSPIICWriteAndRead(gMax17205Var.socchn,MAX1720X_WRITE_ADDR,&gSOCWritebuff[0],1,p_buffer,bufflen); 	 
     }
	 else
	 {
		 regaddress = (u8)(regaddr & 0x00ff);
         gSOCWritebuff[0] = regaddress;
		 //errstat = BSPIICRcvMsg(num,MAX1720X_WRITE_EXPADDR,regaddress,p_buffer,bufflen,1);//发送读取报文，拓展地址，最大10bit
         errstat = BSPIICWriteAndRead(gMax17205Var.socchn,MAX1720X_WRITE_EXPADDR,&gSOCWritebuff[0],1,p_buffer,bufflen); 	 
     }

	 return errstat;
 }

 //=============================================================================================
 //函数名称	: u8 MAX17205ReadRegisterWait(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr)
 //输入参数	: num:eIIC0/eIIC1	*p_buffer:接收缓冲区		bufflen:接收的数据个数		regaddr:MAX17205寄存器地址
 //输出参数	: IIC通信错误信息
 //静态变量	: void
 //功	能	: MAX17205读取信息函数
 //注	意	:
 //=============================================================================================
 u8 MAX17205ReadRegisterWait(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr)
 {
	 u8 errstat = 0;	//IIC通信错误信息
	 u8 regaddress = 0;

	 if((num > eIIC1) || (bufflen > MAX1720X_READBUFF_LEN)
		|| (p_buffer == NULL) ||  (regaddr > MAX1720X_MAX_REGADDR))  //参数有效性判断
	 {
		 return MAX1720X_IIC_FAULT_STAT;
	 }

	 if(regaddr < 0x100)
	 {
		 regaddress = (u8)regaddr;
		 errstat = BSPIICRcvMsg(num,MAX1720X_WRITE_ADDR,regaddress,p_buffer,bufflen,1);	//发送读取报文，标准地址
	 }
	 else
	 {
		 regaddress = (u8)(regaddr & 0x00ff);
		 errstat = BSPIICRcvMsg(num,MAX1720X_WRITE_EXPADDR,regaddress,p_buffer,bufflen,1);//发送读取报文，拓展地址，最大10bit
	 }

	 return errstat;
 }

 //=============================================================================================
 //函数名称	: u16 Max17205GetRegisterValue(u16 max1720x_regaddr)
 //输入参数	: void
 //输出参数	: u16
 //静态变量	: 16位寄存器的值
 //功	能	: MAX17205读取max17205寄存器的值
 //注	意	:
 //=============================================================================================
 u16 Max17205GetRegisterValue(u16 max1720x_regaddr)
{
	u16 tmp = 0;
	u8 errstat = 0;

	errstat = MAX17205ReadRegisterWait(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, max1720x_regaddr);

	if(MAX1720X_IIC_NOFAULT_STAT == errstat)	//如有通信错误，返回数值0
	{
		tmp = ((u16)gMAX172XXReadbuff[1] << 8 | (u16)gMAX172XXReadbuff[0]);

		gMax17205Var.IICErr = 0;	//清零通信错误信号
		
		return tmp;
	}

	gMax17205Var.IICErr++;		//通信错误信号加1，滤波

	MAX17205ClearRecBuff();		//清零接收缓冲区
	return 0;

}

 //=============================================================================================
 //函数名称	: void Max17205GetBatteryData(void)
 //输入参数	: max1720x_regaddr:max17205寄存器地址	max1720x_var:电池数据结构体地址
 //输出参数	: void
 //静态变量	: void
 //功    能	: 获取MAX17205电池数据函数接口
 //注    意	:
 //=============================================================================================
 void Max17205GetBatteryData(u16 max1720x_regaddr,t_MAX1720X_VAR *max1720x_var)
 {
 	u16 readtemp;

 	readtemp = Max17205GetRegisterValue(max1720x_regaddr);

 	//转换寄存器值
     switch(max1720x_regaddr)
     {
         case MAX1720X_REPSOC_ADDR: 	//获取SOC

        	 max1720x_var -> RepSOC = ((float)readtemp) / 256;  								//1%

             break;

         case MAX1720X_REPSOH_ADDR: 	//获取SOH

        	 max1720x_var -> SOH = (u16)((float)readtemp) / 256;  								//1%

             break;

         case MAX1720X_FULLCAP_ADDR: 	//获取满充容量

        	 max1720x_var -> FullChgCap = readtemp / 2;								//1mAH

             break;

         case MAX1720X_REPCAP_ADDR:		//获取剩余容量

        	 max1720x_var -> RepRemainCap = readtemp / 2;								//1mAH

             break;

         case MAX1720X_TTF_ADDR: 		//获取满充时长

        	 max1720x_var -> FullChgTime = (u16)((float)readtemp) * 5.625 / 60;			//1min

             break;

         case MAX1720X_TTE_ADDR: 		//获取放空时长

        	 max1720x_var -> EmptyDchgTime = (u16)((float)readtemp) * 5.625 / 60;			//1min

             break;

         case MAX1720X_VBAT_ADDR:  		//获取电池包电压

        	 max1720x_var -> Voltage = (u16)((float)readtemp) * 1250 / 1000;  					//1mV

         	 break;

         case MAX1720X_AVGCURENT_ADDR:  //获取平均电流

        	 max1720x_var -> Averagecurrent = (s16)((float)readtemp * 1562.52) / gMax17205Cal.rsense; 		//1mA

             break;

         case MAX1720X_MAXMINCURENT_ADDR://获取最大/最小电流

        	 max1720x_var -> MaxCurr = (s16)((float)((readtemp >> 8) & 0xff) * 0.4) / gMax17205Cal.rsense; //1mA

             break;

         case MAX1720X_BATCYCLE_ADDR:  	//获取充放电循环次数

        	 max1720x_var -> CycleCount = (u16)(readtemp);

             break;

         case MAX1720X_TEMP_ADDR:  		//获取电池温度

				max1720x_var -> Temperature = (s16)(((float)readtemp) / 256 * 10);  				  //0.1℃
                //gMax17205Var.Temperature = (u16)((((float)readtemp) - 2730));  				  //0.1℃


             break;

         case MAX1720X_VCELLX_ADDR:  //获取最低单体电压	可选择

        	 max1720x_var -> CellVolt = (u16)((float)readtemp * 0.078125);  					//1mV

             break;

         default:
         	;
             break;
     }

 }

 //=============================================================================================
 //函数名称	: u16 Max17205GetnRSense(void)
 //输入参数	: void
 //输出参数	: u16
 //静态变量	:
 //功    能	: MAX17205读取分流器电阻 单位：mΩ
 //注    意	:
 //=============================================================================================
// static u16 Max17205GetnRSense(void)
// {
// 	float tmp = 0;
// 	u8 errstat = 0;
//
// 	errstat = MAX17205ReadRegisterWait(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_NRSENSE_ADDR);
//
////	errstat |=  MAX17205WaitIIC(eMax17205Read);
//
// 	if(MAX1720X_IIC_NOFAULT_STAT == errstat)	//如有通信错误，返回数值0
// 	{
// 		tmp = ((u16)gMAX172XXReadbuff[1] * 256 + (u16)gMAX172XXReadbuff[0])  / 100;  //mΩ
//
// 		return tmp;
// 	}
//
// 	MAX17205ClearRecBuff();		//清零接收缓冲区
// 	return 0;
// }

 //=============================================================================================
 //函数名称  : void MAX17205_SetCalCurr(s16 act_current)
 //输入参数  : void
 //输出参数  : void
 //静态变量  : void
 //功		 能 : 设置校准电流
 //注		 意 :
 //=============================================================================================
void MAX17205SetCalCurr(s16 act_current)
 {
	gMax17205Cal.calibcurr = act_current;
   	gMax17205Var.Ctrl |= MAX1720X_START_CALIBCURR;
 }


//=============================================================================================
//函数名称	: u8 Max17205FullReset(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: MAX17205芯片复位函数
//注    意	:
//=============================================================================================
u8 Max17205FullReset(void)
{
	u8 i2cerr = 0;

	i2cerr |= MAX17205WriteRegisterWait(MAX1720X_IIC_CHANNEL,MAX1720X_COMMAND_ADDR,0x000f);	//复位IC硬件
//	i2cerr |= MAX17205WaitIIC(eMax17205Write);
	
	BSPSysDelay1ms(10);		//等待 tPOR = 10ms时间

	i2cerr |= MAX17205WriteRegisterWait(MAX1720X_IIC_CHANNEL,MAX1720X_CONFIG2_ADDR,0x0001);	//复位电量计操作
//	i2cerr |= MAX17205WaitIIC(eMax17205Write);
	
	BSPSysDelay1ms(10);		//等待 tPOR = 10ms时间

	return i2cerr;
}

//=============================================================================================
//函数名称	: u8 Max17205ShutDown(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: MAX17205关机函数
//注    意	:
//=============================================================================================
u8 Max17205ShutDown(void)
{
	u8 i2cerr = 0;
	u16 configval = 0;

	configval = Max17205GetRegisterValue(MAX1720X_CONFIG_ADDR);

	i2cerr |= MAX17205WriteRegisterWait(MAX1720X_IIC_CHANNEL,MAX1720X_CONFIG_ADDR,configval | 0x0080);	//发送关机命令

	BSPSysDelay1ms(30);		//等待 tPOR = 10ms时间

	return i2cerr;
}

//=============================================================================================
//函数名称	: void Max17205RefreshDataToBatInfo(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: Max17205刷新数据到battery info中
//注    意	:
//=============================================================================================
void Max17205RefreshDataToBatInfo(void)
{
   #ifdef BMS_USE_SOX_MODEL
    
   gBatteryInfo.Data.MAX_SOC = gMax17205Var.RepSOC;
   gBatteryInfo.Data.MAX_SOH = gMax17205Var.SOH;
   gBatteryInfo.Data.MAX_TolCap = gMax17205Var.FullChgCap;
   gBatteryInfo.Data.MAX_RemainCap = gMax17205Var.RepRemainCap;
   //gBatteryInfo.Data.RemainPower = gMax17205Var.RemainPower;   
   gBatteryInfo.Data.MAX_ChgDchgTimes = gMax17205Var.CycleCount;    

   #else
   gBatteryInfo.Data.SOC = gMax17205Var.RepSOC;
   gBatteryInfo.Data.SOH = gMax17205Var.SOH;
   gBatteryInfo.Data.TolCap = gMax17205Var.FullChgCap;
   gBatteryInfo.Data.RemainCap = gMax17205Var.RepRemainCap;
   //gBatteryInfo.Data.RemainPower = gMax17205Var.RemainPower;   
   gBatteryInfo.Data.ChgDchgTimes = gMax17205Var.CycleCount;
   //gBatteryInfo.Data.FullChgTime = gMax17205Var.FullChgTime;
   #endif
    
   gBatteryInfo.Addit.FuelAvgCurr = gMax17205Var.Averagecurrent;
   gBatteryInfo.Addit.FuelTemp = gMax17205Var.Temperature;
   gBatteryInfo.Addit.FuelCellVolt = gMax17205Var.CellVolt;
   gBatteryInfo.Addit.FuelStaFlag = gMax17205Var.Flags;
}

//=============================================================================================
//函数名称	: u8 Max17205GitBit1(u8 byte)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 获取一个字节中，1的个数
//注    意	:
//=============================================================================================
u8 Max17205GitBit1(u8 byte)
{
    u8 count = 0;
    
    for(u8 i = 0;i < 8;i++)
    {
        if(1 == BITGET(byte,i))
        {
            count++;
        }
    }
    return count;
}

//=============================================================================================
//函数名称	: s16 Max17205DataAvgFilter(u16 *data, u16 len)
//输入参数	: data:滤波数据串 len:滤波数据串长度
//输出参数	: 滤波后的数据值
//函数功能	: 数据平均值滤波
//注意事项	: 除最大最小值
//=============================================================================================
s16 Max17205DataAvgFilter(s16 *data, u16 len)
{
	u8 i = 0;
	s16 max = 0;
	s16 min = 0;
	s32 sum = 0;
	s16 result = 0;

	if(0 == len)                //长度错误
	{
	    result = 0xffff;
	}
	else if(len < 3)            //只有1个或者2个数
	{
		sum = data[0] + data[len - 1];
		result = (u16)(sum / 2);
	}
	else                        //不少于3个数
	{
    	max = data[0];
    	min = data[0];
    	sum = data[0];

    	for(i = 1; i < len; i++)
    	{
    		if(data[i] > max)   //求最大值
    		{
    			max = data[i];
    		}

    		if(data[i] < min)	//求最小值
    		{
    			min = data[i];
    		}

    		sum += data[i];	    //求和
    	}

    	result = (u16)((sum - max - min) / (len - 2));
   }

	return(result);
}

//=============================================================================================
//函数名称	: void Max17205FaultTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: MAX17205故障任务函数
//注    意	:
//=============================================================================================
void Max17205InitTask(void)
{
	static u8 i = 0;
	static u8 dataconfigerr = 0;
    static u16 NVMerr = 0;
    static u8 configfail = 0;

    static u16 HibCfg = 0;
    static u8 index = 0;
    u16 readtemp = 0;
    s32 minustemp = 0;      
    
    PT_BEGIN(&gPTMax17205Init);
    
    //读MAX17205 Shadow Ram里的数据
	for(i = 0; i < DFFS_NUM; i++)					
	{

		gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, c_DFFS_pCMD[i][0]);
        PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }   
        else
        {        
            gDFFSRead[i] = (((u16)(gMAX172XXReadbuff[1]) << 8 |(uint16_t) gMAX172XXReadbuff[0]));       
        }    
    }    
    
    if(False == gMax17205Var.IICErr)
    {
        //检查固件版本号是否一致   或者  是否需要写入新的循环次数
        if(DFFS_CFG_VER != gDFFSRead[DFFS_VER_TAB_INDEX] || (gMax17205Var.Ctrl & MAX1720X_WRITE_CYCLES))
        {
            if(gMax17205Var.Ctrl & MAX1720X_WRITE_CYCLES)
            {
                Max17205ClearCtrl(MAX1720X_WRITE_CYCLES);
                
                //单写入循环次数
                gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL, c_DFFS_pCMD[DFFS_CYCLE_INDEX][0],gMax17205Cal.wrtcycle);         
                PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
                if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                {
                    gMax17205Var.IICErr ++;
                }         

                //写入无异常，则开始回读检验
                if(False == gMax17205Var.IICErr)
                {
                    //回读写入的循环次数
                    gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, c_DFFS_pCMD[DFFS_CYCLE_INDEX][0]);
                    PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
                    if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                    {
                        gMax17205Var.IICErr ++;
                    }   
                    else
                    {        
                        gDFFSRead[DFFS_CYCLE_INDEX] = (((u16)(gMAX172XXReadbuff[1]) << 8 |(uint16_t) gMAX172XXReadbuff[0])); 
                        if(gMax17205Cal.wrtcycle != gDFFSRead[DFFS_CYCLE_INDEX])
                        {
                            dataconfigerr++;
                        }
                    }                        
                }  
                else
                {
                    dataconfigerr++;
                }
            }   
            else
            {
                //写入电量计模型所有数据
                for(i = 0; i < DFFS_NUM; i++)					
                {
                    //循环次数不为0
                    if((DFFS_CYCLE_INDEX == i) && (0 != gDFFSRead[DFFS_CYCLE_INDEX]))
                    {
                        //写入当前循环次数
                        gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL, c_DFFS_pCMD[i][0],gDFFSRead[DFFS_CYCLE_INDEX]);
                    }
                    else
                    {
                        gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL, c_DFFS_pCMD[i][0],c_DFFS_pCMD[i][1]);
                    }
                    
                    PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
                    if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                    {
                        gMax17205Var.IICErr ++;
                    }   
                }  

                //写入无异常，则开始回读检验
                if(False == gMax17205Var.IICErr)
                {
                    //回读
                    for(i = 0; i < DFFS_NUM; i++)
                    {

                        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, c_DFFS_pCMD[i][0]);
                        PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
                        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                        {
                            gMax17205Var.IICErr ++;
                        }   
                        else
                        {        
                            gDFFSRead[i] = (((u16)(gMAX172XXReadbuff[1]) << 8 |(uint16_t) gMAX172XXReadbuff[0]));       
                        }    
                    }   
                
                    //回读无异常，则开始检测写入的参数是否已经配置成功
                    if(False == gMax17205Var.IICErr)
                    {
                    
                        //检查参数配置是否正确
                        for(i = 0; i < DFFS_NUM; i++)					
                        {
                            if((DFFS_CYCLE_INDEX == i) && (0 != gDFFSRead[DFFS_CYCLE_INDEX]))
                            {
                                ;   //跳过不检测
                            }
                            else if(gDFFSRead[i] != c_DFFS_pCMD[i][1])
                            {
                                dataconfigerr++;
                            }
                        }
                    }
                    else
                    {
                        dataconfigerr++;
                    }
                }            
                else
                {
                    dataconfigerr++;
                }                
            } 
            
            /* 获取剩余的更新次数 */
            gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_COMMAND_ADDR,0xE2FA);	    //发送查询更新次数命令
            PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
            {
                gMax17205Var.IICErr ++;
            }   
                
            //延时10ms
            gMax17205Var.Timer = gTimer1ms;
            PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 10));        
                
            gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_NUM_OF_UPDATES);
            PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
            {
                gMax17205Var.IICErr ++;
            }   
            else
            {        
                gMax17205Var.NumOfDates =  8 - Max17205GitBit1(gMAX172XXReadbuff[1]| gMAX172XXReadbuff[0]);       
            }

            //配置无异常/可以配置
            if((0 == dataconfigerr) && (gMax17205Var.NumOfDates > gMax17205Var.AtLesRem))
            {
                /* 更新到memory中 */
                
                //第一步：检查是否有NVError
                gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_COMMAND_STAT_ADDR);
                PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
            
                if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                {
                    gMax17205Var.IICErr ++;
                }   
                else
                {        
                    NVMerr =  (((u16)(gMAX172XXReadbuff[1]) << 8 |(uint16_t) gMAX172XXReadbuff[0]));;       
                }
                
                //NVError正常
                if(False == BITGET16(NVMerr,2))
                {
                    //第二步：发送copy命令，开始更新
                    gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_COMMAND_ADDR,0xE904);	            //发送更新memory命令
                    PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
                    if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                    {
                        gMax17205Var.IICErr ++;
                    }        
                    
                    //延时1S
                    gMax17205Var.Timer = gTimer1ms;
                    PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 1000));        
                    
                    //检查是否有NVError
                    gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_COMMAND_STAT_ADDR);
                    PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
                    if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                    {
                        gMax17205Var.IICErr ++;
                    }   
                    else
                    {        
                        NVMerr =  (((u16)(gMAX172XXReadbuff[1]) <<8 |(uint16_t) gMAX172XXReadbuff[0]));;       
                    }     

                    if(True == BITGET16(NVMerr,2))
                    {
                        dataconfigerr++;
                    }
                    ////第三步：开始复位电量计
                    else
                    {
                        //复位IC硬件
                        gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_COMMAND_ADDR,0x000f);	
                        PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
                        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                        {
                            gMax17205Var.IICErr ++;
                        }                   
                        
                        //延时10ms
                        gMax17205Var.Timer = gTimer1ms;
                        PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 10));        

                        //复位电量计操作
                        gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_CONFIG2_ADDR,0x0001);	
                        PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
                        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                        {
                            gMax17205Var.IICErr ++;
                        }  
                        
                        //延时10ms
                        gMax17205Var.Timer = gTimer1ms;
                        PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 10));              
                    }

                    ////校正空闲电流
                    if(False == gMax17205Var.IICErr)
                    {
                        gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_FILTERCFG_ADDR,0x0EA0);	
                        PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
                        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                        {
                            gMax17205Var.IICErr ++;
                        }
                        
                        //读出当前的休眠状态
                        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_HIBCFG_ADDR);
                        PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
                        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                        {
                            gMax17205Var.IICErr ++;
                        }   
                        else
                        {        
                            HibCfg =  (((u16)(gMAX172XXReadbuff[1]) <<8 |(uint16_t) gMAX172XXReadbuff[0]));;       
                        }   
                        
                        //设置当前状态为ACTIVE状态，加快平均电流采样速度
                        gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_HIBCFG_ADDR,0x0000);	
                        PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
                        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                        {
                            gMax17205Var.IICErr ++;
                        }
                        
                        for(index = 0;index < MAX1720X_CURR_CALIB_CNT;index++)
                        {
                            /*  获取电池包平均电流  */
                            gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_AVGCURENT_ADDR);
                            PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
                            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                            {
                                gMax17205Var.IICErr ++;
                            }
                            else
                            {
                                readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) <<8|(uint16_t) gMAX172XXReadbuff[0]));

                                if(readtemp > 0x7fff)
                                {
                                    minustemp = (s16)(readtemp - 65536);     //转化为负数    s16
                                }
                                else
                                {
                                    minustemp = readtemp;
                                }

                                gAvgCurTab[index] = (s16)(((s32)minustemp * 15625) / ((s16)gMax17205Cal.rsense * 100)); 		 //1mA
                                gMax17205Var.IICErr = 0;
                                index++;
                            }
                            
                            //延时1000ms
                            gMax17205Var.Timer = gTimer1ms;
                            PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 1500));              
                        }
                        
                        index = 0;
                        //零飘值写入FLASH中保存
                        gConfigBuff.calpara.COff = 0 - Max17205DataAvgFilter(gAvgCurTab,MAX1720X_CURR_CALIB_CNT);
                        gStorageCTRL |= FLASH_WR_CFG_MASK;
                        
                        gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_COFF_ADDR,gConfigBuff.calpara.COff);	
                        PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
                        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                        {
                            gMax17205Var.IICErr ++;
                        } 
                        
                        //休眠模式恢复
                        gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_HIBCFG_ADDR,HibCfg);	
                        PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
                        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                        {
                            gMax17205Var.IICErr ++;
                        }                                
                        
                    }
                    
                }
                else
                {
                    dataconfigerr++;
                }
            }            
            
        }
        //关机上电后，需要重新发一次复位命令
        else if(gMax17205Var.ShdnFlag == True)
        {
            gMax17205Var.ShdnFlag = False;
            //复位IC硬件
            gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_COMMAND_ADDR,0x000f);	
            PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
            {
                gMax17205Var.IICErr ++;
            }                   
            
            //延时10ms
            gMax17205Var.Timer = gTimer1ms;
            PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 10));        

            //复位电量计操作
            gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_CONFIG2_ADDR,0x0001);	
            PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
            {
                gMax17205Var.IICErr ++;
            }  
            
            //延时10ms，重新打开 20/5/15
            gMax17205Var.Timer = gTimer1ms;
            PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 1000));              
            
            gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_FILTERCFG_ADDR,0x0EA0);	
            PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
            {
                gMax17205Var.IICErr ++;
            }              
        }
        
        /* 写入电量计电流采样零飘值和电量计电流采样斜率值 */
        gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_COFF_ADDR,gConfig.calpara.COff);	
        PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }  

        gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_CGAIN_ADDR,gConfig.calpara.CGain);	
        PT_WAIT_UNTIL(&gPTMax17205Init,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }          
        
    }
    
    

	if(dataconfigerr > 0 || gMax17205Var.IICErr > 0)
	{
        BITSET(gMax17205Var.InitErr,0);	//发生配置错误
        gMax17205Var.RunStat = eFuel_Sta_Offline;
        
        //失败3次不尝试了
        if(configfail >= 3)
        {
            configfail = 0;
            gMax17205Var.RunStat = eFuel_Sta_Idle;            
        }
        else
        {
            configfail++;
        }
	}
	else
	{
        gMax17205Var.DfVer = gDFFSRead[DFFS_VER_TAB_INDEX]; //获取电量计数据版本号
		BITCLR(gMax17205Var.InitErr,0);	//配置正常
		gMax17205Var.RunStat = eFuel_Sta_Norm;
	}
    
	BSPTaskStart(TASK_ID_MAX17205_TASK, 10);	//空闲时10ms周期
    
    PT_INIT(&gPTMax17205Init);    
    dataconfigerr = 0;
    NVMerr = 0;
    i = 0;
    
    PT_END(&gPTMax17205Init);
}

//=============================================================================================
//函数名称	: void Max17205NormalTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: MAX17205正常任务函数
//注    意	:
//=============================================================================================
void Max17205NormalTask(void)
{
	u16 readtemp = 0;
	s32 minustemp = 0;
	static u8 filtcnt = 0; //滤波计数器
    
//	static u16 tmprsense = 0;
//    static u16 readrsense = 0;
//    static u16 NVMerr = 0;
    
    static u8 index = 0;
    static u16 HibCfg = 0;
    static u16 CGainTemp = 0;
    
	PT_BEGIN(&gPTMax17205Norm); 
    
	//判断采集开始标志
	if(gMax17205Var.Ctrl & MAX1720X_START_SAMPLE)
	{
		/*	获取SOC	*/
		gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_REPSOC_ADDR);
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {  
            gMax17205Var.IICErr ++;
        }
        else
        {
            readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) <<8|(uint16_t) gMAX172XXReadbuff[0]));
			gMax17205Var.RepSOC = (u16)(((float)readtemp / 256) * 10);  	  						//0.1%
            gMax17205Var.IICErr = 0;
        }
                
	    /*	获取SOH	*/
        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_REPSOH_ADDR);
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }
        else
        {
            readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) <<8|(uint16_t) gMAX172XXReadbuff[0]));
			gMax17205Var.SOH = (u16)(((float)readtemp / 256) * 10);  								//0.1%
            
            if(gMax17205Var.SOH > 1000)
            {
                gMax17205Var.SOH = 1000;
            }
            
            gMax17205Var.IICErr = 0;
        }

        /*  获取满充容量  */
        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_REPFULLCAP_ADDR);
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }
        else
        {
            readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) <<8|(uint16_t) gMAX172XXReadbuff[0]));
			gMax17205Var.FullChgCap = (u16)((u32)readtemp * 500 / gMax17205Cal.rsense);								//1mAH
            gMax17205Var.IICErr = 0;
        }

        /*  获取剩余容量  */
        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_REPCAP_ADDR);
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }
        else
        {
            readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) <<8|(uint16_t) gMAX172XXReadbuff[0]));
			gMax17205Var.RepRemainCap = (u16)((u32)readtemp * 500 / gMax17205Cal.rsense);						    //1mAH
            gMax17205Var.IICErr = 0;
        }

         /*  获取满充时长	 */
//        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_TTF_ADDR);
//        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
//        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
//        {
//            gMax17205Var.IICErr ++;
//        }
//        else
//        {
//            readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) <<8|(uint16_t) gMAX172XXReadbuff[0]));
//			gMax17205Var.FullChgTime = (u16)(((float)readtemp) * 5.625 / 60);			//1min
//            gMax17205Var.IICErr = 0;
//        }

         /*  获取放空时长	 */
        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_TTE_ADDR);
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }
        else
        {
            readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) << 8|(uint16_t) gMAX172XXReadbuff[0]));
			gMax17205Var.EmptyDchgTime = (u16)(((float)readtemp) * 5.625 / 60);			//1min
            gMax17205Var.IICErr = 0;
        }

       /*  获取电池包平均电流  */
        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_AVGCURENT_ADDR);
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }
        else
        {
            readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) <<8|(uint16_t) gMAX172XXReadbuff[0]));

            if(readtemp > 0x7fff)
            {
                minustemp = (s16)(readtemp - 65536);     //转化为负数    s16
            }
			else
			{
				minustemp = readtemp;
			}

			gMax17205Var.Averagecurrent = (s16)(((s32)minustemp * 15625) / ((s16)gMax17205Cal.rsense * 100)); 		 //1mA
            gMax17205Var.IICErr = 0;
        }

        /*  获取电池包最大电流  */
        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_MAXMINCURENT_ADDR);
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }
        else
        {
            readtemp = gMAX172XXReadbuff[1];
            if(readtemp > 0x7f)
            {
                minustemp = (s16)(readtemp - 256);     //转化为负数  s8
            }
			else
			{
				minustemp = readtemp;
			}
			gMax17205Var.MaxCurr = ABS(0,(((s32)minustemp * 40000 / (s32)gMax17205Cal.rsense))); 	//1mA   取绝对值  数据类型为u16

            gMax17205Var.IICErr = 0;
        }

        /*  获取充放电循环次数  */
        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_BATCYCLE_ADDR);
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }
        else
        {
            readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) <<8|(uint16_t) gMAX172XXReadbuff[0]));
			gMax17205Var.CycleCount = (u16)((float)readtemp * 0.16); //The Cycles register has a full range of 0 to 10485 cycles with a 16.0% LSb
            gMax17205Var.IICErr = 0;
        }

        /*  获取电池温度 AIN1的电池温度  */
        gMax17205Var.iicres= MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_TEMP1_ADDR);
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }
        else
        {
            readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) <<8|(uint16_t) gMAX172XXReadbuff[0]));
			//gMax17205Var.Temperature = (s16)(((float)readtemp) / 256 * 10);  				  //0.1℃
            gMax17205Var.Temperature = (s16)((((float)readtemp) - 2730));  				  //0.1℃

			if(gMax17205Var.Temperature < -400)
			{
				gMax17205Var.Temperature = MAX1720X_TEMP_OFF;
			}
			else if(gMax17205Var.Temperature > 1200)
			{
				gMax17205Var.Temperature = MAX1720X_TEMP_SHORT;
			}

            gMax17205Var.IICErr = 0;
        }

        /*  获取温度 AIN2的连接器温度  */
        /*
        gMax17205Var.iicres= MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_TEMP2_ADDR);
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }
        else
        {
            readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) <<8|(uint16_t) gMAX172XXReadbuff[0]));
			//gMax17205Var.Temperature = (s16)(((float)readtemp) / 256 * 10);  				  //0.1℃
            gSampleData.PCBTemp = (u16)((((float)readtemp) - 2730));  				  //0.1℃

			if(gSampleData.PCBTemp < -400)
			{
				gSampleData.PCBTemp = MAX1720X_TEMP_OFF;
			}
			else if(gSampleData.PCBTemp > 1200)
			{
				gSampleData.PCBTemp = MAX1720X_TEMP_SHORT;
			}

            gMax17205Var.IICErr = 0;
        }
        */
        
        /*  获取VCELL  */
        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_VCELLX_ADDR);
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }
        else
        {
            readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) <<8|(uint16_t) gMAX172XXReadbuff[0]));
			gMax17205Var.CellVolt = (u16)((float)readtemp * 0.078125);  					//1mV
            gMax17205Var.IICErr = 0;
        }

        /*  获取充满、空闲等标志  */
        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2,MAX1720X_FSTAT_ADDR);
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }
        else
        {
            readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) <<8|(uint16_t) gMAX172XXReadbuff[0]));
			gMax17205Var.Flags = (u16)(readtemp);
            gMax17205Var.IICErr = 0;

        }

        if(gMax17205Var.CellVolt < 2000)	//VCELLX被移除
        {
        	filtcnt++;
        	if(filtcnt > 5)
        	{
        		gMax17205Var.RunStat = eFuel_Sta_Offline;	//进入断线复位处理
        		gMax17205Var.IICErr = 100;	//让电量计故障位置1
        		filtcnt = 0;
        	}
        }
        else
        {
			/*  计算电池包电压  */
			gMax17205Var.Voltage = gMax17205Var.CellVolt * BATTERY_CELLNUM;
			filtcnt = 0;
        }

 		/*  计算剩余电能  */
		/*  0.1Wh = 0.1 mV*mAH/100000  */
		//gMax17205Var.RemainPower = (gMax17205Var.Voltage) * (gMax17205Var.RepRemainCap) / 100000;  //0.1wh

        gMax17205Var.Ctrl &= ~MAX1720X_START_SAMPLE;  //清除采集使能

        DataDealSetBMSDataRdy(e_FuelRdy);
		Max17205RefreshDataToBatInfo(); //更新电量计数据到电池信息中
        
        /* 获取剩余的更新次数 */
        gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_COMMAND_ADDR,0xE2FA);	//发送查询更新次数命令
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }   
        
        gMax17205Var.Timer = gTimer1ms;
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 10));        
            
        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_NUM_OF_UPDATES);
        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
        
        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
        {
            gMax17205Var.IICErr ++;
        }   
        else
        {        
            gMax17205Var.NumOfDates =  8 - Max17205GitBit1(gMAX172XXReadbuff[1]| gMAX172XXReadbuff[0]);       
        }    

	}

	//判断电流校准标志
	if(gMax17205Var.Ctrl & MAX1720X_START_CALIBCURR)
	{
        if(0 != gMax17205Cal.calibcurr)
        {
            gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_FILTERCFG_ADDR,0x0EA0);	
            PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
            {
                gMax17205Var.IICErr ++;
            }
            
            //读出当前的休眠状态
            gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_HIBCFG_ADDR);
            PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
            {
                gMax17205Var.IICErr ++;
            }   
            else
            {        
                HibCfg =  (((u16)(gMAX172XXReadbuff[1]) <<8 |(uint16_t) gMAX172XXReadbuff[0]));;       
            }   
            
            //读当前的电流校准增益
            gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_CGAIN_ADDR);
            PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
            {
                gMax17205Var.IICErr ++;
            }   
            else
            {        
                CGainTemp =  (((u16)(gMAX172XXReadbuff[1]) <<8 |(uint16_t) gMAX172XXReadbuff[0]));;       
            }               
            
            //设置当前状态为ACTIVE状态，加快平均电流采样速度
            gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_HIBCFG_ADDR,0x0000);	
            PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
            {
                gMax17205Var.IICErr ++;
            }
            
            for(index = 0;index < MAX1720X_CURR_CALIB_CNT;index++)
            {
                /*  获取电池包平均电流  */
                gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_AVGCURENT_ADDR);
                PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
                if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
                {
                    gMax17205Var.IICErr ++;
                }
                else
                {
                    readtemp = (((uint16_t)(gMAX172XXReadbuff[1]) <<8|(uint16_t) gMAX172XXReadbuff[0]));

                    if(readtemp > 0x7fff)
                    {
                        minustemp = (s16)(readtemp - 65536);     //转化为负数    s16
                    }
                    else
                    {
                        minustemp = readtemp;
                    }

                    gAvgCurTab[index] = (s16)(((s32)minustemp * 15625) / ((s16)gMax17205Cal.rsense * 100)); 		 //1mA
                    gMax17205Var.IICErr = 0;
                }
                
                //延时1000ms
                gMax17205Var.Timer = gTimer1ms;
                PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 1000));              
            }
            
            index = 0;
            //零飘值写入FLASH中保存
            gConfigBuff.calpara.CGain = (s16)((s32)(CGainTemp * gMax17205Cal.calibcurr) / Max17205DataAvgFilter(gAvgCurTab,MAX1720X_CURR_CALIB_CNT));
            gStorageCTRL |= FLASH_WR_CFG_MASK;
            
            gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_CGAIN_ADDR,gConfigBuff.calpara.CGain);	
            PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
            {
                gMax17205Var.IICErr ++;
            } 
            
            //休眠模式恢复
            gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_HIBCFG_ADDR,HibCfg);	
            PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
            {
                gMax17205Var.IICErr ++;
            }                                
        }

//		if(0 != gMax17205Cal.calibcurr)	//校准电流不为0
//		{
//            
//			tmprsense = ((gMax17205Var.Averagecurrent * (s16)gMax17205Cal.rsense) / gMax17205Cal.calibcurr);

//			if( (50 <= tmprsense) && (200 >= tmprsense))   //校准范围50 - 200
//			{
//				gMax17205Cal.rsense = tmprsense;
//				gConfigBuff.calpara.fuelrense = gMax17205Cal.rsense;
//				gStorageCTRL |= FLASH_WR_CFG_MASK;

//				//写入校准值到寄存器中
//				gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_NRSENSE_ADDR,tmprsense);
//                PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));               
//                if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
//                {
//                    gMax17205Var.IICErr ++;
//                }
//                
//                //写入无异常，则开始回读检验
//                if(False == gMax17205Var.IICErr)
//                {
//                    //回读写入的循环次数
//                    gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_NRSENSE_ADDR);
//                    PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
//                    if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
//                    {
//                        gMax17205Var.IICErr ++;
//                    }   
//                    else
//                    {        
//                        readrsense = (((u16)(gMAX172XXReadbuff[1]) << 8 |(uint16_t) gMAX172XXReadbuff[0])); 
//                        if(tmprsense != readrsense)
//                        {
//                            gMax17205Var.IICErr ++;
//                        }
//                    }                        
//                }

//                //配置无异常/可以配置
//                if((False == gMax17205Var.IICErr) && (gMax17205Var.NumOfDates > gMax17205Var.AtLesRem))
//                {                
//                     /* 更新到memory中 */
//                    
//                    //第一步：检查是否有NVError
//                    gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_COMMAND_STAT_ADDR);
//                    PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
//                
//                    if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
//                    {
//                        gMax17205Var.IICErr ++;
//                    }   
//                    else
//                    {        
//                        NVMerr =  (((u16)(gMAX172XXReadbuff[1]) << 8 |(uint16_t) gMAX172XXReadbuff[0]));;       
//                    }
//                    
//                    //NVError正常
//                    if(False == BITGET16(NVMerr,2))
//                    {
//                        //第二步：发送copy命令，开始更新
//                        gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_COMMAND_ADDR,0xE904);	            //发送更新memory命令
//                        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
//                        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
//                        {
//                            gMax17205Var.IICErr ++;
//                        }        
//                        
//                        //延时1S
//                        gMax17205Var.Timer = gTimer1ms;
//                        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 1000));        
//                        
//                        //检查是否有NVError
//                        gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_COMMAND_STAT_ADDR);
//                        PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
//                        if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
//                        {
//                            gMax17205Var.IICErr ++;
//                        }   
//                        else
//                        {        
//                            NVMerr =  (((u16)(gMAX172XXReadbuff[1]) <<8 |(uint16_t) gMAX172XXReadbuff[0]));;       
//                        }     

//                        if(True == BITGET16(NVMerr,2))
//                        {
//                            gMax17205Var.IICErr ++;
//                        }
//                        ////第三步：开始复位电量计
//                        else
//                        {
//                            //复位IC硬件
//                            gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_COMMAND_ADDR,0x000f);	
//                            PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
//                            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
//                            {
//                                gMax17205Var.IICErr ++;
//                            }                   
//                            
//                            //延时10ms
//                            gMax17205Var.Timer = gTimer1ms;
//                            PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 10));        

//                            //复位电量计操作
//                            gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_CONFIG2_ADDR,0x0001);	
//                            PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
//                            if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
//                            {
//                                gMax17205Var.IICErr ++;
//                            }  
//                            
//                            //延时10ms
//                            gMax17205Var.Timer = gTimer1ms;
//                            PT_WAIT_UNTIL(&gPTMax17205Norm,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 10));              
//                        }   
//                    }
//                    else
//                    {
//                        gMax17205Var.IICErr ++;
//                    }                   
//                    
//                } 
//            }
//		}
		gMax17205Cal.calibcurr = 0;
		gMax17205Var.Ctrl &= ~MAX1720X_START_CALIBCURR;
	}
    
    //写入循环次数
    if(gMax17205Var.Ctrl & MAX1720X_WRITE_CYCLES)
    {
        gMax17205Var.RunStat = eFuel_Sta_Init;
    }
    
    BSPTaskStart(TASK_ID_MAX17205_TASK, 10);	//空闲时10ms周期    
    
    PT_INIT(&gPTMax17205Norm); 
	PT_END(&gPTMax17205Norm); 
}

//=============================================================================================
//函数名称	: void Max17205FaultTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: MAX17205故障任务函数
//注    意	:
//=============================================================================================
void Max17205FaultTask(void)
{
	PT_BEGIN(&gPTMax17205Fault); 
    
    //复位IC硬件
    gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_COMMAND_ADDR,0x000f);	
    PT_WAIT_UNTIL(&gPTMax17205Fault,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
    if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
    {
        gMax17205Var.IICErr ++;
    }
    else
    {
        gMax17205Var.IICErr = 0;    
    }
    
    //延时10ms
    gMax17205Var.Timer = gTimer1ms;
    PT_WAIT_UNTIL(&gPTMax17205Fault,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 10));        

    //复位电量计操作
    gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_CONFIG2_ADDR,0x0001);	
    PT_WAIT_UNTIL(&gPTMax17205Fault,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));
    if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
    {
        gMax17205Var.IICErr ++;
    }  
    else
    {
        gMax17205Var.IICErr = 0;    
    }    
    //延时10ms
    gMax17205Var.Timer = gTimer1ms;
    PT_WAIT_UNTIL(&gPTMax17205Fault,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 10));  
    //
    if((gSampleData.MCU3v3 > 3000) && (MAX1720X_IIC_NOFAULT_STAT == gMax17205Var.IICErr))
    {
        //恢复正常了
        gMax17205Var.RunStat = eFuel_Sta_Init;
    }
    else
    {
        //延时5000ms
        //继续等待5S，重新复位
        gMax17205Var.Timer = gTimer1ms;
        PT_WAIT_UNTIL(&gPTMax17205Fault,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (gTimer1ms - gMax17205Var.Timer >= 5000));              
    }
    
    BSPTaskStart(TASK_ID_MAX17205_TASK, 10);	//空闲时10ms周期    
	PT_INIT(&gPTMax17205Fault);     
	PT_END(&gPTMax17205Fault); 
}

//=============================================================================================
//函数名称	: void Max17205ShutDownTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: MAX17205关机任务函数
//注    意	:
//=============================================================================================
void Max17205ShutDownTask(void)
{
	u16 configval = 0;
    
	PT_BEGIN(&gPTMax17205ShutDown);
    
	/*	获取配置寄存器的值	*/
    gMax17205Var.iicres = MAX17205ReadRegister(MAX1720X_IIC_CHANNEL, gMAX172XXReadbuff, 2, MAX1720X_CONFIG_ADDR);
    PT_WAIT_UNTIL(&gPTMax17205ShutDown,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
    if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
    {
        gMax17205Var.IICErr ++;
    }  
    else
    {
    	configval = (((uint16_t)(gMAX172XXReadbuff[1]) << 8 | (uint16_t) gMAX172XXReadbuff[0]));
        gMax17205Var.IICErr = 0;
    }

    gMax17205Var.iicres = MAX17205WriteRegister(MAX1720X_IIC_CHANNEL,MAX1720X_CONFIG_ADDR,configval | 0x0080);	//发送关机命令
    PT_WAIT_UNTIL(&gPTMax17205ShutDown,(BSPTaskStart(TASK_ID_MAX17205_TASK, 2)) && (BSPIICChannalStateGet(gMax17205Var.socchn) <= 0));        
    if((MAX1720X_IIC_NOFAULT_STAT != gMax17205Var.iicres) || (BSPIICChannalStateGet(gMax17205Var.socchn) < 0))
    {
        gMax17205Var.IICErr ++;
    }  
    else
    {
        gMax17205Var.IICErr = 0;
        gMax17205Var.RunStat = eFuel_Sta_Idle;
    }
    
    BSPTaskStart(TASK_ID_MAX17205_TASK, 10);	//空闲时10ms周期      
    PT_INIT(&gPTMax17205ShutDown);
	PT_END(&gPTMax17205ShutDown); 
}

//=============================================================================================
//函数名称	: void Max17205MainTask(void *p)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: MAX17205主任务函数
//注    意	:
//=============================================================================================
void Max17205MainTask(void *p)
{
	(void)p;
	
	PT_BEGIN(&gPTMax17205);   

	for(;;)
    {
		switch(gMax17205Var.RunStat)
		{
			case eFuel_Sta_Init:
				Max17205InitTask();
				break;

			case eFuel_Sta_Norm:
				Max17205NormalTask();
				break;

			case eFuel_Sta_Offline:
				Max17205FaultTask();
				break;

			case eFuel_Sta_Disable:
				Max17205ShutDownTask();
				break;
				
			case eFuel_Sta_Idle:	//空闲任务
				;
				break;

			default:
				;
				break;
		}

		PT_NEXT(&gPTMax17205);
	}
    PT_END(&gPTMax17205);

}

/*****************************************end of MAX17205.c*****************************************/
