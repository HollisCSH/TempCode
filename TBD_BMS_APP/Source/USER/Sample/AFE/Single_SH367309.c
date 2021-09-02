//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SH367309.c
//创建人  	: Handry
//创建日期	: 
//描述	    : SH367309驱动源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "Single_SH367309.h"
#include "BSPTimer.h"
#include "MOSCtrl.h"
#include "Storage.h"
#include "Sample.h"
#include "CommCtrl.h"
//#include "BSPSysTick.h"
#include "ParaCfg.h"
#include "NTC.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "DataDeal.h"
#include "pt.h"
#include "DTCheck.h"

#ifdef DEBUG
#include "BSP_UART.h"
#endif

//=============================================================================================
//全局变量定义
//=============================================================================================
u8	gSH367309Readbuff[SH367309_READBUFF_LEN] = {0};     //SH367309接收缓冲数组
u8	gSH367309Writebuff[SH367309_WRITEBUFF_LEN] = {0};  	//SH367309发送缓冲数组
u8  gSHAFEWritebuff[10] = {0};

t_SH_AFE_Data_Status gSHAFEData;			            //SH367309采样数据/状态相关结构体
t_SH_AFE_Ctrl gSHAFECtrl;                               //SH367309控制相关结构体
t_SH_AFE_CfgData gSHAFECfg;                             //SH367309配置数据相关结构体
t_SH367309_EEPRom gSHAfeEep;                            //SH367309配置EEP结构体
t_SH367309_EEPRom gSHAfeEepRead;                        //SH367309配置EEP结构体
t_SH367309_Register gSHAFEReg;                          //SH367309寄存器结构体

pt gPTSH367309;					                        //SH367309任务pt线程控制变量
pt gPTSH367309Norm;					                    //SH367309任务正常任务pt线程控制变量

t_AFE_Test g_AFE_Test_VAR = {0,250};

//120424-1			CRC Table
const u8 CRC8Table[] =
{							
    0x00,0x07,0x0E,0x09,0x1C,0x1B,0x12,0x15,0x38,0x3F,0x36,0x31,0x24,0x23,0x2A,0x2D,
    0x70,0x77,0x7E,0x79,0x6C,0x6B,0x62,0x65,0x48,0x4F,0x46,0x41,0x54,0x53,0x5A,0x5D,
    0xE0,0xE7,0xEE,0xE9,0xFC,0xFB,0xF2,0xF5,0xD8,0xDF,0xD6,0xD1,0xC4,0xC3,0xCA,0xCD,
    0x90,0x97,0x9E,0x99,0x8C,0x8B,0x82,0x85,0xA8,0xAF,0xA6,0xA1,0xB4,0xB3,0xBA,0xBD,
    0xC7,0xC0,0xC9,0xCE,0xDB,0xDC,0xD5,0xD2,0xFF,0xF8,0xF1,0xF6,0xE3,0xE4,0xED,0xEA,
    0xB7,0xB0,0xB9,0xBE,0xAB,0xAC,0xA5,0xA2,0x8F,0x88,0x81,0x86,0x93,0x94,0x9D,0x9A,
    0x27,0x20,0x29,0x2E,0x3B,0x3C,0x35,0x32,0x1F,0x18,0x11,0x16,0x03,0x04,0x0D,0x0A,
    0x57,0x50,0x59,0x5E,0x4B,0x4C,0x45,0x42,0x6F,0x68,0x61,0x66,0x73,0x74,0x7D,0x7A,
    0x89,0x8E,0x87,0x80,0x95,0x92,0x9B,0x9C,0xB1,0xB6,0xBF,0xB8,0xAD,0xAA,0xA3,0xA4,
    0xF9,0xFE,0xF7,0xF0,0xE5,0xE2,0xEB,0xEC,0xC1,0xC6,0xCF,0xC8,0xDD,0xDA,0xD3,0xD4,
    0x69,0x6E,0x67,0x60,0x75,0x72,0x7B,0x7C,0x51,0x56,0x5F,0x58,0x4D,0x4A,0x43,0x44,
    0x19,0x1E,0x17,0x10,0x05,0x02,0x0B,0x0C,0x21,0x26,0x2F,0x28,0x3D,0x3A,0x33,0x34,
    0x4E,0x49,0x40,0x47,0x52,0x55,0x5C,0x5B,0x76,0x71,0x78,0x7F,0x6A,0x6D,0x64,0x63,
    0x3E,0x39,0x30,0x37,0x22,0x25,0x2C,0x2B,0x06,0x01,0x08,0x0F,0x1A,0x1D,0x14,0x13,
    0xAE,0xA9,0xA0,0xA7,0xB2,0xB5,0xBC,0xBB,0x96,0x91,0x98,0x9F,0x8A,0x8D,0x84,0x83,
    0xDE,0xD9,0xD0,0xD7,0xC2,0xC5,0xCC,0xCB,0xE6,0xE1,0xE8,0xEF,0xFA,0xFD,0xF4,0xF3
};

//放电电流1保护值配置表
const u32 cSH367309OCD1Tab[16] =
{
	//单位  10mA
    2000000,3000000,4000000,5000000,6000000,7000000,8000000,9000000,
    10000000,11000000,12000000,13000000,14000000,16000000,18000000,20000000,
};

//放电电流2保护值配置表
const u32 cSH367309OCD2Tab[16] =
{
	//单位  10mA
    3000000,4000000,5000000,6000000,7000000,8000000,9000000,10000000,
    12000000,14000000,16000000,18000000,20000000,30000000,40000000,50000000,
};

//短路保护值配置表
const u32 cSH367309SCTab[16] =
{
	//单位  10mA
    5000000,8000000,11000000,14000000,17000000,20000000,23000000,26000000,
    29000000,32000000,35000000,40000000,50000000,60000000,80000000,100000000,
};

//过流保护值配置表
const u32 cSH367309OCCTab[16] =
{
	//单位  10mA
    2000000,3000000,4000000,5000000,6000000,7000000,8000000,9000000,
    10000000,11000000,12000000,13000000,14000000,16000000,18000000,20000000,
};

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================
//函数名称: void SH367309Init(void)
//输入参数: void
//输出参数: void
//功能描述: SH367309上电初始化
//注意事项:
//=============================================================================
void SH367309Init(void)
{
	PT_INIT(&gPTSH367309);
	PT_INIT(&gPTSH367309Norm);    
    gSHAFEData.runstatus = e_SH_AFE_Sta_Init;
//    gSHAFEData.afechn = BSPIICChannalReq(IIC_ROUTE_AFE); ----------------
    gSHAFECfg.adcgain = 0;    //SH367309的AD增益
    gSHAFECfg.adcoffset = 0;  //SH367309的AD偏移
    
	//获取校准增益和零飘值
	gSHAFECfg.calres = gConfig.calpara.sampleres;
    gSHAFECfg.Cadcoffset = gConfig.calpara.sampleoff;
    
	gSHAFEData.runstatus = e_SH_AFE_Sta_Bqinit;
}


//=============================================================================================
//函数名称	: static void SH367309ClearData(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 清除采样的电压、温度、电流数据
//注    意	:
//=============================================================================================
static void SH367309ClearData(void)
{
    u8 i;
    /* 发生错误，不清除AFE数据，避免欠压 */
    for(i = 0;i < BAT_CELL_NUM;i++)
    {
    	//gBatteryInfo.Data.CellVolt[i] = 0;
    }
    for(i = 0;i < BAT_TEMP_NUM;i++)
    {
    	//gBatteryInfo.Data.CellTemp[i] = NTC_TEMP_OFF;
    }
    gBatteryInfo.Data.Curr = 0;
}

//=============================================================================================
//函数名称	: u8 SH367309CalcCRC(u8 *datadataptr, u8 len)
//函数参数	: dataptr需要计算CRC的数组指针, len：数组长度
//输出参数	:
//静态变量	:
//功    能	: 计算数据的CRC
//注    意	:
//=============================================================================================
u8 SH367309CalcCRC(u8 *dataptr, u8 len)
{
    u8 crc8 = 0;    
    
	for(; len > 0; len--)
	{    
        crc8 = CRC8Table[crc8^*dataptr];    
	    dataptr++;    
    } 
       
    return(crc8);       
}

