
#include "Common.h"
#include "JtTlv8900.h"
#include "TlvIn.h"
#include "TlvOut.h"
#include "NvdsUser.h"
//#include "Pms.h"

#ifdef CANBUS_MODE_JT808_ENABLE	

static TlvInMgr g_jtTlvInMgr_8900;

static Jt8900 g_Jt8900 ;

TlvInEventRc JtTlv8900_Event(TlvInMgr* mgr, const TlvIn* pItem, TlvInEvent ev)
{
	if( ev == TE_CHANGED_AFTER )
	{
		switch ( pItem->tag )
		{
			case TAG_ACTIVE :
				g_cfgInfo.isActive = *pItem->storage;
				NvdsUser_Write(NVDS_CFG_INFO);
				break ;
			case TAG_SET_ACC_STATE :
				g_pdoInfo.isRemoteAccOn = *pItem->storage ;
				NvdsUser_Write(NVDS_PDO_INFO);
				break ;
			case TAG_SET_WHELL_LOCK :
				g_pdoInfo.isWheelLock  = *pItem->storage;
				NvdsUser_Write(NVDS_PDO_INFO);
				break ;
			case TAG_SET_CABIN_LOCK :
				g_pdoInfo.isCanbinLock = *pItem->storage;
				NvdsUser_Write(NVDS_PDO_INFO);
				break ;
			case TAG_SET_POWER_OFF :
				break ;
			case TAG_SET_BAT_IDEN_EN :
				break ;
			case TAG_SET_BAT_ALAM_EN :
				break ;
			case TAG_SET_BAT_BID :
				break ;
			default :break ;
		}
	}

	return (TlvInEventRc)UTP_EVENT_RC_SUCCESS;
}

UTP_EVENT_RC JtTlv8900_proc(const uint8* data, int len)
{
	//B9 01 00 01 01 A0 10 00 01 02 01 00 00 00 01 00 00 00 01 00 01 00 00 	--- 错误包
	TlvInMgr_updateStorage(&g_jtTlvInMgr_8900, data, len);

	return UTP_EVENT_RC_SUCCESS;
}

void JtTlv8900_init()
{
	const TlvIn g_tlvIn_8900[] =
	{
		{"ACT_STATE"		, TAG_ACTIVE,			1, &g_Jt8900.mAcitveState },
		{"SET_ACC  "		, TAG_SET_ACC_STATE,	1, &g_Jt8900.mAccState},
		{"SET_WHELL_LOCK"	, TAG_SET_WHELL_LOCK,	1, &g_Jt8900.mWhellState},
		{"SET_CABIN_LOCK"	, TAG_SET_CABIN_LOCK,	1, &g_Jt8900.mCabState},
		{"SET_POWER_OFF"	, TAG_SET_POWER_OFF,	1, &g_Jt8900.mPowerOff},
		{"SET_BAT_IDEN"		, TAG_SET_BAT_IDEN_EN,	1, &g_Jt8900.mBatIDEnable},
		{"SET_BAT_ALAMEN"	, TAG_SET_BAT_ALAM_EN,	1, &g_Jt8900.mBatAlamEnable},
		{"SET_BAT_BID"		, TAG_SET_BAT_BID,		1, (uint8_t*)&g_Jt8900.mBatVerify},
	};
	TlvInMgr_init(&g_jtTlvInMgr_8900, g_tlvIn_8900, GET_ELEMENT_COUNT(g_tlvIn_8900), 1, (TlvInEventFn)JtTlv8900_Event, True);
}

#endif //


