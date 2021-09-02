//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名		: FM11NC08.c
//创建人		: Handry
//创建日期	:
//描述	    : FM11NC08 NFC代码源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人		:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "FM11NC08.h"
#include "BSPGPIO.h"
#include "BSPSysTick.h"
#include "BSPTimer.h"
#include "BSPWatchDog.h"
#include "stdint.h"
#include "pt.h"

//=============================================================================================
//全局变量
//=============================================================================================
//FM11NC08配置数组
//static const u8 cFM11NC08CFG[16] =
//{
//	0x05,0x78,0x01,0x57,0xF7,0xA0,0x02,0x00,
//	0x00,0x00,0xA2,0xB2,0x00,0x00,0x00,0x00
//};

uint8_t cFM11NC08CFG[]=
{
	0x05, 0x72, 0x01, 0x57, 0xF7, 0x60, 0x02, 0x00,
    0x00, 0x00, 0xA2, 0xB2, 0x00, 0x00, 0x00, 0x00
};

static uint8_t c_NFC_HEADSTR[] = {0x02,0x00,0x00,0x01};
uint8_t c_FUNCODE = 103;
static uint8_t NFC_BUF[259];
static uint8_t MB_EXP=0;
u8 gNFCflag = 0;        //NFC中断FLAG

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

	FM11NC08CSLow();    //片选拉低，使能，唤醒NFC

	BSPSysDelay1ns(200);

	FM11NC08WriteEEPROM(NFC_CFG_EEPROM,cFM11NC08CFG,16);

	BSPSysDelay1ms(20);

	//清零中断标志位，读清除
	FM11NC08ReadReg(NFC_MAIN_IRQ);
    FM11NC08ReadReg(NFC_FIFO_IRQ);		
	FM11NC08ReadReg(NFC_AUX_IRQ);	

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
//函数名称	: u8 FM11NC08FrameTx(uint8_t fcode, uint8_t const * buf, uint16_t cnt, uint32_t timeout)
//输入参数	: fcode：功能码；buf：要发送的数据；timeout：超时时间
//输出参数	: 是否发送成功，FALSE:失败；TRUE：成功
//静态变量	: void
//功    能	: FM11NC08发送帧
//注    意	:
//=============================================================================================
u8 FM11NC08FrameTx(uint8_t fcode, uint8_t const * buf, uint16_t cnt, uint32_t timeout)
{
    uint8_t main_irq;//,aux_irq,fifo_irq;
    uint8_t fifocnt,i;
    uint16_t point=0;
    uint32_t timeoutst;
    for(i=0;i<4;i++)
    {
        NFC_BUF[i] = c_NFC_HEADSTR[i];
    }
    NFC_BUF[4] = fcode;
    StringCopy(NFC_BUF+5,buf,cnt);
    cnt = cnt+5;
    CommCalcCRC(COMM_CRC_A,NFC_BUF,cnt,NFC_BUF+cnt,NFC_BUF+cnt+1);//校验NFC CRC
    cnt = cnt+2;
    //设置超时
    timeoutst = gTimer1ms;
    FM11NC08WriteReg(NFC_FIFO_FLUSH,0xff);
    main_irq = FM11NC08ReadReg(NFC_MAIN_IRQ);
    FM11NC08ReadReg(NFC_AUX_IRQ);
    FM11NC08ReadReg(NFC_FIFO_IRQ);
    while(TIMEPASS(timeoutst)<timeout)
    {
        
        if(cnt>0)
        {
            
            fifocnt = FM11NC08ReadReg(NFC_FIFO_WCNT);
            if(fifocnt<=10)
            {
                
                
                fifocnt = 32-fifocnt;
                if(cnt>fifocnt)
                {
                    FM11NC08WriteFIFO(NFC_BUF+point, fifocnt);
                    point += fifocnt;
                    cnt -= fifocnt;
                    FM11NC08WriteReg(NFC_RF_TXEN,0x55);

                }
                else 
                {
                    if(cnt>0)
                    {
                        
                        FM11NC08WriteFIFO(NFC_BUF+point, cnt);
                        FM11NC08WriteReg(NFC_RF_TXEN,0x55);//
                        cnt = 0;
                    }
                }
                
            }
        }
        main_irq = FM11NC08ReadReg(NFC_MAIN_IRQ);
        FM11NC08ReadReg(NFC_AUX_IRQ);
        FM11NC08ReadReg(NFC_FIFO_IRQ);
        if(main_irq&NFC_MAINIRQ_TXDN_MASK)
        {
            FM11NC08WriteReg(NFC_FIFO_FLUSH,0xff);
            return TRUE;
        }
        if(main_irq&NFC_MAINIRQ_AUX_MASK)
        {
            FM11NC08WriteReg(NFC_FIFO_FLUSH,0xff);
            return FALSE;
        }
    }
    FM11NC08WriteReg(NFC_FIFO_FLUSH,0xff);
    return FALSE;
}

