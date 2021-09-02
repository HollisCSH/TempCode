
#ifndef  _JT_TLV_8900_H_
#define  _JT_TLV_8900_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus
#include "BSPTypeDef.h"
#include "utp.h"
	
	typedef enum
	{
		TAG_ACTIVE = 1,
		TAG_SET_ACC_STATE = 2,
		TAG_SET_WHELL_LOCK = 3,
		TAG_SET_CABIN_LOCK = 4,
		//
		TAG_SET_POWER_OFF = 5,	// 远程断电状态
		TAG_SET_BAT_IDEN_EN,	// 电池身份校验使能
		TAG_SET_BAT_ALAM_EN,	// 电池报警使能
		TAG_SET_BAT_BID,		// 电池身份
	}CMD8900_TLV;



	//状态
#pragma pack(1)	

	typedef struct
	{
		uint8 	BatBid[6];
		uint8 	BatVerify;
	}TagBatVerify ;

	typedef struct 
	{
		uint8 		mAcitveState;
		uint8		mAccState;
		uint8		mWhellState;
		uint8		mCabState ;
		uint8		mPowerOff ;
		uint8		mBatIDEnable;
		uint8 		mBatAlamEnable;
		TagBatVerify		mBatVerify ;
	}Jt8900;

#pragma pack()

	

	UTP_EVENT_RC JtTlv8900_proc(const uint8* data, int len);
	void JtTlv8900_init(void);


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


