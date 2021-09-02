//=======================================Copyright(c)===========================================
// 								  ���������Ƽ����޹�˾
//---------------------------------------�ļ���Ϣ----------------------------------------------
//�ļ���   	: SH367309.c
//������  	: Handry
//��������	: 
//����	    : SH367309����Դ�ļ�
//-----------------------------------------------��ǰ�汾�޶�----------------------------------
//�޸���   	:
//�汾	    :
//�޸�����	:
//����	    :
//=============================================================================================

//=============================================================================================
//����ͷ�ļ�
//=============================================================================================
#include "Single_SH367309.h"
#include "BSPTimer.h"
#include "MOSCtrl.h"
#include "Storage.h"
#include "Sample.h"
#include "CommCtrl.h"
//#include "BSPSysTick.h"
#include "ParaCfg.h"
#include "NTC.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "DataDeal.h"
#include "pt.h"
#include "DTCheck.h"

#ifdef DEBUG
#include "BSP_UART.h"
#endif

//=============================================================================================
//ȫ�ֱ�������
//=============================================================================================
u8	gSH367309Readbuff[SH367309_READBUFF_LEN] = {0};     //SH367309���ջ�������
u8	gSH367309Writebuff[SH367309_WRITEBUFF_LEN] = {0};  	//SH367309���ͻ�������
u8  gSHAFEWritebuff[10] = {0};

t_SH_AFE_Data_Status gSHAFEData;			            //SH367309��������/״̬��ؽṹ��
t_SH_AFE_Ctrl gSHAFECtrl;                               //SH367309������ؽṹ��
t_SH_AFE_CfgData gSHAFECfg;                             //SH367309����������ؽṹ��
t_SH367309_EEPRom gSHAfeEep;                            //SH367309����EEP�ṹ��
t_SH367309_EEPRom gSHAfeEepRead;                        //SH367309����EEP�ṹ��
t_SH367309_Register gSHAFEReg;                          //SH367309�Ĵ����ṹ��

pt gPTSH367309;					                        //SH367309����pt�߳̿��Ʊ���
pt gPTSH367309Norm;					                    //SH367309������������pt�߳̿��Ʊ���

t_AFE_Test g_AFE_Test_VAR = {0,250};

//120424-1			CRC Table
const u8 CRC8Table[] =
{							
    0x00,0x07,0x0E,0x09,0x1C,0x1B,0x12,0x15,0x38,0x3F,0x36,0x31,0x24,0x23,0x2A,0x2D,
    0x70,0x77,0x7E,0x79,0x6C,0x6B,0x62,0x65,0x48,0x4F,0x46,0x41,0x54,0x53,0x5A,0x5D,
    0xE0,0xE7,0xEE,0xE9,0xFC,0xFB,0xF2,0xF5,0xD8,0xDF,0xD6,0xD1,0xC4,0xC3,0xCA,0xCD,
    0x90,0x97,0x9E,0x99,0x8C,0x8B,0x82,0x85,0xA8,0xAF,0xA6,0xA1,0xB4,0xB3,0xBA,0xBD,
    0xC7,0xC0,0xC9,0xCE,0xDB,0xDC,0xD5,0xD2,0xFF,0xF8,0xF1,0xF6,0xE3,0xE4,0xED,0xEA,
    0xB7,0xB0,0xB9,0xBE,0xAB,0xAC,0xA5,0xA2,0x8F,0x88,0x81,0x86,0x93,0x94,0x9D,0x9A,
    0x27,0x20,0x29,0x2E,0x3B,0x3C,0x35,0x32,0x1F,0x18,0x11,0x16,0x03,0x04,0x0D,0x0A,
    0x57,0x50,0x59,0x5E,0x4B,0x4C,0x45,0x42,0x6F,0x68,0x61,0x66,0x73,0x74,0x7D,0x7A,
    0x89,0x8E,0x87,0x80,0x95,0x92,0x9B,0x9C,0xB1,0xB6,0xBF,0xB8,0xAD,0xAA,0xA3,0xA4,
    0xF9,0xFE,0xF7,0xF0,0xE5,0xE2,0xEB,0xEC,0xC1,0xC6,0xCF,0xC8,0xDD,0xDA,0xD3,0xD4,
    0x69,0x6E,0x67,0x60,0x75,0x72,0x7B,0x7C,0x51,0x56,0x5F,0x58,0x4D,0x4A,0x43,0x44,
    0x19,0x1E,0x17,0x10,0x05,0x02,0x0B,0x0C,0x21,0x26,0x2F,0x28,0x3D,0x3A,0x33,0x34,
    0x4E,0x49,0x40,0x47,0x52,0x55,0x5C,0x5B,0x76,0x71,0x78,0x7F,0x6A,0x6D,0x64,0x63,
    0x3E,0x39,0x30,0x37,0x22,0x25,0x2C,0x2B,0x06,0x01,0x08,0x0F,0x1A,0x1D,0x14,0x13,
    0xAE,0xA9,0xA0,0xA7,0xB2,0xB5,0xBC,0xBB,0x96,0x91,0x98,0x9F,0x8A,0x8D,0x84,0x83,
    0xDE,0xD9,0xD0,0xD7,0xC2,0xC5,0xCC,0xCB,0xE6,0xE1,0xE8,0xEF,0xFA,0xFD,0xF4,0xF3
};

//�ŵ����1����ֵ���ñ�
const u32 cSH367309OCD1Tab[16] =
{
	//��λ  10mA
    2000000,3000000,4000000,5000000,6000000,7000000,8000000,9000000,
    10000000,11000000,12000000,13000000,14000000,16000000,18000000,20000000,
};

//�ŵ����2����ֵ���ñ�
const u32 cSH367309OCD2Tab[16] =
{
	//��λ  10mA
    3000000,4000000,5000000,6000000,7000000,8000000,9000000,10000000,
    12000000,14000000,16000000,18000000,20000000,30000000,40000000,50000000,
};

//��·����ֵ���ñ�
const u32 cSH367309SCTab[16] =
{
	//��λ  10mA
    5000000,8000000,11000000,14000000,17000000,20000000,23000000,26000000,
    29000000,32000000,35000000,40000000,50000000,60000000,80000000,100000000,
};

//��������ֵ���ñ�
const u32 cSH367309OCCTab[16] =
{
	//��λ  10mA
    2000000,3000000,4000000,5000000,6000000,7000000,8000000,9000000,
    10000000,11000000,12000000,13000000,14000000,16000000,18000000,20000000,
};

//=============================================================================================
//����ӿں���
//=============================================================================================
//=============================================================================
//��������: void SH367309Init(void)
//�������: void
//�������: void
//��������: SH367309�ϵ��ʼ��
//ע������:
//=============================================================================
void SH367309Init(void)
{
	PT_INIT(&gPTSH367309);
	PT_INIT(&gPTSH367309Norm);    
    gSHAFEData.runstatus = e_SH_AFE_Sta_Init;
//    gSHAFEData.afechn = BSPIICChannalReq(IIC_ROUTE_AFE); ----------------
    gSHAFECfg.adcgain = 0;    //SH367309��AD����
    gSHAFECfg.adcoffset = 0;  //SH367309��ADƫ��
    
	//��ȡУ׼�������Ʈֵ
	gSHAFECfg.calres = gConfig.calpara.sampleres;
    gSHAFECfg.Cadcoffset = gConfig.calpara.sampleoff;
    
	gSHAFEData.runstatus = e_SH_AFE_Sta_Bqinit;
}


//=============================================================================================
//��������	: static void SH367309ClearData(void)
//��������	: void
//�������	: void
//��̬����	: void
//��    ��	: ��������ĵ�ѹ���¶ȡ���������
//ע    ��	:
//=============================================================================================
static void SH367309ClearData(void)
{
    u8 i;
    /* �������󣬲����AFE���ݣ�����Ƿѹ */
    for(i = 0;i < BAT_CELL_NUM;i++)
    {
    	//gBatteryInfo.Data.CellVolt[i] = 0;
    }
    for(i = 0;i < BAT_TEMP_NUM;i++)
    {
    	//gBatteryInfo.Data.CellTemp[i] = NTC_TEMP_OFF;
    }
    gBatteryInfo.Data.Curr = 0;
}

//=============================================================================================
//��������	: u8 SH367309CalcCRC(u8 *datadataptr, u8 len)
//��������	: dataptr��Ҫ����CRC������ָ��, len�����鳤��
//�������	:
//��̬����	:
//��    ��	: �������ݵ�CRC
//ע    ��	:
//=============================================================================================
u8 SH367309CalcCRC(u8 *dataptr, u8 len)
{
    u8 crc8 = 0;    
    
	for(; len > 0; len--)
	{    
        crc8 = CRC8Table[crc8^*dataptr];    
	    dataptr++;    
    } 
       
    return(crc8);       
}

