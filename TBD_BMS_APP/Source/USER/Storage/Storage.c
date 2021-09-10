//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Storage.c
//创建人  	: Handry
//创建日期	: 
//描述	    : MX25L16驱动源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	: Handry
//版本	    : 
//修改日期	: 2020/07/14
//描述	    : 
//1.为了在增加配置参数后兼容以前的配置，配置参数长度更改为以前配置参数的长度
//2.增加了加速度计配置参数和额外配置参数的检测
//=============================================================================================
//=============================================================================================
//包含头文件
//=============================================================================================
#include "Storage.h"
#include "MX25L16.h"
#include "BSP_GPIO.h"
#include "BSP_RTC.h"
//#include "PCF85063.h"
#include "Comm.h"
#include "CRC16.h"
#include "ParaCfg.h"
#include "UserData.h"
#include "EventRecord.h"
//#include "BSPSysTick.h"
#include "ParaCfg.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "BSPTimer.h"
#include "BSPSystemCfg.h"
//#include "BSPSPI.h"
#include "BatteryInfo.h"
#include "pt.h"
#ifdef BMS_USE_SOX_MODEL
#include "SocCapCalc.h"
#include "SocEepHook.h"
#endif


//=============================================================================================
//定义全局变量
//=============================================================================================
u8 gFlashReadbuf[sizeof(t_CONFIG_PARA)] = {0};		//读缓存
u8 gStorageBuff[sizeof(t_CONFIG_PARA)];

pt gPTStorage;					//存储任务pt线程控制变量
pt gPTStorageSub;				//存储子任务pt线程控制变量
u16 gStorageCTRL = 0;           //存储控制 增加清除SOC算法位
u8 gStorageErr = 0;				//存储故障,0 -- 无错误;bit1~0 -- config错误; bit3~2 -- user area 错误; bit5~4--事件记录错误;bi7~6 --读取事件错误
u8 gFlashPowSta = 0;            //Flash上电状态：0：未上电；1：已上电
u8 gPowOffTim = 0;              //关闭FLASH延时定时器

static const u32 gStorageAddrTab[2] = {CFG_START_ADDR, USER_START_ADDR};            //配置参数和用户数据存放flash起始地址
static const u32 gStorageLenTab[2] = {CALC_CRC_CONFIG_SIZE, sizeof(t_USER_DATA)};   //配置参数和用户数据结构体长度
static  u8 * const gStorageDataTab[2] = {(u8 *)&gConfig,(u8 *)&gUserID};            //配置参数和用户数据地址
static  u8 * const gStorageDefDataTab[2] = {(u8 *)&cConfigInit,(u8 *)&cUserIDInit}; //默认配置参数和用户数据地址

//=============================================================================================
//声明静态函数
//=============================================================================================
//=============================================================================================
//函数名称	: static void StorageRecordInfoCheck(t_RECORD_INFO *info)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 通过轮询读取的方法获得事件记录个数，编号等
//注	意	:
//=============================================================================================
static void StorageRecordInfoCheck(void);