//=============================================================================================
//函数名称	: u8 SH367309WriteRegister(e_IICNUM num, u8 regaddr,u8* p_buffer)
//输入参数	: num:eIIC0/eIICMax	regaddr:SH367309寄存器地址      *p_buffer:发送缓冲区		bufflen:发送的数据个数
//输出参数	: void
//静态变量	: void
//功    能	: SH367309写1个寄存器函数,阻塞
//注    意	:
//=============================================================================================
u8 SH367309WriteRegister(I2C_TypeDef *hi2cx, u8 regaddr,u8* p_buffer)
{
	u8 errstat = 0;									//IIC通信错误信息
	u8 regaddress = 0;
	u8 deviceaddr = 0;

	if((hi2cx != SH367309_IIC_CHANNEL))  //参数有效性判断
	{
		return SH367309_IIC_FAULT_STAT;
	}

	deviceaddr = SH367309_WRITE_ADDR;
	regaddress = (u8)(regaddr);

	gSH367309Writebuff[0] = deviceaddr;
	gSH367309Writebuff[1] = regaddress;
	gSH367309Writebuff[2] = *p_buffer;
	gSH367309Writebuff[3] = SH367309CalcCRC(gSH367309Writebuff, 3);

	p_buffer++;	//指向下一个字节

	if(SH367309_IIC_NOFAULT_STAT == errstat)
	{     
        //errstat = BSPIICSendMsg(num,gSH367309Writebuff,bufflen * 2 + 3,0);        SH367309_IIC_CHANNEL
        //BSPIICWrite(SH367309_IIC_CHANNEL,gSH367309Writebuff[0],&gSH367309Writebuff[1], 3); 
				//BSPIICWrite(SH367309_IIC_CHANNEL,gSH367309Writebuff[0],&gSH367309Writebuff[1], 3); 
        BSP_I2C_MEM_WRITE(SH367309_IIC_CHANNEL,gSH367309Writebuff[1],&gSH367309Writebuff[2], 2); 
	}

	 return errstat;    
}

//=============================================================================================
//函数名称	: u8 SH367309ReadNRegisters(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr)
//输入参数	: num:eIIC0/eIICMax	*p_buffer:接收缓冲区		bufflen:接收的数据个数	regaddr:SH367309寄存器地址
//输出参数	: IIC通信错误信息
//静态变量	: void
//功	能	: SH367309读取信息函数.阻塞等待
//注	意	:
//=============================================================================================
u8 SH367309ReadNRegisters(I2C_TypeDef *hi2cx , u8* p_buffer, u8 bufflen, u8 regaddr)
{
	u8 errstat = 0;			//IIC通信错误信息
	u16 regaddress = 0;	//寄存器地址

	if((hi2cx != SH367309_IIC_CHANNEL) || (p_buffer == NULL))  //参数有效性判断
	{
		return SH367309_IIC_FAULT_STAT;
	}

	regaddress = (u8)regaddr;
    
    gSH367309Writebuff[0] = regaddress;
    gSH367309Writebuff[1] = bufflen;    
    regaddress = (((u16)gSH367309Writebuff[0])<<8) | ((u16)gSH367309Writebuff[1]);
		//errstat = BSPIICRcvMsg(num,SH367309_WRITE_ADDR,regaddress,p_buffer,bufflen * 2,0);	//发送读取报文，标准地址
    //BSPIICWriteAndRead(gSHAFEData.afechn,SH367309_WRITE_ADDR,&gSH367309Writebuff[0],2,p_buffer,bufflen+1); 
    BSPIICWriteAndRead(SH367309_IIC_CHANNEL,SH367309_WRITE_ADDR,&gSH367309Writebuff[0],2,p_buffer,bufflen); 
		//BSP_I2C_MEM_REAR(SH367309_IIC_CHANNEL,regaddress,p_buffer,bufflen+1); 
    
	return errstat;
}

//=============================================================================================
//函数名称	: void SH367309RefreshDataToBatInfo(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SH367309刷新数据到battery info中
//注    意	:
//=============================================================================================
void SH367309RefreshDataToBatInfo(void)
{
	u8 i = 0,j = 0;

	//更新单体电压到BATTERY INFO
	for(i = 0;i < SH367309_VOLT_NUM;i++)
	{
		if(1 == BITGET32(SH367309_VALID_BAT,i))		//获取有效的标志位
		{
			gBatteryInfo.Data.CellVolt[j] = gSHAFEData.cellvolt[i];
			j++;
		}
	}

    //连接器温度使用AFE采样
    gSampleData.ConnTemp = gSHAFEData.celltemp[2];
	
	//更新单体温度到BATTERY INFO
	//增加温度修改调试功能 210629
    if(True == g_AFE_Test_VAR.IsModTemp)
		gBatteryInfo.Data.CellTemp[0] = g_AFE_Test_VAR.ModTempVal;
    else
        gBatteryInfo.Data.CellTemp[0] = gSHAFEData.celltemp[0];
    //gBatteryInfo.Data.CellTemp[0] = gSHAFEData.celltemp[0];
	gBatteryInfo.Data.CellTemp[1] = gSHAFEData.celltemp[1];
    
//    if((gBatteryInfo.Data.CellTemp[0] < -400) || (gBatteryInfo.Data.CellTemp[0] > 1200))    //不需要判断，采样检测计算已判断
//    {
//        gBatteryInfo.Data.CellTemp[0] = NTC_TEMP_OFF;
//    }
//    if((gBatteryInfo.Data.CellTemp[1] < -400) || (gBatteryInfo.Data.CellTemp[1] > 1200))
//    {
//        gBatteryInfo.Data.CellTemp[1] = NTC_TEMP_OFF;
//    }    

	//更新电流采样到BATTERY INFO
    if(gSHAFEData.curr > 0x7fffffff)
    {
        gBatteryInfo.Data.Curr = (s16)((gSHAFEData.curr - 0xffffffff) / 10);
    }
	else
    {
        gBatteryInfo.Data.Curr = (s16)((gSHAFEData.curr) / 10);    
    }

    gBatteryInfo.Status.MOSStatus = gSHAFEData.mosstatus;
}

//=============================================================================================
//函数名称	: u16 SH367309EventFlagGet(void)
//输入参数	: void
//输出参数	: SH367309任务标志位
//静态变量	: void
//功    能	: 获取SH367309任务标志位
//注    意	:
//=============================================================================================
u16 SH367309EventFlagGet(void)
{
	return gSHAFECtrl.eventctrl;
}

//=============================================================================================
//函数名称	: void SH367309EventFlagSet(u16 eventflag)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SH367309主任务:任务标志置位
//注    意	:
//=============================================================================================
void SH367309EventFlagSet(u16 eventflag)
{
	gSHAFECtrl.eventctrl |= eventflag;
}

//=============================================================================================
//函数名称	: void SH367309EventFlagSet(u16 eventflag)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SH367309主任务:任务标志置位
//注    意	:
//=============================================================================================
void SH367309EventFlagClear(u16 eventflag)
{
	gSHAFECtrl.eventctrl &= ~(eventflag);
}

//=============================================================================================
//函数名称	: void SH367309MosfetSet(uint8_t mos)
//输入参数	: mos:mos管控制状态
//输出参数	: void
//静态变量	: void
//功    能	: SH367309设置MOS控制状态
//注    意	:
//=============================================================================================
void SH367309MosfetSet(u8 mos)
{
	gSHAFECtrl.mosctrl = mos & 0x03;
	SH367309EventFlagSet(SH367309_EVE_CTRL_MOS);
    
  //BSPGPIOSetPin(SH367309_CTL_PORT,SH367309_CTL_PIN);
	HAL_GPIO_WritePin(CTL_AFE_PORT, CTL_AFE_PIN, GPIO_PIN_SET);//需确认--------------------------------

}

//=============================================================================================
//函数名称	: u8 SH367309MosfetGet(void)
//输入参数	: void
//输出参数	: MOS控制状态
//静态变量	: void
//功    能	: SH367309获取MOS控制状态
//注    意	:
//=============================================================================================
u8 SH367309MosfetGet(void)
{
    //return (gSH367309Reg.SysCtrl2.SysCtrl2Byte) & 0x03;
    return gSHAFEData.mosstatus & 0x03;
}

//=============================================================================================
//函数名称	: SH367309CalADtoVolt(u8 hibyte, u8 lobyte)
//输入参数	: hibyte:16位AD高字节	lobyte：16位AD低字节
//输出参数	: 转化后的实际电压值
//功能描述	: 电压AD值转化为实际电压值
//注意事项	:
//=============================================================================================
u16 SH367309CalADtoVolt(u8 hibyte, u8 lobyte)
{
    s32 ret;
    ret = (s32)((((u32)((hibyte))) << 8) | (lobyte));
    
    //ret = ret * gSHAFECfg.adcgain / 1000 + gSHAFECfg.adcoffset;
    ret = ret * 5 / 32;
    
    return (u16)ret;
}

//=============================================================================
//函数名称: s32 SH367309CalCADCtoCurr(u8 hibyte, u8 lobyte)
//输入参数: hibyte:16位AD高字节	lobyte：16位AD低字节
//输出参数: 转化后的实际电流值
//功能描述: 电流CADC值转化为实际电流值
//注意事项: 电流单位为1mA
//=============================================================================
s32 SH367309CalCADCtoCurr(u8 hibyte, u8 lobyte)
{
    s32 ret;
    
    ret = (s32)(s16)((((u16)hibyte)<<8)|(lobyte));
    gSHAFEData.curradval = ret;
    ret = (s32)((float)(ret - gSHAFECfg.Cadcoffset) * SH367309_CALICUR) / (21.47 * (float)gSHAFECfg.calres);	//1000 uR
    return ret;
}

