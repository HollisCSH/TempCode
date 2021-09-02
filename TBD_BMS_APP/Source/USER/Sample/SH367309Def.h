//=======================================Copyright(c)===========================================
// 								  ���������Ƽ����޹�˾
//---------------------------------------�ļ���Ϣ----------------------------------------------
//�ļ���   	: SH367309Def.h
//������  	: Handry
//��������	: 
//����	    : SH367309��������ͷ�ļ�
//-----------------------------------------------��ǰ�汾�޶�----------------------------------
//�޸���   	:
//�汾	    :
//�޸�����	:
//����	    :
//=============================================================================================
#ifndef SH367309_DEF_H_
#define SH367309_DEF_H_

//=============================================================================================
//ͷ�ļ�
//=============================================================================================
#include "BSPTypeDef.h"

//=============================================================================================
//������趨
//=============================================================================================

//----------------------------AFE EEP�ڲ����---------------------------------------------------
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

//----------------------------�Ĵ����������---------------------------------------------------
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
//���ݽṹ����
//=============================================================================
typedef struct _EEPROM_Group
{
    //00H
    union
	{
		struct
		{
			u8 CN          	    :4;	//�������ÿ���λ
			u8 BAL	 	        :1; //ƽ�⹦��ģ��ʹ�ܿ���λ
			u8 OCPM        	    :1; //��ŵ����MOSFET����λ
			u8 ENMOS		 	:1; //���MOSFET�ָ�����λ
			u8 ENPCH	 		:1; //Ԥ���ģ�����λ
		}SCONF1Bit;
		u8 SCONF1Byte;
	}SCONF1;
    
    //01H
    union
	{
		struct
		{
			u8 EUVR          	:1;	//���ŵ�ָ����ÿ���λ
			u8 OCRA          	:1; //�����ָ����ÿ���λ
			u8 CTLC           	:2; //CTL�ܽŹ������ÿ���λ
			u8 DIS_PF	 	    :1; //���ι����ģ��ʹ�ܿ���λ
			u8 UV_OP        	:1; //���ŵ�ʱMOSFET����λ
			u8 RSVD		 	    :1; //����
			u8 EOVB	 		    :1; //��ֹ��ѹ��о��繦�����ÿ���λ
		}SCONF2Bit;
		u8 SCONF2Byte;
	}SCONF2;    
    
    //02H
    union
	{
		struct
		{
			u8 OV8_9          	:2;	
			u8 LDRT          	:2; //�����ͷ���ʱ���ÿ���λ
			u8 OVT           	:4; //����籣����ʱ���ÿ���λ
		}OVHBit;
		u8 OVHByte;
	}OVH;       
    
    //03H
    union
	{
		struct
		{
			u8 OV0_7          	:8;	//����籣����ѹ�����㷽ʽ���Ĵ���ֵ X 5mV
		}OVLBit;
		u8 OVLByte;
	}OVL;           

    //04H
    union
	{
		struct
		{
			u8 OVR8_9          	:2;	
			u8 RVSD          	:2; //����
			u8 UVT           	:4; //���ŵ籣����ʱ���ÿ���λ
		}OVRHBit;
		u8 OVRHByte;
	}OVRH;     
    
    //05H
    union
	{
		struct
		{
            u8 OV0_7          	:8;	//�����ָ���ѹ�����㷽ʽ���Ĵ���ֵ X 5mV
		}OVRLBit;
		u8 OVRLByte;
	}OVRL;        
    
    //06H
    union
	{
		struct
		{
            u8 UV0_7          	:8;	//���ŵ籣����ѹ�����㷽ʽ���Ĵ���ֵ X 20mV
		}UVBit;
		u8 UVByte;
	}UV;         

    //07H
    union
	{
		struct
		{
            u8 UVR0_7          	:8;	//���ŵ籣����ѹ�����㷽ʽ���Ĵ���ֵ X 20mV
		}UVRBit;
		u8 UVRByte;
	}UVR;     
    
    //08H
    u8 BALVByte; //ƽ�⿪����ѹ���üĴ��� ���㷽ʽ���Ĵ���ֵ X 20mV
    
    //09H
    u8 PREVByte; //Ԥ����ѹ���üĴ��� ���㷽ʽ���Ĵ���ֵ X 20mV
    
    //0AH
    u8 L0VByte; //�͵�ѹ��ֹ����ѹ���üĴ��� ���㷽ʽ���Ĵ���ֵ X 20mV    
    
    //0BH
    u8 PFVByte; //���ι���籣����ѹ���üĴ��� ���㷽ʽ���Ĵ���ֵ X 20mV        
    
    //0CH
    union
	{
		struct
		{
            u8 OCD1T          	:4;	//�ŵ����1������ʱ���ÿ���λ
            u8 OCD1V          	:4;	//�ŵ����1������ѹ���ÿ���λ
		}OCD1Bit;
		u8 OCD1Byte;
	}OCD1;         
    
    //0DH
    union
	{
		struct
		{
            u8 OCD2T          	:4;	//�ŵ����2������ʱ���ÿ���λ
            u8 OCD2V          	:4;	//�ŵ����2������ѹ���ÿ���λ
		}OCD2Bit;
		u8 OCD2Byte;
	}OCD2;           
    
    //0EH
    union
	{
		struct
		{
            u8 SCT          	:4;	//��·������ʱ���ÿ���λ
            u8 SCV          	:4;	//��·������ѹ���ÿ���λ
		}SCBit;
		u8 SCByte;
	}SC;
    
    //0FH
    union
	{
		struct
		{
            u8 OCCT          	:4;	//������������ʱ���ÿ���λ
            u8 OCCV          	:4;	//������������ѹ���ÿ���λ
		}OCCBit;
		u8 OCCByte;
	}OCC;    
    
    //10H
    union
	{
		struct
		{
            u8 PFT          	:2;	//���ι���㱣����ʱ����
            u8 OCRT          	:2;	//��ŵ�����Իָ���ʱ����
            u8 MOST          	:2;	//��ŵ�MOSFET������ʱ����       
            u8 CHS          	:2;	//��ŵ�״̬����ѹ����
		}MOST_OCRT_PFTBit;
		u8 MOST_OCRT_PFTByte;
	}MOST_OCRT_PFT;    
    
    //11H
    u8 OTCByte;  //�����±�����ֵ
    
    //12H
    u8 OTCRByte; //�����±����ͷ���ֵ
    
    //13H
    u8 UTCByte;  //�ŵ���±�����ֵ 
    
    //14H
    u8 UTCRByte; //�����±����ͷ���ֵ 
    
    //15H
    u8 OTDByte;  //�ŵ���±�����ֵ
    
    //16H
    u8 OTDRByte; //�ŵ���±����ͷ���ֵ
    
    //17H
    u8 UTDByte;  //�ŵ���±�����ֵ 
    
    //18H
    u8 UTDRByte; //�ŵ���±����ͷ���ֵ     
    
    //19H
    union
	{
		struct
		{
            u8 TR0_6          	:7;	//�¶��ڲ��ο�����ϵ��
            u8 RVSD          	:1;	//����
		}TRBit;
		u8 TRByte;
	}TR;    
    
}
t_SH367309_EEPRom;

