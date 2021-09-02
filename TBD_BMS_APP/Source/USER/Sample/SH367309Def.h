//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SH367309Def.h
//创建人  	: Handry
//创建日期	: 
//描述	    : SH367309驱动定义头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef SH367309_DEF_H_
#define SH367309_DEF_H_

//=============================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"

//=============================================================================================
//宏参数设定
//=============================================================================================

//----------------------------AFE EEP内部相关---------------------------------------------------
#define     SH367309_EEP_SCONF1			    0x00
#define     SH367309_EEP_SCONF2		        0x01
#define     SH367309_EEP_OVH		        0x02
#define     SH367309_EEP_OVL		        0x03
#define     SH367309_EEP_OVRH		        0x04
#define     SH367309_EEP_OVRL		        0x05
#define     SH367309_EEP_UV		            0x06
#define     SH367309_EEP_UVR			    0x07
#define     SH367309_EEP_BALV			    0x08
#define     SH367309_EEP_PREV				0x09
#define     SH367309_EEP_LOV			    0x0A
#define     SH367309_EEP_PFV			    0x0B
#define     SH367309_EEP_OCD1		        0x0C
#define     SH367309_EEP_OCD2		        0x0D
#define     SH367309_EEP_SC			        0x0E
#define     SH367309_EEP_OCC			    0x0F
#define     SH367309_EEP_MOST_OCRT_PFT		0x10
#define     SH367309_EEP_OTC			    0x11
#define     SH367309_EEP_OTCR			    0x12
#define     SH367309_EEP_UTC		        0x13
#define     SH367309_EEP_UTCR		        0x14
#define     SH367309_EEP_OTD			    0x15
#define     SH367309_EEP_OTDR		        0x16
#define     SH367309_EEP_UTD			    0x17
#define     SH367309_EEP_UTDR			    0x18
#define     SH367309_EEP_TR			        0x19

//----------------------------寄存器配置相关---------------------------------------------------
#define     SH367309_REG_CONF			    0x40
#define     SH367309_REG_BALANCEH		    0x41
#define     SH367309_REG_BALANCEL		    0x42
#define     SH367309_REG_BSTATUS1		    0x43
#define     SH367309_REG_BSTATUS2		    0x44
#define     SH367309_REG_BSTATUS3		    0x45
#define     SH367309_REG_TEMP1			    0x46
#define     SH367309_REG_TEMP2			    0x48
#define     SH367309_REG_TEMP3			    0x4A
#define     SH367309_REG_CUR				0x4C
#define     SH367309_REG_CELL1			    0x4E
#define     SH367309_REG_CELL2			    0x50
#define     SH367309_REG_CELL3			    0x52
#define     SH367309_REG_CELL4			    0x54
#define     SH367309_REG_CELL5			    0x56
#define     SH367309_REG_CELL6			    0x58
#define     SH367309_REG_CELL7			    0x5A
#define     SH367309_REG_CELL8			    0x5C
#define     SH367309_REG_CELL9			    0x5E
#define     SH367309_REG_CELL10			    0x60
#define     SH367309_REG_CELL11			    0x62
#define     SH367309_REG_CELL12			    0x64
#define     SH367309_REG_CELL13			    0x66
#define     SH367309_REG_CELL14			    0x68
#define     SH367309_REG_CELL15			    0x6A
#define     SH367309_REG_CELL16			    0x6C
#define     SH367309_REG_ADC2			    0x6E
#define     SH367309_REG_BFLAG1			    0x70
#define     SH367309_REG_BFLAG2			    0x71
#define     SH367309_REG_RSTSTAT			0x72

