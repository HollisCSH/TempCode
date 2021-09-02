//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: FM11NC08.c
//创建人  	: Handry
//创建日期	: 
//描述	    : FM11NC08 NFC代码源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "FM11NC08.h"
#include "BSPGPIO.h"
#include "IOCheck.h"
#include "BSPSysTick.h"
#include "BSPCriticalZoom.h"
#include "BSPTimer.h"
#include "BSPTaskDef.h"
#include "BSPTask.h"
#include "BSPTimer.h"
#include "Comm.h"
#include "stdint.h"
#include "pt.h"

//=============================================================================================
//全局变量
//=============================================================================================
//FM11NC08配置数组
static const u8 cFM11NC08CFG[16] =
{
	0x05,0x78,0x01,0x57,0xF7,0xA0,0x02,0x00,
	0x00,0x00,0xA2,0xB2,0x00,0x00,0x00,0x00
};

u8 gNFCReadData[16] = {0};					    //NFC读EEP数组
static u8 sNFCTxOverTime = 0;				    //NFC发送超时时间
static u32 sTxOverTimer = 0;				    //NFC发送超时时间缓存
static u8 sNFCPowerOn = 0;					    //是否重新上电标志
static u8 sNFCTaskFlag = 0;                     //NFC关机标志
static u8 sNFCRxST = 0;				            //重新接收标志
//static e_NFCStat sNFCStat = eNFCStat_Init;	//定义NFC模块状态
static u16 sNFCRXPtr = 0,sNFCTXPtr = 0;	        //NFC 发送，接收位置
t_NFC_MSG *gNFCRxMsg,*gNFCTxMsg;			    //定义NFC发送，接收信息指针
pt gPTNFC;									    //NFC任务pt线程控制变量

//=============================================================================================
//静态函数声明
//=============================================================================================

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: u8 FM11NC08Init(void)
//输入参数	: void
//输出参数	: 初始化是否成功，TURE:成功  FALSE:失败
//静态变量	: void
//功    能	: FM11NC08初始化函数
//注    意	:
//=============================================================================================
u8 FM11NC08Init(void)
{
	u8 result = 0;
	u8 i = 0;

	PT_INIT(&gPTNFC);	//PT线程控制初始化

//	sNFCStat = eNFCStat_Init;

	FM11NC08CSLow();    //片选拉低，使能，唤醒NFC

	BSPSysDelay1ns(200);

	FM11NC08WriteEEPROM(NFC_CFG_EEPROM,cFM11NC08CFG,16);

	BSPSysDelay1ms(20);

	FM11NC08ReadEEPROM(NFC_CFG_EEPROM,gNFCReadData,16);

	for(i = 0;i < 16;i++)
	{
		if(gNFCReadData[i] != cFM11NC08CFG[i])
		{
			result++;
		}
	}

	//清零中断标志位，读清除
	FM11NC08ReadReg(NFC_MAIN_IRQ);
    FM11NC08ReadReg(NFC_FIFO_IRQ);		
	FM11NC08ReadReg(NFC_AUX_IRQ);
	
    gNFCTxMsg = NULL;
    gNFCRxMsg = NULL;		
	/**************test 测试读写寄存器 开始**************/
	/*
	FM11NC08CSLow();    //片选拉低，使能，唤醒NFC

	FM11NC08WriteReg(0x00,0);

	FM11NC08CSLow();    //片选拉低，使能，唤醒NFC

	readreg = FM11NC08ReadReg(0);

	FM11NC08CSHigh();    //片选拉高，失能
	*/
	/**************test 测试读写寄存器 结束**************/

	return result;
}

//=============================================================================================
//函数名称	: u8 FM11NC08ReadReg(u8 addr)
//输入参数	: addr:读取的地址
//输出参数	: 读取的寄存器的值
//静态变量	: void
//功    能	: FM11NC08读寄存器函数
//注    意	:
//=============================================================================================
u8 FM11NC08ReadReg(u8 addr)
{
	u8 data = 0;

    BSPGPIOIntDisable(NFC_INT_MASK);	//NFC中断输出引脚关中断

	FM11NC08CSLow();    //片选拉低，使能

	BSPSPISendOneData(NFC_SPI_CHN,(addr & 0x0f) | 0x20);
	data = BSPSPIRcvOneData(NFC_SPI_CHN);

    FM11NC08CSHigh();    //片选拉高，失能

    BSPGPIOIntEnable(NFC_INT_MASK);		//NFC中断输出引脚开中断
    return data;
}