//----------------------------AFE EEP�ڲ�����---------------------------------------------------
//SCONF1 ENPCH
//����Ԥ��繦��
#define     SH367309_EEP_ENPCH_ENABLE	    0x01	
//����Ԥ��繦��
#define     SH367309_EEP_ENPCH_DISABLE	    0x00    

//SCONF1 ENMOS
//���ó��MOSFET�ָ�����λ���������/�¶ȱ����رճ��MOSFET���� ����⵽�ŵ����1���߷ŵ�״̬���������MOSFET
#define     SH367309_EEP_ENMOS_ENABLE	    0x01	
//���ó��MOSFET�ָ�����λ
#define     SH367309_EEP_ENMOS_DISABLE	    0x00    

//SCONF1 OCPM
//��ŵ�����رճ�ŵ�MOSFET
#define     SH367309_EEP_OCPM_ENABLE	    0x01	
//������ֻ�رճ��MOSFET���ŵ����ֻ�رշŵ�MOSFET
#define     SH367309_EEP_OCPM_DISABLE	    0x00

//SCONF1 BAL
//ƽ�⿪����SH367309�ڲ��߼�����
#define     SH367309_EEP_BAL_INTERNAL	    0x00	
//ƽ�⿪�����ⲿMCU���ƣ�ƽ��ʱ������SH367309�ڲ��߼�����
#define     SH367309_EEP_BAL_MCU	        0x01