//=============================================================================================
//��������	: u8 SH367309WriteRegister(e_IICNUM num, u8 regaddr,u8* p_buffer)
//�������	: num:eIIC0/eIICMax	regaddr:SH367309�Ĵ�����ַ      *p_buffer:���ͻ�����		bufflen:���͵����ݸ���
//�������	: void
//��̬����	: void
//��    ��	: SH367309д1���Ĵ�������,����
//ע    ��	:
//=============================================================================================
u8 SH367309WriteRegister(I2C_TypeDef *hi2cx, u8 regaddr,u8* p_buffer)
{
	u8 errstat = 0;									//IICͨ�Ŵ�����Ϣ
	u8 regaddress = 0;
	u8 deviceaddr = 0;

	if((hi2cx != SH367309_IIC_CHANNEL))  //������Ч���ж�
	{
		return SH367309_IIC_FAULT_STAT;
	}

	deviceaddr = SH367309_WRITE_ADDR;
	regaddress = (u8)(regaddr);

	gSH367309Writebuff[0] = deviceaddr;
	gSH367309Writebuff[1] = regaddress;
	gSH367309Writebuff[2] = *p_buffer;
	gSH367309Writebuff[3] = SH367309CalcCRC(gSH367309Writebuff, 3);

	p_buffer++;	//ָ����һ���ֽ�

	if(SH367309_IIC_NOFAULT_STAT == errstat)
	{     
        //errstat = BSPIICSendMsg(num,gSH367309Writebuff,bufflen * 2 + 3,0);        SH367309_IIC_CHANNEL
        //BSPIICWrite(SH367309_IIC_CHANNEL,gSH367309Writebuff[0],&gSH367309Writebuff[1], 3); 
				//BSPIICWrite(SH367309_IIC_CHANNEL,gSH367309Writebuff[0],&gSH367309Writebuff[1], 3); 
        BSP_I2C_MEM_WRITE(SH367309_IIC_CHANNEL,gSH367309Writebuff[1],&gSH367309Writebuff[2], 2); 
	}

	 return errstat;    
}

//=============================================================================================
//��������	: u8 SH367309ReadNRegisters(e_IICNUM num,u8* p_buffer, u8 bufflen, u16 regaddr)
//�������	: num:eIIC0/eIICMax	*p_buffer:���ջ�����		bufflen:���յ����ݸ���	regaddr:SH367309�Ĵ�����ַ
//�������	: IICͨ�Ŵ�����Ϣ
//��̬����	: void
//��	��	: SH367309��ȡ��Ϣ����.�����ȴ�
//ע	��	:
//=============================================================================================
u8 SH367309ReadNRegisters(I2C_TypeDef *hi2cx , u8* p_buffer, u8 bufflen, u8 regaddr)
{
	u8 errstat = 0;			//IICͨ�Ŵ�����Ϣ
	u16 regaddress = 0;	//�Ĵ�����ַ

	if((hi2cx != SH367309_IIC_CHANNEL) || (p_buffer == NULL))  //������Ч���ж�
	{
		return SH367309_IIC_FAULT_STAT;
	}

	regaddress = (u8)regaddr;
    
    gSH367309Writebuff[0] = regaddress;
    gSH367309Writebuff[1] = bufflen;    
    regaddress = (((u16)gSH367309Writebuff[0])<<8) | ((u16)gSH367309Writebuff[1]);
		//errstat = BSPIICRcvMsg(num,SH367309_WRITE_ADDR,regaddress,p_buffer,bufflen * 2,0);	//���Ͷ�ȡ���ģ���׼��ַ
    //BSPIICWriteAndRead(gSHAFEData.afechn,SH367309_WRITE_ADDR,&gSH367309Writebuff[0],2,p_buffer,bufflen+1); 
    BSPIICWriteAndRead(SH367309_IIC_CHANNEL,SH367309_WRITE_ADDR,&gSH367309Writebuff[0],2,p_buffer,bufflen); 
		//BSP_I2C_MEM_REAR(SH367309_IIC_CHANNEL,regaddress,p_buffer,bufflen+1); 
    
	return errstat;
}

//=============================================================================================
//��������	: void SH367309RefreshDataToBatInfo(void)
//�������	: void
//�������	: void
//��̬����	: void
//��    ��	: SH367309ˢ�����ݵ�battery info��
//ע    ��	:
//=============================================================================================
void SH367309RefreshDataToBatInfo(void)
{
	u8 i = 0,j = 0;

	//���µ����ѹ��BATTERY INFO
	for(i = 0;i < SH367309_VOLT_NUM;i++)
	{
		if(1 == BITGET32(SH367309_VALID_BAT,i))		//��ȡ��Ч�ı�־λ
		{
			gBatteryInfo.Data.CellVolt[j] = gSHAFEData.cellvolt[i];
			j++;
		}
	}

    //�������¶�ʹ��AFE����
    gSampleData.ConnTemp = gSHAFEData.celltemp[2];
	
	//���µ����¶ȵ�BATTERY INFO
	//�����¶��޸ĵ��Թ��� 210629
    if(True == g_AFE_Test_VAR.IsModTemp)
		gBatteryInfo.Data.CellTemp[0] = g_AFE_Test_VAR.ModTempVal;
    else
        gBatteryInfo.Data.CellTemp[0] = gSHAFEData.celltemp[0];
    //gBatteryInfo.Data.CellTemp[0] = gSHAFEData.celltemp[0];
	gBatteryInfo.Data.CellTemp[1] = gSHAFEData.celltemp[1];
    
//    if((gBatteryInfo.Data.CellTemp[0] < -400) || (gBatteryInfo.Data.CellTemp[0] > 1200))    //����Ҫ�жϣ��������������ж�
//    {
//        gBatteryInfo.Data.CellTemp[0] = NTC_TEMP_OFF;
//    }
//    if((gBatteryInfo.Data.CellTemp[1] < -400) || (gBatteryInfo.Data.CellTemp[1] > 1200))
//    {
//        gBatteryInfo.Data.CellTemp[1] = NTC_TEMP_OFF;
//    }    

	//���µ���������BATTERY INFO
    if(gSHAFEData.curr > 0x7fffffff)
    {
        gBatteryInfo.Data.Curr = (s16)((gSHAFEData.curr - 0xffffffff) / 10);
    }
	else
    {
        gBatteryInfo.Data.Curr = (s16)((gSHAFEData.curr) / 10);    
    }

    gBatteryInfo.Status.MOSStatus = gSHAFEData.mosstatus;
}

//=============================================================================================
//��������	: u16 SH367309EventFlagGet(void)
//�������	: void
//�������	: SH367309�����־λ
//��̬����	: void
//��    ��	: ��ȡSH367309�����־λ
//ע    ��	:
//=============================================================================================
u16 SH367309EventFlagGet(void)
{
	return gSHAFECtrl.eventctrl;
}

//=============================================================================================
//��������	: void SH367309EventFlagSet(u16 eventflag)
//�������	: void
//�������	: void
//��̬����	: void
//��    ��	: SH367309������:�����־��λ
//ע    ��	:
//=============================================================================================
void SH367309EventFlagSet(u16 eventflag)
{
	gSHAFECtrl.eventctrl |= eventflag;
}

//=============================================================================================
//��������	: void SH367309EventFlagSet(u16 eventflag)
//�������	: void
//�������	: void
//��̬����	: void
//��    ��	: SH367309������:�����־��λ
//ע    ��	:
//=============================================================================================
void SH367309EventFlagClear(u16 eventflag)
{
	gSHAFECtrl.eventctrl &= ~(eventflag);
}

//=============================================================================================
//��������	: void SH367309MosfetSet(uint8_t mos)
//�������	: mos:mos�ܿ���״̬
//�������	: void
//��̬����	: void
//��    ��	: SH367309����MOS����״̬
//ע    ��	:
//=============================================================================================
void SH367309MosfetSet(u8 mos)
{
	gSHAFECtrl.mosctrl = mos & 0x03;
	SH367309EventFlagSet(SH367309_EVE_CTRL_MOS);
    
  //BSPGPIOSetPin(SH367309_CTL_PORT,SH367309_CTL_PIN);
	HAL_GPIO_WritePin(CTL_AFE_PORT, CTL_AFE_PIN, GPIO_PIN_SET);//��ȷ��--------------------------------

}

//=============================================================================================
//��������	: u8 SH367309MosfetGet(void)
//�������	: void
//�������	: MOS����״̬
//��̬����	: void
//��    ��	: SH367309��ȡMOS����״̬
//ע    ��	:
//=============================================================================================
u8 SH367309MosfetGet(void)
{
    //return (gSH367309Reg.SysCtrl2.SysCtrl2Byte) & 0x03;
    return gSHAFEData.mosstatus & 0x03;
}

//=============================================================================================
//��������	: SH367309CalADtoVolt(u8 hibyte, u8 lobyte)
//�������	: hibyte:16λAD���ֽ�	lobyte��16λAD���ֽ�
//�������	: ת�����ʵ�ʵ�ѹֵ
//��������	: ��ѹADֵת��Ϊʵ�ʵ�ѹֵ
//ע������	:
//=============================================================================================
u16 SH367309CalADtoVolt(u8 hibyte, u8 lobyte)
{
    s32 ret;
    ret = (s32)((((u32)((hibyte))) << 8) | (lobyte));
    
    //ret = ret * gSHAFECfg.adcgain / 1000 + gSHAFECfg.adcoffset;
    ret = ret * 5 / 32;
    
    return (u16)ret;
}

//=============================================================================
//��������: s32 SH367309CalCADCtoCurr(u8 hibyte, u8 lobyte)
//�������: hibyte:16λAD���ֽ�	lobyte��16λAD���ֽ�
//�������: ת�����ʵ�ʵ���ֵ
//��������: ����CADCֵת��Ϊʵ�ʵ���ֵ
//ע������: ������λΪ1mA
//=============================================================================
s32 SH367309CalCADCtoCurr(u8 hibyte, u8 lobyte)
{
    s32 ret;
    
    ret = (s32)(s16)((((u16)hibyte)<<8)|(lobyte));
    gSHAFEData.curradval = ret;
    ret = (s32)((float)(ret - gSHAFECfg.Cadcoffset) * SH367309_CALICUR) / (21.47 * (float)gSHAFECfg.calres);	//1000 uR
    return ret;
}