//=============================================================================================
//函数名称	: void FM11NC08WriteReg(u8 addr, u8 data)
//输入参数	: addr:写入的地址	data:写入的数据
//输出参数	: void
//静态变量	: void
//功    能	: FM11NC08写寄存器函数
//注    意	:
//=============================================================================================
void FM11NC08WriteReg(u8 addr, u8 data)
{
    BSPGPIOIntDisable(NFC_INT_MASK);	//NFC中断输出引脚关中断

	FM11NC08CSLow();    //片选拉低，使能

	BSPSPISendOneData(NFC_SPI_CHN,(addr & 0x0f));
	BSPSPISendOneData(NFC_SPI_CHN,data);

    FM11NC08CSHigh();    //片选拉高，失能

    BSPGPIOIntEnable(NFC_INT_MASK);		//NFC中断输出引脚开中断
}

//=============================================================================================
//函数名称	: void FM11NC08ReadEEPROM(u16 addr, u8 *data , u8 len)
//输入参数	: addr:读取的地址  *data:读取放入的数组指针	len：读取的长度
//输出参数	: 读取的EEPROM
//静态变量	: void
//功    能	: FM11NC08读取的EEPROM函数
//注    意	:
//=============================================================================================
void FM11NC08ReadEEPROM(u16 addr, u8 *data , u8 len)
{
    u8 mode;

    BSPGPIOIntDisable(NFC_INT_MASK);	//NFC中断输出引脚关中断
    mode = 0x60 | ((addr >> 8) & 0x03);

    FM11NC08CSLow();    //片选拉低，使能

    BSPSysDelay1ns(100);

    BSPSPISendOneData(NFC_SPI_CHN,mode);
    BSPSPISendOneData(NFC_SPI_CHN,addr & 0xff);
	BSPSPIRecNDatas(NFC_SPI_CHN,data,len);

    FM11NC08CSHigh();    //片选拉高，失能

    BSPGPIOIntEnable(NFC_INT_MASK);		//NFC中断输出引脚开中断
}

//=============================================================================================
//函数名称	: void FM11NC08WriteEEPROM(u16 addr, u8 const *data , u8 len)
//输入参数	: addr:写入的地址  *data:写入放入的数组指针	len：写入的长度
//输出参数	: 写入的EEPROM
//静态变量	: void
//功    能	: FM11NC08写入的EEPROM函数
//注    意	:
//=============================================================================================
void FM11NC08WriteEEPROM(u16 addr, u8 const *data , u8 len)
{
    u8 mode;
    u8 *pdata = (u8 *)data;
    //参数有效性判断
    if(len > NFC_WRITE_EEP_MAX_LEN)
    {
        return;
    }

    mode = 0x40 | ((addr >> 8) & 0x03);
    BSPGPIOIntDisable(NFC_INT_MASK);		//NFC中断输出引脚关中断
    FM11NC08CSLow();    //片选拉低，使能

    BSPSPISendOneData(NFC_SPI_CHN,0xCE);	//写使能序列
    BSPSPISendOneData(NFC_SPI_CHN,0x55);

    FM11NC08CSHigh();    //片选拉高，失能

    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");

    FM11NC08CSLow();    //片选拉低，使能

    BSPSPISendOneData(NFC_SPI_CHN,mode);
    BSPSPISendOneData(NFC_SPI_CHN,addr & 0xff);
    BSPSPISendNDatas(NFC_SPI_CHN,pdata,len);

    FM11NC08CSHigh();    //片选拉高，失能
    BSPGPIOIntEnable(NFC_INT_MASK);		//NFC中断输出引脚开中断
}

//=============================================================================================
//函数名称	: void FM11NC08ReadFIFO(u8 *data,u8 len)
//输入参数	: *data:读取放入的数组指针    len:读取的长度
//输出参数	: void
//静态变量	: void
//功    能	: FM11NC08读取FIFO函数
//注    意	:
//=============================================================================================
void FM11NC08ReadFIFO(u8 *data,u8 len)
{
	BSPGPIOIntDisable(NFC_INT_MASK);		//NFC中断输出引脚关中断
	FM11NC08CSLow();    					//片选拉低，使能

	BSPSPISendOneData(NFC_SPI_CHN,0xA0);
	BSPSPIRecNDatas(NFC_SPI_CHN,data,len);

    FM11NC08CSHigh();    //片选拉高，失能
    BSPGPIOIntEnable(NFC_INT_MASK);		//NFC中断输出引脚开中断

}

