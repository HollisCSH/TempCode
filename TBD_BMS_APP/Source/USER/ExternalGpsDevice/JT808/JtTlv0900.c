
#include "Common.h"
#include "JtTlv0900.h"
#include "TlvOut.h"
#include "NvdsUser.h"
//#include "Pms.h"
#include "BatteryInfoGetSetApi.h"

#include "bsptypedef.h"

#ifdef CANBUS_MODE_JT808_ENABLE	


static TlvOutMgr g_jtTlvMgr_0900;


//static uint8	g_smartState = 0x01;
//static uint8	g_smartState_mirror;


//static TlvPMSState	g_tlvPMSState[MAX_BAT_COUNT] = {0x01,0x02};	// PMS运行参数
//static TlvPMSState g_tlvPMSState_mirror[MAX_BAT_COUNT];


static TlvBatInfo g_tlvBatInfo[MAX_BAT_COUNT];	// 电池基本信息
static TlvBatInfo g_tlvBatInfo_mirror[MAX_BAT_COUNT];

static TlvBatWorkInfo g_tlvBatWorkInfo[MAX_BAT_COUNT];	// 电池工作参数
static TlvBatWorkInfo g_tlvBatWorkInfo_mirror[MAX_BAT_COUNT];

static TlvBatTemp g_tlvBatTemp[MAX_BAT_COUNT];	// 电池温度数据
static TlvBatTemp g_tlvBatTemp_mirror[MAX_BAT_COUNT];

static TlvBatFault g_tlvBatFault[MAX_BAT_COUNT];	// 电池故障数据
static TlvBatFault g_tlvBatFault_mirror[MAX_BAT_COUNT];

static uint8 	gCacheBuff[256] = {0x00};
static uint8	gCacheLen = 0 ;

void JtTlv0900_updateBatInfo(uint8 port)	// 更新电池基本信息
{
	TlvBatInfo* pBat = Null;
	uint16 val = 0 ;
	if( port != 0 ) return ;	
	pBat = &g_tlvBatInfo[port];
	uint8 bid[BMS_ID_SIZE] = {0x00};
	memcpy(bid , (uint8*)BatteryInfoGetBID(), 6 );
	pBat->bid[0] = bid[1];
	pBat->bid[1] = bid[0];
	pBat->bid[2] = bid[3];
	pBat->bid[3] = bid[2];
	pBat->bid[4] = bid[5];
	pBat->bid[5] = bid[4];
	//memcpy( pBat->bid , (uint8*)BatteryInfoGetBID(), 6 );
	// 为方便服务处理
	
	pBat->port = port ;
	val  = BatteryInfoGetRateCurrent();
	pBat->nominalCur = SWAP16(val);
	val = BatteryInfoGetRateVoltage();
	pBat->nominalVol = SWAP16(val);
	//
	pBat->HwMainVer = gHwVerRead[0];
	pBat->HwSubVer = gHwVerRead[1];
	pBat->FwMainVer = cAPPInfo.swmain;
	pBat->FwSubVer = cAPPInfo.swsub1;
	pBat->FwRevVer = cAPPInfo.swsub2;
	pBat->FwBuildNunber = 
		( cAPPInfo.swbuild[0] << 24 ) |
		( cAPPInfo.swbuild[1] << 16 ) |
		( cAPPInfo.swbuild[2] << 8 ) |
		( cAPPInfo.swbuild[3] << 0 ) ;
}


void JtTlv0900_updateBatWorkInfo(uint8 port)	// 更新工作数据
{
	TlvBatWorkInfo *pBatWorkInfo = Null ;
	uint16 val = 0 ;
	if( port != 0 ) return ;
	//
	pBatWorkInfo = &g_tlvBatWorkInfo[port];
	uint8 bid[BMS_ID_SIZE] = {0x00};
	memcpy(bid , (uint8*)BatteryInfoGetBID(), 6 );
	pBatWorkInfo->bid[0] = bid[1];
	pBatWorkInfo->bid[1] = bid[0];
	pBatWorkInfo->bid[2] = bid[3];
	pBatWorkInfo->bid[3] = bid[2];
	pBatWorkInfo->bid[4] = bid[5];
	pBatWorkInfo->bid[5] = bid[4];
	//memcpy( pBatWorkInfo->bid , (uint8*)BatteryInfoGetBID(), 6 );
	// 为方便服务处理
	
	pBatWorkInfo->soc = BatteryInfoGetSOC()/10;
	
	val = BatteryInfoGetPackVoltage();
	pBatWorkInfo->voltage = SWAP16(val);
	
	val = BatteryInfoGetPackCurrent();
	pBatWorkInfo->current = SWAP16(val);//(SWAP16(val&0x8000)) | (val&0x8000);
	
	val = BatteryInfoGetMaxCellVolt();
	pBatWorkInfo->maxCellVol = SWAP16(val);
	
	val = BatteryInfoGetMinCellVolt();
	pBatWorkInfo->minCellVol = SWAP16(val);
	
	//val = BatteryInfoGetMaxCellNum();
	pBatWorkInfo->maxCellNum = BatteryInfoGetMaxCellNum();
	
	//val = BatteryInfoGetMinCellNum();
	pBatWorkInfo->minCellNum = BatteryInfoGetMinCellNum();
	
	val = BatteryInfoGetRecentMaxChgCurr() + 30000;
	pBatWorkInfo->maxChgCurr = SWAP16(val);
	
	val = BatteryInfoGetRecentMaxDchgCurr() + 30000;
	pBatWorkInfo->maxDischgCurr = SWAP16(val);
	
	val = BatteryInfoGetHighCurrDet();
	pBatWorkInfo->curWorkFeature = SWAP16(val);
	
	val = BatteryInfoGetCycle();
	pBatWorkInfo->cycCount = SWAP16(val);
	
}

