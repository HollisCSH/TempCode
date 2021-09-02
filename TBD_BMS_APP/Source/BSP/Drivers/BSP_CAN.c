/****************************************************************************/
/* 	File    	BSP_CAN.c 			 
 * 	Author		Hollis
 *	Notes		ʹ��BSPʱ���stm32g0xx_hal_conf.h��Ӧ�ĺ�
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_CAN.h"
#include "BSP_GPIO.h"
#include "main.h"
#include "CanComm.h"
//#include "SafeBuf.h"
#include "kfifo.h"

/* ˽�к궨�� ----------------------------------------------------------------*/
#define	_CAN_TX_COMPLETELY_TIMEOUT		(0x3FF)	//can���ͳ�ʱʱ��
/* ˽�б��� ------------------------------------------------------------------*/
FDCAN_HandleTypeDef hfdcan1;
FDCAN_HandleTypeDef hfdcan2;
FDCAN_RxHeaderTypeDef gCanRxData;
uint8_t RxData[8];
FDCAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];
static uint32_t HAL_RCC_FDCAN_CLK_ENABLED=0;			//����ʱ���ظ���ʼ��
static const uint32_t FDCAN_TX_DateLenIndex[] = 		//�������ݳ���ת��
{
	FDCAN_DLC_BYTES_0,FDCAN_DLC_BYTES_1,FDCAN_DLC_BYTES_2,
	FDCAN_DLC_BYTES_3,FDCAN_DLC_BYTES_4,FDCAN_DLC_BYTES_5,
	FDCAN_DLC_BYTES_6,FDCAN_DLC_BYTES_7,FDCAN_DLC_BYTES_8,
	FDCAN_DLC_BYTES_12,FDCAN_DLC_BYTES_16,FDCAN_DLC_BYTES_20,
	FDCAN_DLC_BYTES_24,FDCAN_DLC_BYTES_32,FDCAN_DLC_BYTES_48,FDCAN_DLC_BYTES_64
};


/* ��չ���� ------------------------------------------------------------------*/

#ifdef CANBUS_MODE_JT808_ENABLE
uint8_t can0_receive_flag = 0 ;
extern void JT808_rxDataProc(const uint8_t* pData, int len);
#else
static flexcan_msgbuff_t gCanRxData;
#endif 

/* �������� ------------------------------------------------------------------*/

/*****************************************************
	* ��������: CAN�˿ڡ�ʱ�ӡ��жϳ�ʼ��
	* �������: FDCAN_HandleTypeDef can����
	* �� �� ֵ: ��
	* ˵    �������ຯ���ڵ���HAL_XX_Init ������
******************************************************/
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(hfdcan->Instance==FDCAN1)
	{
		HAL_RCC_FDCAN_CLK_ENABLED++;
		if(HAL_RCC_FDCAN_CLK_ENABLED==1)
		{
			__HAL_RCC_FDCAN_CLK_ENABLE();
		}
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**FDCAN1 GPIO Configuration
		PD0     ------> FDCAN1_RX		PA11
		PD1     ------> FDCAN1_TX		PA12
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF3_FDCAN1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* FDCAN1 interrupt Init */
		HAL_NVIC_SetPriority(TIM16_FDCAN_IT0_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM16_FDCAN_IT0_IRQn);
		HAL_NVIC_SetPriority(TIM17_FDCAN_IT1_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM17_FDCAN_IT1_IRQn);
	}
	else if(hfdcan->Instance==FDCAN2)
	{
		__HAL_RCC_FDCAN_CLK_ENABLE();

		__HAL_RCC_GPIOC_CLK_ENABLE();
		/**FDCAN2 GPIO Configuration
		PC2     ------> FDCAN2_RX
		PC3     ------> FDCAN2_TX
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF3_FDCAN2;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(TIM16_FDCAN_IT0_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM16_FDCAN_IT0_IRQn);
		//    HAL_NVIC_SetPriority(TIM17_FDCAN_IT1_IRQn, 0, 0);
		//    HAL_NVIC_EnableIRQ(TIM17_FDCAN_IT1_IRQn);
	}
}

/*****************************************************
	* ��������: CAN�˿ڡ�ʱ�ӡ��ж� �ض�
	* �������: FDCAN_HandleTypeDef can����
	* �� �� ֵ: ��
	* ˵    �������ຯ���ڵ���HAL_XX_DeInit ������
******************************************************/
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* hfdcan)
{
	if(hfdcan->Instance==FDCAN2)
    {
        __HAL_RCC_FDCAN_CLK_DISABLE();
        /**FDCAN2 GPIO Configuration
        PC2     ------> FDCAN2_RX
        PC3     ------> FDCAN2_TX	*/
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2|GPIO_PIN_3);
    }
}

