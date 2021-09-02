//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名		: BootLoader.c
//创建人		: Handry
//创建日期	:
//描述	    : BOOT升级源码文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人		:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BootLoader.h"
//#include "BSPWatchDog.h"
//#include "BSPFlash.h"
//#include "BSPPIT.h"
//#include "BSPLowPow.h"
//#include "BSPICS.h"
//#include "S32K118.h"
//#include "FM11NC08.h"
//#include "BSPTimer.h"
//#include "BSPCan.h"
//#include "interrupt_manager.h"
#include "version.h"
#include "crc32.h"
#include "crc.h"
#include "BSP_IWDG.h"
#include "BSP_FLASH.h"
#include "BSP_SYSCLK.h"
#include "BSP_GPIO.h"
#include "BSP_UART.h"
#include "BSP_EXIT.h"
#include "BSP_CAN.h"
#include "BSP_TIM.h"
#include "CanComm.h"
#include "xmodem.h"
#include "Comm.h"
//=============================================================================================
//全局变量
//=============================================================================================
//BOOTLOADER CRC
#if defined ( __ICCARM__ )
#pragma location=0x200000bc
__no_init uint16_t bootloaderCRC;
#elif defined (__CC_ARM)
uint16_t bootloaderCRC ;//__attribute__((at(0x200000bc)));
#elif defined (__GNUC__)
uint16_t bootloaderCRC __attribute__((at(0x200000bc)));
#else
#error Undefined toolkit, need to define alignment
#endif

#ifdef DEBUG
#define UpData_TimeOut 100000
#else
#define UpData_TimeOut 60000
#endif
//回复的字符串
//uint8_t crcString[]     = "\r\nCRC: ";
//uint8_t newLineString[] = "\r\n";
//uint8_t readyString[]   = "\r\nupdateReady\r\n";
//uint8_t okString[]      = "\r\nOK\r\n";
//uint8_t failString[]    = "\r\nFail\r\n";
//uint8_t unknownString[] = "\r\n?\r\n";

const uint8_t info_appcrc[] = "\r\nAppCRC: ";
const uint8_t info_chkcrc[] = "\r\nCheckCRC: ";
const uint8_t info_valid[] = "Valid";
const uint8_t info_invalid[] = "Invalid";
const uint8_t info_bootok[] = "\r\nBoot OK!\r\n->";
const uint8_t info_booterr[] = "\r\nBoot Err!\r\n->";
const uint8_t info_next[]="\r\n->";

const uint8_t info_ver[]="\r\nBL Ver: ";
const uint8_t info_hw[]="\r\nHW Ver: ";//0D.10
const uint8_t info_fw[]="\r\nFW Ver: ";//01.01.01.12345678

//硬件版本和软件版本保存的地址
#define HWVER_ADDR ((uint8_t const*)0x08000FFC)
#define BLVER_ADDR ((uint8_t const*)0x08000FF8)
const uint8_t  hw_ver[4] __attribute__((at(0x08000FFC))) = {HW_MAIN_VER,HW_SUB_VER,0,0};
const uint8_t  bl_ver[4] __attribute__((at(0x08000FF8))) = {BL_MAIN_VER,BL_SUB_VER,0,0};
//#define HWVER_ADDR ((uint8_t const*)0x00000FFC)
//#define BLVER_ADDR ((uint8_t const*)0x00000FF8)
//uint8_t const hw_ver[4] __attribute__((at(0x00000FFC))) = {HW_MAIN_VER,HW_SUB_VER,0,0};
//uint8_t const bl_ver[4] __attribute__((at(0x00000FF8))) = {BL_MAIN_VER,BL_SUB_VER,0,0};

//发送缓冲区
static uint8_t MSG_BUF[128];
u32 gUpdateFailTime = 0;    //升级时间累计时间

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: u8 BootloaderCheckApp(void)
//输入参数	: void
//输出参数	: APP层复位向量地址是否有效
//静态变量	: TRUE：有效；FALSE:无效
//功    能	: 检查APP层复位向量地址是否有效
//注    意	:
//=============================================================================================
u8 BootloaderCheckFirmware(void)
{
  uint32_t pc;
  pc = *((uint32_t *) BOOTLOADER_SIZE + 1);
//#ifndef NDEBUG
//  if (!printedPC)
//  {
//    printedPC = true;
//    _Debug_Printf("Application Reset vector = 0x%x \r\n", pc);
//  }
//#endif
  if (pc < MAX_SIZE_OF_FLASH)
  {
    return TRUE;
  }
  return FALSE;
}