//=============================================================================================
//函数名称	: void FM11NC08WriteFIFO(u8 *data,u8 len)
//输入参数	: *data:写入数组指针    len:写入的长度
//输出参数	: void
//静态变量	: void
//功    能	: FM11NC08写入FIFO函数
//注    意	:
//=============================================================================================
void FM11NC08WriteFIFO(u8 *data,u8 len)
{
	BSPGPIOIntDisable(NFC_INT_MASK);		//NFC中断输出引脚关中断
	FM11NC08CSLow();    					//片选拉低，使能

	BSPSPISendOneData(NFC_SPI_CHN,0x80);
	BSPSPISendNDatas(NFC_SPI_CHN,data,len);

    FM11NC08CSHigh();    //片选拉高，失能
    BSPGPIOIntEnable(NFC_INT_MASK);		//NFC中断输出引脚开中断
}

//=============================================================================================
//函数名称	: e_NFCStat FM11NC08RFStatusCheck(void)
//输入参数	: void
//输出参数	: NFC RF状态
//静态变量	: void
//功    能	: FM11NC08读取NFC RF状态
//注    意	:
//=============================================================================================
e_NFCStat FM11NC08RFStatusCheck(void)
{
    u8 rfstatus = (FM11NC08ReadReg(NFC_RF_STATUS) & NFC_RFSTA_MASK) >> NFC_RFSTA_SHIFT;

    if(rfstatus == NFC_RFSTA_L4)
    {
        return eNFCStat_RX;

    }
    return eNFCStat_Nolink;
}

//=============================================================================================
//函数名称	: u8 FM11NC08FrameRx(t_NFC_MSG * msg)
//输入参数	: msg：要接收放置的地址
//输出参数	: 是否接收成功，FALSE:失败；TRUE：成功
//静态变量	: void
//功    能	: FM11NC08接收帧
//注    意	:
//=============================================================================================
u8 FM11NC08FrameRx(t_NFC_MSG * msg)
{
	BSPEnterCritical();

    msg->sta = 0;

    sNFCRxST = 0;

    gNFCRxMsg = msg;

    BSPExitCritical();

    return TRUE;
}

//=============================================================================================
//函数名称	: u8 FM11NC08FrameTx(t_NFC_MSG * msg, uint16_t overtime)
//输入参数	: msg：要发送的数据   overtime：超时时间
//输出参数	: 是否发送成功，FALSE:失败；TRUE：成功
//静态变量	: void
//功    能	: FM11NC08发送帧
//注    意	:
//=============================================================================================
u8 FM11NC08FrameTx(t_NFC_MSG * msg, uint16_t overtime)
{
    if(FM11NC08RFStatusCheck() == eNFCStat_Nolink)
    {
        msg->sta = 2;
        return FALSE;
    }

    if(NULL != gNFCTxMsg)
    {
        msg->sta = 3;
        return FALSE;
    }

    //BSPEnterCritical();
    
    sTxOverTimer = gTimer1ms;

//    if(!BSPTimerAllocation(&sNFCTxOverTime))    /* 申请定时器成功*/
//    {
//        BSPTimerConfig(sNFCTxOverTime,overtime,0,NULL);
//    }
//    else    /* 申请定时器失败*/
//    {
//        sNFCTxOverTime = 0xFF;
//        gNFCTxMsg = NULL;
//        return FALSE;
//    }

    gNFCTxMsg = msg;
    gNFCTxMsg ->sta = 0;
    sNFCTXPtr = 0;

    FM11NC08WriteReg(NFC_FIFO_FLUSH,0xff);		//清空FIFO

    if(32 <= gNFCTxMsg-> cnt)
    {
    	FM11NC08WriteFIFO(gNFCTxMsg -> msg, 32);
        sNFCTXPtr += 32;
        FM11NC08WriteReg(NFC_FIFO_IRQ_MASK,	0xf3);		//屏蔽FIFO满中断，空中断，溢出中断
        FM11NC08WriteReg(NFC_MAIN_IRQ_MASK,	0x44);		//
    }
    else
    {
    	FM11NC08WriteFIFO(gNFCTxMsg->msg, gNFCTxMsg->cnt);
    	sNFCTXPtr += gNFCTxMsg -> cnt;
        FM11NC08WriteReg(NFC_FIFO_IRQ_MASK,	0xf3);		//屏蔽FIFO满中断，空中断，溢出中断
        FM11NC08WriteReg(NFC_MAIN_IRQ_MASK,	0x44);		//
    }

    //BSPExitCritical();

    FM11NC08WriteReg(NFC_RF_TXEN,0x55);			//发送命令

    return TRUE;
}

