//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: DTCheck.c
//创建人  	: Handry
//创建日期	: 
//描述	    : 故障诊断检测源文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "DTCheck.h"
#include "DTTimer.h"
#include "BSPSystemCfg.h"
#include "BSPTask.h"
#include "BatteryInfo.h"
#include "ParaCfg.h"
#include "SH367309.h"
#include "MAX17205.h"
#include "RTC.h"		//用于替代PCF85063.h
//#include "PCF85063.h"
#include "Storage.h"
#include "MX25L16.h"
#include "MOSCtrl.h"
#include "Sample.h"
#include "NTC.h"
#include "CommCtrl.h"
#include "DataDeal.h"
#include "SleepMode.h"
#include "TestMode.h"
#include "DA213.h"


//=============================================================================================
//定义全局变量
//=============================================================================================
u8 gChgOVFlag = 0;							//充电高压标志
u8 gDchgUV1Flag = 0;						//放电低压1级标志
u8 gDchgUV2Flag = 0;						//放电低压2级标志

u8 gOCLock = 0;								//bit0 : 充电过流锁定；bit1 : 放电过流锁定
u16 gOCLockTimer[2][3] = {{0},{0}};	        //过流锁定定时器
u8 gOCCnt[2] = {0,0};					    //过流锁定计数器
u8 gOCLockLastLevel[2] = {0};			    //上一次的过流等级