//=============================================================================
//函数名称: s32 SH367309CalADtoCurr(u8 hibyte, u8 lobyte)
//输入参数: hibyte:16位AD高字节	lobyte：16位AD低字节
//输出参数: 转化后的实际电流值
//功能描述: 电流AD值转化为实际电流值
//注意事项: 电流单位为1mA
//=============================================================================
s32 SH367309CalADtoCurr(u8 hibyte, u8 lobyte)
{
    s32 ret;
    
    ret = (s32)(s16)((((u16)hibyte)<<8)|(lobyte));
    ret = (s32)((float)(ret - gSHAFECfg.Cadcoffset) * SH367309_CALICUR) / (26.837 * (float)gSHAFECfg.calres);	//1000 uR
    return ret;
}

//=============================================================================
//函数名称: s16 SH367309CalADtoOVTrip(u8 ov)
//输入参数: 读取的寄存器ov trip的值，eepread:t_SH367309_EEPRom指针
//输出参数: 转化后的保护值
//功能描述: 转化AD值为过压触发保护值
//注意事项:
//=============================================================================
s16 SH367309CalADtoOVTrip(t_SH367309_EEPRom* eepread)
{
    u16 tmp = 0;
    
    tmp = (((u16)(eepread->OVH.OVHBit.OV8_9) << 8) | (u16)(eepread->OVL.OVLBit.OV0_7))\
           * SH367309_EEP_OV_GAIN;
    
    return (s16)tmp;
}

//=============================================================================
//函数名称: s16 SH367309CalADtoUVTrip(t_SH367309_EEPRom* eepread)
//输入参数: 读取的寄存器uv trip的值，eepread:t_SH367309_EEPRom指针
//输出参数: 转化后的保护值
//功能描述: 转化AD值为欠压触发保护值
//注意事项:
//=============================================================================
s16 SH367309CalADtoUVTrip(t_SH367309_EEPRom* eepread)
{
    u16 tmp = 0;
    
    tmp = eepread->UV.UVBit.UV0_7 * SH367309_EEP_UV_GAIN;
    
    return (s16)tmp;
}

//=============================================================================
//函数名称: s16 SH367309CalTemptoOTAD(s16 temp)
//输入参数: temp:设置保护的温度
//输出参数: 转化后的保护值
//功能描述: 转化过温触发保护值为AD值
//注意事项:
//=============================================================================
u8 SH367309CalTemptoOTAD(s16 temp)
{
    u8 tmp = 0;
    float Rref = 0;
    float Rt1 = 0;

    Rt1 = NTCFindResByTemp(temp) / 10; //查找阻值
    Rref = 5 * gSHAfeEepRead.TR.TRBit.TR0_6 + SH367309_REF_RES_VAL;
    
    tmp = (u8)(((float)(Rt1 / (Rt1 + Rref))) * 512);
    
    return tmp;
}

//=============================================================================
//函数名称: s16 SH367309CalTemptoUTAD(s16 temp)
//输入参数: temp:设置保护的温度
//输出参数: 转化后的保护值
//功能描述: 转化欠温触发保护值为AD值
//注意事项:
//=============================================================================
u8 SH367309CalTemptoUTAD(s16 temp)
{
    u8 tmp = 0;
    float Rref = 0;
    float Rt1 = 0;

    Rt1 = NTCFindResByTemp(temp) / 10; //查找阻值
    Rref = 5 * gSHAfeEepRead.TR.TRBit.TR0_6 + SH367309_REF_RES_VAL;
    
    tmp = (u8)((((float)(Rt1 / (Rt1 + Rref))) - 0.5) * 512);
    
    return tmp;
}

//=============================================================================
//函数名称: s16 SH367309CalCurrentRes(s32 actcurrent)
//输入参数: 校准电流值
//输出参数: 校准电流对应的采样电阻
//功能描述: 计算校准电流对应的采样电阻
//注意事项:
//=============================================================================
s16 SH367309CalCurrentRes(s32 actcurrent)
{
    //return ((s32)gAFEData.curradval * 8440 / actcurrent);
    //return (gSHAFEData.curradval - gSHAFECfg.Cadcoffset) * SH367309_CALICUR / actcurrent;
    return (u16)(((float)(gSHAFEData.curradval - gSHAFECfg.Cadcoffset) * SH367309_CALICUR) / (21.47 * (float)actcurrent));	//1000 uR
}

//=============================================================================
//函数名称: u16 SH367309GetRealSCDCurr(void)
//输入参数: eepread:t_SH367309_EEPRom指针
//输出参数: SH367309真实的短路触发保护电流值
//功能描述: 获取真实的短路触发保护电流值
//注意事项:
//=============================================================================
u16 SH367309GetRealSCDCurr(t_SH367309_EEPRom* eepread)
{
	return cSH367309SCTab[eepread->SC.SCBit.SCV] / gSHAFECfg.calres;
}

//=============================================================================
//函数名称: u16 SH367309GetRealOCD1Curr(void)
//输入参数: eepread:t_SH367309_EEPRom指针
//输出参数: SH367309真实的放电过流1保护电流值
//功能描述: 获取真实的放电过流1保护电流值
//注意事项:
//=============================================================================
u16 SH367309GetRealOCD1Curr(t_SH367309_EEPRom* eepread)
{
	return cSH367309OCD1Tab[eepread->OCD1.OCD1Bit.OCD1V] / gSHAFECfg.calres;
}

//=============================================================================
//函数名称: u16 SH367309GetRealOCD2Curr(void)
//输入参数: eepread:t_SH367309_EEPRom指针
//输出参数: SH367309真实的放电过流1保护电流值
//功能描述: 获取真实的放电过流2保护电流值
//注意事项:
//=============================================================================
u16 SH367309GetRealOCD2Curr(t_SH367309_EEPRom* eepread)
{
	return cSH367309OCD2Tab[eepread->OCD2.OCD2Bit.OCD2V] / gSHAFECfg.calres;
}

//=============================================================================================
//函数名称	: void SH367309DevStatClear(u8 mask)
//输入参数	: mask：需要清除的位
//输出参数	: void
//静态变量	: void
//功    能	:SH367309清除设备状态标志
//注    意	:
//=============================================================================================
void SH367309DevStatClear(u8 mask)
{
	gSHAFEData.devsta &= ~mask;
}

//=============================================================================================
//函数名称	: void SH367309AlertIntCallback(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: AFE告警引脚中断调用函数
//注    意	:
//=============================================================================================
void SH367309AlertIntCallback(void)
{
    gSHAFEData.IntFlg = True;
}

//=============================================================================
//函数名称: void SH367309CalCurrent(s16 actcurrent)
//输入参数: actcurrent：校准的电流
//输出参数: void
//功能描述: SH367309校准采样电流
//注意事项: void
//=============================================================================
void SH367309CalCurrent(s16 actcurrent)
{
    gSHAFECfg.calcurr = actcurrent;
    SH367309EventFlagSet(SH367309_EVE_CTRL_CAL);
}

//=============================================================================
//函数名称: u8 SH367309OUVConfig(s16 ov,s16 ovr,s16 uv,s16 uvr)
//输入参数: ov:过压保护值，ovr:过压恢复值，uv：欠压保护值，uvr:欠压恢复值
//输出参数: 配置结果 成功:SH367309_IIC_NOFAULT_STAT;  失败:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//功能描述: SH367309过压、欠压配置
//注意事项:
//=============================================================================
u8 SH367309OUVConfig(s16 ov,s16 ovr,s16 uv,s16 uvr)
{
    u8 result = 0;

     //OVH OVL
    gSHAfeEep.OVH.OVHBit.OVT = SH367309_EEP_OVT_DLY_4S;             //过充电保护延时
    gSHAfeEep.OVH.OVHBit.LDRT = SH367309_EEP_LDRT_DLY_1000MS;       //负载释放延时设置控制位
    //过压保护配置
    gSHAfeEep.OVH.OVHBit.OV8_9 = (u8)(((ov / SH367309_EEP_OV_GAIN) & 0x0300) >> 8);
    gSHAfeEep.OVL.OVLBit.OV0_7 = (u8)((ov / SH367309_EEP_OV_GAIN) & 0x00ff);

    //OVRH OVRL
    gSHAfeEep.OVRH.OVRHBit.UVT = SH367309_EEP_UVT_DLY_4S;   //过放电保护延时设置
    //过压恢复配置
    gSHAfeEep.OVRH.OVRHBit.OVR8_9 = (u8)(((ovr / SH367309_EEP_OVR_GAIN) & 0x0300) >> 8);
    gSHAfeEep.OVRL.OVRLBit.OV0_7 = (u8)((ovr / SH367309_EEP_OVR_GAIN) & 0x00ff);
    
    //UV UVR
    gSHAfeEep.UV.UVBit.UV0_7 = (u8)((uv / SH367309_EEP_UV_GAIN) & 0x00ff);
    gSHAfeEep.UVR.UVRBit.UVR0_7 = (u8)((uvr / SH367309_EEP_UVR_GAIN) & 0x00ff);

	SH367309EventFlagSet(SH367309_EVE_CTRL_OUVD);

	return result;
}

