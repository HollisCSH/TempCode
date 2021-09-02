//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SocEepHook.h
//创建人  	: Handry
//创建日期	:
//描述	    : SOC EEPROM操作模块头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef _SOCEEPHOOK_H
#define _SOCEEPHOOK_H

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTypedef.h"
#include "SocTypeDef.h"
#include "SocCapCheck.h"

//=============================================================================
//宏定义
//=============================================================================
#define     SOC_CONFIG_FLAG      (u16)(0x5AA5)    //存在SOC信息标志

#if defined(SY_PB_32000MAH_14S)
#define     SOC_DF_VER            0x0D01
#elif defined(LFP_HL_25000MAH_16S)
#define     SOC_DF_VER            0x0E02
#elif defined(SY_PB_32000MAH_16S)
#define     SOC_DF_VER            0x0F01
#elif defined(SY_PB_32000MAH_17S)
#define     SOC_DF_VER            0x1002
#elif defined(LFP_GF_25000MAH_16S)
#define     SOC_DF_VER            0x1102
#elif defined(LFP_PH_20000MAH_20S)
#define     SOC_DF_VER            0x1202
#elif defined(LFP_TB_20000MAH_20S)
#define     SOC_DF_VER            0x0302
#else
#define     SOC_DF_VER            0x0101
#endif

//=============================================================================
//数据结构声明
//=============================================================================
//SOC算法 保存EEPROM地址
//typedef enum _SOC_EEPROM_ADDR //读取
//{
////  e_SOC_Config_Flag = 10,     //SOC配置标志
////	e_SOC_Config_Flag = 0x08007A00,     //SOC配置标志		0x08007800 - 0x08007F7E 为自定义用户数据区
//	e_SOC_Config_Flag   = 0,                        //SOC配置标志		写在内部flash
//    e_SOC_TolCap_Addr   = e_SOC_Config_Flag + 2,    //总容量
//    e_SOC_RemCap_Addr   = e_SOC_Config_Flag + 4,    //剩余容量
//    e_SOC_DispSoc_Addr  = e_SOC_Config_Flag + 6,    //显示SOC                 6               
//    
//    e_SOC_HisSave_Flag  = e_SOC_DispSoc_Addr + 2,   //SOC历史容量存储标志     8      
//    e_SOC_HisDchgCap_Addr = e_SOC_HisSave_Flag + 2, //历史放电容量            10
//    e_SOC_HisChgCap_Addr= e_SOC_HisSave_Flag + 6,   //历史充电容量            14        
//    
//    
//    e_SOC_VHCapChk_Flag = e_SOC_HisChgCap_Addr+ 4,  //SOC容量高压校正配置标志 18
//    e_SOC_VHValid_Addr  = e_SOC_VHCapChk_Flag + 2,  //高压点记录有效标志
//    e_SOC_VHChgCap_Addr = e_SOC_VHCapChk_Flag + 4,  //高压点记录充电容量
//    e_SOC_VHDchgCap_Addr= e_SOC_VHCapChk_Flag + 8,  //高压点记录放电容量
//    e_SOC_VHAllCap_Addr = e_SOC_VHCapChk_Flag + 12, //高压点记录总容量
//    e_SOC_VHTimer_Addr  = e_SOC_VHCapChk_Flag + 16, //高压点记录运行时间      34         

//    e_SOC_VLCapChk_Flag = e_SOC_VHTimer_Addr  + 4,  //SOC容量低压校正配置标志 38
//    e_SOC_VLValid_Addr  = e_SOC_VLCapChk_Flag + 2,  //低压点记录有效标志
//    e_SOC_VLChgCap_Addr = e_SOC_VLCapChk_Flag + 4,  //低压点记录充电容量
//    e_SOC_VLDchgCap_Addr= e_SOC_VLCapChk_Flag + 8,  //低压点记录放电容量
//    e_SOC_VLAllCap_Addr = e_SOC_VLCapChk_Flag + 12, //低压点记录总容量
//    e_SOC_VLTimer_Addr  = e_SOC_VLCapChk_Flag + 16, //低压点记录运行时间      54
//    
//    e_SOC_DFVer_Addr    = e_SOC_VLTimer_Addr + 4,   //DF版本号                58 
//    
//    e_SOC_CalibCyc_Flag = e_SOC_DFVer_Addr + 2,     //发生容量校准时的循环次数配置标志  60
//    e_SOC_CalibCyc_Addr = e_SOC_CalibCyc_Flag + 2,  //发生容量校准时的循环次数62  
//    
//    
//    e_SOC_InfoNum_Addr  = e_SOC_CalibCyc_Addr + 2,
//    
//    e_SOC_Reserved_Addr = e_SOC_InfoNum_Addr + 2,
//    e_SOC_Max_Addr      = e_SOC_Reserved_Addr + 2,                          //64

