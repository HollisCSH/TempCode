/****************************************************************************/
/* 	File    	RTC.c 			 
 * 	Author		Hollis
 *	Notes		用于替代原来的 PCF85063.c 的功能
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-07-04		first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
//=============================================================================================
//包含头文件
//=============================================================================================
#include "RTC.h"	//用于替代PCF85063.h
#include "BSP_RTC.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "DataDeal.h"
#include "pt.h"

//=============================================================================================
//全局变量
//=============================================================================================
u8	gPCF85063Readbuff[PCF85063_READBUFF_LEN] = {0};  	//PCF85063接收缓冲数组，存放时间
u8	gPCF85063Writebuff[PCF85063_WRITEBUFF_LEN] = {0};  	//PCF85063发送缓冲数组，存放写入时间
u8  gRTCWriteBuff[PCF85063_READBUFF_LEN] = {0};         //IIC写入数据buff

t_PCF85063_DATE gRdTimeAndDate;  						//读时间日期结构体
t_PCF85063_DATE gWrTimeAndDate;  						//写时间日期结构体
t_PCF85063_COND gRTCCond;                               //RTC芯片控制/状态结构体
pt gPTPCF85063;					                        //PCF85063任务pt线程控制变量

t_PCF85063_DATE DefaultInfo=	//初始值:2099/06/01/周二/12：00：00    偏离值以提示未初始化
{
    .year 			= 0x99,	    //BCD码 
    .month 			= RTC_MONTH_JUNE,
    .day 			= 1,
    .hour 			= 0x12,
    .minute 		= 0,
    .second 		= 0,
};

//=============================================================================================
//函数名称	: void PCF85063ClearCtrl(u8 ctrl)
//输入参数	: ctrl:控制位，详细定义见头文件宏定义
//输出参数	: void
//静态变量	: void
//功	能	: PCF85063设置控制标志位函数
//注	意	:
//=============================================================================================
void PCF85063SetCtrl(u8 ctrl)
{
    gRTCCond.ctrl |= ctrl;
}

//=============================================================================================
//函数名称	: void PCF85063ClearCtrl(u8 ctrl)
//输入参数	: ctrl:控制位，详细定义见头文件宏定义
//输出参数	: void
//静态变量	: void
//功	能	: PCF85063清除控制标志位函数
//注	意	:
//=============================================================================================
void PCF85063ClearCtrl(u8 ctrl)
{
    gRTCCond.ctrl &= (~ctrl);
}


//=============================================================================================
//函数名称	: u8 PCF85063Start(void)
//输入参数	: void
//输出参数	: 是否运行成功，0：运行成功，1：运行失败
//静态变量	: void
//功    能	: PCF85063 RTC正常运行
//注    意	:
//=============================================================================================
//u8 PCF85063Start(void)
//{
//	u8 temp;
//	u8 errstat = 0;

//	temp = 0x01;		//选择12.5pf电容

////	errstat = PCF85063WriteRegister(PCF85063_IIC_CHANNEL,PCF85063_CONTROL1_ADDR ,temp);

//	return errstat;
//}

//=============================================================================================
//函数名称	: u8 PCF85063Stop(void)
//输入参数	: void
//输出参数	: 是否停止成功，0：停止成功，1：停止失败
//静态变量	: void
//功    能	: PCF85063 RTC停止运行
//注    意	:
//=============================================================================================
//u8 PCF85063Stop(void)
//{
//	u8 temp;
//	u8 errstat = 0;

//	temp = 0x21;		//选择12.5pf电容
//	
////	errstat = PCF85063WriteRegister(PCF85063_IIC_CHANNEL,PCF85063_CONTROL1_ADDR ,temp);

//	return errstat;
//}

//=============================================================================================
//函数名称	: u8 PCF85063SetDate(t_PCF85063_DATE *tp)
//输入参数	: tp:写入的时间日期指针
//输出参数	: 是否写入成功，0：写入成功，1：写入失败
//静态变量	: void
//功    能	: PCF85063写入时间函数
//注    意	:
//=============================================================================================
u8 PCF85063SetDate(t_PCF85063_DATE *tp)
{
  // u8 year;
   gPCF85063Writebuff[0] = tp->second;
   gPCF85063Writebuff[1] = tp->minute;
   gPCF85063Writebuff[2] = tp->hour;
   gPCF85063Writebuff[3] = tp->day;
   gPCF85063Writebuff[4] = 0;//tp->week;
   gPCF85063Writebuff[5] = tp->month;
   gPCF85063Writebuff[6] = tp->year;
    
   return TRUE;
}

//=============================================================================================
//函数名称	: u8 PCF85063ReadDate(t_PCF85063_DATE *tp)
//输入参数	: tp:读取的时间日期指针
//输出参数	: 是否读取成功，0：读取成功，1：读取失败
//静态变量	: void
//功    能	: PCF85063读取时间函数
//注    意	:
//=============================================================================================
u8 PCF85063ReadDate(t_PCF85063_DATE *tp)
{
    //可自动增量的地址寄存器，从秒寄存器起开始读取
    //   if (PCF85063_IIC_FAULT_STAT ==
    //       PCF85063ReadNRegisters(PCF85063_IIC_CHANNEL, gPCF85063Readbuff, 7,PCF85063_SECOND_ADDR))
    //   {
    //      return FALSE;
    //   }
    //
    //   tp->second     = PCF85063_BCD_to_HEC((gPCF85063Readbuff[0] & 0x7f));
    //   tp->minute     = PCF85063_BCD_to_HEC((gPCF85063Readbuff[1] & 0x7f));
    //   tp->hour       = PCF85063_BCD_to_HEC((gPCF85063Readbuff[2] & 0x3f));
    //   tp->day        = PCF85063_BCD_to_HEC((gPCF85063Readbuff[3] & 0x3f));
    //   tp->week       = PCF85063_BCD_to_HEC((gPCF85063Readbuff[4] & 0x07));
    //   tp->month 	  = PCF85063_BCD_to_HEC((gPCF85063Readbuff[5] & 0x1f));
    //   //tp->year       = PCF85063_BCD_to_HEC(gPCF85063Readbuff[6]) + 2000;
    //   tp->year       = PCF85063_BCD_to_HEC(gPCF85063Readbuff[6]);

   tp->second     = (gPCF85063Readbuff[0] & 0x7f);
   tp->minute     = (gPCF85063Readbuff[1] & 0x7f);
   tp->hour       = (gPCF85063Readbuff[2] & 0x3f);
   tp->day        = (gPCF85063Readbuff[3] & 0x3f);
   tp->month 	  	= (gPCF85063Readbuff[5] & 0x1f);
   tp->year       = (gPCF85063Readbuff[6]);

   return TRUE;
}

//=============================================================================================
//函数名称	: void PCF85063MainTask(void *p)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: PCF85063获取或处理时间函数
//注	意	:
//=============================================================================================
void PCF85063MainTask(void *p)
{
	(void)p;
//    static u8 i = 0;
    static u8 FirstFlg = True;
    
	PT_BEGIN(&gPTPCF85063);

    if(True == FirstFlg)
    {
        FirstFlg = False;
        /*启动时代码添加*/
    }

	for(;;)
    {
        //读取时间
        if(gRTCCond.ctrl & PCF85063_START_READ)
        {
            if(RTC_GetInfo(&gRdTimeAndDate) != 0)
            {
                gRTCCond.err ++;
            }
			//判断时间是否有效	//年月未发生变化作为未校验
			if(	((gRdTimeAndDate.year  | DefaultInfo.year ) == DefaultInfo.year) && 
				((gRdTimeAndDate.month | DefaultInfo.month) == DefaultInfo.month)&&
				((gRdTimeAndDate.day   | DefaultInfo.day  ) == DefaultInfo.day) )
			{
				gRTCCond.calflag = 0;
			}
			else
			{
				gRTCCond.calflag = 1;
			}
			PCF85063ClearCtrl(PCF85063_START_READ); 
			DataDealSetBMSDataRdy(e_RTCRdy);  
        }

        //校准时间
        if(gRTCCond.ctrl & PCF85063_START_WRITE)
        {
            if((0 != gWrTimeAndDate.year) && (0 != gWrTimeAndDate.month) && (0 != gWrTimeAndDate.day))
            {
				if(RTC_SetInfo(&gWrTimeAndDate) != 0)
				{
						gRTCCond.err ++;
				}					
            }
            PCF85063ClearCtrl(PCF85063_START_WRITE);      
        }

		BSPTaskStart(TASK_ID_PCF85063_TASK, 200);	//空闲时200ms周期
		PT_NEXT(&gPTPCF85063);   
	}
    PT_END(&gPTPCF85063);
}

/*****************************************end of PCF85063.c*****************************************/