//SCONF1 CN
//ע��:SH367309����5-16��﮵�ر���,����������С��16����,��ʹ�õĵ� о����ˣ���VBT���̽�����ߴ���о�����˼���
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
//��������ֹ��ѹ��о��硱����
#define     SH367309_EEP_E0VB_ENABLE	    0x01	
//�رա���ֹ��ѹ��о��硱����
#define     SH367309_EEP_E0VB_DISABLE	    0x00

//SCONF2 UV_OP
//���ŵ�رճ�ŵ�MOSFET
#define     SH367309_EEP_UV_OP_ENABLE	    0x01	
//���ŵ�ֻ�رշŵ�MOSFET
#define     SH367309_EEP_UV_OP_DISABLE	    0x00

//SCONF2 DIS_PF
//���ö��ι���籣��
#define     SH367309_EEP_DIS_PF_ENABLE	    0x00	
//��ֹ���ι���籣��
#define     SH367309_EEP_DIS_PF_DISABLE	    0x01

//SCONF2 CTLC
//��ŵ��Ԥ���MOSFET���ڲ��߼����ƣ�CTL�ܽ�������Ч
#define     SH367309_EEP_CTLC_INVALID	    0x00	
//���Ƴ���Ԥ���MOSFET. CTL����L��ƽʱǿ�ƹرճ���Ԥ���MOSFET. CTL����H��ʱ����Ԥ��� MOSFET���ڲ��߼�����
#define     SH367309_EEP_CTLC_CHG_PRCH	    0x01	
//���Ʒŵ�MOSFET. CTL����L��ƽʱǿ�ƹرշŵ� MOSFET�� CTL����H��ƽʱ���ŵ�MOSFET���ڲ��߼�����
#define     SH367309_EEP_CTLC_DCHG	        0x02
//���Ƴ�ŵ��Ԥ���MOSFET. CTL����L��ƽʱǿ�ƹرճ�ŵ��Ԥ���MOSFET��CTL����H��ƽʱ����ŵ��Ԥ���MOSFET���ڲ��߼�����
#define     SH367309_EEP_CTLC_ALL	        0x03

//SCONF2 OCRA
//�������������ʱ�ָ�
#define     SH367309_EEP_OCRA_ENABLE	    0x01
//���������������ʱ�ָ� 
#define     SH367309_EEP_OCRA_DISABLE	    0x00

//SCONF2 EUVR
//���ŵ籣��״̬�ͷŻ��踺���ͷ�
#define     SH367309_EEP_EUVR_ENABLE	    0x01
//���ŵ籣��״̬�ͷ��븺���ͷ��޹� 
#define     SH367309_EEP_EUVR_DISABLE	    0x00

//OVH OVT
//����籣����ʱ���ÿ���λ
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
//�����ͷ���ʱ���ÿ���λ
#define     SH367309_EEP_LDRT_DLY_100MS	    0x00
#define     SH367309_EEP_LDRT_DLY_500MS	    0x01
#define     SH367309_EEP_LDRT_DLY_1000MS	0x02
#define     SH367309_EEP_LDRT_DLY_2000MS	0x03

