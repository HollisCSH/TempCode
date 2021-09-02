
#ifndef  _JT808_H_
#define  _JT808_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus
#include "BSPTypeDef.h"
#include "HwFwVer.h"
#include "JtTlv8103.h"

	typedef enum
	{
		JTCMD_SIM_HB		= 0x01,
		JTCMD_MCU_HB		= 0x02,
		JTCMD_SET_OP_STATE	= 0x03,

		JTCMD_CMD_GET_SIM_ID  = 0x11,
		JTCMD_CMD_GET_SIM_CFG,
		JTCMD_CMD_SET_SIM_CFG,
		JTCMD_CMD_GET_SMS,
		JTCMD_CMD_SEND_TO_SVR = 0x15,

		JTCMD_CMD_GET_FILE_INFO = 0X16,

		JTCMD_CMD_GET_FILE_CONTENT = 0X17,
		JTCMD_CMD_SET_LOCATION_EXTRAS = 0x18,
		JTCMD_CMD_BEEP_CTRL			= 0x1C,

		JTCMD_CMD_GET_BLE_ID = 0x30,
		JTCMD_CMD_GET_BLE_CFG,
		JTCMD_CMD_SET_BLE_CFG,
		JTCMD_CMD_GET_BLE_EN,
		JTCMD_CMD_SET_BLE_EN,
		
		JTCMD_EVENT_DEV_STATE_CHANGED = 0x80,
		JTCMD_EVENT_DEV_STATE_LOCATION,

		JTCMD_BLE_EVT_AUTH = 0x90,
		JTCMD_BLE_EVT_CNT = 0x91,
		JTCMD_BLE_EVT_BEACON = 0x92,
		JTCMD_BLE_RCV_DAT = 0x93,

		JTCMD_EVT_RCV_SVR_DATA =  0xA0,
		JTCMD_EVT_RCV_FILE_DATA ,
	}JT_cmd;

	typedef enum
	{
		JT_STATE_INIT			= 0x00,
		JT_STATE_SLEEP			= 0x01,
		JT_STATE_WAKEUP			= 0x02,
		JT_STATE_PREOPERATION	= 0x04,
		JT_STATE_OPERATION		= 0x05,

		JT_STATE_UNKNOWN		= 0xFF,
	}JT_state;


#define			_BIT(x,n)			((x) << (n))
#define			_SET_BIT(n)			_BIT(1,n)
	

// 0 ~ 1 GPRS状态
#define			_GPRS_POWEROFF		_BIT(0,0)	// GPRS 电源关闭
#define			_GPRS_LOWMODE		_BIT(1,0)	// GPRS 低功耗,TCP连接中断,短信可唤醒
#define			_GPRS_RESERVED		_BIT(2,0)	// 保留
#define			_GPRS_NORMAL		_BIT(3,0)	// 全功能
// 2 ~ 3 GPS状态
#define			_GPS_POWEROFF		_BIT(0,2)	// GPS 关闭
#define			_GPS_LOWMODE		_BIT(1,2)	// GPS 低功耗
#define			_GPS_RESERVED		_BIT(2,2)	// 保留
#define			_GPS_NORMAL			_BIT(3,2)	// 全功能
// 4 ~ 5 BLE 状态
#define			_BLE_POWEROFF		_BIT(0,4)	// 关闭BLE电源
#define			_BLE_LOWMODE		_BIT(1,4)	// BLE低功耗,广播,等待连接
#define			_BLE_RESERVED		_BIT(2,4)	// 保留
#define			_BLE_NORMAL			_BIT(3,4)	// 全功能
/*
模式				BLE	GPS	GPRS SMS
INIT			关	关	关	关
Preoperation	关	关	关	关
Operation		开	开	开	开
Wakeup			开	关	关	开
Sleep			开	关	关	开

*/
#define			_OPERATION_INIT		_BLE_POWEROFF | _GPS_POWEROFF | _GPRS_POWEROFF	
#define			_OPERATION_PRE		_BLE_POWEROFF | _GPS_POWEROFF | _GPRS_POWEROFF
#define			_OPERATION_OPE		_BLE_NORMAL | _GPS_NORMAL | _GPRS_NORMAL
#define			_OPERATION_WKUP		_BLE_LOWMODE | _GPS_POWEROFF | _GPRS_LOWMODE
#define			_OPERATION_SLEEP	_BLE_LOWMODE | _GPS_POWEROFF | _GPRS_LOWMODE