//=============================================================================
//函数名称: u16 SH367309FindSOCDTable(u32 value , const u32 * tab , u16 range )
//输入参数: value：查找的值	* tab：查找表格指针		range:查找范围
//输出参数: 表格的索引，查找错误则回复0xffff
//功能描述: 查找表索引
//注意事项:
//=============================================================================
u16 SH367309FindSOCDTable(u32 value , const u32 * tab , u16 range )
{
    u32 curr1,curr2;

    curr1 = (tab[range-1] / gSHAFECfg.calres);	//计算此表最大值，确认是否值溢出表格

    if(value > curr1)
    {
        return 0xffff;		//溢出，回复0xffff
    }
    else if(value==curr1)
    {
        return range-1;		//溢出，回复此表最大值的数组索引
    }

    range--;

    while(range--)
    {
        curr2 = tab[range] / gSHAFECfg.calres;

        if(value >= curr2)
        {
            if((value - curr2) <= (curr1 - value))	//判断表格的值离实际值接近程度
            {
                return range;
            }
            else
            {
                return range + 1;
            }
        }
        curr1 = curr2;
    }
    return 0;
}

//=============================================================================
//函数名称: u8 SH367309OCD1Config(u16 ocd1value)
//输入参数: ocd1value：放电过流1保护值
//输出参数: 配置结果 成功:SH367309_IIC_NOFAULT_STAT;  失败:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//功能描述: SH367309放电过流1配置
//注意事项:
//=============================================================================
u8 SH367309OCD1Config(u16 ocd1value)
{
	u8 result = 0;			//配置结果
	u16 index = 0;	        //配置表索引
    
    index = SH367309FindSOCDTable(ocd1value, cSH367309OCD1Tab,16);
    
    if(0xffff == index)
    {
        index = SH367309_EEP_OCD1V_200MV;
    }
        
    #if defined(LFP_HL_25000MAH_16S)
    gSHAfeEep.OCD1.OCD1Bit.OCD1V = index;
    gSHAfeEep.OCD1.OCD1Bit.OCD1T = SH367309_EEP_OCD1T_DLY_1S;    
    #else
    gSHAfeEep.OCD1.OCD1Bit.OCD1V = index + 1;
    gSHAfeEep.OCD1.OCD1Bit.OCD1T = SH367309_EEP_OCD1T_DLY_30S;
    #endif
    
    return result;    
}

//=============================================================================
//函数名称: u8 SH367309OCD1Config(u16 ocd2value)
//输入参数: ocd2value：放电过流2保护值
//输出参数: 配置结果 成功:SH367309_IIC_NOFAULT_STAT;  失败:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//功能描述: SH367309放电过流2配置
//注意事项:
//=============================================================================
u8 SH367309OCD2Config(u16 ocd2value)
{
	u8 result = 0;			//配置结果
	u16 index = 0;	        //配置表索引
    
    index = SH367309FindSOCDTable(ocd2value, cSH367309OCD2Tab,16);
    
    if(0xffff == index)
    {
        index = SH367309_EEP_OCD2V_500MV;
    }
        
    #if defined(LFP_HL_25000MAH_16S)
    gSHAfeEep.OCD2.OCD2Bit.OCD2V = index;
    gSHAfeEep.OCD2.OCD2Bit.OCD2T = SH367309_EEP_OCD2T_DLY_200MS;    
    #else
    gSHAfeEep.OCD2.OCD2Bit.OCD2V = index;
    gSHAfeEep.OCD2.OCD2Bit.OCD2T = SH367309_EEP_OCD2T_DLY_1S;
    #endif
    
    return result;    
}

//=============================================================================
//函数名称: u8 SH367309SCConfig(u16 scvalue)
//输入参数: scvalue：短路保护值
//输出参数: 配置结果 成功:SH367309_IIC_NOFAULT_STAT;  失败:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//功能描述: SH367309短路配置
//注意事项:
//=============================================================================
u8 SH367309SCConfig(u16 scvalue)
{
	u8 result = 0;			//配置结果
	u16 index = 0;	        //配置表索引
    
    index = SH367309FindSOCDTable(scvalue, cSH367309SCTab,16);
    
    if(0xffff == index)
    {
        index = SH367309_EEP_SCV_1000MV;
    }
        
    gSHAfeEep.SC.SCBit.SCV = index;
    gSHAfeEep.SC.SCBit.SCT = SH367309_EEP_SCT_DLY_0US;
    
    return result;
}

//=============================================================================
//函数名称: u8 SH367309SCConfig(u16 scvalue)
//输入参数: scvalue：短路保护值
//输出参数: 配置结果 成功:SH367309_IIC_NOFAULT_STAT;  失败:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//功能描述: SH367309短路配置
//注意事项:
//=============================================================================
u8 SH367309OCCConfig(u16 occvalue)
{
	u8 result = 0;			//配置结果
	u16 index = 0;	        //配置表索引
    
    index = SH367309FindSOCDTable(occvalue, cSH367309OCCTab,16);
    
    if(0xffff == index)
    {
        index = SH367309_EEP_OCCV_200MV;
    }
        
    gSHAfeEep.OCC.OCCBit.OCCV = index;
    gSHAfeEep.OCC.OCCBit.OCCT = SH367309_EEP_OCCT_DLY_1S;
    
    return result;
}

//=============================================================================
//函数名称: u8 SH367309OCConfig(u16 ocd1value,u16 ocd2value,u16 scdvalue,u16 occvalue)
//输入参数: ocd1value：放电过流1保护，ocd2value：放电过流2保护，scdvalue:放电短路电流保护值	occvalue：充电过流保护值
//输出参数: 配置结果 成功:SH367309_IIC_NOFAULT_STAT;  失败:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//功能描述: SH367309 配置充放电过流、短路配置
//注意事项:
//=============================================================================
u8 SH367309OCConfig(u16 ocd1value,u16 ocd2value,u16 scdvalue,u16 occvalue)
{
	u8 result = 0;			//配置结果

    ocd1value += 2000;  //软件初级过流保护+20A，为硬件初级保护值
    result |= SH367309OCD1Config(ocd1value);
    result |= SH367309OCD2Config(ocd2value);
    result |= SH367309SCConfig(scdvalue);
    result |= SH367309OCCConfig(occvalue);

    SH367309EventFlagSet(SH367309_EVE_CTRL_OSCD);

	return result;
}


//=============================================================================
//函数名称: s8 BQ769X0_ChangADtoTemp(u8 hibyte, u8 lobyte)
//输入参数: TempAD采样AD值
//输出参数: 转化后的温度值
//功能描述: 温度AD值转化为实际温度值
//注意事项:
//=============================================================================
s16 SH367309CalADtoTemp(u8 hibyte, u8 lobyte)
{
    s32 getdata;
//	u8	i,TempeMiddle;
	u32	tempcalcu;
    
    getdata = (s32)(s16)((((u16)hibyte)<<8)|(lobyte));
    
//    if(res >= NTC_REFV_LSB)
//    {
//        return NTC_TEMP_OFF;
//    }
//    res = 10000 * res / (NTC_REFV_LSB - res);

//    return NTCTempQuery(res , NTC_RESTAB[0].tab, NTC_RESTAB[0].tablen, NTC_RESTAB[0].tabofs);
    
	tempcalcu = (u32)getdata * (5 * gSHAfeEepRead.TR.TRBit.TR0_6 + SH367309_REF_RES_VAL) / (32768 - getdata);
    
    return NTCTempQuery(tempcalcu * 10 , NTC_RESTAB[0].tab, NTC_RESTAB[0].tablen, NTC_RESTAB[0].tabofs);

//	if(tempcalcu >= NTC103AT[0])			//Determine whether the excess temperature resistance range
//	{
//		Temperature = 2731-500;			
//	}
//	else if(tempcalcu <= NTC103AT[160])
//	{
//		Temperature = 2731+1100;
//	}
//	else
//	{
//		i = TempeMiddle;
//		if(tempcalcu > NTC103AT[i])
//		{
//			for(i=TempeMiddle-1; i>0; i--)
//			{
//				if(tempcalcu <= NTC103AT[i])		//NTC103AT[i+1]<resis<NTC103AT[i]
//				{
//					break;
//				}
//			}
//		}
//		else
//		{
//			for(i=TempeMiddle+1; i<160; i++)
//			{
//				if(tempcalcu > NTC103AT[i])		//NTC103AT[i-1]<resis<NTC103AT[i]
//				{
//					break;
//				}
//			}
//			i--;
//		}
//		TempeMiddle = i;
//		
//		Temperature = (u16)(TempeMiddle-50)*10+(NTC103AT[i]-tempcalcu)*10/(NTC103AT[i]-NTC103AT[i+1])+2731;
//	}
//	return Temperature;    
}