//}e_SOC_EEP_ADDR;
//=============================================================================
//数据结构声明
//=============================================================================
//SOC算法 保存EEPROM地址
typedef enum _SOC_EEPROM_ADDR
{
    e_SOC_Config_Flag = 0,     //SOC配置标志
    
    e_SOC_DFVer_Addr    = e_SOC_Config_Flag + 2,        //DF版本号

    e_SOC_TolCap_Addr   = e_SOC_Config_Flag + 4,        //总容量
    e_SOC_RemCap_Addr   = e_SOC_Config_Flag + 6,        //剩余容量
    e_SOC_DispSoc_Addr  = e_SOC_Config_Flag + 8,        //显示SOC
    
    e_SOC_HisSave_Flag  = e_SOC_DispSoc_Addr + 2,       //SOC历史容量存储标志
    e_SOC_HisDchgCap_Addr = e_SOC_HisSave_Flag + 2,     //历史放电容量
    e_SOC_HisChgCap_Addr= e_SOC_HisSave_Flag + 6,       //历史充电容量    
    
    e_SOC_VHCapChk_Flag = e_SOC_HisChgCap_Addr + 4,     //SOC容量高压校正配置标志
    e_SOC_VHValid_Addr  = e_SOC_VHCapChk_Flag + 2,      //高压点记录有效标志
    e_SOC_VHChgCap_Addr = e_SOC_VHCapChk_Flag + 4,      //高压点记录充电容量
    e_SOC_VHDchgCap_Addr= e_SOC_VHCapChk_Flag + 8,      //高压点记录放电容量
    e_SOC_VHAllCap_Addr = e_SOC_VHCapChk_Flag + 12,     //高压点记录总容量
    e_SOC_VHTimer_Addr  = e_SOC_VHCapChk_Flag + 16,     //高压点记录运行时间

    e_SOC_VLCapChk_Flag = e_SOC_VHTimer_Addr + 4,       //SOC容量低压校正配置标志
    e_SOC_VLValid_Addr  = e_SOC_VLCapChk_Flag + 2,      //低压点记录有效标志
    e_SOC_VLChgCap_Addr = e_SOC_VLCapChk_Flag + 4,      //低压点记录充电容量
    e_SOC_VLDchgCap_Addr= e_SOC_VLCapChk_Flag + 8,      //低压点记录放电容量
    e_SOC_VLAllCap_Addr = e_SOC_VLCapChk_Flag + 12,     //低压点记录总容量
    e_SOC_VLTimer_Addr  = e_SOC_VLCapChk_Flag + 16,     //低压点记录运行时间

        
    e_SOC_CalibCyc_Flag = e_SOC_VLTimer_Addr + 4,       //发生容量校准时的循环次数配置标志  60
    e_SOC_CalibCyc_Addr = e_SOC_CalibCyc_Flag + 2,      //发生容量校准时的循环次数62 
    e_SOC_Max_Addr      = e_SOC_CalibCyc_Addr + 2,      //共68个字节

}e_SOC_EEP_ADDR;

//SOC算法 保存EEPROM信息结构体
#pragma pack(push)
#pragma pack(1)
typedef struct
{
    u16 SOC_Config_Flag;    //SOC配置标志

    u16 SOC_DFVer;          //DF版本号   
    u16 SOC_TolCap;         //总容量
    u16 SOC_RemCap;         //剩余容量
    u16 SOC_DispSoc;        //显示SOC

    u16 SOC_HisSave_Flag;   //SOC历史容量存储标志
    u32 SOC_HisDchgCap;     //历史放电容量
    u32 SOC_HisChgCap;      //历史充电容量

    u16 SOC_VHCapChk_Flag;  //SOC容量高压校正配置标志
    u16 SOC_VHValid;        //高压点记录有效标志
    u32 SOC_VHChgCap;       //高压点记录充电容量
    u32 SOC_VHDchgCap;      //高压点记录放电容量
    u32 SOC_VHAllCap;       //高压点记录总容量
    u32 SOC_VHTimer;        //高压点记录运行时间

    u16 SOC_VLCapChk_Flag;  //SOC容量低压校正配置标志
    u16 SOC_VLValid;        //低压点记录有效标志
    u32 SOC_VLChgCap;       //低压点记录充电容量
    u32 SOC_VLDchgCap;      //低压点记录放电容量
    u32 SOC_VLAllCap;       //低压点记录总容量
    u32 SOC_VLTimer;        //低压点记录运行时间

    u16 SOC_CalibCyc_Flag;  //发生容量校准时的循环次数配置标志  60
    u16 SOC_CalibCyc_Addr;  //发生容量校准时的循环次数62 
//    u16 SOC_InfoNum;        //保存到FLASH条数
//    u16 SOC_Reserved;       //SOC预留，4字节对其
}t_SOC_EEP_INFO;
#pragma pack(pop)

