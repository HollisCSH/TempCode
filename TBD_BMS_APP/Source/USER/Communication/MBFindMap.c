//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: MBFindMap.c
//创建人  	: Handry
//创建日期	: 
//描述	    : Modbus从机地址映射处理文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	: Handry
//版本	    : 
//修改日期	: 2020/07/14
//描述	    : 
//1.增加只读地址323-326；增加配置地址837-840
//=============================================================================================

//=============================================================================================
//头文件
//=============================================================================================
#include "MBFindMap.h"
#include "SleepMode.h"
#include "Comm.h"
#include "version.h"
#include "UserData.h"
#include "ParaCfg.h"
#include "DTCheck.h"
#include "Storage.h"
#include "MAX17205.h"
#include "VoltBalance.h"
#include "Sample.h"
#include "stdint.h"
#include "version.h"
#include "EventRecord.h"
#include "CommCtrl.h"
//#include "PCF85063.h"
#include "SH367309.h"
#include "BSPSystemCfg.h"
#include "IOCheck.h"
#include "SysState.h"
#include "MOSCtrl.h"
#include "TestMode.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
//#include "FM11NC08.h"
#include "NTC.h"
#include "HMAC.h"
#include "DA213.h"
#include "BSPTimer.h"
//#include "BSPWatchDog.h"
//#include "BSPFlash.h"
#include "CurrIntegral.h"
#include "SocCalcHook.h"
#include "SocCapCalc.h"
#include "SocEepHook.h"
#include "BatteryInfoGetSetApi.h"
#include "BSP_Flash.h"
#include <stdbool.h>
//=============================================================================================
//定义全局变量
//=============================================================================================
//读寄存器起始地址数组
const u16 cMbRegReadStart[MB_AREA_READ_NUM] =
{
	MB_DEVICE_INFO_START,		//BMS设备信息起始地址
	MB_READONLY_DATA_START,		//BMS只读数据起始地址
	MB_RUN_CTRL_START,			//BMS运行控制起始地址
	MB_PARA_CFG_START,			//BMS参数配置起始地址
	MB_USER_DATA_DATA_START		//用户数据区起始地址
};
//读寄存器结束地址数组
const u16 cMbRegReadEnd[MB_AREA_READ_NUM] =
{
	MB_DEVICE_INFO_END,		//BMS设备信息结束地址
	MB_READONLY_DATA_END,	//BMS只读数据结束地址
	MB_RUN_CTRL_END,		//BMS运行控制结束地址
	MB_PARA_CFG_END,		//BMS参数配置结束地址
	MB_USER_DATA_END		//用户数据区结束地址
};

//读寄存器函数集合
const t_MBREG * const cMBReadRegFunc[MB_AREA_READ_NUM]=
{
	cMBDevInfoRdFunc,		//BMS设备信息读函数集合
	cMBReadonlyDataRdFunc,	//BMS只读数据读函数集合
	cMBRunCTRLRdFunc,		//BMS运行控制读函数集合
	cMBParaCfgRdFunc,		//BMS参数配置读函数集合
	cMBUserDataRdFunc		//用户数据区读函数集合
};

//写寄存器起始地址数组
const u16 cMbRegWriteStart[MB_AREA_WRITE_NUM] =
{
	MB_RUN_CTRL_START,			//BMS运行控制起始地址
	MB_PARA_CFG_START,			//BMS参数配置起始地址
	MB_USER_DATA_DATA_START		//用户数据区起始地址
};

//写寄存器结束地址数组
const u16 cMbRegWriteEnd[MB_AREA_WRITE_NUM] =
{
	MB_RUN_CTRL_END,		//BMS运行控制结束地址
	MB_PARA_CFG_END,		//BMS参数配置结束地址
	MB_USER_DATA_END		//用户数据区结束地址
};

//写寄存器函数集合
const t_MBREG * const cMBWriteRegFunc[MB_AREA_WRITE_NUM] =
{
	cMBRunCTRLWrFunc,		//BMS运行控制写函数集合
	cMBParaCfgWrFunc,		//BMS参数配置写函数集合
	cMBUserDataWrFunc		//用户数据区写函数集合
};

//=============================================================================================
//定义接口函数
//=============================================================================================

/********************************************************* 读函数集合 *********************************************************/

//读空地址
static u8 mb_readnull_r(u16 mbaddr,u16 *data)
{
    (void)mbaddr;
    *data = 0xFFFF;
    return TRUE;
}


/***************************** BMS设备信息读函数集合 *****************************/

//读地址0
static u8 mb_info_prver_r(u16 mbaddr,u16 *data)
{
    (void)mbaddr;
    *data = FW_PROTOCOL_VER;
    return TRUE;
}

//读地址1
static u8 mb_info_hwver_r(u16 mbaddr,u16 *data)
{
    (void)mbaddr;
    *data = ((uint16_t)gHwVerRead[0]<<8)|(gHwVerRead[1]);
    return TRUE;
}

//读地址2
static u8 mb_info_blver_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    //bootloader版本号
    *data = ((uint16_t)gBLVerRead[0]<<8)|(gBLVerRead[1]);
    return TRUE;
}

//读地址3
static u8 mb_info_fwmsv_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = ((uint16_t)cAPPInfo.swmain << 8)|(cAPPInfo.swsub1);
    //*data = ((uint16_t)c_APP_INFO.sw_main<<8)|(c_APP_INFO.sw_s1);
    return TRUE;
}

//读地址4
static u8 mb_info_fwrev_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = cAPPInfo.swsub2;
    //*data = (c_APP_INFO.sw_s2);
    return TRUE;
}

//读地址5
static u8 mb_info_fwbnh_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = ((uint16_t)cAPPInfo.swbuild[3]<<8)|(cAPPInfo.swbuild[2]);
    //*data = ((uint16_t)c_APP_INFO.sw_build[3]<<8)|(c_APP_INFO.sw_build[2]);
    return TRUE;
}

//读地址6
static u8 mb_info_fwbnl_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = ((uint16_t)cAPPInfo.swbuild[1]<<8)|(cAPPInfo.swbuild[0]);
    //*data = ((uint16_t)c_APP_INFO.sw_build[1]<<8)|(c_APP_INFO.sw_build[0]);
    return TRUE;
}

//读地址7 - 8
static u8 mb_info_mcun_r_l(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -= 7;
	
    //*data = *((uint16_t*)&(DEVINFO->UNIQUEL)+3-mbaddr);
		//*data = *((uint16_t*)&(SIM->UIDL) + 1 - mbaddr);
		*data = *((uint16_t*)(UID_BASE+0U) + 1 - mbaddr);
    return TRUE;
}

//读地址9 - 10
static u8 mb_info_mcun_r_h(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -= 9;
	
    //*data = *((uint16_t*)&(DEVINFO->UNIQUEL)+3-mbaddr);
		//*data = *((uint16_t*)&(SIM->UIDH) + 1 - mbaddr);
		*data = *((uint16_t*)(UID_BASE+8U) + 1 - mbaddr);
    return TRUE;
}

//读地址11
static u8 mb_info_btype_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    *data = BAT_TYPE;
    return TRUE;
}

//读地址12
static u8 mb_info_bvolt_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    *data = (BAT_NORM_VOLT * BAT_CELL_NUM / 10);
    return TRUE;
}

//读地址13
static u8 mb_info_bcap_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    *data = BAT_NORM_CAP;
    return TRUE;
}

//读地址14 - 17
static u8 mb_info_sn_r(uint16_t mbaddr,uint16_t *data)
{
    *data = *(gConfig.sn+(mbaddr-14));
    return TRUE;
}

//读地址18
static u8 mb_info_erech_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    *data = gRecordInfo.earliest>>16;
    return TRUE;
}

//读地址19
static u8 mb_info_erecl_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    *data = gRecordInfo.earliest & 0xffff;
    return TRUE;
}

//读地址20
static u8 mb_info_lrech_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    *data = gRecordInfo.latest>>16;
    return TRUE;
}

//读地址21
static u8 mb_info_lrecl_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    *data = gRecordInfo.latest & 0xffff;
    return TRUE;
}

//读地址22 - 32
static u8 mb_info_lifetime_r(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -= 22;
    *data = *(((uint16_t*)&gUserIDBuff.lt)+mbaddr);
    return TRUE;
}

//读地址33
static u8 mb_info_df_ver(uint16_t mbaddr,uint16_t *data)
{
    //电量计版本
    #ifdef    BMS_USE_MAX172XX
    *data = gMax17205Var.DfVer;
    #else
    *data = SOC_DF_VER;
    #endif
    return TRUE;
}

//读地址34
static u8 mb_info_bat_num(uint16_t mbaddr,uint16_t *data)
{
    *data = BAT_CELL_NUM;
    return TRUE;
}

//读地址35
static u8 mb_info_bat_type(uint16_t mbaddr,uint16_t *data)
{
    *data = 0;
    return TRUE;
}

//读地址50
static u8 mb_info_fuel_updatenum(uint16_t mbaddr,uint16_t *data)
{
    #ifdef    BMS_USE_MAX172XX
    *data = gMax17205Var.NumOfDates;
    #else
    *data = 0;
    #endif
    return TRUE;
}

#ifdef BMS_USE_SOX_MODEL
//读地址51 SOX DF版本号
static u8 mb_info_sox_dfver_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = SOC_DF_VER;

    //*data = g_BMS_DATA.balance_test;
    return TRUE;
}
#endif

/***************************** BMS只读数据读函数集合 *****************************/
//读地址256，257,258,259
static u8 mb_data_user_r(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -=256;
    *data = *(((u16*)&gUserID.user)+mbaddr);
    return TRUE;
}

//读地址260
static u8 mb_data_state_r(uint16_t mbaddr,uint16_t *data)
{
	u16 ret = 0;
	
	(void)mbaddr;
    ret |= (gBatteryInfo.Status.MOSStatus& 0x03);

	if(gBatteryInfo.Status.IOStatus & PRE_DCHG_CHECK_SHIFT)
		ret |= STATE_PREMOS;
	if(gBatteryInfo.Status.IOStatus & HALL_CHECK_SHIFT)
		ret |= STATE_HALL;
	if(gBatteryInfo.Status.IOStatus & REMOVE_CHECK_SHIFT)
		ret |= STATE_PHOTO;

	if(gVoltBalance.balctrl & 0x3FFFFFFF)
		ret |= STATE_BALA;
	if(gBatteryInfo.Fault.FaultInfo[0] || gBatteryInfo.Fault.FaultInfo[1])
		ret |= STATE_DEVFT;
	if(gBatteryInfo.Fault.FaultInfo[2] || gBatteryInfo.Fault.FaultInfo[3])
		ret |= STATE_OPTFT;
	if(gBatteryInfo.Fault.AlarmInfo[0] || gBatteryInfo.Fault.AlarmInfo[1])
		ret |= STATE_OPTWARN;
	if((gBatteryInfo.Addit.FuelStaFlag & MAX1720X_FULLCHG_FLAG)||(gBatteryInfo.Fault.FaultInfo[2] & OPT_FAULT1_OV))//满充标志
		ret |= STATE_FC;
    if(HMACReadCertifiSta()==0) //BMS未验证时，BMS状态寄存器Bit8置1，验证通过置0。  
        ret |= STATE_CERT;
    
	if(gBatteryInfo.Status.IOStatus & EXTERN_MODULE_CHECK_SHIFT)
		ret |= STATE_EXMODULE;    
	if(eProt_Full == gBatteryInfo.Status.ProtSta)   //电池内部充满标志
		ret |= STATE_POLL;        
//    ret |= g_AFE_VAR.mosfetread&0x03;
//    ret |= GPIO_PinOutGet(PRE_DSG_PORT,PRE_DSG)<<2;
//    if(g_BG_DATA.signal&BG_G2_MASK)
//        ret |= STATE_G2;
//    if(g_BG_DATA.signal&BG_BREAK_MASK)
//        ret |= STATE_PHOTO;
//    if(g_AFE_VAR.balancecfg)
//        ret |= STATE_BALA;
//    if(g_BMS_DATA.fault[0] || g_BMS_DATA.fault[1])
//        ret |= STATE_DEVFT;
//    if(g_BMS_DATA.fault[2] || g_BMS_DATA.fault[3])
//        ret |= STATE_OPTFT;
//    if(g_BMS_DATA.fault[4] || g_BMS_DATA.fault[5])
//        ret |= STATE_OPTWARN;
//    if((g_BQ27541_VAR.flags&FUEL_FLAGS_FC)||(g_BMS_DATA.fault[2]&OPT_FAULT1_OV))//满充标志
//        ret |= STATE_FC;
    *data = ret;
    return TRUE;
}

//读地址261
static u8 mb_data_soc_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    //*data = g_BMS_RESULT.filt_soc;
    *data = gBatteryInfo.Data.FiltSOC;

    return TRUE;
}

//读地址262
static u8 mb_data_totlev_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    //*data = g_BMS_RESULT.totle_volt;
    *data = gBatteryInfo.Data.TolVolt;

    return TRUE;
}

//读地址263
static u8 mb_data_curr_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    //*data = g_BMS_RESULT.current10ma + 30000;
    *data = (uint16_t)(gBatteryInfo.Data.Curr + 30000);

    return TRUE;
}

//读地址264
static u8 mb_data_maxt_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

//    if((g_BMS_RESULT.max_temp ==NTC_TEMP_OFF)||(g_BMS_RESULT.max_temp ==NTC_TEMP_SHORT))
//        *data = 0xFFFF;
//    else
//        *data = g_BMS_RESULT.max_temp + 400;
	if((gBatteryInfo.TempChara.MaxTemp == NTC_TEMP_OFF)||(gBatteryInfo.TempChara.MaxTemp == NTC_TEMP_SHORT))
		*data = 0xFFFF;
	else
		*data = gBatteryInfo.TempChara.MaxTemp + 400;

    return TRUE;
}

//读地址265
static u8 mb_data_maxtn_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    //*data = gBatteryInfo.TempChara.MaxTNum;
    if(gBatteryInfo.TempChara.MaxTNum!=0xff)
        *data = gBatteryInfo.TempChara.MaxTNum;
    else
        *data = 0xffff;

    return TRUE;
}

//读地址266
static u8 mb_data_mint_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    //*data = gBatteryInfo.TempChara.MinTemp + 400;
    if((gBatteryInfo.TempChara.MinTemp == NTC_TEMP_OFF)||(gBatteryInfo.TempChara.MinTemp == NTC_TEMP_SHORT))
        *data = 0xFFFF;
    else
        *data = gBatteryInfo.TempChara.MinTemp + 400;

    return TRUE;
}

//读地址267
static u8 mb_data_mintn_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    //*data = gBatteryInfo.TempChara.MinTNum;
    if(gBatteryInfo.TempChara.MinTNum != 0xff)
        *data = gBatteryInfo.TempChara.MinTNum;
    else
        *data = 0xffff;

    return TRUE;
}

