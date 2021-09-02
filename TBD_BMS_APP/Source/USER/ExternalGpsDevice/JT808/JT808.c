


#include "Common.h"
#include "JT808.h"
#include "Obj.h"
#include "MsgDef.h"
#include "JtUtp.h"
#include "JtTlv0900.h"
#include "JtTlv8900.h"
#include "JtTlv8103.h"
//#include "status.h"
#include "queue.h"
//#include "flexcan_driver.h"
#include "stm32g0xx_hal.h"
#include "Ble.h"
//#include "BSPCan.h"
#include "BSP_CAN.h"
#include "PT.h"
#include "BatteryInfoGetSetApi.h"
#include "bsptypedef.h"
#include "BSPTask.h"
#include "CanComm.h"
#define  BMS_DEBUG_ENABLE
#ifdef CANBUS_MODE_JT808_ENABLE	


JT808 g_Jt;
JT808* g_pJt = &g_Jt;
static Utp g_JtUtp;
//static uint32_t g_hbIntervalMs = 2000;	//MCU心跳时间间隔，单位Ms
static uint8_t g_txBuf[100];
static uint8_t g_txlen = 0;
uint8_t g_hbdata[4] = {0x00, 0x00, 0x07, 0xD0};

// Jose add
static JtDevBleCfgParam g_BleCfgParam_mirror ;
static uint8_t gSendCanCmdCnt = 0 ;	// 发送计数---连接失败多次,则重启之...
static uint32_t gCanbusRevTimeMS = 0;
#define			_RESEND_CMD_COUNT		(2)	// 重发3

// GPS时间---用以记录 GPS定位用时时间
static uint32 	gGPSTimeCnt ;		// 
static uint32 	gGPSFixCnt;			// GPS 定时时间
// GPRS时间---用以记录GPRS从发送到接收时间
static uint32 	gGPRSSendTimeCnt;
static uint32 	gGPRSConTimeCnt ;		// 连接用时时间
static uint32	gGPRSConCnt;			// GPRS 连接时间
// sleep 
static uint32  gModuleSleepIngTimeoutCnt ;		// 模块请求进休眠超时计时
//static uint32  gModuleSleepCnt;					// 模块休眠计时
// wakeup 
static uint32  gModuleWakupIngTimeoutCnt;		// 模块请求wakeup超时计时
static uint32  gModuleWakeupCnt;				// 模块唤醒计时

uint16_t gCurRevLen = 0 ;



//flexcan_msgbuff_t
#define			_CAN_BUS_REV_TIMEOUT_MS			(1000*25)	// 35s
extern u8 can0_receive_flag ;
extern FDCAN_RxHeaderTypeDef gCanRxData;

#ifdef BMS_DEBUG_ENABLE
#define		_JT808_DEBUGMSG(fmt,...)		printf(fmt,##__VA_ARGS__)
#else
#define		_JT808_DEBUGMSG(fmt,...)
#endif 

static void _UpdataBleAdvData(uint8_t mac[6]);

//static void _ShowSleepWakeup(void);


#define		_MODULE_SET_GPRS_STATE		do{g_Jt.devState.cnt |= _NETWORK_CONNECTION_BIT ;}while(0)	
#define		_MODULE_CLR_GPRS_STATE		do{g_Jt.devState.cnt &= ~_NETWORK_CONNECTION_BIT ;}while(0)
#define		_MODULE_SET_GPS_STATE		do{g_Jt.devState.cnt |= _GPS_FIXE_BIT ;}while(0)
#define		_MODULE_CLR_GPS_STATE		do{g_Jt.devState.cnt &= ~_GPS_FIXE_BIT ;}while(0)


void JT808_fsm(uint8_t msgID, uint32_t param1, uint32_t param2);

static JT808fsmFn JT808_findFsm(JT_state state);

static void _SetOperationState(uint8_t Operation, uint8_t Parameter );


//static SwTimer gModuleHWResetTimer ;		// 外置模块电源重启
//static SwTimer gCANResetTimer ;				// 外置模块CAN重启
pt gPTJT808;

/*
7E B6 01 00 01 01 11 40 00 00 01 00 00 00 01 00 00 00 00 00 54 4D 4B 4A 2D 53 5A 52 30 31 2D 44 31 30 00 00 00 00 00 00 41 32 30 33 41 30 39 30 30 30 30 31 89 86 04 70 19 20 70 47 84 99 04 4D 41 30 31 04 41 31 30 32 7E 


7E B6 01 00 01 01 11 40 
00 
00 01 
00 00 00 01 
00 00 00 00 00 
54 4D 4B 4A 2D 53 5A 52 30 31 2D 44 31 30 00 00 00 00 00 00 
41 32 30 33 41 30 39 30 30 30 30 31 
89 86 04 70 19 20 70 47 84 99 
04 4D 41 30 31 
04 41 31 30 32 
7E 
*/
UTP_EVENT_RC JT808_cmd_getSimID(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	int headSize = sizeof(JtDevProperty) - JT_DEV_HW_VER_SIZE - JT_DEV_FW_VER_SIZE;
	if (ev == UTP_CHANGED_AFTER )
	{
		uint8 ver[JT_DEV_HW_VER_SIZE + JT_DEV_FW_VER_SIZE];

		//接收到数据和property定义的结构不匹配，需要重新赋值HwVer和FwVer

		memcpy(ver, g_Jt.property.hwVer, pCmd->pExt->transferLen - headSize - 1);
		memset( g_Jt.property.hwVer, 0, JT_DEV_HW_VER_SIZE + JT_DEV_FW_VER_SIZE);

		if (ver[0] < JT_DEV_HW_VER_SIZE)
		{
			memcpy(g_Jt.property.hwVer, &ver[1], ver[0]);
		}
		else
		{
			_JT808_DEBUGMSG( "Jt808 hwVer size(%d) error.\n", ver[0]);
		}
		int offset = ver[0] + 1;

		if (ver[offset] < JT_DEV_HW_VER_SIZE)
		{
			memcpy(g_Jt.property.fwVer, &ver[offset + 1], ver[offset]);
		}
		else
		{
			_JT808_DEBUGMSG( "Jt808 fwVer size(%d) error.\n", ver[offset]);
		}

		// 大小端转换
		g_Jt.property.protocolVer = SWAP16(g_Jt.property.protocolVer);//SWAP16( &(g_Jt.property.protocolVer) , DT_UINT16 );
		g_Jt.property.devClass = SWAP32( g_Jt.property.devClass );
		//g_Jt.property.devClass = Dt_convertToU32( &(g_Jt.property.devClass ) , DT_UINT32 );
		gSendCanCmdCnt = 0 ;	// 清0

		_JT808_DEBUGMSG("SIM ID\r\n");
		_JT808_DEBUGMSG("ver:%02X\r\n",g_Jt.property.protocolVer);
		_JT808_DEBUGMSG("Calss:%d\r\n",g_Jt.property.devClass);
		_JT808_DEBUGMSG("devModel:%s\r\n",g_Jt.property.devModel);
		_JT808_DEBUGMSG("devID:%s\r\n",g_Jt.property.devId);
		_JT808_DEBUGMSG("ICCID:%s\r\n",g_Jt.property.iccid);
		_JT808_DEBUGMSG("hwVer:%s\r\n",g_Jt.property.hwVer);
		_JT808_DEBUGMSG("fwVer:%s\r\n",g_Jt.property.fwVer);
		
	}
	else if (ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		_JT808_DEBUGMSG("Read sim param failed.\r\n" );
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			_JT808_DEBUGMSG("Resend Cmd Timeout,reboot\r\n");
			//BSPCan_Reset();
      BatteryInfoSetCanReset();
		}
		else
		{
			_JT808_DEBUGMSG("Resend Cmd :%02X[%d]\r\n",JTCMD_CMD_GET_SIM_ID,gSendCanCmdCnt);
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_ID);
		}
	}
	
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_getSimCfg(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	static int readParamOffset = 0;
	static int i = 0;
	uint16* paramIDs = (uint16*)pCmd->pExt->transferData;
	
	if (ev == UTP_TX_START)
	{
		//const TlvIn* p = &g_jtTlvInMgr_8103.itemArray[readParamOffset];
		const TlvOut *p = &g_jtTlvOutMgr_8103.itemArray[readParamOffset];
		//配置读取参数
		for (i = 0; (i + readParamOffset) < g_jtTlvOutMgr_8103.itemCount && i < 5; i++, p++)
		{
			paramIDs[i] = p->tag;
		}
		pCmd->pExt->transferLen = i * 2;
	}
	else if (ev == UTP_REQ_SUCCESS)	//读取成功
	{
		readParamOffset += i;
		JtTlv8103_updateMirror( &pCmd->pExt->transferData[1], pCmd->pExt->transferLen - 1 );
		//JtTlv8103_updateStorage(&pCmd->pExt->transferData[1], pCmd->pExt->transferLen - 1);
		if (readParamOffset < g_jtTlvOutMgr_8103.itemCount )//  g_jtTlvInMgr_8103.itemCount)
		{
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);
		}
		else //全部读取完毕
		{
			//i = 0;
			readParamOffset = 0;
			// 如果有默认设置,则重新设置,如果为远程设置，则不处理之
			if( JtTlv8103_getFactoryCofnig() != 0 )	// 非出厂设置
			{
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_SIM_CFG );
			}
		}
		i = 0 ;
		gSendCanCmdCnt = 0x00 ;
		
	}
	else if (ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		readParamOffset = 0 ;
		i = 0 ;
//		_JT808_DEBUGMSG_WARNING("Write sim param failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0 ;
			_JT808_DEBUGMSG("Resend Cmd Timeout,reboot\r\n");
			BatteryInfoSetCanReset();
		}
		else
		{
			_JT808_DEBUGMSG("Resend Cmd :%02X[%d]\r\n",JTCMD_CMD_GET_SIM_CFG,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);
	}

	return UTP_EVENT_RC_SUCCESS;
}