/*****************************************************
	* ��������: ��ȡcan�ķ��Ϳ���״̬
	* �������: FDCAN_HandleTypeDef can����
	* �� �� ֵ: [0,3],
	* ˵    ������0�ɷ���
******************************************************/
uint32_t GetCanTxFifoFreeLevel(void)
{
	return HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2);
}
/*****************************************************
	* ��������: CAN�˿ڡ�ʱ�ӡ��жϳ�ʼ��
    * �������: can�ṹ��hfdcan | ����֡����txHeader | ֡ID exfid	| ���ݵ�ַpbuff	|���ݳ���len
	* �� �� ֵ: uint8_t 0 Ϊ���ͳɹ�	��0��ʾ����ʧ��
	* ˵    �������ͳ�ʱ������
******************************************************/
static uint8_t HAL_FDCANx_SendMsg( FDCAN_HandleTypeDef *hfdcan , FDCAN_TxHeaderTypeDef *txHeader , 
																uint32_t exfid , unsigned char * pbuff , unsigned short len )
{
	unsigned char sedlen = 0;
    unsigned char psdBuff[256];
    unsigned short DateLen = len;
	unsigned short timeout = _CAN_TX_COMPLETELY_TIMEOUT;

	memcpy(psdBuff,pbuff,len);
	while( HAL_FDCAN_GetTxFifoFreeLevel(hfdcan) == 0 && timeout )	//�ж�FIFO�Ƿ�Ϊ��
	{
		timeout--;
	}
	
	if( !timeout ) return 1; 										//����ʧ�� �޷�����

	while( len )
	{
		sedlen = len > 8 ? 8 : len ;	
		txHeader->DataLength = FDCAN_TX_DateLenIndex[sedlen] ;		//���÷��ͳ���
		txHeader->Identifier = exfid ;								//���÷���ID
		if( HAL_FDCAN_AddMessageToTxFifoQ( hfdcan, txHeader, &psdBuff[DateLen-len] ) == HAL_OK )	//����ʹ��
		{
			timeout = _CAN_TX_COMPLETELY_TIMEOUT;									
		  while ((HAL_FDCAN_GetTxFifoFreeLevel(hfdcan) == 0)&&timeout )	//�ȴ��������
			{
				timeout--;
			}
//		  (hfdcan)->Instance->IR &= FDCAN_IR_TFE;					//���������ɱ�־
			if( !timeout ) return 2; 								//����ʧ�� �涨ʱ�����޷��������
		}
		//psdBuff += sedlen ;										//����������λ ���ȸ���
		len -= sedlen ;
	}
	return 0; 
}