//读地址268
static u8 mb_data_maxv_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gBatteryInfo.VoltChara.MaxVolt;
    //*data = g_BMS_RESULT.max_volt;

    return TRUE;
}

//读地址269
static u8 mb_data_maxvn_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    //*data = gBatteryInfo.VoltChara.MaxVNum;
    if(gBatteryInfo.VoltChara.MaxVNum!=0xff)
        *data = gBatteryInfo.VoltChara.MaxVNum;
    else
        *data = 0xffff;

    return TRUE;
}

//读地址270
static u8 mb_data_minv_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    //*data = g_BMS_RESULT.min_volt;
    *data = gBatteryInfo.VoltChara.MinVolt;

    return TRUE;
}

//读地址271
static u8 mb_data_minvn_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    //*data = gBatteryInfo.VoltChara.MinVNum;
    if(gBatteryInfo.VoltChara.MinVNum != 0xff)
        *data = gBatteryInfo.VoltChara.MinVNum;
    else
        *data = 0xffff;

    return TRUE;
}

//读地址272
static u8 mb_data_dsgc_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gBatteryInfo.Limit.DchgCurrLim;
    //*data = g_BMS_RESULT.max_dsg_c;

    return TRUE;
}

//读地址273
static u8 mb_data_chgc_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gBatteryInfo.Limit.ChgCurrLim;
    //*data = g_BMS_RESULT.max_chg_c;

    return TRUE;
}

//读地址274
static u8 mb_data_soh_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gBatteryInfo.Data.SOH;
    //*data = g_BQ27541_VAR.soh*10;

    return TRUE;
}

//读地址275,833
static u8 mb_data_cyc_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gBatteryInfo.Data.ChgDchgTimes;
    //*data = g_BQ27541_VAR.cycle_count;

    return TRUE;
}

//读地址276
static u8 mb_data_rcap_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gBatteryInfo.Data.RemainCap;
    //*data = g_BQ27541_VAR.remain_capacity;
    return TRUE;
}

//读地址277
static u8 mb_data_fcap_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gBatteryInfo.Data.TolCap;
    //*data = g_BQ27541_VAR.full_charge_capacity;
    return TRUE;
}

//读地址278
static u8 mb_data_fctime_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gBatteryInfo.Data.FullChgTime;
    //*data = g_BMS_RESULT.fctime;
    return TRUE;
}

//读地址279
static u8 mb_data_repow_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gBatteryInfo.Data.RemainPower;
    //*data = g_BMS_RESULT.repow;
    return TRUE;
}

//读地址280,281,282,283
static u8 mb_data_wnft_r(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -= 280;

    *data = gBatteryInfo.Fault.FaultInfo[mbaddr];
    //*data = *(g_BMS_DATA.fault+mbaddr);
    return TRUE;
}

//读地址284,285
static u8 mb_data_wnalarm_r(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -= 284;

    *data = gBatteryInfo.Fault.AlarmInfo[mbaddr];
    //*data = *(g_BMS_DATA.fault+mbaddr);
    return TRUE;
}

//读地址286,287
static u8 mb_data_mostemp_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr ;

    if((gSampleData.PCBTemp ==NTC_TEMP_OFF)||(gSampleData.PCBTemp ==NTC_TEMP_SHORT))
        *data = 0xFFFF;
    else
        *data = gSampleData.PCBTemp + 400;
    //*data = gSampleData.PCBTemp + 400;

    return TRUE;
}

//读地址288
static u8 mb_data_pretemp_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr ;

    if((gBatteryInfo.Addit.FuelTemp == NTC_TEMP_OFF)||(gBatteryInfo.Addit.FuelTemp==NTC_TEMP_SHORT))
        *data = 0xFFFF;
    else
        //*data = gBatteryInfo.Addit.FuelTemp + 400;
    *data = gBatteryInfo.Data.CellTemp[0] + 400;
    //*data = gBatteryInfo.Addit.FuelTemp + 400;

    return TRUE;
}

//读地址289
static u8 mb_data_contemp_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr ;

    if((gSampleData.ConnTemp ==NTC_TEMP_OFF)||(gSampleData.ConnTemp ==NTC_TEMP_SHORT))
        *data = 0xFFFF;
    else
        *data = gSampleData.ConnTemp + 400;
    //*data = gSampleData.ConnTemp + 400;

    return TRUE;
}

//读地址290,291
static u8 mb_data_battemp_r(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -=290 ;


    if((gBatteryInfo.Data.CellTemp[mbaddr] ==NTC_TEMP_OFF)||(gBatteryInfo.Data.CellTemp[mbaddr] ==NTC_TEMP_SHORT))
        *data = 0xFFFF;
    else
        *data = gBatteryInfo.Data.CellTemp[mbaddr] + 400;
    //*data = gBatteryInfo.Data.CellTemp[mbaddr] + 400;

    return TRUE;
}

//读地址292 - 307
static u8 mb_data_batvolt_r(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -=292 ;

    *data = BITGET32(SH367309_VALID_BAT,mbaddr)?gBatteryInfo.Data.CellVolt[mbaddr]:0xffff;
    //*data = g_BMS_RESULT.filt_volt[mbaddr];
    return TRUE;
}

//读地址308
static u8 mb_data_bala_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    //*data = gVoltBalance.balctrl;
	#if defined(USE_B20_TOPBAND_PCBA) || defined(USE_B22_IM_PCBA)
    u32 balah,balal = 0;
    balal = (((u16)gSHAFEReg.BalanceH) << 8 | (u16)gSHAFEReg.BalanceL);
    balah = (((u16)gSHAFEReg_H.BalanceH) << 8 | (u16)gSHAFEReg_H.BalanceL);
    u32 bala = (((balal & 0x03FF))) | ((balah & 0x003F) << 10);
    *data = bala;
    #elif defined(USE_B21_IM_PCBA)
    *data = ((u16)gSHAFEReg.BalanceH) << 8 | (u16)gSHAFEReg.BalanceL;
	#endif
	
    //*data = g_AFE_VAR.balancecfg;
    return TRUE;
}

//读地址309
static u8 mb_data_accx_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
    //*data = g_ACC_DATA.x;
    *data = gAccVar.AccData.ax;
    #else
    *data = 0;
    #endif

    return TRUE;
}

//读地址310
static u8 mb_data_accy_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
    //*data = g_ACC_DATA.y;
    *data = gAccVar.AccData.ay;
    #else
    *data = 0;
    #endif
    
    return TRUE;
}

//读地址311
static u8 mb_data_accz_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
    //*data = g_ACC_DATA.z;
    *data = gAccVar.AccData.az;
    #else
    *data = 0;
    #endif
    
    return TRUE;
}

//读地址312
static u8 mb_data_mcu3v3_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gSampleData.MCU3v3;
    //*data = g_MM_DATA.mcu3v3;
    return TRUE;
}

//读地址313
static u8 mb_data_prevolt_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gSampleData.PreVolt;
    //*data = gSampleData.PreCurr;
    
    return TRUE;
}

//读地址314
static u8 mb_data_fuelcurr_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gBatteryInfo.Addit.FuelAvgCurr;
    //*data = g_BQ27541_VAR.averagecurrent;
    return TRUE;
}

//读地址315
static u8 mb_data_chgvolt_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gBatteryInfo.Limit.MaxChgVolt;
    //*data = g_BMS_RESULT.max_chg_volt;
    return TRUE;
}

//读地址316
static u8 mb_data_fuelvolt_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gBatteryInfo.Addit.FuelCellVolt;
    //*data = g_BQ27541_VAR.voltage;
    return TRUE;
}

//读地址317
static u8 mb_data_tvstemp_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr ;

    if((gSampleData.TVSTemp ==NTC_TEMP_OFF)||(gSampleData.TVSTemp ==NTC_TEMP_SHORT))
        *data = 0xFFFF;
    else
        *data = gSampleData.TVSTemp + 400;

    return TRUE;
}

//读地址318,319,320
static u8 mb_data_tempspeed_r(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -=318 ;

    *data = gBatteryInfo.TempChara.TempRiseSpeed[mbaddr];
    //*data = g_BMS_RESULT.speed_temp[mbaddr];
    return TRUE;
}

//读地址321
static u8 mb_data_minvs_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gBatteryInfo.VoltChara.MinDropVolt;
    //*data = g_BMS_RESULT.min_speed_volt;

    return TRUE;
}

//读地址322
static u8 mb_data_minvsn_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    if(gBatteryInfo.VoltChara.MinDropVoltNum != 0xff)
    {
        *data = gBatteryInfo.VoltChara.MinDropVoltNum + 1;
    }
    else
    {
        *data = 0xffff;
    }
//    if(g_BMS_RESULT.speed_volt_num!=0xff)
//        *data = g_BMS_RESULT.speed_volt_num+1;
//    else
//        *data = 0xffff;

    return TRUE;
}

//读地址323
static u8 mb_data_max_chg_c_recent(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    *data = gBatteryInfo.CurrChara.MaxChgCurrRecent + 30000;

    return true;
}

//读地址324
static u8 mb_data_max_dsg_c_recent(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    *data = gBatteryInfo.CurrChara.MaxDchgCurrRecent + 30000;

    return true;
}

//读地址325 保险丝温度，B6暂无
static u8 mb_data_fusetemp_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr ;
    *data = 0xFFFF;
    return true;
}

//读地址326
static u8 mb_data_currdet_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr ;
	uint16_t currdet = 0;

	currdet = (((uint16_t)gBatteryInfo.CurrChara.PerCurr30S) << 10)  | (((uint16_t)gBatteryInfo.CurrChara.MaxCurr30S) << 4) | 
              ((uint16_t)gBatteryInfo.CurrChara.HighCurrTime);
	
    *data = currdet;

    return true;
}

//读地址339-342
static u8 mb_data_batvolt17_20_r(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -=339 ;

    *data = BITGET32(SH367309_VALID_BAT,16+mbaddr)?gBatteryInfo.Data.CellVolt[16+mbaddr]:0xffff;
    
    return TRUE;
}

//读地址343
static u8 mb_data_bala17_20_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr ;
    
	#if defined(USE_B20_TOPBAND_PCBA) || defined(USE_B22_IM_PCBA)
    u32 balah = 0;
    balah = (((u16)gSHAFEReg_H.BalanceH) << 8 | (u16)gSHAFEReg_H.BalanceL);
    u32 bala = ((balah & 0x03FF) >> 6);
    if(BITGET32(gVoltBalance.balctrl,30))
        BITSET32(bala,14);
    else
        BITCLR32(bala,14);
    
    if(BITGET32(gVoltBalance.balctrl,31))
        BITSET32(bala,15);
    else
        BITCLR32(bala,15);  
    *data = bala;
	#elif defined(USE_B21_IM_PCBA)
	*data = 0;
	#endif
    
    return TRUE;
}

#ifdef BMS_USE_SOX_MODEL
///*SOX算法拓展*/
////读地址343
//static u8 mb_data_sox_soc_r(uint16_t mbaddr,uint16_t *data)
//{
//    (void)mbaddr;
//    //*data = g_BMS_RESULT.filt_soc;
//    *data = gBatteryInfo.Data.MAX_SOC;

//    return TRUE;
//}

////读地址344
//static u8 mb_data_sox_soh_r(uint16_t mbaddr,uint16_t *data)
//{
//    (void)mbaddr;
//    //*data = g_BMS_RESULT.filt_soc;
//    *data = gBatteryInfo.Data.MAX_SOH;

//    return TRUE;
//}

////读地址345
//static u8 mb_data_sox_cyc_r(uint16_t mbaddr,uint16_t *data)
//{
//    (void)mbaddr;

//    *data = gBatteryInfo.Data.MAX_ChgDchgTimes;

//    return TRUE;
//}

////读地址346
//static u8 mb_data_sox_rcap_r(uint16_t mbaddr,uint16_t *data)
//{
//    (void)mbaddr;

//    *data = gBatteryInfo.Data.MAX_RemainCap;
//    return TRUE;
//}

////读地址347
//static u8 mb_data_sox_fcap_r(uint16_t mbaddr,uint16_t *data)
//{
//    (void)mbaddr;

//    *data = gBatteryInfo.Data.MAX_TolCap;
//    return TRUE;
//}

//读地址344    历史充电容量高位
static u8 mb_data_HisChgCapH_r(uint16_t mbaddr,uint16_t *data)
{
    //转换成BCD数显示
    //temp = SOCDataDEC2BCD(GetHisChgCap());
    *data = (GetHisChgCap() >> 16) & 0x0000ffff;
    return TRUE;    
}

//读地址345    历史充电容量低位
static u8 mb_data_HisChgCapL_r(uint16_t mbaddr,uint16_t *data)
{
    //转换成BCD数显示
    //temp = SOCDataDEC2BCD(GetHisChgCap());
    *data = GetHisChgCap() & 0x0000ffff;
    return TRUE;        
}

//读地址346    历史放电容量高位
static u8 mb_data_HisDchgCapH_r(uint16_t mbaddr,uint16_t *data)
{
    //转换成BCD数显示
    //temp = SOCDataDEC2BCD(GetHisDchgCap());
    *data = (GetHisDchgCap() >> 16) & 0x0000ffff;
    return TRUE;    
}

//读地址347    历史放电容量低位
static u8 mb_data_HisDchgCapL_r(uint16_t mbaddr,uint16_t *data)
{
    //转换成BCD数显示
    //temp = SOCDataDEC2BCD(GetHisDchgCap());
    *data = (GetHisDchgCap()) & 0x0000ffff;
    return TRUE;  
}

#endif

#ifdef CANBUS_MODE_JT808_ENABLE	
//读地址348
static u8 mb_data_csq_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    pGPRSMSGInfo pGPRS = GPRSMsgInfoGet();
    
    *data = pGPRS->CSQ;

    return TRUE;
}

//读地址349
static u8 mb_data_firstfixtime_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    pGPSMSGInfo pGPS = GPSMsgInfoGet();
    
    *data = pGPS->ActiveTimeCnt / 1000;    

    return TRUE;
}

//读地址350
static u8 mb_data_gpssignal_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    pGPSMSGInfo pGPS = GPSMsgInfoGet();
    
    *data = pGPS->SNR;

    return TRUE;
}

//读地址351
static u8 mb_data_sateinview_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    pGPSMSGInfo pGPS = GPSMsgInfoGet();
    
    *data = pGPS->StateView;    

    return TRUE;
}
#endif


#ifdef CANBUS_MODE_JT808_ENABLE	

//读地址352    连接标志
static u8 mb_data_ExModuleFlag_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    pGPSMSGInfo pGPS = GPSMsgInfoGet();
    pGPRSMSGInfo pGPRS = GPRSMsgInfoGet();
    pBLEMSGInfo pBLE = BLEMsgInfoGet();
    
    u16 flag = 0;
    
    //GPS定位标志
    if(pGPS->Activ)
    {
        BITSET16(flag,0);
    }
    else
    {
        BITCLR16(flag,0);
    }
    
    //连接服务器标志
    if(pGPRS->connected)
    {
        BITSET16(flag,1);
    }
    else
    {
        BITCLR16(flag,1);
    }    
    
    //是否有短信
    if(pGPRS->sms)
    {
        BITSET16(flag,2);
    }
    else
    {
        BITCLR16(flag,2);
    } 
    
    //蓝牙是否已连接
    if(pBLE->connect)
    {
        BITSET16(flag,3);
    }
    else
    {
        BITCLR16(flag,3);
    }      
    
    *data = flag;
    
    return TRUE;  
}