extern t_SOC_EEP_INFO gSocEepInfo;     //保存到FLASH的信息

//=============================================================================================
//声明接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void SocReadEepInit(void) 
//输入参数	: void  
//输出参数	: void
//函数功能	: SOC读取EEP初始化
//注意事项	:  
//=============================================================================================
void SocReadEepInit(void);
    
//=============================================================================================
//函数名称	: u16 SocReadEepValueByAddr16(e_SOC_EEP_ADDR socaddr)
//输入参数	: socaddr：SOC存储EEP地址    
//输出参数	: void
//函数功能	: SOC读取16位 EEPROM数据
//注意事项	:  
//=============================================================================================
u16 SocReadEepValueByAddr16(e_SOC_EEP_ADDR socaddr);

//=============================================================================================
//函数名称	: u32 SocReadEepValueByAddr32(e_SOC_EEP_ADDR socaddr)
//输入参数	: socaddr：SOC存储EEP地址  
//输出参数	: void
//函数功能	: SOC读取32位 EEPROM数据
//注意事项	:  
//=============================================================================================
u32 SocReadEepValueByAddr32(e_SOC_EEP_ADDR socaddr);

//=============================================================================================
//函数名称	: u8 SocWriteEepValueByAddr16(e_SOC_EEP_ADDR socaddr,u16 data)
//输入参数	: socaddr：SOC存储EEP地址  data：写入的数据
//输出参数	: False:写失败；True：写成功
//函数功能	: SOC写16位 EEPROM数据
//注意事项	:  
//=============================================================================================
u8 SocWriteEepValueByAddr16(e_SOC_EEP_ADDR socaddr,u16 data);

//=============================================================================================
//函数名称	: u8 SocWriteEepValueByAddr32(e_SOC_EEP_ADDR socaddr,u32 data)
//输入参数	: socaddr：SOC存储EEP地址  data：写入的数据
//输出参数	: False:写失败；True：写成功
//函数功能	: SOC写32位 EEPROM数据
//注意事项	:  
//=============================================================================================
u8 SocWriteEepValueByAddr32(e_SOC_EEP_ADDR socaddr,u32 data);

//=============================================================================================
//函数名称	: u8 SocWriteEepClearAllData(void)
//输入参数	: void
//输出参数	: False:清除失败；True：清除成功
//函数功能	: SOC清除EEPROM数据
//注意事项	:  
//=============================================================================================
u8 SocWriteEepClearAllData(void);
    
//=============================================================================================
//函数名称	: void SocEepSaveVHCheckPoint(void)
//输入参数	: VhPoint:高压点指针
//输出参数	: True:写成功；False:写失败
//函数功能	: 写高压点数据到EEP中
//注意事项	:  
//=============================================================================================
u8 SocEepSaveVHCheckPoint(t_CapCheck *VhPoint);

//=============================================================================================
//函数名称	: void SocEepSaveVLCheckPoint(void)
//输入参数	: VlPoint:低压点指针
//输出参数	: True:写成功；False:写失败
//函数功能	: 写低压点数据到EEP中
//注意事项	:  
//=============================================================================================
u8 SocEepSaveVLCheckPoint(t_CapCheck *VlPoint);

//=============================================================================================
//函数名称	: void SocSetEepFirstFlag(void)
//输入参数	: void
//输出参数	: void
//函数功能	: SOC算法初始运行标志置1
//注意事项	:  
//=============================================================================================
void SocSetEepFirstFlag(void);

//=============================================================================================
//函数名称	: void SocClrEepFirstFlag(void)
//输入参数	: void
//输出参数	: void
//函数功能	: SOC算法初始运行标志清零
//注意事项	:  
//=============================================================================================
void SocClrEepFirstFlag(void);

//=============================================================================================
//函数名称	: u8 SocGetEepFirstFlag(void)
//输入参数	: void
//输出参数	: SOC算法初始运行标志
//函数功能	: 获取SOC算法初始运行标志
//注意事项	:  
//=============================================================================================
u8 SocGetEepFirstFlag(void);

//=============================================================================================
//函数名称	: void SocEepRefreshInfo(void) 
//输入参数	: void  
//输出参数	: void
//函数功能	: SOC写入EEP参数刷新
//注意事项	:  
//=============================================================================================
void SocEepRefreshInfo(void);

#endif

/*****************************************end of SocEepHook.h*****************************************/