#define			_NETWORK_CONNECTION_BIT			_SET_BIT(0)
#define			_GPS_FIXE_BIT					_SET_BIT(1)
#define			_SMS_EXIT_BIT					_SET_BIT(2)

	

#pragma pack(1)

	typedef struct
		{
			uint8_t 	OperationState;
			uint8_t 	StateParameter;
		}JT_SetOperationState;
		

	typedef struct _JT_devState
		{
			uint16_t cnt;	// 设备状态-GPRS,GPS,SMS
			uint8_t csq;	// GPRS/4G 信号强度
			uint8_t snr;	// GPS 信噪比
			uint8_t siv;	// GPS 可见星
		}JT_devState;

	//????μ?éè±?ê?D??¨ò?
#define JT_DEV_HW_VER_SIZE 10
#define JT_DEV_FW_VER_SIZE 20
	typedef struct _JtDevProperty
	{
		uint16_t  protocolVer;	//协议版本号
		uint32_t devClass;	//终端类型
		uint8_t  vendorID[5];	//制造商ID
		uint8_t  devModel[8];// [8] 终端型号
		uint8_t  devId[12];	//终端ID
		uint8_t  iccid[10];	//ICCID
		uint8_t  hwVer[JT_DEV_HW_VER_SIZE];	//硬件版本号
		uint8_t  fwVer[JT_DEV_FW_VER_SIZE];	//固件版本号
	}JtDevProperty;
	// Jose add 2020/09/17
	// ????à??à
#define		JT_DEV_BLE_MAC_ADDR_SIZE	6
#define		JT_DEV_BLE_HW_VER_SIZE		16
#define		JT_DEV_BLE_FW_VER_SIZE		16

	typedef struct _JtDevBleProperty
	{
		uint16_t BleType ;
		uint8_t  BleMac[JT_DEV_BLE_MAC_ADDR_SIZE];
		uint8_t  BlehwVer[JT_DEV_BLE_HW_VER_SIZE];
		uint8_t  BlefwVer[JT_DEV_BLE_FW_VER_SIZE];
	}JtDevBleProperty;
	
	// Jose add 2020/09/17
	typedef struct _JtDevBleCfgParam
	{
		uint8_t BleName[32];		// 蓝牙名
		uint32_t BleAdvInterval ;	// 广播间隔(1ms)
		uint8_t BleAdvPower ;		// 发射功率
		uint8_t BleAdvData[62];		// 广播&回应 数据
	}JtDevBleCfgParam;

	typedef struct 
	{
		uint32_t Updatefilelength ;
		uint8_t  UpdatefileVersion;
	}UpdateFileInfo;

	typedef struct
	{
		uint8_t	fileType;
		uint32_t 	fileOffset;
	}FileContent;

	// ?¨??×?±ê
	typedef struct
	{
		int32_t longitude ;	// 经度 x 10^7
		int32_t latitude ;	// 纬度 x 10^7
	}Jt_LocationData;

#define		_BLE_CONNECT_BIT		_SET_BIT(0)
	// ble×′ì?
	typedef struct
	{
		uint8_t bleConnectState ;
		uint8_t bleConnectMAC[6];
	}Jt_BleState;

// ???óéè±?×′ì??¨ò?
#define			_STATE_ACC				_SET_BIT(0)		// ACC 
#define			_STATE_WHEL_LLOCK		_SET_BIT(1)		// ??ì±
#define			_STATE_CAB_LOCK			_SET_BIT(2)		//	×ù2???×′ì?
#define			_STATE_REPOWER_OFF		_SET_BIT(3)		//  ??3ì??μ?
#define			_STATE_ACTIVE			_SET_BIT(4)		//	?¤??×′ì?
#define			_STATE_BAT_AUTH_EN		_SET_BIT(5)		//	?¤??×′ì?
#define			_STATE_ALAM_EN			_SET_BIT(6)		//	ò??ˉ

#define		_SMS_SIZE		(140)
	typedef struct
	{
		uint32 	smsRevTime;
		uint8	smsLen;
		uint8	smsText[_SMS_SIZE];
	}SMSContext;
	#define		_SMS_ARRY_CNT	(2)
	typedef struct
	{
		uint8 		smsExist;
		uint8 		smsCount;
		SMSContext	smsText;//[_SMS_ARRY_CNT];
	}GetSMSContext;

	typedef struct
	{
		uint16 				beepFreq;		// 20 ~ 20000hz
		uint8				beepDuty;		// 占空比
		uint16				beepOntim;		// 响时间 ms
		uint16				beepOfftim;		// 停时间 ms
		uint16				beepCycles;		// 循环次数
	}_BeepCtrl;
	