//=============================================================================
//��������: s32 SH367309CalADtoCurr(u8 hibyte, u8 lobyte)
//�������: hibyte:16λAD���ֽ�	lobyte��16λAD���ֽ�
//�������: ת�����ʵ�ʵ���ֵ
//��������: ����ADֵת��Ϊʵ�ʵ���ֵ
//ע������: ������λΪ1mA
//=============================================================================
s32 SH367309CalADtoCurr(u8 hibyte, u8 lobyte)
{
    s32 ret;
    
    ret = (s32)(s16)((((u16)hibyte)<<8)|(lobyte));
    ret = (s32)((float)(ret - gSHAFECfg.Cadcoffset) * SH367309_CALICUR) / (26.837 * (float)gSHAFECfg.calres);	//1000 uR
    return ret;
}

//=============================================================================
//��������: s16 SH367309CalADtoOVTrip(u8 ov)
//�������: ��ȡ�ļĴ���ov trip��ֵ��eepread:t_SH367309_EEPRomָ��
//�������: ת����ı���ֵ
//��������: ת��ADֵΪ��ѹ��������ֵ
//ע������:
//=============================================================================
s16 SH367309CalADtoOVTrip(t_SH367309_EEPRom* eepread)
{
    u16 tmp = 0;
    
    tmp = (((u16)(eepread->OVH.OVHBit.OV8_9) << 8) | (u16)(eepread->OVL.OVLBit.OV0_7))\
           * SH367309_EEP_OV_GAIN;
    
    return (s16)tmp;
}

//=============================================================================
//��������: s16 SH367309CalADtoUVTrip(t_SH367309_EEPRom* eepread)
//�������: ��ȡ�ļĴ���uv trip��ֵ��eepread:t_SH367309_EEPRomָ��
//�������: ת����ı���ֵ
//��������: ת��ADֵΪǷѹ��������ֵ
//ע������:
//=============================================================================
s16 SH367309CalADtoUVTrip(t_SH367309_EEPRom* eepread)
{
    u16 tmp = 0;
    
    tmp = eepread->UV.UVBit.UV0_7 * SH367309_EEP_UV_GAIN;
    
    return (s16)tmp;
}

//=============================================================================
//��������: s16 SH367309CalTemptoOTAD(s16 temp)
//�������: temp:���ñ������¶�
//�������: ת����ı���ֵ
//��������: ת�����´�������ֵΪADֵ
//ע������:
//=============================================================================
u8 SH367309CalTemptoOTAD(s16 temp)
{
    u8 tmp = 0;
    float Rref = 0;
    float Rt1 = 0;

    Rt1 = NTCFindResByTemp(temp) / 10; //������ֵ
    Rref = 5 * gSHAfeEepRead.TR.TRBit.TR0_6 + SH367309_REF_RES_VAL;
    
    tmp = (u8)(((float)(Rt1 / (Rt1 + Rref))) * 512);
    
    return tmp;
}

//=============================================================================
//��������: s16 SH367309CalTemptoUTAD(s16 temp)
//�������: temp:���ñ������¶�
//�������: ת����ı���ֵ
//��������: ת��Ƿ�´�������ֵΪADֵ
//ע������:
//=============================================================================
u8 SH367309CalTemptoUTAD(s16 temp)
{
    u8 tmp = 0;
    float Rref = 0;
    float Rt1 = 0;

    Rt1 = NTCFindResByTemp(temp) / 10; //������ֵ
    Rref = 5 * gSHAfeEepRead.TR.TRBit.TR0_6 + SH367309_REF_RES_VAL;
    
    tmp = (u8)((((float)(Rt1 / (Rt1 + Rref))) - 0.5) * 512);
    
    return tmp;
}

//=============================================================================
//��������: s16 SH367309CalCurrentRes(s32 actcurrent)
//�������: У׼����ֵ
//�������: У׼������Ӧ�Ĳ�������
//��������: ����У׼������Ӧ�Ĳ�������
//ע������:
//=============================================================================
s16 SH367309CalCurrentRes(s32 actcurrent)
{
    //return ((s32)gAFEData.curradval * 8440 / actcurrent);
    //return (gSHAFEData.curradval - gSHAFECfg.Cadcoffset) * SH367309_CALICUR / actcurrent;
    return (u16)(((float)(gSHAFEData.curradval - gSHAFECfg.Cadcoffset) * SH367309_CALICUR) / (21.47 * (float)actcurrent));	//1000 uR
}

//=============================================================================
//��������: u16 SH367309GetRealSCDCurr(void)
//�������: eepread:t_SH367309_EEPRomָ��
//�������: SH367309��ʵ�Ķ�·������������ֵ
//��������: ��ȡ��ʵ�Ķ�·������������ֵ
//ע������:
//=============================================================================
u16 SH367309GetRealSCDCurr(t_SH367309_EEPRom* eepread)
{
	return cSH367309SCTab[eepread->SC.SCBit.SCV] / gSHAFECfg.calres;
}

//=============================================================================
//��������: u16 SH367309GetRealOCD1Curr(void)
//�������: eepread:t_SH367309_EEPRomָ��
//�������: SH367309��ʵ�ķŵ����1��������ֵ
//��������: ��ȡ��ʵ�ķŵ����1��������ֵ
//ע������:
//=============================================================================
u16 SH367309GetRealOCD1Curr(t_SH367309_EEPRom* eepread)
{
	return cSH367309OCD1Tab[eepread->OCD1.OCD1Bit.OCD1V] / gSHAFECfg.calres;
}

//=============================================================================
//��������: u16 SH367309GetRealOCD2Curr(void)
//�������: eepread:t_SH367309_EEPRomָ��
//�������: SH367309��ʵ�ķŵ����1��������ֵ
//��������: ��ȡ��ʵ�ķŵ����2��������ֵ
//ע������:
//=============================================================================
u16 SH367309GetRealOCD2Curr(t_SH367309_EEPRom* eepread)
{
	return cSH367309OCD2Tab[eepread->OCD2.OCD2Bit.OCD2V] / gSHAFECfg.calres;
}

//=============================================================================================
//��������	: void SH367309DevStatClear(u8 mask)
//�������	: mask����Ҫ�����λ
//�������	: void
//��̬����	: void
//��    ��	:SH367309����豸״̬��־
//ע    ��	:
//=============================================================================================
void SH367309DevStatClear(u8 mask)
{
	gSHAFEData.devsta &= ~mask;
}

//=============================================================================================
//��������	: void SH367309AlertIntCallback(void)
//��������	: void
//�������	: void
//��̬����	: void
//��    ��	: AFE�澯�����жϵ��ú���
//ע    ��	:
//=============================================================================================
void SH367309AlertIntCallback(void)
{
    gSHAFEData.IntFlg = True;
}

//=============================================================================
//��������: void SH367309CalCurrent(s16 actcurrent)
//�������: actcurrent��У׼�ĵ���
//�������: void
//��������: SH367309У׼��������
//ע������: void
//=============================================================================
void SH367309CalCurrent(s16 actcurrent)
{
    gSHAFECfg.calcurr = actcurrent;
    SH367309EventFlagSet(SH367309_EVE_CTRL_CAL);
}

//=============================================================================
//��������: u8 SH367309OUVConfig(s16 ov,s16 ovr,s16 uv,s16 uvr)
//�������: ov:��ѹ����ֵ��ovr:��ѹ�ָ�ֵ��uv��Ƿѹ����ֵ��uvr:Ƿѹ�ָ�ֵ
//�������: ���ý�� �ɹ�:SH367309_IIC_NOFAULT_STAT;  ʧ��:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//��������: SH367309��ѹ��Ƿѹ����
//ע������:
//=============================================================================
u8 SH367309OUVConfig(s16 ov,s16 ovr,s16 uv,s16 uvr)
{
    u8 result = 0;

     //OVH OVL
    gSHAfeEep.OVH.OVHBit.OVT = SH367309_EEP_OVT_DLY_4S;             //����籣����ʱ
    gSHAfeEep.OVH.OVHBit.LDRT = SH367309_EEP_LDRT_DLY_1000MS;       //�����ͷ���ʱ���ÿ���λ
    //��ѹ��������
    gSHAfeEep.OVH.OVHBit.OV8_9 = (u8)(((ov / SH367309_EEP_OV_GAIN) & 0x0300) >> 8);
    gSHAfeEep.OVL.OVLBit.OV0_7 = (u8)((ov / SH367309_EEP_OV_GAIN) & 0x00ff);

    //OVRH OVRL
    gSHAfeEep.OVRH.OVRHBit.UVT = SH367309_EEP_UVT_DLY_4S;   //���ŵ籣����ʱ����
    //��ѹ�ָ�����
    gSHAfeEep.OVRH.OVRHBit.OVR8_9 = (u8)(((ovr / SH367309_EEP_OVR_GAIN) & 0x0300) >> 8);
    gSHAfeEep.OVRL.OVRLBit.OV0_7 = (u8)((ovr / SH367309_EEP_OVR_GAIN) & 0x00ff);
    
    //UV UVR
    gSHAfeEep.UV.UVBit.UV0_7 = (u8)((uv / SH367309_EEP_UV_GAIN) & 0x00ff);
    gSHAfeEep.UVR.UVRBit.UVR0_7 = (u8)((uvr / SH367309_EEP_UVR_GAIN) & 0x00ff);

	SH367309EventFlagSet(SH367309_EVE_CTRL_OUVD);

	return result;
}

