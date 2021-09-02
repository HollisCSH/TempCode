//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Version.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 版本信息文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//头文件定义
//=============================================================================================
#include "version.h"
#include "stm32g0xx_hal.h"
//#include "BSPSystemCfg.h"
//#include "BSPCriticalZoom.h"
//#include "BSPWatchDog.h"
//#include "BSPFLASH.h"
//#include "S32K118.h"
//#include "device_registers.h"
//#include "system_S32K118.h"

//=============================================================================================
//全局变量定义
//=============================================================================================
//硬件版本号和BOOTLOADER版本号
#ifdef DEBUG_MODE_ENABLE
u8 const gHwVer[4];
u8 const gBLVer[4];
u8 gHwVerRead[4];
u8 gBLVerRead[4];
#else
//u8 const gHwVer[4] __attribute__((at(HW_VER_ADDR)));
//u8 const gBLVer[4] __attribute__((at(BL_VER_ADDR)));
u8  gHwVer[4] __attribute__((at(HW_VER_ADDR)));
u8  gBLVer[4] __attribute__((at(BL_VER_ADDR)));
u8 	gHwVerRead[4];
u8 	gBLVerRead[4];
#endif



//放在烧录文件末尾128个字节，由boot放置,APP读取
t_VERSION_INFO  cAPPInfo = 
{
    0,
    APP_LENGTH,//(u32)&cAPPInfo - 0x1000,
	FW_VER_MAIN,
	FW_VER_S1,
    FW_VER_S2,
    {FW_VER_BUILD&0xff,(FW_VER_BUILD>>8)&0xff,(FW_VER_BUILD>>16)&0xff,(FW_VER_BUILD>>24)&0xff},
	HW_VER_MAIN,
	HW_VER_SUB,
    {0},
    0,
};

//烧录文件末尾生成128字节对齐的app info，暂不使用 
//volatile t_VERSION_INFO  const cAPPInfo2Boot __attribute__((aligned (128))) __attribute__((section(".AppInfo")))=
//{
//    0,
//    APP_LENGTH,//(u32)&cAPPInfo - 0x1000,
//	FW_VER_MAIN,
//	FW_VER_S1,
//    FW_VER_S2,
//    {FW_VER_BUILD&0xff,(FW_VER_BUILD>>8)&0xff,(FW_VER_BUILD>>16)&0xff,(FW_VER_BUILD>>24)&0xff},
//	HW_VER_MAIN,
//	HW_VER_SUB,
//    {0},
//    0,
//};

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void BootloaderInit(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: BOOTLOADER初始化
//注    意	:
//=============================================================================================
void BootloaderInit(void)
{
    #ifdef DEBUG_MODE_ENABLE
    ;
    #else
    u8 i = 0;
    //S32_SCB->VTOR = (uint32_t)APPLICATION_BASE;   //中断向量偏移			
    //(void)cAPPInfo2Boot;
    cAPPInfo = *APPINFO_ADDR;   //获取由boot写入的app信息     
    for(i = 0;i < 4;i++)
    {
        gHwVerRead[i] = gHwVer[i];
        gBLVerRead[i] = gBLVer[i];
    }
    #endif
}

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
//    typedef void(*boot_entry_t)(void);

//    static u32 s_stackPointer = 0;
//    static u32 s_bootEntry = 0;
//    static boot_entry_t s_boot = 0;

//	u32 *vectorTable = (u32*)BOOTLOADER_BASE;
//    u32 sp = vectorTable[0];
//    u32 pc = vectorTable[1];
//	
//	BSPEnterCritical();         //关中断，进入屏蔽区，初始化	
//	
//    s_stackPointer = sp;
//    s_bootEntry = pc;
//    s_boot = (boot_entry_t)s_bootEntry;

//    // Change MSP and PSP
//    __set_MSP(s_stackPointer);
//    __set_PSP(s_stackPointer);

//    // Jump to bootlication
//    s_boot();

//    // Should never reach here.
//    __NOP();
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
//    typedef void(*app_entry_t)(void);

//    static u32 s_stackPointer = 0;
//    static u32 s_applicationEntry = 0;
//    static app_entry_t s_application = 0;

//	u32 *vectorTable = (u32*)APPLICATION_BASE;
//    u32 sp = vectorTable[0];
//    u32 pc = vectorTable[1];
//	
//    BSPEnterCritical();         //关中断，进入屏蔽区，初始化	
//	
//    s_stackPointer = sp;
//    s_applicationEntry = pc;
//    s_application = (app_entry_t)s_applicationEntry;

//    // Change MSP and PSP
//    __set_MSP(s_stackPointer);
//    __set_PSP(s_stackPointer);

//    // Jump to application
//    s_application();

//    // Should never reach here.
//    __NOP();
}

//=============================================================================================
//函数名称	: void SoftReset(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 软件复位
//注    意	:
//=============================================================================================
void SoftReset(void)
{
//    BSPWatchDogStop();      //关闭看门狗    
//    SystemSoftwareReset();
	HAL_NVIC_SystemReset();
}

/*****************************************end of Version.c*****************************************/
