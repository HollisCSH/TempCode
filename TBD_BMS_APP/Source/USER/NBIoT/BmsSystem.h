//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: BmsSystem.h
//创建人  	: 
//创建日期	:
//描述	    : Sim模组关于电池BMS操作头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================
#ifndef __SMART_SYSTEM_H__
#define __SMART_SYSTEM_H__

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SimCommon.h"
#ifdef __cplusplus
extern "C"
{
#endif

//=============================================================================================
//宏参数设定
//=============================================================================================
#define MAX_BATTERY_COUNT   2
#define BMS_DEVICE_TYPE     0x03
#define BAT_ERR_SIZE 		6
#define SERIAL_NUM_SIZE		6

//=============================================================================================
//定义数据类型
//=============================================================================================
//电池信息
typedef struct _Battery
{
	uint8  port; 			//
	
	uint8  isPresent:1; 	//is in port
	uint8  isCharged:1; 		
	uint8  Reserved:6; 		

	//Battery properties
	uint8  serialNum[SERIAL_NUM_SIZE];
	uint8  soc;
	uint8  soh;
	uint8  temperature;
	uint16 voltage;
	uint16 current;
	uint16 cycleCount; 	//
	uint16 capacity; 	//
	
	uint8  batteryErr[BAT_ERR_SIZE]; //故障码
}Battery;

//App file info area
#pragma pack(1) 
//版本号
typedef struct _VerDesc
{
	uint8 m_HwMainVer;
	uint8 m_HwSubVer;
	
	uint8  m_AppMainVer;
	uint8  m_AppSubVer;
	uint8  m_AppMinorVer;
    u32 m_AppBuildeNum;    
    //uint8  m_AppBuildeNum[4];
	
}VerDesc;

typedef struct _DataRom
{
	//Only write once section, 64 bytes
	uint8 m_Version;	//DataArea version.
	uint8 m_HwMainVer;
	uint8 m_HwSubVer;
	uint8 m_isUpdateFw;
	char  m_IpAddr[16];	//"255.255.255.255"
	uint16  m_IpPort;	
	
	uint8  m_isFactoryMode:1;
	uint8  m_isSlience:1;	//=1时，处于静默状态,蜂鸣器或者喇叭都不会响,用于RTC定时复位系统，设备必须保持静默，不能被用户感知
	uint8  m_reserved0:6;
	uint8 m_reserved1[9];

	//Updated with app upgrade, 64 bytes
	uint32 m_AppFileLen;
	uint32 m_AppFileCrc;
	
	uint8  m_AppMainVer;
	uint8  m_AppSubVer;
	uint8  m_AppMinorVer;
	uint32 m_AppBuildeNum;
	
	#if 1
	uint8  m_OlderAppMainVer;
	uint8  m_OlderAppSubVer;
	uint8  m_OlderAppMinorVer;
	uint32 m_OlderAppBuildeNum;
	#endif
	
	uint8  m_reserved2[8];
}DataRom;
#pragma pack() 

typedef struct 
{
	uint8 type; //0x00: invalid, 0x01: admin, 0x02:guest, 
	char userID[USERID_LEN+1];	
}UserInfo;

typedef struct
{
	char admin[MD5_KEY_LEN+1];
	char guest[MD5_KEY_LEN+1];
}MD5_Key;

typedef struct
{
	char iccid[CCID_LEN+1];
	uint16 CC;  //Country Code
	uint8  csq;
}SimCard_Info;

typedef struct
{
	uint8 maxSpeed;
}ModeSetting;

typedef enum _ErrType
{
	 ET_NONE = 0
	,ET_FAULT	//故障，LCD上显示"!"
	,ET_ALARM	//告警
}ErrType;


typedef struct
{
	uint8 systemError;
	uint8 gpsError;	
	uint16 motorError;
	uint32 batteryError;
	uint8 gprsError;
	uint8 peripheralError;
	uint8 reserve;
	uint8 lcdErrorcode;
}ErrorCode;

#pragma anon_unions
#pragma pack(1) 
typedef struct 
{	
	uint8 mac[6];
//	Bool  isFlashOk; //Flash is OK;
//	Bool  isGyroOk;  //陀螺仪 is OK;
//	
//	/*Settings*/
//	uint8 settingVer;	//settin版本号，必须放在第一个字节,如果与保存的settinVer版本号不一致，会导致删除保存的数据而使用默认值
//	
//	uint8 isLockTest:1;	//锁测试
//	uint8 isRemoteAccOn:1;	//远程控制ACC
//	uint8 reserved10:6;
//	
//	uint8 isLock:1;		//是否锁车
//	uint8 isGyroAssert:1;	//陀螺仪异动
	uint8 isMoving:1;		//是否在移动
//	uint8 isVolValid:1;
//	uint8 canNotUsed:1;		//该位不能被使用，默认值为1
//	uint8 reserved11:3;
//#ifdef CFG_NVC	
//	uint8 LoSocPlayFlag[3];	//低电量播报标志
//	uint16 SocPlayFlag;		//电量播报标志
//#endif	
//#ifdef CFG_LOGIN_TIME	
	union 
	{
		uint8 devcfg;
		struct
		{
		uint8 IsDeactive:1;			//是否去激活,兼容以前版本
		uint8 IsSignEnable:1;	 	//登记打卡功能使能
		uint8 IsForbidDischarge:1;	//是否禁止放电
		uint8 IsAlarmMode:1;		//是否境界模式
		uint8 IsBatVerifyEn:1;		//是否使能电池身份验证
		uint8 reserved01:3;
		};
	};

	uint32 loginMaxMinute;		//打卡最长时间，分钟为单位,
	uint32 remainSeconds;		//打卡剩余时间，分钟为单位,
//	uint8 vol;					//音量
//	//uint16 StopTime;			//停车判定条件-小电流持续时间
//	//uint16 StopCurrent;			//停车判定条件-小电流阈值
//	uint8 reserved1[6];//保留
//#else
//	uint8 reserved1[16];//保留
//#endif
//	uint8 settingEnd;	//setting存储结束位置,该值没有意义，仅用其位置信息，计算保存的数据区块大小,不会存进Flash中
//	
//	//misc sector
//	uint8 miscVer;		//misc版本号，必须放在第一个字节,如果与保存的miscVer版本号不一致，会导致删除保存的数据而使用默认值。
//	char sID[SID_LEN+1];
//	MD5_Key localKey;	//保存在Flash的另外一个扇区
	
//	uint8 isModifiedMfInfo:1;
//	uint8 isGetBtMac:1;
//	uint8 reserved2:6;
//	
//	VerDesc pmsVer;		//不能直接声明为结构类型，否则会存在字节对齐问题
//	uint32  pmsFwLen;	//pms固件长度,0表示没有待更新的固件
//	uint8 	isBleUpg;	//是否蓝牙升级固件
//	uint32 	bmsFwLen;	//bms固件长度
//	uint8 reserved3[7];//保留
//	uint8 miscEnd;		//misc存储结束位置,该值没有意义，仅用其位置信息，计算保存的数据区块大小,不会存进Flash中
	
}ScooterSettings;
#pragma pack() 

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern  ScooterSettings  g_Settings;

//=============================================================================================
//声明接口函数
//=============================================================================================

//禁止放电
void SetForbidDischarge(Bool isEnable);

//获取是否进制放电标志
Bool IsForbidDischarge(void);

//设置激活标志
void SetActive(Bool isActive);

//获取激活标志
Bool IsActive(void);

//设置错误标志
void SetErrorCode(uint8 err_type, uint32 errcode, ErrType errType);

//获取错误码
uint8 GetErrorCode(uint8 err_type);

//根据错误码确定处于警戒状态
Bool ErrorCode_IsAlarm(uint8 errcode);

//获取警戒标志
Bool IsAlarmMode(void);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************end of BmsSystem.h*****************************************/
