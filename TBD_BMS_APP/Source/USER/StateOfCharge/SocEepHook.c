//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SocEepHook.c
//创建人  	: Handry
//创建日期	:
//描述	    : SOC EEPROM操作模块
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SocTypeDef.h"
#include "SocEepHook.h"
#include "CurrIntegral.h"
#include "SocCapCalc.h"
#include "SocSlideShow.h"
#include "SocOCVCorr.h"
#include "SocLTCompensate.h"
#include "BatteryInfo.h"
//#include "BSPFlash.h"
//#include "BSPSysTick.h"
#include "BSP_Flash.h"
#include "Storage.h"

//=============================================================================================
//全局变量
//=============================================================================================
static u8 sFirstEepFlag = False;
t_SOC_EEP_INFO gSocEepInfo;     //保存到FLASH的信息
//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void SocReadEepInit(void) 
//输入参数	: void  
//输出参数	: void
//函数功能	: SOC读取EEP初始化
//注意事项	:  
//=============================================================================================
void SocReadEepInit(void)
{
    u8 ret = False;
    
    //标志有效，说明运行SOC算法程序
    if(SOC_CONFIG_FLAG == SocReadEepValueByAddr16(e_SOC_Config_Flag) && SOC_DF_VER == SocReadEepValueByAddr16(e_SOC_DFVer_Addr))
    {
        //将EEP的数据读取上来
        gSOCInfo.tolcap = SocReadEepValueByAddr16(e_SOC_TolCap_Addr); //总容量
        
        gSOCInfo.remaincap = SocReadEepValueByAddr16(e_SOC_RemCap_Addr);  //剩余容量
        
        gSOCInfo.displaySOC = SocReadEepValueByAddr16(e_SOC_DispSoc_Addr);    //显示SOC
        
        if(SOC_CONFIG_FLAG == SocReadEepValueByAddr16(e_SOC_CalibCyc_Flag))
            gSOCInfo.Calibcycles = SocReadEepValueByAddr16(e_SOC_CalibCyc_Addr);  //发生容量校准时的循环次数
        else
            gSOCInfo.Calibcycles = 0;

        if(gSOCInfo.displaySOC > 1000)
        {
            gSOCInfo.displaySOC = 1000;
        }
        
        SocCalcInit();          //SOC计算初始化
        
        //重新运行，修正为显示SOC的容量，避免充放电时SOC跳变
        CorrNowCapBySoc(gSOCInfo.displaySOC);                    
    }
    //尚未运行过SOC算法
    else
    {
        //写入DF版本号
        SocWriteEepValueByAddr16(e_SOC_DFVer_Addr,SOC_DF_VER);
        
        //ret = BSPEEPWriteOneWord(e_SOC_Config_Flag,SOC_CONFIG_FLAG);
        SocSetEepFirstFlag();
        
        if(False == ret)
        {
            ;   //显示SOC配置故障
        }    
        
        //初始化SOC值，等待SOC OCV校正
        gSOCInfo.remaincap = 0;
        gSOCInfo.tolcap = BAT_NORM_CAP_SOX;
        gSOCInfo.realSOC = 0;
        gSOCInfo.displaySOC = 0;
        gSOCInfo.soh = 1000;        
        gSOCInfo.cycles = 0;
        gSOCInfo.Calibcycles = 0;
        SocCalcInit();          //SOC计算初始化
    }
    
    //标志有效，说明运行历史容量存储
    if(SOC_CONFIG_FLAG == SocReadEepValueByAddr16(e_SOC_HisSave_Flag))
    {
        //更新到RAM中
        SetHisChgCap(SocReadEepValueByAddr32(e_SOC_HisChgCap_Addr));
        SetHisDchgCap(SocReadEepValueByAddr32(e_SOC_HisDchgCap_Addr));
    }   
    //尚未运行过历史容量存储
    else
    {
        SetHisChgCap(0);
        SetHisDchgCap(0);       
    }        
    
    TotalCapCheckInit();    //SOC高低压容量校正初始化
    
    SocSlideShowInit();     //SOC平滑初始化
    SocOCVInit();           //SOC OCV初始化
    
    #ifdef SOC_LT_COMPENSATE_ENABLE
    SocLTCompensateInit();  //SOC低温补偿 初始化
    #endif
}