//����籣����ѹ�����㷽ʽ���Ĵ���ֵ X 5mV
#define     SH367309_EEP_OV_GAIN	        5

//OVRH UVT
//���ŵ籣����ʱ���ÿ���λ
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

//�����ָ���ѹ�����㷽ʽ���Ĵ���ֵ X 5mV
#define     SH367309_EEP_OVR_GAIN	        5

//���ŵ籣����ѹ�����㷽ʽ���Ĵ���ֵ X 20mV
#define     SH367309_EEP_UV_GAIN	        20

//���ŵ�ָ���ѹ�����㷽ʽ���Ĵ���ֵ X 20mV
#define     SH367309_EEP_UVR_GAIN	        20

//ƽ�⿪����ѹ�����㷽ʽ���Ĵ���ֵ X 20mV
#define     SH367309_EEP_BALV_GAIN	        20

//Ԥ����ѹ�����㷽ʽ���Ĵ���ֵ X 20mV
#define     SH367309_EEP_PREV_GAIN	        20

//��ѹ��ֹ����ѹ�����㷽ʽ���Ĵ���ֵ X 20mV
#define     SH367309_EEP_L0V_GAIN	        20

//���ι�ѹ��籣����ѹ�����㷽ʽ���Ĵ���ֵ X 20mV
#define     SH367309_EEP_PFV_GAIN	        20

//OCD1 OCD1V
//�ŵ����1���� ��ѹ / ��������ֵ
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
//�ŵ����1������ʱ���ÿ���λ
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
//�ŵ����2���� ��ѹ / ��������ֵ
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
//�ŵ����2������ʱ���ÿ���λ
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
//��·���� ��ѹ / ��������ֵ
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
//��·������ʱ���ÿ���λ
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
//���������� ��ѹ / ��������ֵ
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
//������������ʱ���ÿ���λ
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
//��ŵ�״̬����ѹ ��ѹ / ��������ֵ
#define     SH367309_EEP_CHS_200UV	        0x00
#define     SH367309_EEP_CHS_500UV	        0x01
#define     SH367309_EEP_CHS_1000UV	        0x02
#define     SH367309_EEP_CHS_2000UV	        0x03

//MOST
//��ŵ�MOSFET������ʱ
#define     SH367309_EEP_MOST_DLY_64US      0x00
#define     SH367309_EEP_MOST_DLY_128US	    0x01
#define     SH367309_EEP_MOST_DLY_256US	    0x02
#define     SH367309_EEP_MOST_DLY_512US	    0x03

//OCRT
//��ŵ�����Իָ���ʱ
#define     SH367309_EEP_OCRT_DLY_8S        0x00
#define     SH367309_EEP_OCRT_DLY_16S	    0x01
#define     SH367309_EEP_OCRT_DLY_32S	    0x02
#define     SH367309_EEP_OCRT_DLY_64S	    0x03

//PFT
//���ι���籣����ʱ
#define     SH367309_EEP_PFT_DLY_8S         0x00
#define     SH367309_EEP_PFT_DLY_16S	    0x01
#define     SH367309_EEP_PFT_DLY_32S	    0x02
#define     SH367309_EEP_PFT_DLY_64S	    0x03