//=============================================================================================
//函数名称	: u8 SH367309DirectReadData(u16 *volt,s16 *temp,s16 *curr);
//输入参数	: volt:放置的电压数组指针
//输出参数	: 是否有误：0：无错误；1：读取iic错误；2：校验crc错误
//功能描述	: 直接获取AFE采样的数据，不阻塞
//注意事项	:
//=============================================================================================
//u8 SH367309DirectReadData(u16 *volt,s16 *temp,s16 *curr)	//该函数暂时不使用
//{
//    status_t u8Status = STATUS_SUCCESS;	
//    u8 i = 0;
//    
//    gSH367309Writebuff[0] = SH367309_REG_CONF;
//    gSH367309Writebuff[1] = sizeof(gSHAFEReg); 
//    
//    u8Status = BSPIICRcvMsgEx(SH367309_IIC_CHANNEL,SH367309_WRITE_ADDR,gSH367309Writebuff,2,
//                                                        (u8 *)&gSHAFEReg,sizeof(gSHAFEReg),1);
//    
//    if(STATUS_SUCCESS != u8Status)      
//    {
//        gSHAFEData.iicerr ++;
//    }            
//    else
//    {
//        *curr = SH367309CalADtoCurr(gSHAFEReg.CurrH,gSHAFEReg.CurrL);
//        
//        //获取单体温度值
//        for(i = 0;i < SH367309_TEMP_NUM;i++)
//        {
//            //单体温度ad转换
//            *temp++ = SH367309CalADtoTemp(*(((u8 *)&gSHAFEReg.Temp1H) + 2 * i),\
//                                                         *(((u8 *)&gSHAFEReg.Temp1L) + 2 * i) );                
//        }
//        
//        //获取单体电压值
//        for(i = 0;i < SH367309_VOLT_NUM;i++)
//        {
//            //单体电压ad转换
//            *volt++ = SH367309CalADtoVolt(*(((u8 *)&gSHAFEReg.Cell1H) + 2 * i),\
//                                                         *(((u8 *)&gSHAFEReg.Cell1H) + 1 + 2 * i) );
//        }
//    }
//   
//    return (u8)STATUS_SUCCESS;
//}

//=============================================================================================
//函数名称	: u8 SH367309BqInitReset(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SH367309复位
//注    意	:
//=============================================================================================
void SH367309BqInitReset(void)
{
	PT_INIT(&gPTSH367309);
	PT_INIT(&gPTSH367309Norm);    
    
    gSHAFEData.ResetFlg = True;
	gSHAFEData.runstatus = e_SH_AFE_Sta_Bqinit;    
}

//=============================================================================================
//函数名称	: u8 SH367309MemCopy(const u8 *msg1, const u8 *msg2, u16 cnt)
//输入参数	: dest：目标数据地址； *sour：源数据地址；cnt：复制的长度
//输出参数	:
//函数功能	: 数据复制
//注意事项	:
//=============================================================================================
void SH367309MemCopy(u8 *dest, const u8 *sour, u16 cnt)
{
    while(0 < cnt--)
    {
       *(dest++) = *(sour++);
    }
}

//=============================================================================================
//函数名称	: u8 SH367309MemComp(const u8 *msg1, const u8 *msg2, u16 cnt)
//输入参数	: msg1：比较数据1；msg2：比较的数据2；cnt：比较的长度
//输出参数	: TRUE：比较正确；FALSE：比较错误
//函数功能	: 数据比较
//注意事项	:
//=============================================================================================
u8 SH367309MemComp(const u8 *msg1, const u8 *msg2, u16 cnt)
{
    while(0 < cnt--)
    {
        if(*(msg1++) != *(msg2++))
        {
            return FALSE;
        }
    }
    return TRUE;
}