//=============================================================================================
//函数名称	: void FM11NC08INTCallBackFunc(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: FM11NC08 int引脚中断回调函数
//注    意	:
//=============================================================================================
void FM11NC08INTCallBackFunc(void)
{
	//u8 aux_irq = 0;		//辅助中断标志寄存器值
    u8 fifo_irq = 0;	//fifo中断标志寄存器值
    u8 main_irq;		//主中断标志寄存器值
    u16 cnt;

    main_irq = FM11NC08ReadReg(NFC_MAIN_IRQ);

    if(NFC_MAINIRQ_FIFO_MASK == (main_irq & NFC_MAINIRQ_FIFO_MASK))
    {
        fifo_irq = FM11NC08ReadReg(NFC_FIFO_IRQ);
    }

    //if(NFC_MAINIRQ_AUX_MASK == (main_irq & NFC_MAINIRQ_AUX_MASK))
    //{
    //    aux_irq = FM11NC08ReadReg(NFC_AUX_IRQ);
    //}

    if(NFC_MAINIRQ_PWON_MASK == (main_irq & NFC_MAINIRQ_PWON_MASK))//用作上电中断
    {
    	//休眠唤醒处理
    	sNFCRxST = 0;//重新初始化接收状态
        sNFCPowerOn = 1; //置位在任务中初始化寄存器
        gNFCTxMsg = NULL;
        gNFCRxMsg = NULL;
        CommInit();
		//IOCheckNFCIntCallback();
        //if(NFC_WK_CB!=NULL)
        //(*NFC_WK_CB)();
    }  
    
    /**************发送数据**************/
    if(NULL  != gNFCTxMsg)		//有发送请求,发送数组不为空
    {        
        if(NFC_FIFOIRQ_WL_MASK == (fifo_irq & NFC_FIFOIRQ_WL_MASK))	    //发送渐空中断发生
        {
            cnt = gNFCTxMsg->cnt - sNFCTXPtr;

            if(cnt >= 24)													//FIFO至少有24字节空间
            {
            	FM11NC08WriteFIFO(gNFCTxMsg->msg + sNFCTXPtr,24);
                sNFCTXPtr += 24;
            }
            else if(cnt>0)
            {
            	FM11NC08WriteFIFO(gNFCTxMsg->msg + sNFCTXPtr,cnt);
                sNFCTXPtr += cnt;
            }

        }

        if(NFC_MAINIRQ_TXDN_MASK == (main_irq & NFC_MAINIRQ_TXDN_MASK))	//发送完成中断发生
        {
            if(sNFCTXPtr == gNFCTxMsg->cnt)
            {
            	gNFCTxMsg->sta = 1;
            }
            else
            {
            	gNFCTxMsg->sta = 2;
            }

            gNFCTxMsg = NULL;		//发送完毕，清空发送信息结构体
            sNFCRxST = 0;			//初始化接收状态
            BSPTimerRelease(sNFCTxOverTime);	//释放超时时间定时器
        }
    }
    /**************接收数据**************/
    else if(NULL != gNFCRxMsg)					//有接收任务
    {
        if(main_irq&NFC_MAINIRQ_RXST_MASK)		//开始接收数据
        {
        	sNFCRxST = 1;
        	sNFCRXPtr = 0;
        }

        if(sNFCRxST == 1)
        {
        	//接收帧错误/CRC错误/校验错误
        	if(NFC_MAINIRQ_AUX_MASK == (main_irq & NFC_MAINIRQ_AUX_MASK))
        	{
        		sNFCRxST = 0;
        	     FM11NC08WriteReg(NFC_FIFO_FLUSH,1);	//清空FIFO缓存
        	}
        	else
        	{
        		//接收渐满
                if(NFC_FIFOIRQ_WL_MASK == (fifo_irq & NFC_FIFOIRQ_WL_MASK))
                {
                    if(gNFCRxMsg->cnt >= (sNFCRXPtr + 22))
                    {
                    	FM11NC08ReadFIFO(gNFCRxMsg->msg + sNFCRXPtr,22);
                        sNFCRXPtr += 22;
                    }
                    else
                    {
                        sNFCRxST = 0;
                        FM11NC08WriteReg(NFC_FIFO_FLUSH,1);	//清空FIFO缓存
                    }
                }

                //已经全部接收完毕，读出FIFO剩下的数据
                if(main_irq&NFC_MAINIRQ_RXDN_MASK)
                {

                    cnt = FM11NC08ReadReg(NFC_FIFO_WCNT);	//读出FIFO剩余字节数

                    if(gNFCRxMsg->cnt >= (sNFCRXPtr + cnt))
                    {
                        if(0 != cnt)
                        {
                        	FM11NC08ReadFIFO(gNFCRxMsg->msg + sNFCRXPtr,cnt);
                        }

                        FM11NC08WriteReg(NFC_FIFO_FLUSH,1);	//清空FIFO缓存
                        gNFCRxMsg->cnt = sNFCRXPtr+cnt;
                        gNFCRxMsg->sta = 1;
                        sNFCRxST=0;
                        gNFCRxMsg = NULL;
                    }
                    else
                    {
                        sNFCRxST = 0;
                        FM11NC08WriteReg(NFC_FIFO_FLUSH,1);	//清空FIFO缓存
                    }
                }

        	}
        }

    }
    else
    {
    	FM11NC08WriteReg(NFC_FIFO_FLUSH,1);	//清空FIFO缓存
    }

}