//=============================================================================
//数据结构声明
//=============================================================================
typedef struct _EEPROM_Group
{
    //00H
    union
	{
		struct
		{
			u8 CN          	    :4;	//串数配置控制位
			u8 BAL	 	        :1; //平衡功能模块使能控制位
			u8 OCPM        	    :1; //充放电过流MOSFET控制位
			u8 ENMOS		 	:1; //充电MOSFET恢复控制位
			u8 ENPCH	 		:1; //预充电模块控制位
		}SCONF1Bit;
		u8 SCONF1Byte;
	}SCONF1;
    
    //01H
    union
	{
		struct
		{
			u8 EUVR          	:1;	//过放电恢复设置控制位
			u8 OCRA          	:1; //电流恢复设置控制位
			u8 CTLC           	:2; //CTL管脚功能设置控制位
			u8 DIS_PF	 	    :1; //二次过充电模块使能控制位
			u8 UV_OP        	:1; //过放电时MOSFET控制位
			u8 RSVD		 	    :1; //保留
			u8 EOVB	 		    :1; //禁止低压电芯充电功能设置控制位
		}SCONF2Bit;
		u8 SCONF2Byte;
	}SCONF2;    
    
    //02H
    union
	{
		struct
		{
			u8 OV8_9          	:2;	
			u8 LDRT          	:2; //负载释放延时设置控制位
			u8 OVT           	:4; //过充电保护延时设置控制位
		}OVHBit;
		u8 OVHByte;
	}OVH;       
    
    //03H
    union
	{
		struct
		{
			u8 OV0_7          	:8;	//过充电保护电压，计算方式：寄存器值 X 5mV
		}OVLBit;
		u8 OVLByte;
	}OVL;           

    //04H
    union
	{
		struct
		{
			u8 OVR8_9          	:2;	
			u8 RVSD          	:2; //保留
			u8 UVT           	:4; //过放电保护延时设置控制位
		}OVRHBit;
		u8 OVRHByte;
	}OVRH;     
    
    //05H
    union
	{
		struct
		{
            u8 OV0_7          	:8;	//过充电恢复电压，计算方式：寄存器值 X 5mV
		}OVRLBit;
		u8 OVRLByte;
	}OVRL;        
    
    //06H
    union
	{
		struct
		{
            u8 UV0_7          	:8;	//过放电保护电压，计算方式：寄存器值 X 20mV
		}UVBit;
		u8 UVByte;
	}UV;         

    //07H
    union
	{
		struct
		{
            u8 UVR0_7          	:8;	//过放电保护电压，计算方式：寄存器值 X 20mV
		}UVRBit;
		u8 UVRByte;
	}UVR;     
    
    //08H
    u8 BALVByte; //平衡开启电压设置寄存器 计算方式：寄存器值 X 20mV
    
    //09H
    u8 PREVByte; //预充电电压设置寄存器 计算方式：寄存器值 X 20mV
    
    //0AH
    u8 L0VByte; //低电压禁止充电电压设置寄存器 计算方式：寄存器值 X 20mV    
    
    //0BH
    u8 PFVByte; //二次过充电保护电压设置寄存器 计算方式：寄存器值 X 20mV        
    
    //0CH
    union
	{
		struct
		{
            u8 OCD1T          	:4;	//放电过流1保护延时设置控制位
            u8 OCD1V          	:4;	//放电过流1保护电压设置控制位
		}OCD1Bit;
		u8 OCD1Byte;
	}OCD1;         
    
    //0DH
    union
	{
		struct
		{
            u8 OCD2T          	:4;	//放电过流2保护延时设置控制位
            u8 OCD2V          	:4;	//放电过流2保护电压设置控制位
		}OCD2Bit;
		u8 OCD2Byte;
	}OCD2;           
    
    //0EH
    union
	{
		struct
		{
            u8 SCT          	:4;	//短路保护延时设置控制位
            u8 SCV          	:4;	//短路保护电压设置控制位
		}SCBit;
		u8 SCByte;
	}SC;
    
    //0FH
    union
	{
		struct
		{
            u8 OCCT          	:4;	//充电过流保护延时设置控制位
            u8 OCCV          	:4;	//充电过流保护电压设置控制位
		}OCCBit;
		u8 OCCByte;
	}OCC;    
    
    //10H
    union
	{
		struct
		{
            u8 PFT          	:2;	//二次过充点保护延时设置
            u8 OCRT          	:2;	//充放电过流自恢复延时设置
            u8 MOST          	:2;	//充放电MOSFET开启延时设置       
            u8 CHS          	:2;	//充放电状态检测电压设置
		}MOST_OCRT_PFTBit;
		u8 MOST_OCRT_PFTByte;
	}MOST_OCRT_PFT;    
    
    //11H
    u8 OTCByte;  //充电高温保护阈值
    
    //12H
    u8 OTCRByte; //充电高温保护释放阈值
    
    //13H
    u8 UTCByte;  //放电低温保护阈值 
    
    //14H
    u8 UTCRByte; //充电低温保护释放阈值 
    
    //15H
    u8 OTDByte;  //放电高温保护阈值
    
    //16H
    u8 OTDRByte; //放电高温保护释放阈值
    
    //17H
    u8 UTDByte;  //放电低温保护阈值 
    
    //18H
    u8 UTDRByte; //放电低温保护释放阈值     
    
    //19H
    union
	{
		struct
		{
            u8 TR0_6          	:7;	//温度内部参考电阻系数
            u8 RVSD          	:1;	//保留
		}TRBit;
		u8 TRByte;
	}TR;    
    
}
t_SH367309_EEPRom;

//----------------------------AFE EEP内部配置---------------------------------------------------
//SCONF1 ENPCH
//启用预充电功能
#define     SH367309_EEP_ENPCH_ENABLE	    0x01	
//禁用预充电功能
#define     SH367309_EEP_ENPCH_DISABLE	    0x00    