void JtTlv0900_updateBatTemperature(uint8 port) // 更新温度数据
{
	TlvBatTemp *pBatTemp = Null ;
	uint16 val = 0 ;
	if( port != 0 ) return ;
	pBatTemp = &g_tlvBatTemp[port];
	//
	uint8 bid[BMS_ID_SIZE] = {0x00};
	memcpy(bid , (uint8*)BatteryInfoGetBID(), 6 );
	pBatTemp->bid[0] = bid[1];
	pBatTemp->bid[1] = bid[0];
	pBatTemp->bid[2] = bid[3];
	pBatTemp->bid[3] = bid[2];
	pBatTemp->bid[4] = bid[5];
	pBatTemp->bid[5] = bid[4];
	//memcpy( pBatTemp->bid , (uint8*)BatteryInfoGetBID(), 6 );
	// 为方便服务处理


	val = BatteryInfoGetChgMosTemp() + 400;
	pBatTemp->cMostTemp = SWAP16(val);
	
	val = BatteryInfoGetDchgMosTemp() + 400;
	pBatTemp->dMostTemp = SWAP16(val);
	
	val = BatteryInfoGetFuelTemp() + 400;
	pBatTemp->fuelTemp = SWAP16(val);
	
	val = BatteryInfoGetConnTemp() + 400;
	pBatTemp->contTemp = SWAP16(val);
	
	val = BatteryInfoBatteryTemp(0) + 400;
	pBatTemp->batTemp1 = SWAP16(val);
	
	val = BatteryInfoBatteryTemp(1) + 400;
	pBatTemp->batTemp2 = SWAP16(val);
	
	val = BatteryInfoGetTVSTemp() + 400;
	pBatTemp->tvsTemp = SWAP16(val);
	
	
	val = BatteryInfoGetFuseTemp();
	pBatTemp->fuseTemp = SWAP16(val);
}


void JtTlv0900_updateBatFault(uint8 port)
{
	TlvBatFault *pBatFault = Null;
	uint16 val = 0 ;
	if( port != 0 ) return ;
	pBatFault = &g_tlvBatFault[port];

	uint8 bid[BMS_ID_SIZE] = {0x00};
	memcpy(bid , (uint8*)BatteryInfoGetBID(), 6 );
	pBatFault->bid[0] = bid[1];
	pBatFault->bid[1] = bid[0];
	pBatFault->bid[2] = bid[3];
	pBatFault->bid[3] = bid[2];
	pBatFault->bid[4] = bid[5];
	pBatFault->bid[5] = bid[4];
	
	//memcpy( pBatFault->bid ,(uint8*)BatteryInfoGetBID(), 6 );
	// 为方便服务处理
	val = BatteryInfoGetBattFault(0);
	pBatFault->devft1 = SWAP16(val);
	
	val = BatteryInfoGetBattFault(1);
	pBatFault->devft2 = SWAP16(val);
	
	val = BatteryInfoGetBattFault(2);
	pBatFault->opft1 = SWAP16(val);
		
	val = BatteryInfoGetBattFault(3);
	pBatFault->opft2 = SWAP16(val);
	
	val = BatteryInfoGetBattAlarm(0);
	pBatFault->opwarn1 = SWAP16(val);
	
	val = BatteryInfoGetBattAlarm(1);
	pBatFault->opwarn2 = SWAP16(val);
	
}