/*****************************************************
	* ��������: CAN1��������
	* �������: void
	* �� �� ֵ: void
	* ˵    ����
******************************************************/
void MX_FDCAN1_Init(void)
{
	FDCAN_FilterTypeDef sFilterConfig;
	HAL_FDCAN_DeInit(&hfdcan1);                                         //�������ǰ������
    hfdcan1.Instance = FDCAN1;
    hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;						//CANʱ�Ӳ���Ƶ
    hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;					    //����CANģʽ  
    hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;								//��ͨģʽ	
    hfdcan1.Init.AutoRetransmission = DISABLE;							//�ر��Զ��ش�����ͳģʽ�¹ر�
    hfdcan1.Init.TransmitPause = DISABLE;								//�رմ�����ͣ
    hfdcan1.Init.ProtocolException = ENABLE;						    //Э���쳣����

    hfdcan1.Init.NominalPrescaler = 4;									//��Ƶϵ��
    hfdcan1.Init.NominalSyncJumpWidth = 1;								//����ͬ����Ծ���
    hfdcan1.Init.NominalTimeSeg1 = 11;									//������֮ǰ��־ʱ���:2~256
    hfdcan1.Init.NominalTimeSeg2 = 4;									//������֮ǰ��־ʱ���:2~128
    //baud = can_clk/DataPrescaler/(DataTimeSeg1+DataTimeSeg2+1)  
    hfdcan1.Init.DataPrescaler = 4;										//���ݶ�Ԥ��Ƶ				
    hfdcan1.Init.DataSyncJumpWidth = 1;								    //����λͬ����Ծ��� DataSyncJumpWidth��С��DataTimeSeg1 DataTimeSeg2����Сֵ
    hfdcan1.Init.DataTimeSeg1 = 11;										//������֮ǰ����ʱ���
    hfdcan1.Init.DataTimeSeg2 = 4;										//������֮������ʱ���

    hfdcan1.Init.StdFiltersNbr = 0;										//��׼��ϢID�˲�������
    hfdcan1.Init.ExtFiltersNbr = 1;										//��չ��ϢID�˲�������
    hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;	            //����FIFO����ģʽ	��ѡ���к�FIFO
    if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)								//��ʼ��
    {
        assert_param(0);                                                //���Բ���
    }
    sFilterConfig.IdType = FDCAN_EXTENDED_ID;							//�˲�������	��չ֡
    sFilterConfig.FilterIndex = 0;										//�˲�������
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;						//�˲�������	��ͳ�˲�
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;		        //������0������FIFO0
    sFilterConfig.FilterID1 = 0x0001;									//32λID
    sFilterConfig.FilterID2 = 0;									    //���FDCAN����Ϊ��ͳģʽ�Ļ���������32λ���� ����ΪĳbitΪ1��ʾ��ʶ���Ӧ��bit�̶� 
    if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)		//�˲�������
    {
        assert_param(0);                  //���Բ���
    }

    if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, //����Զ��֡
                                        FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)	
    {
        assert_param(0);                  //���Բ���
    }
    if (HAL_FDCAN_ActivateNotification(&hfdcan1,FDCAN_IT_GROUP_RX_FIFO0 | FDCAN_IT_GROUP_RX_FIFO1 |
                FDCAN_IT_RX_FIFO0_NEW_MESSAGE| FDCAN_IT_RX_FIFO1_NEW_MESSAGE|FDCAN_IT_BUS_OFF, 0) != HAL_OK)
    {
        assert_param(0);                  //���Բ���
    }

    if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_GROUP_RX_FIFO0 | FDCAN_IT_GROUP_RX_FIFO1|
            FDCAN_IT_RX_FIFO0_NEW_MESSAGE|FDCAN_IT_RX_FIFO1_NEW_MESSAGE,
          FDCAN_TX_BUFFER0 |FDCAN_TX_BUFFER1|FDCAN_TX_BUFFER2 ) != HAL_OK)
    {
        assert_param(0);                  //���Բ���
    }

    TxHeader.Identifier = 0x4321;												//����֡���� 32λID
    TxHeader.IdType = FDCAN_EXTENDED_ID;										//��չID
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;									//����֡
    //TxHeader.DataLength = FDCAN_DLC_BYTES_8;								    //���ݳ��� �ǹ̶� ����ʱ������
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;										//�������л�
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;										//��ͳCANģʽ  
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;					        //�޷����¼�
    TxHeader.MessageMarker = 0;													//����ʶ��Tx��Ϣ״̬��ȡֵ��ΧΪ0 ~ 0xFF֮�������
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)									//����CANģ��
    {
        assert_param(0);                  //���Բ���
    }
}