//=============================================================================
//��������: u16 SH367309FindSOCDTable(u32 value , const u32 * tab , u16 range )
//�������: value�����ҵ�ֵ	* tab�����ұ��ָ��		range:���ҷ�Χ
//�������: �������������Ҵ�����ظ�0xffff
//��������: ���ұ�����
//ע������:
//=============================================================================
u16 SH367309FindSOCDTable(u32 value , const u32 * tab , u16 range )
{
    u32 curr1,curr2;

    curr1 = (tab[range-1] / gSHAFECfg.calres);	//����˱����ֵ��ȷ���Ƿ�ֵ������

    if(value > curr1)
    {
        return 0xffff;		//������ظ�0xffff
    }
    else if(value==curr1)
    {
        return range-1;		//������ظ��˱����ֵ����������
    }

    range--;

    while(range--)
    {
        curr2 = tab[range] / gSHAFECfg.calres;

        if(value >= curr2)
        {
            if((value - curr2) <= (curr1 - value))	//�жϱ���ֵ��ʵ��ֵ�ӽ��̶�
            {
                return range;
            }
            else
            {
                return range + 1;
            }
        }
        curr1 = curr2;
    }
    return 0;
}

//=============================================================================
//��������: u8 SH367309OCD1Config(u16 ocd1value)
//�������: ocd1value���ŵ����1����ֵ
//�������: ���ý�� �ɹ�:SH367309_IIC_NOFAULT_STAT;  ʧ��:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//��������: SH367309�ŵ����1����
//ע������:
//=============================================================================
u8 SH367309OCD1Config(u16 ocd1value)
{
	u8 result = 0;			//���ý��
	u16 index = 0;	        //���ñ�����
    
    index = SH367309FindSOCDTable(ocd1value, cSH367309OCD1Tab,16);
    
    if(0xffff == index)
    {
        index = SH367309_EEP_OCD1V_200MV;
    }
        
    #if defined(LFP_HL_25000MAH_16S)
    gSHAfeEep.OCD1.OCD1Bit.OCD1V = index;
    gSHAfeEep.OCD1.OCD1Bit.OCD1T = SH367309_EEP_OCD1T_DLY_1S;    
    #else
    gSHAfeEep.OCD1.OCD1Bit.OCD1V = index + 1;
    gSHAfeEep.OCD1.OCD1Bit.OCD1T = SH367309_EEP_OCD1T_DLY_30S;
    #endif
    
    return result;    
}

//=============================================================================
//��������: u8 SH367309OCD1Config(u16 ocd2value)
//�������: ocd2value���ŵ����2����ֵ
//�������: ���ý�� �ɹ�:SH367309_IIC_NOFAULT_STAT;  ʧ��:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//��������: SH367309�ŵ����2����
//ע������:
//=============================================================================
u8 SH367309OCD2Config(u16 ocd2value)
{
	u8 result = 0;			//���ý��
	u16 index = 0;	        //���ñ�����
    
    index = SH367309FindSOCDTable(ocd2value, cSH367309OCD2Tab,16);
    
    if(0xffff == index)
    {
        index = SH367309_EEP_OCD2V_500MV;
    }
        
    #if defined(LFP_HL_25000MAH_16S)
    gSHAfeEep.OCD2.OCD2Bit.OCD2V = index;
    gSHAfeEep.OCD2.OCD2Bit.OCD2T = SH367309_EEP_OCD2T_DLY_200MS;    
    #else
    gSHAfeEep.OCD2.OCD2Bit.OCD2V = index;
    gSHAfeEep.OCD2.OCD2Bit.OCD2T = SH367309_EEP_OCD2T_DLY_1S;
    #endif
    
    return result;    
}

//=============================================================================
//��������: u8 SH367309SCConfig(u16 scvalue)
//�������: scvalue����·����ֵ
//�������: ���ý�� �ɹ�:SH367309_IIC_NOFAULT_STAT;  ʧ��:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//��������: SH367309��·����
//ע������:
//=============================================================================
u8 SH367309SCConfig(u16 scvalue)
{
	u8 result = 0;			//���ý��
	u16 index = 0;	        //���ñ�����
    
    index = SH367309FindSOCDTable(scvalue, cSH367309SCTab,16);
    
    if(0xffff == index)
    {
        index = SH367309_EEP_SCV_1000MV;
    }
        
    gSHAfeEep.SC.SCBit.SCV = index;
    gSHAfeEep.SC.SCBit.SCT = SH367309_EEP_SCT_DLY_0US;
    
    return result;
}

//=============================================================================
//��������: u8 SH367309SCConfig(u16 scvalue)
//�������: scvalue����·����ֵ
//�������: ���ý�� �ɹ�:SH367309_IIC_NOFAULT_STAT;  ʧ��:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//��������: SH367309��·����
//ע������:
//=============================================================================
u8 SH367309OCCConfig(u16 occvalue)
{
	u8 result = 0;			//���ý��
	u16 index = 0;	        //���ñ�����
    
    index = SH367309FindSOCDTable(occvalue, cSH367309OCCTab,16);
    
    if(0xffff == index)
    {
        index = SH367309_EEP_OCCV_200MV;
    }
        
    gSHAfeEep.OCC.OCCBit.OCCV = index;
    gSHAfeEep.OCC.OCCBit.OCCT = SH367309_EEP_OCCT_DLY_1S;
    
    return result;
}

//=============================================================================
//��������: u8 SH367309OCConfig(u16 ocd1value,u16 ocd2value,u16 scdvalue,u16 occvalue)
//�������: ocd1value���ŵ����1������ocd2value���ŵ����2������scdvalue:�ŵ��·��������ֵ	occvalue������������ֵ
//�������: ���ý�� �ɹ�:SH367309_IIC_NOFAULT_STAT;  ʧ��:SH367309_IIC_FAULT_STAT/SH367309_IIC_CRCFAULT_STAT
//��������: SH367309 ���ó�ŵ��������·����
//ע������:
//=============================================================================
u8 SH367309OCConfig(u16 ocd1value,u16 ocd2value,u16 scdvalue,u16 occvalue)
{
	u8 result = 0;			//���ý��

    ocd1value += 2000;  //���������������+20A��ΪӲ����������ֵ
    result |= SH367309OCD1Config(ocd1value);
    result |= SH367309OCD2Config(ocd2value);
    result |= SH367309SCConfig(scdvalue);
    result |= SH367309OCCConfig(occvalue);

    SH367309EventFlagSet(SH367309_EVE_CTRL_OSCD);

	return result;
}


//=============================================================================
//��������: s8 BQ769X0_ChangADtoTemp(u8 hibyte, u8 lobyte)
//�������: TempAD����ADֵ
//�������: ת������¶�ֵ
//��������: �¶�ADֵת��Ϊʵ���¶�ֵ
//ע������:
//=============================================================================
s16 SH367309CalADtoTemp(u8 hibyte, u8 lobyte)
{
    s32 getdata;
//	u8	i,TempeMiddle;
	u32	tempcalcu;
    
    getdata = (s32)(s16)((((u16)hibyte)<<8)|(lobyte));
    
//    if(res >= NTC_REFV_LSB)
//    {
//        return NTC_TEMP_OFF;
//    }
//    res = 10000 * res / (NTC_REFV_LSB - res);

//    return NTCTempQuery(res , NTC_RESTAB[0].tab, NTC_RESTAB[0].tablen, NTC_RESTAB[0].tabofs);
    
	tempcalcu = (u32)getdata * (5 * gSHAfeEepRead.TR.TRBit.TR0_6 + SH367309_REF_RES_VAL) / (32768 - getdata);
    
    return NTCTempQuery(tempcalcu * 10 , NTC_RESTAB[0].tab, NTC_RESTAB[0].tablen, NTC_RESTAB[0].tabofs);

//	if(tempcalcu >= NTC103AT[0])			//Determine whether the excess temperature resistance range
//	{
//		Temperature = 2731-500;			
//	}
//	else if(tempcalcu <= NTC103AT[160])
//	{
//		Temperature = 2731+1100;
//	}
//	else
//	{
//		i = TempeMiddle;
//		if(tempcalcu > NTC103AT[i])
//		{
//			for(i=TempeMiddle-1; i>0; i--)
//			{
//				if(tempcalcu <= NTC103AT[i])		//NTC103AT[i+1]<resis<NTC103AT[i]
//				{
//					break;
//				}
//			}
//		}
//		else
//		{
//			for(i=TempeMiddle+1; i<160; i++)
//			{
//				if(tempcalcu > NTC103AT[i])		//NTC103AT[i-1]<resis<NTC103AT[i]
//				{
//					break;
//				}
//			}
//			i--;
//		}
//		TempeMiddle = i;
//		
//		Temperature = (u16)(TempeMiddle-50)*10+(NTC103AT[i]-tempcalcu)*10/(NTC103AT[i]-NTC103AT[i+1])+2731;
//	}
//	return Temperature;    
}