//=============================================================================
//函数名称: static void SH367309ConfigInitTask(void)
//输入参数: void
//输出参数: void
//功能描述: SH367309上电配置初始化任务
//注意事项: 50ms周期
//=============================================================================
static void SH367309ConfigInitTask(void)
{
	static u8 result = SH367309_IIC_NOFAULT_STAT;
	static u16 timer = 0;
    static u8 slaststep = 0;
	static u8 sstep = 0;
	static u8 errcnt = 0;
    static u8 firstflag = 0;
    static u8 i = 0;
    u8 data = 0;
    
    if(gSHAFEData.ResetFlg == True)
    {
        gSHAFEData.ResetFlg = False;
        result = SH367309_IIC_NOFAULT_STAT;
        timer = 0;
        slaststep = 0;
        sstep = 0;
        firstflag = 0;
        i = 0;
    }
    
	switch(sstep)
	{
        case 0:
            SH367309_SHIP_OFF;  /* 仓运模式关闭 */
			sstep++;
            BSPTaskStart(TASK_ID_SH367309_TASK, 250);	
			break;
        
        //clear 309 WAKE_FLG
		case 1:
            data = 0x00;
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BFLAG1,&data);
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;   
        
        //clear 309 Protect flag
		case 2:
            data = 0x00;
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BFLAG2,&data);
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;                       
        
        
        case 3:    
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,(u8 *)&gSHAfeEepRead,\
                                                            sizeof(t_SH367309_EEPRom),SH367309_EEP_SCONF1);
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;
        
        /* 给配置参数赋值 */
		case 4:
           //SCONF1
            gSHAfeEep.SCONF1.SCONF1Bit.ENPCH = SH367309_EEP_ENPCH_DISABLE;  //禁用预充电功能
            gSHAfeEep.SCONF1.SCONF1Bit.ENMOS = SH367309_EEP_ENMOS_ENABLE;   //启用充电MOSFET恢复控制位。当过充电/温度保护关闭充电MOSFET后，如果检测到放电过流1或者放电状态，则开启充电MOSFET
            gSHAfeEep.SCONF1.SCONF1Bit.OCPM = SH367309_EEP_OCPM_DISABLE;    //充电过流只关闭充电MOSFET；放电过流只关闭放电MOSFET
            gSHAfeEep.SCONF1.SCONF1Bit.BAL = SH367309_EEP_BAL_MCU;          //平衡开启由外部MCU控制，平衡时序仍由SH367309内部逻辑控制
        
            #ifdef BMS_BAT_16S
            gSHAfeEep.SCONF1.SCONF1Bit.CN = SH367309_EEP_CELL16;            //16节电芯应用  
            #elif defined(BMS_BAT_15S)
            gSHAfeEep.SCONF1.SCONF1Bit.CN = SH367309_EEP_CELL15;            //15节电芯应用
            #elif defined(BMS_BAT_14S)
            gSHAfeEep.SCONF1.SCONF1Bit.CN = SH367309_EEP_CELL14;            //15节电芯应用        
            #endif
        
            //SCONF2
            gSHAfeEep.SCONF2.SCONF2Bit.EOVB = SH367309_EEP_E0VB_ENABLE;     //开启“禁止低压电芯充电”功能
            gSHAfeEep.SCONF2.SCONF2Bit.UV_OP = SH367309_EEP_UV_OP_DISABLE;  //过放电只关闭放电MOSFET
            gSHAfeEep.SCONF2.SCONF2Bit.DIS_PF = SH367309_EEP_DIS_PF_DISABLE; //禁止二次过充电保护
            gSHAfeEep.SCONF2.SCONF2Bit.CTLC = SH367309_EEP_CTLC_ALL;//SH367309_EEP_CTLC_INVALID;    //充放电和预充电MOSFET由内部逻辑控制，CTL管脚输入无效
            gSHAfeEep.SCONF2.SCONF2Bit.OCRA = SH367309_EEP_OCRA_ENABLE;     //允许电流保护定时恢复 
            gSHAfeEep.SCONF2.SCONF2Bit.EUVR = SH367309_EEP_EUVR_DISABLE;    //过放电保护状态释放无需负载释放
                   
            //OVH OVL OVRH OVRL UV UVR 充放电保护和恢复值配置
            SH367309OUVConfig(gConfig.cellov.matth[3],gConfig.cellov.resth[3],\
                              gConfig.celluv.matth[3],gConfig.celluv.resth[3]);
            SH367309EventFlagClear(SH367309_EVE_CTRL_OUVD);
        
            //BALV 平衡开启电压
            gSHAfeEep.BALVByte = SH367309_CFG_BALV / SH367309_EEP_BALV_GAIN;
            
            //PREV 预充电电压设定值
            gSHAfeEep.PREVByte = SH367309_CFG_PREV / SH367309_EEP_PREV_GAIN;
        
            //L0V 低压禁止充电电压设定值
            gSHAfeEep.L0VByte = SH367309_CFG_L0V / SH367309_EEP_L0V_GAIN;
            
            //PFV 二次过压充电保护电压
            gSHAfeEep.PFVByte = SH367309_CFG_PFV / SH367309_EEP_PFV_GAIN;
            
            //OCD1 OCD2 SC OCC 充放电过流、短路保护配置
            SH367309OCConfig(0 - gConfig.dcurr.matth[1],0-gConfig.dcurr.matth[2],0 - gConfig.dcurr.matth[3],\
                             SH367309_CFG_OCC); //充电过流配置为20A
            SH367309EventFlagClear(SH367309_EVE_CTRL_OSCD);
                           
            //充放电状态检测电压                           
            gSHAfeEep.MOST_OCRT_PFT.MOST_OCRT_PFTBit.CHS = SH367309_EEP_CHS_500UV; 
            //充放电MOSFET开启延时
            gSHAfeEep.MOST_OCRT_PFT.MOST_OCRT_PFTBit.MOST = SH367309_EEP_MOST_DLY_64US;
            //充放电过流自恢复延时
            gSHAfeEep.MOST_OCRT_PFT.MOST_OCRT_PFTBit.OCRT = SH367309_EEP_OCRT_DLY_8S;
            //二次过充电保护延时
            gSHAfeEep.MOST_OCRT_PFT.MOST_OCRT_PFTBit.PFT = SH367309_EEP_PFT_DLY_8S;
            
            //充电高温保护，更改为连接器的保护温度
            gSHAfeEep.OTCByte = SH367309CalTemptoOTAD(gConfig.connot.matth[3]);            
            //gSHAfeEep.OTCByte = 0x78;
            
            //充电高温保护释放
            gSHAfeEep.OTCRByte = SH367309CalTemptoOTAD(gConfig.connot.resth[3]);
            //gSHAfeEep.OTCRByte = 0x87;
            
            //充电低温保护
            gSHAfeEep.UTCByte = SH367309CalTemptoUTAD(gConfig.cellcut.matth[3] - SH367309_CFG_UT_OFFSET);
            //gSHAfeEep.UTCByte = 0x79;
            
            //充电低温保护释放
            gSHAfeEep.UTCRByte = SH367309CalTemptoUTAD(gConfig.cellcut.resth[3] - SH367309_CFG_UT_OFFSET);
            //gSHAfeEep.UTCRByte = 0x63;      
            
            //放电高温保护，更改为连接器的保护温度
            gSHAfeEep.OTDByte = SH367309CalTemptoOTAD(gConfig.connot.matth[3]);
            //gSHAfeEep.OTDByte = 0x5E;
            
            //放电高温保护释放
            gSHAfeEep.OTDRByte = SH367309CalTemptoOTAD(gConfig.connot.resth[3]);
            //gSHAfeEep.OTDRByte = 0x6B;
            
            //放电低温保护
            gSHAfeEep.UTDByte = SH367309CalTemptoUTAD(gConfig.celldut.matth[3] - SH367309_CFG_UT_OFFSET);
            //gSHAfeEep.UTDByte = 0x8E;
            
            //放电低温保护释放
            gSHAfeEep.UTDRByte = SH367309CalTemptoUTAD(gConfig.celldut.resth[3] - SH367309_CFG_UT_OFFSET);
            //gSHAfeEep.UTDRByte = 0x79;          
            //温度内部参考电阻系数寄存器
            //gSHAfeEep.TR.TRBit.TR0_6 = 0x37; //无法更改             
            
            if(SH367309MemComp((u8 *)&gSHAfeEep,(u8 *)&gSHAfeEepRead,(sizeof(t_SH367309_EEPRom) - 1)))
            {
                sstep += 4;
                BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
            }
            else
            {   
                i = 0;
                //进行参数配置
                SH367309_WRITE_EEP_ON;
                BSPTaskStart(TASK_ID_SH367309_TASK, 10);       
                sstep++;
            }
            
            slaststep = sstep;
            
			break;  
            
        //更新内部EEP
		case 5:
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_EEP_SCONF1 + i,\
                                                    ((u8 *)&gSHAfeEep) + i);
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;                           
            
            
        //复位命令1
		case 6:
            data = 0xC0;
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,0xEA,&data);
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;   
        
        //复位命令2
		case 7:
            data = 0xA5;
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,0xEA,&data);            
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;                 
        
		case 8:
            //关闭MOS管，打开CADCON
            data = 0x08;
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_CONF,&(data));
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;

        case 9:
            sstep++;
            BSPTaskStart(TASK_ID_SH367309_TASK, 250);            
			break;            
            
        case 10:
            //再读一次配置参数
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,(u8 *)&gSHAfeEepRead,\
                                                            sizeof(t_SH367309_EEPRom),SH367309_EEP_SCONF1);
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;
        
        case 11:
            BITCLR(gSHAFEData.initerr,0);
            errcnt = 0;
            timer = 0;
            
            //首次上电，不用进入延时
            if(0 == firstflag)
            {
                firstflag = 1;
                
                //计算实际配置的保护值
                gSHAFECfg.scdval = SH367309GetRealSCDCurr(&gSHAfeEepRead);
                gSHAFECfg.ocd1val = SH367309GetRealOCD1Curr(&gSHAfeEepRead);
                gSHAFECfg.ocd2val = SH367309GetRealOCD2Curr(&gSHAfeEepRead);
                
                gSHAFECfg.ovdval = SH367309CalADtoOVTrip(&gSHAfeEepRead);
                gSHAFECfg.uvdval = SH367309CalADtoUVTrip(&gSHAfeEepRead);   
                
                gSHAFEData.runstatus = e_SH_AFE_Sta_Norm;
                sstep = 0;
                slaststep = 0;
                errcnt = 0;
            }
            else
            {
                sstep++;               
            }
            
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);	                    
            break;
        
        case 12:
			if(timer >= 40)			//延迟2s进入采样
			{
				timer = 0;
				sstep = 0;
                //配置保护值
                gSHAFEData.runstatus = e_SH_AFE_Sta_Norm;
			}
			else
			{
				timer++;
			}
            
            BSPTaskStart(TASK_ID_SH367309_TASK, 50);	                
            break;
        
        //IIC通信处理步骤
        case 0xaa:
            if(BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0)
            {
                if(BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0)      
                {
                    result = SH367309_IIC_FAULT_STAT; 
                    sstep = 0xbb; //错误处理步骤
                    i = 0;
                    SH367309_WRITE_EEP_OFF;
                    BSPTaskStart(TASK_ID_SH367309_TASK, 2);
                }               
                else
                {
                    result = SH367309_IIC_NOFAULT_STAT; 
                    if(5 == slaststep && i < (sizeof(t_SH367309_EEPRom) - 1))  //配置EEP
                    {
                        //烧写完EEP一个字节后，至少延时35ms
                        i++;
                        sstep = slaststep;
                        BSPTaskStart(TASK_ID_SH367309_TASK, 35);	      
                    }
                    else
                    {
                        i = 0;
                        sstep = slaststep + 1;
                        SH367309_WRITE_EEP_OFF;
                        
                        BSPTaskStart(TASK_ID_SH367309_TASK, 2);
                    }                
                }
            }
            else
            {
                BSPTaskStart(TASK_ID_SH367309_TASK, 2);
            }
            
            break;
            
        //步骤出现错误处理
        case 0xbb:  
            //连续初始化2次仍失败则跳出
            if(SH367309_IIC_FAULT_STAT == result)
            {
                errcnt++;
            }
            if(errcnt >= (u16)2)
            {
                BITSET(gSHAFEData.initerr,0);
                errcnt = 0;
                timer = 0;
                sstep = 0;
                slaststep = 0;
                i = 0;
                gSHAFEData.runstatus = e_SH_AFE_Sta_Offline;
                result = SH367309_IIC_NOFAULT_STAT;
            }
            else
            {
                timer = 0;
                sstep = 0;
                slaststep = 0;       
                i = 0;
            }

            BSPTaskStart(TASK_ID_SH367309_TASK, 50);	                 
            break;     

		default:
			sstep = 0;
            slaststep = 0;
			break;            
		}
}

//=============================================================================================
//函数名称	: void SH367309DisableTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SH367309关闭任务函数
//注    意	: 50ms周期
//=============================================================================================
static void SH367309DisableTask(void)
{
    static u8 timer = 0;
    
    PT_INIT(&gPTSH367309);
    PT_INIT(&gPTSH367309Norm);
    
	SH367309EventFlagClear(SH367309_EVE_CTRL_SLP);
	gSHAFEData.runstatus = e_SH_AFE_Sta_Disable;
    BSPTaskStart(TASK_ID_SH367309_TASK, 50);	  
    
	if(timer++ >= 10)	//延迟500ms
	{
		timer = 0;
		//进入初始化函数
        SH367309BqInitReset();
		gSHAFEData.runstatus = e_SH_AFE_Sta_Bqinit;

	}    
}

