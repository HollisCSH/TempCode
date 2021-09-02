/****************************************************************************/
/* 	File    	RTC.c 			 
 * 	Author		Hollis
 *	Notes		�������ԭ���� PCF85063.c �Ĺ���
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-07-04		first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
//=============================================================================================
//����ͷ�ļ�
//=============================================================================================
#include "RTC.h"	//�������PCF85063.h
#include "BSP_RTC.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "DataDeal.h"
#include "pt.h"

//=============================================================================================
//ȫ�ֱ���
//=============================================================================================
u8	gPCF85063Readbuff[PCF85063_READBUFF_LEN] = {0};  	//PCF85063���ջ������飬���ʱ��
u8	gPCF85063Writebuff[PCF85063_WRITEBUFF_LEN] = {0};  	//PCF85063���ͻ������飬���д��ʱ��
u8  gRTCWriteBuff[PCF85063_READBUFF_LEN] = {0};         //IICд������buff

t_PCF85063_DATE gRdTimeAndDate;  						//��ʱ�����ڽṹ��
t_PCF85063_DATE gWrTimeAndDate;  						//дʱ�����ڽṹ��
t_PCF85063_COND gRTCCond;                               //RTCоƬ����/״̬�ṹ��
pt gPTPCF85063;					                        //PCF85063����pt�߳̿��Ʊ���

t_PCF85063_DATE DefaultInfo=	//��ʼֵ:2099/06/01/�ܶ�/12��00��00    ƫ��ֵ����ʾδ��ʼ��
{
    .year 			= 0x99,	    //BCD�� 
    .month 			= RTC_MONTH_JUNE,
    .day 			= 1,
    .hour 			= 0x12,
    .minute 		= 0,
    .second 		= 0,
};

//=============================================================================================
//��������	: void PCF85063ClearCtrl(u8 ctrl)
//�������	: ctrl:����λ����ϸ�����ͷ�ļ��궨��
//�������	: void
//��̬����	: void
//��	��	: PCF85063���ÿ��Ʊ�־λ����
//ע	��	:
//=============================================================================================
void PCF85063SetCtrl(u8 ctrl)
{
    gRTCCond.ctrl |= ctrl;
}

//=============================================================================================
//��������	: void PCF85063ClearCtrl(u8 ctrl)
//�������	: ctrl:����λ����ϸ�����ͷ�ļ��궨��
//�������	: void
//��̬����	: void
//��	��	: PCF85063������Ʊ�־λ����
//ע	��	:
//=============================================================================================
void PCF85063ClearCtrl(u8 ctrl)
{
    gRTCCond.ctrl &= (~ctrl);
}


//=============================================================================================
//��������	: u8 PCF85063Start(void)
//�������	: void
//�������	: �Ƿ����гɹ���0�����гɹ���1������ʧ��
//��̬����	: void
//��    ��	: PCF85063 RTC��������
//ע    ��	:
//=============================================================================================
//u8 PCF85063Start(void)
//{
//	u8 temp;
//	u8 errstat = 0;

//	temp = 0x01;		//ѡ��12.5pf����

////	errstat = PCF85063WriteRegister(PCF85063_IIC_CHANNEL,PCF85063_CONTROL1_ADDR ,temp);

//	return errstat;
//}

//=============================================================================================
//��������	: u8 PCF85063Stop(void)
//�������	: void
//�������	: �Ƿ�ֹͣ�ɹ���0��ֹͣ�ɹ���1��ֹͣʧ��
//��̬����	: void
//��    ��	: PCF85063 RTCֹͣ����
//ע    ��	:
//=============================================================================================
//u8 PCF85063Stop(void)
//{
//	u8 temp;
//	u8 errstat = 0;

//	temp = 0x21;		//ѡ��12.5pf����
//	
////	errstat = PCF85063WriteRegister(PCF85063_IIC_CHANNEL,PCF85063_CONTROL1_ADDR ,temp);

//	return errstat;
//}

//=============================================================================================
//��������	: u8 PCF85063SetDate(t_PCF85063_DATE *tp)
//�������	: tp:д���ʱ������ָ��
//�������	: �Ƿ�д��ɹ���0��д��ɹ���1��д��ʧ��
//��̬����	: void
//��    ��	: PCF85063д��ʱ�亯��
//ע    ��	:
//=============================================================================================
u8 PCF85063SetDate(t_PCF85063_DATE *tp)
{
  // u8 year;
   gPCF85063Writebuff[0] = tp->second;
   gPCF85063Writebuff[1] = tp->minute;
   gPCF85063Writebuff[2] = tp->hour;
   gPCF85063Writebuff[3] = tp->day;
   gPCF85063Writebuff[4] = 0;//tp->week;
   gPCF85063Writebuff[5] = tp->month;
   gPCF85063Writebuff[6] = tp->year;
    
   return TRUE;
}

//=============================================================================================
//��������	: u8 PCF85063ReadDate(t_PCF85063_DATE *tp)
//�������	: tp:��ȡ��ʱ������ָ��
//�������	: �Ƿ��ȡ�ɹ���0����ȡ�ɹ���1����ȡʧ��
//��̬����	: void
//��    ��	: PCF85063��ȡʱ�亯��
//ע    ��	:
//=============================================================================================
u8 PCF85063ReadDate(t_PCF85063_DATE *tp)
{
    //���Զ������ĵ�ַ�Ĵ���������Ĵ�����ʼ��ȡ
    //   if (PCF85063_IIC_FAULT_STAT ==
    //       PCF85063ReadNRegisters(PCF85063_IIC_CHANNEL, gPCF85063Readbuff, 7,PCF85063_SECOND_ADDR))
    //   {
    //      return FALSE;
    //   }
    //
    //   tp->second     = PCF85063_BCD_to_HEC((gPCF85063Readbuff[0] & 0x7f));
    //   tp->minute     = PCF85063_BCD_to_HEC((gPCF85063Readbuff[1] & 0x7f));
    //   tp->hour       = PCF85063_BCD_to_HEC((gPCF85063Readbuff[2] & 0x3f));
    //   tp->day        = PCF85063_BCD_to_HEC((gPCF85063Readbuff[3] & 0x3f));
    //   tp->week       = PCF85063_BCD_to_HEC((gPCF85063Readbuff[4] & 0x07));
    //   tp->month 	  = PCF85063_BCD_to_HEC((gPCF85063Readbuff[5] & 0x1f));
    //   //tp->year       = PCF85063_BCD_to_HEC(gPCF85063Readbuff[6]) + 2000;
    //   tp->year       = PCF85063_BCD_to_HEC(gPCF85063Readbuff[6]);

   tp->second     = (gPCF85063Readbuff[0] & 0x7f);
   tp->minute     = (gPCF85063Readbuff[1] & 0x7f);
   tp->hour       = (gPCF85063Readbuff[2] & 0x3f);
   tp->day        = (gPCF85063Readbuff[3] & 0x3f);
   tp->month 	  	= (gPCF85063Readbuff[5] & 0x1f);
   tp->year       = (gPCF85063Readbuff[6]);

   return TRUE;
}

//=============================================================================================
//��������	: void PCF85063MainTask(void *p)
//�������	: void
//�������	: void
//��̬����	: void
//��	��	: PCF85063��ȡ����ʱ�亯��
//ע	��	:
//=============================================================================================
void PCF85063MainTask(void *p)
{
	(void)p;
//    static u8 i = 0;
    static u8 FirstFlg = True;
    
	PT_BEGIN(&gPTPCF85063);

    if(True == FirstFlg)
    {
        FirstFlg = False;
        /*����ʱ�������*/
    }

	for(;;)
    {
        //��ȡʱ��
        if(gRTCCond.ctrl & PCF85063_START_READ)
        {
            if(RTC_GetInfo(&gRdTimeAndDate) != 0)
            {
                gRTCCond.err ++;
            }
			//�ж�ʱ���Ƿ���Ч	//����δ�����仯��ΪδУ��
			if(	((gRdTimeAndDate.year  | DefaultInfo.year ) == DefaultInfo.year) && 
				((gRdTimeAndDate.month | DefaultInfo.month) == DefaultInfo.month)&&
				((gRdTimeAndDate.day   | DefaultInfo.day  ) == DefaultInfo.day) )
			{
				gRTCCond.calflag = 0;
			}
			else
			{
				gRTCCond.calflag = 1;
			}
			PCF85063ClearCtrl(PCF85063_START_READ); 
			DataDealSetBMSDataRdy(e_RTCRdy);  
        }

        //У׼ʱ��
        if(gRTCCond.ctrl & PCF85063_START_WRITE)
        {
            if((0 != gWrTimeAndDate.year) && (0 != gWrTimeAndDate.month) && (0 != gWrTimeAndDate.day))
            {
				if(RTC_SetInfo(&gWrTimeAndDate) != 0)
				{
						gRTCCond.err ++;
				}					
            }
            PCF85063ClearCtrl(PCF85063_START_WRITE);      
        }

		BSPTaskStart(TASK_ID_PCF85063_TASK, 200);	//����ʱ200ms����
		PT_NEXT(&gPTPCF85063);   
	}
    PT_END(&gPTPCF85063);
}

/*****************************************end of PCF85063.c*****************************************/
