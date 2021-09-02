//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: PCF85063.c
//创建人  	: Handry
//创建日期	: 
//描述	    : PCF85063实时时钟代码源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "PCF85063.h"
#include "BSPRTC.h"
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

//=============================================================================================
//函数定义
//=============================================================================================
//=============================================================================================
//函数名称	: u8 PCF85063Init(void)
//输入参数	: void
//输出参数	: 初始化是否成功，TURE:成功  FALSE:失败
//静态变量	: void
//功    能	: PCF85063初始化函数
//注    意	:
//=============================================================================================
u8 PCF85063Init(void)
{
	//初始化数据
    PT_INIT(&gPTPCF85063);	        //初始化通信任务pt线程控制变量
	gRTCCond.calflag = 0;			//0:未校准；1：校准
	gRTCCond.ctrl = 0;				//RTC控制字节，控制读取时间和校准时间
	gRTCCond.err = 0;				//RTC通信错误计数器
	gRTCCond.iicres = 0;		    //阻塞时使用变量，用于等待IIC通信的结果
    gRTCCond.rtcchn = BSPIICChannalReq(IIC_ROUTE_RTC);   //获取RTC的IIC通道
	/*
	PCF85063Stop();
	gWrTimeAndDate.year     = 0x19;
	gWrTimeAndDate.month    = 0x11;
	gWrTimeAndDate.day      = 0x25;
	//gRdTimeAndDate.week     = 0x01;
	gWrTimeAndDate.hour     = 0x10;
	gWrTimeAndDate.minute   = 0x30;
	gWrTimeAndDate.second   = 00;
	PCF85063SetDate(&gWrTimeAndDate);
    */
	//PCF85063Start();
	return TRUE;
}

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
//函数名称	: u8 PCF85063ReadNRegisters(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr)
//输入参数	: num:eIIC0/eIIC1	*p_buffer:接收缓冲区		bufflen:接收的数据个数	regaddr:PCF85063寄存器地址
//输出参数	: IIC通信错误信息
//静态变量	: void
//功	能	: PCF85063读取信息函数
//注	意	:
//=============================================================================================
u8 PCF85063ReadNRegisters(e_IICNUM num,u8* p_buffer, u8 bufflen, u8 regaddr)
{
	u8 errstat = 0;	//IIC通信错误信息
	u8 regaddress = 0;

	if((num > eIIC1) || (p_buffer == NULL))  //参数有效性判断
	{
		return PCF85063_IIC_FAULT_STAT;
	}
    
	regaddress = (u8)regaddr;
    gRTCWriteBuff[0] = regaddress;
    BSPIICWriteAndRead(gRTCCond.rtcchn,PCF85063_WRITE_ADDR,&gRTCWriteBuff[0],1,p_buffer,bufflen);
	//errstat = BSPIICRcvMsg(num,PCF85063_WRITE_ADDR,regaddress,p_buffer,bufflen,0);	//发送读取报文，标准地址
    
	return errstat;
}

 //=============================================================================================
 //函数名称	:  u8 PCF85063_WriteRegister(e_IICNUM num,u8 regaddr ,u8 byte )
 //输入参数	: num:eIIC0/eIIC1	regaddr:PCF85063寄存器地址	byte:写入的数值
 //输出参数	: void
 //静态变量	: void
 //功	能	: PCF85063写1个寄存器函数
 //注	意	:
 //=============================================================================================
 u8 PCF85063WriteRegister(e_IICNUM num, u8 regaddr, u8 byte)
{
	u8 errstat = 0;									//IIC通信错误信息
	u8 regaddress = 0;
	u8 deviceaddr = 0;
    u8 writebuff[3] = {0};

	if((num > eIIC1))  //参数有效性判断
	{
		return PCF85063_IIC_FAULT_STAT;
	}

	deviceaddr = PCF85063_WRITE_ADDR;
    regaddress = (u8)(regaddr);

	writebuff[0] = deviceaddr;
	writebuff[1] = regaddress;
	writebuff[2] = byte;

    errstat = BSPIICSendMsg(num,writebuff,3,1);

	 return errstat;
}

 //=============================================================================================
 //函数名称	: u8 PCF85063WriteRegister(e_IICNUM num,u8* p_buffer, u8 bufflen, u8 regaddr)
 //输入参数	: num:eIIC0/eIICMax	*p_buffer:发送缓冲区		bufflen:发送的数据个数		regaddr:PCF85063寄存器地址
 //输出参数	: void
 //静态变量	: void
 //功	能	: PCF85063写n个寄存器函数
 //注	意	:
 //=============================================================================================