//SCONF1 ENMOS
//启用充电MOSFET恢复控制位。当过充电/温度保护关闭充电MOSFET后，如 果检测到放电过流1或者放电状态，则开启充电MOSFET
#define     SH367309_EEP_ENMOS_ENABLE	    0x01	
//禁用充电MOSFET恢复控制位
#define     SH367309_EEP_ENMOS_DISABLE	    0x00    

//SCONF1 OCPM
//充放电过流关闭充放电MOSFET
#define     SH367309_EEP_OCPM_ENABLE	    0x01	
//充电过流只关闭充电MOSFET；放电过流只关闭放电MOSFET
#define     SH367309_EEP_OCPM_DISABLE	    0x00

//SCONF1 BAL
//平衡开启由SH367309内部逻辑控制
#define     SH367309_EEP_BAL_INTERNAL	    0x00	
//平衡开启由外部MCU控制，平衡时序仍由SH367309内部逻辑控制
#define     SH367309_EEP_BAL_MCU	        0x01

//SCONF1 CN
//注释:SH367309兼容5-16串锂电池保护,当串数设置小于16串数,不使用的电 芯输入端（靠VBT）短接至最高串电芯的正端即可
#define     SH367309_EEP_CELL5			    0x05	
#define     SH367309_EEP_CELL6				0x06	
#define     SH367309_EEP_CELL7				0x07	
#define     SH367309_EEP_CELL8				0x08	
#define     SH367309_EEP_CELL9				0x09	
#define     SH367309_EEP_CELL10				0x0A	
#define     SH367309_EEP_CELL11				0x0B	
#define     SH367309_EEP_CELL12				0x0C	
#define     SH367309_EEP_CELL13				0x0D	
#define     SH367309_EEP_CELL14				0x0E	
#define     SH367309_EEP_CELL15				0x0F	
#define     SH367309_EEP_CELL16				0x00	

//SCONF2 E0VB
//开启“禁止低压电芯充电”功能
#define     SH367309_EEP_E0VB_ENABLE	    0x01	
//关闭“禁止低压电芯充电”功能
#define     SH367309_EEP_E0VB_DISABLE	    0x00

//SCONF2 UV_OP
//过放电关闭充放电MOSFET
#define     SH367309_EEP_UV_OP_ENABLE	    0x01	
//过放电只关闭放电MOSFET
#define     SH367309_EEP_UV_OP_DISABLE	    0x00

//SCONF2 DIS_PF
//启用二次过充电保护
#define     SH367309_EEP_DIS_PF_ENABLE	    0x00	
//禁止二次过充电保护
#define     SH367309_EEP_DIS_PF_DISABLE	    0x01

//SCONF2 CTLC
//充放电和预充电MOSFET由内部逻辑控制，CTL管脚输入无效
#define     SH367309_EEP_CTLC_INVALID	    0x00	
//控制充电和预充电MOSFET. CTL输入L电平时强制关闭充电和预充电MOSFET. CTL输入H电时充电和预充电 MOSFET由内部逻辑控制
#define     SH367309_EEP_CTLC_CHG_PRCH	    0x01	
//控制放电MOSFET. CTL输入L电平时强制关闭放电 MOSFET： CTL输入H电平时，放电MOSFET由内部逻辑控制
#define     SH367309_EEP_CTLC_DCHG	        0x02
//控制充放电和预充电MOSFET. CTL输入L电平时强制关闭充放电和预充电MOSFET；CTL输入H电平时，充放电和预充电MOSFET由内部逻辑控制
#define     SH367309_EEP_CTLC_ALL	        0x03

//SCONF2 OCRA
//允许电流保护定时恢复
#define     SH367309_EEP_OCRA_ENABLE	    0x01
//不允许电流保护定时恢复 
#define     SH367309_EEP_OCRA_DISABLE	    0x00

//SCONF2 EUVR
//过放电保护状态释放还需负载释放
#define     SH367309_EEP_EUVR_ENABLE	    0x01
//过放电保护状态释放与负载释放无关 
#define     SH367309_EEP_EUVR_DISABLE	    0x00