//=============================================================================================
//��������	: u8 SH367309DirectReadData(u16 *volt,s16 *temp,s16 *curr);
//�������	: volt:���õĵ�ѹ����ָ��
//�������	: �Ƿ�����0���޴���1����ȡiic����2��У��crc����
//��������	: ֱ�ӻ�ȡAFE���������ݣ�������
//ע������	:
//=============================================================================================
//u8 SH367309DirectReadData(u16 *volt,s16 *temp,s16 *curr)	//�ú�����ʱ��ʹ��
//{
//    status_t u8Status = STATUS_SUCCESS;	
//    u8 i = 0;
//    
//    gSH367309Writebuff[0] = SH367309_REG_CONF;
//    gSH367309Writebuff[1] = sizeof(gSHAFEReg); 
//    
//    u8Status = BSPIICRcvMsgEx(SH367309_IIC_CHANNEL,SH367309_WRITE_ADDR,gSH367309Writebuff,2,
//                                                        (u8 *)&gSHAFEReg,sizeof(gSHAFEReg),1);
//    
//    if(STATUS_SUCCESS != u8Status)      
//    {
//        gSHAFEData.iicerr ++;
//    }            
//    else
//    {
//        *curr = SH367309CalADtoCurr(gSHAFEReg.CurrH,gSHAFEReg.CurrL);
//        
//        //��ȡ�����¶�ֵ
//        for(i = 0;i < SH367309_TEMP_NUM;i++)
//        {
//            //�����¶�adת��
//            *temp++ = SH367309CalADtoTemp(*(((u8 *)&gSHAFEReg.Temp1H) + 2 * i),\
//                                                         *(((u8 *)&gSHAFEReg.Temp1L) + 2 * i) );                
//        }
//        
//        //��ȡ�����ѹֵ
//        for(i = 0;i < SH367309_VOLT_NUM;i++)
//        {
//            //�����ѹadת��
//            *volt++ = SH367309CalADtoVolt(*(((u8 *)&gSHAFEReg.Cell1H) + 2 * i),\
//                                                         *(((u8 *)&gSHAFEReg.Cell1H) + 1 + 2 * i) );
//        }
//    }
//   
//    return (u8)STATUS_SUCCESS;
//}

//=============================================================================================
//��������	: u8 SH367309BqInitReset(void)
//�������	: void
//�������	: void
//��̬����	: void
//��    ��	: SH367309��λ
//ע    ��	:
//=============================================================================================
void SH367309BqInitReset(void)
{
	PT_INIT(&gPTSH367309);
	PT_INIT(&gPTSH367309Norm);    
    
    gSHAFEData.ResetFlg = True;
	gSHAFEData.runstatus = e_SH_AFE_Sta_Bqinit;    
}

//=============================================================================================
//��������	: u8 SH367309MemCopy(const u8 *msg1, const u8 *msg2, u16 cnt)
//�������	: dest��Ŀ�����ݵ�ַ�� *sour��Դ���ݵ�ַ��cnt�����Ƶĳ���
//�������	:
//��������	: ���ݸ���
//ע������	:
//=============================================================================================
void SH367309MemCopy(u8 *dest, const u8 *sour, u16 cnt)
{
    while(0 < cnt--)
    {
       *(dest++) = *(sour++);
    }
}

//=============================================================================================
//��������	: u8 SH367309MemComp(const u8 *msg1, const u8 *msg2, u16 cnt)
//�������	: msg1���Ƚ�����1��msg2���Ƚϵ�����2��cnt���Ƚϵĳ���
//�������	: TRUE���Ƚ���ȷ��FALSE���Ƚϴ���
//��������	: ���ݱȽ�
//ע������	:
//=============================================================================================
u8 SH367309MemComp(const u8 *msg1, const u8 *msg2, u16 cnt)
{
    while(0 < cnt--)
    {
        if(*(msg1++) != *(msg2++))
        {
            return FALSE;
        }
    }
    return TRUE;
}