//读地址353    经度 高位
static u8 mb_data_Longitude_H_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    pGPSMSGInfo pGPS = GPSMsgInfoGet();
    
    u32 temp = pGPS->longitude;
    
    *data = (temp >> 16) & 0xFFFF;   
    
    return TRUE;      
}

//读地址354    经度 低位
static u8 mb_data_Longitude_L_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    pGPSMSGInfo pGPS = GPSMsgInfoGet();
    
    u32 temp = pGPS->longitude;
    
    *data = temp & 0xFFFF;   
    
    return TRUE;      
}


//读地址355    纬度 高位
static u8 mb_data_Latitude_H_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    pGPSMSGInfo pGPS = GPSMsgInfoGet();
    
    u32 temp = pGPS->latitude;
    
    *data = (temp >> 16) & 0xFFFF;   
    
    return TRUE;      
}

//读地址356    纬度 低位
static u8 mb_data_Latitude_L_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    pGPSMSGInfo pGPS = GPSMsgInfoGet();
    
    u32 temp = pGPS->latitude;
    
    *data = temp & 0xFFFF;   
    
    return TRUE;      
}

//读地址357    GPRS连接时间
static u8 mb_data_GPRSConnectTime_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    pGPRSMSGInfo pGPRS = GPRSMsgInfoGet();
    
    u32 temp = pGPRS->ConnectTimeCnt;
    
    *data = temp / 10;   
    
    return TRUE;      
}

//读地址358    GPRS发送到接收数据的最长时间
static u8 mb_data_GPRSTranMaxTime_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    pGPRSMSGInfo pGPRS = GPRSMsgInfoGet();
    
    u32 temp = pGPRS->TranMaxTimeCnt;
    
    *data = temp / 10;   
    
    return TRUE;      
}

//读地址359    GPRS发送到接收数据的最小时间
static u8 mb_data_GPRSTranMinTime_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    pGPRSMSGInfo pGPRS = GPRSMsgInfoGet();
    
    u32 temp = pGPRS->TranMinTimeCnt;
    
    *data = temp / 10;   
    
    return TRUE;      
}

#endif

#ifdef BMS_ENABLE_LITTLE_CURRENT_DET
//读地址364    小电流采样P1基准值
static u8 mb_data_LowCurrP1BaseVal_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    *data = gSampleData.P1ADBaseVolt;   
    
    return TRUE;      
}

//读地址365    小电流采样P2基准值
static u8 mb_data_LowCurrP2BaseVal_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    *data = gSampleData.P2ADBaseVolt;   
    
    return TRUE;      
}

//读地址366    小电流采样P1电流值
static u8 mb_data_LowCurrP1Curr_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    *data = gSampleData.P1LowCurrVal;   
    
    return TRUE;      
}

//读地址367    小电流采样P2电流值
static u8 mb_data_LowCurrP2Curr_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    *data = gSampleData.P2LowCurrVal;   
    
    return TRUE;      
}

#endif

//读地址360    预放电电阻温度
static u8 mb_data_PreDsgTemp_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    *data = gSampleData.PreTemp + 400;   
    
    return TRUE;      
}

//读地址361    预放电电阻温度
static u8 mb_data_CalibCyc_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    *data = gSOCInfo.Calibcycles;   
    
    return TRUE;      
}

/***************************** BMS运行控制读函数集合 *****************************/
//读地址512 - 528
static u8 mb_ctrl_r(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -= 512;
    *data = *(((uint16_t*)&gMBCtrl)+mbaddr);
    return TRUE;
}

//读地址529
static u8 mb_ctrl_afeccal_r(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -= 512;

    //电流标定
    if(SH367309EventFlagGet() & SH367309_EVE_CTRL_CAL)
    {
        *data = *(((uint16_t*)&gMBCtrl)+mbaddr);
    }
    else
    {
        *data = 0;
    }
    return TRUE;
}

//读地址530
static u8 mb_ctrl_b16vcal_r(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -= 512;
    *data = 0;
    //16节电压标定
//    if(SH367309EventFlagGet() & BQ76940_EVE_CTRL_V16CAL)
//    {
//        *data = *(((uint16_t*)&gMBCtrl)+mbaddr);
//    }
//    else
//    {
//        *data = 0;
//    }
    return TRUE;
}

//读地址531 - 533
static u8 mb_ctrl_rtc_r(uint16_t mbaddr,uint16_t *data)
{
    uint8_t *p= (uint8_t*)(&gRdTimeAndDate) + ((mbaddr -531) * 2);

    *data = (((uint16_t)*p)<<8) | *(p+1);
    return TRUE;
}

//读地址534
static u8 mb_ctrl_rst_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    *data = 0;
    return TRUE;
}

//读地址535
static u8 mb_ctrl_rec_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    //事件记录间隔
    *data = grecdiv;
    //*data = g_BMS_DATA.rec_div;
    return TRUE;
}

//读地址536
static u8 mb_ctrl_balctl_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gVoltBalance.testbalctrl;
    //*data = g_BMS_DATA.balance_test;
    return TRUE;
}

//读地址537
static u8 mb_ctrl_tctrl_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    *data = gPCBTest.presentstep;
    return TRUE;
}

//读地址538
static u8 mb_ctrl_tres_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    *data = gPCBTest.result;
    return TRUE;
}

#if defined(USE_B20_TOPBAND_PCBA) || defined(USE_B22_IM_PCBA)
//读地址555
static u8 mb_ctrl_balctl_17_20_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = (gVoltBalance.testbalctrl >> 16);
    //*data = g_BMS_DATA.balance_test;
    return TRUE;
}
#elif defined(USE_B21_IM_PCBA)
#else
#endif

/***************************** BMS参数配置读函数集合 *****************************/
//读地址768
static u8 mb_cfg_nvmcmd_r(u16 mbaddr,u16 *data)
{
    (void)mbaddr;
    *data = gNVMPermitCmd;
    return TRUE;
}

//读地址769 - 772
static u8 mb_cfg_sn_r(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -=769;
    *data = *(gConfig.sn+mbaddr);
    return TRUE;
}

//读地址773
static u8 mb_cfg_sovp_r(uint16_t mbaddr,uint16_t *data)//过压保护，ov level4
{
    (void)mbaddr;

    *data = gSHAFECfg.ovdval;
    //*data = gConfig.cellov.matth[3];//
    //*data  = g_AFE_VAR.ovd_act;
    return TRUE;
}

//读地址774
static u8 mb_cfg_sovpr_r(uint16_t mbaddr,uint16_t *data)//过压保护恢复，ov level4
{
    (void)mbaddr;
    *data = gConfig.cellov.resth[3];
    //*data = gConfig.cellov.resth[3];
    return TRUE;
}

//读地址775
static u8 mb_cfg_suvp1_r(uint16_t mbaddr,uint16_t *data)//初级欠压保护，uv level3
{
    (void)mbaddr;
    *data = gConfig.celluv.matth[2];
    return TRUE;
}

//读地址776
static u8 mb_cfg_suvpt1_r(uint16_t mbaddr,uint16_t *data)//初级欠压保护延时，uv level3
{
    (void)mbaddr;
    *data = gConfig.celluv.mattime[2];
    return TRUE;
}

//读地址777
static u8 mb_cfg_suvpr1_r(uint16_t mbaddr,uint16_t *data)//初级欠压保护恢复，uv level3
{
    (void)mbaddr;
    *data = gConfig.celluv.resth[2];
    return TRUE;
}

//读地址778
static u8 mb_cfg_suvpsoc1_r(uint16_t mbaddr,uint16_t *data)//初级欠压保护SOC，usoc level4
{
    (void)mbaddr;
    *data = gConfig.usoc.matth[3];
    return TRUE;
}

//读地址779
static u8 mb_cfg_suvpsoct1_r(uint16_t mbaddr,uint16_t *data)//初级欠压保护SOC延时，usoc level4
{
    (void)mbaddr;
    *data = gConfig.usoc.mattime[3];
    return TRUE;
}

//读地址780
static u8 mb_cfg_suvpsocr1_r(uint16_t mbaddr,uint16_t *data)//初级欠压保护SOC恢复，usoc level4
{
    (void)mbaddr;
    *data = gConfig.usoc.resth[3];
    return TRUE;
}

//读地址781
static u8 mb_cfg_suvp2_r(uint16_t mbaddr,uint16_t *data)//二级欠压保护，uv level4
{
    (void)mbaddr;
    *data = gSHAFECfg.uvdval;
    //*data  = g_AFE_VAR.uvd_act;
    return TRUE;
}

//读地址782
static u8 mb_cfg_suvpr2_r(uint16_t mbaddr,uint16_t *data)//二级欠压保护恢复，uv level4
{
    (void)mbaddr;
    *data = gConfig.celluv.resth[3];
    return TRUE;
}

//读地址783
static u8 mb_cfg_cocp_r(uint16_t mbaddr,uint16_t *data)//充电过流保护，ccurr level4
{
    (void)mbaddr;
    *data = gConfig.ccurr.matth[3];
    return TRUE;
}

//读地址784
static u8 mb_cfg_cocpt_r(uint16_t mbaddr,uint16_t *data)//充电过流保护延时，ccurr level4
{
    (void)mbaddr;
    *data = gConfig.ccurr.mattime[3];
    return TRUE;
}

//读地址785
static u8 mb_cfg_docp1_r(uint16_t mbaddr,uint16_t *data)//初级放电过流保护，dcurr level2
{
    (void)mbaddr;
    *data = 0-gConfig.dcurr.matth[1];
    //*data = gSHAFECfg.ocd1val;
    return TRUE;
}

//读地址786
static u8 mb_cfg_docpt1_r(uint16_t mbaddr,uint16_t *data)//初级放电过流保护延时，dcurr level2
{
    (void)mbaddr;
    *data = gConfig.dcurr.mattime[1];
    return TRUE;
}

//读地址787
static u8 mb_cfg_docp2_r(uint16_t mbaddr,uint16_t *data)//二级放电过流保护，dcurr level3
{
    (void)mbaddr;
    *data = gSHAFECfg.ocd2val;
    //*data  = g_AFE_VAR.ocd_act;
    return TRUE;
}

//读地址788
static u8 mb_cfg_dscp_r(uint16_t mbaddr,uint16_t *data)//短路放电过流保护，dcurr level4
{
    (void)mbaddr;
    *data = gSHAFECfg.scdval;
    //*data  = g_AFE_VAR.scd_act;
    return TRUE;
}

//读地址789
static u8 mb_cfg_cotp_r(uint16_t mbaddr,uint16_t *data)//充电高温保护，cot level4
{
    (void)mbaddr;
    *data = gConfig.cellcot.matth[3]+400;
    return TRUE;
}

//读地址790
static u8 mb_cfg_cotpt_r(uint16_t mbaddr,uint16_t *data)//充电高温保护延时，cot level4
{
    (void)mbaddr;
    *data = gConfig.cellcot.mattime[3];
    return TRUE;
}

//读地址791
static u8 mb_cfg_cotpr_r(uint16_t mbaddr,uint16_t *data)//充电高温保护延时，cot level4
{
    (void)mbaddr;
    *data = gConfig.cellcot.resth[3]+400;
    return TRUE;
}

//读地址792
static u8 mb_cfg_dotp_r(uint16_t mbaddr,uint16_t *data)//放电高温保护，dot level4
{
    (void)mbaddr;
    *data = gConfig.celldot.matth[3]+400;
    return TRUE;
}