//=============================================================================================
//函数名称	: void SocEepRefreshInfo(void) 
//输入参数	: void  
//输出参数	: void
//函数功能	: SOC写入EEP参数刷新
//注意事项	:  
//=============================================================================================
void SocEepRefreshInfo(void)
{
    t_CapCheck *pVh = TotalCapCheckGetPVHPoint();
    t_CapCheck *pVl = TotalCapCheckGetPVLPoint();    

    gSocEepInfo.SOC_Config_Flag = SOC_CONFIG_FLAG;
    gSocEepInfo.SOC_DFVer = SOC_DF_VER;

    #ifdef SOC_LT_COMPENSATE_ENABLE
    if(True == SocCompCrt.CompState.CompStateBit.IsNeedComp)
        gSocEepInfo.SOC_TolCap = SocCompCrt.LtcAllCap;
    else
        gSocEepInfo.SOC_TolCap = gSOCInfo.tolcap;
    #else
    gSocEepInfo.SOC_TolCap = gSOCInfo.tolcap;
    #endif

    gSocEepInfo.SOC_RemCap = gSOCInfo.remaincap;    
    gSocEepInfo.SOC_DispSoc = gSOCInfo.displaySOC;

    gSocEepInfo.SOC_HisSave_Flag = SOC_CONFIG_FLAG;
    gSocEepInfo.SOC_HisChgCap = GetHisChgCap();
    gSocEepInfo.SOC_HisDchgCap = GetHisDchgCap();


    gSocEepInfo.SOC_VHCapChk_Flag = SOC_CONFIG_FLAG;
    gSocEepInfo.SOC_VHValid = pVh->valid;
    gSocEepInfo.SOC_VHChgCap = pVh->chgCap;
    gSocEepInfo.SOC_VHDchgCap = pVh->dchgCap;
    gSocEepInfo.SOC_VHAllCap = pVh->allCap;
    gSocEepInfo.SOC_VHTimer = pVh->timer;

   gSocEepInfo.SOC_VLCapChk_Flag = SOC_CONFIG_FLAG;
   gSocEepInfo.SOC_VLValid = pVl->valid;
   gSocEepInfo.SOC_VLChgCap = pVl->chgCap;
   gSocEepInfo.SOC_VLDchgCap = pVl->dchgCap;
   gSocEepInfo.SOC_VLAllCap = pVl->allCap;
   gSocEepInfo.SOC_VLTimer = pVl->timer;
   
   gSocEepInfo.SOC_CalibCyc_Flag = SOC_CONFIG_FLAG;         //新增
   gSocEepInfo.SOC_CalibCyc_Addr = gSOCInfo.Calibcycles;
}
//=============================================================================================
//函数名称	: u16 SocReadEepValueByAddr16(e_SOC_EEP_ADDR socaddr)
//输入参数	: socaddr：SOC存储EEP地址    
//输出参数	: void
//函数功能	: SOC读取16位 EEPROM数据
//注意事项	:  
//=============================================================================================
//u16 SocReadEepValueByAddr16(e_SOC_EEP_ADDR socaddr)
//{
////    return BSPEEPReadOneWord(socaddr);
//		return ReadFlash_16bit(socaddr);
//}
u16 SocReadEepValueByAddr16(e_SOC_EEP_ADDR socaddr)
{
    //return BSPEEPReadOneWord(socaddr);
    //return (u16)(*((u8 *)&gSocEepInfo + socaddr));
     u16 temp;   
    
    switch(socaddr)
    {
        case e_SOC_Config_Flag:
            temp = gSocEepInfo.SOC_Config_Flag;
        break;

        case e_SOC_DFVer_Addr:
            temp = gSocEepInfo.SOC_DFVer;
        break;

        case e_SOC_TolCap_Addr:
            temp = gSocEepInfo.SOC_TolCap;
        break;

        case e_SOC_RemCap_Addr:
            temp = gSocEepInfo.SOC_RemCap;
        break;

        case e_SOC_DispSoc_Addr:
            temp = gSocEepInfo.SOC_DispSoc;
        break;

        case e_SOC_HisSave_Flag:
            temp = gSocEepInfo.SOC_HisSave_Flag;
        break;

        case e_SOC_VHCapChk_Flag:
            temp = gSocEepInfo.SOC_VHCapChk_Flag;
        break;

        case e_SOC_VHValid_Addr:
            temp = gSocEepInfo.SOC_VHValid;
        break;

        case e_SOC_VLCapChk_Flag:
            temp = gSocEepInfo.SOC_VLCapChk_Flag;
        break;

        case e_SOC_VLValid_Addr:
            temp = gSocEepInfo.SOC_VLValid;
        break;
        
        case e_SOC_CalibCyc_Flag:
            temp = gSocEepInfo.SOC_CalibCyc_Flag;
        break;

        case e_SOC_CalibCyc_Addr:
            temp = gSocEepInfo.SOC_CalibCyc_Addr;
        break;
//        case e_SOC_InfoNum_Addr:
//            temp = gSocEepInfo.SOC_InfoNum;
//        break;

//        case e_SOC_Reserved_Addr:
//            temp = 0;
//        break;

        default:
            temp = 0;
        break;
    }

    return (u16)temp;
}
//=============================================================================================
//函数名称	: u32 SocReadEepValueByAddr32(e_SOC_EEP_ADDR socaddr)
//输入参数	: socaddr：SOC存储EEP地址  
//输出参数	: void
//函数功能	: SOC读取32位 EEPROM数据
//注意事项	:  
//=============================================================================================
//u32 SocReadEepValueByAddr32(e_SOC_EEP_ADDR socaddr)
//{
////    return BSPEEPReadOneDWord(socaddr);
//		return ReadFlash_32bit(socaddr);
//}
u32 SocReadEepValueByAddr32(e_SOC_EEP_ADDR socaddr)
{
    //return BSPEEPReadOneDWord(socaddr);
    //return (u32)(*((u8 *)&gSocEepInfo + socaddr));
    u32 temp;   
    
    switch(socaddr)
    {
        case e_SOC_HisDchgCap_Addr:
            temp = gSocEepInfo.SOC_HisDchgCap;
        break;

        case e_SOC_HisChgCap_Addr:
            temp = gSocEepInfo.SOC_HisChgCap;
        break;

        case e_SOC_VHChgCap_Addr:
            temp = gSocEepInfo.SOC_VHChgCap;
        break;

        case e_SOC_VHDchgCap_Addr:
            temp = gSocEepInfo.SOC_VHDchgCap;
        break;        

        case e_SOC_VHAllCap_Addr:
            temp = gSocEepInfo.SOC_VHAllCap;
        break;                

        case e_SOC_VHTimer_Addr:
            temp = gSocEepInfo.SOC_VHTimer;
        break;     

        case e_SOC_VLChgCap_Addr:
            temp = gSocEepInfo.SOC_VLChgCap;
        break;

        case e_SOC_VLDchgCap_Addr:
            temp = gSocEepInfo.SOC_VLDchgCap;
        break;        

        case e_SOC_VLAllCap_Addr:
            temp = gSocEepInfo.SOC_VLAllCap;
        break;                

        case e_SOC_VLTimer_Addr:
            temp = gSocEepInfo.SOC_VLTimer;
        break;                      
        default:
            temp = 0;
        break;
    }

    return (u32)temp;
}
//=============================================================================================
//函数名称	: u8 SocWriteEepValueByAddr16(e_SOC_EEP_ADDR socaddr,u16 data)
//输入参数	: socaddr：SOC存储EEP地址  data：写入的数据
//输出参数	: False:写失败；True：写成功
//函数功能	: SOC写16位 EEPROM数据
//注意事项	:  
//=============================================================================================
//u8 SocWriteEepValueByAddr16(e_SOC_EEP_ADDR socaddr,u16 data)
//{
////    return BSPEEPWriteOneWord(socaddr,data);
//		return WriteFlagToFlash(socaddr,data,sizeof(data));
//}
e_SOC_EEP_ADDR soctest;
u8 SocWriteEepValueByAddr16(e_SOC_EEP_ADDR socaddr,u16 data)
{
//    extern uint16_t STOR_CTRL;
    extern uint16_t gStorageCTRL;
    soctest = socaddr;
    //return BSPEEPWriteOneWord(socaddr,data);
    gStorageCTRL |= FLASH_SOC_WR_MASK;
    return True;
}
//=============================================================================================
//函数名称	: u8 SocWriteEepValueByAddr32(e_SOC_EEP_ADDR socaddr,u32 data)
//输入参数	: socaddr：SOC存储EEP地址  data：写入的数据
//输出参数	: False:写失败；True：写成功
//函数功能	: SOC写32位 EEPROM数据
//注意事项	:  
//=============================================================================================
//u8 SocWriteEepValueByAddr32(e_SOC_EEP_ADDR socaddr,u32 data)
//{
////    return BSPEEPWriteOneDWord(socaddr,data);
//		return WriteFlagToFlash(socaddr,data,sizeof(data)); 
//}
u8 SocWriteEepValueByAddr32(e_SOC_EEP_ADDR socaddr,u32 data)
{
//    extern uint16_t STOR_CTRL;
    extern uint16_t gStorageCTRL;
    soctest = socaddr;
    //return BSPEEPWriteOneDWord(socaddr,data);
    gStorageCTRL |= FLASH_SOC_WR_MASK;
    return True;
}
//=============================================================================================
//函数名称	: u8 SocWriteEepClearAllData(void)
//输入参数	: void
//输出参数	: False:清除失败；True：清除成功
//函数功能	: SOC清除EEPROM数据
//注意事项	:  
//=============================================================================================
u8 SocWriteEepClearAllData(void)
{
    u8 ret = False;
    u8 i = 0;
    
    for(i = 0;i < ((e_SOC_Max_Addr + 2 - e_SOC_Config_Flag) / 2);i++)
    {
        ret |= SocWriteEepValueByAddr16((e_SOC_EEP_ADDR)(e_SOC_Config_Flag + (2 * i)),0xFFFF);
        //BSPSysDelay1ms(100);
				HAL_Delay(100);
    }
    SocWriteEepValueByAddr16(e_SOC_DFVer_Addr,0xFFFF);
    
    return ret;
}