//=============================================================================================
//函数名称	: u8 FM11NC08FrameRx(uint8_t * buf, uint16_t * cnt, uint32_t timeout)
//输入参数	: buf：要接收放置的地址；cnt：接收的长度指针；timeout：超时时间
//输出参数	: 是否接收成功，FALSE:失败；TRUE：成功
//静态变量	: void
//功    能	: FM11NC08接收帧
//注    意	:
//=============================================================================================
u8 FM11NC08FrameRx(uint8_t * buf, uint16_t * cnt, uint32_t timeout)
{
    static u32 feedtime = 0;
    uint8_t main_irq,sta,fifocnt,i,err,crc1,crc2;//,l4flag=1;aux_irq,fifo_irq,
    uint16_t point;
    uint32_t l4timeoutst;
    uint32_t timeoutst = gTimer1ms; //设置超时
    //uint32_t tick;
LOOP:
    sta = 0;
    err = 0;
    //FM11NC08WriteReg(NFC_FIFO_FLUSH,0xff);
    while(TIMEPASS(timeoutst)<timeout)
    {
        if(feedtime++ > 0xfff)
        {
            feedtime = 0;
            BSPFeedWDogTask();
        }
        if((FM11NC08ReadReg(NFC_RF_STATUS)>>5) != NFC_RFSTA_L4)
        {
            //l4flag = 0;
            l4timeoutst = gTimer1ms;
            while(TIMEPASS(l4timeoutst) < TIMEOUTMS(10));
            goto LOOP;
        }
        //else if(l4flag == 0)
        //{
        //    l4flag = 1;
        //    FM11NC08WriteReg(NFC_FIFO_FLUSH,0xff);
            //l4timeoutst = gTimer1ms;
            //while(TIMEPASS(l4timeoutst)< TIMEOUTMS(1500));
        //}
        
        main_irq = FM11NC08ReadReg(NFC_MAIN_IRQ);
        FM11NC08ReadReg(NFC_AUX_IRQ);
        FM11NC08ReadReg(NFC_FIFO_IRQ);

        if((main_irq&NFC_MAINIRQ_RXST_MASK))
        {
            sta = 1;
            err = 0;
            point = 0;
        }

        if(sta == 1)
        {
                if(main_irq&NFC_MAINIRQ_AUX_MASK)//接收帧错误,CRC错误，校验错误
                {
                    sta = 0;
                    FM11NC08WriteReg(NFC_FIFO_FLUSH,0xff);
                }
                else
                {
                    fifocnt = FM11NC08ReadReg(NFC_FIFO_WCNT);
                    if(fifocnt>0)
                    {
                        
                        if(/*((*cnt)+7)*/259>=(point+fifocnt))  //数据未超长
                        {
                            FM11NC08ReadFIFO(NFC_BUF + point,fifocnt);
                            point += fifocnt;
                        }
                        else
                        {
                            err = 1;
                        }
                    }

                    if(main_irq&NFC_MAINIRQ_RXDN_MASK)
                    {
                        FM11NC08WriteReg(NFC_FIFO_FLUSH,0xff);
                        if((point<7)||(err))//数据过短，或超长
                        {
                            goto LOOP;
                        }
                        goto RX_CMP;
                    }
                }
                
            }
    }
    return FALSE;
    
RX_CMP:
    for(i=0;i<4;i++)
    {
        if(c_NFC_HEADSTR[i] != NFC_BUF[i])//无有效NFC帧头重新接收，
        {
            goto LOOP;
        }
    }
    CommCalcCRC(COMM_CRC_A,NFC_BUF,point-2,&crc1,&crc2);//校验NFC CRC
    if((crc1!=NFC_BUF[point-2])||(crc2!=NFC_BUF[point-1]))
    {
        //MB_EXP = 0x01;//for test
        goto LOOP;
    }
    if(c_FUNCODE != NFC_BUF[4])//功能码错误，返回错误码后重新接收
    {
        MB_EXP = 0x01;
        FM11NC08FrameTx(NFC_BUF[4]|0x80,&MB_EXP,1,TIMEOUTMS(10));
        goto LOOP;
    }
    if((point-7) > *cnt)//数据最大长度错误
    {
        MB_EXP = 0x02;
        FM11NC08FrameTx(NFC_BUF[4]|0x80,&MB_EXP,1,TIMEOUTMS(10));
        goto LOOP;
    }
    
    *cnt = point-7;
    StringCopy(buf,NFC_BUF+5,*cnt);
    return TRUE;
    
}

/*****************************************end of FM11NC08.c*****************************************/