/*****************************************************
	* ��������: CAN�˿ڡ�ʱ�ӡ��жϳ�ʼ��
	* �������: can�ṹ��hfdcan | ����֡����txHeader | ֡ID exfid	| ���ݵ�ַpbuff	|���ݳ���len
	* �� �� ֵ: uint8_t 0 Ϊ���ͳɹ�	��0��ʾ����ʧ��
	* ˵    �������ͳ�ʱ������    ������250k
******************************************************/
void MX_FDCAN2_Init(void)
{
	FDCAN_FilterTypeDef sFilterConfig;
//	HAL_FDCAN_DeInit(&hfdcan2);                                         //�������ǰ������
    hfdcan2.Instance = FDCAN2;
    hfdcan2.Init.ClockDivider = FDCAN_CLOCK_DIV1;						//CANʱ�Ӳ���Ƶ
    hfdcan2.Init.FrameFormat = FDCAN_FRAME_CLASSIC;					    //����CANģʽ  
    hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;								//��ͨģʽ	
    hfdcan2.Init.AutoRetransmission = DISABLE;							//�ر��Զ��ش�����ͳģʽ�¹ر�
    hfdcan2.Init.TransmitPause = DISABLE;							    //�رմ�����ͣ
    hfdcan2.Init.ProtocolException = ENABLE;							//Э���쳣����
    
    hfdcan2.Init.NominalPrescaler = CAN_DATA_PRESCALER;			        //��Ƶϵ��  
    hfdcan2.Init.NominalSyncJumpWidth = 1;								//����ͬ����Ծ���
    hfdcan2.Init.NominalTimeSeg1 = 11;									//������֮ǰ��־ʱ���:2~256
    hfdcan2.Init.NominalTimeSeg2 = 4;									//������֮ǰ��־ʱ���:2~128
    //baud = can_clk/DataPrescaler/(DataTimeSeg1+DataTimeSeg2+1)  
    hfdcan2.Init.DataPrescaler = CAN_DATA_PRESCALER;		            //���ݶ�Ԥ��Ƶ				
    hfdcan2.Init.DataSyncJumpWidth = 1;								    //����λͬ����Ծ��� DataSyncJumpWidth��С��DataTimeSeg1 DataTimeSeg2����Сֵ
    hfdcan2.Init.DataTimeSeg1 = 11;										//������֮ǰ����ʱ���
    hfdcan2.Init.DataTimeSeg2 = 4;										//������֮������ʱ���

    hfdcan2.Init.StdFiltersNbr = 0;										//��׼��ϢID�˲�������
    hfdcan2.Init.ExtFiltersNbr = 1;										//��չ��ϢID�˲�������
    hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;	            //����FIFO����ģʽ	��ѡ���к�FIFO
    if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK)								//��ʼ��
    {
		assert_param(0);                                                //���� ������ʾ
    }
    sFilterConfig.IdType = FDCAN_EXTENDED_ID;							//�˲�������	��չ֡
    sFilterConfig.FilterIndex = 0;										//�˲�������
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;						//�˲�������	����֡
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;		        //������0������FIFO0
    sFilterConfig.FilterID1 = 0x0020;								    //32λID 
    sFilterConfig.FilterID2 = 0x002F;									//���FDCAN����Ϊ��ͳģʽ�Ļ���������32λ���� ����ΪĳbitΪ1��ʾ��ʶ���Ӧ��bit�̶� 
    if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig) != HAL_OK)		//�˲�������
    {
		assert_param(0);                  								//���� ������ʾ
    }

    if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_REJECT, FDCAN_REJECT, //����Զ��֡
                                        FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)	
    {
		assert_param(0);                  								//���� ������ʾ
    }

    /* ʹ��FDCAN�����ж� */
    HAL_FDCAN_ActivateNotification(&hfdcan2,FDCAN_IT_RX_FIFO1_NEW_MESSAGE | FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0);

    /* Prepare Tx Header */
    TxHeader.Identifier = 0x4321;										//32λID
    TxHeader.IdType = FDCAN_EXTENDED_ID;								//��չID
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;							//����֡
    //TxHeader.DataLength = FDCAN_DLC_BYTES_8;							//���ݳ��� �ǹ̶� ����ʱ������
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;								//�������л�
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;								//��ͳCANģʽ  
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;					//�޷����¼�
    TxHeader.MessageMarker = 0;											//����ʶ��Tx��Ϣ״̬��ȡֵ��ΧΪ0 ~ 0xFF֮�������
    if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK)							//����CANģ��
    {
        assert_param(0);                                                //���� ������ʾ
    }
}

/*****************************************************
	* ��������: CAN�豸 ��Դ ��ʼ��
	* �������: void
	* �� �� ֵ: void
	* ˵    ����
******************************************************/
void BSPCan_Init(void)
{
    PWR_14V_5V_ENABLE();
    CAN_3V_ENABLE();
    MX_FDCAN2_Init();
}

/*****************************************************
	* ��������: CAN�豸 ��Դ �ض�
	* �������: void
	* �� �� ֵ: void
	* ˵    ����
******************************************************/
void BSPCan_DeInit(void)
{
    HAL_NVIC_DisableIRQ(TIM16_FDCAN_IT0_IRQn);
	HAL_FDCAN_DeInit(&hfdcan2);
    CAN_3V_DISABLE();
}

/*****************************************************
	* ��������: CAN�豸 ��Դ ��λ
	* �������: void
	* �� �� ֵ: void
	* ˵    ����
******************************************************/
void BSPCan_Reset(void)
{
	BSPCan_DeInit();
	BSPCan_Init();
}

//=============================================================================================
//��������	: u32 FlexCanChangeRx3Canid(u8 id)
//��������	:
//�������	: ���ĺ��CAN ID
//��̬����	:
//��	��	: ���ӽ���ID
//ע	��	:
//=============================================================================================
uint32_t FlexCanChangeRx3Canid(uint8_t id)
{
    uint32_t canid;
    canid = (VARI_RX_ID & 0x00FF) | (((uint16_t)id) << 8);
    return canid;
}