//=============================================================================
//��������: static void SH367309ConfigInitTask(void)
//�������: void
//�������: void
//��������: SH367309�ϵ����ó�ʼ������
//ע������: 50ms����
//=============================================================================
static void SH367309ConfigInitTask(void)
{
	static u8 result = SH367309_IIC_NOFAULT_STAT;
	static u16 timer = 0;
    static u8 slaststep = 0;
	static u8 sstep = 0;
	static u8 errcnt = 0;
    static u8 firstflag = 0;
    static u8 i = 0;
    u8 data = 0;
    
    if(gSHAFEData.ResetFlg == True)
    {
        gSHAFEData.ResetFlg = False;
        result = SH367309_IIC_NOFAULT_STAT;
        timer = 0;
        slaststep = 0;
        sstep = 0;
        firstflag = 0;
        i = 0;
    }
    
	switch(sstep)
	{
        case 0:
            SH367309_SHIP_OFF;  /* ����ģʽ�ر� */
			sstep++;
            BSPTaskStart(TASK_ID_SH367309_TASK, 250);	
			break;
        
        //clear 309 WAKE_FLG
		case 1:
            data = 0x00;
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BFLAG1,&data);
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;   
        
        //clear 309 Protect flag
		case 2:
            data = 0x00;
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BFLAG2,&data);
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;                       
        
        
        case 3:    
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,(u8 *)&gSHAfeEepRead,\
                                                            sizeof(t_SH367309_EEPRom),SH367309_EEP_SCONF1);
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;
        
        /* �����ò�����ֵ */
		case 4:
           //SCONF1
            gSHAfeEep.SCONF1.SCONF1Bit.ENPCH = SH367309_EEP_ENPCH_DISABLE;  //����Ԥ��繦��
            gSHAfeEep.SCONF1.SCONF1Bit.ENMOS = SH367309_EEP_ENMOS_ENABLE;   //���ó��MOSFET�ָ�����λ���������/�¶ȱ����رճ��MOSFET�������⵽�ŵ����1���߷ŵ�״̬���������MOSFET
            gSHAfeEep.SCONF1.SCONF1Bit.OCPM = SH367309_EEP_OCPM_DISABLE;    //������ֻ�رճ��MOSFET���ŵ����ֻ�رշŵ�MOSFET
            gSHAfeEep.SCONF1.SCONF1Bit.BAL = SH367309_EEP_BAL_MCU;          //ƽ�⿪�����ⲿMCU���ƣ�ƽ��ʱ������SH367309�ڲ��߼�����
        
            #ifdef BMS_BAT_16S
            gSHAfeEep.SCONF1.SCONF1Bit.CN = SH367309_EEP_CELL16;            //16�ڵ�оӦ��  
            #elif defined(BMS_BAT_15S)
            gSHAfeEep.SCONF1.SCONF1Bit.CN = SH367309_EEP_CELL15;            //15�ڵ�оӦ��
            #elif defined(BMS_BAT_14S)
            gSHAfeEep.SCONF1.SCONF1Bit.CN = SH367309_EEP_CELL14;            //15�ڵ�оӦ��        
            #endif
        
            //SCONF2
            gSHAfeEep.SCONF2.SCONF2Bit.EOVB = SH367309_EEP_E0VB_ENABLE;     //��������ֹ��ѹ��о��硱����
            gSHAfeEep.SCONF2.SCONF2Bit.UV_OP = SH367309_EEP_UV_OP_DISABLE;  //���ŵ�ֻ�رշŵ�MOSFET
            gSHAfeEep.SCONF2.SCONF2Bit.DIS_PF = SH367309_EEP_DIS_PF_DISABLE; //��ֹ���ι���籣��
            gSHAfeEep.SCONF2.SCONF2Bit.CTLC = SH367309_EEP_CTLC_ALL;//SH367309_EEP_CTLC_INVALID;    //��ŵ��Ԥ���MOSFET���ڲ��߼����ƣ�CTL�ܽ�������Ч
            gSHAfeEep.SCONF2.SCONF2Bit.OCRA = SH367309_EEP_OCRA_ENABLE;     //�������������ʱ�ָ� 
            gSHAfeEep.SCONF2.SCONF2Bit.EUVR = SH367309_EEP_EUVR_DISABLE;    //���ŵ籣��״̬�ͷ����踺���ͷ�
                   
            //OVH OVL OVRH OVRL UV UVR ��ŵ籣���ͻָ�ֵ����
            SH367309OUVConfig(gConfig.cellov.matth[3],gConfig.cellov.resth[3],\
                              gConfig.celluv.matth[3],gConfig.celluv.resth[3]);
            SH367309EventFlagClear(SH367309_EVE_CTRL_OUVD);
        
            //BALV ƽ�⿪����ѹ
            gSHAfeEep.BALVByte = SH367309_CFG_BALV / SH367309_EEP_BALV_GAIN;
            
            //PREV Ԥ����ѹ�趨ֵ
            gSHAfeEep.PREVByte = SH367309_CFG_PREV / SH367309_EEP_PREV_GAIN;
        
            //L0V ��ѹ��ֹ����ѹ�趨ֵ
            gSHAfeEep.L0VByte = SH367309_CFG_L0V / SH367309_EEP_L0V_GAIN;
            
            //PFV ���ι�ѹ��籣����ѹ
            gSHAfeEep.PFVByte = SH367309_CFG_PFV / SH367309_EEP_PFV_GAIN;
            
            //OCD1 OCD2 SC OCC ��ŵ��������·��������
            SH367309OCConfig(0 - gConfig.dcurr.matth[1],0-gConfig.dcurr.matth[2],0 - gConfig.dcurr.matth[3],\
                             SH367309_CFG_OCC); //����������Ϊ20A
            SH367309EventFlagClear(SH367309_EVE_CTRL_OSCD);
                           
            //��ŵ�״̬����ѹ                           
            gSHAfeEep.MOST_OCRT_PFT.MOST_OCRT_PFTBit.CHS = SH367309_EEP_CHS_500UV; 
            //��ŵ�MOSFET������ʱ
            gSHAfeEep.MOST_OCRT_PFT.MOST_OCRT_PFTBit.MOST = SH367309_EEP_MOST_DLY_64US;
            //��ŵ�����Իָ���ʱ
            gSHAfeEep.MOST_OCRT_PFT.MOST_OCRT_PFTBit.OCRT = SH367309_EEP_OCRT_DLY_8S;
            //���ι���籣����ʱ
            gSHAfeEep.MOST_OCRT_PFT.MOST_OCRT_PFTBit.PFT = SH367309_EEP_PFT_DLY_8S;
            
            //�����±���������Ϊ�������ı����¶�
            gSHAfeEep.OTCByte = SH367309CalTemptoOTAD(gConfig.connot.matth[3]);            
            //gSHAfeEep.OTCByte = 0x78;
            
            //�����±����ͷ�
            gSHAfeEep.OTCRByte = SH367309CalTemptoOTAD(gConfig.connot.resth[3]);
            //gSHAfeEep.OTCRByte = 0x87;
            
            //�����±���
            gSHAfeEep.UTCByte = SH367309CalTemptoUTAD(gConfig.cellcut.matth[3] - SH367309_CFG_UT_OFFSET);
            //gSHAfeEep.UTCByte = 0x79;
            
            //�����±����ͷ�
            gSHAfeEep.UTCRByte = SH367309CalTemptoUTAD(gConfig.cellcut.resth[3] - SH367309_CFG_UT_OFFSET);
            //gSHAfeEep.UTCRByte = 0x63;      
            
            //�ŵ���±���������Ϊ�������ı����¶�
            gSHAfeEep.OTDByte = SH367309CalTemptoOTAD(gConfig.connot.matth[3]);
            //gSHAfeEep.OTDByte = 0x5E;
            
            //�ŵ���±����ͷ�
            gSHAfeEep.OTDRByte = SH367309CalTemptoOTAD(gConfig.connot.resth[3]);
            //gSHAfeEep.OTDRByte = 0x6B;
            
            //�ŵ���±���
            gSHAfeEep.UTDByte = SH367309CalTemptoUTAD(gConfig.celldut.matth[3] - SH367309_CFG_UT_OFFSET);
            //gSHAfeEep.UTDByte = 0x8E;
            
            //�ŵ���±����ͷ�
            gSHAfeEep.UTDRByte = SH367309CalTemptoUTAD(gConfig.celldut.resth[3] - SH367309_CFG_UT_OFFSET);
            //gSHAfeEep.UTDRByte = 0x79;          
            //�¶��ڲ��ο�����ϵ���Ĵ���
            //gSHAfeEep.TR.TRBit.TR0_6 = 0x37; //�޷�����             
            
            if(SH367309MemComp((u8 *)&gSHAfeEep,(u8 *)&gSHAfeEepRead,(sizeof(t_SH367309_EEPRom) - 1)))
            {
                sstep += 4;
                BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
            }
            else
            {   
                i = 0;
                //���в�������
                SH367309_WRITE_EEP_ON;
                BSPTaskStart(TASK_ID_SH367309_TASK, 10);       
                sstep++;
            }
            
            slaststep = sstep;
            
			break;  
            
        //�����ڲ�EEP
		case 5:
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_EEP_SCONF1 + i,\
                                                    ((u8 *)&gSHAfeEep) + i);
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;                           
            
            
        //��λ����1
		case 6:
            data = 0xC0;
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,0xEA,&data);
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;   
        
        //��λ����2
		case 7:
            data = 0xA5;
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,0xEA,&data);            
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;                 
        
		case 8:
            //�ر�MOS�ܣ���CADCON
            data = 0x08;
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_CONF,&(data));
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;

        case 9:
            sstep++;
            BSPTaskStart(TASK_ID_SH367309_TASK, 250);            
			break;            
            
        case 10:
            //�ٶ�һ�����ò���
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,(u8 *)&gSHAfeEepRead,\
                                                            sizeof(t_SH367309_EEPRom),SH367309_EEP_SCONF1);
            slaststep = sstep;
            sstep = 0xaa;
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);            
			break;
        
        case 11:
            BITCLR(gSHAFEData.initerr,0);
            errcnt = 0;
            timer = 0;
            
            //�״��ϵ磬���ý�����ʱ
            if(0 == firstflag)
            {
                firstflag = 1;
                
                //����ʵ�����õı���ֵ
                gSHAFECfg.scdval = SH367309GetRealSCDCurr(&gSHAfeEepRead);
                gSHAFECfg.ocd1val = SH367309GetRealOCD1Curr(&gSHAfeEepRead);
                gSHAFECfg.ocd2val = SH367309GetRealOCD2Curr(&gSHAfeEepRead);
                
                gSHAFECfg.ovdval = SH367309CalADtoOVTrip(&gSHAfeEepRead);
                gSHAFECfg.uvdval = SH367309CalADtoUVTrip(&gSHAfeEepRead);   
                
                gSHAFEData.runstatus = e_SH_AFE_Sta_Norm;
                sstep = 0;
                slaststep = 0;
                errcnt = 0;
            }
            else
            {
                sstep++;               
            }
            
            BSPTaskStart(TASK_ID_SH367309_TASK, 2);	                    
            break;
        
        case 12:
			if(timer >= 40)			//�ӳ�2s�������
			{
				timer = 0;
				sstep = 0;
                //���ñ���ֵ
                gSHAFEData.runstatus = e_SH_AFE_Sta_Norm;
			}
			else
			{
				timer++;
			}
            
            BSPTaskStart(TASK_ID_SH367309_TASK, 50);	                
            break;
        
        //IICͨ�Ŵ�����
        case 0xaa:
            if(BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0)
            {
                if(BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0)      
                {
                    result = SH367309_IIC_FAULT_STAT; 
                    sstep = 0xbb; //��������
                    i = 0;
                    SH367309_WRITE_EEP_OFF;
                    BSPTaskStart(TASK_ID_SH367309_TASK, 2);
                }               
                else
                {
                    result = SH367309_IIC_NOFAULT_STAT; 
                    if(5 == slaststep && i < (sizeof(t_SH367309_EEPRom) - 1))  //����EEP
                    {
                        //��д��EEPһ���ֽں�������ʱ35ms
                        i++;
                        sstep = slaststep;
                        BSPTaskStart(TASK_ID_SH367309_TASK, 35);	      
                    }
                    else
                    {
                        i = 0;
                        sstep = slaststep + 1;
                        SH367309_WRITE_EEP_OFF;
                        
                        BSPTaskStart(TASK_ID_SH367309_TASK, 2);
                    }                
                }
            }
            else
            {
                BSPTaskStart(TASK_ID_SH367309_TASK, 2);
            }
            
            break;
            
        //������ִ�����
        case 0xbb:  
            //������ʼ��2����ʧ��������
            if(SH367309_IIC_FAULT_STAT == result)
            {
                errcnt++;
            }
            if(errcnt >= (u16)2)
            {
                BITSET(gSHAFEData.initerr,0);
                errcnt = 0;
                timer = 0;
                sstep = 0;
                slaststep = 0;
                i = 0;
                gSHAFEData.runstatus = e_SH_AFE_Sta_Offline;
                result = SH367309_IIC_NOFAULT_STAT;
            }
            else
            {
                timer = 0;
                sstep = 0;
                slaststep = 0;       
                i = 0;
            }

            BSPTaskStart(TASK_ID_SH367309_TASK, 50);	                 
            break;     

		default:
			sstep = 0;
            slaststep = 0;
			break;            
		}
}

//=============================================================================================
//��������	: void SH367309DisableTask(void)
//�������	: void
//�������	: void
//��̬����	: void
//��    ��	: SH367309�ر�������
//ע    ��	: 50ms����
//=============================================================================================
static void SH367309DisableTask(void)
{
    static u8 timer = 0;
    
    PT_INIT(&gPTSH367309);
    PT_INIT(&gPTSH367309Norm);
    
	SH367309EventFlagClear(SH367309_EVE_CTRL_SLP);
	gSHAFEData.runstatus = e_SH_AFE_Sta_Disable;
    BSPTaskStart(TASK_ID_SH367309_TASK, 50);	  
    
	if(timer++ >= 10)	//�ӳ�500ms
	{
		timer = 0;
		//�����ʼ������
        SH367309BqInitReset();
		gSHAFEData.runstatus = e_SH_AFE_Sta_Bqinit;

	}    
}