//OVH OVT
//过充电保护延时设置控制位
#define     SH367309_EEP_OVT_DLY_100MS	    0x00
#define     SH367309_EEP_OVT_DLY_200MS	    0x01
#define     SH367309_EEP_OVT_DLY_300MS	    0x02
#define     SH367309_EEP_OVT_DLY_400MS	    0x03
#define     SH367309_EEP_OVT_DLY_600MS	    0x04
#define     SH367309_EEP_OVT_DLY_800MS	    0x05
#define     SH367309_EEP_OVT_DLY_1S	        0x06
#define     SH367309_EEP_OVT_DLY_2S	        0x07
#define     SH367309_EEP_OVT_DLY_3S	        0x08
#define     SH367309_EEP_OVT_DLY_4S	        0x09
#define     SH367309_EEP_OVT_DLY_6S	        0x0A
#define     SH367309_EEP_OVT_DLY_8S	        0x0B
#define     SH367309_EEP_OVT_DLY_10S	    0x0C
#define     SH367309_EEP_OVT_DLY_20S	    0x0D
#define     SH367309_EEP_OVT_DLY_30S	    0x0E
#define     SH367309_EEP_OVT_DLY_40S	    0x0F

//OVH LDRT
//负载释放延时设置控制位
#define     SH367309_EEP_LDRT_DLY_100MS	    0x00
#define     SH367309_EEP_LDRT_DLY_500MS	    0x01
#define     SH367309_EEP_LDRT_DLY_1000MS	0x02
#define     SH367309_EEP_LDRT_DLY_2000MS	0x03

//过充电保护电压，计算方式：寄存器值 X 5mV
#define     SH367309_EEP_OV_GAIN	        5

//OVRH UVT
//过放电保护延时设置控制位
#define     SH367309_EEP_UVT_DLY_100MS	    0x00
#define     SH367309_EEP_UVT_DLY_200MS	    0x01
#define     SH367309_EEP_UVT_DLY_300MS	    0x02
#define     SH367309_EEP_UVT_DLY_400MS	    0x03
#define     SH367309_EEP_UVT_DLY_600MS	    0x04
#define     SH367309_EEP_UVT_DLY_800MS	    0x05
#define     SH367309_EEP_UVT_DLY_1S	        0x06
#define     SH367309_EEP_UVT_DLY_2S	        0x07
#define     SH367309_EEP_UVT_DLY_3S	        0x08
#define     SH367309_EEP_UVT_DLY_4S	        0x09
#define     SH367309_EEP_UVT_DLY_6S	        0x0A
#define     SH367309_EEP_UVT_DLY_8S	        0x0B
#define     SH367309_EEP_UVT_DLY_10S	    0x0C
#define     SH367309_EEP_UVT_DLY_20S	    0x0D
#define     SH367309_EEP_UVT_DLY_30S	    0x0E
#define     SH367309_EEP_UVT_DLY_40S	    0x0F

//过充电恢复电压，计算方式：寄存器值 X 5mV
#define     SH367309_EEP_OVR_GAIN	        5

//过放电保护电压，计算方式：寄存器值 X 20mV
#define     SH367309_EEP_UV_GAIN	        20

//过放电恢复电压，计算方式：寄存器值 X 20mV
#define     SH367309_EEP_UVR_GAIN	        20

//平衡开启电压，计算方式：寄存器值 X 20mV
#define     SH367309_EEP_BALV_GAIN	        20

//预充电电压，计算方式：寄存器值 X 20mV
#define     SH367309_EEP_PREV_GAIN	        20

//低压禁止充电电压，计算方式：寄存器值 X 20mV
#define     SH367309_EEP_L0V_GAIN	        20

//二次过压充电保护电压，计算方式：寄存器值 X 20mV
#define     SH367309_EEP_PFV_GAIN	        20

//OCD1 OCD1V
//放电过流1保护 电压 / 分流器阻值
#define     SH367309_EEP_OCD1V_20MV	        0x00
#define     SH367309_EEP_OCD1V_30MV	        0x01
#define     SH367309_EEP_OCD1V_40MV	        0x02
#define     SH367309_EEP_OCD1V_50MV	        0x03
#define     SH367309_EEP_OCD1V_60MV	        0x04
#define     SH367309_EEP_OCD1V_70MV	        0x05
#define     SH367309_EEP_OCD1V_80MV	        0x06
#define     SH367309_EEP_OCD1V_90MV	        0x07
#define     SH367309_EEP_OCD1V_100MV	    0x08
#define     SH367309_EEP_OCD1V_110MV	    0x09
#define     SH367309_EEP_OCD1V_120MV	    0x0A
#define     SH367309_EEP_OCD1V_130MV	    0x0B
#define     SH367309_EEP_OCD1V_140MV	    0x0C
#define     SH367309_EEP_OCD1V_160MV	    0x0D
#define     SH367309_EEP_OCD1V_180MV	    0x0E
#define     SH367309_EEP_OCD1V_200MV	    0x0F

