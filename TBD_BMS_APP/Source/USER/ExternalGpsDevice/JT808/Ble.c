
/*
 * Copyright (c) 2020-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 * 
 * Ble对象实现文件
 */

#include "Common.h"
#include "Ble.h"
#include "BleTpu.h"
#include "TlvIn.h"
#include "jt808.h"
#include "nvdsuser.h"
#include "BatteryInfoGetSetApi.h"

#ifdef CANBUS_MODE_JT808_ENABLE	

Ble g_Ble;
static BleTpu g_BleTpu;
extern JT808* g_pJt;
extern PdoInfo	g_pdoInfo;

#ifdef BMS_DEBUG_ENABLE
#define		_BLE_DEBUGMSG(fmt,...)		printf(fmt,##__VA_ARGS__)
#else
#define		_BLE_DEBUGMSG(fmt,...)
#endif 



UTP_EVENT_RC Ble_getSelfTestResult(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		SelfTestResult*  pResult = (SelfTestResult*)pCmd->pExt->transferData;
		//pResult的结构赋值
		pResult->simState = 1;	//SIM卡状态 -- 默认一直在
		pResult->gprsState = (g_pJt->devState.cnt & _NETWORK_CONNECTION_BIT)?1:0;
		pResult->gpsState = (g_Jt.devState.cnt & _GPS_FIXE_BIT)?1:0;
		pResult->devState = g_pdoInfo.isRemoteAccOn ;
		pResult->_18650Vol = 0;
		pResult->devState2 = g_cfgInfo.isActive;
		pResult->batVerify = BatteryInfoIsBattAuth();
		pResult->periheral = 0x00;
		pCmd->pExt->transferLen = sizeof(SelfTestResult);	// 实际上已经符值了
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC Ble_getBatteryInfo(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if ( ev == UTP_GET_RSP )
	{
		pCmd->pExt->transferLen = 0 ;
		//输入数据
		if( pCmd->pStorage[0] != 0 )
		{
			return (UTP_EVENT_RC)0x0C;
		}
		else
		{
			//BatteryDesc
			BatteryDesc*  pResult = (BatteryDesc*)pCmd->pExt->transferData;
			//memcpy( pResult->serialNum , (uint8_t*)BatteryInfoGetBID(), SERIAL_NUM_SIZE );
			{
				uint8 bid[BMS_ID_SIZE] = {0x00};
				memcpy(bid , (uint8*)BatteryInfoGetBID(), SERIAL_NUM_SIZE );
				pResult->serialNum[0] = bid[1];
				pResult->serialNum[1] = bid[0];
				pResult->serialNum[2] = bid[3];
				pResult->serialNum[3] = bid[2];
				pResult->serialNum[4] = bid[5];
				pResult->serialNum[5] = bid[4];
			}
			pResult->portId = 0;
			pResult->SOC = BatteryInfoGetSOC()/10;
			pResult->voltage = BatteryInfoGetPackVoltage();
			pResult->current = BatteryInfoGetPackCurrent();
			pResult->temp = BatteryInfoBatteryTemp(0)/10;
			pResult->fault = BatteryInfoGetBattFault(0)&0xFF;
			pResult->damage = BatteryInfoGetBattFault(0) >> 8;
			pResult->cycleCount = BatteryInfoGetCycle();

			pCmd->pExt->transferLen = sizeof(BatteryDesc);
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC Ble_authent(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		pCmd->pExt->transferLen = 0;
		if (BLE_USER_INVALID == BleUser_login(&pBle->user, (char*)pCmd->pStorage))
		{
			return (UTP_EVENT_RC)ERR_USERID_INVALID;
		}
	}

	return UTP_EVENT_RC_SUCCESS;
}
extern uint8 		gCurServerIndex;
extern void updateServerAddr( uint8_t serindex );
extern void JT808_CheckResetSerAddr(void);

static TlvInEventRc _SetServerAddr(void* pObj, const struct _TlvIn* pItem, TlvInEvent ev)
{
	if( ev == TE_CHANGED_AFTER )
	{
		updateServerAddr( gCurServerIndex );
		JT808_CheckResetSerAddr();
	}
	return TERC_SUCCESS ;
}

UTP_EVENT_RC Ble_setNvds(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	_BLE_DEBUGMSG("setNvds:%x,ev:%d\r\n" , pCmd->pStorage[0] , ev );
	if (ev == UTP_GET_RSP)
	{
		const static TlvIn tlvs[] =
		{
			//{"SmtFwVer", 0x01, 7, Null, DT_STRUCT},
			//{"PmsFwVer", 0x02, 7, Null, DT_STRUCT},
			//{"SmtHwVer", 0x04, 2, Null, DT_STRUCT},
			{"SerAddr", 0x10, 1, (uint8_t*)&gCurServerIndex, DT_UINT8 ,(TlvInEventFn)_SetServerAddr }
		};

		for (int i = 0; i < GET_ELEMENT_COUNT(tlvs); i++)
		{
			if (tlvs[i].tag == pCmd->pStorage[0])
			{
				if (tlvs[i].storage)
				{
					if( tlvs[i].Event ) tlvs[i].Event( pBle,&tlvs[i],TE_CHANGED_BEFORE);
					memcpy(tlvs[i].storage, &pCmd->pStorage[2], Min(pCmd->pStorage[1], tlvs[i].len));
					if( tlvs[i].Event ) tlvs[i].Event( pBle,&tlvs[i],TE_CHANGED_AFTER);
				}
			}
		}
		pCmd->pExt->transferLen = 0;
	}
	return UTP_EVENT_RC_SUCCESS;
}
UTP_EVENT_RC Ble_getNvds(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	_BLE_DEBUGMSG("getNvds:%x,ev:%d:%d\r\n" , pCmd->pStorage[0] , ev , gCurServerIndex );
	if (ev == UTP_GET_RSP)
	{
		pCmd->pExt->transferLen = 0;
		if ( 0x10 == pCmd->pStorage[0] )
		{
			otvItem*  pResult = (otvItem*)pCmd->pExt->transferData;
			pResult->item = 0x10;
			pResult->len = 0x01 ;
			pResult->param = gCurServerIndex;
			pCmd->pExt->transferLen = 3;
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}



//检验命令是否允许执行
static Bool Ble_cmdIsAllow(Ble* pBle, uint8 cmd)
{
	//定义需要用户身份认证的命令码
	struct
	{
		BLE_USER_ROLE role;
		uint8 cmd;
	}
	static const cmdRoleMatch[] = 
	{
		{BLE_USER_ADMIN, REQ_ID_SET_NVDS},
		{BLE_USER_ADMIN, REQ_ID_ACTIVE_REQ},
		{BLE_USER_ADMIN, REQ_ID_BAT_VERIFY},
		{BLE_USER_ADMIN, REQ_ID_SET_ALARM_MODE},
	};

	for (int i = 0; i < GET_ELEMENT_COUNT(cmdRoleMatch); i++)
	{
		if (cmdRoleMatch[i].cmd == cmd)
		{
			return cmdRoleMatch[i].role & pBle->user.role;
		}
	}

	//默认允许
	return True;
}

UTP_EVENT_RC Ble_getGpsGprsInfo(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_GET_RSP)
	{
		GpsPkt*  pResult = (GpsPkt*)pCmd->pExt->transferData;
		//pResult的结构赋值
		pResult->CSQ = g_Jt.devState.csq;
		pResult->Satellites = g_Jt.devState.siv;
		pResult->SNR = g_Jt.devState.snr;
		if( g_Jt.devState.cnt & _GPS_FIXE_BIT )
		{
			pResult->longitude = g_Jt.locatData.longitude;
			pResult->latitude = g_Jt.locatData.latitude;
		}
		else
		{
			pResult->longitude = 0 ;
			pResult->latitude = 0 ;
		}
		pResult->speed = 0;
		pCmd->pExt->transferLen = sizeof( GpsPkt);	// 已经符值了
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC Ble_utpEventCb(Ble* pBle, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	_BLE_DEBUGMSG("BLE Event:%s:%d\r\n",pCmd->cmdName , ev );
	
	if (ev == UTP_GET_RSP)
	{
		//判断命令是否允许
		if (!Ble_cmdIsAllow(pBle, pCmd->cmd)) return (UTP_EVENT_RC)ERR_CMD_NOT_ALLOW;

		switch( pCmd->cmd )
		{
			case REQ_ID_GET_GPS_GPRS_INFO :
				
				break ;
			case REQ_ID_RESET :
				SoftReset();		// 重启
				break ;
		}
		
		//if (pCmd->cmd == REQ_ID_GET_PORT_STATE)
		//{
		//	uint8 port = *pCmd->pStorage;
		//	if (port >= MAX_BAT_COUNT) return ERR_PARA_INVALID;

		//	pCmd->pExt->transferData = (uint8*)&g_Ble.batDesc[port];
		//	pCmd->pExt->transferLen = sizeof(BatteryDesc);
		//}
		//else if (pCmd->cmd == REQ_ID_RESET)
		//{
			//Mcu reset, To do... 
		//}
	}

	return UTP_EVENT_RC_SUCCESS;
}

uint8* Ble_ReqProc(const uint8_t* pReq, int frameLen, uint8* rspLen)
{
	if( g_BleTpu.cfg )
	{
		return BleTpu_ReqProc(&g_BleTpu, pReq, frameLen, rspLen);
	}
	return Null ;
}

int Ble_txData(uint8_t cmd, const uint8_t* pData, uint32_t len)
{
	return len;
}
/*
0x04 --- 获取电池信息   -- REQ_ID_GET_BATTERYINFO ok
0x19 --- 获取版本信息  REQ_ID_GET_DEVICEID     -- ok
0x08 --- 获取 GPS/GPRS 信息 REQ_ID_GET_GPS_GPRS_INFO -- ok
0x07 --- 自检结果				REQ_ID_GET_TESTRESULT -- 
0x2A --- 读取设备Nvds参数  -- REQ_ID_GET_NVDS
0x29 --- 设置设备Nvds参数  -- REQ_ID_SET_NVDS
0x1A --- 读取PMS信息		  -- 不支持
0x7E --- 升级指令			 -- 不支持
0x39 --- 设置ACC			 -- 不支持
0x38 --- 设置座舱锁状态 -- 不支持
*/
void Ble_init(uint8* mac)
{
#define BLE_CMD_SIZE 9
	static UtpCmdEx g_JtCmdEx[BLE_CMD_SIZE];
	static uint8 rxBuf[32];
	static uint8 txBuf[32];
	
	static const UtpCmd g_UtpCmds[BLE_CMD_SIZE] =
	{
		{&g_JtCmdEx[0],UTP_EVENT, REQ_ID_AUTHR			 , "Authr"       , rxBuf  , sizeof(rxBuf),txBuf,sizeof(txBuf),(UtpEventFn)Ble_authent},
		{&g_JtCmdEx[1],UTP_EVENT, REQ_ID_GET_DEVICEID    , "GetDevID"    , Null, 0, (uint8*)&g_Ble.devIdPkt  , sizeof(BleGetDevIDPkt)},
		{&g_JtCmdEx[2],UTP_EVENT, REQ_ID_GET_PORT_STATE  , "GetPortState",Null, 0, (uint8*)&g_Ble.portState , sizeof(PmsPortStatePkt)},
		{&g_JtCmdEx[3],UTP_EVENT, REQ_ID_GET_BATTERYINFO , "GetBatInfo"  , (uint8*)&rxBuf, sizeof(rxBuf),txBuf,sizeof(BatteryDesc),(UtpEventFn)Ble_getBatteryInfo},
		{&g_JtCmdEx[4],UTP_EVENT, REQ_ID_GET_TESTRESULT  , "GetSelfTest" , Null, 0, txBuf, sizeof(SelfTestResult), (UtpEventFn)Ble_getSelfTestResult},
		{&g_JtCmdEx[5],UTP_EVENT, REQ_ID_GET_GPS_GPRS_INFO,"GetGpsInfo"  , Null, 0, (uint8*)& g_Ble.gpsPkt, sizeof(GpsPkt),(UtpEventFn)Ble_getGpsGprsInfo},
		{&g_JtCmdEx[6],UTP_WRITE, REQ_ID_SET_NVDS 			,"SetNvds", (uint8*)&rxBuf, sizeof(rxBuf), Null, 0, (UtpEventFn)Ble_setNvds},
		{&g_JtCmdEx[7],UTP_EVENT, REQ_ID_GET_NVDS		  ,"GetNvds"     , (uint8*)&rxBuf, sizeof(rxBuf), txBuf,sizeof(txBuf),(UtpEventFn)Ble_getNvds},
		//{&g_JtCmdEx[5],UTP_EVENT, REQ_ID_GET_PMS_INFO	  ,"GetPmsInfo"  , Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		//{&g_JtCmdEx[8],UTP_EVENT, REQ_ID_SET_FACTORY_SETTINGS ,"RecoverFactoryCfg", Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		//{&g_JtCmdEx[10],UTP_EVENT, REQ_ID_ACTIVE_REQ	  ,"DevActive"   , Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		//{&g_JtCmdEx[11],UTP_EVENT, REQ_ID_BAT_VERIFY	  ,"BatVerify"   , Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		//{&g_JtCmdEx[12],UTP_EVENT, REQ_ID_SET_ALARM_MODE,"SetAlarmMode", Null, 0, (uint8*)& g_Ble.pmsPkt, sizeof(PmsPkt)},
		{&g_JtCmdEx[8],UTP_EVENT, REQ_ID_RESET	  			,"DevReset"},
	};
	//
	static const UtpCfg cfg =
	{
		.cmdCount = BLE_CMD_SIZE,
		.cmdArray = g_UtpCmds,
		.TxFn = Ble_txData,
		.TresferEvent = (UtpEventFn)Ble_utpEventCb,
		.pCbObj = &g_Ble,
	};

	static uint8_t g_JtUtp_txBuff[32];			//发送数据缓冲区
	static uint8_t g_JtUtp_rxBuff[32];			//接收数据缓冲区
	static const BleTpuFrameCfg g_BleFrameCfg =
	{
		//帧特征配置
		.cmdByteInd = 0,
		.dataByteInd = 2,

		//分配协议缓冲区
		.txBufLen = sizeof(g_JtUtp_txBuff),
		.txBuf = g_JtUtp_txBuff,
		.rxBufLen = sizeof(g_JtUtp_rxBuff),
		.rxBuf = g_JtUtp_rxBuff,

		//返回码定义
		.result_SUCCESS = SUCCESS,
		.result_UNSUPPORTED = ERR_UNSUPPORTED,
	};

	//初始化g_Ble.portState变量
	g_Ble.portState.portState = 0x01;
	g_Ble.portState.portCount = 2;
	g_Ble.portState.property[0].portNum = 0;
	g_Ble.portState.property[0].nominalCur = BatteryInfoGetRateCurrent();
	g_Ble.portState.property[0].nominalVol = BatteryInfoGetRateVoltage();	
	g_Ble.portState.property[1].portNum = 1;
	
	g_Ble.batDesc[0].portId = 0;
	g_Ble.batDesc[1].portId = 1;

	g_Ble.pmsPkt.Version = g_Ble.devIdPkt.protocolVer = 1;
	g_Ble.pmsPkt.HwMainVer = g_Ble.devIdPkt.hwMainVer = gHwVerRead[0];//= HW_VER_MAIN;
	g_Ble.pmsPkt.HwSubVer = g_Ble.devIdPkt.hwSubVer = gHwVerRead[1];//= HW_VER_SUB;
	g_Ble.pmsPkt.AppMainVer = g_Ble.devIdPkt.fwMainVer = cAPPInfo.swmain;//FW_VER_MAIN;
	g_Ble.pmsPkt.AppSubVer = g_Ble.devIdPkt.fwSubVer = cAPPInfo.swsub1;//FW_VER_S1;
	g_Ble.pmsPkt.AppMinorVer = g_Ble.devIdPkt.fwMinorVer = cAPPInfo.swsub2;//FW_VER_S2;
	g_Ble.pmsPkt.AppBuildNum = g_Ble.devIdPkt.buildNum = ( cAPPInfo.swbuild[0] << 24 ) |
														( cAPPInfo.swbuild[1] << 16 ) |
														( cAPPInfo.swbuild[2] << 8 ) |
														( cAPPInfo.swbuild[3] << 0 ) ;
	g_Ble.pmsPkt.State = 1;
	
	BleTpu_Init(&g_BleTpu, &cfg, &g_BleFrameCfg);
	BleUser_init(&g_Ble.user, mac);
}

#endif //