//=============================================================================================
//��������	: static void SH367309NormalTask(void)
//�������	: void
//�������	: void
//��̬����	: void
//��    ��	: SH367309����ִ��������
//ע    ��	: 50ms����
//=============================================================================================
static void SH367309NormalTask(void)
{
	static u8 i = 0;
    u8 balance;
    static u8 data;
    
	PT_BEGIN(&gPTSH367309Norm);

    //��AFE�澯���������и澯����
    //if(BSPGPIOGetPin(SH367309_ALERT_PORT, SH367309_ALERT_PIN) != 0)
    if(gSHAFEData.IntFlg)
    {
        gSHAFEData.IntFlg = False;
        
        //��ȡAFE STATUS��־
        gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                    (u8 *)&gSHAFEReg.BSTATUS1.BSTATUS1Byte,1,SH367309_REG_BSTATUS1);  
        
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
        
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }            
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
            
            if(gSHAFEReg.BSTATUS1.BSTATUS1Bit.OCD2)
            {
                gSHAFEData.devsta |= SH367309_CLR_OCD2;
            }
            else
            {
                gSHAFEData.devsta &= ~SH367309_CLR_OCD2;
            }
            
             if(gSHAFEReg.BSTATUS1.BSTATUS1Bit.OCD1)
            {
                gSHAFEData.devsta |= SH367309_CLR_OCD1;
            }
            else
            {
                gSHAFEData.devsta &= ~SH367309_CLR_OCD1;
            }           
        }        
        
        //��ȡAFE BFLAG��־
        gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                    (u8 *)&gSHAFEReg.BFLAG1.BFLAG1Byte,2,SH367309_REG_BFLAG1);
        
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
        
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }            
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
                        
            gSHAFEReg.BFLAG1.BFLAG1Bit.OCD_FLG = 0;
            gSHAFEData.devsta |= gSHAFEReg.BFLAG1.BFLAG1Byte;
        }
        
        DTCheckAFEProtectFlag();		//AFE�豸״̬��־�ж�
        
        if(gSHAFEReg.BFLAG2.BFLAG2Bit.CADC_FLG)
        {
            //��CADC����
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                    (u8 *)gSH367309Readbuff,2,SH367309_REG_ADC2);
            
            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }            
            else
            {
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;

                //��ȡ��������ֵ
                //�������adת��
                gSHAFEData.curr = SH367309CalCADCtoCurr(gSH367309Readbuff[0],gSH367309Readbuff[1]); 
            }                   
        }
        
        if(gSHAFEReg.BFLAG1.BFLAG1Byte 
            || gSHAFEReg.BFLAG2.BFLAG2Bit.UTC_FLG || gSHAFEReg.BFLAG2.BFLAG2Bit.OTC_FLG
            || gSHAFEReg.BFLAG2.BFLAG2Bit.UTD_FLG || gSHAFEReg.BFLAG2.BFLAG2Bit.OTD_FLG)
        {
            data = 0;
            //������־1
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BFLAG1,&data);

            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
                
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }                    
            else
            {
                //������־�ɹ�
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
            }      

            //������־2
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BFLAG2,&data);

            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
                
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }                    
            else
            {
                //������־�ɹ�
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
            }         
        }
        
        //���ڸ�λ���߻��ѱ�־
        if(gSHAFEReg.BFLAG2.BFLAG2Bit.WAKE_FLG || gSHAFEReg.BFLAG2.BFLAG2Bit.RST_FLG)
        {
            data = 0;
            //������־1
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BFLAG1,&data);

            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
                
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }                    
            else
            {
                //������־�ɹ�
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
            }      

            //������־2
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BFLAG2,&data);

            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
                
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }                    
            else
            {
                //������־�ɹ�
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
            }                
            //��CADC��ͬʱ�ر�IDLE��SLEEPģʽ
            gSHAFEReg.CONF.CONFByte |= 0x08;
            gSHAFEReg.CONF.CONFByte = (gSHAFEReg.CONF.CONFByte&0xF8) | 0x08;
            
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_CONF,&gSHAFEReg.CONF.CONFByte);

            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
                
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }                    
            else
            {
                //������־�ɹ�
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
            }                      
        }
    }

    //����
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_SMP)
    {
        gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,(u8 *)&gSHAFEReg,sizeof(gSHAFEReg),SH367309_REG_CONF);
        
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
        
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }            
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;         
            
            if(gSHAFEReg.BSTATUS3.BSTATUS3Bit.CHG_FET)
            {
                BITSET(gSHAFEData.mosstatus,0);
            }
            else
            {
                BITCLR(gSHAFEData.mosstatus,0);
            }

            if(gSHAFEReg.BSTATUS3.BSTATUS3Bit.DSG_FET)
            {
                BITSET(gSHAFEData.mosstatus,1);
            }
            else
            {
                BITCLR(gSHAFEData.mosstatus,1);
            }
            //��ȡAFE STATUS��־
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                        (u8 *)&gSHAFEReg.BSTATUS1.BSTATUS1Byte,1,SH367309_REG_BSTATUS1);  
            
            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }            
            else
            {
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
            
                if(gSHAFEReg.BSTATUS1.BSTATUS1Bit.OCD2)
                {
                    gSHAFEData.devsta |= SH367309_CLR_OCD2;
                }
                else
                {
                    gSHAFEData.devsta &= ~SH367309_CLR_OCD2;
                }
                
                 if(gSHAFEReg.BSTATUS1.BSTATUS1Bit.OCD1)
                {
                    gSHAFEData.devsta |= SH367309_CLR_OCD1;
                }
                else
                {
                    gSHAFEData.devsta &= ~SH367309_CLR_OCD1;
                }           
            }        
            
            //��ȡAFE BFLAG��־
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                        (u8 *)&gSHAFEReg.BFLAG1.BFLAG1Byte,2,SH367309_REG_BFLAG1);
            
            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }            
            else
            {
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;
                
                gSHAFEReg.BFLAG1.BFLAG1Bit.OCD_FLG = 0;
                gSHAFEData.devsta |= gSHAFEReg.BFLAG1.BFLAG1Byte;
            }
            
            DTCheckAFEProtectFlag();		//AFE�豸״̬��־�ж�            
            gSHAFEData.MosUptFlg = True;
            
            //��ȡ�����¶�ֵ
            for(i = 0;i < SH367309_TEMP_NUM;i++)
            {
                //�����¶�adת��
                gSHAFEData.celltemp[i] = SH367309CalADtoTemp(*(((u8 *)&gSHAFEReg.Temp1H) + 2 * i),\
                                                             *(((u8 *)&gSHAFEReg.Temp1L) + 2 * i) );                
            }
            
            //��ȡ�����ѹֵ
            for(i = 0;i < SH367309_VOLT_NUM;i++)
            {
                //�����ѹadת��
                gSHAFEData.cellvolt[i] = SH367309CalADtoVolt(*(((u8 *)&gSHAFEReg.Cell1H) + 2 * i),\
                                                             *(((u8 *)&gSHAFEReg.Cell1H) + 1 + 2 * i) );
            }
            if(True == gModVoltTestCmd) //�ڶ��ڵ����ѹ���Ĳ���
            {
                gSHAFEData.cellvolt[1] = ((u32)gSHAFEData.cellvolt[1] * gConfig.calpara.b16gain) / 2000;
            }                
            //ˢ������
            SH367309RefreshDataToBatInfo();
            DataDealSetBMSDataRdy(e_AFERdy); 
            //
            DataDealSetBMSDataRdy(e_FuelRdy);  
