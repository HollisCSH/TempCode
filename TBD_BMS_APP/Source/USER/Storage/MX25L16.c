//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: MX25L16.c
//创建人  	: Handry
//创建日期	: 
//描述	    : MX25L16驱动源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
//=============================================================================================
//包含头文件
//=============================================================================================
#include "MX25L16.h"
#include "BSP_SPI.h"
//#include "BSPSysTick.h"

//=============================================================================================
//全局变量定义
//=============================================================================================
u8 gFlashErr = 0;						//flash芯片故障

//=============================================================================================
//静态函数声明
//=============================================================================================

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void MX25L16Init(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: M25L16初始化函数
//注    意	:
//=============================================================================================
void MX25L16Init(void)
{
	u16 flashid = 0;
    
	HAL_Delay(300);
	FLASH_PWR_ON;				//flash供电打开
	HAL_Delay(10);
	flashid = MX25L16ReadDevID();
	
	if(FLASH_DEV_ID != flashid )	//设备id与读取的不符合
	{
		gFlashErr++;
	}
	else
	{
		gFlashErr = 0;
	}

    MX25L16WriteEnable();		//允许写
    MX25L16WriteStatus(0);		//去除block保护
}

//=============================================================================================
//函数名称	: void MX25L16FaultTest(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: M25L16错误检测
//注    意	:
//=============================================================================================
void MX25L16FaultCheck(void)
{
	u16 flashid = 0;
	
	flashid = MX25L16ReadDevID();
	
	if(FLASH_DEV_ID != flashid )	//设备id与读取的不符合
	{
		gFlashErr++;
	}
	else
	{
		gFlashErr = 0;
	}    
}

//=============================================================================================
//函数名称	: u16 MX25L16ReadDevID(void)
//函数参数	:
//输出参数	: ID
//静态变量	:
//功    能	: 读取设备ID函数
//注    意	:
//=============================================================================================
u16 MX25L16ReadDevID(void)
{
	u16 Temp = 0;
	uint8_t SPI_TX_BUF[4];
	uint8_t SPI_RX_BUF[2];
	
	FLASH_POWER_ENABLE();	//芯片供电
	MX25L16CSLow();    		//片选拉低，使能
	SPI_TX_BUF[0] = M25L_MANUFACTDEVICEID;
	SPI_TX_BUF[1] = 0x00;
	SPI_TX_BUF[2] = 0x00;
	SPI_TX_BUF[3] = 0x00;
//	SPI_Transmit(SPI_TX_BUF,  4,  10);
//	SPI_Receive(SPI_RX_BUF,  2,  10);   
    BSPSPISendNDatas(FLASH_SPI_CHN , SPI_TX_BUF, 4 );
    BSPSPIRecNDatas(FLASH_SPI_CHN , SPI_RX_BUF, 2 );

	Temp|= (u16)SPI_RX_BUF[0] << 8;
	Temp|= (u16)SPI_RX_BUF[1];

	MX25L16CSHigh();    //片选拉高，失能

	return Temp;
}

//=============================================================================================
//函数名称	: u8 MX25L16CheckBusy(void)
//函数参数	:
//输出参数	: 1：FLASH忙；0：FLASH空闲
//静态变量	:
//功    能	: flash全片擦除
//注    意	:
//=============================================================================================
u8 MX25L16CheckBusy(void)
{
    if(MX25L16ReadStatus() & 0x01)
    {
        return 1;
    }
    else
    {
    	return 0;
    }
}

//=============================================================================================
//函数名称	: void  MX25L16_WriteEnable(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: 去除FLASH写保护
//注    意	:
//=============================================================================================
void  MX25L16WriteEnable(void)
{
	MX25L16CSLow();    //片选拉低，使能

	BSPSPISendOneData(FLASH_SPI_CHN,M25L_WRITEENABLE);	//发送写使能命令

	MX25L16CSHigh();    //片选拉高，失能
}

//=============================================================================================
//函数名称	: void  MX25L16_WriteDisable(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: FLASH写保护
//注    意	:
//=============================================================================================
void  MX25L16WriteDisable(void)
{
	MX25L16CSLow();    //片选拉低，使能

	BSPSPISendOneData(FLASH_SPI_CHN,M25L_WRITEDISABLE);	//发送写保护命令

	MX25L16CSHigh();    //片选拉高，失能
}

//=============================================================================================
//函数名称	: u8 MX25L16ReadStatus(void)
//函数参数	:
//输出参数	: 状态寄存器的值
//静态变量	:
//功    能	: 读取状态寄存器
//注    意	:
//=============================================================================================
u8 MX25L16ReadStatus(void)
{
	u8 stat = 0;

	MX25L16CSLow();    //片选拉低，使能

	BSPSPISendOneData(FLASH_SPI_CHN,M25L_READSTATUS);	//发送读取状态寄存器命令
	stat = BSPSPIRcvOneData(FLASH_SPI_CHN);

	MX25L16CSHigh();    //片选拉高，失能

	return stat;
}

//=============================================================================================
//函数名称	: void MX25L16WriteStatus(u8 sr)
//函数参数	: 写状态寄存器的值
//输出参数	:
//静态变量	:
//功    能	: 读取状态寄存器
//注    意	:
//=============================================================================================
void MX25L16WriteStatus(u8 sr)
{
	MX25L16CSLow();    //片选拉低，使能

	BSPSPISendOneData(FLASH_SPI_CHN,M25L_WRITESTATUS);	//发送写状态寄存器的命令
	BSPSPISendOneData(FLASH_SPI_CHN,sr);
	
	HAL_Delay(100);

	MX25L16CSHigh();    //片选拉高，失能
}

//=============================================================================================
//函数名称	: void MX25L16ReadData(u32 addr,u8 * readbuf,u32 len)
//函数参数	: pageaddr:写页地址
//			  writebuf:写入的数组地址
//			  len:写入的长度
//输出参数	:
//静态变量	:
//功	能	: flash存储页编程
//注	意	:
//=============================================================================================
void MX25L16WritePage(u32 pageaddr,u8 *writebuf,u32 len)
{
    u8 temp[4];

    temp[0] = M25L_PAGEPROGRAM;			//页编程命令
    temp[1] = (pageaddr>>16)&0x3f;
    temp[2] = (pageaddr>>8)&0xff;
    temp[3] = (pageaddr)&0xff;

    MX25L16WriteEnable();	//去除写保护

    MX25L16CSLow();    //片选拉低，使能

//    BSPSPISendOneData(FLASH_SPI_CHN,temp[0]);
//    BSPSPISendOneData(FLASH_SPI_CHN,temp[1]);
//    BSPSPISendOneData(FLASH_SPI_CHN,temp[2]);
//    BSPSPISendOneData(FLASH_SPI_CHN,temp[3]);
    BSPSPISendNDatas(FLASH_SPI_CHN,&temp[0],4);	//写入数据
    BSPSPISendNDatas(FLASH_SPI_CHN,&writebuf[0],len);	//写入数据

    MX25L16CSHigh();    //片选拉高，失能
}

//=============================================================================================
//函数名称	: void MX25L16ReadData(u32 addr,u8 * readbuf,u32 len)
//函数参数	: addr:读地址
//			  readbuf:读取的数组地址
//			  len:读取的长度
//输出参数	:
//静态变量	:
//功	能	: 读取flash存储的数据
//注	意	:
//=============================================================================================
void MX25L16ReadData(u32 addr,u8 * readbuf,u32 len)
{
    u8 temp[4];

    temp[0] = M25L_READDATA;		//读取数据命令
    temp[1] = (addr>>16)&0x3f;
    temp[2] = (addr>>8)&0xff;
    temp[3] = (addr)&0xff;
    //FLASH_PWR_ON;          //供电开启
    MX25L16CSLow();    //片选拉低，使能

//    BSPSPISendOneData(FLASH_SPI_CHN,temp[0]);
//    BSPSPISendOneData(FLASH_SPI_CHN,temp[1]);
//    BSPSPISendOneData(FLASH_SPI_CHN,temp[2]);
//    BSPSPISendOneData(FLASH_SPI_CHN,temp[3]);
    
	
//	readbuf[0] = BSPSPIRcvOneData(FLASH_SPI_CHN);
//	readbuf[1] = BSPSPIRcvOneData(FLASH_SPI_CHN);
    BSPSPISendNDatas(FLASH_SPI_CHN,&temp[0],4);	//写入数据
    BSPSPIRecNDatas(FLASH_SPI_CHN,&readbuf[0],len);
  
//    SPI_Transmit(temp,  4,  10);
//    SPI_Receive(readbuf,  len,  10);

    MX25L16CSHigh();    //片选拉高，失能

}

//=============================================================================================
//函数名称	: u8 MX25L16EraseSector(u32 secaddr)
//函数参数	: secaddr:扇区地址
//输出参数	: 1：扇区擦除成功；0：扇区擦除失败
//静态变量	:
//功    能	: flash扇区擦除
//注    意	: 1 扇区 = 4k字节    16Mb最多有512个扇区
//=============================================================================================
u8 MX25L16EraseSector(u32 secaddr)
{
    u8 temp[4];

    temp[0] = M25L_SECTORERASE;		//扇区擦除命令
    temp[1] = (secaddr>>16)&0x3f;
    temp[2] = (secaddr>>8)&0xff;
    temp[3] = (secaddr)&0xff;

    MX25L16WriteEnable();

    MX25L16CSLow();    //片选拉低，使能

//    BSPSPISendOneData(FLASH_SPI_CHN,temp[0]);
//    BSPSPISendOneData(FLASH_SPI_CHN,temp[1]);
//    BSPSPISendOneData(FLASH_SPI_CHN,temp[2]);
//    BSPSPISendOneData(FLASH_SPI_CHN,temp[3]);
	BSPSPISendNDatas(FLASH_SPI_CHN,&temp[0],4);	//写入数据

    MX25L16CSHigh();    //片选拉高，失能

    return 1;

}

//=============================================================================================
//函数名称	: u8 MX25L16EraseBlock(u32 blockaddr)
//函数参数	: blockaddr:块地址
//输出参数	: 1：块擦除成功；0：块擦除失败
//静态变量	:
//功    能	: flash块擦除
//注    意	:
//=============================================================================================
u8 MX25L16EraseBlock(u32 blockaddr)
{
    uint8_t temp[4];

    temp[0] = M25L_BLOCKERASE;		//块擦除命令
    temp[1] = (blockaddr>>16)&0x3f;
    temp[2] = (blockaddr>>8)&0xff;
    temp[3] = (blockaddr)&0xff;

    MX25L16WriteEnable();

    MX25L16CSLow();    //片选拉低，使能

    BSPSPISendOneData(FLASH_SPI_CHN,temp[0]);
    BSPSPISendOneData(FLASH_SPI_CHN,temp[1]);
    BSPSPISendOneData(FLASH_SPI_CHN,temp[2]);
    BSPSPISendOneData(FLASH_SPI_CHN,temp[3]);

    MX25L16CSHigh();    //片选拉高，失能

    return 1;
}

//=============================================================================================
//函数名称	: u8 MX25L16EraseChip(void)
//函数参数	:
//输出参数	: 1：全片擦除成功；0：全片擦除失败
//静态变量	:
//功    能	: flash全片擦除
//注    意	:
//=============================================================================================
u8 MX25L16EraseChip(void)
{
    MX25L16WriteEnable();

	MX25L16CSLow();    //片选拉低，使能

	BSPSPISendOneData(FLASH_SPI_CHN,M25L_CHIPERASE);	//发送全片擦除命令

    MX25L16CSHigh();    //片选拉高，失能

    return 1;
}

/*****************************************end of MX25L16.c*****************************************/
