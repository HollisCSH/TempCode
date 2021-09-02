
#ifndef  _JT_TLV_8103_H_
#define  _JT_TLV_8103_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus
#include "BSPTypeDef.h"
#include "utp.h"
#include "TlvIn.h"
#include "TlvOut.h"
	
	typedef struct _JtDevCfgParam
	{
		uint32 hbIntervalS   ;		
		uint32 tcpWaitRspTime;
		uint32 tcpReTxCount  ;
		char   mainSvrUrl[256]    ;
		uint32 mainSvrPort   ;
		uint32 locReportWay;
		uint32 locReportPlan;
		uint32 sleepLocReportInterval;
		uint32 urgLocReportInterval;
		uint32 defLocReportInterval;
		uint32 gpsSampFre;
		uint32 devType;
		uint32 factoryFlag;
		uint8  devmodule[8];
	}JtDevCfgParam;

	//GPS_SAMPLING_FREQUENCY
	
	//TAG是大端格式
	typedef enum
	{
		TAG_HB_INTERVAL	   = 0x0100,		//终端心跳发送间隔，单位为秒(s)
		TAG_TCP_RSP_TIME   = 0x0200,		//TCP 消息应答超时时间，单位为秒(s)
		TAG_TCP_RETX_COUNT = 0x0300,		//TCP 消息重传次数
		TAG_MAIN_SVR_URL   = 0x1300,		//主服务器地址， IP 或域名
		TAG_MAIN_SVR_PORT  = 0x1800,		//主服务器端口
		TAG_LOC_REPORT_WAY		= 0x2000,	//位置汇报策略， 0：定时汇报； 1：定距汇报； 2：定时和定距汇报。
		TAG_LOC_REPORT_PLAN		= 0x2100,	//位置汇报方案， 0：定时汇报； 1：定距汇报； 2：定时和定距汇报。
		TAG_SLEEP_LOC_INTERVAL	= 0x2700,	//休眠时汇报时间间隔，单位为秒(s),>0。
		TAG_URG_LOC_INTERVAL	= 0x2800,	//紧急时汇报时间间隔，单位为秒(s),>0。
		TAG_DEF_LOC_INTERVAL	= 0x2900,	//默认汇报时间间隔，单位为秒(s),>0。
		TAG_GPS_SAMPL_FREQUENCY	= 0x2A00,	// GPS采样周期
		TAG_DEVICE_TYPE		 = 0x3000,		//设备类型。
		TAG_FACTORY_CFG_FLAG = 0x3100,		//出厂配置标志，0-出厂配置，1-非出厂配置。
		TAG_DEVICE_MODLE 	= 0x3200		// 终端型号
		//未完待续。。。
	}CMD8103_TLV;

	void JtTlv8103_init(void);
	void JtTlv8103_updateMirror(const uint8* data, int len);
	int JtTlv8103_getChanged(uint8* buf, int len, uint8* tlvCount);
	int JtTlv8103_getDefChanged(uint8* buf, int len);
	void JtTlv8103_updateStorage(const uint8* data, int len);
	uint32 JtTlv8103_getFactoryCofnig(void);

	extern TlvInMgr  g_jtTlvInMgr_8103;
	extern TlvOutMgr g_jtTlvOutMgr_8103;

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