/*****************************************************
	* ��������: CAN2���ݷ���
    * �������: ֡ID exfid	| ���ݵ�ַpbuff	|���ݳ���len
	* �� �� ֵ: uint8_t 0 Ϊ���ͳɹ�	��0��ʾ����ʧ��
	* ˵    �������ͳ�ʱ������
******************************************************/
uint8_t SendCANData(unsigned int exfid, unsigned char * pbuff , unsigned int len )
{
	return HAL_FDCANx_SendMsg( &hfdcan2 , &TxHeader , exfid , pbuff , len );
}
/*****************************************************
	* ��������: CAN2���ջص�����
    * �������: ֡ID exfid	| ���ݵ�ַpbuff	|���ݳ���len
	* �� �� ֵ: ��
    * ˵    �����ж������Դ��Ҫ����FIFO0���ݷ��ͱ仯�����ص����ڻص��н�FIFO0һ�ζ���
******************************************************/
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
		Loop:
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &gCanRxData, RxData) != HAL_OK)
		{
		//assert_param(0);                  //���� ������ʾ
		}
		gCanRxData.DataLength = gCanRxData.DataLength>>16;

		if ( gCanRxData.Identifier == SOLID_RX_ID  )		  //�̶�IDDEST_SOLID_ADDRESS��0x1130 ����ԴΪ��λ�������
		{
			//			SafeBuf_Write(&gCanDealMsg.gCanRecSafeBuf_ChangeId, gCanRxData.data,gCanRxData.dataLen);
			//			SafeBuf_Write(&gCanDealMsg.gCanRecSafeBuf_SolidId, RxData,gCanRxData.DataLength);
			can0_receive_flag = 1 ;
			kfifo_in(&gCanDealMsg.gCanRecSafeBuf_SolidId, RxData,gCanRxData.DataLength);       
		}
		else if( (gCanRxData.Identifier & 0x00FF) == 0x0030  )	//�ɱ�ID��0x--30 �����豸 �ɷ����λ��ַ
			{
			#ifdef CANBUS_MODE_JT808_ENABLE
			can0_receive_flag = 1 ;
			#endif
			kfifo_in(&gCanDealMsg.gCanRecSafeBuf_ChangeId, RxData,gCanRxData.DataLength);  
		}
		#ifdef CANBUS_MODE_JT808_ENABLE
		else if ( gCanRxData.Identifier == EX_MODU_RX_ID )      //�̶�IDDEST_SOLID_ADDRESS��0x1120 ����ԴΪ����ģ��
		{
			can0_receive_flag = 1 ;
			JT808_rxDataProc(RxData , gCanRxData.DataLength);
		}
		#endif 

		if(gCanRxData.DataLength != 0)							//gCanRxData.DataLength���ᷢ���仯   ���Ƶ�ʱ�����ݻ�仯
		goto Loop;
		HAL_FDCAN_ActivateNotification(hfdcan,FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0);
	}
}
/*****************************************************
	* ��������: CAN1���ջص�����
    * �������: ֡ID exfid	| ���ݵ�ַpbuff	|���ݳ���len
	* �� �� ֵ: ��
    * ˵    �����ж������Դ��Ҫ����FIFO0���ݷ��ͱ仯�����ص����ڻص��н�FIFO0һ�ζ���
******************************************************/
//void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
//{
//  if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET)
//  {
//    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &gCanRxData, RxData) != HAL_OK)
//    {
//    Error_Handler();
//    }

//    if ((gCanRxData.IdType == FDCAN_EXTENDED_ID) 
//			)		
//    {
////			_UN_NB_Printf("rec can\n");
////			SafeBuf_Write(&gCanDealMsg.gCanRecSafeBuf_ChangeId, gCanRxData.data,gCanRxData.dataLen);
//			gCanRxData.DataLength = gCanRxData.DataLength>>16;
////			SafeBuf_Write(&gCanDealMsg.gCanRecSafeBuf_SolidId, RxData,gCanRxData.DataLength);
//      kfifo_in(&gCanDealMsg.gCanRecSafeBuf_SolidId, RxData,gCanRxData.DataLength);
//			if(gCanRxData.Identifier == 0x0001)	
//			{
//				MX_CAN_TEST();
//			}
//      HAL_FDCAN_ActivateNotification(hfdcan,FDCAN_IT_RX_FIFO1_NEW_MESSAGE,0);
//    }
//  }
//}