//=============================================================================================
//函数名称	: u8 BootloaderCheckAppInfo(void)
//输入参数	: void
//输出参数	: APP版本信息区校验是否有效
//静态变量	: TRUE：有效；FALSE:无效
//功    能	: 检查APP版本信息区校验是否有效
//注    意	:
//=============================================================================================
u8 BootloaderCheckAppInfo(void)
{
    uint32_t crc;

    crc = Crc32_Calc(CRC32_CHECK_CODE,(uint8_t*)&cAPPInfo,128-4);
    
    if(cAPPInfo.info_crc != crc)
    {
        return FALSE;
    }
    if(cAPPInfo.length > (MAX_SIZE_OF_FLASH - BOOTLOADER_SIZE - 1024UL))
    {
        return FALSE;
    }

    return TRUE;
}

//=============================================================================================
//函数名称	: u8 BootloaderCheckAppCRC(u32 *crc)
//输入参数	: void
//输出参数	: APP的CRC校验是否有效
//静态变量	: TRUE：有效；FALSE:无效
//功    能	: 检查APP的CRC校验是否有效
//注    意	:
//=============================================================================================
u8 BootloaderCheckAppCRC(uint32_t *crc)
{
    *crc = Crc32_Calc(CRC32_CHECK_CODE,(uint8_t *)APPLICATION_BASE,cAPPInfo.length);

    
    if((0 != *crc) || (0xffffffff != *crc))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//=============================================================================================
//函数名称	: void BootloaderCheckApp(void)
//输入参数	: void
//输出参数	: 检查APP层有效，是否校验成功
//静态变量	: TRUE：有效；FALSE:无效
//功    能	: BOOTLOADER初始化函数
//注    意	:
//=============================================================================================
u8 BootloaderCheckApp(void)
{
    uint32_t crc1;

    if(BootloaderCheckFirmware() && BootloaderCheckAppInfo()                    //APP复位向量有效，APP CRC校验有效
       && (*(HWVER_ADDR)==cAPPInfo.hwmain) && (*(HWVER_ADDR+1)==cAPPInfo.hwsub) //硬件版本号一致
       && BootloaderCheckAppCRC(&crc1) && (cAPPInfo.appcrc == crc1))            //APP info校验有效
    {
        return TRUE;
    }
    
    return FALSE; 
}

//=============================================================================================
//函数名称	: void BootloaderInit(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: BOOTLOADER初始化函数
//注    意	:
//=============================================================================================
//void BootloaderInit(void)
//{
////	uint32_t cnt;
//	
//	bootloaderCRC  = CRC_calc((void *)BOOTLOADER_START_ADDR,
//							(void *)BOOTLOADER_END_ADDR);
////	bootloaderCRC |= CRC_calc((void *)(DEVINFO_START_ADDR + 2),
////							// Skip first 2 bytes, they are DEVINFO crc.
////							(void *)DEVINFO_END_ADDR) << 16;
//	Crc32_Init();//初始化CRC32表
//	gUpdateFailTime = 0;
//    
//	if (BootloaderCheckApp())    //App无效则停留在BOOT
//    {
//        //应用程序标志有效,停留boot标志无效
//        if((BOOT_PRO_FLAG == BSPEEPReadOneWord(e_BOOT_PRO_FLAG_ADDR))
//            && (BOOT_STAY_FLAG != BSPEEPReadOneWord(e_BOOT_STAY_FLAG_ADDR)))
//        {
//            //擦除BOOT标志
//            BSPEEPWriteOneWord(e_BOOT_STAY_FLAG_ADDR,0xFFFF);              
//            JumpToApplication();
//        }
//        //应用程序跳转到boot的，希望进行升级
//        else if((BOOT_PRO_FLAG == BSPEEPReadOneWord(e_BOOT_PRO_FLAG_ADDR))
//            && (BOOT_STAY_FLAG == BSPEEPReadOneWord(e_BOOT_STAY_FLAG_ADDR)))
//        {
//            //本身包括应用程序，接收到命令后进入Boot，进入一次后进行清除
//            //写入停留boot标志
//            BSPEEPWriteOneWord(e_BOOT_STAY_FLAG_ADDR,0xFFFF);
//            return;
//        }
//        //烧录的是FLASH文件而非OTA文件
//        else if((BOOT_PRO_FLAG != BSPEEPReadOneWord(e_BOOT_PRO_FLAG_ADDR))
//               && (BOOT_STAY_FLAG != BSPEEPReadOneWord(e_BOOT_STAY_FLAG_ADDR)))
//        {
//            BSPEEPWriteOneWord(e_BOOT_PRO_FLAG_ADDR,BOOT_PRO_FLAG);
//            BSPEEPWriteOneWord(e_BOOT_STAY_FLAG_ADDR,0xFFFF);
//            JumpToApplication();
//        }
//        //无应用程序标志，有boot停留标志，程序有异常，写入应用程序标志后跳转到app
//        else
//        {
//            BSPEEPWriteOneWord(e_BOOT_PRO_FLAG_ADDR,BOOT_PRO_FLAG);
//            BSPEEPWriteOneWord(e_BOOT_STAY_FLAG_ADDR,0xFFFF);
//            JumpToApplication();
//        }
//        
//        //非上电复位，复位管脚复位，软件复位均直接启动应用程序,因其他原因导致的复位不能停留在BOOT
//        //if((RMU->RSTCAUSE & (RMU_RSTCAUSE_SYSREQRST|RMU_RSTCAUSE_EXTRST|RMU_RSTCAUSE_PORST))==0)
//        if((RCM->SSRS & (RCM_SSRS_SPOR_MASK | RCM_SSRS_SPIN_MASK | RCM_SSRS_SSW_MASK))==0)
//        {
//            //跳转到应用层
//            JumpToApplication();
//        }        
//        
////        cnt=100;
////        while(cnt--)
////        {
////            //if((GPIO->P[NFC_INT_PORT].DIN & NFC_INT_MASK) != 0)
////			if(0 != (BSPGPIOGetPin(NFC_INT_PORT,NFC_INT_PIN)))	//有一次检测到为高则退出bootloader
////			{
////                //跳转到应用层
////                JumpToApplication();
////                
////            }
////        }
//    }
//    else
//    {
//        //擦除应用层标志
//        BSPEEPWriteOneWord(e_BOOT_PRO_FLAG_ADDR,0xFFFF);
//        //擦除BOOT标志
//        BSPEEPWriteOneWord(e_BOOT_STAY_FLAG_ADDR,0xFFFF);        
//    }
//}
void BootloaderInit(void)
{
//	uint32_t cnt;
	
	bootloaderCRC  = CRC_calc((void *)BOOTLOADER_START_ADDR,
							(void *)BOOTLOADER_END_ADDR);
	Crc32_Init();//初始化CRC32表
	gUpdateFailTime = 0;
	if (BootloaderCheckApp())    //App无效则停留在BOOT
    {
        //应用程序标志有效,停留boot标志无效
        if((BOOT_PRO_FLAG == ReadFlagFmFlash(e_BOOT_PRO_FLAG_ADDR))
            && (BOOT_STAY_FLAG != ReadFlagFmFlash(e_BOOT_STAY_FLAG_ADDR)))
        {
            //擦除BOOT标志
            WriteFlagToFlash(e_BOOT_STAY_FLAG_ADDR,CLEAR_FLAG);              
            JumpToApplication();
        }
        //应用程序跳转到boot的，希望进行升级
        else if((BOOT_PRO_FLAG == ReadFlagFmFlash(e_BOOT_PRO_FLAG_ADDR))
            && (BOOT_STAY_FLAG == ReadFlagFmFlash(e_BOOT_STAY_FLAG_ADDR)))
        {
            //本身包括应用程序，接收到命令后进入Boot，进入一次后进行清除
            //写入停留boot标志
            WriteFlagToFlash(e_BOOT_STAY_FLAG_ADDR,CLEAR_FLAG);
            return;
        }
        //烧录的是FLASH文件而非OTA文件
        else if((BOOT_PRO_FLAG != ReadFlagFmFlash(e_BOOT_PRO_FLAG_ADDR))
               && (BOOT_STAY_FLAG != ReadFlagFmFlash(e_BOOT_STAY_FLAG_ADDR)))
        {
            WriteFlagToFlash(e_BOOT_PRO_FLAG_ADDR,BOOT_PRO_FLAG);
            WriteFlagToFlash(e_BOOT_STAY_FLAG_ADDR,CLEAR_FLAG);
            JumpToApplication();
        }
        //无应用程序标志，有boot停留标志，程序有异常，写入应用程序标志后跳转到app
        else
        {
            WriteFlagToFlash(e_BOOT_PRO_FLAG_ADDR,BOOT_PRO_FLAG);
            WriteFlagToFlash(e_BOOT_STAY_FLAG_ADDR,CLEAR_FLAG);
            JumpToApplication();
        }
        
        //非上电复位，复位管脚复位，软件复位均直接启动应用程序,因其他原因导致的复位不能停留在BOOT
        //if((RMU->RSTCAUSE & (RMU_RSTCAUSE_SYSREQRST|RMU_RSTCAUSE_EXTRST|RMU_RSTCAUSE_PORST))==0)
        //if((RCM->SSRS & (RCM_SSRS_SPOR_MASK | RCM_SSRS_SPIN_MASK | RCM_SSRS_SSW_MASK))==0)
        /*RCC_FLAG_LPWRRST	进入非法停止、待机或关断模式而发生复位
            RCC_FLAG_WWDGRST	发生窗口看门狗复位
            RCC_FLAG_IWDGRST	发生独立看门狗复位
            RCC_FLAG_SFTRST		发生软件复位时
            RCC_FLAG_PWRRST		发生 BOR(欠压) 或 POR/PDR	（上电/掉电）
            RCC_FLAG_PINRST		发生来自 NRST 引脚的复位
            RCC_FLAG_OBLRST		发生来自选项字节加载的复位
        */
        if((RCC->CSR & (RCC_FLAG_SFTRST | RCC_FLAG_PWRRST | RCC_FLAG_PINRST))==0)
        {
            //跳转到应用层
            JumpToApplication();
        }        
    }
    else
    {
        //擦除应用层标志
        WriteFlagToFlash(e_BOOT_PRO_FLAG_ADDR,CLEAR_FLAG);
        //擦除BOOT标志
        WriteFlagToFlash(e_BOOT_STAY_FLAG_ADDR,CLEAR_FLAG);        
    }
}
///** \brief  Set Process Stack Pointer

//    This function assigns the given value to the Process Stack Pointer (PSP).

//    \param [in]    topOfProcStack  Process Stack Pointer value to set
// */
//static inline void __set_PSP(uint32_t topOfProcStack)
//{
//  register uint32_t __regProcessStackPointer  __asm("psp");
//  __regProcessStackPointer = topOfProcStack;
//}


///** \brief  Set Main Stack Pointer

//    This function assigns the given value to the Main Stack Pointer (MSP).

//    \param [in]    topOfMainStack  Main Stack Pointer value to set
// */
//static inline void __set_MSP(uint32_t topOfMainStack)
//{
//  register uint32_t __regMainStackPointer     __asm("msp");
//  __regMainStackPointer = topOfMainStack;
//}

//=============================================================================================
//函数名称	: void JumpToBootloader(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 跳转到BOOTLOADER
//注    意	:
//=============================================================================================
void JumpToBootloader(void)
{
    typedef void(*boot_entry_t)(void);

    static u32 s_stackPointer = 0;
    static u32 s_bootEntry = 0;
    static boot_entry_t s_boot = 0;

	u32 *vectorTable = (u32*)BOOTLOADER_BASE;
    u32 sp = vectorTable[0];
    u32 pc = vectorTable[1];
    
	//关中断，进入屏蔽区
	INT_SYS_DisableIRQGlobal();         
	
    s_stackPointer = sp;
    s_bootEntry = pc;
    s_boot = (boot_entry_t)s_bootEntry;

    //改变MSP和PSP
    __set_MSP(s_stackPointer);
    __set_PSP(s_stackPointer);

    //跳转到boot地址
    s_boot();

    //正常不应执行到此处
    __nop();
}

/*******************************************************************
** function: clean the bootloader used RAM  for NVM driver
** input: none
** output: none
********************************************************************/
void CleanRAM(void)
{
    uint32_t cnt;
    uint32_t *pRam;

    /*
     * clean system SRAM
     */
    cnt = (0x5c00 - 0x400)/4; 
    pRam = (uint32_t*)0x20000000;

    while(cnt--)
        *pRam++ = 0;

}

//=============================================================================================
//函数名称	: void JumpToApplication(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 跳转到Application应用层
//注    意	:
//=============================================================================================
void JumpToApplication(void)
{
    uint32_t time = 500000;    
    typedef void(*app_entry_t)(void);

    static u32 s_stackPointer = 0;
    static u32 s_applicationEntry = 0;
    static app_entry_t s_application = 0;
    
    CleanRAM();
    //BSPWatchDogStop();
    #ifndef  BOOT_DIRECT_JUMP   //如果直接跳转 则不使用看门狗
    MX_IWDG_Init(4095);
    FeedIDog();
    #endif
	u32 *vectorTable = (u32*)APPLICATION_BASE;
    u32 sp = vectorTable[0];
    u32 pc = vectorTable[1];
	
	//关中断，进入屏蔽区
	INT_SYS_DisableIRQGlobal();  
    
    s_stackPointer = sp;
    s_applicationEntry = pc;
    s_application = (app_entry_t)s_applicationEntry;

    //改变MSP和PSP
    __set_MSP(s_stackPointer);
    __set_PSP(s_stackPointer);
//    S32_SCB->VTOR = APPLICATION_BASE;
    SCB->VTOR = APPLICATION_BASE;
    
	while(time--)
    {
    
    };      //必须等待一会
    
    //跳转到app地址
    s_application();

    //正常不应执行到此处
    __nop();
}

//=============================================================================================
//函数名称: void BootLoaderDealTask(void)
//输	入: void
//输	出: void
//静态变量:
//功	能: boot处理主函数
//=============================================================================================
void BootLoaderDealTask(void)
{
	uint32_t flashSize;
	uint8_t *returnString;
	uint32_t crc1;
	uint16_t cnt;
	uint8_t cmd;
	u8 crcvalid;
	
	flashSize = BOOTLOADER_BASE + MAX_FLASH_SIZE - 1024;     //擦除的flash末尾地址
	
	while(1)                           
	{

		cnt = 1;
		//if(!FM11NC08FrameRx(&cmd,&cnt,TIMEOUTMS(30000)))
		if(!CanCommFrameRx(&cmd,&cnt,TIMEOUTMS(UpData_TimeOut)) || (GET_TICKS() - gUpdateFailTime > CAN_UPDATE_FAIL_TIME))
		{
			if(BootloaderCheckApp())
			{
                CanCommFrameTx(BOOT_FUNCODE, info_bootok,14,TIMEOUTMS(100));
				JumpToApplication();
			}
			// 超时后进入睡眠模式,等待CAN_INT唤醒
			SET_MODU_STA(0);
            MX_FDCAN2_DeInit();
			CAN_POWER_DISABLE();
            BSPGPIODeInit();
			
			HAL_NVIC_EnableIRQ(MODULE_INT_IRQ);                       //开启外部中断
            FeedIDog();                                               //喂狗
            
//            if(READ_BIT(DBG->APBFZ1, DBG_APB_FZ1_DBG_IWDG_STOP) == 0) //看门狗冻结位是否已置位
//            {   
//                #ifdef DEBUG
//                static uint8_t sleep_flag = 0;
//                _Debug_Printf("sleep time:%d",sleep_flag++);  
//                #endif                
//                HAL_FLASH_Unlock();             
//                HAL_FLASH_OB_Unlock();        //解锁FLASH 
//                __HAL_FREEZE_IWDG_DBGMCU();   //冻结看门狗计数                           
//                HAL_FLASH_OB_Lock();          //上锁FLASH 
//                HAL_FLASH_Lock();
//            }
//			HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

//            {   //该段为休眠 喂狗 代码 唤醒喂狗增加功耗
//                MX_LPTIM1_Init();                   //使能低功耗定时器
//                
//                SleepSet:
//                HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI); //进入停止模式
//                if( Get_TIM_FeedDogFlag()           //定时器唤醒  
//                    &&(GET_MODU_INT_STA() == 0)     //没有外部唤醒
//                    )                               
//                {
//                    FeedIDog();	                    //喂狗任务
//                    BSP_TIM_ClearFeedDogFlag();     //清除喂狗标志
//                    goto SleepSet;                  //继续睡眠
//                }
//                MX_LPTIM1_DeInit();                 //跳出睡眠关闭定时器
//            }
            {   
                #ifdef DEBUG
                static uint8_t sleep_flag = 0;
                _Debug_Printf("sleep time:%d",sleep_flag++);  
                #endif
                static FLASH_OBProgramInitTypeDef OptionsBytesStruct;
                HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);
                {
                    if(OptionsBytesStruct.USERConfig & 0x00020000)  //bit17为0时停止模式下冻结看门狗
                    {
                        OptionsBytesStruct.USERConfig &= 0xFFFDFFFF;//      
                        HAL_FLASH_Unlock();             
                        HAL_FLASH_OB_Unlock();                      //解锁FLASH 
                        HAL_FLASHEx_OBProgram(&OptionsBytesStruct); //flash为ff时候才能写入，所以先擦除，网上很多不成功就是没擦除                
                        HAL_FLASH_OB_Lock();                        //上锁FLASH 
                        HAL_FLASH_Lock();
                        HAL_FLASH_OB_Launch();                      //会导致重启一次  永久不会再进此处          
                    }
                }                               
            }
            SleepSet:
            HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI); //进入停止模式
            
			if(GET_MODU_INT_STA() == 1)
			{
				SystemClock_Config();
				SET_MODU_STA(0);
                BSPGPIOInit();
				CAN_POWER_ENABLE();
				HAL_NVIC_DisableIRQ(MODULE_INT_IRQ);
				MX_FDCAN2_Init();
			}
            else
                goto SleepSet;                      //非接入唤醒则继续睡眠
			continue;			           
		}
		
        switch(cmd)
        {
            case 'U':
                
                XMODEM_download(APPLICATION_START_ADDR, flashSize);
                break;
            
            case 'B':
                
                if(BootloaderCheckApp())
                {
                    CanCommFrameTx(BOOT_FUNCODE, info_bootok,14,TIMEOUTMS(100));
                    JumpToApplication();
                }
                else
                {
                    CanCommFrameTx(BOOT_FUNCODE, info_booterr,15,TIMEOUTMS(100));
                }
                break;
                
            default:
               
                returnString = MSG_BUF;
            
                //打印出BootLoader版本号
                StringCopy(returnString, info_ver, 10);
                returnString += 10;
                ChangeU8ToCharString(returnString,BLVER_ADDR,1);
                returnString += 2;
                *returnString++ = '.';
                ChangeU8ToCharString(returnString,BLVER_ADDR+1,1);
                returnString += 2;
            
                //打印出硬件版本号
                StringCopy(returnString, info_hw, 10);
                returnString += 10;
                ChangeU8ToCharString(returnString,HWVER_ADDR,1);
                returnString += 2;
                *returnString++ = '.';
                ChangeU8ToCharString(returnString,HWVER_ADDR+1,1);
                returnString += 2;
            
                //打印出固件版本号
                StringCopy(returnString, info_fw, 10);
                returnString += 10;

                crcvalid = BootloaderCheckAppInfo();
                if(crcvalid)
                {
                    ChangeU8ToCharString(returnString,(uint8_t const*)&cAPPInfo.swmain,1);
                    returnString += 2;
                    *returnString++ = '.';
                    ChangeU8ToCharString(returnString,(uint8_t const*)&cAPPInfo.swsub1,1);
                    returnString += 2;
                    *returnString++ = '.';
                    ChangeU8ToCharString(returnString,(uint8_t const*)&cAPPInfo.swsub2,1);
                    returnString += 2;
                    *returnString++ = '.';
                    for(cnt=3;cnt<=3;cnt--)//cnt为unsigned
                    {
                        ChangeU8ToCharString(returnString,(uint8_t const*)&cAPPInfo.swbuild[cnt],1);
                        returnString += 2;
                    }
                }
                else
                {
                    StringCopy(returnString, info_invalid, 7);
                    returnString += 7;
                }
                
                //打印出APP的CRC
                StringCopy(returnString, info_appcrc, 10);
                returnString += 10;
                if(crcvalid)
                {
                    *returnString++ = '0';
                    *returnString++ = 'x';
                    BootloaderCheckAppCRC(&crc1);
                    ChangeU32ToChar(returnString,crc1);
                    returnString += 8;
                }
                else
                {
                    StringCopy(returnString, info_invalid, 7);
                    returnString += 7;
                }
                
                 //打印出检测到的CRC
                StringCopy(returnString, info_chkcrc, 12);
                returnString += 12;
                if(crcvalid && (cAPPInfo.appcrc == crc1))
                {
                    StringCopy(returnString, info_valid, 5);
                    returnString += 5;
                }
                else
                {
                    StringCopy(returnString, info_invalid, 7);
                    returnString += 7;
                }
                StringCopy(returnString, info_next, 4);
                returnString += 4;

                //FM11NC08FrameTx(c_FUNCODE, MSG_BUF,returnString - MSG_BUF,TIMEOUTMS(100));
                CanCommFrameTx(BOOT_FUNCODE, MSG_BUF,returnString - MSG_BUF,TIMEOUTMS(100));
                break;
        }
	}
}

/*****************************************end of BootLoader.c*****************************************/