//OCD1 OCD1T
//放电过流1保护延时设置控制位
#define     SH367309_EEP_OCD1T_DLY_50MS	    0x00
#define     SH367309_EEP_OCD1T_DLY_100MS	0x01
#define     SH367309_EEP_OCD1T_DLY_200MS    0x02
#define     SH367309_EEP_OCD1T_DLY_400MS	0x03
#define     SH367309_EEP_OCD1T_DLY_600MS	0x04
#define     SH367309_EEP_OCD1T_DLY_800MS	0x05
#define     SH367309_EEP_OCD1T_DLY_1S	    0x06
#define     SH367309_EEP_OCD1T_DLY_2S	    0x07
#define     SH367309_EEP_OCD1T_DLY_4S	    0x08
#define     SH367309_EEP_OCD1T_DLY_6S	    0x09
#define     SH367309_EEP_OCD1T_DLY_8S	    0x0A
#define     SH367309_EEP_OCD1T_DLY_10S	    0x0B
#define     SH367309_EEP_OCD1T_DLY_15S	    0x0C
#define     SH367309_EEP_OCD1T_DLY_20S	    0x0D
#define     SH367309_EEP_OCD1T_DLY_30S	    0x0E
#define     SH367309_EEP_OCD1T_DLY_40S	    0x0F

//OCD2 OCD2V
//放电过流2保护 电压 / 分流器阻值
#define     SH367309_EEP_OCD2V_30MV	        0x00
#define     SH367309_EEP_OCD2V_40MV	        0x01
#define     SH367309_EEP_OCD2V_50MV	        0x02
#define     SH367309_EEP_OCD2V_60MV	        0x03
#define     SH367309_EEP_OCD2V_70MV	        0x04
#define     SH367309_EEP_OCD2V_80MV	        0x05
#define     SH367309_EEP_OCD2V_90MV	        0x06
#define     SH367309_EEP_OCD2V_100MV	    0x07
#define     SH367309_EEP_OCD2V_120MV	    0x08
#define     SH367309_EEP_OCD2V_140MV	    0x09
#define     SH367309_EEP_OCD2V_160MV	    0x0A
#define     SH367309_EEP_OCD2V_180MV	    0x0B
#define     SH367309_EEP_OCD2V_200MV	    0x0C
#define     SH367309_EEP_OCD2V_300MV	    0x0D
#define     SH367309_EEP_OCD2V_400MV	    0x0E
#define     SH367309_EEP_OCD2V_500MV	    0x0F

//OCD2 OCD2T
//放电过流2保护延时设置控制位
#define     SH367309_EEP_OCD2T_DLY_10MS	    0x00
#define     SH367309_EEP_OCD2T_DLY_20MS	    0x01
#define     SH367309_EEP_OCD2T_DLY_40MS	    0x02
#define     SH367309_EEP_OCD2T_DLY_60MS	    0x03
#define     SH367309_EEP_OCD2T_DLY_80MS	    0x04
#define     SH367309_EEP_OCD2T_DLY_100MS	0x05
#define     SH367309_EEP_OCD2T_DLY_200MS	0x06
#define     SH367309_EEP_OCD2T_DLY_400MS	0x07
#define     SH367309_EEP_OCD2T_DLY_600MS    0x08
#define     SH367309_EEP_OCD2T_DLY_800MS	0x09
#define     SH367309_EEP_OCD2T_DLY_1S	    0x0A
#define     SH367309_EEP_OCD2T_DLY_2S	    0x0B
#define     SH367309_EEP_OCD2T_DLY_4S	    0x0C
#define     SH367309_EEP_OCD2T_DLY_8S	    0x0D
#define     SH367309_EEP_OCD2T_DLY_10S	    0x0E
#define     SH367309_EEP_OCD2T_DLY_20S	    0x0F

//SC SCV
//短路保护 电压 / 分流器阻值
#define     SH367309_EEP_SCV_50MV	        0x00
#define     SH367309_EEP_SCV_80MV	        0x01
#define     SH367309_EEP_SCV_110MV	        0x02
#define     SH367309_EEP_SCV_140MV	        0x03
#define     SH367309_EEP_SCV_170MV	        0x04
#define     SH367309_EEP_SCV_200MV	        0x05
#define     SH367309_EEP_SCV_230MV	        0x06
#define     SH367309_EEP_SCV_260MV	        0x07
#define     SH367309_EEP_SCV_290MV	        0x08
#define     SH367309_EEP_SCV_320MV	        0x09
#define     SH367309_EEP_SCV_350MV	        0x0A
#define     SH367309_EEP_SCV_400MV	        0x0B
#define     SH367309_EEP_SCV_500MV	        0x0C
#define     SH367309_EEP_SCV_600MV	        0x0D
#define     SH367309_EEP_SCV_800MV	        0x0E
#define     SH367309_EEP_SCV_1000MV	        0x0F