// Jose add 2020/9/17


UTP_EVENT_RC JT808_cmd_setSimCfg(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_TX_START )
	{
		uint8_t tlvCount = 0 ;
		//pCmd->pExt->transferLen = JtTlv8103_getChanged( (uint8_t*)pCmd->pExt->transferData ,  pCmd->storageLen , &tlvCount );
		if( JtTlv8103_getFactoryCofnig() != 0 )	// 锟角筹拷锟斤拷锟斤拷锟斤拷
		{
			pCmd->pExt->transferLen = JtTlv8103_getChanged( (uint8_t*)pCmd->pExt->transferData ,  pCmd->storageLen , &tlvCount );
		}
		else
		{
			pCmd->pExt->transferLen = JtTlv8103_getDefChanged( (uint8_t*)pCmd->pExt->transferData ,  pCmd->storageLen ) ;
		}
	}
	else if( ev == UTP_REQ_SUCCESS )
	{
		gSendCanCmdCnt = 0x00 ;
		gGPRSConTimeCnt = GET_TICKS();		// 开始计时
	}
	else if ( ev == UTP_REQ_FAILED  )	//读取失败，重新读取
	{
		_JT808_DEBUGMSG("Write sim CFG failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0x00;
			_JT808_DEBUGMSG("Resend Cmd Timeout,reboot\r\n");
			BatteryInfoSetCanReset();
		}
		else
		{
			_JT808_DEBUGMSG("Resend Cmd :%02X[%d]\r\n",JTCMD_CMD_SET_SIM_CFG,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_SIM_CFG);
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_setBleCfg(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_TX_START )
	{
		// 发送前，临时改变
		pJt->blecfgParam.BleAdvInterval = SWAP32(pJt->blecfgParam.BleAdvInterval);
	}
	else if( ev == UTP_TX_DONE )
	{
		// 发送后,恢复
		pJt->blecfgParam.BleAdvInterval = SWAP32(pJt->blecfgParam.BleAdvInterval);
	}
	else if( ev == UTP_REQ_SUCCESS )
	{
		gSendCanCmdCnt = 0x00 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		_JT808_DEBUGMSG("Write BLE CFG failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0 ;
			_JT808_DEBUGMSG("Resend Cmd Timeout,reboot\r\n");
			BatteryInfoSetCanReset();
		}
		else
		{
			_JT808_DEBUGMSG("Resend Cmd :%02X[%d]\r\n",JTCMD_CMD_SET_SIM_CFG,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_BLE_CFG );
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_getBleEnable(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_REQ_SUCCESS )
	{
		pJt->bleEnCtrl = SWAP16( pJt->bleEnCtrl );
		_JT808_DEBUGMSG("BLE Enable:%d\r\n",pJt->bleEnCtrl);
		if( pJt->bleEnCtrl & 0x01 )
		{
			_JT808_DEBUGMSG("Disable BLE Scan\r\n");
			pJt->bleEnCtrl = 0x00 ;	//
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_BLE_EN );
		}
		gSendCanCmdCnt = 0x00 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		_JT808_DEBUGMSG("Get BLE Enable failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0 ;
			_JT808_DEBUGMSG("Resend Cmd Timeout,reboot\r\n");
			BatteryInfoSetCanReset();
		}
		else
		{
			_JT808_DEBUGMSG("Resend Cmd :%02X[%d]\r\n",JTCMD_CMD_GET_BLE_EN,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_EN );
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_setBleEnable(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_TX_START )
	{
		// 发送前，临时改变
		pJt->bleEnCtrl = SWAP16( pJt->bleEnCtrl );
	}
	else if( ev == UTP_TX_DONE )
	{
		// 发送后,恢复
		pJt->bleEnCtrl = SWAP16( pJt->bleEnCtrl );
	}
	else if( ev == UTP_REQ_SUCCESS )
	{
		gSendCanCmdCnt = 0x00 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		_JT808_DEBUGMSG("Set BLE Enable failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0 ;
			_JT808_DEBUGMSG("Resend Cmd Timeout,reboot\r\n");
			BatteryInfoSetCanReset();
		}
		else
		{
			_JT808_DEBUGMSG("Resend Cmd :%02X[%d]\r\n",JTCMD_CMD_SET_BLE_EN,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_BLE_EN );
	}
	return UTP_EVENT_RC_SUCCESS;
}



UTP_EVENT_RC JT808_cmd_getBleID(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	int headSize = sizeof(JtDevBleProperty) - JT_DEV_BLE_HW_VER_SIZE - JT_DEV_BLE_FW_VER_SIZE;
	if (ev == UTP_REQ_SUCCESS)
	{
		uint8 ver[JT_DEV_BLE_HW_VER_SIZE + JT_DEV_BLE_FW_VER_SIZE];

		//接收到数据和property定义的结构不匹配，需要重新赋值HwVer和FwVer

		memcpy(ver, g_Jt.bleproperty.BlehwVer, pCmd->pExt->transferLen - headSize - 1);
		memset(g_Jt.bleproperty.BlehwVer, 0, JT_DEV_BLE_HW_VER_SIZE + JT_DEV_BLE_FW_VER_SIZE );

		if (ver[0] < JT_DEV_BLE_HW_VER_SIZE )
		{
			memcpy(g_Jt.bleproperty.BlehwVer, &ver[1], ver[0]);
		}
		else
		{
			_JT808_DEBUGMSG( "Jt808 BlehwVer size(%d) error.\n", ver[0]);
		}
		int offset = ver[0] + 1;

		if (ver[offset] < JT_DEV_BLE_FW_VER_SIZE )
		{
			memcpy(g_Jt.bleproperty.BlefwVer, &ver[offset + 1], ver[offset]);
		}
		else
		{
			_JT808_DEBUGMSG( "Jt808 BlefwVer size(%d) error.\n", ver[offset]);
		}
		g_Jt.bleproperty.BleType = SWAP16(g_Jt.bleproperty.BleType);//SWAP16(&g_Jt.bleproperty.BleType,DT_UINT16);
		
		gSendCanCmdCnt = 0x00 ;
		
		_JT808_DEBUGMSG("BLE:%02X[%02X%02X%02X%02X%02X%02X]\r\n",
			g_Jt.bleproperty.BleType , 
			g_Jt.bleproperty.BleMac[0],g_Jt.bleproperty.BleMac[1],
			g_Jt.bleproperty.BleMac[2],g_Jt.bleproperty.BleMac[3],
			g_Jt.bleproperty.BleMac[4],g_Jt.bleproperty.BleMac[5]);
		_JT808_DEBUGMSG("BLE Ver:%s---%s\r\n",
			g_Jt.bleproperty.BlehwVer,g_Jt.bleproperty.BlefwVer );

		Ble_init( g_Jt.bleproperty.BleMac );

		_UpdataBleAdvData( g_Jt.bleproperty.BleMac );

	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		_JT808_DEBUGMSG("Get BLE ID failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			_JT808_DEBUGMSG("Resend Cmd Timeout,reboot\r\n");
			BatteryInfoSetCanReset();
		}
		else
		{
			_JT808_DEBUGMSG("Resend Cmd :%02X[%d]\r\n", JTCMD_CMD_GET_BLE_ID,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp,  JTCMD_CMD_GET_BLE_ID );
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC JT808_cmd_getBleCfg(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_TX_START)		// 上报
	{
		//
	}
	else if (ev == UTP_REQ_SUCCESS)	//读取成功
	{
		g_BleCfgParam_mirror.BleAdvInterval = SWAP32(g_BleCfgParam_mirror.BleAdvInterval);//Dt_convertToU32(&g_BleCfgParam_mirror.BleAdvInterval,DT_UINT32);
		//g_BleCfgParam_mirror

		//_JT808_DEBUGMSG("BLE\r\nName:%s",
		//	g_BleCfgParam_mirror.BleName );
		_JT808_DEBUGMSG("Inter:%d:%d,Power:%d\r\n",
		g_BleCfgParam_mirror.BleAdvInterval,g_Jt.blecfgParam.BleAdvInterval,g_BleCfgParam_mirror.BleAdvPower );
		//_JT808_DEBUGMSG("Adv:%s\r\n",g_BleCfgParam_mirror.BleAdvData);
		
		// 查看是否需要设置BLE Name
		//JtDevBleCfgParam
		if( 0 != memcmp( &g_BleCfgParam_mirror , &g_Jt.blecfgParam ,sizeof(JtDevBleCfgParam) ))
		{
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SET_BLE_CFG );
			_JT808_DEBUGMSG("Update BleCfg\r\n");
		}
		gSendCanCmdCnt = 0x00;
		
	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		_JT808_DEBUGMSG("Get BLE CFG failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			_JT808_DEBUGMSG("Resend Cmd Timeout,reboot\r\n");
			BatteryInfoSetCanReset();
		}
		else
		{
			_JT808_DEBUGMSG("Resend Cmd :%02X[%d]\r\n", JTCMD_CMD_GET_BLE_CFG,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp,  JTCMD_CMD_GET_BLE_CFG );
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_getFileInfo(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
//	uint16* paramIDs = (uint16*)pCmd->pExt->transferData;
	//
	if (ev == UTP_TX_START)		// 上报
	{
		//
	}
	else if (ev == UTP_REQ_SUCCESS)	//读取成功
	{
		g_Jt.updatefileinfo.Updatefilelength = SWAP32(g_Jt.updatefileinfo.Updatefilelength);//Dt_convertToU32(&g_Jt.updatefileinfo.Updatefilelength,DT_UINT32);

		_JT808_DEBUGMSG("Update file Len:%d,Ver:%d\r\n",
			g_Jt.updatefileinfo.Updatefilelength,g_Jt.updatefileinfo.UpdatefileVersion );
		gSendCanCmdCnt = 0 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		_JT808_DEBUGMSG("Get UpdateFile Info failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			_JT808_DEBUGMSG("Resend Cmd Timeout,reboot\r\n");
			BatteryInfoSetCanReset();
		}
		else
		{
			_JT808_DEBUGMSG("Resend Cmd :%02X[%d]\r\n", JTCMD_CMD_GET_FILE_INFO,gSendCanCmdCnt);
		}
		Utp_SendCmd(&g_JtUtp,  JTCMD_CMD_GET_FILE_INFO );
	}
	return UTP_EVENT_RC_SUCCESS;
}




UTP_EVENT_RC JT808_cmd_setToOpState(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
//	static int readParamOffset = 0;
//	static int i = 0;
//	uint16* paramIDs = (uint16*)pCmd->pExt->transferData;
	//
	if (ev == UTP_TX_START) 	// 上报
	{
		//
	}
	else if (ev == UTP_REQ_SUCCESS) //设置成功
	{
		gSendCanCmdCnt = 0 ;
		//设置状态成功

		// 更新之
		switch( pJt->setToOpState.OperationState )
		{
			case JT_STATE_SLEEP:
				pJt->opState = (JT_state)pJt->setToOpState.OperationState ;
				//pJt->bleState.bleConnectState = 0x00;
				pJt->devState.cnt = 0x00 ;
//				gModuleSleepCnt = GET_TICKS();
				Utp_Reset(&g_JtUtp);		// 停止所有发送请求
				//FlexCANStanbyMode();
				break;
			case JT_STATE_WAKEUP:
				//pJt->bleState.bleConnectState = 0x00;
				pJt->devState.cnt = 0x00 ;
				gModuleWakeupCnt = GET_TICKS();
				break;
			default:break ;
		}
		_JT808_DEBUGMSG("设置OpState:%d[%d]\r\n",pJt->setToOpState.OperationState,pJt->opState );
	}
	else if ( ev == UTP_REQ_FAILED )	//读取失败，重新读取
	{
		_JT808_DEBUGMSG("Set To OpState failed.\r\n");

		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0 ;
			_JT808_DEBUGMSG("Resend Cmd Timeout,CAN Reset\r\n");
			BatteryInfoSetCanReset();
			//if( g_Jt.setToOpState.OperationState == JT_STATE_SLEEP )
			//{
				// 请求进休眠的指令超时,此时要不要重启好了
			//	_JT808_DEBUGMSG("Set Sleep Fail,reboot system\r\n");
			//	ModuleHWReset();								
			//}
		}
		else
		{
			_JT808_DEBUGMSG("Resend Cmd :%02X[%d]\r\n", JTCMD_SET_OP_STATE,gSendCanCmdCnt);
		}
		//Utp_SendCmd(&g_JtUtp,  JTCMD_SET_OP_STATE );		
		Utp_DelaySendCmd(&g_JtUtp, JTCMD_SET_OP_STATE, 500);		// 永远发送状态
		
		//Utp_DelaySendCmd(&g_JtUtp, JTCMD_MCU_HB, 2000);
	}
	return UTP_EVENT_RC_SUCCESS;
}



void JT808_switchState(JT808* pJt, JT_state newState)
{
	if (pJt->opState == newState) return;
	//
	switch (newState)
	{
		case JT_STATE_INIT:
		{
			// 已经请求进入休眠
			if( pJt->setToOpState.OperationState != JT_STATE_SLEEP )
			{
				// 发送心跳
				Utp_SendCmd(&g_JtUtp, JTCMD_MCU_HB);
				
				_SetOperationState( JT_STATE_PREOPERATION,_OPERATION_PRE );
			}

			// 清除所有状态
			g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
			break;
		}
		case JT_STATE_SLEEP:
		{
			//g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
//			gModuleSleepCnt = GET_TICKS();

			//FlexCANStanbyMode();		// CAN Sleep
			break;
		}
		case JT_STATE_WAKEUP:
		{
			// 已经请求进入休眠
			if( pJt->setToOpState.OperationState != JT_STATE_SLEEP )
			{
				// 发送心跳
				Utp_SendCmd(&g_JtUtp, JTCMD_MCU_HB);
				_SetOperationState(JT_STATE_PREOPERATION,_OPERATION_PRE );
			}
			//g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
			gModuleWakeupCnt = GET_TICKS();
			break;
		}
		case JT_STATE_PREOPERATION:
		{
			// 已经请求进入休眠
			if( pJt->setToOpState.OperationState != JT_STATE_SLEEP )
			{
				// 发送心跳
				Utp_SendCmd(&g_JtUtp, JTCMD_MCU_HB);
				
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_ID);// 0x11
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);//0x12
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_ID); //0x30
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_CFG); //0x31
				//Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_BLE_EN); // 0x33			
				//Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_FILE_INFO);//0x16
			}
			//g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;

			gGPSTimeCnt = GET_TICKS();
			gGPRSConTimeCnt = GET_TICKS();			
			break;
		}
		case JT_STATE_OPERATION:
		{
			//if( pJt->opState != JT_STATE_PREOPERATION )	// 如果之前状态不为 PREOPERATION
			//{
			//	_SetOperationState(JT_STATE_PREOPERATION,_OPERATION_PRE );
			//}
			//
			gGPSTimeCnt = GET_TICKS();
			gGPRSConTimeCnt = GET_TICKS();
			break;
		}
	}

	pJt->fsm = JT808_findFsm(newState);
}

UTP_EVENT_RC JT808_event_simHb(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if (ev == UTP_CHANGED_BEFORE)
	{
		JT_state newOp = (JT_state)*pCmd->pExt->transferData;
		JT808_switchState(pJt, newOp);	// 返回的状态有变化时
	}
	else if( ev == UTP_CHANGED_AFTER )
	{
		// 通信模组状态与 本地保存状态不一至时
		if( pJt->opState != pJt->setToOpState.OperationState )
		{
			_JT808_DEBUGMSG("Change OPState[%d-%d]\r\n",pJt->opState ,pJt->setToOpState.OperationState );
			// 已经请求进入休眠
			if( pJt->setToOpState.OperationState != JT_STATE_SLEEP )
			{
				//pJt->opState = JT_STATE_INIT;
				_SetOperationState(JT_STATE_PREOPERATION,_OPERATION_PRE );
			}
			//g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}


/*
gGPSTimeCnt = GET_TICKS();
gGPRSSendTimeCnt = GET_TICKS();
*/
UTP_EVENT_RC JT808_event_devStateChanged(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_CHANGED_AFTER )
	{
		g_Jt.devState.cnt = SWAP16(g_Jt.devState.cnt);
		
		if( g_Jt.devState.cnt & _NETWORK_CONNECTION_BIT )
		{
			_JT808_DEBUGMSG("Network Connected\r\n");
			if( !g_Jt.gprsmsgInfo.connected )
			{
				gGPRSConCnt = GET_TICKS();
				g_Jt.gprsmsgInfo.ConnectTimeCnt = GET_TICKS() - gGPRSConTimeCnt ;
				_JT808_DEBUGMSG("Newtwork Connect time:%dms\r\n", g_Jt.gprsmsgInfo.ConnectTimeCnt );
			}
		}
		else
		{
			_JT808_DEBUGMSG("Network disconnect\r\n");
			if( g_Jt.gprsmsgInfo.connected )
			{
				gGPRSConTimeCnt = GET_TICKS();
				g_Jt.gprsmsgInfo.ConnectTimeCnt = 0;
			}
		}
		
		if( g_Jt.devState.cnt & _GPS_FIXE_BIT )
		{
			_JT808_DEBUGMSG("GPS Fixe\r\n");
			if( ! g_Jt.gpsmsgInfo.Activ )
			{
				gGPSFixCnt = GET_TICKS();
				g_Jt.gpsmsgInfo.ActiveTimeCnt = GET_TICKS() - gGPSTimeCnt;
				_JT808_DEBUGMSG("GPS Fixe Time:%dms\r\n",g_Jt.gpsmsgInfo.ActiveTimeCnt );
			}
		}
		else
		{
			_JT808_DEBUGMSG("GPS No Fixe\r\n");
			if( g_Jt.gpsmsgInfo.Activ )
			{
				gGPSTimeCnt = GET_TICKS();
				g_Jt.gpsmsgInfo.ActiveTimeCnt = 0 ;
			}
		}
		
		if( g_Jt.devState.cnt & _SMS_EXIT_BIT )
		{
			_JT808_DEBUGMSG("Rev SMS\r\n");
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SMS );// 获取短信
		}
		else
		{
			_JT808_DEBUGMSG("No SMS\r\n");
		}
		_JT808_DEBUGMSG("CSQ: %d,SNR: %d,Stae In View: %d\r\n",g_Jt.devState.csq,g_Jt.devState.snr,g_Jt.devState.siv );

		// GPS
		g_Jt.gpsmsgInfo.Activ = g_Jt.devState.cnt & _GPS_FIXE_BIT;		// GPS
		g_Jt.gpsmsgInfo.SNR = g_Jt.devState.snr ;
		g_Jt.gpsmsgInfo.StateView = g_Jt.devState.siv ;

		// GPRS
		g_Jt.gprsmsgInfo.connected = g_Jt.devState.cnt & _NETWORK_CONNECTION_BIT;
		g_Jt.gprsmsgInfo.sms = g_Jt.devState.cnt & _SMS_EXIT_BIT;
		g_Jt.gprsmsgInfo.CSQ = g_Jt.devState.csq;
		
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_beepCtrl(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_TX_START )
	{
		pJt->beepCtrl.beepFreq = SWAP16(pJt->beepCtrl.beepFreq);
		pJt->beepCtrl.beepOntim = SWAP16(pJt->beepCtrl.beepOntim);
		pJt->beepCtrl.beepOfftim = SWAP16(pJt->beepCtrl.beepOfftim);
		pJt->beepCtrl.beepCycles = SWAP16(pJt->beepCtrl.beepCycles);
	}
	else if( ev == UTP_TX_DONE )
	{
		pJt->beepCtrl.beepFreq = SWAP16(pJt->beepCtrl.beepFreq);
		pJt->beepCtrl.beepOntim = SWAP16(pJt->beepCtrl.beepOntim);
		pJt->beepCtrl.beepOfftim = SWAP16(pJt->beepCtrl.beepOfftim);
		pJt->beepCtrl.beepCycles = SWAP16(pJt->beepCtrl.beepCycles);
	}
	else if( ev == UTP_REQ_SUCCESS )
	{
		gSendCanCmdCnt = 0x00 ;
	}
	else if ( ev == UTP_REQ_FAILED )	//锟斤拷取失锟杰ｏ拷锟斤拷锟铰讹拷取
	{
		_JT808_DEBUGMSG("beep Ctrl failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0 ;
			//BatteryInfoSetCanReset();
			_JT808_DEBUGMSG("Resend Cmd Timeout,CAN reset\r\n");
		}
		else
		{
			_JT808_DEBUGMSG("Resend Cmd :%02X[%d]\r\n",JTCMD_CMD_BEEP_CTRL,gSendCanCmdCnt);
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC JT808_event_getSMSContext(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_CHANGED_AFTER )
	{
		//smsRevTime
		g_Jt.smsContext.smsText.smsRevTime = SWAP32( g_Jt.smsContext.smsText.smsRevTime );
		// 处理短信
		_JT808_DEBUGMSG("SMS:%d,SMSCout:%d,Len:%d,Time:%d\r\n",
			g_Jt.smsContext.smsExist,g_Jt.smsContext.smsCount,
			g_Jt.smsContext.smsText.smsLen,g_Jt.smsContext.smsText.smsRevTime );
		// 还有短信
		if( g_Jt.smsContext.smsExist )
		{
			Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SMS );
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC JT808_event_LocationChanged(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_CHANGED_AFTER )
	{
		g_Jt.locatData.latitude = SWAP32(g_Jt.locatData.latitude);//Dt_convertToU32(&g_Jt.locatData.latitude,DT_UINT32);
		g_Jt.locatData.longitude = SWAP32(g_Jt.locatData.longitude);//Dt_convertToU32(&g_Jt.locatData.longitude,DT_UINT32);

		g_Jt.gpsmsgInfo.latitude = g_Jt.locatData.latitude ;
		g_Jt.gpsmsgInfo.longitude = g_Jt.locatData.longitude ;
		//
		_JT808_DEBUGMSG("lat:%d,long:%d\r\n",g_Jt.locatData.latitude,g_Jt.locatData.longitude) ;

		//_ShowSleepWakeup();
	}
	return UTP_EVENT_RC_SUCCESS;
}



UTP_EVENT_RC JT808_event_rcvSvrData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	 if (ev == UTP_CHANGED_AFTER)
	 {
	  	//sJtTlv8900_proc(pCmd->pExt->transferData, pCmd->pExt->transferLen);
	  	//CommClearTimeout();
	 }
	 else if (ev == UTP_GET_RSP)
	 {
	 	
	 }
	 return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_event_rcvFileData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	 if (ev == UTP_CHANGED_AFTER)
	 {
	 	// 处理数据
	 }
	 else if (ev == UTP_GET_RSP)
	 {
	 	//
	 }
	 return UTP_EVENT_RC_SUCCESS;
}

//

UTP_EVENT_RC JT808_event_sendSvrData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
//	static int readParamOffset = 0;
//	static int i = 0;
//	int len = 0;
//	uint16* paramIDs = (uint16*)pCmd->pExt->transferData;

	if(ev == UTP_TX_START)	
	{
		//pCmd->pExt->transferLen = 
		//s	JtTlv0900_getChangedTlv( pCmd->pExt->transferData, pCmd->pExt->transferLen, Null);
		g_JtUtp.waitRspMs = 1000 * 10 ;	// 临时修改--10s
		
		pCmd->pExt->transferLen = g_txlen;
		_JT808_DEBUGMSG("Change Len:%d\r\n",pCmd->pExt->transferLen );
	}
	else if(ev == UTP_REQ_SUCCESS)
	{
		g_txlen = 0;
		gSendCanCmdCnt = 0 ;
		// 此时应该更新 Mirror---
		//int len = JtTlv0900_getChangedTlv(g_txBuf, sizeof(g_txBuf), Null);
		//JtTlv0900_updateMirror( g_txBuf , len );
		JtTlv0900_updateMirror();	// 更新
		_JT808_DEBUGMSG("update Bat mirror Info\r\n");
		gGPRSSendTimeCnt = GET_TICKS() - gGPRSSendTimeCnt ;
		
		if( gGPRSSendTimeCnt > g_Jt.gprsmsgInfo.TranMaxTimeCnt ||
			g_Jt.gprsmsgInfo.TranMaxTimeCnt == 0 )
		{
			g_Jt.gprsmsgInfo.TranMaxTimeCnt = gGPRSSendTimeCnt;
		}
		if( gGPRSSendTimeCnt < g_Jt.gprsmsgInfo.TranMinTimeCnt ||
			g_Jt.gprsmsgInfo.TranMinTimeCnt == 0 )
		{
			g_Jt.gprsmsgInfo.TranMinTimeCnt = gGPRSSendTimeCnt;
		}
		_JT808_DEBUGMSG("GPRS RevTime:%dms-%dms\r\n",g_Jt.gprsmsgInfo.TranMinTimeCnt,g_Jt.gprsmsgInfo.TranMaxTimeCnt );
	}

	else if ( ev == UTP_REQ_FAILED )	//出错 
	{
		_JT808_DEBUGMSG("update Bat failed.\r\n");
		if( ++gSendCanCmdCnt > _RESEND_CMD_COUNT )
		{
			gSendCanCmdCnt = 0 ;
			_JT808_DEBUGMSG("Resend Cmd Timeout,CAN Reset\r\n");
			BatteryInfoSetCanReset();
			// 清除状态
			//g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
		}
		else
		{
			_JT808_DEBUGMSG("Resend Cmd :%02X[%d]\r\n", JTCMD_CMD_SEND_TO_SVR,gSendCanCmdCnt);
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JT808_cmd_getFileContent(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if(ev == UTP_CHANGED_AFTER )
	{
		//Offset[UINT32] + file Data[UINT8[]
	}
	return UTP_EVENT_RC_SUCCESS;
}



UTP_EVENT_RC JT808_event_setLocationExtras(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	return UTP_EVENT_RC_SUCCESS;
}




UTP_EVENT_RC JT808_event_rcvBleData(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_CHANGED_AFTER )	// 收到数据
	{
		BatteryInfoExSetAwakened();
	}
	else if (ev == UTP_GET_RSP) 	// 返回数据
	{
		
		// 将返回数据写入 pCmd->pExt->transferData		& pCmd->pExt->transferLen及可
		// test
		//uint8_t test_buf[] = {'0','1','2','3','4','5','6','7','8','9','A'};
		//memcpy(pCmd->pExt->transferData,test_buf,sizeof(test_buf));
		//pCmd->pExt->transferLen = sizeof(test_buf);
		
		uint8 rspLen = 0;
		pCmd->pExt->transferData = Ble_ReqProc(pCmd->pStorage, gCurRevLen, &rspLen);
		pCmd->pExt->transferLen = rspLen;
		if (pCmd->pExt->transferData == Null)
		{
			return UTP_EVENT_RC_FAILED;
		}
	}
	return UTP_EVENT_RC_SUCCESS;
}

//
UTP_EVENT_RC JT808_event_bleStateChanged(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	if( ev == UTP_CHANGED_AFTER )
	{
		if( g_Jt.bleState.bleConnectState & _BLE_CONNECT_BIT )
		{
			_JT808_DEBUGMSG("BLE Connect[%02X:%02X:%02X:%02X:%02X:%02X]\r\n",
				g_Jt.bleState.bleConnectMAC[0],g_Jt.bleState.bleConnectMAC[1],
				g_Jt.bleState.bleConnectMAC[2],g_Jt.bleState.bleConnectMAC[3],
				g_Jt.bleState.bleConnectMAC[4],g_Jt.bleState.bleConnectMAC[5]);
		}
		else
		{
			_JT808_DEBUGMSG("Ble Disconnect\r\n");
		}
		g_Jt.BLEmsgInfo.connect = g_Jt.bleState.bleConnectState & _BLE_CONNECT_BIT ;
	}
	return UTP_EVENT_RC_SUCCESS;
}


UTP_EVENT_RC JT808_event_bleAuthChanged(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
	return UTP_EVENT_RC_SUCCESS;
}



//所有的命令，有传输事件发生，都会调用该回调函数
UTP_EVENT_RC JT808_utpEventCb(JT808* pJt, const UtpCmd* pCmd, UTP_TXF_EVENT ev)
{
     
	if (ev == UTP_REQ_SUCCESS)
	{
		JT808_fsm(MSG_UTP_REQ_DONE, (uint32_t)pCmd, ev);
        //gStructCanFlag.DeviceFlag.DeviceFlagBit.IsExModule = True;
	}
	if (ev == UTP_GET_RSP)
	{
        //加入外置模块通信标志
        gStructCanFlag.DeviceFlag.DeviceFlagBit.IsExModule = True;
	}

	if (pCmd->cmd == JTCMD_MCU_HB)
	{
		if (ev == UTP_REQ_SUCCESS)
		{
			Utp_DelaySendCmd(&g_JtUtp, JTCMD_MCU_HB, 2000);
		}
	}
	else if (pCmd->cmd == JTCMD_CMD_SEND_TO_SVR)
	{
		//if (ev == UTP_REQ_SUCCESS)
		//{
		//	JtTlv0900_updateMirror(&pCmd->pExt->transferData[1], pCmd->pExt->transferLen - 1);
		//}
	}

	return UTP_EVENT_RC_SUCCESS;
}

void JT808_fsm_init(uint8_t msgID, uint32_t param1, uint32_t param2)
{
}

void JT808_fsm_preoperation(uint8_t msgID, uint32_t param1, uint32_t param2)
{
	if (msgID == MSG_UTP_REQ_DONE)
	{
		//获取到所有的设备信息后，切换到OPERATION状态
		const UtpCmd* pCmd = (UtpCmd*)param1;
		if ( pCmd->cmd == JTCMD_CMD_GET_BLE_CFG/*JTCMD_CMD_GET_FILE_INFO*/ )
		{
			_SetOperationState(JT_STATE_OPERATION,_OPERATION_OPE );
		}
	}
}

void JT808_fsm_operation(uint8_t msgID, uint32_t param1, uint32_t param2)
{
	if (msgID == MSG_RUN)
	{
		// 更新JtTlv9000数据
		JtTlv0900_updateStorage();	//
		
		if (Utp_isIdle(&g_JtUtp) && (g_Jt.devState.cnt & _NETWORK_CONNECTION_BIT) )	// 空闲 & 连接网络s
		{
			int len = JtTlv0900_getChangedTlv(g_txBuf, sizeof(g_txBuf), Null);
			if (len) 
			{
				g_txlen = len;
				Utp_SendCmd(&g_JtUtp, JTCMD_CMD_SEND_TO_SVR);
				_JT808_DEBUGMSG("Send Bat Info:%d\r\n",len );
				//
				gGPRSSendTimeCnt = GET_TICKS();
				// save 
				JtTlv0900_Cache( g_txBuf , len );
			}
		}
	}
}

void JT808_fsm_sleep(uint8_t msgID, uint32_t param1, uint32_t param2)
{
	// 当前为Sleep
}

void JT808_fsm_wakeup(uint8_t msgID, uint32_t param1, uint32_t param2)
{
	// 当前为Wakeup
}

static JT808fsmFn JT808_findFsm(JT_state state)
{
	struct
	{
		JT_state state;
		JT808fsmFn fsm;
	}
	static const fsmDispatch[] =
	{
		{JT_STATE_INIT			, JT808_fsm_init},
		{JT_STATE_SLEEP			, JT808_fsm_sleep},
		{JT_STATE_WAKEUP		, JT808_fsm_wakeup},
		{JT_STATE_PREOPERATION	, JT808_fsm_preoperation},
		{JT_STATE_OPERATION		, JT808_fsm_operation},
	};
	for (int i = 0; i < GET_ELEMENT_COUNT(fsmDispatch); i++)
	{
		if (state == fsmDispatch[i].state)
		{
			return fsmDispatch[i].fsm;
		}
	}

	//程序不可能运行到这里
//	Assert(False);
	return Null;
}

void JT808_fsm(uint8_t msgID, uint32_t param1, uint32_t param2)
{
	g_pJt->fsm(msgID, param1, param2);
}

void BeepCtrl(  uint16  freq , uint8 duty , uint16 ontim , uint16 offtim , uint16 cycles )
{
	g_Jt.beepCtrl.beepFreq = freq;
	g_Jt.beepCtrl.beepDuty = duty ;
	g_Jt.beepCtrl.beepOntim = ontim ;
	g_Jt.beepCtrl.beepOfftim = offtim ;
	g_Jt.beepCtrl.beepCycles = cycles ;
	Utp_SendCmd(&g_JtUtp, JTCMD_CMD_BEEP_CTRL );
	_JT808_DEBUGMSG("Beep Ctrl:%dhz %d%% %dms-%dms %d\n",
		freq , duty , ontim , offtim , cycles );
}

//Utp_DeleteCmd

static void _SetOperationState(uint8_t Operation, uint8_t Parameter )
{
	g_Jt.setToOpState.OperationState = Operation ;
	g_Jt.setToOpState.StateParameter = Parameter ;
	Utp_SendCmd(&g_JtUtp, JTCMD_SET_OP_STATE);
}

void JT808_CheckResetSerAddr(void)
{
	Utp_SendCmd(&g_JtUtp, JTCMD_CMD_GET_SIM_CFG);
}


// 进入休眠

void SetComModeSleep(void)
{
	if( g_Jt.opState != JT_STATE_SLEEP )
	{
		Utp_Reset(&g_JtUtp);		// 清除所有指令
		_SetOperationState( JT_STATE_SLEEP , _OPERATION_SLEEP );
	}
}

// 模块已经休眠
Bool ComModeSleep(void)
{
	return ( g_Jt.opState == JT_STATE_SLEEP );
}
// 模块正在准备休眠
Bool ComModeSleepIng(void)
{
	return ( ( g_Jt.opState != JT_STATE_SLEEP ) && ( g_Jt.setToOpState.OperationState == JT_STATE_SLEEP ) ) ;
}

// 模块已经唤醒
Bool ComModeWakeup(void)
{
	return ( g_Jt.opState == JT_STATE_WAKEUP );
}


// 模块正在唤醒
Bool ComModeWakeupIng(void)
{
	return ( ( g_Jt.opState == JT_STATE_SLEEP ) && ( g_Jt.setToOpState.OperationState == JT_STATE_WAKEUP ) ) ;
}

// 外置模块唤醒
Bool ComIRQWakeup(void)
{
	return can0_receive_flag == 1 ;
}

void SetComModeWakeup(void)
{
	if( g_Jt.opState == JT_STATE_SLEEP )
	{
		Utp_Reset(&g_JtUtp);		// 清除所有指令
		_SetOperationState( JT_STATE_WAKEUP , _OPERATION_WKUP );
	}
}




#define	_CAN_TX_TIMEOUT_MS		(10)

//鍙戦€佹暟鎹埌鎬荤嚎
int JT808_txData(uint8_t cmd, const uint8_t* pData, uint32_t len)	//cmd为CAN协议的PF
{
  uint8_t  TempData[len];
  memcpy(TempData,pData,len);
  return  SendCANData(EX_MODU_TX_ID, TempData, len);
//	uint32_t send_len = 0;
////	static uint32_t count = 0;
//	uint32_t tx_timeout = 0;//system_ms_tick
//	//transfer data to bus.
//	while(len)
//	{
//		if(len > 8)
//		{
//			send_len = 8;
//		}
//		else
//		{
//			send_len = len;
//		}
//		//SendCANData(TX_MAILBOX, EX_MODU_TX_ID, pData, send_len);
//    SendCANData(EX_MODU_TX_ID, pData, send_len);
//		tx_timeout = GET_TICKS();
//		while( FLEXCAN_DRV_GetTransferStatus(0, 1) == STATUS_BUSY &&
//			GET_TICKS() - tx_timeout <_CAN_TX_TIMEOUT_MS );

//		if(len > 8)
//		{
//			pData = pData + 8;
//			len = len - 8;
//		}
//		else
//		{
//			break;
//		}
//	}
//	return len;
}

#if 0
uint32_t JT808_rxData(void)
{
	uint32_t i = 0;
	uint32_t recv_len = 0;
	/* Define receive buffer */
	flexcan_msgbuff_t recvBuff;

	memset(&recvBuff, 0, sizeof(flexcan_msgbuff_t));
	/* Start receiving data in RX_MAILBOX. */
	FLEXCAN_DRV_Receive(0, 0, &recvBuff);
	/* Wait until the previous FlexCAN receive is completed */
	while(FLEXCAN_DRV_GetTransferStatus(0, 0) == STATUS_BUSY)
	{
		if(i > 50000)
		{
			break;
		}
		else
		{
			i++;
		}
	}
	recv_len = recvBuff.dataLen;
	if(recv_len > 0)
	{
		Utp_RxData(&g_JtUtp, recvBuff.data, recv_len);
	}
	return recv_len;
}
#endif //


//从总线上接收数据
void JT808_rxDataProc(const uint8_t* pData, int len)
{
	//Utp_RxData(&g_JtUtp, pData, len);
	CirBuffPush( &g_JtUtp.rxBuffCirBuff, pData ,len );
}


void JT808_timerProc()
{
	if( can0_receive_flag == 0x01 )
	{
		can0_receive_flag = 0;
		//JT808_rxDataProc(gCanRxData.data,gCanRxData.dataLen);//( receive_message.rx_data , receive_message.rx_dlen );
		gCanbusRevTimeMS = GET_TICKS();
	}
	else
	{
		if( GET_TICKS() - gCanbusRevTimeMS > _CAN_BUS_REV_TIMEOUT_MS )
		{
			_JT808_DEBUGMSG("Can Rev Timeout.reset can\r\n");
			BatteryInfoSetCanReset();
			gCanbusRevTimeMS = GET_TICKS();

			// 清除所有状态
			g_Jt.bleState.bleConnectState = 0x00;
			g_Jt.devState.cnt = 0x00 ;
		}
	}
}

void JT808_run()
{
	JT808_timerProc();
	JT808_fsm(MSG_RUN, 0, 0);
	Utp_Run(&g_JtUtp);
}

void JT808_start()
{
	//启动硬件，使能中断
	JT808_switchState(g_pJt, JT_STATE_INIT);
	gCanbusRevTimeMS = GET_TICKS();
}


#define		_JT808_TASK_NORMAL_RUN_MS			15
#define		_JT808_TASK_SLEEP_RUN_MS			200
#define		_JT808_TASK_HALT_RUN_MS				500
#include "BSPTaskDef.h"
#include "IOCheck.h"

// 外围模块存在的IO状态,默认不存在
//static uint8 last_module_io = 1 ;	// 默认不存在
//uint8 cur_modeul_io ;	
//static uint8 module_pwr_on = 0 ;					// 外围模块是否供电，默认没有供电

static uint8 last_module_st = 0;	// 不存在 or 短路



/*
//
*/


void ResetJT808(void)
{
	JT808_init();
	g_Jt.bleState.bleConnectState = 0x00;
	g_Jt.devState.cnt = 0x00 ;
	can0_receive_flag = 0 ;	// 清除can
}



/*
	如果外置模块为休眠模式:
		检测是否需要唤醒(外置模块唤醒 or 12小时唤醒 or MCU唤醒)
	如果外置为非休眠模式:
		检测是否需要休眠
*/
#if 1

// 唤醒
#define		_WAKEUP_TIMEOUT_MS			(30*1000)			// 15*1000	模块唤醒超时
#define		_WAKEUP_PWR_CNT				(5*1000)			// 模块重启,电源 IO控制 
#define		_WAKEUP_ID_IRQ_CNT			(20*1000)			// 模块唤醒,IO脚唤醒时间
// 休眠
#define		_SLEEP_TIMEOUT_MS			(15*1000)			// 15*1000 模块休眠


// 唤醒12小时后，一些超时设置
#define		_WAKEUP_WORK_TIMEOUT		(3*60*1000)				// 3分钟 唤醒后超时间
#define		_GPRS_CONNECT_TIMEOUT		(10*1000)				// GPRS连接超时时间
#define		_GPS_FIX_TIMEOUT			(10*1000)				// GPS定位超时时间

/*
static void _ShowSleepWakeup(void)
{
	_JT808_DEBUGMSG("CS:%d:%d,%d\r\n",ComModeSleep(),ComModeWakeupIng(),ComModeSleepIng());
	_JT808_DEBUGMSG("MS:%d:%d:%d:%d:%d\r\n",
		gModuleSleepIngTimeoutCnt,gModuleSleepCnt,
		gModuleWakupIngTimeoutCnt,gModuleWakeupCnt,
		GET_TICKS() );
	_JT808_DEBUGMSG("LS:%d:%d\r\n",BatteryInfoIsExEnterLightSleep(),BatteryInfoExIsWakeUp());
}
*/

static uint32_t cur_tick = 0 ;

static Bool _ReStartMouduleTask( uint32_t ms)
{
	BSPTaskStart(TASK_ID_JT808_TASK, ms );
	return 1 ;
}

static void MouduleWork(void)
{
	PT_BEGIN(&gPTJT808);
	
	if( ComModeSleep() )	// 模块已经休眠---检测是否需要唤醒处理
	{
		if( ComModeWakeupIng() )	// 模块正在被唤醒中...
		{
			if( GET_TICKS() - gModuleWakupIngTimeoutCnt > _WAKEUP_TIMEOUT_MS )	// 15ms
			{
				_JT808_DEBUGMSG("Module Wakeup Timerout,reboot after %dms :%d\r\n",_WAKEUP_PWR_CNT,system_ms_tick);

				// 外置模块重启
				//ModuleHWReset();		
				
                if(gStructCanFlag.DeviceFlag.DeviceFlagBit.IsCabinet == True && gStructCanFlag.DeviceFlag.DeviceFlagBit.IsExModule == False)
                {
                    ;
                }
                else
                {
                    MODULE_PWR_OFF();
                }
				cur_tick = system_ms_tick ;
				PT_WAIT_UNTIL(&gPTJT808, 
					_ReStartMouduleTask(_JT808_TASK_SLEEP_RUN_MS) && 
					( system_ms_tick - cur_tick > _WAKEUP_PWR_CNT ) );
				PT_INIT(&gPTJT808);
				MODULE_PWR_ON();
				
				gModuleWakupIngTimeoutCnt = GET_TICKS();	// 重新计数
				gModuleWakeupCnt = GET_TICKS();		// 
                
				_JT808_DEBUGMSG("外置模块重启成功:%d\r\n",system_ms_tick);
			}
			JT808_run();			// 此处要么重新开始写入状态
			BSPTaskStart(TASK_ID_JT808_TASK, _JT808_TASK_NORMAL_RUN_MS );
		}
		else						// 模块未被唤醒---检测是否需要唤醒 or 继续休眠
		{
			if( !BatteryInfoIsExEnterLightSleep() )			// MCU唤醒
			{
				//FlexCANNormalMode();
        BSPCan_Init();
				
				//ModuleIntPinSetOutput();		// 设置为输出 
				//MODULE_WAKEUP();				// 唤醒外置模块
				//cur_tick = system_ms_tick ;
				//PT_WAIT_UNTIL(&gPTJT808, 
				//	_ReStartMouduleTask(_JT808_TASK_SLEEP_RUN_MS) && 
				//	( system_ms_tick - cur_tick > _WAKEUP_ID_IRQ_CNT ) &&
				//	!BatteryInfoIsExEnterLightSleep() );
				//PT_INIT(&gPTJT808);
				//ModuleIntPinSetInput();			// 恢复检测功能
				//
				BatteryInfoExSetAwakened();		// 清除12小时唤醒
				gModuleWakupIngTimeoutCnt = GET_TICKS();	// 计时
				SetComModeWakeup();
				JT808_run();
				gModuleWakeupCnt = GET_TICKS();
				BSPTaskStart(TASK_ID_JT808_TASK, _JT808_TASK_NORMAL_RUN_MS );
				_JT808_DEBUGMSG((char*)"MCU唤醒外置模块\r\n");
			}
			else if( BatteryInfoExIsWakeUp() )	// 12小时唤醒---设置状态
			{
				//FlexCANNormalMode();
        BSPCan_Init();
				//ModuleIntPinSetOutput();		// 设置为输出 
				//MODULE_WAKEUP();				// 唤醒外置模块
				//cur_tick = system_ms_tick ;
				//PT_WAIT_UNTIL(&gPTJT808, 
				//	_ReStartMouduleTask(_JT808_TASK_SLEEP_RUN_MS) && 
				//	( system_ms_tick - cur_tick > _WAKEUP_ID_IRQ_CNT ) &&
				//	BatteryInfoExIsWakeUp() );
				//PT_INIT(&gPTJT808);
				//ModuleIntPinSetInput();			// 恢复检测功能

				gModuleWakupIngTimeoutCnt = GET_TICKS();	// 计时
				SetComModeWakeup();
				JT808_run();
				gModuleWakeupCnt = GET_TICKS();
				BSPTaskStart(TASK_ID_JT808_TASK, _JT808_TASK_NORMAL_RUN_MS );
                
				_JT808_DEBUGMSG("休眠12小时后，MCU请求上报GPS数据\r\n");//
			}
			else if( ComIRQWakeup() )							// 外置模块唤醒
			{
				//FlexCANNormalMode();
        BSPCan_Init();
				
				BatteryInfoExSetAwakened();		// 设置清除5分钟
				
				//ModuleIntPinSetOutput();		// 设置为输出 
				//MODULE_WAKEUP();				// 唤醒外置模块
				//cur_tick = system_ms_tick ;
				//PT_WAIT_UNTIL(&gPTJT808, _ReStartMouduleTask(_JT808_TASK_SLEEP_RUN_MS) && ( system_ms_tick - cur_tick > _WAKEUP_ID_IRQ_CNT ) );
				//PT_INIT(&gPTJT808);
				//ModuleIntPinSetInput();			// 恢复检测功能
				
				gModuleWakupIngTimeoutCnt = GET_TICKS();	// 计时
				SetComModeWakeup();
				JT808_run();
				gModuleWakeupCnt = GET_TICKS();
				BSPTaskStart(TASK_ID_JT808_TASK, _JT808_TASK_NORMAL_RUN_MS );
				_JT808_DEBUGMSG("外置模块主动唤醒\r\n");
			}
			else
			{
				BSPTaskStart(TASK_ID_JT808_TASK, _JT808_TASK_SLEEP_RUN_MS );
				//_JT808_DEBUGMSG("没有唤醒事件,继续休眠--空跑\r\n");
			}
		}
	}
	else					// 模块已经处于正常模式---检测是否需要进入休眠	
	{
		if( ComModeSleepIng() )		// 模块正在被休眠中...检测休眠是否超时 or 是否需要唤醒
		{
			//进入休眠计时---
			if( GET_TICKS() - gModuleSleepIngTimeoutCnt > _SLEEP_TIMEOUT_MS )		// 休眠超时
			{
				BatteryInfoExClrWakeUp();		// 清除状态标志
				//ModuleHWReset();				// 外置模块重启--然后重新开始发送

                if(gStructCanFlag.DeviceFlag.DeviceFlagBit.IsCabinet == True && gStructCanFlag.DeviceFlag.DeviceFlagBit.IsExModule == False)
                {
                    ;
                }
                else
                {
                    MODULE_PWR_OFF();
                }
				cur_tick = system_ms_tick ;
				PT_WAIT_UNTIL(&gPTJT808, 
					_ReStartMouduleTask(_JT808_TASK_SLEEP_RUN_MS) && 
					( system_ms_tick - cur_tick > _WAKEUP_PWR_CNT ) && 
					BatteryInfoIsExEnterLightSleep() );
				PT_INIT(&gPTJT808);
				MODULE_PWR_ON();
				
				gModuleSleepIngTimeoutCnt = GET_TICKS();
				_JT808_DEBUGMSG("模块休眠超时,重启模块\r\n");
			}
			// 如果此时
			if( !BatteryInfoIsExEnterLightSleep() )// MCU or 外置模块 唤醒
			{
				BatteryInfoExClrWakeUp();
				// 重新唤醒---
				ResetJT808();
				_JT808_DEBUGMSG("MCU唤醒,模块任务重新运行\r\n");
			}
		}
		else						// 模块不处于正在休眠下,检测是否需要休眠 or 正常运行
		{
			// 有5分钟进休眠状态---MCU请求进入浅休眠 or 进入浅休眠12小时后临时唤醒
			if( BatteryInfoIsExEnterLightSleep() )
			{
				if( BatteryInfoExIsWakeUp() )		// 有12小时唤醒
				{
					// 如果 唤醒3分钟后,或者GPS定位&GPRS连接30s后
					if( GET_TICKS() - gModuleWakeupCnt > _WAKEUP_WORK_TIMEOUT ||
						( ( g_Jt.devState.cnt & _NETWORK_CONNECTION_BIT ) && 
						  ( g_Jt.devState.cnt & _GPS_FIXE_BIT ) &&
						  GET_TICKS() - gGPRSConCnt > _GPRS_CONNECT_TIMEOUT &&
						  GET_TICKS() - gGPSFixCnt > _GPS_FIX_TIMEOUT ))
					{
						BatteryInfoExClrWakeUp();		// 清除12小时唤醒标志
						gModuleSleepIngTimeoutCnt = GET_TICKS();
						SetComModeSleep();	// 请求进入休眠
						_JT808_DEBUGMSG("MCU请求外置模块重新进入休眠\r\n");	
					}
				}
				else
				{
					gModuleSleepIngTimeoutCnt = GET_TICKS();
					SetComModeSleep();	// 请求进入休眠
					_JT808_DEBUGMSG("MCU请求外置模块进入休眠\r\n");	
				}
			}
		}
		JT808_run();
		BSPTaskStart(TASK_ID_JT808_TASK, _JT808_TASK_NORMAL_RUN_MS );
	}
	PT_END(&gPTJT808);
	
}
#else
static void MouduleWork(void)
{
	JT808_run();
	BSPTaskStart(TASK_ID_JT808_TASK, _JT808_TASK_NORMAL_RUN_MS );
}

#endif //



void JT808TaskEnter(void *p )
{
	uint8 cur_module_st = BITGET( gBatteryInfo.Status.IOStatus,3 );
	
	if( cur_module_st != last_module_st )
	{
		if( cur_module_st == 1 )		// 外置模块重新接入
		{
			// 电源已经被启动
			//FlexCANNormalMode();		// CAN初始化
        BSPCan_Init();
        _JT808_DEBUGMSG("外置模块连接\r\n");
		}
		else							// 外置模块断开
		{
			// 电池已经被关闭
			ResetJT808();				// 重设JT808任务
			//FlexCANStanbyMode();		// CAN 休眠
			//ModuleIntPinSetInput();		// ID设置为中断检测			
			PT_INIT(&gPTJT808);			// PT初始化
			_JT808_DEBUGMSG("外置模块断开\r\n");
		}
		last_module_st = cur_module_st ;

		//_ShowSleepWakeup();
	}
	//
	if( cur_module_st )		// 外置模块有接入时
	{
		MouduleWork();
	}
	else					// 没有接入
	{
		BSPTaskStart(TASK_ID_JT808_TASK, _JT808_TASK_HALT_RUN_MS  );
	}
}

/*
0E09494D542D43372D343544374246
0319C0
03020105
03031218
09FF33300015830FF008
11073923CF407316429A5C417E7DC49A8314
*/
static void _UpdataBleAdvData(uint8_t mac[6])
{
	uint8_t i = 0 ;
	uint8_t _adv[31+31] ={0x00};
	uint8_t _adv_type[] = {0x03,0x19,0xC0,0x03,0x02,0x01,0x05,0x03,0x03,0x12,0x18};
	uint8_t _adv_uuid[] = {0x11,0x07,0x39,0x23,0xCF,0x40,0x73,0x16,0x42,0x9A,0x5C,0x41,0x7E,0x7D,0xC4,0x9A,0x83,0x14};
	memset( _adv , 0 , sizeof(_adv) );
	// 添加名字
	// Name
	{
		uint8_t j = i , len = 0 ;		
		_adv[i++] = 0;// 锟斤拷锟斤拷 1 + sizeof(_BLE_NAME) - 1;
		_adv[i++] = 0x09;	// name
		len = sprintf( (char*)_adv + i ,_BLE_NAME ,_DEV_MODEL, mac[3],mac[4],mac[5]);
		_adv[j] = 1 + len ;
		i += len ;
	}
	//
	memcpy( _adv + i , _adv_type , sizeof(_adv_type) );
	i += sizeof(_adv_type);
	// MAC 地址
	{
		_adv[i++] = 1 + sizeof(_BLE_COMPY) - 1 + 6;
		_adv[i++] = 0xFF;
		//_adv[i++] = 
		_adv[i++] = mac[5];
		_adv[i++] = mac[4];
		_adv[i++] = mac[3];
		_adv[i++] = mac[2];
		_adv[i++] = mac[1];
		_adv[i++] = mac[0];
        memcpy( _adv + i , _BLE_COMPY , sizeof(_BLE_COMPY) - 1 );
		i += sizeof(_BLE_COMPY) - 1;
	}
	// UUID
	memcpy( _adv + i , _adv_uuid , sizeof(_adv_uuid) );
	i += sizeof(_adv_uuid);
	
	sprintf( (char*)g_Jt.blecfgParam.BleName ,_BLE_NAME ,_DEV_MODEL, mac[3],mac[4],mac[5]);
	memcpy( g_Jt.blecfgParam.BleAdvData , _adv , sizeof(_adv));
}





void JT808_init()
{
	
/************************************************
*以下变量是协议变量，仅供协议使用
************************************************/
	
	#define JT_CMD_SIZE 24
	static uint8_t g_protocolVer = 1;	//传输协议版本号
	static uint8_t g_updatefiletype = 1; // 中控
	static uint8_t g_rxBuf[192];	
	static UtpCmdEx g_JtCmdEx[JT_CMD_SIZE];
	static const UtpCmd g_JtCmd[JT_CMD_SIZE] =
	{
		//位置越靠前，发送优先级越高
		{&g_JtCmdEx[0],UTP_NOTIFY, JTCMD_MCU_HB, "McuHb", g_hbdata, sizeof(g_hbdata),Null,0,Null},	/*(uint8_t*)& g_hbIntervalMs*/
		{&g_JtCmdEx[1],UTP_EVENT_NOTIFY, JTCMD_SIM_HB, "SimHb", (uint8_t*)& g_Jt.opState, 1, Null, 0, (UtpEventFn)JT808_event_simHb},
		//	
		{&g_JtCmdEx[2],UTP_READ , JTCMD_CMD_GET_SIM_ID , "GetSimID"	, (uint8_t*)& g_Jt.property, sizeof(JtDevProperty), &g_protocolVer, 1, (UtpEventFn)JT808_cmd_getSimID},
		{&g_JtCmdEx[3],UTP_READ , JTCMD_CMD_GET_SIM_CFG, "GetSimCfg", (uint8_t*)& g_rxBuf, sizeof(g_rxBuf), (uint8_t*)& g_txBuf, 0/*sizeof(g_txBuf)*/, (UtpEventFn)JT808_cmd_getSimCfg},
		{&g_JtCmdEx[4],UTP_WRITE, JTCMD_CMD_SET_SIM_CFG, "SetSimCfg", (uint8_t*)& g_rxBuf, sizeof(g_rxBuf), Null, 0, (UtpEventFn)JT808_cmd_setSimCfg},
		{&g_JtCmdEx[5],UTP_READ , JTCMD_CMD_GET_BLE_ID , "GetBLEID",(uint8_t*)&g_Jt.bleproperty, sizeof(JtDevBleProperty),Null,0,(UtpEventFn)JT808_cmd_getBleID},
		{&g_JtCmdEx[6],UTP_READ , JTCMD_CMD_GET_BLE_CFG , "GetBLECfg",(uint8_t*)& g_BleCfgParam_mirror, sizeof(JtDevBleCfgParam),Null,0,(UtpEventFn)JT808_cmd_getBleCfg},
		{&g_JtCmdEx[7],UTP_WRITE, JTCMD_CMD_SET_BLE_CFG, "SetBLECfg", (uint8_t*)&g_Jt.blecfgParam , sizeof( JtDevBleCfgParam ), Null, 0, (UtpEventFn)JT808_cmd_setBleCfg},
		{&g_JtCmdEx[8],UTP_READ, JTCMD_CMD_GET_BLE_EN, "GetBLEEnable", (uint8_t*)& g_Jt.bleEnCtrl, 2,Null, 0, (UtpEventFn)JT808_cmd_getBleEnable},
		{&g_JtCmdEx[9],UTP_WRITE, JTCMD_CMD_SET_BLE_EN, "SetBLEEnable", (uint8_t*)&g_Jt.bleEnCtrl , 2, Null, 0, (UtpEventFn)JT808_cmd_setBleEnable},
		{&g_JtCmdEx[10],UTP_READ , JTCMD_CMD_GET_FILE_INFO , "GetFileInfo",(uint8_t*)&g_Jt.updatefileinfo , sizeof(UpdateFileInfo),(uint8_t*)&g_updatefiletype,sizeof(g_updatefiletype),(UtpEventFn)JT808_cmd_getFileInfo },
		{&g_JtCmdEx[11],UTP_WRITE , JTCMD_SET_OP_STATE, "SetOpState"	, (uint8_t*)& g_Jt.setToOpState, sizeof(JT_SetOperationState) , Null , 0 , (UtpEventFn)JT808_cmd_setToOpState},
		{&g_JtCmdEx[12],UTP_WRITE, JTCMD_CMD_SEND_TO_SVR, "SendDataToSvr", (uint8_t*)g_txBuf, sizeof(g_txBuf), Null, 0, (UtpEventFn)JT808_event_sendSvrData},
		{&g_JtCmdEx[13],UTP_READ,JTCMD_CMD_GET_SMS,"GetSMSContext",(uint8_t*)&g_Jt.smsContext,sizeof(GetSMSContext),Null,0,(UtpEventFn)JT808_event_getSMSContext},
		{&g_JtCmdEx[14],UTP_READ , JTCMD_CMD_GET_FILE_CONTENT, "GetFileContent"	, (uint8_t*)g_rxBuf, sizeof(g_rxBuf), (uint8_t*)& g_Jt.filecontent, sizeof(FileContent) , (UtpEventFn)JT808_cmd_getFileContent},
		{&g_JtCmdEx[15],UTP_WRITE, JTCMD_CMD_SET_LOCATION_EXTRAS, "SetLocationExtras", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null, 0, (UtpEventFn)JT808_event_setLocationExtras},
		{&g_JtCmdEx[16],UTP_WRITE, JTCMD_CMD_BEEP_CTRL, "BeepCtrl", (uint8_t*)&g_Jt.beepCtrl, sizeof(_BeepCtrl), Null, 0, (UtpEventFn)JT808_cmd_beepCtrl},

		// EVENT 
		// 网络 & GPS
		{&g_JtCmdEx[17],UTP_EVENT, JTCMD_EVENT_DEV_STATE_CHANGED, "DevStateChanged", (uint8_t*)& g_Jt.devState, sizeof(JT_devState), Null, 0, (UtpEventFn)JT808_event_devStateChanged},
		{&g_JtCmdEx[18],UTP_EVENT, JTCMD_EVENT_DEV_STATE_LOCATION, "LocationChanged", (uint8_t*)& g_Jt.locatData, sizeof(Jt_LocationData), Null, 0, (UtpEventFn)JT808_event_LocationChanged},
		{&g_JtCmdEx[19],UTP_EVENT, JTCMD_EVT_RCV_SVR_DATA, "RcvSvrData", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null, 0, (UtpEventFn)JT808_event_rcvSvrData},
		{&g_JtCmdEx[20],UTP_EVENT, JTCMD_EVT_RCV_FILE_DATA, "RcvFileData", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null, 0, (UtpEventFn)JT808_event_rcvFileData},
		// 蓝牙
		{&g_JtCmdEx[21],UTP_EVENT, JTCMD_BLE_EVT_AUTH, "BleAuth", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), Null , 0 , (UtpEventFn)JT808_event_bleAuthChanged },
		{&g_JtCmdEx[22],UTP_EVENT, JTCMD_BLE_EVT_CNT, "BleCnt", (uint8_t*)&g_Jt.bleState, sizeof(Jt_BleState) , Null , 0 , (UtpEventFn)JT808_event_bleStateChanged },
		// 
		{&g_JtCmdEx[23],UTP_EVENT, JTCMD_BLE_RCV_DAT, "BleRcvDat", (uint8_t*)g_rxBuf, sizeof(g_rxBuf), (uint8_t*)g_txBuf, 0/*sizeof(g_txBuf)*/, (UtpEventFn)JT808_event_rcvBleData},
	}; 
	
	static const UtpCfg g_cfg =
	{
		.cmdCount = JT_CMD_SIZE,
		.cmdArray = g_JtCmd,
		.TxFn = JT808_txData,
		.TresferEvent = (UtpEventFn)JT808_utpEventCb,
		.pCbObj = &g_Jt,
	};

	// Jose
	//static const Obj obj = { "JT808", JT808_start, (ObjFn)JT808_sleep, JT808_run };
	//ObjList_add(&obj);
	memset( &g_Jt , 0 , sizeof(g_Jt));
	g_Jt.opState = JT_STATE_UNKNOWN;	//初始化为一个UNKNOWN值

	// 蓝牙初始化
	{
		_UpdataBleAdvData( g_Jt.bleproperty.BleMac );
		g_Jt.blecfgParam.BleAdvInterval = 30;	
		g_Jt.bleEnCtrl = 0x00 ;
	}
	// 终端初始化
	{
		g_Jt.property.protocolVer = 0x00;
		g_Jt.property.devClass = _DEV_TYPE;
		strcpy( (char*)g_Jt.property.devModel,_DEV_MODEL);
	}
	
	Utp_Init(&g_JtUtp, &g_cfg, &g_jtFrameCfg);

	//JtTlv8900_init();
	JtTlv0900_init();
	JtTlv8103_init();

	// Jose
	JT808_start();

	PT_INIT(&gPTJT808); 
	gSendCanCmdCnt = 0x00;
}



// 返回接口


#endif //#ifdef CANBUS_MODE_JT808_ENABLE	