//=============================================================================================
//函数名称	: static void SH367309NormalTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SH367309正常执行任务函数
//注    意	: 50ms周期
//=============================================================================================
static void SH367309NormalTask(void)
{
	static u8 i = 0;
    u8 balance;
    static u8 data;
    
	PT_BEGIN(&gPTSH367309Norm);

    //有AFE告警发生，进行告警处理
    //if(BSPGPIOGetPin(SH367309_ALERT_PORT, SH367309_ALERT_PIN) != 0)
    if(gSHAFEData.IntFlg)
    {
        gSHAFEData.IntFlg = False;
        
        //读取AFE STATUS标志
        gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                    (u8 *)&gSHAFEReg.BSTATUS1.BSTATUS1Byte,1,SH367309_REG_BSTATUS1);  
        
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
        
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }            
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
            
            if(gSHAFEReg.BSTATUS1.BSTATUS1Bit.OCD2)
            {
                gSHAFEData.devsta |= SH367309_CLR_OCD2;
            }
            else
            {
                gSHAFEData.devsta &= ~SH367309_CLR_OCD2;
            }
            
             if(gSHAFEReg.BSTATUS1.BSTATUS1Bit.OCD1)
            {
                gSHAFEData.devsta |= SH367309_CLR_OCD1;
            }
            else
            {
                gSHAFEData.devsta &= ~SH367309_CLR_OCD1;
            }           
        }        
        
        //读取AFE BFLAG标志
        gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                    (u8 *)&gSHAFEReg.BFLAG1.BFLAG1Byte,2,SH367309_REG_BFLAG1);
        
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
        
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }            
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
                        
            gSHAFEReg.BFLAG1.BFLAG1Bit.OCD_FLG = 0;
            gSHAFEData.devsta |= gSHAFEReg.BFLAG1.BFLAG1Byte;
        }
        
        DTCheckAFEProtectFlag();		//AFE设备状态标志判断
        
        if(gSHAFEReg.BFLAG2.BFLAG2Bit.CADC_FLG)
        {
            //读CADC电流
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                    (u8 *)gSH367309Readbuff,2,SH367309_REG_ADC2);
            
            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }            
            else
            {
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;

                //获取电流采样值
                //单体电流ad转换
                gSHAFEData.curr = SH367309CalCADCtoCurr(gSH367309Readbuff[0],gSH367309Readbuff[1]); 
            }                   
        }
        
        if(gSHAFEReg.BFLAG1.BFLAG1Byte 
            || gSHAFEReg.BFLAG2.BFLAG2Bit.UTC_FLG || gSHAFEReg.BFLAG2.BFLAG2Bit.OTC_FLG
            || gSHAFEReg.BFLAG2.BFLAG2Bit.UTD_FLG || gSHAFEReg.BFLAG2.BFLAG2Bit.OTD_FLG)
        {
            data = 0;
            //消除标志1
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BFLAG1,&data);

            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
                
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }                    
            else
            {
                //消除标志成功
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
            }      

            //消除标志2
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BFLAG2,&data);

            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
                
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }                    
            else
            {
                //消除标志成功
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
            }         
        }
        
        //存在复位或者唤醒标志
        if(gSHAFEReg.BFLAG2.BFLAG2Bit.WAKE_FLG || gSHAFEReg.BFLAG2.BFLAG2Bit.RST_FLG)
        {
            data = 0;
            //消除标志1
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BFLAG1,&data);

            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
                
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }                    
            else
            {
                //消除标志成功
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
            }      

            //消除标志2
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BFLAG2,&data);

            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
                
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }                    
            else
            {
                //消除标志成功
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
            }                
            //开CADC，同时关闭IDLE和SLEEP模式
            gSHAFEReg.CONF.CONFByte |= 0x08;
            gSHAFEReg.CONF.CONFByte = (gSHAFEReg.CONF.CONFByte&0xF8) | 0x08;
            
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_CONF,&gSHAFEReg.CONF.CONFByte);

            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
                
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }                    
            else
            {
                //消除标志成功
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
            }                      
        }
    }

    //采样
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_SMP)
    {
        gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,(u8 *)&gSHAFEReg,sizeof(gSHAFEReg),SH367309_REG_CONF);
        
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
        
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }            
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;         
            
            if(gSHAFEReg.BSTATUS3.BSTATUS3Bit.CHG_FET)
            {
                BITSET(gSHAFEData.mosstatus,0);
            }
            else
            {
                BITCLR(gSHAFEData.mosstatus,0);
            }

            if(gSHAFEReg.BSTATUS3.BSTATUS3Bit.DSG_FET)
            {
                BITSET(gSHAFEData.mosstatus,1);
            }
            else
            {
                BITCLR(gSHAFEData.mosstatus,1);
            }
            //读取AFE STATUS标志
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                        (u8 *)&gSHAFEReg.BSTATUS1.BSTATUS1Byte,1,SH367309_REG_BSTATUS1);  
            
            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }            
            else
            {
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
            
                if(gSHAFEReg.BSTATUS1.BSTATUS1Bit.OCD2)
                {
                    gSHAFEData.devsta |= SH367309_CLR_OCD2;
                }
                else
                {
                    gSHAFEData.devsta &= ~SH367309_CLR_OCD2;
                }
                
                 if(gSHAFEReg.BSTATUS1.BSTATUS1Bit.OCD1)
                {
                    gSHAFEData.devsta |= SH367309_CLR_OCD1;
                }
                else
                {
                    gSHAFEData.devsta &= ~SH367309_CLR_OCD1;
                }           
            }        
            
            //读取AFE BFLAG标志
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                        (u8 *)&gSHAFEReg.BFLAG1.BFLAG1Byte,2,SH367309_REG_BFLAG1);
            
            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }            
            else
            {
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
                
                gSHAFEReg.BFLAG1.BFLAG1Bit.OCD_FLG = 0;
                gSHAFEData.devsta |= gSHAFEReg.BFLAG1.BFLAG1Byte;
            }
            
            DTCheckAFEProtectFlag();		//AFE设备状态标志判断            
            gSHAFEData.MosUptFlg = True;
            
            //获取单体温度值
            for(i = 0;i < SH367309_TEMP_NUM;i++)
            {
                //单体温度ad转换
                gSHAFEData.celltemp[i] = SH367309CalADtoTemp(*(((u8 *)&gSHAFEReg.Temp1H) + 2 * i),\
                                                             *(((u8 *)&gSHAFEReg.Temp1L) + 2 * i) );                
            }
            
            //获取单体电压值
            for(i = 0;i < SH367309_VOLT_NUM;i++)
            {
                //单体电压ad转换
                gSHAFEData.cellvolt[i] = SH367309CalADtoVolt(*(((u8 *)&gSHAFEReg.Cell1H) + 2 * i),\
                                                             *(((u8 *)&gSHAFEReg.Cell1H) + 1 + 2 * i) );
            }
            if(True == gModVoltTestCmd) //第二节单体电压更改测试
            {
                gSHAFEData.cellvolt[1] = ((u32)gSHAFEData.cellvolt[1] * gConfig.calpara.b16gain) / 2000;
            }                
            //刷新数据
            SH367309RefreshDataToBatInfo();
            DataDealSetBMSDataRdy(e_AFERdy); 
            //
            DataDealSetBMSDataRdy(e_FuelRdy);  