//SC SCT
//短路保护延时设置控制位
#define     SH367309_EEP_SCT_DLY_0US	    0x00
#define     SH367309_EEP_SCT_DLY_64US	    0x01
#define     SH367309_EEP_SCT_DLY_128US	    0x02
#define     SH367309_EEP_SCT_DLY_192US	    0x03
#define     SH367309_EEP_SCT_DLY_256US	    0x04
#define     SH367309_EEP_SCT_DLY_320US	    0x05
#define     SH367309_EEP_SCT_DLY_384US	    0x06
#define     SH367309_EEP_SCT_DLY_448US	    0x07
#define     SH367309_EEP_SCT_DLY_512US	    0x08
#define     SH367309_EEP_SCT_DLY_576US	    0x09
#define     SH367309_EEP_SCT_DLY_640US	    0x0A
#define     SH367309_EEP_SCT_DLY_704US	    0x0B
#define     SH367309_EEP_SCT_DLY_768US	    0x0C
#define     SH367309_EEP_SCT_DLY_832US	    0x0D
#define     SH367309_EEP_SCT_DLY_896US	    0x0E
#define     SH367309_EEP_SCT_DLY_960US	    0x0F

//OCC OCCV
//充电过流保护 电压 / 分流器阻值
#define     SH367309_EEP_OCCV_20MV	        0x00
#define     SH367309_EEP_OCCV_30MV	        0x01
#define     SH367309_EEP_OCCV_40MV	        0x02
#define     SH367309_EEP_OCCV_50MV	        0x03
#define     SH367309_EEP_OCCV_60MV	        0x04
#define     SH367309_EEP_OCCV_70MV	        0x05
#define     SH367309_EEP_OCCV_80MV	        0x06
#define     SH367309_EEP_OCCV_90MV	        0x07
#define     SH367309_EEP_OCCV_100MV	        0x08
#define     SH367309_EEP_OCCV_110MV	        0x09
#define     SH367309_EEP_OCCV_120MV	        0x0A
#define     SH367309_EEP_OCCV_130MV	        0x0B
#define     SH367309_EEP_OCCV_140MV	        0x0C
#define     SH367309_EEP_OCCV_160MV	        0x0D
#define     SH367309_EEP_OCCV_180MV	        0x0E
#define     SH367309_EEP_OCCV_200MV	        0x0F

//OCC OCCT
//充电过流保护延时设置控制位
#define     SH367309_EEP_OCCT_DLY_10MS	    0x00
#define     SH367309_EEP_OCCT_DLY_20MS	    0x01
#define     SH367309_EEP_OCCT_DLY_40MS	    0x02
#define     SH367309_EEP_OCCT_DLY_60MS	    0x03
#define     SH367309_EEP_OCCT_DLY_80MS	    0x04
#define     SH367309_EEP_OCCT_DLY_100MS	    0x05
#define     SH367309_EEP_OCCT_DLY_200MS	    0x06
#define     SH367309_EEP_OCCT_DLY_400MS	    0x07
#define     SH367309_EEP_OCCT_DLY_600MS	    0x08
#define     SH367309_EEP_OCCT_DLY_800MS	    0x09
#define     SH367309_EEP_OCCT_DLY_1S	    0x0A
#define     SH367309_EEP_OCCT_DLY_2S	    0x0B
#define     SH367309_EEP_OCCT_DLY_4S	    0x0C
#define     SH367309_EEP_OCCT_DLY_8S	    0x0D
#define     SH367309_EEP_OCCT_DLY_10S	    0x0E
#define     SH367309_EEP_OCCT_DLY_20S	    0x0F

//CHS
//充放电状态检测电压 电压 / 分流器阻值
#define     SH367309_EEP_CHS_200UV	        0x00
#define     SH367309_EEP_CHS_500UV	        0x01
#define     SH367309_EEP_CHS_1000UV	        0x02
#define     SH367309_EEP_CHS_2000UV	        0x03

//MOST
//充放电MOSFET开启延时
#define     SH367309_EEP_MOST_DLY_64US      0x00
#define     SH367309_EEP_MOST_DLY_128US	    0x01
#define     SH367309_EEP_MOST_DLY_256US	    0x02
#define     SH367309_EEP_MOST_DLY_512US	    0x03

//OCRT
//充放电过流自恢复延时
#define     SH367309_EEP_OCRT_DLY_8S        0x00
#define     SH367309_EEP_OCRT_DLY_16S	    0x01
#define     SH367309_EEP_OCRT_DLY_32S	    0x02
#define     SH367309_EEP_OCRT_DLY_64S	    0x03

