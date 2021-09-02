
#ifndef  _JT_TLV_0900_H_
#define  _JT_TLV_0900_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus
#include "BSPTypeDef.h"
//#include "BmsReg.h"

	typedef enum
	{
		TAG_SMART_STATE		= 0x01,	// Smart 板运行状态数据
		TAG_PMS_STATE		= 0x10,	// PMS 运行状态数据

		TAG_BAT_INFO		= 0x20,	//电池基本信息
		TAG_BAT_WORK_PARAM	= 0x21,	//电池工作参数

		TAG_BAT_TEMP		= 0x22,	// 电池温度数据
		TAG_BAT_FAULT		= 0x23,	// 电池故障数据
	}CMD0900_TLV;


#define		_SMART_STATE_SIZE		(128)	
#define 	BMS_ID_SIZE					6
#define 	MAX_BAT_COUNT				1

#pragma pack(1)	
/*
	Smart 板运行参数 格式如下：
	ip,port,hw,fw,url,conmin
*/	
	// PMS 板温度 INT16 (实际温度+40) + BYTE 电池在位状态
	typedef struct
	{
		int16 pmsTemp;	// 温度
		uint8 batState;
	}TlvPMSState;

	// 电池温度数据
	typedef struct
	{
		uint8	bid[BMS_ID_SIZE];
		uint16	cMostTemp;	// 充电MOS温度
		uint16	dMostTemp;	// 放电MOS温度
		uint16	fuelTemp;	// 电量计温度
		uint16	contTemp;	// 连接器温度
		uint16	batTemp1;	// 电池1温度
		uint16	batTemp2;
		uint16	tvsTemp;	// tvs 温度
		uint16	fuseTemp ;	// 保险丝温度
	}TlvBatTemp;

	//电池故障数据
	typedef struct
	{
		uint8	bid[BMS_ID_SIZE];
		uint16	devft1 ;	// 故障1
		uint16	devft2;
		uint16 	opft1 ;
		uint16 	opft2 ;
		uint16 	opwarn1 ;
		uint16	opwarn2 ;
	}TlvBatFault;
	

	typedef struct _TlvBatInfo
	{
		uint8 bid[BMS_ID_SIZE];
		uint8 port;
		uint16 nominalVol;
		uint16 nominalCur;
		//
		uint8 HwMainVer;	// 硬件主板本号
		uint8 HwSubVer;		// 硬件子版本号
		uint8 FwMainVer;	// 固件主版本号
		uint8 FwSubVer;		// 固件了版本号
		uint8 FwRevVer;		// 固件修订版本号
		uint32 FwBuildNunber;// 固件子版本号
	}TlvBatInfo;

	typedef struct _TlvBatWorkInfo
	{
		uint8 bid[BMS_ID_SIZE];
		uint8 soc;
		uint16 voltage;
		int16 current;
		uint16 maxCellVol;
		uint16 minCellVol;
		uint8 maxCellNum;
		uint8 minCellNum;

		uint16 maxChgCurr;
		uint16 maxDischgCurr;
		uint16 curWorkFeature;

		uint16 cycCount;
	}TlvBatWorkInfo;

#pragma pack()	

	void JtTlv0900_init(void);
	int JtTlv0900_getChangedTlv(uint8* buf, int len, uint8* tlvCount);
	//void JtTlv0900_updateMirror(const uint8* data, int len);
	//void JtTlv0900_updateStorage(const uint8* data, int len);
	//void JtTlv0900_updateMirror(void);
	void JtTlv0900_updateStorage(void);
	void JtTlv0900_updateMirror(void);
	void JtTlv0900_Cache( uint8*,uint8 );	// 获取缓存数据
	//
	void JtTlv0900_updateBatInfo(uint8 port);	// 更新电池基本信息
	void JtTlv0900_updateBatWorkInfo(uint8 port);	// 更新工作数据
	void JtTlv0900_updateBatTemperature(uint8 port); // 更新温度数据
	void JtTlv0900_updateBatFault(uint8 port);

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