//=============================================================================================
//函数名称	: void SocEepSaveVHCheckPoint(void)
//输入参数	: VhPoint:高压点指针
//输出参数	: True:写成功；False:写失败
//函数功能	: 写高压点数据到EEP中
//注意事项	:  
//=============================================================================================
u8 SocEepSaveVHCheckPoint(t_CapCheck *VhPoint)
{
    u8 ret = False;
    
    ret |= SocWriteEepValueByAddr16(e_SOC_VHCapChk_Flag,SOC_CONFIG_FLAG);    
    ret |= SocWriteEepValueByAddr16(e_SOC_VHValid_Addr,VhPoint->valid);
        
    ret |= SocWriteEepValueByAddr32(e_SOC_VHChgCap_Addr,VhPoint->chgCap);
    ret |= SocWriteEepValueByAddr32(e_SOC_VHDchgCap_Addr,VhPoint->dchgCap);
    ret |= SocWriteEepValueByAddr32(e_SOC_VHAllCap_Addr,VhPoint->allCap);
    ret |= SocWriteEepValueByAddr32(e_SOC_VHTimer_Addr,VhPoint->timer);  

    return ret;    
}

//=============================================================================================
//函数名称	: void SocEepSaveVLCheckPoint(void)
//输入参数	: VlPoint:低压点指针
//输出参数	: True:写成功；False:写失败
//函数功能	: 写低压点数据到EEP中
//注意事项	:  
//=============================================================================================
u8 SocEepSaveVLCheckPoint(t_CapCheck *VlPoint)
{
    u8 ret = False;
    
    ret |= SocWriteEepValueByAddr16(e_SOC_VLCapChk_Flag,SOC_CONFIG_FLAG);    
    ret |= SocWriteEepValueByAddr16(e_SOC_VLValid_Addr,VlPoint->valid);
        
    ret |= SocWriteEepValueByAddr32(e_SOC_VLChgCap_Addr,VlPoint->chgCap);
    ret |= SocWriteEepValueByAddr32(e_SOC_VLDchgCap_Addr,VlPoint->dchgCap);
    ret |= SocWriteEepValueByAddr32(e_SOC_VLAllCap_Addr,VlPoint->allCap);
    ret |= SocWriteEepValueByAddr32(e_SOC_VLTimer_Addr,VlPoint->timer);       
    
    return ret;
}

//=============================================================================================
//函数名称	: void SocSetEepFirstFlag(void)
//输入参数	: void
//输出参数	: void
//函数功能	: SOC算法初始运行标志置1
//注意事项	:  
//=============================================================================================
void SocSetEepFirstFlag(void)
{
    sFirstEepFlag = True;
}

//=============================================================================================
//函数名称	: void SocClrEepFirstFlag(void)
//输入参数	: void
//输出参数	: void
//函数功能	: SOC算法初始运行标志清零
//注意事项	:  
//=============================================================================================
void SocClrEepFirstFlag(void)
{
    sFirstEepFlag = False;
}

//=============================================================================================
//函数名称	: u8 SocGetEepFirstFlag(void)
//输入参数	: void
//输出参数	: SOC算法初始运行标志
//函数功能	: 获取SOC算法初始运行标志
//注意事项	:  
//=============================================================================================
u8 SocGetEepFirstFlag(void)
{
    return sFirstEepFlag;
}

/*****************************************end of SocEepHook.c*****************************************/