//故障检测值
const t_DTCheck gDTConfig[DT_CHECK_CFG_LEN] =
{
	/*故障id，故障使能，故障等级，成熟阈值，恢复阈值，成熟时间，恢复时间，检测类型，检测值获取函数*/

	/* AFE故障 */
	{DT_ID_AFE_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.afeflt.mattime, &gConfig.afeflt.restime, DT_Bool, DTCheckGetAFEFault},
	/* 电量计故障 */
	{DT_ID_FUEL_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.fuelflt.mattime, &gConfig.fuelflt.restime, DT_Bool, DTCheckGetFuelFault},
	/* RTC故障 */
	{DT_ID_RTC_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.rtcflt.mattime, &gConfig.rtcflt.restime, DT_Bool, DTCheckGetRTCFault},
	/* FLASH故障 */
	{DT_ID_STOR_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.flashflt.mattime, &gConfig.flashflt.restime, DT_Bool, DTCheckGetStorageFault},
	/* 放电MOS失效故障 */
	{DT_ID_DIS_MOS_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.dmosflt.mattime, &gConfig.dmosflt.restime, DT_Bool, DTCheckGetDchgMosFault},
	/* 充电MOS失效故障 */
	{DT_ID_CHG_MOS_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.cmosflt.mattime, &gConfig.cmosflt.restime, DT_Bool, DTCheckGetChgMosFault},
	/* 预启动电路故障 */
	{DT_ID_PRE_CHG_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.preflt.mattime, &gConfig.preflt.restime, DT_Bool, DTCheckGetPreMosFault},
	/* 时钟未校准故障 */
	{DT_ID_RTC_NOCALIB_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.rtccalflt.mattime, &gConfig.rtccalflt.restime, DT_Bool, DTCheckGetRTCValFault},
	/* 参数未校准故障 */
	{DT_ID_NOPARA_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.paraflt.mattime, &gConfig.paraflt.restime, DT_Bool, DTCheckGetParaCfgFault},
	/* 保险丝失效故障 */
	{DT_ID_FUSE_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.fuseflt.mattime, &gConfig.fuseflt.restime, DT_Bool, DTCheckGetFuseFault},
	/* 加速度计故障 */
	{DT_ID_ACCELE_FAULT,DT_Disable,DT_LV1, NULL, NULL, &gConfig.accflt.mattime, &gConfig.accflt.restime, DT_Bool, DTCheckGetAccFault},
	/* 其他设备故障  不使能，预留 */
	{DT_ID_OTHERS_FAULT,DT_Disable,DT_LV1, NULL, NULL, &gConfig.othflt.mattime, &gConfig.othflt.restime, DT_Bool, NULL},


	/* 充电MOS NTC失效故障 */
	{DT_ID_CHG_NTC_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.cntcflt.mattime, &gConfig.cntcflt.restime, DT_Bool, DTCheckGetMosNTCFault},
	/* 放电MOS NTC失效故障 */
	{DT_ID_DIS_NTC_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.dntcflt.mattime, &gConfig.dntcflt.restime, DT_Bool, DTCheckGetMosNTCFault},
	/* 电量计NTC失效故障 */
	{DT_ID_FULE_NTC_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.fuelntcflt.mattime, &gConfig.fuelntcflt.restime, DT_Bool, DTCheckGetFuelNTCFault},
	/* 连接器NTC失效故障 */
	{DT_ID_CONN_NTC_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.conntcflt.mattime, &gConfig.conntcflt.restime, DT_Bool, DTCheckGetConnNTCFault},
	/* TVS NTC失效故障 */
	{DT_ID_TVS_NTC_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.tvsntcflt.mattime, &gConfig.tvsntcflt.restime, DT_Bool, DTCheckGetTVSNTCFault},
	/* 电池 NTC1失效故障 */
	{DT_ID_BAT_NTC1_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.b1ntcflt.mattime, &gConfig.b1ntcflt.restime, DT_Bool, DTCheckGetBat1NTCFault},
	/* 电池 NTC2失效故障 */
	{DT_ID_BAT_NTC2_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.b2ntcflt.mattime, &gConfig.b2ntcflt.restime, DT_Bool, DTCheckGetBat2NTCFault},

	/* 外置模块短路故障 */
	{DT_ID_EX_MODULE_FAULT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.exmodflt.mattime, &gConfig.exmodflt.restime, DT_Bool, DTCheckGetExModuleFault},

	/* 电池过压 */
    {DT_ID_VH,       DT_Enable,DT_LV4, &gConfig.cellov.matth[DT_LV4], &gConfig.cellov.resth[DT_LV4],&gConfig.cellov.mattime[DT_LV4], &gConfig.cellov.restime[DT_LV4], DT_Over, DTCheckGetMaxCellVolt},
	/* 电池一级欠压 */
	{DT_ID_VL_LV1,   DT_Enable,DT_LV3, &gConfig.celluv.matth[DT_LV3], &gConfig.celluv.resth[DT_LV3],&gConfig.celluv.mattime[DT_LV3], &gConfig.celluv.restime[DT_LV3], DT_Under, DTCheckGetMinCellVolt},
	/* 电池二级欠压 */
	{DT_ID_VL_LV2,   DT_Enable,DT_LV4, &gConfig.celluv.matth[DT_LV4], &gConfig.celluv.resth[DT_LV4],&gConfig.celluv.mattime[DT_LV4], &gConfig.celluv.restime[DT_LV4], DT_Under, DTCheckGetMinCellVolt},
	/* SOC过低 */
	{DT_ID_USOC_LV1,  DT_Enable,DT_LV4, &gConfig.usoc.matth[DT_LV4], &gConfig.usoc.resth[DT_LV4],&gConfig.usoc.mattime[DT_LV4], &gConfig.usoc.restime[DT_LV4], DT_Under, DTCheckGetSOC},
	/* 充电过流1级 */
	{DT_ID_CHG_CURH_LV1,  DT_Enable,DT_LV2, &gConfigChgCurr.matth[DT_LV2], &gConfigChgCurr.resth[DT_LV2],&gConfig.ccurr.mattime[DT_LV2], &gConfig.ccurr.restime[DT_LV2], DT_Over, DTCheckGetCurrent},
	/* 充电过流2级 */
	{DT_ID_CHG_CURH_LV2,  DT_Enable,DT_LV3, &gConfigChgCurr.matth[DT_LV3], &gConfigChgCurr.resth[DT_LV3],&gConfig.ccurr.mattime[DT_LV3], &gConfig.ccurr.restime[DT_LV3], DT_Over, DTCheckGetCurrent},
	/* 充电过流3级 */
	{DT_ID_CHG_CURH_LV3,  DT_Enable,DT_LV4, &gConfigChgCurr.matth[DT_LV4], &gConfigChgCurr.resth[DT_LV4],&gConfig.ccurr.mattime[DT_LV4], &gConfig.ccurr.restime[DT_LV4],DT_Over, DTCheckGetCurrent},
    /* 放电过流1级 */
	{DT_ID_DIS_CURH_LV1,  DT_Enable,DT_LV2, &gConfig.dcurr.matth[DT_LV2], &gConfig.dcurr.resth[DT_LV2],&gConfig.dcurr.mattime[DT_LV2], &gConfig.dcurr.restime[DT_LV2], DT_Under, DTCheckGetCurrent},
	/* 放电过流1级 */
	{DT_ID_DIS_CURH_LV2,  DT_Enable,DT_LV3, &gConfig.dcurr.matth[DT_LV3], &gConfig.dcurr.resth[DT_LV3],&gConfig.dcurr.mattime[DT_LV3], &gConfig.dcurr.restime[DT_LV3], DT_Under, DTCheckGetCurrent},
	/* 充电温度过高 */
	{DT_ID_CHG_TH,  DT_Enable,DT_LV4, &gConfig.cellcot.matth[DT_LV4], &gConfig.cellcot.resth[DT_LV4],&gConfig.cellcot.mattime[DT_LV4], &gConfig.cellcot.restime[DT_LV4], DT_Over, DTCheckGetMaxCellTemp},
	/* 放电温度过高 */
	{DT_ID_DIS_TH,  DT_Enable,DT_LV4, &gConfig.celldot.matth[DT_LV4], &gConfig.celldot.resth[DT_LV4],&gConfig.celldot.mattime[DT_LV4], &gConfig.celldot.restime[DT_LV4], DT_Over, DTCheckGetMaxCellTemp},
	/* 充电温度过低 */
	{DT_ID_CHG_TL,  DT_Enable,DT_LV4, &gConfig.cellcut.matth[DT_LV4], &gConfig.cellcut.resth[DT_LV4],&gConfig.cellcut.mattime[DT_LV4], &gConfig.cellcut.restime[DT_LV4], DT_Under, DTCheckGetMinCellTemp},
	/* 放电温度过低 */
	{DT_ID_DIS_TL,  DT_Enable,DT_LV4, &gConfig.celldut.matth[DT_LV4], &gConfig.celldut.resth[DT_LV4],&gConfig.celldut.mattime[DT_LV4], &gConfig.celldut.restime[DT_LV4], DT_Under, DTCheckGetMinCellTemp},
	/* 充电MOS温度过高 */
	{DT_ID_CHG_MOS_TH,  DT_Enable,DT_LV4, &gConfig.pcbot.matth[DT_LV4], &gConfig.pcbot.resth[DT_LV4],&gConfig.pcbot.mattime[DT_LV4], &gConfig.pcbot.restime[DT_LV4], DT_Over, DTCheckGetPCBTemp},
	/* 放电MOS温度过高 */
	{DT_ID_DIS_MOS_TH,  DT_Enable,DT_LV4, &gConfig.pcbot.matth[DT_LV4], &gConfig.pcbot.resth[DT_LV4],&gConfig.pcbot.mattime[DT_LV4], &gConfig.pcbot.restime[DT_LV4], DT_Over, DTCheckGetPCBTemp},
	/* 连接器温度过高*/
	{DT_ID_CONN_TH,  DT_Enable,DT_LV4, &gConfig.connot.matth[DT_LV4], &gConfig.connot.resth[DT_LV4],&gConfig.connot.mattime[DT_LV4], &gConfig.connot.restime[DT_LV4], DT_Over, DTCheckGetConnTemp},
	/* 预启动温度过高*/
	{DT_ID_PRE_CHG_OTEMP,DT_Enable,DT_LV4, &gConfig.configadd.preotemp.matth[DT_LV4], &gConfig.configadd.preotemp.resth[DT_LV4],&gConfig.configadd.preotemp.mattime[DT_LV4], &gConfig.configadd.preotemp.restime[DT_LV4], DT_Over, DTCheckGetPreDsgTemp},
    /* 预启动失败超时*/
	{DT_ID_PRE_CHG_OT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.preotflt.mattime, &gConfig.preotflt.restime, DT_Bool, DTCheckGetPreOverTime},
	/* 预启动失败外部重载/短路*/
	{DT_ID_PRE_CHG_OC,DT_Enable,DT_LV1, NULL, NULL, &gConfig.preocflt.mattime, &gConfig.preocflt.restime, DT_Bool, DTCheckGetPreOverLoad},


	/* 放电短路 */
	{DT_ID_DIS_SC,  DT_Enable,DT_LV4, &gConfig.dcurr.matth[DT_LV4], &gConfig.dcurr.resth[DT_LV4],&gConfig.dcurr.mattime[DT_LV4], &gConfig.dcurr.restime[DT_LV4], DT_Under, DTCheckGetCurrent},
	/* 放电过流保护锁定*/
	{DT_ID_DIS_CURRH_LOCK,DT_Enable,DT_LV1, NULL, NULL, &gConfig.doclock.mattime, &gConfig.doclock.restime, DT_Bool, DTCheckGetDchgOCLock},
	/* 充电过流保护锁定*/
	{DT_ID_CHG_CURRH_LOCK,DT_Enable,DT_LV1, NULL, NULL, &gConfig.coclock.mattime, &gConfig.coclock.restime, DT_Bool, DTCheckGetChgOCLock},
	/* AFE ALERT故障 */
	{DT_ID_AFE_ALERT,DT_Enable,DT_LV1, NULL, NULL, &gConfig.afeale.mattime, &gConfig.afeale.restime, DT_Bool, DTCheckGetAFEOverAlertFault},
	/* AFE XREADY故障 */
	{DT_ID_AFE_XREADY,DT_Enable,DT_LV1, NULL, NULL, &gConfig.afexrdy.mattime, &gConfig.afexrdy.restime, DT_Bool, DTCheckGetAFEXRdyFault},
	/* 睡眠模式过流故障 */
	{DT_ID_SLEEP_CURRH,DT_Enable,DT_LV1, NULL, NULL, &gConfig.slpoc.mattime, &gConfig.slpoc.restime, DT_Bool, DTCheckGetSleepOCFault},
	/* TVS管温度过高  */
	{DT_ID_TVS_TH,  DT_Enable,DT_LV4, &gConfig.tvsot.matth[DT_LV4], &gConfig.tvsot.resth[DT_LV4],&gConfig.tvsot.mattime[DT_LV4], &gConfig.tvsot.restime[DT_LV4], DT_Over, DTCheckGetTVSTemp},


	/* 过充告警  */
    {DT_ID_CHG_VH_WARM, DT_Enable,DT_LV3, &gConfig.cellov.matth[DT_LV3], &gConfig.cellov.resth[DT_LV3],&gConfig.cellov.mattime[DT_LV3], &gConfig.cellov.restime[DT_LV3], DT_Over, DTCheckGetMaxCellVolt},
	/* 过放告警 */
	{DT_ID_DIS_VL_WARM, DT_Enable,DT_LV2, &gConfig.celluv.matth[DT_LV2], &gConfig.celluv.resth[DT_LV2],&gConfig.celluv.mattime[DT_LV2], &gConfig.celluv.restime[DT_LV2], DT_Under, DTCheckGetMinCellVolt},
	/* 充电过流告警 */
	{DT_ID_CHG_CURRH_WARM,  DT_Enable,DT_LV1, &gConfigChgCurr.matth[DT_LV1], &gConfigChgCurr.resth[DT_LV1],&gConfig.ccurr.mattime[DT_LV1], &gConfig.ccurr.restime[DT_LV1], DT_Over, DTCheckGetCurrent},
	/* 放电过流告警 */
	{DT_ID_DIS_CURRL_WARM,  DT_Enable,DT_LV1, &gConfig.dcurr.matth[DT_LV1], &gConfig.dcurr.resth[DT_LV1],&gConfig.dcurr.mattime[DT_LV1], &gConfig.dcurr.restime[DT_LV1], DT_Under, DTCheckGetCurrent},
	/* 充电温度过高告警 */
	{DT_ID_CHG_TH_WARM,  DT_Enable,DT_LV3, &gConfig.cellcot.matth[DT_LV3], &gConfig.cellcot.resth[DT_LV3],&gConfig.cellcot.mattime[DT_LV3], &gConfig.cellcot.restime[DT_LV3], DT_Over, DTCheckGetMaxCellTemp},
	/* 放电温度过高告警 */
	{DT_ID_DIS_TH_WARM,  DT_Enable,DT_LV3, &gConfig.celldot.matth[DT_LV3], &gConfig.celldot.resth[DT_LV3],&gConfig.celldot.mattime[DT_LV3], &gConfig.celldot.restime[DT_LV3], DT_Over, DTCheckGetMaxCellTemp},
	/* 充电温度过低告警 */
	{DT_ID_CHG_TL_WARM,  DT_Enable,DT_LV3, &gConfig.cellcut.matth[DT_LV3], &gConfig.cellcut.resth[DT_LV3],&gConfig.cellcut.mattime[DT_LV3], &gConfig.cellcut.restime[DT_LV3], DT_Under, DTCheckGetMinCellTemp},
	/* 放电温度过低告警 */
	{DT_ID_DIS_TL_WARM,  DT_Enable,DT_LV3, &gConfig.celldut.matth[DT_LV3], &gConfig.celldut.resth[DT_LV3],&gConfig.celldut.mattime[DT_LV3], &gConfig.celldut.restime[DT_LV3], DT_Under, DTCheckGetMinCellTemp},
	/* 最高电芯电压大于4.05V且最大压差大于50mV */
	{DT_ID_UN_BALANCE_LV1,  DT_Enable,DT_LV1, &gConfig.vdiff.matth[DT_LV1], &gConfig.vdiff.resth[DT_LV1],&gConfig.vdiff.mattime[DT_LV1], &gConfig.vdiff.restime[DT_LV1], DT_Over, DTCheckGetCellVoltDiff},
	/* 最高电芯电压大于4.05V且最大压差大于100mV */
	{DT_ID_UN_BALANCE_LV2,  DT_Enable,DT_LV2, &gConfig.vdiff.matth[DT_LV2], &gConfig.vdiff.resth[DT_LV2],&gConfig.vdiff.mattime[DT_LV2], &gConfig.vdiff.restime[DT_LV2], DT_Over, DTCheckGetCellVoltDiff},
	/* 最高电芯电压大于4.05V且最大压差大于200mV */
	{DT_ID_UN_BALANCE_LV3,  DT_Enable,DT_LV3, &gConfig.vdiff.matth[DT_LV3], &gConfig.vdiff.resth[DT_LV3],&gConfig.vdiff.mattime[DT_LV3], &gConfig.vdiff.restime[DT_LV3], DT_Over, DTCheckGetCellVoltDiff},
	/* 最高电芯电压大于4.05V且最大压差大于300mV */
	{DT_ID_UN_BALANCE_LV4,  DT_Enable,DT_LV4, &gConfig.vdiff.matth[DT_LV4], &gConfig.vdiff.resth[DT_LV4],&gConfig.vdiff.mattime[DT_LV4], &gConfig.vdiff.restime[DT_LV4], DT_Over, DTCheckGetCellVoltDiff},
	/* 电池温度传感器温差大于6度 */
	{DT_ID_TEMP_DIFF_LV1,  DT_Enable,DT_LV1, &gConfig.tdiff.matth[DT_LV1], &gConfig.tdiff.resth[DT_LV1],&gConfig.tdiff.mattime[DT_LV1], &gConfig.tdiff.restime[DT_LV1], DT_Over, DTCheckGetCellTempDiff},
	/* 电池温度传感器温差大于9度 */
	{DT_ID_TEMP_DIFF_LV2,  DT_Enable,DT_LV2, &gConfig.tdiff.matth[DT_LV2], &gConfig.tdiff.resth[DT_LV2],&gConfig.tdiff.mattime[DT_LV2], &gConfig.tdiff.restime[DT_LV2], DT_Over, DTCheckGetCellTempDiff},
	/* 电池温度传感器温差大于15度 */
	{DT_ID_TEMP_DIFF_LV3,  DT_Enable,DT_LV3, &gConfig.tdiff.matth[DT_LV3], &gConfig.tdiff.resth[DT_LV3],&gConfig.tdiff.mattime[DT_LV3], &gConfig.tdiff.restime[DT_LV3], DT_Over, DTCheckGetCellTempDiff},
	/* 电池温度传感器温差大于30度 */
	{DT_ID_TEMP_DIFF_LV4,  DT_Enable,DT_LV4, &gConfig.tdiff.matth[DT_LV4], &gConfig.tdiff.resth[DT_LV4],&gConfig.tdiff.mattime[DT_LV4], &gConfig.tdiff.restime[DT_LV4], DT_Over, DTCheckGetCellTempDiff},

};