//=============================================================================================
//函数名称	: static void StorageSOCInfoCheck(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 通过轮询获取写入SOC的地址
//注	意	:
//=============================================================================================
static void StorageSOCInfoCheck(void);

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void StorageInit(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 存储任务初始化函数
//注	意	:
//=============================================================================================
void StorageInit(void)
{
	u16 crc16 = 0;
	u16 crc16_1 = 0;
	u16 crc16_2 = 0;
    u8 area = 0;    //区域：0表示原始数据；1表示备份数据
    u8 areavalid = 0;   //区域有效标志
    u8 writecnt = 0;
    u8 type = 0;
    u16 paraflag = 0;
    
    FLASH_PWR_ON;
    gFlashPowSta = 1;
    gConfig = cConfigInit;
    
	/* type = 0;读配置参数初始化,读取原始区和备份区，检测是否都无效，其中一个有效则写入另一个扇区 */
    /* type = 1;读用户数据参数初始化,读取原始区和备份区，检测是否都无效，其中一个有效则写入另一个扇区 */   
    for(type = 0;type < 2;type++)
    {
        paraflag = (1 == type)? USER_CFG_FLAG : PARA_CFG_FLAG;  //更改数据标志
        
        while(1)
        {
            for(area = 0; area < 2;area++)  //有原始区和备份区，都进行校验
            {
                //原始数据和备份数据相隔一个扇区
                StorageReadFlash(gStorageAddrTab[type] + area * SECTOR_SIZE,gStorageBuff,gStorageLenTab[type]);

                crc16_1 = CRC16Calculate(gStorageBuff,gStorageLenTab[type] - 2);
                crc16_2 = *(u16*)(gStorageBuff + gStorageLenTab[type] - 2);

                if( (*(u16*)gStorageBuff != paraflag) || (crc16_1 != crc16_2))
                {
                    BITCLR(areavalid,area);     //设置原始/备份区域有效标志
                }
                else
                {
                    StorageStringCopy(gStorageDataTab[type], gStorageBuff, gStorageLenTab[type]);
                    BITSET(areavalid,area);     //设置原始/备份区域无效标志
                }
            }
            
            if((writecnt > 1) && (areavalid == 0))
            {
                //写入2次后，依然都校验不通过
                BITSET(gStorageErr,0 + type * 2);  //原始/备份配置参数区都异常
                BITSET(gStorageErr,1 + type * 2);
                writecnt = 0;
                break;     //跳出while循环，进入下一个数据类型的存储检测         
            }
            
            if(areavalid == 0x03)   //直到两个区域数据都校验ok
            {
                BITCLR(gStorageErr,0 + type * 2);  //原始/备份配置参数区无异常
                BITCLR(gStorageErr,1 + type * 2);
                writecnt = 0;
                break;     //跳出while循环，进入下一个数据类型的存储检测 
            }
            else if(areavalid == 0x01)  //原始数据区有效，备份区无效
            {
                BITCLR(gStorageErr,0 + type * 2);      //原始配置参数区无异常
                MX25L16EraseSector(gStorageAddrTab[type] + SECTOR_SIZE);      //擦除备份区扇区
                while(MX25L16CheckBusy());     
                StorageStringCopy(gStorageBuff,gStorageDataTab[type], gStorageLenTab[type]);	            //放入缓冲区中
                StorageWriteFlashWait(gStorageAddrTab[type] + SECTOR_SIZE,(u8 *)gStorageBuff,gStorageLenTab[type]);   //写入备份区      
                writecnt++; //写入flash次数+1         
            }
            else if(areavalid == 0x02)  //备份数据区有效，原始区无效
            {
                BITCLR(gStorageErr,1 + type * 2);              //备份配置参数区无异常
                MX25L16EraseSector(gStorageAddrTab[type]);     //擦除原始区扇区
                while(MX25L16CheckBusy());     
                StorageStringCopy(gStorageBuff,gStorageDataTab[type], gStorageLenTab[type]);	        //放入缓冲区中
                StorageWriteFlashWait(gStorageAddrTab[type],(u8 *)gStorageBuff,gStorageLenTab[type]);   //写入备份区             
                writecnt++; //写入flash次数+1         
            }        
            else if(areavalid == 0x00)   //两个区域数据都校验无效
            {
                StorageStringCopy(gStorageDataTab[type],gStorageDefDataTab[type],gStorageLenTab[type]); //导入默认数据
                StorageStringCopy(gStorageBuff,gStorageDataTab[type], gStorageLenTab[type]);	//放入缓冲区中
                crc16 = CRC16Calculate((u8 *)gStorageBuff,gStorageLenTab[type] - 2);

                *(u16*)(&gStorageBuff[gStorageLenTab[type] -2]) = (crc16);	//放入crc校验

                MX25L16EraseSector(gStorageAddrTab[type]);//擦除原始扇区
                while(MX25L16CheckBusy());
                
                MX25L16EraseSector(gStorageAddrTab[type] + SECTOR_SIZE);//擦除备份扇区
                while(MX25L16CheckBusy());
                StorageWriteFlashWait(gStorageAddrTab[type],(u8 *)gStorageBuff,gStorageLenTab[type]);                   //写入原始区
                StorageWriteFlashWait(gStorageAddrTab[type] + SECTOR_SIZE,(u8 *)gStorageBuff,gStorageLenTab[type]);     //写入备份区         
                writecnt++; //写入flash次数+1         
            }
        }
    }

    //初始化配置参数BUFF
    gConfigBuff = gConfig;
    //初始化用户数据BUFF
    gUserIDBuff = gUserID;

    /* 读历史记录初始化 */
    StorageRecordInfoCheck();   //recordinfo不使用存储的值而是自己查找
    
    /* SOC信息初始化 */
    StorageSOCInfoCheck();
    
    //每次重启后，除了SN码，所有配置参数复位并写入FLASH
    if(((!StorageStringComp((u8 *)&(cConfigInit.cellov),(u8 *)&(gConfigBuff.cellov),\
        ((u32)(&(gConfigBuff.calpara)) - (u32)(&(gConfigBuff.cellov)))))
       #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
       || (!StorageStringComp((u8 *)&(cConfigInit.accpara),(u8 *)&(gConfigBuff.accpara),sizeof(t_ACC_PARA)))
       #endif 
       || (!StorageStringComp((u8 *)&(cConfigInit.configadd),(u8 *)&(gConfigBuff.configadd),sizeof(t_CONFIG_PARA_ADD))))
       && cConfigInit.flag==gConfigBuff.flag)
    {
        gConfigBuff = cConfigInit;
        gConfigBuff.calpara = gConfig.calpara;
        gConfigBuff.sn[0] = gConfig.sn[0];
        gConfigBuff.sn[1] = gConfig.sn[1];
        gConfigBuff.sn[2] = gConfig.sn[2];
        gConfigBuff.sn[3] = gConfig.sn[3];
        gStorageCTRL |= (FLASH_WR_CFG_MASK);
    }
    
}

//const pin_settings_config_t cPinMuxSpiDeConfigArr[4] =
//{
//    /* SPI 1 FLASH SPI*/
//    {
//        .base          = LPSPI1_MOSI_GPIOBASE,
//        .pinPortIdx    = LPSPI1_MOSI_PN,
//        .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
//        .passiveFilter = false,
//        .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
//        .mux           = PORT_MUX_AS_GPIO,
//        .pinLock       = false,
//        .intConfig     = PORT_DMA_INT_DISABLED,
//        .clearIntFlag  = false,
//        .gpioBase      = LPSPI1_MOSI_BASE,
//        .direction     = GPIO_OUTPUT_DIRECTION,        
//        .digitalFilter = false,
//        .initValue     = 0u,
//    },
//    {
//        .base          = LPSPI1_SCK_GPIOBASE,
//        .pinPortIdx    = LPSPI1_SCK_PN,
//        .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
//        .passiveFilter = false,
//        .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
//        .mux           = PORT_MUX_AS_GPIO,
//        .pinLock       = false,
//        .intConfig     = PORT_DMA_INT_DISABLED,
//        .clearIntFlag  = false,
//        .gpioBase      = LPSPI1_SCK_BASE,
//        .direction     = GPIO_INPUT_DIRECTION,        
//        .digitalFilter = false,
//    },
//    {
//        .base          = LPSPI1_CS_GPIOBASE,
//        .pinPortIdx    = LPSPI1_CS_PN,
//        .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
//        .passiveFilter = false,
//        .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
//        .mux           = PORT_MUX_AS_GPIO,
//        .pinLock       = false,
//        .intConfig     = PORT_DMA_INT_DISABLED,
//        .clearIntFlag  = false,
//        .gpioBase      = LPSPI1_CS_BASE,
//        .direction     = GPIO_INPUT_DIRECTION,
//        .digitalFilter = false,
//        .initValue     = 0u,
//    },
//    {
//        .base          = LPSPI1_MISO_GPIOBASE,
//        .pinPortIdx    = LPSPI1_MISO_PN,
//        .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
//        .passiveFilter = false,
//        .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
//        .mux           = PORT_MUX_AS_GPIO,
//        .pinLock       = false,
//        .intConfig     = PORT_DMA_INT_DISABLED,
//        .clearIntFlag  = false,
//        .gpioBase      = LPSPI1_MISO_BASE,
//        .direction     = GPIO_INPUT_DIRECTION,        
//        .digitalFilter = false,
//    },       
//    
//};

//const pin_settings_config_t cPinMuxSpiConfigArr[4] =
//{
//    /* SPI 1 FLASH SPI*/
//    {
//        .base          = LPSPI1_MOSI_GPIOBASE,
//        .pinPortIdx    = LPSPI1_MOSI_PN,
//        .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
//        .passiveFilter = false,
//        .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
//        .mux           = PORT_MUX_ALT3,
//        .pinLock       = false,
//        .intConfig     = PORT_DMA_INT_DISABLED,
//        .clearIntFlag  = false,
//        .gpioBase      = LPSPI1_MOSI_BASE,
//        .digitalFilter = false,
//    },
//    {
//        .base          = LPSPI1_SCK_GPIOBASE,
//        .pinPortIdx    = LPSPI1_SCK_PN,
//        .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
//        .passiveFilter = false,
//        .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
//        .mux           = PORT_MUX_ALT3,
//        .pinLock       = false,
//        .intConfig     = PORT_DMA_INT_DISABLED,
//        .clearIntFlag  = false,
//        .gpioBase      = LPSPI1_SCK_BASE,
//        .digitalFilter = false,
//    },
//    {
//        .base          = LPSPI1_CS_GPIOBASE,
//        .pinPortIdx    = LPSPI1_CS_PN,
//        .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
//        .passiveFilter = false,
//        .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
//        .mux           = PORT_MUX_AS_GPIO,
//        .pinLock       = false,
//        .intConfig     = PORT_DMA_INT_DISABLED,
//        .clearIntFlag  = false,
//        .gpioBase      = LPSPI1_CS_BASE,
//        .direction     = GPIO_OUTPUT_DIRECTION,
//        .digitalFilter = false,
//        .initValue     = 0u,
//    },
//    {
//        .base          = LPSPI1_MISO_GPIOBASE,
//        .pinPortIdx    = LPSPI1_MISO_PN,
//        .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
//        .passiveFilter = false,
//        .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
//        .mux           = PORT_MUX_ALT5,
//        .pinLock       = false,
//        .intConfig     = PORT_DMA_INT_DISABLED,
//        .clearIntFlag  = false,
//        .gpioBase      = LPSPI1_MISO_BASE,
//        .digitalFilter = false,
//    },     
//};

//=============================================================================================
//函数名称	: void StoragePowerOff(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 存储关闭
//注	意	:
//=============================================================================================
void StoragePowerOff(void)
{
//    BSPSPIDisable(FLASH_SPI_CHN);    //关闭SPI  
		BSPSPIDisable();    //关闭SPI     
//    PINS_DRV_Init(4, cPinMuxSpiDeConfigArr);   
    
//	    BSPGPIOClrPin(FLASH_CS_PORT,FLASH_CS_PIN);   
//	    BSPGPIOClrPin(FLASH_CLK_PORT,FLASH_CLK_PIN);
//	    BSPGPIOClrPin(FLASH_MOSI_PORT,FLASH_MOSI_PIN); 
    
    FLASH_PWR_OFF;          //供电关闭
}

//=============================================================================================
//函数名称	: void StoragePowerOn(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 存储开启，供电
//注	意	:
//=============================================================================================
void StoragePowerOn(void)
{
    FLASH_PWR_ON;          //供电开启

//    PINS_DRV_Init(4, cPinMuxSpiConfigArr); 
    BSPSPIInit();  //开启SPI
}

//=============================================================================================
//函数名称	: void StorageWriteFlashWait(u32 addr,u8 *writebuf,u32 len)
//函数参数	: addr：写入的地址；*writebuf:写入数据的数组指针；len：写入的长度
//输出参数	: void
//静态变量	: void
//功	能	: 读取flash函数
//注	意	:
//=============================================================================================
 void StorageWriteFlashWait(u32 addr,u8 *writebuf,u32 len)
 {
	u32 writeaddr = 0;
	u32 writelen = 0;
	u8 *pwrite = 0;

	writeaddr = addr;
	pwrite = writebuf;
	writelen = len;

	while(writelen)
	{
		if(writelen > 256)
		{
			MX25L16WritePage(writeaddr,pwrite,256);
			while(MX25L16CheckBusy());
			writelen = writelen - 256;
			pwrite = pwrite + 256;
			writeaddr = writeaddr + 256;

		}
		else
		{
			MX25L16WritePage(writeaddr,pwrite,writelen);
			while(MX25L16CheckBusy());
			writelen = 0;
		}
	}
 }

//=============================================================================================
//函数名称	: void StorageReadFlash(u32 addr,u8 *readbuf,u32 len)
//函数参数	: addr：读取的地址；*readbuf:读取数据放置的数组指针；len：读取的长度
//输出参数	: void
//静态变量	: void
//功	能	: 读取flash函数
//注	意	:
//=============================================================================================
void StorageReadFlash(u32 addr,u8 *readbuf,u32 len)
{
	u32 readaddr = 0;
	u32 readlen = 0;
	u8 *pread = 0;

	readaddr = addr;
	pread = readbuf;
	readlen = len;

	while(readlen)
	{
		if(readlen > 256)
		{
			MX25L16ReadData(readaddr,pread,256);
			readlen = readlen - 256;
			pread = pread + 256;
			readaddr = readaddr + 256;

		}
		else
		{
			MX25L16ReadData(readaddr,pread,readlen);
			readlen = 0;
		}
	}
}

//=============================================================================================
//函数名称	: u8 StorageStringCopy(const u8 *msg1, u8 *msg2, u16 cnt)
//输入参数	: dest：目标字符串地址； *sour：源字符串地址；cnt：复制的长度
//输出参数	: void
//函数功能	: 字符串复制
//注意事项	:
//=============================================================================================
void StorageStringCopy(u8 *dest, u8 *sour, u16 cnt)
{
    while(0 < cnt--)
    {
       *(dest++) = *(sour++);
    }
}

//=============================================================================================
//函数名称	: u8 StorageStringComp(const u8 *msg1, const u8 *msg2, u16 cnt)
//输入参数	: msg1：比较字符串1；msg2：比较的字符串2；cnt：比较的长度
//输出参数	: TRUE：比较正确；FALSE：比较错误
//函数功能	: 字符串比较
//注意事项	:
//=============================================================================================
u8 StorageStringComp(const u8 *msg1, const u8 *msg2, u16 cnt)
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

//=============================================================================================
//函数名称	: static void StorageRecordInfoCheck(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 通过轮询读取的方法获得事件记录个数，编号等
//注	意	:
//=============================================================================================
static void StorageRecordInfoCheck(void)
{
    u32 addr = RECORD_START_ADDR;                                //遍历的地址
    u32 tempnum = 0,maxnum = 0,minnum = 0xFFFFFFFF;              //查找的最大编号，最小编号
    u32 listhead1 = 0,listtail1 = 0,listhead2 = 0,listtail2 = 0; //事件记录可能有2段，每一段的头部编号为尾部编号都找出来
    u8 checksta=0x00;                                            //遍历状态

    for(addr = RECORD_START_ADDR;addr <=RECORD_END_ADDR;addr += RECORD_SIZE)
    {
        StorageReadFlash(addr, (u8*)&tempnum, 4);
        
        if((tempnum > 0) && (tempnum <= RECORD_MAX_NUM) && (((tempnum - 1) % RECORD_MAX_ITEM) == (addr / RECORD_SIZE)))
        {
            if(tempnum > maxnum)
            {
                maxnum = tempnum;  //找出其中的最大编号
            }
            if(tempnum < minnum)
            {
                minnum = tempnum;  //找出其中的最小编号
            }

            switch(checksta)
            {
                case 0x00:
                    listhead1 = tempnum;               //初始化第一段记录头部和尾部
                    listtail1 = listhead1;
                    checksta = 0x01;
                break;
                
                case 0x01:
                    if((listtail1 + 1 ) == tempnum)   //还没到第一段事件编号的尾部
                    {
                        listtail1 = tempnum;          //更新第一段尾部信息
                    }
                    else
                    {
                        listhead2 = tempnum;       //比上个比较的编号小，说明此处是最早记录的编号，为第二段
                        listtail2 = tempnum;
                        checksta = 0x02;
                    }
                    
                break;
                    
                case 0x02:
                    if((listtail2 + 1) == tempnum)
                    {
                        listtail2 = tempnum;   //更新第二段尾部信息
                    }
                    else
                    {
                        //异常
                        listhead2 = tempnum;
                        listtail2 = tempnum;
                        checksta = 0x02;
                    }
                break;
                    
                default:
                    checksta = 0x00;
                break;
            }

        }
    }
    //遍历结束后，没有任何一条事件记录
    if((maxnum == 0) || (minnum == 0xFFFFFFFF))
    {
        gRecordInfo.earliest = 0;
        gRecordInfo.latest = 0;
    }
    else
    {
        if(listtail2 >= RECORD_MAX_NUM)
        {
            gRecordInfo.earliest = listhead2;
            gRecordInfo.latest = listtail1;
        }
        else
        {
            gRecordInfo.earliest = minnum;
            gRecordInfo.latest = maxnum;
        }
    }
}

//=============================================================================================
//函数名称	: static void StorageSOCInfoCheck(void)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 通过轮询获取写入SOC的地址
//注	意	:
//=============================================================================================
u16 gSOC_InfoNum = 0;
static void StorageSOCInfoCheck(void)
{
    u32 addr = SOC_START_ADDR;                                //遍历的地址
    u16 tempnum = 0;

    gSOC_InfoNum = 0;
    for(addr = SOC_START_ADDR; addr <= SOC_END_ADDR; addr += sizeof(t_SOC_EEP_INFO))
    {
        StorageReadFlash(addr, (u8*)&tempnum, 2);
        ////////////////////////////////////////////
//        MX25L16Init();			//MX25L16	FLASH芯片初始化     
//        tempnum = 0x5A5A;
//        StorageWriteFlashWait(SOC_START_ADDR,(u8 *)&tempnum,2);	    //写数据
//        tempnum = 0;
//        StorageReadFlash(SOC_START_ADDR, (u8*)&tempnum, 2); 
        ////////////////////////////////////////////
        
        if(0xffff != tempnum && 0 != tempnum)   //有效数据
        {
            gSOC_InfoNum++;
        }
        else
        {
            break;
        }       
    }
    
    if(gSOC_InfoNum >= 1)    //代表有存储soc数据
    {
        addr = (gSOC_InfoNum - 1) * sizeof(t_SOC_EEP_INFO) + SOC_START_ADDR;
        StorageReadFlash(addr,(u8*)&gSocEepInfo,sizeof(t_SOC_EEP_INFO)); //读取数据上来       
//        gSOCSaveInfo.num = *(u16*)(gFlashReadbuf);
//        gSOCInfo.tolcap = *(u16*)(gFlashReadbuf + 2);
//        gSOCInfo.remaincap = *(u16*)(gFlashReadbuf + 4);
//        gSOCInfo.displaySOC = *(u16*)(gFlashReadbuf + 6);
    }
}

//=============================================================================================
//函数名称	: void StorageMainTask(void *p)
//函数参数	: void
//输出参数	: void
//静态变量	: void
//功	能	: 存储任务函数
//注	意	:
//=============================================================================================
void StorageMainTask(void *p)
{
	u16 crc16 = 0;
	static u32 everecaddr;
	static u32 sticktimer = 0;
    u8 area = 0;                //0表示原始数据区，1表示备份数据区
    u8 areavalid = 0;           //bit0表示原始数据区有效，bit1表示备份数据区有效
    static u8 cfgwritecnt = 0;  //配置参数重新写入的次数
    static u8 userwritecnt = 0; //用户数据重新写入的次数
	(void)p;

	PT_BEGIN(&gPTStorage);

	for(;;)
    {
		//电源关闭
        if(gStorageCTRL & FLASH_POW_OFF_MASK)
        {
			//FLASH_PWR_OFF;
            StoragePowerOff();
			gFlashPowSta = 0;

			gStorageCTRL &= ~(FLASH_POW_OFF_MASK);
            BSPTaskStart(TASK_ID_STORAGE_TASK, 200);	//空闲时200ms周期
			return;
        }
		else if(gStorageCTRL & 0x017f) //电源测试或flash操作打开电源
		{
            gPowOffTim = 0;
			//FLASH_PWR_ON;
            if(0 == gFlashPowSta)
            {
                StoragePowerOn();
                sticktimer = gTimer1ms;
                PT_WAIT_UNTIL(&gPTStorage,(BSPTaskStart(TASK_ID_STORAGE_TASK, 2))&(gTimer1ms - sticktimer >= 100));  //延迟50ms
                gFlashPowSta = 1;
                //MX25L16FaultCheck();    //FLASH上电，加一次读ID检测
            }
		}
		else
		{
			if(gFlashPowSta == 1)	//检查电源是否关闭
            {
				gPowOffTim++;
				if(gPowOffTim >= 50) //200ms周期，10S
				{
					gPowOffTim = 0;
                    //FLASH_PWR_OFF;
                    StoragePowerOff();                  
					gFlashPowSta = 0;
				}	
			}			
		}
			
		//写配置参数
		if(gStorageCTRL & FLASH_WR_CFG_MASK)
		{
            //配置参数有变化，或者存在第二次写入的情况
			if((!StorageStringComp((u8 *)&(gConfig.flag),(u8 *)&(gConfigBuff.flag),sizeof(t_CONFIG_PARA))) || (cfgwritecnt > 0))
			{
                gConfigBuff.flag = PARA_CFG_FLAG;
				gConfig = gConfigBuff;
                
				MX25L16EraseSector(CFG_START_ADDR); //擦除原始区		
				PT_WAIT_UNTIL(&gPTStorage,(BSPTaskStart(TASK_ID_STORAGE_TASK, 2))&(!MX25L16CheckBusy()));//等待擦除扇区完毕
                
				MX25L16EraseSector(CFG_START_ADDR + SECTOR_SIZE); //擦除备份区
				PT_WAIT_UNTIL(&gPTStorage,(BSPTaskStart(TASK_ID_STORAGE_TASK, 2))&(!MX25L16CheckBusy()));  //等待擦除扇区完毕         

				StorageStringCopy(gStorageBuff,(u8 *)&gConfig, sizeof(t_CONFIG_PARA));	//放入缓冲区中
				crc16 = CRC16Calculate((u8 *)gStorageBuff,CALC_CRC_CONFIG_SIZE - 2); /* 不计算额外的配置参数 */
                
                gConfig.crc16 = crc16;
				*(u16*)(&gStorageBuff[CALC_CRC_CONFIG_SIZE -2]) = (crc16);	//放入crc校验
               
                while(1)
                {
                    for(area = 0;area < 2;area++)
                    {
                        StorageWriteFlashWait(CFG_START_ADDR + area * SECTOR_SIZE,(u8 *)gStorageBuff,sizeof(t_CONFIG_PARA));//写入原始区和备份区
                        
                        StorageReadFlash(CFG_START_ADDR + area * SECTOR_SIZE,gFlashReadbuf,sizeof(t_CONFIG_PARA));//读取数据上来，读取校验
                        
                        if(!StorageStringComp(gFlashReadbuf,gStorageBuff,sizeof(t_CONFIG_PARA)))   //读取上来的值与写入的值不一致
                        {
                            BITCLR(areavalid,area);
                        }
                        else
                        {
                            BITSET(areavalid,area);
                        }
                    }
                    
                    if(areavalid == 0x03)   //数据都有效
                    {
                        BITCLR(gStorageErr,0);
                        BITCLR(gStorageErr,1);
                        cfgwritecnt = 0;
                        break;
                    }
                    else//由一个区或两个区存在无效数据
                    {
                        if(cfgwritecnt > 1)                     //写入2次以上，依然无效
                        {
                            cfgwritecnt = 0;
                            gStorageErr = (~areavalid) & 0x03;  //记录故障
                            break;
                        }
                        else
                        {
                            cfgwritecnt++;
                        }
                        gStorageCTRL |= (FLASH_WR_CFG_MASK);
                        PT_INIT(&gPTStorage);                   //需要重新擦除
                        return;                                 //再走一次写入，校验流程
                    }
                }
			}
			gStorageCTRL &= ~(FLASH_WR_CFG_MASK);
		}

		//写用户数据
		if(gStorageCTRL & FLASH_WR_UD_MASK)
		{
			if((!StorageStringComp((u8 *)&(gUserID.flag),(u8 *)&(gUserIDBuff.flag),sizeof(t_USER_DATA))) || (userwritecnt > 0))
			{
				gUserID = gUserIDBuff;
                
				MX25L16EraseSector(USER_START_ADDR);		
				PT_WAIT_UNTIL(&gPTStorage,(BSPTaskStart(TASK_ID_STORAGE_TASK, 2))&(!MX25L16CheckBusy()));//等待擦除扇区完毕
                
				MX25L16EraseSector(USER_START_ADDR + SECTOR_SIZE);		
				PT_WAIT_UNTIL(&gPTStorage,(BSPTaskStart(TASK_ID_STORAGE_TASK, 2))&(!MX25L16CheckBusy()));//等待擦除扇区完毕
                
				StorageStringCopy(gStorageBuff,(u8 *)&gUserID, sizeof(t_USER_DATA));	//放入缓冲区中
				crc16 = CRC16Calculate((u8 *)gStorageBuff,sizeof(t_USER_DATA) - 2);
				*(u16*)(&gStorageBuff[sizeof(t_USER_DATA) -2]) = (crc16);	//放入crc校验

                while(1)
                {
                    for(area = 0;area < 2;area++)
                    {
                        StorageWriteFlashWait(USER_START_ADDR + area * SECTOR_SIZE,(u8 *)gStorageBuff,sizeof(t_USER_DATA));//写入原始区和备份区
                        
                        StorageReadFlash(USER_START_ADDR + area * SECTOR_SIZE,gFlashReadbuf,sizeof(t_USER_DATA));//读取数据上来，读取校验
                        
                        if(!StorageStringComp(gFlashReadbuf,gStorageBuff,sizeof(t_USER_DATA)))   //读取上来的值与写入的值不一致
                        {
                            BITCLR(areavalid,area);
                        }
                        else
                        {
                            BITSET(areavalid,area);
                        }
                    }
                    
                    if(areavalid == 0x03)   //数据都有效
                    {
                        BITCLR(gStorageErr,2);
                        BITCLR(gStorageErr,3);
                        userwritecnt = 0;
                        break;
                    }
                    else//由一个区或两个区存在无效数据
                    {
                        if(userwritecnt > 1) //写入2次以上，依然无效
                        {
                            userwritecnt = 0;
                            gStorageErr = (~areavalid) & 0x0c;  //记录故障
                            break;
                        }
                        else
                        {
                            userwritecnt++;
                        }
                        gStorageCTRL |= (FLASH_WR_UD_MASK);
                        PT_INIT(&gPTStorage);               //需要重新擦除
                        return; //再走一次写入，校验流程
                    }
                }

			}
			gStorageCTRL &= ~(FLASH_WR_UD_MASK);
		}

		//写事件记录
		if(gStorageCTRL & FLASH_WR_EVE_MASK)
		{
			static u32 searliestaddr;   //最早记录事件记录编号的存储flash的地址

            gRecordInfo.lt = gRdTimeAndDate;	//获取时间

            if((gRecordInfo.earliest == 0)|| (gRecordInfo.latest == 0))
            {
                gRecordInfo.earliest = 1; //编号从1开始记录
                gRecordInfo.latest = 1;
            }
            else
            {
                if(gRecordInfo.latest >= RECORD_MAX_NUM)
                {
                    gRecordInfo.latest = 1;
                }
                else
                {
                    gRecordInfo.latest += 1;
                }
            }

            gRecordWr.t = gRecordInfo.lt;
            gRecordWr.num = gRecordInfo.latest;
            gRecordWr.crc16 = CommCalcCRC(COMM_CRC_A,(u8 *)&gRecordWr, sizeof(t_RECORD)-2,NULL,NULL);//使用NFC帧CRC校验

            everecaddr = (gRecordInfo.latest - 1) % RECORD_MAX_ITEM * RECORD_SIZE + RECORD_START_ADDR;//计算要写入事件记录的地址

            if((everecaddr % SECTOR_SIZE) == 0) //一个扇区能存储32个历史事件
            {
                searliestaddr = (gRecordInfo.earliest - 1) % RECORD_MAX_ITEM * RECORD_SIZE + RECORD_START_ADDR;

                if((searliestaddr - everecaddr) < SECTOR_SIZE)
                {
                    if((gRecordInfo.earliest > RECORD_MAX_ITEM) || (gRecordInfo.latest > RECORD_MAX_ITEM))
                    {
                        gRecordInfo.earliest += (SECTOR_SIZE/RECORD_SIZE - (searliestaddr - everecaddr) / RECORD_SIZE);
                        if(gRecordInfo.earliest > RECORD_MAX_NUM)
                        {
                            gRecordInfo.earliest -= RECORD_MAX_NUM;
                        }
                    }
                }

				MX25L16EraseSector(everecaddr);
				PT_WAIT_UNTIL(&gPTStorage,(BSPTaskStart(TASK_ID_STORAGE_TASK, 2))&(!MX25L16CheckBusy()));   //等待擦除扇区完毕
            }

            StorageStringCopy(gStorageBuff,(u8 *)&gRecordWr, sizeof(t_RECORD));	    //放入缓冲区中
			StorageWriteFlashWait(everecaddr,(u8 *)gStorageBuff,sizeof(t_RECORD));	//写数据
			//PT_SPAWN_S(&ptSTOR,&sub,mem_write);//写数据
			gStorageCTRL &= ~(FLASH_WR_EVE_MASK);
		}
        
        if(gStorageCTRL & FLASH_RD_EVE_MASK)
        {
            gStorageCTRL &= ~(FLASH_RD_EVE_MASK);
        }            
        
		//清除事件记录
        if(gStorageCTRL & FLASH_CLR_EVE_MASK)
        {
//            for(area = 0;area < 2;area++)
//            {
//                everecaddr = RECORD_INF_START_ADDR + (u32)area*SECTOR_SIZE;
//                MX25L16EraseSector(everecaddr);          
//                PT_WAIT_UNTIL(&gPTStorage,(BSPTaskStart(TASK_ID_STORAGE_TASK, 2))&(!MX25L16CheckBusy())); //等待擦除扇区完毕
//            }
            for(everecaddr = RECORD_START_ADDR;everecaddr <= RECORD_END_ADDR;everecaddr += BLOCK_SIZE)
            {
            	MX25L16EraseBlock(everecaddr);
                PT_WAIT_UNTIL(&gPTStorage,(BSPTaskStart(TASK_ID_STORAGE_TASK, 2))&(!MX25L16CheckBusy()));//等待擦除扇区完毕
            }

            gRecordInfo.earliest = 0;
            gRecordInfo.latest= 0;
            gRecordInfo.flag = EVE_REC_CFG_FLAG;

            gStorageCTRL &= ~(FLASH_CLR_EVE_MASK);

        }
        
        //保存SOC信息，测试SOC算法使用
        if(gStorageCTRL & FLASH_SOC_WR_MASK)
        {
            static u32 socaddr;   //SOC存储flash的地址
            socaddr = SOC_START_ADDR + gSOC_InfoNum * sizeof(t_SOC_EEP_INFO);              
//            if((socaddr % SECTOR_SIZE) == 0)     //已经写满一个扇区，需要重新擦除
            if(socaddr >= SOC_END_ADDR)
            {
                
                gSOC_InfoNum = 0;
                socaddr = SOC_START_ADDR;        //重新写入
                for(socaddr=SOC_START_ADDR; socaddr<=SOC_END_ADDR; socaddr += SECTOR_SIZE)                      //全部擦除
	            {
                    MX25L16EraseSector(socaddr);
                    PT_WAIT_UNTIL(&gPTStorage,(BSPTaskStart(TASK_ID_STORAGE_TASK, 2))&(!MX25L16CheckBusy()));   //等待擦除扇区完毕
	            }                    
            }
            
            SocEepRefreshInfo();                                                        //gSocEepInfo刷新数据
            StorageStringCopy(gStorageBuff,(u8 *)&gSocEepInfo, sizeof(gSocEepInfo));	//放入缓冲区中
			StorageWriteFlashWait(socaddr,(u8 *)gStorageBuff,sizeof(gSocEepInfo));	    //写数据
            gSOC_InfoNum++;
            gStorageCTRL &= ~(FLASH_SOC_WR_MASK);
        }
        
        //清除SOC信息
        if(gStorageCTRL & FLASH_SOC_CLR_MASK)
        {
            static u32 socaddr_temp;                                                                        //SOC存储flash的地址
            gSOC_InfoNum = 0;
            //SocEepRefreshInfo();            
            socaddr_temp = SOC_START_ADDR;                                                                  //重新写入
            for(socaddr_temp=SOC_START_ADDR; socaddr_temp<=SOC_END_ADDR; socaddr_temp += SECTOR_SIZE)       //全部擦除
            {
                MX25L16EraseSector(socaddr_temp);
                PT_WAIT_UNTIL(&gPTStorage,(BSPTaskStart(TASK_ID_STORAGE_TASK, 2))&(!MX25L16CheckBusy()));   //等待擦除扇区完毕
            }
            gStorageCTRL &= ~(FLASH_SOC_CLR_MASK);
            HAL_NVIC_SystemReset();            
        }

		BSPTaskStart(TASK_ID_STORAGE_TASK, 200);	//空闲时200ms周期
		PT_NEXT(&gPTStorage);
	}
	PT_END(&gPTStorage);
}

/*****************************************end of Storage.c*****************************************/