void JtTlv0900_updateStorage(void)
{
	JtTlv0900_updateBatInfo(0);
	JtTlv0900_updateBatWorkInfo(0);
	JtTlv0900_updateBatTemperature(0);
	JtTlv0900_updateBatFault(0);
}

//void BeepCtrl(  uint16  freq , uint8 duty , uint16 ontim , uint16 offtim , uint16 cycles )
static Bool BatteryWorkInfoChange( void* pObj, const struct _TlvOut* pItem, TlvEvent ev )
{
    extern void BeepCtrl(  uint16  freq , uint8 duty , uint16 ontim , uint16 offtim , uint16 cycles );
	TlvBatWorkInfo *pCurBatWorkInfo , *pLastBatWorkInfo ;
	if( ev == TE_CHANGED )
	{
		pCurBatWorkInfo = (TlvBatWorkInfo*)pItem->storage ;
		pLastBatWorkInfo = (TlvBatWorkInfo*)pItem->mirror ;
		//首次小于30% or 20%时
		if( pCurBatWorkInfo->soc <= 30 && pLastBatWorkInfo->soc > 30 )
		{
			BeepCtrl(23,34,5000,1000,5);
		}
		else if( pCurBatWorkInfo->soc <= 20 && pLastBatWorkInfo->soc > 20 )
		{
			BeepCtrl(23,34,2000,1000,10);
		}
	}
	return ev == TE_CHANGED ;
}



//void JtTlv0900_updateMirror(const uint8* data, int len)
//{
//	TlvOutMgr_updateMirror(&g_jtTlvMgr_0900, data, len);
//}

void JtTlv0900_updateMirror(void)
{
	if( gCacheLen )
	{
		TlvOutMgr_updateMirror(&g_jtTlvMgr_0900, gCacheBuff, gCacheLen );
		gCacheLen = 0x00 ;
	}
}

void JtTlv0900_Cache( uint8* pdata ,uint8 len )	// 获取缓存数据
{
	//static uint8 	gCacheBuff[256] = {0x00};
	//static uint8	gCacheLen = 0 ;
	gCacheLen = len > sizeof(gCacheBuff) ? sizeof(gCacheBuff) : len ;
	memcpy( gCacheBuff , pdata , gCacheLen );
}





int JtTlv0900_getChangedTlv(uint8* buf, int len, uint8* tlvCount)
{
	return TlvOutMgr_getChanged(&g_jtTlvMgr_0900, buf, len, tlvCount);
}

void JtTlv0900_init()
{
#define TLV_OUT_COUNT	4
	static TlvOutEx g_tlvBuf_0900Ex[TLV_OUT_COUNT];
	static const TlvOut g_tlv_0900[TLV_OUT_COUNT] =
	{
		//{"SMART", &g_tlvBuf_0900Ex[0], TAG_SMART_STATE ,  sizeof(g_smartState), &g_smartState, DT_UINT8, &g_smartState_mirror, Null, 0, 0},
		//{"PMS_STATE", &g_tlvBuf_0900Ex[1], TAG_PMS_STATE , MAX_BAT_COUNT * sizeof(TlvPMSState), &g_tlvPMSState[0], DT_STRUCT, &g_tlvPMSState_mirror[0], Null, 0, 0},
		//
		{"BAT_INFO", &g_tlvBuf_0900Ex[0], TAG_BAT_INFO		  , MAX_BAT_COUNT * sizeof(TlvBatInfo), &g_tlvBatInfo[0]    , DT_STRUCT, &g_tlvBatInfo_mirror[0] , Null, 0, 0},
		{"BAT_WORK", &g_tlvBuf_0900Ex[1], TAG_BAT_WORK_PARAM , MAX_BAT_COUNT * sizeof(TlvBatWorkInfo), &g_tlvBatWorkInfo[0], DT_STRUCT, &g_tlvBatWorkInfo_mirror[0], BatteryWorkInfoChange, 0, 0},
		//
		{"BAT_TEMP"	, &g_tlvBuf_0900Ex[2], TAG_BAT_TEMP , MAX_BAT_COUNT * sizeof(TlvBatTemp), &g_tlvBatTemp[0], DT_STRUCT, &g_tlvBatTemp_mirror[0], Null, 0, 0},
		{"BAT_FAULT", &g_tlvBuf_0900Ex[3], TAG_BAT_FAULT , MAX_BAT_COUNT * sizeof(TlvBatFault), &g_tlvBatFault[0], DT_STRUCT, &g_tlvBatFault_mirror[0], Null, 0, 0},
	};

	TlvOutMgr_init(&g_jtTlvMgr_0900, g_tlv_0900, GET_ELEMENT_COUNT(g_tlv_0900), 1, True);
}


#endif //