//=============================================================================================
//声明静态函数
//=============================================================================================

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	:  void DTCheckInit(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  DTC 故障检测初始化函数
//注意事项	:  
//=============================================================================================
void DTCheckInit(void)
{
	;
}

//=============================================================================================
//函数名称	:  void DTCheckTask(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  DTC 故障检测任务函数
//注意事项	:
//=============================================================================================
void DTCheckTask(void)
{
	u8 i = 0;
	s16 checkval;
	static e_DTState laststat[DT_CHECK_CFG_LEN] = {(e_DTState)0};

    if(
    #ifdef BMS_USE_SOX_MODEL
        0 == DataDealGetBMSDataRdy(0x77)    //使用SOX算法，则需要关心SOX算法数据
    #else
        0 == DataDealGetBMSDataRdy(0x37) 
    #endif
    && (BSPTaskGetSysTimer() < 20))   //不关心任务数据是否准备，直接诊断
    {
    	return;
    }

	for(i = 0; i < DT_CHECK_CFG_LEN;i++)
	{
		//故障检测参数有效性判断
		//是否使能故障检测
		if((NULL != gDTConfig[i].getfunc)&& (DT_Enable == gDTConfig[i].enable)\
		   && (DT_ID_FIN > gDTConfig[i].id))
		{
			checkval = (*gDTConfig[i].getfunc)();	//获取检测值

			switch(gDTConfig[i].checktype)
			{
				//检测值大于成熟值，为故障；检测值小于恢复值，为正常
				case DT_Over:
					if(checkval >= *gDTConfig[i].matth)
					{
						DTTimerSetCurrentState((e_DT_ID)gDTConfig[i].id,DT_F);
					}
					else
					{
						DTTimerSetCurrentState((e_DT_ID)gDTConfig[i].id,DT_N);
					}

					//上一个状态是故障状态,并且还没到达恢复值
					if(DT_F == laststat[i] && (checkval > *gDTConfig[i].resth))
					{
						//保持在故障状态
						DTTimerSetCurrentState((e_DT_ID)gDTConfig[i].id,DT_F);
					}
					else if(checkval <= *gDTConfig[i].resth)
					{
						//已经恢复为正常状态
						DTTimerSetCurrentState((e_DT_ID)gDTConfig[i].id,DT_N);
					}

					laststat[i] = DTTimerGetState((e_DT_ID)gDTConfig[i].id);
					break;

				case DT_Under:
					if(checkval <= *gDTConfig[i].matth)
					{
						DTTimerSetCurrentState((e_DT_ID)gDTConfig[i].id,DT_F);
					}
					else
					{
						DTTimerSetCurrentState((e_DT_ID)gDTConfig[i].id,DT_N);
					}

					//上一个状态是故障状态,并且还没到达恢复值
					if(DT_F == laststat[i] && (checkval < *gDTConfig[i].resth))
					{
						//保持在故障状态
						DTTimerSetCurrentState((e_DT_ID)gDTConfig[i].id,DT_F);
					}
					else if(checkval >= *gDTConfig[i].resth)
					{
						//已经恢复为正常状态
						DTTimerSetCurrentState((e_DT_ID)gDTConfig[i].id,DT_N);
					}

					laststat[i] = DTTimerGetState((e_DT_ID)gDTConfig[i].id);

					break;

				case DT_Bool:
					if(IsEqual(1,checkval))
					{
						DTTimerSetCurrentState((e_DT_ID)gDTConfig[i].id,DT_F);
					}
					else
					{
						DTTimerSetCurrentState((e_DT_ID)gDTConfig[i].id,DT_N);
					}

					break;

				default:
					break;
			}
		}
	}
}

//=============================================================================================
//函数名称	:  u8 DTCheckFindIndexByDTID(u8 id)
//输入参数	:  id:DT故障id
//输出参数	:  DT故障id对应的数组索引,0xff:无效
//函数功能	:  查找DT故障id对应的数组索引
//注意事项	:
//=============================================================================================
u8 DTCheckFindIndexByDTID(u8 id)
{
	u8 i = 0;

	//判断id是否越界
	if(DT_ID_FIN <= id)
	{
		return 0xFF;

	}

	for(i = 0; i < DT_CHECK_CFG_LEN;i++)
	{
		if(id == gDTConfig[i].id)
		{
			return i;
		}
	}

	return 0xFF;
}

//=============================================================================================
//函数名称	: static u8 DTCheckGetChargePlugStaus(void)
//输入参数	: void
//输出参数	: 充电枪是否插入。1：插入；0：未插入
//静态变量	: void
//功    能	: 检测充电枪是否插入函数
//注    意	:
//=============================================================================================
static u8 DTCheckGetChargePlugStaus(void)
{
	u8 chgplgsta = 0;	//充电器是否插入状态

	chgplgsta =  CommCtrlIsChargerPlugIn();

	return	chgplgsta;
}

//=============================================================================================
//函数名称	:  void DTCheckRefreshFault(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  DTC 更新故障标志位
//注意事项	:
//=============================================================================================
void DTCheckRefreshFault(void)
{
	/* 设备故障1 */
	//AFE故障
    //if(DTTimerGetState(DT_ID_AFE_FAULT))
    if(DTCheckGetAFEFault())
    {
    	gBatteryInfo.Fault.FaultInfo[0] |= DEV_FAULT1_AFE;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[0] &= ~DEV_FAULT1_AFE;
    }

    //电量计故障
    if(DTTimerGetState(DT_ID_FUEL_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[0] |= DEV_FAULT1_FUEL;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[0] &= ~DEV_FAULT1_FUEL;
    }

    //时钟芯片故障
    if(DTTimerGetState(DT_ID_RTC_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[0] |= DEV_FAULT1_RTC;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[0] &= ~DEV_FAULT1_RTC;
    }

    //存储芯片故障
    if(DTTimerGetState(DT_ID_STOR_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[0] |= DEV_FAULT1_FLASH;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[0] &= ~DEV_FAULT1_FLASH;
    }

    //放电MOS失效故障
    if(DTTimerGetState(DT_ID_DIS_MOS_FAULT))
    {
        gBatteryInfo.Fault.FaultInfo[0] |= DEV_FAULT1_DCHG_MOS;
    }
    else
    {
        gBatteryInfo.Fault.FaultInfo[0] &= ~DEV_FAULT1_DCHG_MOS;
    }

    //充电MOS失效故障
    if(DTTimerGetState(DT_ID_CHG_MOS_FAULT))
    {
        gBatteryInfo.Fault.FaultInfo[0] |= DEV_FAULT1_CHG_MOS;
    }
    else
    {
        gBatteryInfo.Fault.FaultInfo[0] &= ~DEV_FAULT1_CHG_MOS;
    }

    //预启动电路故障
    if(DTTimerGetState(DT_ID_PRE_CHG_FAULT))
    {
        gBatteryInfo.Fault.FaultInfo[0] |= DEV_FAULT1_PRECHG;
    }
    else
    {
        gBatteryInfo.Fault.FaultInfo[0] &= ~DEV_FAULT1_PRECHG;
    }

    //时钟未校准故障
    if(DTTimerGetState(DT_ID_RTC_NOCALIB_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[0] |= DEV_FAULT1_RTCVAL;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[0] &= ~DEV_FAULT1_RTCVAL;
    }

    //参数未校准故障
    if(DTTimerGetState(DT_ID_NOPARA_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[0] |= DEV_FAULT1_CONFIG;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[0] &= ~DEV_FAULT1_CONFIG;
    }

    //保险丝失效故障
    if(DTTimerGetState(DT_ID_FUSE_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[0] |= DEV_FAULT1_FUSE;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[0] &= ~DEV_FAULT1_FUSE;
    }
    
    //加速度计故障
    if(DTTimerGetState(DT_ID_ACCELE_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[0] |= DEV_FAULT1_ACC;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[0] &= ~DEV_FAULT1_ACC;
    }    

    /* 设备故障1 */
    //充电MOS温度传感器失效故障
    if(DTTimerGetState(DT_ID_CHG_NTC_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[1] |= DEV_FAULT2_CHG_NTC;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[1] &= ~DEV_FAULT2_CHG_NTC;
    }

    //放电MOS温度传感器失效故障
    if(DTTimerGetState(DT_ID_DIS_NTC_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[1] |= DEV_FAULT2_DCHG_NTC;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[1] &= ~DEV_FAULT2_DCHG_NTC;
    }

    //电量计温度传感器失效故障
    if(DTTimerGetState(DT_ID_FULE_NTC_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[1] |= DEV_FAULT2_FUEL_NTC;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[1] &= ~DEV_FAULT2_FUEL_NTC;
    }

    //连接器温度传感器失效故障
    if(DTTimerGetState(DT_ID_CONN_NTC_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[1] |= DEV_FAULT2_CONN_NTC;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[1] &= ~DEV_FAULT2_CONN_NTC;
    }

    //电池温度传感器1失效故障
    if(DTTimerGetState(DT_ID_BAT_NTC1_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[1] |= DEV_FAULT2_B1_NTC;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[1] &= ~DEV_FAULT2_B1_NTC;
    }

    //电池温度传感器2失效故障
    if(DTTimerGetState(DT_ID_BAT_NTC2_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[1] |= DEV_FAULT2_B2_NTC;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[1] &= ~DEV_FAULT2_B2_NTC;
    }

    //TVS温度传感器失效故障
    if(DTTimerGetState(DT_ID_TVS_NTC_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[1] |= DEV_FAULT2_TVS_NTC;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[1] &= ~DEV_FAULT2_TVS_NTC;
    }
    
    //外置模块短路失效故障
    if(DTTimerGetState(DT_ID_EX_MODULE_FAULT))
    {
    	gBatteryInfo.Fault.FaultInfo[1] |= DEV_EX_MODULE_SHORT;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[1] &= ~DEV_EX_MODULE_SHORT;
    }
    
    /* 运行故障1 */
    //电池过压
    if(gChgOVFlag)
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_OV;
    }
    else
    {
        gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_OV;
    }

    //电池欠压1级
    if(gDchgUV1Flag)
    {
        gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_UV1;
    }
    else
    {
        gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_UV1;
    }

    //电池欠压2级
    if(gDchgUV2Flag)
    {
        gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_UV2;
    }
    else
    {
        gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_UV2;
    }
    
    //充电过流
    if((DTTimerGetState(DT_ID_CHG_CURH_LV1))
     ||(DTTimerGetState(DT_ID_CHG_CURH_LV2)) 
     ||(DTTimerGetState(DT_ID_CHG_CURH_LV3)))
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_CHG_OC;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_CHG_OC;
    }
    
    //放电过流1级
    if(DTTimerGetState(DT_ID_DIS_CURH_LV1))
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_DCHG_OC1;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_DCHG_OC1;
    }
    
    //放电过流2级
    if(DTTimerGetState(DT_ID_DIS_CURH_LV2))
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_DCHG_OC2;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_DCHG_OC2;
    }
    
    //电池充电过温
    if(DTTimerGetState(DT_ID_CHG_TH))
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_CHG_OT;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_CHG_OT;
    }
    
    //电池放电过温
    if(DTTimerGetState(DT_ID_DIS_TH))
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_DCHG_OT;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_DCHG_OT;
    }
    
    //电池充电低温
    if(DTTimerGetState(DT_ID_CHG_TL))
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_CHG_UT;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_CHG_UT;
    }
    
    //电池放电低温
    if(DTTimerGetState(DT_ID_DIS_TL))
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_DCHG_UT;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_DCHG_UT;
    }
    
    //充电MOS温度过高
    if(DTTimerGetState(DT_ID_CHG_MOS_TH))
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_CHG_MOSOT;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_CHG_MOSOT;
    }
    
    //放电MOS温度过高
    if(DTTimerGetState(DT_ID_DIS_MOS_TH))
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_DCHG_MOSOT;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_DCHG_MOSOT;
    }
    
    //连接器温度过高
    if(DTTimerGetState(DT_ID_CONN_TH))
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_CONN_OT;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_CONN_OT;
    }
    
    //B11使用，预启动电阻温度过高 21.01.26
    if(DTTimerGetState(DT_ID_PRE_CHG_OTEMP))
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_PRE_OTEMP;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_PRE_OTEMP;
    }
    
    //预启动失败超时
    //if(DTTimerGetState(DT_ID_PRE_CHG_OT))
    if((0 != (gMCData.predsg_err & PREDSG_ERR_TIMEOUT_MASK)))
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_PRE_OTIME;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_PRE_OTIME;
    }
    
    //预启动失败外部重载/短路
    if(DTTimerGetState(DT_ID_PRE_CHG_OC))
    {
    	gBatteryInfo.Fault.FaultInfo[2] |= OPT_FAULT1_PRE_OLOAD;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[2] &= ~OPT_FAULT1_PRE_OLOAD;
    }

    /* 运行故障2 */
    //放电短路
    if(DTTimerGetState(DT_ID_DIS_SC))
    {
    	gBatteryInfo.Fault.FaultInfo[3] |= OPT_FAULT2_DCHG_SHORT;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[3] &= ~OPT_FAULT2_DCHG_SHORT;
    }
    
    //放电过流保护锁定
    if(DTTimerGetState(DT_ID_DIS_CURRH_LOCK))
    {
    	gBatteryInfo.Fault.FaultInfo[3] |= OPT_FAULT2_DCHG_OC_LK;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[3] &= ~OPT_FAULT2_DCHG_OC_LK;
    }
    
    //充电过流保护锁定
    if(DTTimerGetState(DT_ID_CHG_CURRH_LOCK))
    {
    	gBatteryInfo.Fault.FaultInfo[3] |= OPT_FAULT2_CHG_OC_LK;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[3] &= ~OPT_FAULT2_CHG_OC_LK;
    }
    
    //AFE ALERT警告故障
    if(DTTimerGetState(DT_ID_AFE_ALERT))
    {
    	gBatteryInfo.Fault.FaultInfo[3] |= OPT_FAULT2_AFE_ALERT;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[3] &= ~OPT_FAULT2_AFE_ALERT;
    }
    
    //AFE XREADY故障
    if(DTTimerGetState(DT_ID_AFE_XREADY))
    {
    	gBatteryInfo.Fault.FaultInfo[3] |= OPT_FAULT2_AFE_XREADY;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[3] &= ~OPT_FAULT2_AFE_XREADY;
    }
    
    //睡眠模式过流故障
    if(DTTimerGetState(DT_ID_SLEEP_CURRH))
    {
    	gBatteryInfo.Fault.FaultInfo[3] |= OPT_FAULT2_SLP_OC;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[3] &= ~OPT_FAULT2_SLP_OC;
    }
    
    //TVS管温度过高
    if(DTTimerGetState(DT_ID_TVS_TH))
    {
    	gBatteryInfo.Fault.FaultInfo[3] |= OPT_FAULT2_TVS_OT;
    }
    else
    {
    	gBatteryInfo.Fault.FaultInfo[3] &= ~OPT_FAULT2_TVS_OT;
    }
    
    //非法充电
    //电量异常：在无充电器接入命令的情况下，非法充入250、500、1000mAh电量，置为对应标志
    if((gBatteryInfo.IllegalChg.ChgCapacity / 10000000) > 100)      //1000mAh
    {
        gBatteryInfo.Fault.FaultInfo[3] &= ~(OPT_FAULT2_ILLEGAL_CHG_ALL);
        gBatteryInfo.Fault.FaultInfo[3] |= (OPT_FAULT2_ILLEGAL_CHG_1000MAH);
    }
    else if((gBatteryInfo.IllegalChg.ChgCapacity / 10000000) > 50)  //500mAh
    {
        gBatteryInfo.Fault.FaultInfo[3] &= ~(OPT_FAULT2_ILLEGAL_CHG_ALL);
        gBatteryInfo.Fault.FaultInfo[3] |= (OPT_FAULT2_ILLEGAL_CHG_500MAH);
    }
//    else if((gBatteryInfo.IllegalChg.ChgCapacity / 10000000) > 25)  //250mAh
//    {
//        gBatteryInfo.Fault.FaultInfo[3] &= ~(OPT_FAULT2_ILLEGAL_CHG_1000MAH);
//        gBatteryInfo.Fault.FaultInfo[3] |= (OPT_FAULT2_ILLEGAL_CHG_250MAH);
//    }
    else
    {
        gBatteryInfo.Fault.FaultInfo[3] &= ~(OPT_FAULT2_ILLEGAL_CHG_ALL);
    }
    
    //在无充电器接入命令的情况下，电压下降3.75V以下持续2分钟以上，电压回升到4.08V以上持续2分钟以上
    if(gBatteryInfo.IllegalChg.volt_rise_sta == 0xAA)
    {
        gBatteryInfo.Fault.FaultInfo[3] |= (OPT_FAULT2_ILLEGAL_VOLT_RISE);
    }
    else
    {
        gBatteryInfo.Fault.FaultInfo[3] &= ~(OPT_FAULT2_ILLEGAL_VOLT_RISE);
    }    

    //热失控,近10s内温升大于或等于3度且持续8s,且近120s内电压下降超过100mV
    //if((gTempRunAwayCnt[0] >= 8) || (gTempRunAwayCnt[1] >= 8) || (gTempRunAwayCnt[2] >= 8))
    if((gTempRunAwayCnt[0] >= 8) || (gTempRunAwayCnt[1] >= 8))
    {
        if(gBatteryInfo.VoltChara.MinDropVolt < (-100))
        {
            gBatteryInfo.Fault.FaultInfo[3] |= OPT_FAULT2_TEMP_RISE_FAST;
        }
    }
    //恢复条件：温升速度下降到1.5度以下，等待6+故障持续时间，最大64s，解除故障
    //else if((gTempRunAwayCnt[0] < 2) && (gTempRunAwayCnt[1] < 2) && (gTempRunAwayCnt[2] < 2))
    else if((gTempRunAwayCnt[0] < 2) && (gTempRunAwayCnt[1] < 2))
    {
        gBatteryInfo.Fault.FaultInfo[3] &= ~OPT_FAULT2_TEMP_RISE_FAST;
    }    

    /* 运行告警1 */
    //过充告警
    if(DTTimerGetState(DT_ID_CHG_VH_WARM))
    {
    	gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_OV;
    }
    else
    {
    	gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_OV;
    }
    
    //过放告警
    if(DTTimerGetState(DT_ID_DIS_VL_WARM))
    {
    	gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_UV;
    }
    else
    {
    	gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_UV;
    }
    
    //充电过流告警
    if(DTTimerGetState(DT_ID_CHG_CURRH_WARM))
    {
    	gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_CHG_OC;
    }
    else
    {
    	gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_CHG_OC;
    }
    
    //放电过流告警
    if(DTTimerGetState(DT_ID_DIS_CURRL_WARM))
    {
    	gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_DCHG_OC;
    }
    else
    {
    	gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DCHG_OC;
    }
    
    //充电高温告警
    if(DTTimerGetState(DT_ID_CHG_TH_WARM))
     {
     	gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_CHG_OT;
     }
     else
     {
     	gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_CHG_OT;
     }
     
     //放电高温告警
     if(DTTimerGetState(DT_ID_DIS_TH_WARM))
     {
     	gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_DCHG_OT;
     }
     else
     {
     	gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DCHG_OT;
     }
     
     //充电低温告警
     if(DTTimerGetState(DT_ID_CHG_TL_WARM))
     {
     	gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_CHG_UT;
     }
     else
     {
     	gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_CHG_UT;
     }
     
     //放电低温告警
     if(DTTimerGetState(DT_ID_DIS_TL_WARM))
     {
     	gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_DCHG_UT;
     }
     else
     {
     	gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DCHG_UT;
     }

     //只有充电时才会检测压差问题，并报告警
     if((DTCheckGetMaxCellVolt() >= DT_CHECK_VDIFF_VOLT) && (DTCheckGetChargePlugStaus()))
     {
		 //最高电芯电压大于3.4V且最大压差大于300mV
		 if(DTTimerGetState(DT_ID_UN_BALANCE_LV4))
		 {
             gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV1;
             gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV2;
             gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV3;
             gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_DIFF_V_LV4;
		 }
		 else
		 {
			 gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV4;
             //最高电芯电压大于3.4V且最大压差大于200mV
             if(DTTimerGetState(DT_ID_UN_BALANCE_LV3))
             {
                 gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV1;
                 gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV2;
                 gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV4;	                 
                 gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_DIFF_V_LV3;
             }
             else
             {
                 gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV3;
                 //最高电芯电压大于3.4V且最大压差大于100mV
                 if(DTTimerGetState(DT_ID_UN_BALANCE_LV2))
                 {
                    gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV1;
                    gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV3;
                    gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV4;                     
                    gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_DIFF_V_LV2;
                 }
                 else
                 {
                     gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV2;
                     //最高电芯电压大于3.4V且最大压差大于50mV
                     if(DTTimerGetState(DT_ID_UN_BALANCE_LV1))
                     {
                        gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV2;
                        gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV3;
                        gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV4;                         
                        gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_DIFF_V_LV1;
                     }
                     else
                     {
                        gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV1;
                     }                 
                 
                 }                 
             }		 
         
         }
         
     }
//     else
//     {
//    	 gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV1;
//    	 gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV2;
//    	 gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV3;
//    	 gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_V_LV4;
//     }

      //电池温度传感器温差大于30度
      if(DTTimerGetState(DT_ID_TEMP_DIFF_LV4))
      {
          gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV1;
          gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV2;
          gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV3;
      	  gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_DIFF_T_LV4;
      }
      else
      {
          gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV4;
          //电池温度传感器温差大于15度
          if(DTTimerGetState(DT_ID_TEMP_DIFF_LV3))
          {
              gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV1;
              gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV2;
              gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV4;
              gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_DIFF_T_LV3;
          }
          else
          {
              gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV3;
              //电池温度传感器温差大于9度
              if(DTTimerGetState(DT_ID_TEMP_DIFF_LV2))
              {
                  gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV1;
                  gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV3;
                  gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV4;                  
                  gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_DIFF_T_LV2;
              }
              else
              {
                  gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV2;
                 //电池温度传感器温差大于6度
                 if(DTTimerGetState(DT_ID_TEMP_DIFF_LV1))
                  {
                      gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV2;
                      gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV3;
                      gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV4;                        
                      gBatteryInfo.Fault.AlarmInfo[0] |= OPT_WARN1_DIFF_T_LV1;
                  }
                  else
                  {
                      gBatteryInfo.Fault.AlarmInfo[0] &= ~OPT_WARN1_DIFF_T_LV1;
                  }                  
              }              
          }          
      }
}

//=============================================================================================
//函数名称	:  void DTCheckChargeOV(void)
//输入参数	:  void
//输出参数	:
//函数功能	:  检测充电高压判断
//注意事项	:
//=============================================================================================
void DTCheckChargeOV(void)
{
    static u8 timer;

    if(0 == DataDealGetBMSDataRdy(0x03))    //AFE、电量计数据已准备好
    {
    	return;
    }

    if(gChgOVFlag == 0)
    {
        if(DT_F == DTTimerGetState(DT_ID_VH))
        {
            gChgOVFlag = 1;
            timer = 0;
        }
    }		//恢复条件
    else
    {
        #if defined(LFP_AK_15000MAH_16S) || defined(LFP_TB_20000MAH_20S) || defined(LFP_HL_25000MAH_16S)
        if((DT_N == DTTimerGetState(DT_ID_VH)))	//充电器拔出 和 高压恢复
        #else
        if((!CommCtrlIsChargerPlugIn()) && (DT_N == DTTimerGetState(DT_ID_VH)))	//充电器拔出 和 高压恢复
        #endif
        {
            gChgOVFlag = 0;
        }

        if(timer < 100)	//100ms周期，10s为计数100
        {
             timer++;
        	if(DT_N == DTTimerGetState(DT_ID_VH))//如果10s内恢复则不判断charger接入状态
            {
        		timer = 0;
                gChgOVFlag = 0;
            }
        }
        else
        {
        	timer = 101;	//10s后不回复则等待高压消失
        }

    }
}


//=============================================================================================
//函数名称	:  void DTCheckChargeUV1(void)
//输入参数	:  void
//输出参数	:
//函数功能	:  检测放电低压1级判断
//注意事项	:
//=============================================================================================
void DTCheckChargeUV1(void)
{
    if(0 == DataDealGetBMSDataRdy(0x03))    //AFE、电量计数据已准备好
    {
    	return;
    }

    if(gDchgUV1Flag == 0)
    {
    	//发生一级欠压和soc过低
        if(((DT_F == DTTimerGetState(DT_ID_VL_LV1)) && (DT_F == DTTimerGetState(DT_ID_USOC_LV1))))
        {
            gDchgUV1Flag = 1;
        }
    }
    else
    {
    	//恢复一级欠压和soc过低
        //老化模式下，电压恢复就可以恢复
        //if((CommCtrlIsChargerPlugIn()) &&((DT_N == DTTimerGetState(DT_ID_VL_LV1)) && (DT_N == DTTimerGetState(DT_ID_USOC_LV1))))
        if((CommCtrlIsChargerPlugIn() && (DT_N == DTTimerGetState(DT_ID_VL_LV1)) && (8 == gPCBTest.presentstep))
            || (CommCtrlIsChargerPlugIn() && (DT_N == DTTimerGetState(DT_ID_VL_LV1)) && (DT_N == DTTimerGetState(DT_ID_USOC_LV1)) && (8 != gPCBTest.presentstep)))
        {
            gDchgUV1Flag = 0;
        }
    }
}

//=============================================================================================
//函数名称	:  void DTCheckChargeUV2(void)
//输入参数	:  void
//输出参数	:
//函数功能	:  检测放电低压2级判断
//注意事项	:
//=============================================================================================
void DTCheckChargeUV2(void)
{
//	static u8 timer;

    if(0 == DataDealGetBMSDataRdy(0x03))    //AFE、电量计数据已准备好
    {
    	return;
    }

    if(gDchgUV2Flag == 0)
    {
        if((DT_F == DTTimerGetState(DT_ID_VL_LV2)))
        {
            gDchgUV2Flag = 1;
//            timer = 0;
        }
    }
    else
    {
        if((CommCtrlIsChargerPlugIn()) && (DT_N == DTTimerGetState(DT_ID_VL_LV2))/*&&(g_PROTECT_CTRL.discharge_uvp1_flag==0)*/)
        {
            gDchgUV2Flag = 0;
        }
        /*
        if(timer < 100)	//100ms周期，10s为计数100
        {
             timer++;
        	if(DT_N == DTTimerGetState(DT_ID_VL_LV2))//如果10s内恢复则不判断charger接入状态
            {
        		timer = 0;
        		gDchgUV2Flag = 0;
            }
        }
        else
        {
        	timer = 101;	//10s后不回复则等待高压消失
        }
        */
    }
}

//=============================================================================================
//函数名称	:  void DTCheckOCLockStatusClr(uint8_t ctrl)
//输入参数	:  ctrl：//bit0 : 充电过流锁定；bit1 : 放电过流锁定
//输出参数	:  void
//函数功能	:  充放过流保护锁定清除
//注意事项	:
//=============================================================================================
void DTCheckOCLockStatusClr(uint8_t ctrl)
{
    uint8_t i;
    for(i=0;i<2;i++)
    {
        if(ctrl&(0x1<<i))
        {
			gOCCnt[i] = 0;
			gOCLockTimer[i][0] = 0;
			gOCLockTimer[i][1] = 0;
			gOCLockTimer[i][2] = 0;
			gOCLock &= ~(0x01<<i);
        }
    }
}

//=============================================================================================
//函数名称	:  void DTCheckOCLockStatusUpdate(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  充放过流保护锁定标志更新任务
//注意事项	:
//=============================================================================================
void DTCheckOCLockStatusUpdate(void)
{
    u32 sum;
    u8 i;

    if(0 == DataDealGetBMSDataRdy(0x03))    //AFE、电量计数据已准备好
    {
    	return;
    }

    //if((g_PROTECT_CTRL.ocurr.level >= 2) && (gOCLockLastLevel[0] <= 1))
    if(((DT_F == DTTimerGetState(DT_ID_CHG_CURH_LV1)) || (DT_F == DTTimerGetState(DT_ID_CHG_CURH_LV2))|| (DT_F == DTTimerGetState(DT_ID_CHG_CURH_LV3))) 
        && (gOCLockLastLevel[0] <= 1))
    {
        gOCCnt[0]++;
    }

    //计算出最高的等级
    //gOCLockLastLevel[0] = g_PROTECT_CTRL.ocurr.level;
    if(DT_F == DTTimerGetState(DT_ID_CHG_CURH_LV3))
    {
        gOCLockLastLevel[0] = DT_LV4 + 1;
    }
    else if(DT_F == DTTimerGetState(DT_ID_CHG_CURH_LV2))
    {
        gOCLockLastLevel[0] = DT_LV3 + 1;
    }    
    else if(DT_F == DTTimerGetState(DT_ID_CHG_CURH_LV1))
    {
        gOCLockLastLevel[0] = DT_LV2 + 1;
    }     
    else if(DT_F == DTTimerGetState(DT_ID_CHG_CURRH_WARM))
    {
        gOCLockLastLevel[0] = DT_LV1 + 1;
    }  
    else
    {
        gOCLockLastLevel[0] = 0;
    }   
    
    //if((g_PROTECT_CTRL.ucurr.level >= 2)&&(gOCLockLastLevel[1] <= 1))
    if(((DT_F == DTTimerGetState(DT_ID_DIS_CURH_LV1)) || (DT_F == DTTimerGetState(DT_ID_DIS_CURH_LV2))|| (DT_F == DTTimerGetState(DT_ID_DIS_SC)))   
        && (gOCLockLastLevel[1] <= 1))
    {
        gOCCnt[1]++;
    }

    //计算出最高的等级
    //gOCLockLastLevel[1] = g_PROTECT_CTRL.ucurr.level;
    if(DT_F == DTTimerGetState(DT_ID_DIS_SC))
    {
        gOCLockLastLevel[1] = DT_LV4 + 1;
    }
    else if(DT_F == DTTimerGetState(DT_ID_DIS_CURH_LV2))
    {
        gOCLockLastLevel[1] = DT_LV3 + 1;
    }    
    else if(DT_F == DTTimerGetState(DT_ID_DIS_CURH_LV1))
    {
        gOCLockLastLevel[1] = DT_LV2 + 1;
    }     
    else if(DT_F == DTTimerGetState(DT_ID_DIS_CURRL_WARM))
    {
        gOCLockLastLevel[1] = DT_LV1 + 1;
    }  
    else
    {
        gOCLockLastLevel[1] = 0;
    }   
   

    for(i = 0;i < 2;i++)
    {
        if(gOCCnt[i]!=0)
        {
            if(gOCCnt[i]>=3)
            {
                gOCCnt[i] = 0;
                gOCLockTimer[i][0] = 0;
                gOCLockTimer[i][1] = 0;
                gOCLockTimer[i][2] = 0;
                gOCLock |= (0x01<<i);
            }
            else
            {
                sum = gOCLockTimer[i][0]+gOCLockTimer[i][1]+gOCLockTimer[i][2];
                if(sum >= 6000)	//100ms周期，10min
                {
                    gOCLockTimer[i][0] = gOCLockTimer[i][1];
                    gOCLockTimer[i][1] = gOCLockTimer[i][2] ;
                    gOCLockTimer[i][2] = 0;
                    gOCCnt[i]--;
                }
                gOCLockTimer[i][gOCCnt[i]-1]++;
            }
        }
    }

}

//=============================================================================================
//函数名称	:  void DTCheckIllegalChgUpdate(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  非法充电、电压异常上升标志更新任务
//注意事项	:
//=============================================================================================
void DTCheckIllegalChgUpdate(void)
{
    u32 coulom;    
    
    //增加非法充电检测：电量异常：在无充电器接入命令的情况下，非法充入250、500、1000mAh电量，置为对应标志
    //计算对应的充入容量
    if((!DTCheckGetChargePlugStaus()) && (0 == gPCBTest.presentstep) && (DataDealGetBMSDataRdy(0x08)))  //时间RTC准备好
    {
        if(gBatteryInfo.Data.Curr > 5)  //大于50ma的充电电流
        {
            //250ms周期下
            //coulom = gBatteryInfo.IllegalChg.ChgCapacity + gBatteryInfo.Data.Curr * 6944 / 10;//capcity=250*c/(1000*60*60)
            //100ms周期下
            coulom = gBatteryInfo.IllegalChg.ChgCapacity + gBatteryInfo.Data.Curr * 2778 / 10;  //capcity = 100 * c / (1000*60*60)
            
            if(coulom > gBatteryInfo.IllegalChg.ChgCapacity)
            {
                gBatteryInfo.IllegalChg.ChgCapacity = coulom;
            }
            else
            {
                gBatteryInfo.IllegalChg.ChgCapacity = 0xFFFFFFFF;
            }
        }
        else if(gBatteryInfo.Data.Curr < -5)    //放电
        {
            gBatteryInfo.IllegalChg.ChgCapacity = 0;
        }
        
        //计算间隔时间
        if((gRdTimeAndDate.minute > gBatteryInfo.IllegalChg.start_dsg_time_m) ||
            (gRdTimeAndDate.minute==gBatteryInfo.IllegalChg.start_dsg_time_m 
            && gRdTimeAndDate.second>=gBatteryInfo.IllegalChg.start_dsg_time_s))
        {
            gBatteryInfo.IllegalChg.curr_dsg_time =((gRdTimeAndDate.minute >> 4)*10+(gRdTimeAndDate.minute & 0x0F)) * 60+
                (gRdTimeAndDate.second >> 4) * 10+(gRdTimeAndDate.second & 0x0F)-
                (((gBatteryInfo.IllegalChg.start_dsg_time_m >> 4) * 10+(gBatteryInfo.IllegalChg.start_dsg_time_m & 0x0F)) * 60+
                (gBatteryInfo.IllegalChg.start_dsg_time_s >> 4)*10+(gBatteryInfo.IllegalChg.start_dsg_time_s & 0x0F));
        }
        else
        {
            gBatteryInfo.IllegalChg.curr_dsg_time = 3600+((gRdTimeAndDate.minute>>4)*10+(gRdTimeAndDate.minute&0x0F))*60+
                (gRdTimeAndDate.second >> 4)*10+(gRdTimeAndDate.second & 0x0F)-
                (((gBatteryInfo.IllegalChg.start_dsg_time_m >> 4)*10+(gBatteryInfo.IllegalChg.start_dsg_time_m & 0x0F))*60+
                (gBatteryInfo.IllegalChg.start_dsg_time_s >> 4)*10+(gBatteryInfo.IllegalChg.start_dsg_time_s & 0x0F));
        }
        
        #if defined(SY_PB_32000MAH_17S) || defined(SY_PB_32000MAH_14S)
        //电压异常：在无充电器接入命令的情况下，电压下降3.75V以下持续2分钟以上，电压回升到4.08V以上持续2分钟以上
        if(gBatteryInfo.IllegalChg.volt_rise_sta <= 0x01)
        {
            if(gBatteryInfo.VoltChara.MaxVolt < (MAX_CHG_VOLT - 350))
            {
                gBatteryInfo.IllegalChg.volt_rise_sta = 0x02;
                gBatteryInfo.IllegalChg.curr_dsg_cnt = 0;
                gBatteryInfo.IllegalChg.start_dsg_time_s = gRdTimeAndDate.second;
                gBatteryInfo.IllegalChg.start_dsg_time_m = gRdTimeAndDate.minute;  
            }
        }        
        else if(gBatteryInfo.IllegalChg.volt_rise_sta == 0x02)
        {
            if(gBatteryInfo.VoltChara.MaxVolt < (MAX_CHG_VOLT - 350))
            {
                if(gBatteryInfo.IllegalChg.curr_dsg_time > 120 ||
                     gBatteryInfo.IllegalChg.curr_dsg_cnt > 1200)    //持续了2min
                {
                    gBatteryInfo.IllegalChg.volt_rise_sta = 0x03;
                    gBatteryInfo.IllegalChg.curr_dsg_cnt = 0;
                    gBatteryInfo.IllegalChg.start_dsg_time_s = gRdTimeAndDate.second;
                    gBatteryInfo.IllegalChg.start_dsg_time_m = gRdTimeAndDate.minute;  
                }
                else
                {
                     gBatteryInfo.IllegalChg.curr_dsg_cnt++;
                }
            }
            else
            {
                gBatteryInfo.IllegalChg.curr_dsg_cnt = 0;
                gBatteryInfo.IllegalChg.start_dsg_time_s = gRdTimeAndDate.second;
                gBatteryInfo.IllegalChg.start_dsg_time_m = gRdTimeAndDate.minute;  
            }
        }        
        else if(gBatteryInfo.IllegalChg.volt_rise_sta == 0x03)
        {
            if(gBatteryInfo.VoltChara.MaxVolt > (MAX_CHG_VOLT - 20))
            {
                if(gBatteryInfo.IllegalChg.curr_dsg_time > 120||
                    gBatteryInfo.IllegalChg.curr_dsg_cnt > 1200)    //持续了2min
                {
                    gBatteryInfo.IllegalChg.volt_rise_sta = 0xAA;
                    gBatteryInfo.IllegalChg.curr_dsg_cnt = 0x0000;
                    gBatteryInfo.IllegalChg.start_dsg_time_s = gRdTimeAndDate.second;
                    gBatteryInfo.IllegalChg.start_dsg_time_m = gRdTimeAndDate.minute;  
                }
                else
                {
                    gBatteryInfo.IllegalChg.curr_dsg_cnt++;
                }
            }
            else
            {
                gBatteryInfo.IllegalChg.curr_dsg_cnt = 0x0000;
                gBatteryInfo.IllegalChg.start_dsg_time_s = gRdTimeAndDate.second;
                gBatteryInfo.IllegalChg.start_dsg_time_m = gRdTimeAndDate.minute;  
            }
        }  
          #endif
    }
    else
    {
        gBatteryInfo.IllegalChg.curr_dsg_cnt = 0;
        gBatteryInfo.IllegalChg.ChgCapacity = 0;
        #if defined(SY_PB_32000MAH_17S) || defined(SY_PB_32000MAH_14S)
        gBatteryInfo.IllegalChg.volt_rise_sta = 0;
        #endif
        gBatteryInfo.IllegalChg.start_dsg_time_s = gRdTimeAndDate.second;
        gBatteryInfo.IllegalChg.start_dsg_time_m = gRdTimeAndDate.minute;  
    }
}

//=============================================================================================
//函数名称	:  void DTCheckAFEProtectFlag(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  AFE设备保护标志更新任务
//注意事项	:
//=============================================================================================
//void DTCheckAFEProtectFlag(void)
//{
//    static uint16_t cnt=0;
//    uint8_t stat;

//    stat = gAFEData.devsta;	//获取AFE设备系统状态

//    if(stat & 0x3f)
//    {
//    	//存在异常,需要保护
//        gAFEData.protflag = 1;
//    }


//    if(BQ76940_STAT_OCD == (stat & BQ76940_STAT_OCD))
//    {
//        //g_PROTECT_CTRL.ucurr.level = 3;
//    	//直接设置为过流2级故障
//    	DTTimerSetState(DT_ID_DIS_CURH_LV2,DT_F);
//        BQ76940DevStatClear(BQ76940_STAT_OCD);
//    }
//    if(BQ76940_STAT_SCD == (stat & BQ76940_STAT_SCD))
//    {
//        //g_PROTECT_CTRL.ucurr.level = 4;
//    	//直接设置为短路故障
//    	DTTimerSetState(DT_ID_DIS_SC,DT_F);
//        BQ76940DevStatClear(BQ76940_STAT_SCD);
//    }
//    if(BQ76940_STAT_OV == (stat & BQ76940_STAT_OV))
//    {
//        //g_PROTECT_CTRL.sig_ov.level = 4;
//    	//直接设置为过压故障
//    	DTTimerSetState(DT_ID_VH,DT_F);
//        BQ76940DevStatClear(BQ76940_STAT_OV);
//    }
//    if(BQ76940_STAT_UV == (stat & BQ76940_STAT_UV))
//    {
//        //g_PROTECT_CTRL.sig_uv.level = 4;
//    	//直接设置为2级欠压故障
//    	DTTimerSetState(DT_ID_VL_LV2,DT_F);
//        BQ76940DevStatClear(BQ76940_STAT_UV);
//    }


//    if((BQ76940_STAT_OVRD_ALERT == (stat & BQ76940_STAT_OVRD_ALERT))
//    	||(BQ76940_STAT_XREADY_ERR == (stat & BQ76940_STAT_XREADY_ERR)))
//    {
//        cnt=1;
//        gAFEData.devfault = (stat&(BQ76940_STAT_OVRD_ALERT | BQ76940_STAT_XREADY_ERR)) >> 4;
//        BQ76940DevStatClear(BQ76940_STAT_OVRD_ALERT | BQ76940_STAT_XREADY_ERR);
//    }
//    else if(cnt!=0)
//    {
//        cnt++;
//        if(cnt >= 100) //100ms周期
//        {
//            cnt = 0;	//10S秒后清零
//            gAFEData.devfault = 0;
//        }
//    }
//}

//=============================================================================================
//函数名称	:  void DTCheckAFEProtectFlag(void)
//输入参数	:  void
//输出参数	:  void
//函数功能	:  AFE设备保护标志更新任务
//注意事项	:
//=============================================================================================
void DTCheckAFEProtectFlag(void)
{
    uint8_t stat;

    stat = gSHAFEData.devsta;	//获取AFE设备系统状态

    if(stat & 0x7f)
    {
    	//存在异常,需要保护
        gSHAFEData.protflag = 1;
    }
    
    if(SH367309_STAT_OCD1 == (stat & SH367309_STAT_OCD1))
    {
        //g_PROTECT_CTRL.ucurr.level = 2;
    	//直接设置为过流1级故障
        #if defined(LFP_TB_20000MAH_20S) || defined(SY_PB_32000MAH_17S) || defined(LFP_HL_25000MAH_16S)
        DTTimerSetState(DT_ID_DIS_CURH_LV2,DT_F);
        #else
        DTTimerSetState(DT_ID_DIS_CURH_LV1,DT_F);
        #endif
        SH367309DevStatClear(SH367309_CLR_OCD1);
    }

    if(SH367309_STAT_OCD2 == (stat & SH367309_STAT_OCD2))
    {
        //g_PROTECT_CTRL.ucurr.level = 3;
    	//直接设置为过流2级故障
    	DTTimerSetState(DT_ID_DIS_CURH_LV2,DT_F);
        SH367309DevStatClear(SH367309_CLR_OCD2);
    }
    if(SH367309_STAT_SC == (stat & SH367309_STAT_SC))
    {
        //g_PROTECT_CTRL.ucurr.level = 4;
    	//直接设置为短路故障
    	DTTimerSetState(DT_ID_DIS_SC,DT_F);
        SH367309DevStatClear(SH367309_CLR_SC);
    }
    if(SH367309_STAT_OV == (stat & SH367309_STAT_OV))
    {
        //g_PROTECT_CTRL.sig_ov.level = 4;
    	//直接设置为过压故障
    	DTTimerSetState(DT_ID_VH,DT_F);
        SH367309DevStatClear(SH367309_CLR_OV);
    }
    if(SH367309_STAT_UV == (stat & SH367309_STAT_UV))
    {
        //g_PROTECT_CTRL.sig_uv.level = 4;
    	//直接设置为2级欠压故障
    	DTTimerSetState(DT_ID_VL_LV2,DT_F);
        SH367309DevStatClear(SH367309_CLR_UV);
    }
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetMaxCellVolt(void)
//输入参数	:  void
//输出参数	:  单体电压最大值
//函数功能	:  获取单体电压最大值
//注意事项	:
//=============================================================================================
s16 DTCheckGetMaxCellVolt(void)
{
	return	(s16)gBatteryInfo.VoltChara.MaxVolt;
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetCellVoltDiff(void)
//输入参数	:  void
//输出参数	:  单体电压差异值
//函数功能	:  获取单体电压差异值
//注意事项	:
//=============================================================================================
s16 DTCheckGetCellVoltDiff(void)
{
	return	(s16)(gBatteryInfo.VoltChara.MaxVolt - gBatteryInfo.VoltChara.MinVolt);
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetMinCellVolt(void)
//输入参数	:  void
//输出参数	:  单体电压最小值
//函数功能	:  获取单体电压最小值
//注意事项	:
//=============================================================================================
s16 DTCheckGetMinCellVolt(void)
{
	return	(s16)gBatteryInfo.VoltChara.MinVolt;
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetMaxCellTemp(void)
//输入参数	:  void
//输出参数	:  单体温度最大值
//函数功能	:  获取单体温度最大值
//注意事项	:
//=============================================================================================
s16 DTCheckGetMaxCellTemp(void)
{
	return	(s16)gBatteryInfo.TempChara.MaxTemp;
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetCellTempDiff(void)
//输入参数	:  void
//输出参数	:  单体温度差异值
//函数功能	:  获取单体温度差异值
//注意事项	:
//=============================================================================================
s16 DTCheckGetCellTempDiff(void)
{
	return	(s16)(gBatteryInfo.TempChara.MaxTemp - gBatteryInfo.TempChara.MinTemp);
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetMinCellTemp(void)
//输入参数	:  void
//输出参数	:  单体温度最小值
//函数功能	:  获取单体温度最小值
//注意事项	:
//=============================================================================================
s16 DTCheckGetMinCellTemp(void)
{
	return	(s16)gBatteryInfo.TempChara.MinTemp;
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetPCBTemp(void)
//输入参数	:  void
//输出参数	:  PCB温度
//函数功能	:  获取PCB温度
//注意事项	:
//=============================================================================================
s16 DTCheckGetPCBTemp(void)
{
	return	(s16)gSampleData.PCBTemp;
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetConnTemp(void)
//输入参数	:  void
//输出参数	:  连接器温度
//函数功能	:  获取连接器温度
//注意事项	:
//=============================================================================================
s16 DTCheckGetConnTemp(void)
{
	return	(s16)gSampleData.ConnTemp;
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetPreDsgTemp(void)
//输入参数	:  void
//输出参数	:  预放电电阻温度
//函数功能	:  获取预放电电阻温度
//注意事项	:
//=============================================================================================
s16 DTCheckGetPreDsgTemp(void)
{
	return	(s16)gSampleData.PreTemp;
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetTVSTemp(void)
//输入参数	:  void
//输出参数	:  TVS温度
//函数功能	:  获取TVS温度
//注意事项	:
//=============================================================================================
s16 DTCheckGetTVSTemp(void)
{
	return	(s16)gSampleData.TVSTemp;
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetCurrent(void)
//输入参数	:  void
//输出参数	:
//函数功能	:  获取电流大小
//注意事项	:
//=============================================================================================
s16 DTCheckGetCurrent(void)
{
	return	(s16)gBatteryInfo.Data.Curr;
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetSOC(void)
//输入参数	:  void
//输出参数	:
//函数功能	:  获取SOC大小
//注意事项	:
//=============================================================================================
s16 DTCheckGetSOC(void)
{
	return	(s16)gBatteryInfo.Data.FiltSOC;
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetAFEFault(void)
//输入参数	:  void
//输出参数	:  AFE故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetAFEFault(void)
{
#if defined(USE_B20_TOPBAND_PCBA) || defined(USE_B22_IM_PCBA)
    return (gSHAFEData.faultcnt > 0) || (gSHAFEData.initerr > 0) || (gSHAFEData.initerr_h > 0);
#else
	return  (gSHAFEData.faultcnt > 0) || (gSHAFEData.initerr > 0);
#endif
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetFuelFault(void)
//输入参数	:  电量计AFE故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetFuelFault(void)
{
    #ifdef    BMS_USE_MAX172XX
	return ((gMax17205Var.InitErr > 0) || (gMax17205Var.IICErr >= 100));
    #else
    return 0;
    #endif
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetRTCFault(void)
//输入参数	:  void
//输出参数	:  RTC故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetRTCFault(void)
{
	return  (gRTCCond.err > 5);
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetFuelFault(void)
//输入参数	:  FLASH故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetStorageFault(void)
{
	return ((gFlashErr > 0));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetDchgMosFault(void)
//输入参数	:  放电MOS管失效故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetDchgMosFault(void)
{
	return (0 != (gMCData.mos_fault & DSG_MASK));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetChgMosFault(void)
//输入参数	:  充电MOS管失效故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetChgMosFault(void)
{
	return (0 != (gMCData.mos_fault & CHG_MASK));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetPreMosFault(void)
//输入参数	:  预放电MOS管失效故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetPreMosFault(void)
{
	return (0 != (gMCData.mos_fault & PDS_MASK));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetParaCfgFault(void)
//输入参数	:  配置参数错误故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetParaCfgFault(void)
{
	return (0 != (gStorageErr & 0x01));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetFuseFault(void)
//输入参数	:  保险丝熔断故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetFuseFault(void)
{
	return (1 == gFuseFault);
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetAccFault(void)
//输入参数	:  加速度计故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetAccFault(void)
{
    #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
	return ((gAccVar.IICErr > 0) || (gAccVar.IICErr > 10));
    #else
    return 0;
    #endif
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetPreOverTime(void)
//输入参数	:  预启动失败超时故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetPreOverTime(void)
{
	return  (0 != (gMCData.predsg_err & PREDSG_ERR_TIMEOUT_MASK));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetPreOverLoad(void)
//输入参数	:  预启动短路故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetPreOverLoad(void)
{
	return  (0 != (gMCData.predsg_err & PREDSG_ERR_SHORT_MASK));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetMosNTCFault(void)
//输入参数	:  MOS管NTC故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetMosNTCFault(void)
{
	return ((gSampleData.PCBTemp == NTC_TEMP_OFF) || (gSampleData.PCBTemp == NTC_TEMP_SHORT));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetFuelNTCFault(void)
//输入参数	:  电量计NTC故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetFuelNTCFault(void)
{
	return ((gBatteryInfo.Addit.FuelTemp == NTC_TEMP_OFF) || (gBatteryInfo.Addit.FuelTemp == NTC_TEMP_SHORT));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetConnNTCFault(void)
//输入参数	:  连接器NTC故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetConnNTCFault(void)
{
	return ((gSampleData.ConnTemp == NTC_TEMP_OFF) || (gSampleData.ConnTemp == NTC_TEMP_SHORT));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetTVSNTCFault(void)
//输入参数	:  TVS管NTC故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetTVSNTCFault(void)
{
	return ((gSampleData.TVSTemp == NTC_TEMP_OFF) || (gSampleData.TVSTemp == NTC_TEMP_SHORT));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetBat1NTCFault(void)
//输入参数	:  电池1 NTC故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetBat1NTCFault(void)
{
	return ((gBatteryInfo.Data.CellTemp[0] == NTC_TEMP_OFF) || (gBatteryInfo.Data.CellTemp[0] == NTC_TEMP_SHORT));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetBat2NTCFault(void)
//输入参数	:  电池2 NTC故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetBat2NTCFault(void)
{
	return ((gBatteryInfo.Data.CellTemp[1] == NTC_TEMP_OFF) || (gBatteryInfo.Data.CellTemp[1] == NTC_TEMP_SHORT));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetExModuleFault(void)
//输入参数	:  外置模块短路故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetExModuleFault(void)
{
	return (1 == gExModuleFault);
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetRTCValFault(void)
//输入参数	:  void
//输出参数	:  RTC未校准判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetRTCValFault(void)
{
    return  gRTCCond.calflag == 0;
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetDchgOCLock(void)
//输入参数	:  放电过流锁定故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetDchgOCLock(void)
{
	return  (0 != (gOCLock & 0x02));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetChgOCLock(void)
//输入参数	:  充电过流锁定故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetChgOCLock(void)
{
	return  (0 != (gOCLock & 0x01));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetAFEOverAlertFault(void)
//输入参数	:  AFE发生过流过压等故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetAFEOverAlertFault(void)
{
	return  (0 != (gSHAFEData.devfault & 0x01));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetAFEXRdyFault(void)
//输入参数	:  AFE发生内部XReady故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetAFEXRdyFault(void)
{
	return  (0 != (gSHAFEData.devfault & 0x02));
}

//=============================================================================================
//函数名称	:  s16 DTCheckGetSleepOCFault(void)
//输入参数	:  睡眠电流过流故障判断条件
//函数功能	:
//注意事项	:
//=============================================================================================
s16 DTCheckGetSleepOCFault(void)
{
    return 0;//gSleepCheckData.res & SLPPRO_OC_MASK;
}

/*****************************************end of DTCheck.c*****************************************/