u8 PCF85063WriteNRegisters(e_IICNUM num,u8* p_buffer, u8 bufflen, u8 regaddr)
{
	u8 writelen = 0;
	u8 errstat = 0;
	u8 i = 0;
    
	writelen = bufflen;
    
	gRTCWriteBuff[0] = PCF85063_WRITE_ADDR;
	gRTCWriteBuff[1] = regaddr;


	for(i = 0; i < writelen;i++)
	{
        gRTCWriteBuff[2 + i] = p_buffer[i];
	}     
    
    BSPIICWrite(gRTCCond.rtcchn,gRTCWriteBuff[0],&gRTCWriteBuff[1],bufflen+1);
    
	return errstat;
}

//=============================================================================================
//函数名称	: u8 PCF85063Start(void)
//输入参数	: void
//输出参数	: 是否运行成功，0：运行成功，1：运行失败
//静态变量	: void
//功    能	: PCF85063 RTC正常运行
//注    意	:
//=============================================================================================
u8 PCF85063Start(void)
{
	u8 temp;
	u8 errstat = 0;

	temp = 0x01;		//选择12.5pf电容

	errstat = PCF85063WriteRegister(PCF85063_IIC_CHANNEL,PCF85063_CONTROL1_ADDR ,temp);

	return errstat;
}