//=============================================================================================
//函数名称	: void FM11NC08Disable(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: FM11NC08关闭函数
//注	意	:
//=============================================================================================
void FM11NC08Disable(void)
{
	sNFCTaskFlag = 1;
}

//=============================================================================================
//函数名称	: u8 FM11NC08Disable(void)
//输入参数	: void
//输出参数	: 1:关闭完成；0：关闭失败
//静态变量	: void
//功	能	: FM11NC08关闭完成判断函数
//注	意	:
//=============================================================================================
u8 FM11NC08DisableComplete(void)
{
	return sNFCTaskFlag == 0;
}

//=============================================================================================
//函数名称	: void FM11NC08PowerOn(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: FM11NC08上电使能
//注	意	:
//=============================================================================================
void FM11NC08PowerOn(void)
{
	sNFCPowerOn = 1;
}

//=============================================================================================
//函数名称	: void FM11NC08MainTask(void *p)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: NFC主任务函数
//注    意	:
//=============================================================================================
void FM11NC08MainTask(void *p)
{
	(void)p;

    PT_BEGIN(&gPTNFC);
    for(;;)
    {
    	//BSPEnterCritical();

        if(NULL  != gNFCTxMsg)		//有发送请求,发送数组不为空
        {
//            if(0xFF != sNFCTxOverTime)  /* 申请定时器成功 */
//            {
//                if(BSPTimerCheck(sNFCTxOverTime))		//判断超时时间
//                {
//                    BSPTimerRelease(sNFCTxOverTime);
//                    gNFCTxMsg->sta = 4;
//                    gNFCTxMsg = NULL;
//                }
//            }
//            else      /* 申请定时器失败 */
//            {
//            
//            }
            if(gTimer1ms - sTxOverTimer > 100)
            {
                gNFCTxMsg->sta = 4;
                gNFCTxMsg = NULL;            
            }
            
        }

    	if(0 == BSPGPIOGetPin(NFC_INT_PORT,NFC_INT_PIN))	//中断已触发
    	{
    		//NVIC_SetPendingIRQ(KBI1_IRQn);
    	}
        
        //BSPExitCritical();

    	if(1 == sNFCPowerOn)		//重新上电
    	{
    		sNFCPowerOn = 0;

            FM11NC08CSLow();		//唤醒NFC
            BSPSysDelay1ns(100);

            FM11NC08WriteReg(NFC_FIFO_IRQ_MASK,	0x07);		//屏蔽FIFO满中断，空中断，溢出中断
            FM11NC08WriteReg(NFC_AUX_IRQ_MASK,	0xC0);		//
			FM11NC08WriteReg(NFC_FIFO_FLUSH,	0xff);		//清除FIFO缓存，标志
			
			//清零中断标志位，读清除
			FM11NC08ReadReg(NFC_MAIN_IRQ);
			FM11NC08ReadReg(NFC_FIFO_IRQ);		
			FM11NC08ReadReg(NFC_AUX_IRQ);	
    	}
		
		if(1 == sNFCTaskFlag)
		{
			//清零中断标志位，读清除
            if(0 == BSPGPIOGetPin(NFC_INT_PORT,NFC_INT_PIN))
            {
                FM11NC08ReadReg(NFC_MAIN_IRQ);
                FM11NC08ReadReg(NFC_FIFO_IRQ);		
                FM11NC08ReadReg(NFC_AUX_IRQ);
                FM11NC08ReadReg(NFC_MAIN_IRQ_MASK);
                FM11NC08ReadReg(NFC_FIFO_IRQ_MASK);		
                FM11NC08ReadReg(NFC_AUX_IRQ_MASK);         
            }                
            sNFCTaskFlag = 0;
		}
		
    	BSPTaskStart(TASK_ID_NFC_TASK, 5);
        PT_NEXT(&gPTNFC);
    }
    PT_END(&gPTNFC);
}

/*****************************************end of FM11NC08.c*****************************************/