//PFT
//二次过充电保护延时
#define     SH367309_EEP_PFT_DLY_8S         0x00
#define     SH367309_EEP_PFT_DLY_16S	    0x01
#define     SH367309_EEP_PFT_DLY_32S	    0x02
#define     SH367309_EEP_PFT_DLY_64S	    0x03

//SH367309寄存器
typedef struct _SH_Register_Group
{
    //40H 系统配置寄存器
    union
	{
		struct
		{
			u8 IDLE          	:1;	//IDLE设置控制位
			u8 SLEEP          	:1; //SLEEP设置控制位
			u8 ENWDT           	:1; //看门狗设置控制位
			u8 CADCON			:1; //CADC设置控制位
			u8 CHGMOS  	 	    :1; //充电MOSFET控制位
			u8 DSGMOS	        :1; //放电MOSFET控制位
			u8 PCHMOS		 	:1; //预充电MOSFET控制位
			u8 OCRC	 		    :1; //过流保护控制位
		}CONFBit;
		u8 CONFByte;
	}CONF;
    
    //41H 均衡控制寄存器
    u8 BalanceH;
    //42H
    u8 BalanceL;
    
    //43H 系统状态寄存器1
    union
	{
		struct
		{
			u8 OV          	:1;	//过压保护状态位，0：未发生，1：发生
			u8 UV          	:1; //欠压保护状态位，0：未发生，1：发生
			u8 OCD1         :1; //欠压保护状态位，0：未发生，1：发生
			u8 OCD2			:1; //放电过流2保护状态位，0：未发生，1：发生
			u8 OCC  	 	:1; //充电过流保护状态位，0：未发生，1：发生
			u8 SC	        :1; //短路保护状态位，0：未发生，1：发生
			u8 PF		 	:1; //二次过充电保护状态位，0：未发生，1：发生
			u8 WDT	 		:1; //看门狗状态位，0：看门狗正常，1：看门狗溢出
		}BSTATUS1Bit;
		u8 BSTATUS1Byte;
	}BSTATUS1;    
    
    //44H 系统状态寄存器2
    union
	{
		struct
		{
			u8 UTC          :1;	//充电低温保护状态位，0：未发生，1：发生
			u8 OTC          :1; //充电高温保护状态位，0：未发生，1：发生
			u8 UTD          :1; //放电低温保护状态位，0：未发生，1：发生
			u8 OTD			:1; //放电高温保护状态位，0：未发生，1：发生
			u8 RVSD  	 	:4; //保留
		}BSTATU2Bit;
		u8 BSTATUS2Byte;
	}BSTATUS2;       
    
    //45H 系统状态寄存器3
    union
	{
		struct
		{
			u8 DSG_FET          :1;	//放电MOSFET开关状态位，1：放电MOSFET开启，0：放电MOSFET关闭
			u8 CHG_FET          :1; //充电MOSFET开关状态位，1：充电MOSFET开启，0：充电MOSFET关闭
			u8 PCHG_FET         :1; //预充电MOSFET开关状态位，1：预充电MOSFET开启，0：预充电MOSFET关闭
			u8 LOV			    :1; //低电压禁止充电状态位，0：未发生，1：发生
			u8 EEPR_WR  	 	:1; //EEPROM写操作状态位，1： EEPROM写操作错误，0： EEPROM写操作正确
			u8 RVSD	            :1; //保留
			u8 DSGING		 	:1; //放电状态位，1：放电状态，0：非放电状态
			u8 CHGING	 		:1; //充电状态位，1：充电状态，0：非充电状态
		}BSTATUS3Bit;
		u8 BSTATUS3Byte;
	}BSTATUS3;     
    
    //46H - 4BH ,温度采样
    u8 Temp1H;  
    u8 Temp1L;      
    u8 Temp2H;   
    u8 Temp2L;     
    u8 Temp3H; 
    u8 Temp3L;     
    
    //4CH - 4DH ,电流采样
    u8 CurrH;  
    u8 CurrL; 
    
    //4EH - 6DH ,单体电压采样
    u8 Cell1H;    
    u8 Cell1L;      
    u8 Cell2H;    
    u8 Cell2L;       
    u8 Cell3H;    
    u8 Cell3L;      
    u8 Cell4H;    
    u8 Cell4L;        
    u8 Cell5H;    
    u8 Cell5L;      
    u8 Cell6H;    
    u8 Cell6L;       
    u8 Cell7H;    
    u8 Cell7L;      
    u8 Cell8H;    
    u8 Cell8L;      
    u8 Cell9H;    
    u8 Cell9L;      
    u8 Cell10H;    
    u8 Cell10L;       
    u8 Cell11H;    
    u8 Cell11L;      
    u8 Cell12H;    
    u8 Cell12L;        
    u8 Cell13H;    
    u8 Cell13L;      
    u8 Cell14H;    
    u8 Cell14L;       
    u8 Cell15H;    
    u8 Cell15L;      
    u8 Cell16H;    
    u8 Cell16L;    
    
    //6EH - 6FH ,CADC电流采样
    u8 CADCCurrH;      
    u8 CADCCurrL;      
    
    //70H 系统标志寄存器1
    union
	{
		struct
		{
			u8 OV_FLG           :1;	//过压保护标志位，0：未发生过，1：发生过
			u8 UV_FLG           :1; //欠压保护标志位，0：未发生过，1：发生过
			u8 OCD_FLG          :1; //放电过流保护标志位，0：未发生过，1：发生过
			u8 LOAD_FLG		    :1; //LDO3过流标志位，0：未发生过，1：发生过
			u8 OCC_FLG  	    :1; //充电过流保护标志位，0：未发生过，1：发生过
			u8 SC_FLG	        :1; //短路保护标志位，0：未发生过，1：发生过
			u8 PF_FLG		    :1; //二次过充电保护标志位，0：未发生过，1：发生过
			u8 WDT_FLG	 	    :1; //看门狗标志位，0：未发生过看门狗溢出，1：发生过看门狗溢出
		}BFLAG1Bit;
		u8 BFLAG1Byte;
	}BFLAG1;    
    
    //71H 系统标志寄存器2
    union
	{
		struct
		{
			u8 UTC_FLG          :1;	//充电低温保护标志位，0：未发生过，1：发生过
			u8 OTC_FLG          :1; //充电高温保护标志位，0：未发生过，1：发生过
			u8 UTD_FLG          :1; //放电低温保护标志位，0：未发生过，1：发生过
			u8 OTD_FLG		    :1; //放电高温保护标志位，0：未发生过，1：发生过
			u8 VADC_FLG  	    :1; //VADC中断标志位，0：未发生过，1：发生过
			u8 CADC_FLG	        :1; //CADC中断标志位，0：未发生过，1：发生过，该bit被读取之后，硬件会自动清零
			u8 WAKE_FLG		    :1; //唤醒中断标志位，1：从IDLE状态（检测到充放电电流）或者SLEEP状态（充电器连接）被唤醒 0：未被唤醒
			u8 RST_FLG	 	    :1; //复位标志位，1:系统复位后，自动置1,需MCU清零，0：未被唤醒
		}BFLAG2Bit;
		u8 BFLAG2Byte;
	}BFLAG2;         
    
    //72H 复位状态
    union
	{
		struct
		{
			u8 WDT0             :1;	//
			u8 WDT1             :1; //
			u8 RVSD             :6; //
        }RSTSTATBit;
		u8 RSTSTATByte;
	}RSTSTAT;         
    
}t_SH367309_Register;