//读地址793
static u8 mb_cfg_dotpt_r(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，dot level4
{
    (void)mbaddr;
    *data = gConfig.celldot.mattime[3];
    return TRUE;
}

//读地址794
static u8 mb_cfg_dotpr_r(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，dot level4
{
    (void)mbaddr;
    *data = gConfig.celldot.resth[3]+400;
    return TRUE;
}

//读地址795
static u8 mb_cfg_cutp_r(uint16_t mbaddr,uint16_t *data)//充电高温保护，cut level4
{
    (void)mbaddr;
    *data = gConfig.cellcut.matth[3]+400;
    return TRUE;
}

//读地址796
static u8 mb_cfg_cutpt_r(uint16_t mbaddr,uint16_t *data)//充电高温保护延时，cut level4
{
    (void)mbaddr;
    *data = gConfig.cellcut.mattime[3];
    return TRUE;
}

//读地址797
static u8 mb_cfg_cutpr_r(uint16_t mbaddr,uint16_t *data)//充电高温保护恢复，cut level4
{
    (void)mbaddr;
    *data = gConfig.cellcut.resth[3]+400;
    return TRUE;
}

//读地址798
static u8 mb_cfg_dutp_r(uint16_t mbaddr,uint16_t *data)//放电高温保护，dut level4
{
    (void)mbaddr;
    *data = gConfig.celldut.matth[3]+400;
    return TRUE;
}

//读地址799
static u8 mb_cfg_dutpt_r(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，dut level4
{
    (void)mbaddr;
    *data = gConfig.celldut.mattime[3];
    return TRUE;
}

//读地址800
static u8 mb_cfg_dutpr_r(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，dut level4
{
    (void)mbaddr;
    *data = gConfig.celldut.resth[3]+400;
    return TRUE;
}

//读地址801
static u8 mb_cfg_mosotp_r(uint16_t mbaddr,uint16_t *data)//放电高温保护，mosot level4
{
    (void)mbaddr;
    *data = gConfig.pcbot.matth[3]+400;
    return TRUE;
}

//读地址802
static u8 mb_cfg_mosotpt_r(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，mosot level4
{
    (void)mbaddr;
    *data = gConfig.pcbot.mattime[3];
    return TRUE;
}

//读地址803
static u8 mb_cfg_mosotpr_r(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，mosot level4
{
    (void)mbaddr;
    *data = gConfig.pcbot.resth[3]+400;
    return TRUE;
}

//读地址804
static u8 mb_cfg_conotp_r(uint16_t mbaddr,uint16_t *data)//放电高温保护，conot level4
{
    (void)mbaddr;
    *data = gConfig.connot.matth[3]+400;
    return TRUE;
}

//读地址805
static u8 mb_cfg_conotpt_r(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，conot level4
{
    (void)mbaddr;
    *data = gConfig.connot.mattime[3];
    return TRUE;
}

//读地址806
static u8 mb_cfg_conotpr_r(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，conot level4
{
    (void)mbaddr;
    *data = gConfig.connot.resth[3]+400;
    return TRUE;
}

//读地址807
static u8 mb_cfg_sovw_r(uint16_t mbaddr,uint16_t *data)//过压告警，ov level3
{
    (void)mbaddr;
    *data = gConfig.cellov.matth[2];
    return TRUE;
}

//读地址808
static u8 mb_cfg_sovwt_r(uint16_t mbaddr,uint16_t *data)//过压告警延时，ov level3
{
    (void)mbaddr;
    *data = gConfig.cellov.mattime[2];
    return TRUE;
}

//读地址809
static u8 mb_cfg_sovwr_r(uint16_t mbaddr,uint16_t *data)//过压告警恢复，ov level3
{
    (void)mbaddr;
    *data = gConfig.cellov.resth[2];
    return TRUE;
}

//读地址810
static u8 mb_cfg_suvw_r(uint16_t mbaddr,uint16_t *data)//欠压告警，uv level2
{
    (void)mbaddr;
    *data = gConfig.celluv.matth[1];
    return TRUE;
}

//读地址811
static u8 mb_cfg_suvwt_r(uint16_t mbaddr,uint16_t *data)//欠压告警延时，uv level2
{
    (void)mbaddr;
    *data = gConfig.celluv.mattime[1];
    return TRUE;
}

//读地址812
static u8 mb_cfg_suvwr_r(uint16_t mbaddr,uint16_t *data)//欠压告警恢复，uv level2
{
    (void)mbaddr;
    *data = gConfig.celluv.resth[1];
    return TRUE;
}

//读地址813
static u8 mb_cfg_cocw_r(uint16_t mbaddr,uint16_t *data)//充电过流告警，ccurr level3
{
    (void)mbaddr;
    *data = gConfig.ccurr.matth[0];
    return TRUE;
}

//读地址814
static u8 mb_cfg_cocwt_r(uint16_t mbaddr,uint16_t *data)//充电过流告警延时，ccurr level3
{
    (void)mbaddr;
    *data = gConfig.ccurr.mattime[0];
    return TRUE;
}

//读地址815
static u8 mb_cfg_cocwr_r(uint16_t mbaddr,uint16_t *data)//充电过流告警恢复，ccurr level3
{
    (void)mbaddr;
    *data = gConfig.ccurr.resth[0];
    return TRUE;
}

//读地址816
static u8 mb_cfg_docw_r(uint16_t mbaddr,uint16_t *data)//放电过流告警，dcurr level1
{
    (void)mbaddr;
    *data = 0-gConfig.dcurr.matth[0];
    return TRUE;
}

//读地址817
static u8 mb_cfg_docwt_r(uint16_t mbaddr,uint16_t *data)//放电过流告警延时，dcurr level1
{
    (void)mbaddr;
    *data = gConfig.dcurr.mattime[0];
    return TRUE;
}

//读地址818
static u8 mb_cfg_docwr_r(uint16_t mbaddr,uint16_t *data)//充电过流告警恢复，dcurr level1
{
    (void)mbaddr;
    *data = 0-gConfig.dcurr.resth[0];
    return TRUE;
}

//读地址819
static u8 mb_cfg_cotw_r(uint16_t mbaddr,uint16_t *data)//充电高温告警，cot level3
{
    (void)mbaddr;
    *data = gConfig.cellcot.matth[2]+400;
    return TRUE;
}

//读地址820
static u8 mb_cfg_cotwt_r(uint16_t mbaddr,uint16_t *data)//充电高温告警延时，cot level3
{
    (void)mbaddr;
    *data = gConfig.cellcot.mattime[2];
    return TRUE;
}

//读地址821
static u8 mb_cfg_cotwr_r(uint16_t mbaddr,uint16_t *data)//充电高温告警恢复，cot level3
{
    (void)mbaddr;
    *data = gConfig.cellcot.resth[2]+400;
    return TRUE;
}

//读地址822
static u8 mb_cfg_dotw_r(uint16_t mbaddr,uint16_t *data)//放电高温告警，dot level3
{
    (void)mbaddr;
    *data = gConfig.celldot.matth[2]+400;
    return TRUE;
}

//读地址823
static u8 mb_cfg_dotwt_r(uint16_t mbaddr,uint16_t *data)//放电高温告警延时，dot level3
{
    (void)mbaddr;
    *data = gConfig.celldot.mattime[2];
    return TRUE;
}

//读地址824
static u8 mb_cfg_dotwr_r(uint16_t mbaddr,uint16_t *data)//放电高温告警恢复，dot level3
{
    (void)mbaddr;
    *data = gConfig.celldot.resth[2]+400;
    return TRUE;
}

//读地址825
static u8 mb_cfg_cutw_r(uint16_t mbaddr,uint16_t *data)//充电高温告警，cut level3
{
    (void)mbaddr;
    *data = gConfig.cellcut.matth[2]+400;
    return TRUE;
}

//读地址826
static u8 mb_cfg_cutwt_r(uint16_t mbaddr,uint16_t *data)//充电高温告警延时，cut level3
{
    (void)mbaddr;
    *data = gConfig.cellcut.mattime[2];
    return TRUE;
}

//读地址827
static u8 mb_cfg_cutwr_r(uint16_t mbaddr,uint16_t *data)//充电高温告警恢复，cut level3
{
    (void)mbaddr;
    *data = gConfig.cellcut.resth[2]+400;
    return TRUE;
}

//读地址828
static u8 mb_cfg_dutw_r(uint16_t mbaddr,uint16_t *data)//放电高温告警，dut level3
{
    (void)mbaddr;
    *data = gConfig.celldut.matth[2]+400;
    return TRUE;
}

//读地址829
static u8 mb_cfg_dutwt_r(uint16_t mbaddr,uint16_t *data)//放电高温告警延时，dut level3
{
    (void)mbaddr;
    *data = gConfig.celldut.mattime[2];
    return TRUE;
}

//读地址830
static u8 mb_cfg_dutwr_r(uint16_t mbaddr,uint16_t *data)//放电高温告警恢复，dut level3
{
    (void)mbaddr;
    *data = gConfig.celldut.resth[2]+400;
    return TRUE;
}

//读地址831
static u8 mb_cfg_afecg_r(uint16_t mbaddr,uint16_t *data)//电流校准值
{
    (void)mbaddr;
    *data = 0;
    //电流校准
    *data = gConfig.calpara.sampleres;
    return TRUE;
}

//读地址832
static u8 mb_cfg_b16vg_r(uint16_t mbaddr,uint16_t *data)//B16电压校准值
{
    (void)mbaddr;
    *data = 0;
    //B16电压校准值
    *data = gConfig.calpara.b16gain;
    return TRUE;
}

//读地址834
static u8 mb_cfg_tvsotp_r(uint16_t mbaddr,uint16_t *data)//TVS管高温保护，tvsot level4
{
    (void)mbaddr;

    *data = gConfig.tvsot.matth[3]+400;

    return TRUE;
}

//读地址835
static u8 mb_cfg_tvsotpt_r(uint16_t mbaddr,uint16_t *data)//TVS管高温保护延时，tvsot level4
{
    (void)mbaddr;

    *data = gConfig.tvsot.mattime[3];

    return TRUE;
}

//读地址836
static u8 mb_cfg_tvsotpr_r(uint16_t mbaddr,uint16_t *data)//TVS管高温保护恢复值，tvsot level4
{
    (void)mbaddr;

    *data = gConfig.tvsot.resth[3]+400;

    return TRUE;
}

//读地址837 B6无保险丝温度，暂不使用
static u8 mb_cfg_fuseotp_r(uint16_t mbaddr,uint16_t *data)//fuse高温保护，tvsot level4
{
    (void)mbaddr;
    *data = 0xffff;
    return true;
}

//读地址838 B6无保险丝温度，暂不使用
static u8 mb_cfg_fuseotpt_r(uint16_t mbaddr,uint16_t *data)//fuse高温保护延时，tvsot level4
{
    (void)mbaddr;
    *data = 0xffff;
    return true;
}

//读地址839 B6无保险丝温度，暂不使用
static u8 mb_cfg_fuseotpr_r(uint16_t mbaddr,uint16_t *data)//fuse高温保护恢复值，tvsot level4
{
    (void)mbaddr;
    *data = 0xffff;
    return true;
}

//读地址840 大电流检测配置读取
static u8 mb_cfg_currdetcig_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
	
    *data = gConfig.configadd.currdet.highcurrdet;
	
    return true;
}

////读地址853 gRense分流器阻值
//static u8 mb_cfg_grense_r(uint16_t mbaddr,uint16_t *data)//电量计分流器阻值查询
//{
//    (void)mbaddr;
//    
//    #ifdef    BMS_USE_MAX172XX
//    *data = gMax17205Cal.rsense;
//    #else
//    *data = 0;
//    #endif

//    return TRUE;
//}

//读地址853
static u8 mb_cfg_pretotp_r(uint16_t mbaddr,uint16_t *data)//预放电阻高温保护，pretot level4
{
    (void)mbaddr;

    *data = gConfig.configadd.preotemp.matth[3]+400;

    return TRUE;
}

//读地址854
static u8 mb_cfg_pretotpt_r(uint16_t mbaddr,uint16_t *data)//预放电阻高温保护延时，pretot level4
{
    (void)mbaddr;

    *data = gConfig.configadd.preotemp.mattime[3];

    return TRUE;
}

//读地址855
static u8 mb_cfg_pretotpr_r(uint16_t mbaddr,uint16_t *data)//预放电阻高温保护恢复值，pretot level4
{
    (void)mbaddr;

    *data = gConfig.configadd.preotemp.resth[3]+400;

    return TRUE;
}

//读地址 862 电量计至少剩余的更新次数，避免一次用完
static u8 mb_cfg_fuel_atlearem_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    *data = gConfig.calpara.FuelAtLesRem;
    
    return TRUE;
}

//读地址 863 电量计电流采样零飘值
static u8 mb_cfg_fuel_coff_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gConfig.calpara.COff;

    return TRUE;
}

//读地址 864 电量计电流采样斜率值
static u8 mb_cfg_fuel_cgain_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gConfig.calpara.CGain;
    
    return TRUE;
}

//读地址 865 afe零飘电流
static u8 mb_cfg_afe_coff_r(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    *data = gSHAFECfg.Cadcoffset;

    return TRUE;
}

/***************************** 用户数据区读函数集合 *****************************/

//读地址4096
static u8 mb_user_cmd_r(uint16_t mbaddr,uint16_t *data)
{
    if(True == gModVoltTestCmd)
        gUserPermitCmd |= USER_MOD_VOLT_CMD;
    else
        gUserPermitCmd &= ~USER_MOD_VOLT_CMD;
    
    *data = gUserPermitCmd;
    return TRUE;
}

//读地址4097 - 4100
static u8 mb_user_data_r(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -=4097;
    *data = *(((uint16_t*)&gUserID.user)+mbaddr);
    return TRUE;
}

/********************************************************* 写函数集合 *********************************************************/
//写空地址
static u8 mb_writenull_w(u16 mbaddr,u16 *data)
{
    (void)mbaddr;
    //*data = 0xFFFF;
    return TRUE;
}

/***************************** BMS运行控制写函数集合 *****************************/

//写地址512
u8 mb_ctrl_ctrl_w(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -= 512;
    if((gPCBTest.sta == 1)&&(gPCBTest.presentstep == 8)&&(gPCBTest.nextstep == 0)&&(*data != (CTRL1_CHG_MASK|CTRL1_DSG_MASK|CTRL1_CPI_MASK)))//处于老化模式时不响应ctrl额外的控制
    {
        return TRUE;
    }

    if((*data & CTRL1_CHG_MASK) && !(gMBCtrl.ctrl&CTRL1_CHG_MASK))//
    {

        //gOCLock &= ~0x1;
    	//控制充电开关控制和放电开关控制
        if(gOCLock&0x01)
        {
            DTCheckOCLockStatusClr(0x1);
        }
    }
    if((*data & CTRL1_DSG_MASK) && !(gMBCtrl.ctrl&CTRL1_DSG_MASK))//
    {
    	//控制充电开关控制和放电开关控制
        //gOCLock &= ~0x2;
        if(gOCLock&0x02)
        {
            DTCheckOCLockStatusClr(0x02);
        }
    }
    if((*data & CTRL1_PDS_MASK) && !(gMBCtrl.ctrl&CTRL1_PDS_MASK))
    {
        gMCData.predsg_flag = 0 ;//复位预放电标志
    }
    else if (((*data & CTRL1_PDS_MASK)==0)&&((*data & CTRL1_DSG_MASK)==0))
    {
        if(gMCData.predsg_ctrl)
        {
            gMCData.predsg_ctrl = 0;
            gMCData.predsg_inner5s = 0;
            //task_delete(gMCData.predsg_task);
        }
    }
    if((*data & CTRL1_LOP_MASK)==0 /*&& !(gMBCtrl.ctrl&CTRL1_LOP_MASK)*/)//写零可清除
    {
        //SleepModeClearData();   //浅层休眠重新置位后复位睡眠信号
        //bms_sleep_process_reset(&gSleepCheckData);
    }
    *(((uint16_t*)&gMBCtrl)+mbaddr) = *data;
    return TRUE;
}

//写地址513 - 528,531,532
static u8 mb_ctrl_w(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -= 512;
    *(((uint16_t*)&gMBCtrl)+mbaddr) = *data;
    return TRUE;
}

//写地址529
static u8 mb_ctrl_afeccal_w(uint16_t mbaddr,uint16_t *data)
{
    if((gNVMPermitCmd&NVM_PERMIT_CMD_EN)==0)
        return FALSE;
    mbaddr -= 512;
    *(((uint16_t*)&gMBCtrl)+mbaddr) = *data;

    if((*data<=29500) || (*data>=30500))
    {
    	//电流标定
        SH367309CalCurrent((int16_t)*data - 30000);
    	//MAX17205SetCalCurr(((int16_t)*data -30000) * 10);  //mA为单位输入，Handry
    }
    else if(30000 == *data)
    {
        SH367309CalCurrent(0);
    }

    return TRUE;
}

//写地址530
static u8 mb_ctrl_b16vcal_w(uint16_t mbaddr,uint16_t *data)
{
    if((gNVMPermitCmd & NVM_PERMIT_CMD_EN)==0)
    {
        return FALSE;
    }

    mbaddr -= 512;

    *(((uint16_t*)&gMBCtrl)+mbaddr) = *data;
    if(*data>=1000)
    {
    	//无第16节单体电压标定
        //BQ76940CalV16((int16_t)*data);
    }
    return TRUE;
}

//写地址533
static u8 mb_ctrl_mnsc_w(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    gMBCtrl.mnsc = *data;
    gWrTimeAndDate.year = gMBCtrl.yrmo>>8;
    gWrTimeAndDate.month = gMBCtrl.yrmo&0xff;
    gWrTimeAndDate.day = gMBCtrl.dthr>>8;
    gWrTimeAndDate.hour = gMBCtrl.dthr&0xff;
    gWrTimeAndDate.minute = gMBCtrl.mnsc>>8;
    gWrTimeAndDate.second = gMBCtrl.mnsc&0xff;

    //标定时间
	PCF85063SetCtrl(PCF85063_START_WRITE);
    //rtc_time_set(&t);

    return TRUE;
}

//写地址534
static u8 mb_ctrl_rst_w(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    
    //跳转BootLoader
    if(*data == 0x7AB9)
    {
    	gReSetCmd = 0x7AB9;
        //GPIO_IntDisable(NFC_INT_MASK);//禁止NFC中断
        //uint8_t tid;
        //timer_allocation(&tid);
        //timer_config(tid,1000,0,NULL);
        //while(timer_check(tid)==0)
        //{
        //    WDOG_Feed();
        //}
					//清零中断标志位，读清除
//        if(0 == BSPGPIOGetPin(NFC_INT_PORT,NFC_INT_PIN))
//        {
//            FM11NC08ReadReg(NFC_MAIN_IRQ);
//            FM11NC08ReadReg(NFC_FIFO_IRQ);		
//            FM11NC08ReadReg(NFC_AUX_IRQ);
//            FM11NC08ReadReg(NFC_MAIN_IRQ_MASK);
//            FM11NC08ReadReg(NFC_FIFO_IRQ_MASK);		
//            FM11NC08ReadReg(NFC_AUX_IRQ_MASK);         
//        }   
//        BSPEEPWriteOneWord(e_BOOT_STAY_FLAG_ADDR,BOOT_STAY_FLAG);           //写入boot停留标志
        //BSPGPIODisablePullUp(NFC_INT_PORT,NFC_INT_PIN);						//引脚不拉高
        //SoftReset();	
				WriteFlagToFlash(e_BOOT_STAY_FLAG_ADDR,BOOT_STAY_FLAG,8);           //写入boot停留标志
    }
    
    //复位
    if(*data == 0x7ABA)
    {  
        gReSetCmd = 0x7ABA;
        //BSPGPIODisablePullUp(NFC_INT_PORT,NFC_INT_PIN);						//引脚不拉高
        SoftReset(); 
    }   
    
    return TRUE;
}

//写地址535
u8 mb_ctrl_rec_w(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    if(*data <= 120)
    	grecdiv = *data;
        //g_BMS_DATA.rec_div = *data;
    else
    	grecdiv = 0;
        //g_BMS_DATA.rec_div = 0;
    return TRUE;
}

//写地址536
u8 mb_ctrl_balctl_w(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    if(*data & VoltBalanceGetBatValidMask(0))
    	gVoltBalance.testbalctrl = *data & VoltBalanceGetBatValidMask(0);
    else
    	gVoltBalance.testbalctrl = *data & VoltBalanceGetBatValidMask(1);
    /*
    if(*data&bms_balance_mask(0))
        g_BMS_DATA.balance_test = *data&bms_balance_mask(0);
    else
        g_BMS_DATA.balance_test = *data&bms_balance_mask(1);
        */

    return TRUE;
}

//写地址537
static u8 mb_ctrl_tctrl_w(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    if((*data>=0x5701)&&(*data<=0x5709))
    {
        gPCBTest.nextstep = *data-0x5700;
    }
    else if(*data == 0x57AB)
    {
        if(gPCBTest.sta == 0)
        {
        	gPCBTest.en = 1;	//此处执行测试函数
        	BSPTaskStart(TASK_ID_PCBTEST_TASK, 3);
            //task_add(pcbatest_task,&gPCBTest);
        }
        else
        {
            gPCBTest.nextstep = gPCBTest.presentstep+1;
        }
    }
    else
    {
        gPCBTest.nextstep = 0xff;
    }
    return TRUE;
}

//写地址538
static u8 mb_ctrl_tres_w(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    return TRUE;
}

#if defined(USE_B20_TOPBAND_PCBA) || defined(USE_B22_IM_PCBA)
//写地址555
u8 mb_ctrl_balctl_17_20_w(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    if(*data & VoltBalanceGetBatValidMask(0))
    	gVoltBalance.testbalctrl |= ((u32)(*data & VoltBalanceGetBatValidMask(0))) << 16;
    else
    	gVoltBalance.testbalctrl |= ((u32)(*data & VoltBalanceGetBatValidMask(1))) << 16;
    
    return TRUE;
}
#elif defined(USE_B21_IM_PCBA)
#else
#endif


/***************************** BMS参数配置写函数集合 *****************************/

//写地址768
static u8 mb_cfg_nvmcmd_w(u16 mbaddr,u16 *data)
{
    (void)mbaddr;

    (void)mbaddr;
    if(*data == 0x1235)//复位除校准值外的所有默认配置
    {

        gConfigBuff = cConfigInit;
        gConfigBuff.calpara = gConfig.calpara;
        
        //BQ76940OUVConfig(gConfigBuff.cellov.matth[3],gConfigBuff.celluv.matth[3]);
        //OVH OVL OVRH OVRL UV UVR 充放电保护和恢复值配置
        SH367309OUVConfig(gConfigBuff.cellov.matth[3],gConfigBuff.cellov.resth[3],\
                          gConfigBuff.celluv.matth[3],gConfigBuff.celluv.resth[3]);        
        
        //BQ76940SOCDConfig(0-gConfigBuff.dcurr.matth[3],0-gConfigBuff.dcurr.matth[2]);
        //OCD1 OCD2 SC OCC 充放电过流、短路保护配置
        SH367309OCConfig(0 - gConfigBuff.dcurr.matth[1],0-gConfigBuff.dcurr.matth[2],0 - gConfigBuff.dcurr.matth[3],\
                         SH367309_CFG_OCC); //充电过流配置为20A        
        
        gStorageCTRL |= FLASH_WR_CFG_MASK;

    }
    else if(*data == 0x1237)//仅复位序列号
    {
        gConfigBuff.sn[0] = 0xffff;
        gConfigBuff.sn[1] = 0xffff;
        gConfigBuff.sn[2] = 0xffff;
        gConfigBuff.sn[3] = 0xffff;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    else if(*data == 0x1238)//仅复位参数配置
    {
        gConfigBuff = cConfigInit;
        gConfigBuff.calpara = gConfig.calpara;
        gConfigBuff.sn[0] = gConfig.sn[0];
        gConfigBuff.sn[1] = gConfig.sn[1];
        gConfigBuff.sn[2] = gConfig.sn[2];
        gConfigBuff.sn[3] = gConfig.sn[3];
        
        //BQ76940OUVConfig(gConfigBuff.cellov.matth[3],gConfigBuff.celluv.matth[3]);
        //OVH OVL OVRH OVRL UV UVR 充放电保护和恢复值配置
        SH367309OUVConfig(gConfigBuff.cellov.matth[3],gConfigBuff.cellov.resth[3],\
                          gConfigBuff.celluv.matth[3],gConfigBuff.celluv.resth[3]);        
        
        //BQ76940SOCDConfig(0-gConfigBuff.dcurr.matth[3],0-gConfigBuff.dcurr.matth[2]);
        //OCD1 OCD2 SC OCC 充放电过流、短路保护配置
        SH367309OCConfig(0 - gConfigBuff.dcurr.matth[1],0-gConfigBuff.dcurr.matth[2],0 - gConfigBuff.dcurr.matth[3],\
                         SH367309_CFG_OCC); //充电过流配置为20A  
        
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    else if(*data == 0x1246)//复位所有配置参数包括校准值
    {
        gConfigBuff = cConfigInit;
        
        //BQ76940OUVConfig(gConfigBuff.cellov.matth[3],gConfigBuff.celluv.matth[3]);
        //OVH OVL OVRH OVRL UV UVR 充放电保护和恢复值配置
        SH367309OUVConfig(gConfigBuff.cellov.matth[3],gConfigBuff.cellov.resth[3],\
                          gConfigBuff.celluv.matth[3],gConfigBuff.celluv.resth[3]);        
        
        //BQ76940SOCDConfig(0-gConfigBuff.dcurr.matth[3],0-gConfigBuff.dcurr.matth[2]);
        //OCD1 OCD2 SC OCC 充放电过流、短路保护配置
        SH367309OCConfig(0 - gConfigBuff.dcurr.matth[1],0-gConfigBuff.dcurr.matth[2],0 - gConfigBuff.dcurr.matth[3],\
                         SH367309_CFG_OCC); //充电过流配置为20A 
        
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    else if(*data == 0x1253)//reset cycles to 1
    {
    	/*
        g_FUEL_DFFS.cycles=1;
        g_BQ27541_VAR.ctrl |= FUEL_CTRL_SET_CYCLES;
        */
        #ifdef    BMS_USE_MAX172XX
        gMax17205Var.CycleCount = 1;
        gMax17205Cal.wrtcycle = 7;  
        Max17205SetCtrl(MAX1720X_WRITE_CYCLES);
        #else
        
        #endif
    }
    else if(*data == 0x1259)//configure repaired-ID
    {
        if(gConfig.sn[1]!=0xFFFF)
        {
            uint16_t temp;
            temp=(gConfig.sn[1]&0x0FF0)>>4;
            if(temp>=100 && temp<0xFF)
            {
                temp++;
            }
            else
            {
                temp=100;
            }
            gConfigBuff.sn[0] = gConfig.sn[0];
            gConfigBuff.sn[1]=gConfig.sn[1]&(~0x0FF0);
            gConfigBuff.sn[1]|=(temp<<4)&0x0FF0;
            gConfigBuff.sn[2] = gConfig.sn[2];
            gConfigBuff.sn[3] = gConfig.sn[3];
            gStorageCTRL |= FLASH_WR_CFG_MASK;
        }
    }
    else if(*data == 0x625A)
    {
        gNVMPermitCmd |= NVM_PERMIT_CMD_EN;
    }
    else if(*data != 0)
    {
        //gConfigBuff = gConfig;
        gNVMPermitCmd &= ~NVM_PERMIT_CMD_EN;
    }

    return TRUE;
}

//写地址769 - 772
static u8 mb_cfg_sn_w(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -=769;

    if(*(gConfig.sn+mbaddr) ==0xffff)
    {
        *(gConfigBuff.sn+mbaddr) = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 773
static u8 mb_cfg_sovp_w(uint16_t mbaddr,uint16_t *data)//过压保护，ov level4
{
    (void)mbaddr;
    //*data = gConfig.cellov.matth[3];//
    if(*data>=2000 && *data<=4500)
    {
        gConfigBuff.cellov.matth[DT_LV4] = *data;
        //BQ76940OUVConfig(gConfigBuff.cellov.matth[3],gConfigBuff.celluv.matth[3]);
        //OVH OVL OVRH OVRL UV UVR 充放电保护和恢复值配置
        SH367309OUVConfig(gConfig.cellov.matth[3],gConfig.cellov.resth[3],\
                          gConfig.celluv.matth[3],gConfig.celluv.resth[3]);
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 774
static u8 mb_cfg_sovpr_w(uint16_t mbaddr,uint16_t *data)//过压保护恢复，ov level4
{
    (void)mbaddr;
    if(*data>=2000 && *data<=4500)
    {
        gConfigBuff.cellov.resth[DT_LV4] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 775
static u8 mb_cfg_suvp1_w(uint16_t mbaddr,uint16_t *data)//初级欠压保护，uv level3
{
    (void)mbaddr;
    if(*data>=2000 && *data<=4500)
    {
        gConfigBuff.celluv.matth[2] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 776
static u8 mb_cfg_suvpt1_w(uint16_t mbaddr,uint16_t *data)//初级欠压保护延时，uv level3
{
    (void)mbaddr;
    if(*data<=120)
    {
        gConfigBuff.celluv.mattime[2] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 777
static u8 mb_cfg_suvpr1_w(uint16_t mbaddr,uint16_t *data)//初级欠压保护恢复，uv level3
{
    (void)mbaddr;
    if(*data>=2000 && *data<=4500)
    {
        gConfigBuff.celluv.resth[2] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 778
static u8 mb_cfg_suvpsoc1_w(uint16_t mbaddr,uint16_t *data)//初级欠压保护SOC，usoc level4
{
    (void)mbaddr;
    if(*data>=1 && *data<=1000)
    {
        gConfigBuff.usoc.matth[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 779
static u8 mb_cfg_suvpsoct1_w(uint16_t mbaddr,uint16_t *data)//初级欠压保护SOC延时，usoc level4
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.usoc.mattime[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 780
static u8 mb_cfg_suvpsocr1_w(uint16_t mbaddr,uint16_t *data)//初级欠压保护SOC恢复，usoc level4
{
    (void)mbaddr;
    if(*data>=1 && *data<=1000)
    {
        gConfigBuff.usoc.resth[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 781
static u8 mb_cfg_suvp2_w(uint16_t mbaddr,uint16_t *data)//二级欠压保护，uv level4
{
    (void)mbaddr;
    //*data = gConfig.celluv.matth[3];
    if(*data>=2000 && *data<=4500)
    {
        gConfigBuff.celluv.matth[3] = *data;
        //BQ76940OUVConfig(gConfigBuff.cellov.matth[3],gConfigBuff.celluv.matth[3]);
        //OVH OVL OVRH OVRL UV UVR 充放电保护和恢复值配置
        SH367309OUVConfig(gConfig.cellov.matth[3],gConfig.cellov.resth[3],\
                          gConfig.celluv.matth[3],gConfig.celluv.resth[3]);        
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 782
static u8 mb_cfg_suvpr2_w(uint16_t mbaddr,uint16_t *data)//二级欠压保护恢复，uv level4
{
    (void)mbaddr;
    if(*data>=2000 && *data<=4500)
    {
        gConfigBuff.celluv.resth[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 783
static u8 mb_cfg_cocp_w(uint16_t mbaddr,uint16_t *data)//充电过流保护，ccurr level4
{
    (void)mbaddr;
    if(*data>=1 && *data<=30000)
    {
        gConfigBuff.ccurr.matth[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 784
static u8 mb_cfg_cocpt_w(uint16_t mbaddr,uint16_t *data)//充电过流保护延时，ccurr level4
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.ccurr.mattime[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 785
static u8 mb_cfg_docp1_w(uint16_t mbaddr,uint16_t *data)//初级放电过流保护，dcurr level2
{
    (void)mbaddr;
    if(*data>=1 && *data<=30000)
    {
        gConfigBuff.dcurr.matth[1] = 0-(int16_t)*data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
        
        SH367309OCConfig(0 - gConfigBuff.dcurr.matth[1],0 - gConfig.dcurr.matth[2],0 - gConfig.dcurr.matth[3],\
                                 SH367309_CFG_OCC); //充电过流配置为20A                 
    }
    return TRUE;
}

//写地址 786
static u8 mb_cfg_docpt1_w(uint16_t mbaddr,uint16_t *data)//初级放电过流保护延时，dcurr level2
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.dcurr.mattime[1] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 787
static u8 mb_cfg_docp2_w(uint16_t mbaddr,uint16_t *data)//二级放电过流保护，dcurr level3
{
    (void)mbaddr;
    //*data = gConfig.dcurr.matth[2];
    if(*data>=1 && *data<=30000)
    {
        gConfigBuff.dcurr.matth[2] = 0-(int16_t)*data;
        //BQ76940SOCDConfig(0-gConfigBuff.dcurr.matth[3],0 - gConfigBuff.dcurr.matth[2]);
        //OCD1 OCD2 SC OCC 充放电过流、短路保护配置
        SH367309OCConfig(0 - gConfig.dcurr.matth[1],0-gConfigBuff.dcurr.matth[2],0 - gConfig.dcurr.matth[3],\
                         SH367309_CFG_OCC); //充电过流配置为20A         
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 788
static u8 mb_cfg_dscp_w(uint16_t mbaddr,uint16_t *data)//短路放电过流保护，dcurr level4
{
    (void)mbaddr;
    //*data = gConfig.dcurr.matth[3];
    if(*data>=1 && *data<=30000)
    {
        gConfigBuff.dcurr.matth[3] = 0-(int16_t)*data;
        //BQ76940SOCDConfig(0-gConfigBuff.dcurr.matth[3],0 - gConfigBuff.dcurr.matth[2]);
        //OCD1 OCD2 SC OCC 充放电过流、短路保护配置
        SH367309OCConfig(0 - gConfig.dcurr.matth[1],0-gConfig.dcurr.matth[2],0 - gConfigBuff.dcurr.matth[3],\
                         SH367309_CFG_OCC); //充电过流配置为20A        
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 789
static u8 mb_cfg_cotp_w(uint16_t mbaddr,uint16_t *data)//充电高温保护，cot level4
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.cellcot.matth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 790
static u8 mb_cfg_cotpt_w(uint16_t mbaddr,uint16_t *data)//充电高温保护延时，cot level4
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.cellcot.mattime[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 791
static u8 mb_cfg_cotpr_w(uint16_t mbaddr,uint16_t *data)//充电高温保护延时，cot level4
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.cellcot.resth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 792
static u8 mb_cfg_dotp_w(uint16_t mbaddr,uint16_t *data)//放电高温保护，dot level4
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.celldot.matth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 793
static u8 mb_cfg_dotpt_w(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，dot level4
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.celldot.mattime[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 794
static u8 mb_cfg_dotpr_w(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，dot level4
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.celldot.resth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 795
static u8 mb_cfg_cutp_w(uint16_t mbaddr,uint16_t *data)//充电高温保护，cut level4
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.cellcut.matth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 796
static u8 mb_cfg_cutpt_w(uint16_t mbaddr,uint16_t *data)//充电高温保护延时，cut level4
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.cellcut.mattime[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 797
static u8 mb_cfg_cutpr_w(uint16_t mbaddr,uint16_t *data)//充电高温保护恢复，cut level4
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.cellcut.resth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 798
static u8 mb_cfg_dutp_w(uint16_t mbaddr,uint16_t *data)//放电高温保护，dut level4
{
    (void)mbaddr;

    if(*data<=1600)
    {
        gConfigBuff.celldut.matth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 799
static u8 mb_cfg_dutpt_w(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，dut level4
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.celldut.mattime[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 800
static u8 mb_cfg_dutpr_w(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，dut level4
{
    (void)mbaddr;

    if(*data<=1600)
    {
        gConfigBuff.celldut.resth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 801
static u8 mb_cfg_mosotp_w(uint16_t mbaddr,uint16_t *data)//放电高温保护，mosot level4
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.pcbot.matth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 802
static u8 mb_cfg_mosotpt_w(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，mosot level4
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.pcbot.mattime[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 803
static u8 mb_cfg_mosotpr_w(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，mosot level4
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.pcbot.resth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 804
static u8 mb_cfg_conotp_w(uint16_t mbaddr,uint16_t *data)//放电高温保护，conot level4
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.connot.matth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 805
static u8 mb_cfg_conotpt_w(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，conot level4
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.connot.mattime[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 806
static u8 mb_cfg_conotpr_w(uint16_t mbaddr,uint16_t *data)//放电高温保护延时，conot level4
{
    (void)mbaddr;

    if(*data<=1600)
    {
        gConfigBuff.connot.resth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 807
static u8 mb_cfg_sovw_w(uint16_t mbaddr,uint16_t *data)//过压告警，ov level3
{
    (void)mbaddr;
    if(*data>=2000 && *data<=4500)
    {
        gConfigBuff.cellov.matth[2] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 808
static u8 mb_cfg_sovwt_w(uint16_t mbaddr,uint16_t *data)//过压告警延时，ov level3
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.cellov.mattime[2] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 809
static u8 mb_cfg_sovwr_w(uint16_t mbaddr,uint16_t *data)//过压告警恢复，ov level3
{
    (void)mbaddr;
    if(*data>=2000 && *data<=4500)
    {
        gConfigBuff.cellov.resth[2] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 810
static u8 mb_cfg_suvw_w(uint16_t mbaddr,uint16_t *data)//欠压告警，uv level2
{
    (void)mbaddr;
    if(*data>=2000 && *data<=4500)
    {
        gConfigBuff.celluv.matth[1] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 811
static u8 mb_cfg_suvwt_w(uint16_t mbaddr,uint16_t *data)//欠压告警延时，uv level2
{
    (void)mbaddr;

    if(*data>=1 && *data<=120)
    {
        gConfigBuff.celluv.mattime[1] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 812
static u8 mb_cfg_suvwr_w(uint16_t mbaddr,uint16_t *data)//欠压告警恢复，uv level2
{
    (void)mbaddr;
    if(*data>=2000 && *data<=4500)
    {
        gConfigBuff.celluv.resth[1] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 813
static u8 mb_cfg_cocw_w(uint16_t mbaddr,uint16_t *data)//充电过流告警，ccurr level3
{
    (void)mbaddr;
    if(*data>=1 && *data<=30000)
    {
        gConfigBuff.ccurr.matth[2] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 814
static u8 mb_cfg_cocwt_w(uint16_t mbaddr,uint16_t *data)//充电过流告警延时，ccurr level3
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.ccurr.mattime[2] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 815
static u8 mb_cfg_cocwr_w(uint16_t mbaddr,uint16_t *data)//充电过流告警恢复，ccurr level3
{
    (void)mbaddr;
    if(*data>=1 && *data<=30000)
    {
        gConfigBuff.ccurr.resth[2] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 816
static u8 mb_cfg_docw_w(uint16_t mbaddr,uint16_t *data)//放电过流告警，dcurr level1
{
    (void)mbaddr;
    if(*data>=1 && *data<=30000)
    {
        gConfigBuff.dcurr.matth[0] = 0 - (uint16_t)*data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 817
static u8 mb_cfg_docwt_w(uint16_t mbaddr,uint16_t *data)//放电过流告警延时，dcurr level1
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.dcurr.mattime[0] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 818
static u8 mb_cfg_docwr_w(uint16_t mbaddr,uint16_t *data)//放电过流告警恢复，dcurr level1
{
    (void)mbaddr;
    if(*data>=1 && *data<=30000)
    {
        gConfigBuff.dcurr.resth[0] = 0 - (uint16_t)*data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 819
static u8 mb_cfg_cotw_w(uint16_t mbaddr,uint16_t *data)//充电高温告警，cot level3
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.cellcot.matth[2] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 820
static u8 mb_cfg_cotwt_w(uint16_t mbaddr,uint16_t *data)//充电高温告警延时，cot level3
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.cellcot.mattime[2] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 821
static u8 mb_cfg_cotwr_w(uint16_t mbaddr,uint16_t *data)//充电高温告警恢复，cot level3
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.cellcot.resth[2] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 822
static u8 mb_cfg_dotw_w(uint16_t mbaddr,uint16_t *data)//放电高温告警，dot level3
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.celldot.matth[2] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 823
static u8 mb_cfg_dotwt_w(uint16_t mbaddr,uint16_t *data)//放电高温告警延时，dot level3
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.celldot.mattime[2] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 824
static u8 mb_cfg_dotwr_w(uint16_t mbaddr,uint16_t *data)//放电高温告警恢复，dot level3
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.celldot.resth[2] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 825
static u8 mb_cfg_cutw_w(uint16_t mbaddr,uint16_t *data)//充电高温告警，cut level3
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.cellcut.matth[2] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 826
static u8 mb_cfg_cutwt_w(uint16_t mbaddr,uint16_t *data)//充电高温告警延时，cut level3
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.cellcut.mattime[2] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 827
static u8 mb_cfg_cutwr_w(uint16_t mbaddr,uint16_t *data)//充电高温告警恢复，cut level3
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.cellcut.resth[2] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 828
static u8 mb_cfg_dutw_w(uint16_t mbaddr,uint16_t *data)//放电高温告警，dut level3
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.celldut.matth[2] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 829
static u8 mb_cfg_dutwt_w(uint16_t mbaddr,uint16_t *data)//放电高温告警延时，dut level3
{
    (void)mbaddr;
    if(*data>=1 && *data<=120)
    {
        gConfigBuff.celldut.mattime[2] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 830
static u8 mb_cfg_dutwr_w(uint16_t mbaddr,uint16_t *data)//放电高温告警恢复，dut level3
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.celldut.resth[2] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 831
static u8 mb_cfg_afecg_w(uint16_t mbaddr,uint16_t *data)//电流校准值
{
    (void)mbaddr;
    if(*data> 900 && *data < 1300)
    {
    	//电流校准
        gSHAFECfg.calres = *data;
        SH367309EventFlagSet(SH367309_EVE_CTRL_OSCD);	//设置真实过流断流电流值
        gConfigBuff.calpara.sampleres = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
        //afe_ctrl_set(AFE_CTRL_CAL);
    }
    return TRUE;
}

//写地址 832
static u8 mb_cfg_b16vg_w(uint16_t mbaddr,uint16_t *data)//B16电压校准值
{
    (void)mbaddr;
    //B16电压校准值
    if(*data>0 && *data<30000)
    {
        if(True == gModVoltTestCmd)
        {
            gConfig.calpara.b16gain = *data;
        }
    	//第16节电压校准,17串AFE无此校准
        //gConfigBuff.calpara.b16gain = *data;
        //gAFECfg.vcellgain = *data;
        //gConfigBuff.calpara.afe_b16_gain = *data;
        //gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    return TRUE;
}

//写地址 833
static u8 mb_cfg_cycles_w(uint16_t mbaddr,uint16_t *data)//
{
    (void)mbaddr;
    if((*data) < 10485)
    {
    	//写循环次数
    	/*
        g_FUEL_DFFS.cycles=*data;
        g_BQ27541_VAR.ctrl |= FUEL_CTRL_SET_CYCLES;
        */
		#ifdef BMS_USE_MAX172XX
        gMax17205Var.CycleCount = *data;
        Max17205SetCtrl(MAX1720X_WRITE_CYCLES);
        gMax17205Cal.wrtcycle = ((float)*data / 0.16);
		#endif
        
        #ifdef BMS_USE_SOX_MODEL
            #ifdef BMS_USE_SOX_MODEL_NEED_FUEL
            //gBatteryInfo.Status.DataReady &= ~0x02;
            //gTICycRefreshFlg = False;
            SetHisDchgCap((u32)*data * GetStandCap());
            #else
            SetHisDchgCap(*data * GetStandCap());
            #endif
        #endif
    }
    return TRUE;
}

//写地址 834
static u8 mb_cfg_tvsotp_w(uint16_t mbaddr,uint16_t *data)//TVS管高温保护，conot level4
{
    (void)mbaddr;

    if(*data<=1600)
    {
        gConfigBuff.tvsot.matth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }

    return TRUE;
}

//写地址 835
static u8 mb_cfg_tvsotpt_w(uint16_t mbaddr,uint16_t *data)//TVS管高温保护延时，conot level4
{
    (void)mbaddr;

    if(*data>=1 && *data<=120)
    {
        gConfigBuff.tvsot.mattime[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }

    return TRUE;
}

//写地址 836
static u8 mb_cfg_tvsotpr_w(uint16_t mbaddr,uint16_t *data)//TVS管高温保护恢复值，tvsot level4
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.tvsot.resth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }

    return TRUE;
}

//写地址 837
static u8 mb_cfg_fuseotp_w(uint16_t mbaddr,uint16_t *data)//fuse高温保护，conot level4
{
    (void)mbaddr;
    return true;
}

//写地址 838
static u8 mb_cfg_fuseotpt_w(uint16_t mbaddr,uint16_t *data)//fuse高温保护延时，conot level4
{
    (void)mbaddr;
    return true;
}

//写地址 839
static u8 mb_cfg_fuseotpr_w(uint16_t mbaddr,uint16_t *data)//fuse高温保护恢复值，tvsot level4
{
    (void)mbaddr;
    return true;
}

//写地址 840 大电流检测配置
static u8 mb_cfg_currdetcig_w(uint16_t mbaddr,uint16_t *data)	
{
    (void)mbaddr;
	
	gConfigBuff.configadd.currdet.highcurrdet = *data;
	gStorageCTRL |= FLASH_WR_CFG_MASK;
	
	return true;
}

////写地址 853 电量计分流器配置
//static u8 mb_cfg_grense_w(uint16_t mbaddr,uint16_t *data)
//{
//    (void)mbaddr;
//    #ifdef    BMS_USE_MAX172XX
//    if((*data)>=50 && (*data)<=200)
//    {
//        gMax17205Cal.rsense = *data;
//        gConfigBuff.calpara.fuelrense = gMax17205Cal.rsense;
//        gStorageCTRL |= FLASH_WR_CFG_MASK;
//    }
//    else
//    {
//        return FALSE;
//    }
//    return TRUE;
//    #else
//    return TRUE;
//    #endif
//    
//}

//写地址 853
static u8 mb_cfg_pretotp_w(uint16_t mbaddr,uint16_t *data)//预放电阻高温保护，conot level4
{
    (void)mbaddr;

    if(*data<=1600)
    {
        gConfigBuff.configadd.preotemp.matth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }

    return TRUE;
}

//写地址 854
static u8 mb_cfg_pretotpt_w(uint16_t mbaddr,uint16_t *data)//预放电阻高温保护延时，conot level4
{
    (void)mbaddr;

    if(*data>=1 && *data<=120)
    {
        gConfigBuff.configadd.preotemp.mattime[3] = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }

    return TRUE;
}

//写地址 855
static u8 mb_cfg_pretotpr_w(uint16_t mbaddr,uint16_t *data)//预放电阻高温保护恢复值，tvsot level4
{
    (void)mbaddr;
    if(*data<=1600)
    {
        gConfigBuff.configadd.preotemp.resth[3] = (int16_t)*data-400;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }

    return TRUE;
}

//写地址 862 电量计至少剩余的更新次数，避免一次用完
static u8 mb_cfg_fuel_atlearem_w(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;
    #ifdef    BMS_USE_MAX172XX
    if((*data) <= 6)
    {
        gMax17205Var.AtLesRem = *data;
        gConfigBuff.calpara.FuelAtLesRem = *data;
        gStorageCTRL |= FLASH_WR_CFG_MASK;
    }
    else
    {
        return FALSE;
    }
    return TRUE;
    #else
    return TRUE;
    #endif
}

//写地址 863 电量计电流采样零飘值
static u8 mb_cfg_fuel_coff_w(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    gConfigBuff.calpara.COff = *data;
    gStorageCTRL |= FLASH_WR_CFG_MASK;

    return TRUE;
}

//写地址 864 电量计电流采样斜率值
static u8 mb_cfg_fuel_cgain_w(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    gConfigBuff.calpara.CGain = *data;
    gStorageCTRL |= FLASH_WR_CFG_MASK;

    return TRUE;
}

//写地址 865 afe零飘电流
static u8 mb_cfg_afe_coff_w(uint16_t mbaddr,uint16_t *data)
{
    (void)mbaddr;

    gConfigBuff.calpara.sampleoff = *data;
    gSHAFECfg.Cadcoffset = *data;
    gStorageCTRL |= FLASH_WR_CFG_MASK;

    return TRUE;
}


/***************************** 用户数据区写函数集合 *****************************/

//写地址4096
static u8 mb_user_cmd_w(uint16_t mbaddr,uint16_t *data)
{
    #ifdef USER_MOD_HARD_VER_ENABLE
    if(USER_MOD_HARD_VER == (gUserPermitCmd & USER_MOD_HARD_VER))
    {
        #define CRC32_CHECK_CODE       0xa55aa55a
        u8 FlashRead[2048] = {0};
        t_VERSION_INFO *appinfo = NULL;
        uint32_t crc;
        
        //更改硬件版本号
        BSPFlashReadByte(2048,FlashRead,2048);
        BSPFLASHEraseSector(1);
        FlashRead[0xffc - 2048] = (u8)((*data >> 8) & 0x00ff);
        FlashRead[0xffd - 2048] = (u8)((*data) & 0x00ff);
        BSPFlashWriteByte(2048,FlashRead,2048);
        
        //更改APP INFO
        BSPFlashReadByte(0x7800,FlashRead,2048);
        BSPFLASHEraseSector(15);
        appinfo = (t_VERSION_INFO *)&FlashRead[0x7F80 - 0x7800];
        appinfo->hwmain = (u8)((*data >> 8) & 0x00ff);
        appinfo->hwsub = (u8)((*data) & 0x00ff);
        crc = Crc32_Calc(CRC32_CHECK_CODE,(uint8_t*)appinfo,128-4);
        appinfo->info_crc = crc;
        BSPFlashWriteByte(0x7800,FlashRead,2048);
        
        gUserPermitCmd = 0;
        SoftReset();
    }
    #endif
    
    #ifdef BMS_USE_SOX_MODEL
    if(gUserPermitCmd & USER_MODIFY_SOC)
    {
        if(*data <= 1000)
        {
            extern void SocSetIsSlideShow(u8 IsSlideShow);
            CorrNowCapBySoc(*data);      //修正当前SOC为100%
            SocSetIsSlideShow(False);   //不平滑显示了，直接显示真实SOC               
        }

    }
    #endif
    else if(gUserPermitCmd & UD_MODTEMP_EN)
	{
		if(((int16_t)*data) <= 1200)
		{
			g_AFE_Test_VAR.ModTempVal = ((int16_t)*data);
		}
	}
    
    if(*data == 0x1235)//清除lifetime数据
    {
        gUserIDBuff.lt = cUserIDInit.lt;
        gStorageCTRL |= FLASH_WR_UD_MASK;
    }
    else if(*data == 0x2468)//清除事件记录
    {
        gStorageCTRL |= FLASH_CLR_EVE_MASK;
    }
    else if(*data == 0x9C15) //用户数据命令使能
    {
        gUserPermitCmd |= USER_PERMIT_CMD_EN;
    }
    else if(*data == 0x468B) //恢复正式环境
    {
        gUserPermitCmd &= ~USER_TESTENVIR_CMD_EN;
    }
    else if(*data == 0x468A) //切换测试环境
    {
        gUserPermitCmd |= USER_TESTENVIR_CMD_EN;
    }    
    else if(*data == 0x9988) //复位电量计DF
    {
        #ifdef BMS_USE_SOX_MODEL
        gUserPermitCmd |= USER_CLEAR_SOX_DATA;
        #endif
    }
    #ifdef BMS_USE_SOX_MODEL
    else if(*data == 0x9977) //复位SOX算法
    {
        #ifdef BMS_USE_SOX_MODEL
//        gUserPermitCmd |= USER_CLEAR_SOX_DATA;
        gStorageCTRL |= FLASH_SOC_CLR_MASK;
        #endif
    }
  	else if(*data == 0x9966)
	{
		gUserPermitCmd |= USER_MODIFY_SOC;
	} 
  	else if(*data == 0x9955)
	{
		gUserPermitCmd &= ~USER_MODIFY_SOC;
	}    
    #endif
  	else if(*data == 0x9944)
	{
		g_AFE_Test_VAR.IsModTemp = True;
		gUserPermitCmd |= UD_MODTEMP_EN;
	} 
  	else if(*data == 0x9933)
	{
		g_AFE_Test_VAR.IsModTemp = False;
		gUserPermitCmd &= ~UD_MODTEMP_EN;
	}	    
    #ifdef USER_MOD_HARD_VER_ENABLE
    else if(*data == 0x8877) //可以进行硬件版本号更改
    {
        gUserPermitCmd |= USER_MOD_HARD_VER;
    } 
    #endif
    
    #ifdef BMS_ACC_ENABLE
    else if(*data == 0x7766) //进行加速度零飘校准
    {
        gUserPermitCmd |= USER_CALI_ACC_OFFSET;
    } 
    #endif    
    else if(*data == 0x625b) //更改第二节电压测试
    {
        gModVoltTestCmd = True;
    }    
    else if(*data == 0x625c) //取消第二节电压测试
    {
        gModVoltTestCmd = False;
    }
    else if(*data == 0x7431)
    {
        //清除认证标志
        certifi_tcb.certifi_cmd_ack = 1;//true
        certifi_tcb.IsCertifi = 1;
    }         
    else if(*data != 0)		//用户数据命令失能
    {
        //gUserIDBuff = gUserID;
        gUserPermitCmd &= ~USER_PERMIT_CMD_EN;
    }
    return TRUE;
}

//写地址4097 - 4100
static u8 mb_user_data_w(uint16_t mbaddr,uint16_t *data)
{
    mbaddr -=4097;
    *(((uint16_t*)&gUserIDBuff.user)+mbaddr) = *data;
    gStorageCTRL |= FLASH_WR_UD_MASK;
    return TRUE;
}


/********************************************************* 读函数集合 *********************************************************/

/***************************** BMS设备信息读函数集合 *****************************/

t_MBREG const cMBDevInfoRdFunc[MB_DEVICE_INFO_NUM] =
{
    mb_info_prver_r,	//读地址0
    mb_info_hwver_r,	//读地址1
    mb_info_blver_r,	//读地址2
    mb_info_fwmsv_r,	//读地址3
    mb_info_fwrev_r,	//读地址4
    mb_info_fwbnh_r,	//读地址5
    mb_info_fwbnl_r,	//读地址6
    mb_info_mcun_r_l,	//读地址7
    mb_info_mcun_r_l,	//读地址8
    mb_info_mcun_r_h,	//读地址9
    mb_info_mcun_r_h,	//读地址10
    mb_info_btype_r,	//读地址11
    mb_info_bvolt_r,	//读地址12
    mb_info_bcap_r,		//读地址13
    mb_info_sn_r,		//读地址14
    mb_info_sn_r,		//读地址15
    mb_info_sn_r,		//读地址16
    mb_info_sn_r,		//读地址17
    mb_info_erech_r,	//读地址18
    mb_info_erecl_r,	//读地址19
    mb_info_lrech_r,	//读地址20
    mb_info_lrecl_r,	//读地址21
    mb_info_lifetime_r,	//读地址22
    mb_info_lifetime_r,	//读地址23
    mb_info_lifetime_r,	//读地址24
    mb_info_lifetime_r,	//读地址25
    mb_info_lifetime_r,	//读地址26
    mb_info_lifetime_r,	//读地址27
    mb_info_lifetime_r,	//读地址28
    mb_info_lifetime_r,	//读地址29
    mb_info_lifetime_r,	//读地址30
    mb_info_lifetime_r,	//读地址31
    mb_info_lifetime_r,	//读地址32
    mb_info_df_ver,		//读地址33
    mb_info_bat_num,    //读地址34
    mb_info_bat_type,   //读地址35
    /* 设备信息预留开始 */  
    //读地址36 - 49
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,       
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    /* 设备信息预留结束 */  
    mb_info_fuel_updatenum,		//读地址50
    
    #ifdef BMS_USE_SOX_MODEL
    mb_info_sox_dfver_r,   //读地址51
    #else
    mb_readnull_r,   //读地址51
    #endif
};

/***************************** BMS只读数据读函数集合 *****************************/

t_MBREG const cMBReadonlyDataRdFunc[MB_READONLY_DATA_NUM] =
{
    mb_data_user_r,		//读地址256
    mb_data_user_r,		//读地址257
    mb_data_user_r,		//读地址258
    mb_data_user_r,		//读地址259
	mb_data_state_r,	//读地址260
    mb_data_soc_r,		//读地址261
    mb_data_totlev_r,	//读地址262
    mb_data_curr_r,		//读地址263
    mb_data_maxt_r,		//读地址264
    mb_data_maxtn_r,	//读地址265
    mb_data_mint_r,		//读地址266
    mb_data_mintn_r,	//读地址267
    mb_data_maxv_r,		//读地址268
    mb_data_maxvn_r,	//读地址269
    mb_data_minv_r,		//读地址270
    mb_data_minvn_r,	//读地址271
    mb_data_dsgc_r,		//读地址272
    mb_data_chgc_r,		//读地址273
    mb_data_soh_r,		//读地址274
    mb_data_cyc_r,		//读地址275
    mb_data_rcap_r,		//读地址276
    mb_data_fcap_r,		//读地址277
    mb_data_fctime_r,	//读地址278
    mb_data_repow_r,	//读地址279
    mb_data_wnft_r,		//读地址280
    mb_data_wnft_r,		//读地址281
    mb_data_wnft_r,		//读地址282
    mb_data_wnft_r,		//读地址283
	mb_data_wnalarm_r,	//读地址284
	mb_data_wnalarm_r,	//读地址285
    mb_data_mostemp_r,	//读地址286
    mb_data_mostemp_r,	//读地址287
    mb_data_pretemp_r,	//读地址288
    mb_data_contemp_r,	//读地址289
    mb_data_battemp_r,	//读地址290
    mb_data_battemp_r,	//读地址291
    mb_data_batvolt_r,	//读地址292
    mb_data_batvolt_r,	//读地址293
    mb_data_batvolt_r,	//读地址294
    mb_data_batvolt_r,	//读地址295
    mb_data_batvolt_r,	//读地址296
    mb_data_batvolt_r,	//读地址297
    mb_data_batvolt_r,	//读地址298
    mb_data_batvolt_r,	//读地址299
    mb_data_batvolt_r,	//读地址300
    mb_data_batvolt_r,	//读地址301
    mb_data_batvolt_r,	//读地址302
    mb_data_batvolt_r,	//读地址303
    mb_data_batvolt_r,	//读地址304
    mb_data_batvolt_r,	//读地址305
    mb_data_batvolt_r,	//读地址306
    mb_data_batvolt_r,	//读地址307
    mb_data_bala_r,		//读地址308
    mb_data_accx_r,		//读地址309
    mb_data_accy_r,		//读地址310
    mb_data_accz_r,		//读地址311
    mb_data_mcu3v3_r,	//读地址312
    mb_data_prevolt_r,	//读地址313
    mb_data_fuelcurr_r,	//读地址314
    mb_data_chgvolt_r,	//读地址315
    mb_data_fuelvolt_r,	//读地址316
    mb_data_tvstemp_r,	//读地址317
    mb_data_tempspeed_r,//读地址318
    mb_data_tempspeed_r,//读地址319
    mb_data_tempspeed_r,//读地址320
    mb_data_minvs_r,	//读地址321
    mb_data_minvsn_r,	//读地址322
    mb_data_max_chg_c_recent,//读地址323
    mb_data_max_dsg_c_recent,//读地址324
    mb_data_fusetemp_r,      //读地址325
    mb_data_currdet_r,       //读地址326    
    /* BMS只读数据预留开始 */  
    //读地址327 - 338
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,       
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,
    
    mb_data_batvolt17_20_r,
    mb_data_batvolt17_20_r,
    mb_data_batvolt17_20_r,
    mb_data_batvolt17_20_r,
    mb_data_bala17_20_r,

    #ifdef BMS_USE_SOX_MODEL
    /*SOX算法*/
//    mb_data_sox_soc_r,      //读地址
//    mb_data_sox_soh_r,      //读地址
//    mb_data_sox_cyc_r,      //读地址
//    mb_data_sox_rcap_r,     //读地址
//    mb_data_sox_fcap_r,     //读地址
    mb_data_HisChgCapH_r,    //读地址344
    mb_data_HisChgCapL_r,    //读地址345
    mb_data_HisDchgCapH_r,   //读地址346
    mb_data_HisDchgCapL_r,   //读地址347
    #else
//    mb_readnull_r,  //读地址  
//    mb_readnull_r,  //读地址
//    mb_readnull_r,  //读地址  
//    mb_readnull_r,  //读地址
//    mb_readnull_r,  //读地址 
    mb_readnull_r,  //读地址352
    mb_readnull_r,  //读地址353    
    mb_readnull_r,  //读地址354
    mb_readnull_r,  //读地址355      
    #endif
    
    /* BMS只读数据预留结束 */     
    #ifdef CANBUS_MODE_JT808_ENABLE	
    mb_data_csq_r,          //读地址348
    mb_data_firstfixtime_r, //读地址349
    mb_data_gpssignal_r,    //读地址350
    mb_data_sateinview_r,   //读地址351
    #else
    mb_readnull_r,          //读地址339
    mb_readnull_r,          //读地址340
    mb_readnull_r,          //读地址341
    mb_readnull_r,          //读地址342
    #endif

    #ifdef CANBUS_MODE_JT808_ENABLE	
    mb_data_ExModuleFlag_r, //读地址352
    mb_data_Longitude_H_r,  //读地址353
    mb_data_Longitude_L_r,  //读地址354
    mb_data_Latitude_H_r,   //读地址355
    mb_data_Latitude_L_r,   //读地址356
    mb_data_GPRSConnectTime_r,//读地址357
    mb_data_GPRSTranMaxTime_r,//读地址358
    mb_data_GPRSTranMinTime_r, //读地址359
    #else
    mb_readnull_r,          //读地址356
    mb_readnull_r,          //读地址357
    mb_readnull_r,          //读地址358
    mb_readnull_r,          //读地址359
    mb_readnull_r,          //读地址360  
    mb_readnull_r,          //读地址361
    mb_readnull_r,          //读地址362
    mb_readnull_r,          //读地址363     
    #endif    
    
//    #ifdef BMS_ENABLE_LITTLE_CURRENT_DET
//    mb_data_LowCurrP1BaseVal_r,//读地址364
//    mb_data_LowCurrP2BaseVal_r,//读地址365
//    mb_data_LowCurrP1Curr_r,   //读地址366
//    mb_data_LowCurrP2Curr_r,   //读地址367
//    #else
//    mb_readnull_r,          //读地址364 
//    mb_readnull_r,          //读地址365
//    mb_readnull_r,          //读地址366
//    mb_readnull_r,          //读地址367        
//    #endif
    
    mb_data_PreDsgTemp_r,   //读地址360
    mb_data_CalibCyc_r,     //读地址361
};

/***************************** BMS运行控制读函数集合 *****************************/

t_MBREG const cMBRunCTRLRdFunc[MB_RUN_CTRL_NUM] =
{
	mb_ctrl_r,			//读地址512
	mb_ctrl_r,			//读地址513
	mb_ctrl_r,			//读地址514
	mb_ctrl_r,			//读地址515
	mb_ctrl_r,			//读地址516
	mb_ctrl_r,			//读地址517
	mb_ctrl_r,			//读地址518
	mb_ctrl_r,			//读地址519
	mb_ctrl_r,			//读地址520
	mb_ctrl_r,			//读地址521
	mb_ctrl_r,			//读地址522
	mb_ctrl_r,			//读地址523
	mb_ctrl_r,			//读地址524
	mb_ctrl_r,			//读地址525
	mb_ctrl_r,			//读地址526
	mb_ctrl_r,			//读地址527
	mb_ctrl_r,			//读地址528
	mb_ctrl_afeccal_r,	//读地址529
	mb_ctrl_b16vcal_r,	//读地址530
	mb_ctrl_rtc_r,		//读地址531
	mb_ctrl_rtc_r,		//读地址532
	mb_ctrl_rtc_r,		//读地址533
	mb_ctrl_rst_r,		//读地址534
	mb_ctrl_rec_r,		//读地址535
	mb_ctrl_balctl_r,	//读地址536
	mb_ctrl_tctrl_r,	//读地址537
	mb_ctrl_tres_r,		//读地址538
    /* BMS运行控制预留开始 */  
    //读地址539 - 554
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,       
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    /* BMS运行控制预留结束 */  
	#if defined(USE_B20_TOPBAND_PCBA) || defined(USE_B22_IM_PCBA)
    mb_ctrl_balctl_17_20_r,//读地址555
	#elif defined(USE_B21_IM_PCBA)
	#else
	#endif
};

/***************************** BMS参数配置读函数集合 *****************************/

t_MBREG const cMBParaCfgRdFunc[MB_PARA_CFG_NUM] =
{
	mb_cfg_nvmcmd_r,	//读地址768
	mb_cfg_sn_r,		//读地址769
	mb_cfg_sn_r,		//读地址770
	mb_cfg_sn_r,		//读地址771
	mb_cfg_sn_r,		//读地址772
	mb_cfg_sovp_r,		//读地址773
	mb_cfg_sovpr_r,		//读地址774
    mb_cfg_suvp1_r,		//读地址775
    mb_cfg_suvpt1_r,	//读地址776
    mb_cfg_suvpr1_r,	//读地址777
    mb_cfg_suvpsoc1_r,	//读地址778
    mb_cfg_suvpsoct1_r,	//读地址779
    mb_cfg_suvpsocr1_r,	//读地址780
    mb_cfg_suvp2_r,		//读地址781
    mb_cfg_suvpr2_r,	//读地址782
    mb_cfg_cocp_r,		//读地址783
    mb_cfg_cocpt_r,		//读地址784
    mb_cfg_docp1_r,		//读地址785
    mb_cfg_docpt1_r,	//读地址786
    mb_cfg_docp2_r,		//读地址787
    mb_cfg_dscp_r,		//读地址788
    mb_cfg_cotp_r,		//读地址789
    mb_cfg_cotpt_r,		//读地址790
    mb_cfg_cotpr_r,		//读地址791
    mb_cfg_dotp_r,		//读地址792
    mb_cfg_dotpt_r,		//读地址793
    mb_cfg_dotpr_r,		//读地址794
    mb_cfg_cutp_r,		//读地址795
    mb_cfg_cutpt_r,		//读地址796
    mb_cfg_cutpr_r,		//读地址797
    mb_cfg_dutp_r,		//读地址798
    mb_cfg_dutpt_r,		//读地址799
    mb_cfg_dutpr_r,		//读地址800
    mb_cfg_mosotp_r,	//读地址801
    mb_cfg_mosotpt_r,	//读地址802
    mb_cfg_mosotpr_r,	//读地址803
    mb_cfg_conotp_r,	//读地址804
    mb_cfg_conotpt_r,	//读地址805
    mb_cfg_conotpr_r,	//读地址806
    mb_cfg_sovw_r,		//读地址807
    mb_cfg_sovwt_r,		//读地址808
    mb_cfg_sovwr_r,		//读地址809
    mb_cfg_suvw_r,		//读地址810
    mb_cfg_suvwt_r,		//读地址811
    mb_cfg_suvwr_r,		//读地址812
    mb_cfg_cocw_r,		//读地址813
    mb_cfg_cocwt_r,		//读地址814
    mb_cfg_cocwr_r,		//读地址815
    mb_cfg_docw_r,		//读地址816
    mb_cfg_docwt_r,		//读地址817
    mb_cfg_docwr_r,		//读地址818
    mb_cfg_cotw_r,		//读地址819
    mb_cfg_cotwt_r,		//读地址820
    mb_cfg_cotwr_r,		//读地址821
    mb_cfg_dotw_r,		//读地址822
    mb_cfg_dotwt_r,		//读地址823
    mb_cfg_dotwr_r,		//读地址824
    mb_cfg_cutw_r,		//读地址825
    mb_cfg_cutwt_r,		//读地址826
    mb_cfg_cutwr_r,		//读地址827
    mb_cfg_dutw_r,		//读地址828
    mb_cfg_dutwt_r,		//读地址829
    mb_cfg_dutwr_r,		//读地址830
    mb_cfg_afecg_r,		//读地址831
    mb_cfg_b16vg_r,		//读地址832
    mb_data_cyc_r,		//读地址833
    mb_cfg_tvsotp_r,	//读地址834
    mb_cfg_tvsotpt_r,	//读地址835
    mb_cfg_tvsotpr_r,	//读地址836
    mb_cfg_fuseotp_r,	//读地址837
    mb_cfg_fuseotpt_r,	//读地址838
    mb_cfg_fuseotpr_r,	//读地址839
    mb_cfg_currdetcig_r,//读地址840
    /* BMS参数配置预留开始 */  
    //读地址841 - 852    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,       
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    /* BMS参数配置预留结束 */     
	mb_cfg_pretotp_r,	  //读地址853
    mb_cfg_pretotpt_r,	  //读地址854
    mb_cfg_pretotpr_r,	  //读地址855   
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r, 
    mb_cfg_fuel_atlearem_r, //读地址862
    mb_cfg_fuel_coff_r,     //读地址863
    mb_cfg_fuel_cgain_r,    //读地址864
    mb_cfg_afe_coff_r       //读地址865
};

/***************************** 用户数据区读函数集合 *****************************/

t_MBREG const cMBUserDataRdFunc[MB_USER_DATA_NUM] =
{
	mb_user_cmd_r,		//读地址4096
	mb_user_data_r,		//读地址4097
	mb_user_data_r,		//读地址4098
	mb_user_data_r,		//读地址4099
	mb_user_data_r,		//读地址4100
    /* 用户数据区预留开始 */  
    //读地址4101 - 4116
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,       
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    mb_readnull_r,
    mb_readnull_r,    
    /* 用户数据区预留结束 */       
};


/********************************************************* 写函数集合 *********************************************************/

/***************************** BMS运行控制写函数集合 *****************************/

t_MBREG const cMBRunCTRLWrFunc[MB_RUN_CTRL_NUM] =
{
    mb_ctrl_ctrl_w,		//写地址512
    mb_ctrl_w,			//写地址513
    mb_ctrl_w,			//写地址514
    mb_ctrl_w,			//写地址515
    mb_ctrl_w,			//写地址516
    mb_ctrl_w,			//写地址517
    mb_ctrl_w,			//写地址518
    mb_ctrl_w,			//写地址519
    mb_ctrl_w,			//写地址520
    mb_ctrl_w,			//写地址521
    mb_ctrl_w,			//写地址522
    mb_ctrl_w,			//写地址523
    mb_ctrl_w,			//写地址524
    mb_ctrl_w,			//写地址525
    mb_ctrl_w,			//写地址526
    mb_ctrl_w,			//写地址527
    mb_ctrl_w,			//写地址528
    mb_ctrl_afeccal_w,	//写地址529
    mb_ctrl_b16vcal_w,	//写地址530
    mb_ctrl_w,			//写地址531
    mb_ctrl_w,			//写地址532
    mb_ctrl_mnsc_w,		//写地址533
    mb_ctrl_rst_w,		//写地址534
    mb_ctrl_rec_w,		//写地址535
    mb_ctrl_balctl_w,	//写地址536
    mb_ctrl_tctrl_w,	//写地址537
    mb_ctrl_tres_w,		//写地址538
    
    /* BMS运行控制预留开始 */  
    //写地址539 - 554
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,       
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    /* BMS运行控制预留结束 */   
	#if defined(USE_B20_TOPBAND_PCBA) || defined(USE_B22_IM_PCBA)
    mb_ctrl_balctl_17_20_w,	//写地址555	
	#elif defined(USE_B21_IM_PCBA)
	#else
	#endif
};

/***************************** BMS参数配置写函数集合 *****************************/
t_MBREG const cMBParaCfgWrFunc[MB_PARA_CFG_NUM] =
{
	mb_cfg_nvmcmd_w,	//写地址768
	mb_cfg_sn_w,		//写地址769
	mb_cfg_sn_w,		//写地址770
	mb_cfg_sn_w,		//写地址771
	mb_cfg_sn_w,		//写地址772
	mb_cfg_sovp_w,		//写地址773
	mb_cfg_sovpr_w,		//写地址774
	mb_cfg_suvp1_w,		//写地址775
	mb_cfg_suvpt1_w,	//写地址776
	mb_cfg_suvpr1_w,	//写地址777
	mb_cfg_suvpsoc1_w,	//写地址778
	mb_cfg_suvpsoct1_w,	//写地址779
	mb_cfg_suvpsocr1_w,	//写地址780
	mb_cfg_suvp2_w,		//写地址781
	mb_cfg_suvpr2_w,	//写地址782
	mb_cfg_cocp_w,		//写地址783
	mb_cfg_cocpt_w,		//写地址784
	mb_cfg_docp1_w,		//写地址785
	mb_cfg_docpt1_w,	//写地址786
	mb_cfg_docp2_w,		//写地址787
	mb_cfg_dscp_w,		//写地址788
	mb_cfg_cotp_w,		//写地址789
	mb_cfg_cotpt_w,		//写地址790
	mb_cfg_cotpr_w,		//写地址791
	mb_cfg_dotp_w,		//写地址792
	mb_cfg_dotpt_w,		//写地址793
	mb_cfg_dotpr_w,		//写地址794
	mb_cfg_cutp_w,		//写地址795
	mb_cfg_cutpt_w,		//写地址796
	mb_cfg_cutpr_w,		//写地址797
	mb_cfg_dutp_w,		//写地址798
	mb_cfg_dutpt_w,		//写地址799
	mb_cfg_dutpr_w,		//写地址800
	mb_cfg_mosotp_w,	//写地址801
	mb_cfg_mosotpt_w,	//写地址802
	mb_cfg_mosotpr_w,	//写地址803
	mb_cfg_conotp_w,	//写地址804
	mb_cfg_conotpt_w,	//写地址805
	mb_cfg_conotpr_w,	//写地址806
	mb_cfg_sovw_w,		//写地址807
	mb_cfg_sovwt_w,		//写地址808
	mb_cfg_sovwr_w,		//写地址809
	mb_cfg_suvw_w,		//写地址810
	mb_cfg_suvwt_w,		//写地址811
	mb_cfg_suvwr_w,		//写地址812
	mb_cfg_cocw_w,		//写地址813
	mb_cfg_cocwt_w,		//写地址814
	mb_cfg_cocwr_w,		//写地址815
	mb_cfg_docw_w,		//写地址816
	mb_cfg_docwt_w,		//写地址817
	mb_cfg_docwr_w,		//写地址818
	mb_cfg_cotw_w,		//写地址819
	mb_cfg_cotwt_w,		//写地址820
	mb_cfg_cotwr_w,		//写地址821
	mb_cfg_dotw_w,		//写地址822
	mb_cfg_dotwt_w,		//写地址823
	mb_cfg_dotwr_w,		//写地址824
	mb_cfg_cutw_w,		//写地址825
	mb_cfg_cutwt_w,		//写地址826
	mb_cfg_cutwr_w,		//写地址827
	mb_cfg_dutw_w,		//写地址828
	mb_cfg_dutwt_w,		//写地址829
	mb_cfg_dutwr_w,		//写地址830
	mb_cfg_afecg_w,		//写地址831
	mb_cfg_b16vg_w,		//写地址832
	mb_cfg_cycles_w,	//写地址833
	mb_cfg_tvsotp_w,	//写地址834
	mb_cfg_tvsotpt_w,	//写地址835
	mb_cfg_tvsotpr_w,	//写地址836
    mb_cfg_fuseotp_w,	//写地址837
    mb_cfg_fuseotpt_w,	//写地址838
    mb_cfg_fuseotpr_w,	//写地址839
    mb_cfg_currdetcig_w,//写地址840
    /* BMS参数配置预留开始 */  
    //写地址841 - 852    
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,       
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    /* BMS参数配置预留结束 */       
    
	mb_cfg_pretotp_w,	//写地址853
    mb_cfg_pretotpt_w,	//写地址854
    mb_cfg_pretotpr_w,	//写地址855   
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    mb_cfg_fuel_atlearem_w,//写地址862
    mb_cfg_fuel_coff_w,    //写地址863
    mb_cfg_fuel_cgain_w,   //写地址864    
    mb_cfg_afe_coff_w      //写地址865       
};

/***************************** 用户数据区写函数集合 *****************************/
t_MBREG const cMBUserDataWrFunc[MB_USER_DATA_NUM] =
{
	mb_user_cmd_w,		//写地址4096
	mb_user_data_w,		//写地址4097
	mb_user_data_w,		//写地址4098
	mb_user_data_w,		//写地址4099
	mb_user_data_w,		//写地址4100
    /* 用户数据区预留开始 */  
    //写地址4101 - 4116
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,       
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    mb_writenull_w,
    mb_writenull_w,    
    /* 用户数据区预留结束 */        
};

/*****************************************end of MBFindMap.c*****************************************/