//=============================================================================================
//函数名称	: u8 PCF85063Stop(void)
//输入参数	: void
//输出参数	: 是否停止成功，0：停止成功，1：停止失败
//静态变量	: void
//功    能	: PCF85063 RTC停止运行
//注    意	:
//=============================================================================================
u8 PCF85063Stop(void)
{
	u8 temp;
	u8 errstat = 0;

	temp = 0x21;		//选择12.5pf电容
	
	errstat = PCF85063WriteRegister(PCF85063_IIC_CHANNEL,PCF85063_CONTROL1_ADDR ,temp);

	return errstat;
}

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
   tp->month 	  = (gPCF85063Readbuff[5] & 0x1f);
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
	#if defined(USE_PCF85063)
    static u8 FirstFlg = True;
	#endif
    
	PT_BEGIN(&gPTPCF85063);
	
	#if defined(USE_PCF85063)
    if(True == FirstFlg)
    {
        FirstFlg = False;
        //启动RTC 选择12.5pf电容
        gPCF85063Writebuff[0] = 0x01;
        gRTCCond.iicres = PCF85063WriteNRegisters(PCF85063_IIC_CHANNEL,gPCF85063Writebuff,1,PCF85063_CONTROL1_ADDR);   
        
        PT_WAIT_UNTIL(&gPTPCF85063,BSPTaskStart(TASK_ID_PCF85063_TASK, 2) && (BSPIICChannalStateGet(gRTCCond.rtcchn) <= 0));
        
        if((gRTCCond.iicres > 0) || (BSPIICChannalStateGet(gRTCCond.rtcchn) < 0))
        {
            gRTCCond.err ++;
        }       
    }
	#endif
	
	for(;;)
    {
        //读取时间
        if(gRTCCond.ctrl & PCF85063_START_READ)
        {
            gRTCCond.iicres = PCF85063ReadNRegisters(PCF85063_IIC_CHANNEL, gPCF85063Readbuff, 7,PCF85063_SECOND_ADDR);

            PT_WAIT_UNTIL(&gPTPCF85063,BSPTaskStart(TASK_ID_PCF85063_TASK, 2) && (BSPIICChannalStateGet(gRTCCond.rtcchn) <= 0));
            
            if((gRTCCond.iicres > 0) || (BSPIICChannalStateGet(gRTCCond.rtcchn) < 0))
            {
                gRTCCond.err ++;
            }
            else
            {
                PCF85063ReadDate(&gRdTimeAndDate);
                
                if(PCF85063_OS_MASK != (PCF85063_OS_MASK & gPCF85063Readbuff[0]))   //已经校准
                {
                    gRTCCond.calflag = 1;
                }
                else
                {
                    //判断时间是否有效
                    if(((gRdTimeAndDate.year & 0x20) == 0x20) && ((gRdTimeAndDate.month&0x1f) <= 0x12) 
                        && ((gRdTimeAndDate.month&0x0f) <= 0x09)
                    )
                    {
                        gRTCCond.calflag = 1;
                    }
                    else
                    {
                        gRTCCond.calflag = 0;
                    }
                }

                gRTCCond.err = 0;


            }
            
            PCF85063ClearCtrl(PCF85063_START_READ);

            DataDealSetBMSDataRdy(e_RTCRdy);
        }

        //校准时间
        if(gRTCCond.ctrl & PCF85063_START_WRITE)
        {
            if((0 != gWrTimeAndDate.year) && (0 != gWrTimeAndDate.month) && (0 != gWrTimeAndDate.day))
            {
				#if defined(USE_PCF85063)
                //停止RTC
                gPCF85063Writebuff[0] = 0x21;
                gRTCCond.iicres = PCF85063WriteNRegisters(PCF85063_IIC_CHANNEL,gPCF85063Writebuff,1,PCF85063_CONTROL1_ADDR);   
                
                PT_WAIT_UNTIL(&gPTPCF85063,BSPTaskStart(TASK_ID_PCF85063_TASK, 2) && (BSPIICChannalStateGet(gRTCCond.rtcchn) <= 0));
                
                if((gRTCCond.iicres > 0) || (BSPIICChannalStateGet(gRTCCond.rtcchn) < 0))
                {
                    gRTCCond.err ++;
                }                
            	#endif    
                PCF85063SetDate(&gWrTimeAndDate);	//标定时间

                gRTCCond.iicres = PCF85063WriteNRegisters(PCF85063_IIC_CHANNEL,gPCF85063Writebuff,7,PCF85063_SECOND_ADDR);   
                
                PT_WAIT_UNTIL(&gPTPCF85063,BSPTaskStart(TASK_ID_PCF85063_TASK, 2) && (BSPIICChannalStateGet(gRTCCond.rtcchn) <= 0));
                
                if((gRTCCond.iicres > 0) || (BSPIICChannalStateGet(gRTCCond.rtcchn) < 0))
                {
                    gRTCCond.err ++;
                }  
                #if defined(USE_PCF85063)
                //启动RTC 选择12.5pf电容
                gPCF85063Writebuff[0] = 0x01;
                gRTCCond.iicres = PCF85063WriteNRegisters(PCF85063_IIC_CHANNEL,gPCF85063Writebuff,1,PCF85063_CONTROL1_ADDR);   
                
                PT_WAIT_UNTIL(&gPTPCF85063,BSPTaskStart(TASK_ID_PCF85063_TASK, 2) && (BSPIICChannalStateGet(gRTCCond.rtcchn) <= 0));
                
                if((gRTCCond.iicres > 0) || (BSPIICChannalStateGet(gRTCCond.rtcchn) < 0))
                {
                    gRTCCond.err ++;
                }
				#endif                 
            }

            PCF85063ClearCtrl(PCF85063_START_WRITE);
      
        }

    BSPTaskStart(TASK_ID_PCF85063_TASK, 200);	//空闲时200ms周期
    PT_NEXT(&gPTPCF85063);
        
	}
    PT_END(&gPTPCF85063);
}

/*****************************************end of PCF85063.c*****************************************/