//            BSPTaskStart(TASK_ID_SAMPLE_TASK, 5);
//            _UN_NB_Printf("ADC start %d \r\n",HAL_GetTick());             
        }

        SH367309EventFlagClear(SH367309_EVE_CTRL_SMP);
    }

    //����
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_BAL)
    {
        balance = (u8)(gSHAFECtrl.balctrl >> 8);
        SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BALANCEH,(u8 *)(&balance));
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
    
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }    
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        
        balance = (u8)(gSHAFECtrl.balctrl);
        SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_BALANCEL,(u8 *)(&balance));
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
    
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }    
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        
        SH367309EventFlagClear(SH367309_EVE_CTRL_BAL);
    }

    //˯��
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_SLP)
    {

        SH367309_SHIP_ON;   /* ����ģʽ���� */
        
        gSHAFEData.runstatus = e_SH_AFE_Sta_Disable;
        SH367309EventFlagClear(SH367309_EVE_CTRL_SLP);
    }

    //�����궨
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_CAL)
    {
        if(((0 != gSHAFEData.curr) && (0 != gSHAFECfg.calcurr)))
        {
            s16 ttmp;
           
            //��CADC����
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                    (u8 *)&gSHAFEReg.CADCCurrH,2,SH367309_REG_ADC2);            
            
            gSHAFEData.curradval = (s32)(s16)((((u16)gSHAFEReg.CADCCurrH)<<8)|(gSHAFEReg.CADCCurrL));
            
            ttmp = SH367309CalCurrentRes(((s32)gSHAFECfg.calcurr)*10);

            if((ttmp >= SH367309_SAMPLE_RES_CAL_MIN)&&(ttmp <= SH367309_SAMPLE_RES_CAL_MAX))//�жϼ���ֵ
            {
                //��ֵ��gConfig��gConfigBuffһ�����޷�����
                gConfigBuff.calpara.sampleres = ttmp;
                gSHAFECfg.calres = ttmp;
                gStorageCTRL |= FLASH_WR_CFG_MASK;	//�洢
                //����OCD SCD�ļĴ���ֵ
                //OCD1 OCD2 SC OCC ��ŵ��������·��������
                SH367309OCConfig(0 - gConfig.dcurr.matth[1],0-gConfig.dcurr.matth[2],0 - gConfig.dcurr.matth[3],\
                                 SH367309_CFG_OCC); //����������Ϊ20A
            }
        }
        /* ��ƮУ׼ */
        else if(0 == gSHAFECfg.calcurr)
        {
            //��CADC����
            gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,
                                                    (u8 *)&gSHAFEReg.CADCCurrH,2,SH367309_REG_ADC2);            
            
            gSHAFEData.curradval = (s32)(s16)((((u16)gSHAFEReg.CADCCurrH)<<8)|(gSHAFEReg.CADCCurrL));
            
            gConfigBuff.calpara.sampleoff = gSHAFEData.curradval;
            gSHAFECfg.Cadcoffset = gSHAFEData.curradval;
            gStorageCTRL |= FLASH_WR_CFG_MASK;	//�洢            
        }
        
        SH367309EventFlagClear(SH367309_EVE_CTRL_CAL);
    }

    //����OV TRIP �� UP TRIP
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_OUVD)
    {
        //д��OV �� UVֵ
        SH367309_WRITE_EEP_ON;
        gSHAFEData.currtime = gTimer1ms;
        
        //��ʱ10ms
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                        && ((gTimer1ms - gSHAFEData.currtime) > 20));
        
        for(i = 0;i < (SH367309_EEP_UVR - SH367309_EEP_OVH + 1);i++)
        {
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL, SH367309_EEP_OVH + i, \
            ((u8 *)(&gSHAfeEep.OVH))+ i);
            
            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }    
            else
            {
                //���Ƴɹ�
                //��ʱ35ms
                PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                                && ((gTimer1ms - gSHAFEData.currtime) > 35));                  
            }
        }

        //�����ʵ�ʵ�ʹ��ֵ
        gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,(u8 *)&gSHAfeEepRead.OVH,\
                                          (SH367309_EEP_UVR - SH367309_EEP_OVH + 1),SH367309_EEP_OVH);

        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }    
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
            gSHAFECfg.ovdval = SH367309CalADtoOVTrip(&gSHAfeEepRead);
            gSHAFECfg.uvdval = SH367309CalADtoUVTrip(&gSHAfeEepRead);
        }
        
        SH367309_WRITE_EEP_OFF;
        
        gSHAFEData.currtime = gTimer1ms;
        //��ʱ10ms
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                        && ((gTimer1ms - gSHAFEData.currtime) > 20));
        
        /* ��λ */
        data = 0xC0;
        gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,0xEA,&data);
 
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }                    
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        
        data = 0xA5;
        gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,0xEA,&data);
 
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        
        gSHAFEData.currtime = gTimer1ms;
        //��ʱ500ms
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                        && ((gTimer1ms - gSHAFEData.currtime) > 500));        
        
        SH367309EventFlagClear(SH367309_EVE_CTRL_OUVD);
    }

    //���ó�ŵ��������
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_OSCD)
    {
        //������дģʽ
        SH367309_WRITE_EEP_ON;
        
        gSHAFEData.currtime = gTimer1ms;
        //��ʱ10ms
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                        && ((gTimer1ms - gSHAFEData.currtime) > 20));        
        
        for(i = 0;i < (SH367309_EEP_OCC - SH367309_EEP_OCD1 + 1);i++)
        {
            gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL, SH367309_EEP_OCD1 + i, \
            ((u8 *)(&gSHAfeEep.OCD1.OCD1Byte))+ i);
            
            PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
            if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
            {
                gSHAFEData.iicerr ++;
            }    
            else
            {
                if(gSHAFEData.iicerr > 0)
                    gSHAFEData.iicerr --;

                //���Ƴɹ�
                gSHAFEData.currtime = gTimer1ms;
                //��ʱ35ms
                PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                                && ((gTimer1ms - gSHAFEData.currtime) > 35));                      
            }
        }

        //�����ʵ�ʵ�ʹ��ֵ				BSPIIC_StateGet( DA213_IIC_CHANNEL )
        gSHAFEData.iicres = SH367309ReadNRegisters(SH367309_IIC_CHANNEL,(u8 *)&gSHAfeEepRead.OCD1.OCD1Byte,\
                                          (SH367309_EEP_OCC - SH367309_EEP_OCD1 + 1),SH367309_EEP_OCD1);

        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }    
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;

            gSHAFECfg.scdval = SH367309GetRealSCDCurr(&gSHAfeEepRead);
            gSHAFECfg.ocd1val = SH367309GetRealOCD1Curr(&gSHAfeEepRead);
            gSHAFECfg.ocd2val = SH367309GetRealOCD2Curr(&gSHAfeEepRead);         
        }
        
        //�ر���дģʽ
        SH367309_WRITE_EEP_OFF;
        
        gSHAFEData.currtime = gTimer1ms;
        //��ʱ10ms
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                        && ((gTimer1ms - gSHAFEData.currtime) > 20));
        
        /* ��λ */
        data = 0xC0;
        gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,0xEA,&data);
 
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }                    
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        
        data = 0xA5;
        gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,0xEA,&data);
 
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }
        else
        {
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        
        gSHAFEData.currtime = gTimer1ms;
        //��ʱ500ms
        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) 
                                        && ((gTimer1ms - gSHAFEData.currtime) > 500));               
        
        SH367309EventFlagClear(SH367309_EVE_CTRL_OSCD);
    }

    //MOS�ܿ���
    if(gSHAFECtrl.eventctrl & SH367309_EVE_CTRL_MOS)
    {
        //д�����MOS��
        //result = SH367309MosCtrl(gSHAFECtrl.mosctrl);
        data = ((gSHAFECtrl.mosctrl & 0x03) << 4) | (gSHAFEReg.CONF.CONFByte & 0xCF);
        gSHAFEData.iicres = SH367309WriteRegister(SH367309_IIC_CHANNEL,SH367309_REG_CONF,\
                                                        &(data));

        PT_WAIT_UNTIL(&gPTSH367309Norm,BSPTaskStart(TASK_ID_SH367309_TASK, 2) && (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) <= 0));
            
        if((gSHAFEData.iicres > 0) || (BSPIIC_StateGet( SH367309_IIC_CHANNEL ) < 0))      
        {
            gSHAFEData.iicerr ++;
        }    
        else
        {
            //���Ƴɹ�
            if(gSHAFEData.iicerr > 0)
                gSHAFEData.iicerr --;
        }
        SH367309EventFlagClear(SH367309_EVE_CTRL_MOS);
    }

	gSHAFEData.faultcnt = 0;	//������ϱ�־
    
	if(gSHAFEData.iicerr >= 5 && gSHAFEData.iicerr < 30)	//����ͨ�Ź���
	{
        //IIC����
        /*
        BSPGPIOUnLockLpIIC();
        BSPIICReInit(eIIC0);
        */
        BSPI2C_DeInit(SH367309_IIC_CHANNEL);
        BSPI2C_Init(SH367309_IIC_CHANNEL);
	}    
	else if(gSHAFEData.iicerr >= 30)	//���ֶ��߹���
	{
		gSHAFEData.runstatus = e_SH_AFE_Sta_Offline;
	}
    
    BSPTaskStart(TASK_ID_SH367309_TASK, 50);	  
    PT_INIT(&gPTSH367309Norm);      
	PT_END(&gPTSH367309Norm);    
}

//=============================================================================================
//��������	: void SH367309OffLineTask(void)
//�������	: void
//�������	: void
//��̬����	: void
//��    ��	: SH367309����/���Ϻ�������
//ע    ��	: 50ms����
//=============================================================================================
static void SH367309OffLineTask(void)
{
	static u8 timer = 0;	//��ʱ����������ʱ��100ms����

    if(gSHAFEData.faultcnt < 100)
    {
    	gSHAFEData.faultcnt ++;//����AFE����
    }

    if(gSHAFEData.faultcnt >= 5)
    {
        SH367309ClearData();
    }

    if(gSHAFEData.iicerr >= 1000)
    {
        gSHAFEData.iicerr = 1000;
    }

    timer++;
	if(SH367309EventFlagGet() & SH367309_EVE_CTRL_SLP)
	{
		gSHAFEData.runstatus = e_SH_AFE_Sta_Disable;
	}

	if(timer >= 10)	//�ӳ�500ms
	{
		timer = 0;
        
        //IIC����
        /*
        BSPGPIOUnLockLpIIC();
        BSPIICReInit(eIIC0);
        */
        BSPI2C_DeInit(SH367309_IIC_CHANNEL);
        BSPI2C_Init(SH367309_IIC_CHANNEL);
		//�����ʼ������
        SH367309BqInitReset();
		gSHAFEData.runstatus = e_SH_AFE_Sta_Bqinit;

	}
    BSPTaskStart(TASK_ID_SH367309_TASK, 50);	         
}

//=============================================================================================
//��������	: void SH367309MainTask(void *p)
//�������	: void
//�������	: void
//��̬����	: void
//��    ��	: SH367309��������
//ע    ��	:
//=============================================================================================
void SH367309MainTask(void *p)
{
	(void)p;

	PT_BEGIN(&gPTSH367309);

	for(;;)
    {
		switch(gSHAFEData.runstatus)
		{
			/*	AFE��ʼ������	*/
			case e_SH_AFE_Sta_Bqinit:
				SH367309ConfigInitTask();
				break;

			/*	AFE��������	*/
			case e_SH_AFE_Sta_Norm:
				SH367309NormalTask();
			break;

			/*	AFE��������	*/
			case e_SH_AFE_Sta_Offline:
				SH367309OffLineTask();
				break;

			/*	AFE�ر�����	*/
			case e_SH_AFE_Sta_Disable:
				SH367309DisableTask();
				break;

			default:
			;
			break;
		}

		PT_NEXT(&gPTSH367309);
	}
	PT_END(&gPTSH367309);
}

/*****************************************end of SH367309.c*****************************************/