//CONF PCHMOS
//预充电MOSFET关闭
#define     SH367309_REG_PCHMOS_OFF	    0x00
//预充电MOSFET由硬件保护模块决定
#define     SH367309_REG_PCHMOS_ON	    0x01

//CONF DSGMOS
//放电MOSFET关闭
#define     SH367309_REG_DSGMOS_OFF	    0x00
//放电MOSFET由硬件保护模块决定
#define     SH367309_REG_DSGMOS_ON	    0x01

//CONF CHGMOS
//充电MOSFET关闭
#define     SH367309_REG_CHGMOS_OFF	    0x00
//充电MOSFET由硬件保护模块决定
#define     SH367309_REG_CHGMOS_ON	    0x01

//CONF CADCON
//SH367309关闭CADC
#define     SH367309_REG_CADCON_OFF	    0x00
//SH367309开启CADC进行电流采集
#define     SH367309_REG_CADCON_ON	    0x01

//CONF ENWDT
//SH367309关闭看门狗模块
#define     SH367309_REG_ENWDT_OFF	    0x00
//SH367309开启看门狗模块
#define     SH367309_REG_ENWDT_ON	    0x01

//CONF SLEEP
//SH367309不进入SLEEP状态
#define     SH367309_REG_SLEEP_DISABLE	0x00
//SH367309将进入SLEEP状态，唤醒后硬件自动清零
//注释：当设置为“1”时，如果SH367309连接充电器，则不进入SLEEP状态，硬件会自动清零
#define     SH367309_REG_SLEEP_ENABLE	0x01

//CONF IDLE
//SH367309不进入IDLE状态
#define     SH367309_REG_IDLE_DISABLE	0x00
// SH367309将进入IDLE状态，唤醒后硬件自动清零
//注释：当设置为"时，如果SH367309发生了任何保护，则不进入IDLE状态，硬件会自动清零
#define     SH367309_REG_IDLE_ENABLE	0x01

#endif

/*****************************************end of SH367309.h*****************************************/