// ???ó±¨?ˉ×′ì??¨ò?
	// NFC message
	typedef struct
	{
		uint8 	Activ;			// 1：定位,0:未定位
		uint8 	SNR ;			// 信噪比
		uint8 	StateView;		// 
		int32 	longitude ;		// 经度 x 10^7
		int32 	latitude ;		// 纬度 x 10^7
		uint32	ActiveTimeCnt ; // 定位时间ms---当前定位时有效果 0--表示无效
	}GPSMSGInfo , *pGPSMSGInfo;
	//
	typedef struct
	{
		uint8 	connected;		// 1:已连接,0:未连接
		uint8 	CSQ;			// 信号值
		uint8 	sms;			// 是否存在短信
		uint32	ConnectTimeCnt;	// GPRS连接时间ms
		uint32 	TranMaxTimeCnt; // GPRS发送到接收数据的最长时间
		uint32  TranMinTimeCnt;	// GPRS发送到接收数据的最小时间
	}GPRSMSGInfo , *pGPRSMSGInfo;
	
	typedef struct
	{
		uint8 	connect ;
		
	}BLEMSGInfo , *pBLEMSGInfo ;



	// Jose add end
	
#pragma pack()

	typedef void (*JT808fsmFn)(uint8_t msgID, uint32_t param1, uint32_t param2);
	typedef struct _JT808
	{
		//μ±?°JT808μ?2ù×÷×′ì?
		JT_state opState;
		//éè??JT808μ?2ù×÷×′ì?
		//JT_state setToOpState;
		JT_SetOperationState setToOpState;

		JT_devState devState;

		Jt_LocationData locatData ;
		
		//ê?·??¨??3é1|
		//Bool isLocation;
		
		//????ê?D?êy?Y
		JtDevProperty property;
		// bleê?D?
		Jt_BleState		bleState ;
		uint16_t 		bleEnCtrl;
		JtDevBleProperty bleproperty ;
		JtDevBleCfgParam blecfgParam ;

		GetSMSContext	smsContext ;
		
		// ???tê?D?
		UpdateFileInfo  updatefileinfo ;
		FileContent		filecontent ;
		//

		_BeepCtrl		beepCtrl ;

		JT808fsmFn fsm;

		//
		GPSMSGInfo		gpsmsgInfo;
		GPRSMSGInfo		gprsmsgInfo;
		BLEMSGInfo		BLEmsgInfo;

		// 时间计时
	}JT808;

	extern JT808* g_pJt;
	extern JT808 g_Jt;
	void JT808_init(void);
	void JT808_rxDataProc(const uint8_t* pData, int len);

// Jose 外置模块运行状态

#define		_BLE_TYPE			_SET_BIT(0)	// 蓝牙
#define		_PMS_TYPE			_SET_BIT(1)	// PMS
#define		_ACC_TYPE			_SET_BIT(2)	// 物理点火键
#define		_LOCL_TYPE			_SET_BIT(3)	// 座舱锁
#define		_CABIN_TYPE			_SET_BIT(4)	// 轮毂锁

// 终端类型
#define		_DEV_TYPE			_BLE_TYPE


// 终端型号
#ifdef BMS_BAT_26V_14S_7P_EVE
#define		_DEV_MODEL		"B12"
#elif 		defined(BMS_BAT_32E_17S_6P_SAM)
#define		_DEV_MODEL		"B10"
#elif 		defined(LFP_AK_15000MAH_16S)
#define		_DEV_MODEL		"B11"
#elif 		defined(LFP_HL_25000MAH_16S)
#define		_DEV_MODEL		"B19"
#elif 		defined(SY_PB_32000MAH_14S)
#define		_DEV_MODEL		"B21"
#elif 		defined(SY_PB_32000MAH_17S) || defined(SY_PB_32000MAH_16S)
#define		_DEV_MODEL		"B22"
#elif 		defined(LFP_TB_20000MAH_20S)
#define		_DEV_MODEL		"B20"
#endif


// 蓝牙名称
//BLE name
//#ifdef BMS_BAT_26V_14S_7P_EVE
//#define		_BLE_NAME		"IMT-B12-%02X%02X%02X"		//IMT-B9-xxxxxx
//#elif 		defined(BMS_BAT_32E_17S_6P_SAM)
//#define		_BLE_NAME		"IMT-B10-%02X%02X%02X"		//IMT-B9-xxxxxx
//#endif
#define		_BLE_NAME			"IMT-%s-%02X%02X%02X"
#define		_BLE_COMPY		"30"




#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