//SH367309�Ĵ���
typedef struct _SH_Register_Group
{
    //40H ϵͳ���üĴ���
    union
	{
		struct
		{
			u8 IDLE          	:1;	//IDLE���ÿ���λ
			u8 SLEEP          	:1; //SLEEP���ÿ���λ
			u8 ENWDT           	:1; //���Ź����ÿ���λ
			u8 CADCON			:1; //CADC���ÿ���λ
			u8 CHGMOS  	 	    :1; //���MOSFET����λ
			u8 DSGMOS	        :1; //�ŵ�MOSFET����λ
			u8 PCHMOS		 	:1; //Ԥ���MOSFET����λ
			u8 OCRC	 		    :1; //������������λ
		}CONFBit;
		u8 CONFByte;
	}CONF;
    
    //41H ������ƼĴ���
    u8 BalanceH;
    //42H
    u8 BalanceL;
    
    //43H ϵͳ״̬�Ĵ���1
    union
	{
		struct
		{
			u8 OV          	:1;	//��ѹ����״̬λ��0��δ������1������
			u8 UV          	:1; //Ƿѹ����״̬λ��0��δ������1������
			u8 OCD1         :1; //Ƿѹ����״̬λ��0��δ������1������
			u8 OCD2			:1; //�ŵ����2����״̬λ��0��δ������1������
			u8 OCC  	 	:1; //����������״̬λ��0��δ������1������
			u8 SC	        :1; //��·����״̬λ��0��δ������1������
			u8 PF		 	:1; //���ι���籣��״̬λ��0��δ������1������
			u8 WDT	 		:1; //���Ź�״̬λ��0�����Ź�������1�����Ź����
		}BSTATUS1Bit;
		u8 BSTATUS1Byte;
	}BSTATUS1;    
    
    //44H ϵͳ״̬�Ĵ���2
    union
	{
		struct
		{
			u8 UTC          :1;	//�����±���״̬λ��0��δ������1������
			u8 OTC          :1; //�����±���״̬λ��0��δ������1������
			u8 UTD          :1; //�ŵ���±���״̬λ��0��δ������1������
			u8 OTD			:1; //�ŵ���±���״̬λ��0��δ������1������
			u8 RVSD  	 	:4; //����
		}BSTATU2Bit;
		u8 BSTATUS2Byte;
	}BSTATUS2;       
    
    //45H ϵͳ״̬�Ĵ���3
    union
	{
		struct
		{
			u8 DSG_FET          :1;	//�ŵ�MOSFET����״̬λ��1���ŵ�MOSFET������0���ŵ�MOSFET�ر�
			u8 CHG_FET          :1; //���MOSFET����״̬λ��1�����MOSFET������0�����MOSFET�ر�
			u8 PCHG_FET         :1; //Ԥ���MOSFET����״̬λ��1��Ԥ���MOSFET������0��Ԥ���MOSFET�ر�
			u8 LOV			    :1; //�͵�ѹ��ֹ���״̬λ��0��δ������1������
			u8 EEPR_WR  	 	:1; //EEPROMд����״̬λ��1�� EEPROMд��������0�� EEPROMд������ȷ
			u8 RVSD	            :1; //����
			u8 DSGING		 	:1; //�ŵ�״̬λ��1���ŵ�״̬��0���Ƿŵ�״̬
			u8 CHGING	 		:1; //���״̬λ��1�����״̬��0���ǳ��״̬
		}BSTATUS3Bit;
		u8 BSTATUS3Byte;
	}BSTATUS3;     
    
    //46H - 4BH ,�¶Ȳ���
    u8 Temp1H;  
    u8 Temp1L;      
    u8 Temp2H;   
    u8 Temp2L;     
    u8 Temp3H; 
    u8 Temp3L;     
    
    //4CH - 4DH ,��������
    u8 CurrH;  
    u8 CurrL; 
    
    //4EH - 6DH ,�����ѹ����
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
    
    //6EH - 6FH ,CADC��������
    u8 CADCCurrH;      
    u8 CADCCurrL;      
    
    //70H ϵͳ��־�Ĵ���1
    union
	{
		struct
		{
			u8 OV_FLG           :1;	//��ѹ������־λ��0��δ��������1��������
			u8 UV_FLG           :1; //Ƿѹ������־λ��0��δ��������1��������
			u8 OCD_FLG          :1; //�ŵ����������־λ��0��δ��������1��������
			u8 LOAD_FLG		    :1; //LDO3������־λ��0��δ��������1��������
			u8 OCC_FLG  	    :1; //������������־λ��0��δ��������1��������
			u8 SC_FLG	        :1; //��·������־λ��0��δ��������1��������
			u8 PF_FLG		    :1; //���ι���籣����־λ��0��δ��������1��������
			u8 WDT_FLG	 	    :1; //���Ź���־λ��0��δ���������Ź������1�����������Ź����
		}BFLAG1Bit;
		u8 BFLAG1Byte;
	}BFLAG1;    
    
    //71H ϵͳ��־�Ĵ���2
    union
	{
		struct
		{
			u8 UTC_FLG          :1;	//�����±�����־λ��0��δ��������1��������
			u8 OTC_FLG          :1; //�����±�����־λ��0��δ��������1��������
			u8 UTD_FLG          :1; //�ŵ���±�����־λ��0��δ��������1��������
			u8 OTD_FLG		    :1; //�ŵ���±�����־λ��0��δ��������1��������
			u8 VADC_FLG  	    :1; //VADC�жϱ�־λ��0��δ��������1��������
			u8 CADC_FLG	        :1; //CADC�жϱ�־λ��0��δ��������1������������bit����ȡ֮��Ӳ�����Զ�����
			u8 WAKE_FLG		    :1; //�����жϱ�־λ��1����IDLE״̬����⵽��ŵ����������SLEEP״̬����������ӣ������� 0��δ������
			u8 RST_FLG	 	    :1; //��λ��־λ��1:ϵͳ��λ���Զ���1,��MCU���㣬0��δ������
		}BFLAG2Bit;
		u8 BFLAG2Byte;
	}BFLAG2;         
    
    //72H ��λ״̬
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
//Ԥ���MOSFET�ر�
#define     SH367309_REG_PCHMOS_OFF	    0x00
//Ԥ���MOSFET��Ӳ������ģ�����
#define     SH367309_REG_PCHMOS_ON	    0x01

//CONF DSGMOS
//�ŵ�MOSFET�ر�
#define     SH367309_REG_DSGMOS_OFF	    0x00
//�ŵ�MOSFET��Ӳ������ģ�����
#define     SH367309_REG_DSGMOS_ON	    0x01

//CONF CHGMOS
//���MOSFET�ر�
#define     SH367309_REG_CHGMOS_OFF	    0x00
//���MOSFET��Ӳ������ģ�����
#define     SH367309_REG_CHGMOS_ON	    0x01

//CONF CADCON
//SH367309�ر�CADC
#define     SH367309_REG_CADCON_OFF	    0x00
//SH367309����CADC���е����ɼ�
#define     SH367309_REG_CADCON_ON	    0x01

//CONF ENWDT
//SH367309�رտ��Ź�ģ��
#define     SH367309_REG_ENWDT_OFF	    0x00
//SH367309�������Ź�ģ��
#define     SH367309_REG_ENWDT_ON	    0x01

//CONF SLEEP
//SH367309������SLEEP״̬
#define     SH367309_REG_SLEEP_DISABLE	0x00
//SH367309������SLEEP״̬�����Ѻ�Ӳ���Զ�����
//ע�ͣ�������Ϊ��1��ʱ�����SH367309���ӳ�������򲻽���SLEEP״̬��Ӳ�����Զ�����
#define     SH367309_REG_SLEEP_ENABLE	0x01

//CONF IDLE
//SH367309������IDLE״̬
#define     SH367309_REG_IDLE_DISABLE	0x00
// SH367309������IDLE״̬�����Ѻ�Ӳ���Զ�����
//ע�ͣ�������Ϊ"ʱ�����SH367309�������κα������򲻽���IDLE״̬��Ӳ�����Զ�����
#define     SH367309_REG_IDLE_ENABLE	0x01

#endif

/*****************************************end of SH367309.h*****************************************/