//            BSPTaskStart(TASK_ID_SAMPLE_TASK, 5);
//            _UN_NB_Printf("ADC start %d \r\n",HAL_GetTick());             
        }

        SH367309EventFlagClear(SH367309_EVE_CTRL_SMP);
    }

    //均衡
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_BAL)
    {
        balance = (u8)(gSHAFECtrl.balctrl >> 8);
        SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BALANCEH,(u8 *)(&balance));
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
    
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }    
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        
        balance = (u8)(gSHAFECtrl.balctrl);
        SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BALANCEL,(u8 *)(&balance));
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
    
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }    
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        
        SH367309EventFlagClear(SH367309_EVE_CTRL_BAL);
    }

    //睡眠
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_SLP)
    {

        SH367309_SHIP_ON;   /* 仓运模式开启 */
        
        gSHAFEData.runstatus = e_SH_AFE_Sta_Disable;
        SH367309EventFlagClear(SH367309_EVE_CTRL_SLP);
    }

    //电流标定
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_CAL)
    {
        if(((0 != gSHAFEData.curr) && (0 != gSHAFECfg.calcurr)))
        {
            s16 ttmp;
           
            //读CADC电流
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                    (u8 *)&gSHAFEReg.CADCCurrH,2,SH367309_REG_ADC2);            
            
            gSHAFEData.curradval = (s32)(s16)((((u16)gSHAFEReg.CADCCurrH)<<8)|(gSHAFEReg.CADCCurrL));
            
            ttmp = SH367309CalCurrentRes(((s32)gSHAFECfg.calcurr)*10);

            if((ttmp >= SH367309_SAMPLE_RES_CAL_MIN)&&(ttmp <= SH367309_SAMPLE_RES_CAL_MAX))//判断计算值
            {
                //赋值后gConfig与gConfigBuff一样就无法保存
                gConfigBuff.calpara.sampleres = ttmp;
                gSHAFECfg.calres = ttmp;
                gStorageCTRL |= FLASH_WR_CFG_MASK;	//存储
                //更新OCD SCD的寄存器值
                //OCD1 OCD2 SC OCC 充放电过流、短路保护配置
                SH367309OCConfig(0 - gConfig.dcurr.matth[1],0-gConfig.dcurr.matth[2],0 - gConfig.dcurr.matth[3],\
                                 SH367309_CFG_OCC); //充电过流配置为20A
            }
        }
        /* 零飘校准 */
        else if(0 == gSHAFECfg.calcurr)
        {
            //读CADC电流
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                    (u8 *)&gSHAFEReg.CADCCurrH,2,SH367309_REG_ADC2);            
            
            gSHAFEData.curradval = (s32)(s16)((((u16)gSHAFEReg.CADCCurrH)<<8)|(gSHAFEReg.CADCCurrL));
            
            gConfigBuff.calpara.sampleoff = gSHAFEData.curradval;
            gSHAFECfg.Cadcoffset = gSHAFEData.curradval;
            gStorageCTRL |= FLASH_WR_CFG_MASK;	//存储            
        }
        
        SH367309EventFlagClear(SH367309_EVE_CTRL_CAL);
    }

    //配置OV TRIP 和 UP TRIP
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_OUVD)
    {
        //写入OV 和 UV值
        SH367309_WRITE_EEP_ON;
        gSHAFEData.currtime = gTimer1ms;
        
        //延时10ms
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                        && ((gTimer1ms - gSHAFEData.currtime) > 20));
        
        for(i = 0;i < (SH367309_EEP_UVR - SH367309_EEP_OVH + 1);i++)
        {
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL, SH367309_EEP_OVH + i, \
            ((u8 *)(&gSHAfeEep.OVH))+ i);
            
            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }    
            else
            {
                //控制成功
                //延时35ms
                PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                                && ((gTimer1ms - gSHAFEData.currtime) > 35));                  
            }
        }

        //计算出实际的使用值
        gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,(u8 *)&gSHAfeEepRead.OVH,\
                                          (SH367309_EEP_UVR - SH367309_EEP_OVH + 1),SH367309_EEP_OVH);

        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }    
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
            gSHAFECfg.ovdval = SH367309CalADtoOVTrip(&gSHAfeEepRead);
            gSHAFECfg.uvdval = SH367309CalADtoUVTrip(&gSHAfeEepRead);
        }
        
        SH367309_WRITE_EEP_OFF;
        
        gSHAFEData.currtime = gTimer1ms;
        //延时10ms
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                        && ((gTimer1ms - gSHAFEData.currtime) > 20));
        
        /* 复位 */
        data = 0xC0;
        gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,0xEA,&data);
 
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }                    
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        
        data = 0xA5;
        gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,0xEA,&data);
 
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        
        gSHAFEData.currtime = gTimer1ms;
        //延时500ms
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                        && ((gTimer1ms - gSHAFEData.currtime) > 500));        
        
        SH367309EventFlagClear(SH367309_EVE_CTRL_OUVD);
    }

    //配置充放电过流保护
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_OSCD)
    {
        //开启烧写模式
        SH367309_WRITE_EEP_ON;
        
        gSHAFEData.currtime = gTimer1ms;
        //延时10ms
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                        && ((gTimer1ms - gSHAFEData.currtime) > 20));        
        
        for(i = 0;i < (SH367309_EEP_OCC - SH367309_EEP_OCD1 + 1);i++)
        {
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL, SH367309_EEP_OCD1 + i, \
            ((u8 *)(&gSHAfeEep.OCD1.OCD1Byte))+ i);
            
            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }    
            else
            {
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;

                //控制成功
                gSHAFEData.currtime = gTimer1ms;
                //延时35ms
                PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                                && ((gTimer1ms - gSHAFEData.currtime) > 35));                      
            }
        }

        //计算出实际的使用值				BSPIIC_StateGet( DA213_IIC_CHANNEL )
        gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,(u8 *)&gSHAfeEepRead.OCD1.OCD1Byte,\
                                          (SH367309_EEP_OCC - SH367309_EEP_OCD1 + 1),SH367309_EEP_OCD1);

        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }    
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;

            gSHAFECfg.scdval = SH367309GetRealSCDCurr(&gSHAfeEepRead);
            gSHAFECfg.ocd1val = SH367309GetRealOCD1Curr(&gSHAfeEepRead);
            gSHAFECfg.ocd2val = SH367309GetRealOCD2Curr(&gSHAfeEepRead);         
        }
        
        //关闭烧写模式
        SH367309_WRITE_EEP_OFF;
        
        gSHAFEData.currtime = gTimer1ms;
        //延时10ms
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                        && ((gTimer1ms - gSHAFEData.currtime) > 20));
        
        /* 复位 */
        data = 0xC0;
        gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,0xEA,&data);
 
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }                    
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        
        data = 0xA5;
        gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,0xEA,&data);
 
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        
        gSHAFEData.currtime = gTimer1ms;
        //延时500ms
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                        && ((gTimer1ms - gSHAFEData.currtime) > 500));               
        
        SH367309EventFlagClear(SH367309_EVE_CTRL_OSCD);
    }

    //MOS管控制
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_MOS)
    {
        //写入控制MOS管
        //result = SH367309MosCtrl(gSHAFECtrl.mosctrl);
        data = ((gSHAFECtrl.mosctrl & 0x03) << 4) | (gSHAFEReg.CONF.CONFByte & 0xCF);
        gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_CONF,\
                                                        &(data));

        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }    
        else
        {
            //控制成功
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        SH367309EventFlagClear(SH367309_EVE_CTRL_MOS);
    }

	gSHAFEData.faultcnt = 0;	//清零故障标志
    
	if(gSHAFEData.iicerr >= 5 && gSHAFEData.iicerr < 30)	//出现通信故障
	{
        //IIC解锁
        /*
        BSPGPIOUnLockLpIIC();
        BSPIICReInit(eIIC0);
        */
        BSPI2C_DeInit(SH367309_IIC_CHANNEL);
        BSPI2C_Init(SH367309_IIC_CHANNEL);
	}    
	else if(gSHAFEData.iicerr >= 30)	//出现断线故障
	{
		gSHAFEData.runstatus = e_SH_AFE_Sta_Offline;
	}
    
    BSPTaskStart(TASK_ID_SH367309_TASK, 50);	  
    PT_INIT(&gPTSH367309Norm);      
	PT_END(&gPTSH367309Norm);    
}

//=============================================================================================
//函数名称	: void SH367309OffLineTask(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SH367309断线/故障后任务函数
//注    意	: 50ms周期
//=============================================================================================
static void SH367309OffLineTask(void)
{
	static u8 timer = 0;	//定时器，用于延时，100ms周期

    if(gSHAFEData.faultcnt < 100)
    {
    	gSHAFEData.faultcnt ++;//设置AFE故障
    }

    if(gSHAFEData.faultcnt >= 5)
    {
        SH367309ClearData();
    }

    if(gSHAFEData.iicerr >= 1000)
    {
        gSHAFEData.iicerr = 1000;
    }

    timer++;
	if(SH367309EventFlagGet() & SH367309_EVE_CTRL_SLP)
	{
		gSHAFEData.runstatus = e_SH_AFE_Sta_Disable;
	}

	if(timer >= 10)	//延迟500ms
	{
		timer = 0;
        
        //IIC解锁
        /*
        BSPGPIOUnLockLpIIC();
        BSPIICReInit(eIIC0);
        */
        BSPI2C_DeInit(SH367309_IIC_CHANNEL);
        BSPI2C_Init(SH367309_IIC_CHANNEL);
		//进入初始化函数
        SH367309BqInitReset();
		gSHAFEData.runstatus = e_SH_AFE_Sta_Bqinit;

	}
    BSPTaskStart(TASK_ID_SH367309_TASK, 50);	         
}

//=============================================================================================
//函数名称	: void SH367309MainTask(void *p)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SH367309主任务函数
//注    意	:
//=============================================================================================
void SH367309MainTask(void *p)
{
	(void)p;

	PT_BEGIN(&gPTSH367309);

	for(;;)
    {
		switch(gSHAFEData.runstatus)
		{
			/*	AFE初始化任务	*/
			case e_SH_AFE_Sta_Bqinit:
				SH367309ConfigInitTask();
				break;

			/*	AFE正常任务	*/
			case e_SH_AFE_Sta_Norm:
				SH367309NormalTask();
			break;

			/*	AFE掉线任务	*/
			case e_SH_AFE_Sta_Offline:
				SH367309OffLineTask();
				break;

			/*	AFE关闭任务	*/
			case e_SH_AFE_Sta_Disable:
				SH367309DisableTask();
				break;

			default:
			;
			break;
		}

		PT_NEXT(&gPTSH367309);
	}
	PT_END(&gPTSH367309);
}

/*****************************************end of SH367309.c*****************************************/
