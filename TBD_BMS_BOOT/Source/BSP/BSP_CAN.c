/****************************************************************************/
/* 	File    	BSP_CAN.c 			 
 * 	Author		Hollis
 *	Notes		使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/

//#include "stm32g0xx_hal.h"
#include "BSP_CAN.h"
#include "main.h"
#include "CanComm.h"
#include "SafeBuf.h"

/* 私有宏定义 ----------------------------------------------------------------*/
#define	_CAN_TX_COMPLETELY_TIMEOUT		(0x7FF)	        //can发送超时时间
/* 私有变量 ------------------------------------------------------------------*/
FDCAN_HandleTypeDef hfdcan1;
FDCAN_HandleTypeDef hfdcan2;
FDCAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
FDCAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];
static uint32_t HAL_RCC_FDCAN_CLK_ENABLED=0;			//放置时钟重复初始化
static const uint32_t FDCAN_TX_DateLenIndex[] = 	    //发送数据长度转换
{
	FDCAN_DLC_BYTES_0,FDCAN_DLC_BYTES_1,FDCAN_DLC_BYTES_2,
	FDCAN_DLC_BYTES_3,FDCAN_DLC_BYTES_4,FDCAN_DLC_BYTES_5,
	FDCAN_DLC_BYTES_6,FDCAN_DLC_BYTES_7,FDCAN_DLC_BYTES_8,
	FDCAN_DLC_BYTES_12,FDCAN_DLC_BYTES_16,FDCAN_DLC_BYTES_20,
	FDCAN_DLC_BYTES_24,FDCAN_DLC_BYTES_32,FDCAN_DLC_BYTES_48,FDCAN_DLC_BYTES_64
};


/* 扩展变量 ------------------------------------------------------------------*/

/* 函数声明 ------------------------------------------------------------------*/

/*****************************************************
	* 函数功能: CAN端口、时钟、中断初始化
    * 输入参数: FDCAN_HandleTypeDef can类型
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(hfdcan->Instance==FDCAN1)
    {
    HAL_RCC_FDCAN_CLK_ENABLED++;
    if(HAL_RCC_FDCAN_CLK_ENABLED==1){
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
        //    HAL_NVIC_SetPriority(TIM16_FDCAN_IT0_IRQn, 0, 0);
        //    HAL_NVIC_EnableIRQ(TIM16_FDCAN_IT0_IRQn);
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
	* 函数功能: CAN端口、时钟、中断 关断
	* 输入参数: FDCAN_HandleTypeDef can类型
	* 返 回 值: 无
	* 说    明：该类函数在调用HAL_XX_DeInit 会内联
******************************************************/
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* hfdcan)
{
	if(hfdcan->Instance==FDCAN2)
    {
        __HAL_RCC_FDCAN_CLK_DISABLE();
        /**FDCAN2 GPIO Configuration
        PC2     ------> FDCAN2_RX
        PC3     ------> FDCAN2_TX
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2|GPIO_PIN_3);
    }
}
/*****************************************************
	* 函数功能: CAN设备  关断
	* 输入参数: void
	* 返 回 值: void
	* 说    明：
******************************************************/
void MX_FDCAN2_DeInit(void)
{
	HAL_FDCAN_DeInit(&hfdcan2);
}

/*****************************************************
	* 函数功能: CAN端口、时钟、中断初始化
  * 输入参数: can结构体hfdcan | 数据帧参数txHeader | 帧ID exfid	| 数据地址pbuff	|数据长度len
	* 返 回 值: uint8_t 0 为发送成功	非0表示发送失败
	* 说    明：发送超时需配置
******************************************************/
static uint8_t HAL_FDCANx_SendMsg( FDCAN_HandleTypeDef *hfdcan , FDCAN_TxHeaderTypeDef *txHeader , 
																uint32_t exfid ,unsigned char * pbuff , unsigned short len )
{
	unsigned char sedlen = 0;
	unsigned char *psdBuff = pbuff ;
	unsigned short timeout = _CAN_TX_COMPLETELY_TIMEOUT;	
	while( HAL_FDCAN_GetTxFifoFreeLevel(hfdcan) == 0 && timeout )		//判断FIFO是否为空
	{
		timeout--;
	}
	if( !timeout ) return 1; 																	//发送失败 无法发送

	while( len )
	{
		sedlen = len > 8 ? 8 : len ;	
		txHeader->DataLength = FDCAN_TX_DateLenIndex[sedlen] ;	//配置发送长度
		txHeader->Identifier = exfid ;													//配置发送ID
		if( HAL_FDCAN_AddMessageToTxFifoQ( hfdcan, txHeader, psdBuff ) == HAL_OK )	//发送使能
		{
			timeout = _CAN_TX_COMPLETELY_TIMEOUT;									
		  while (((hfdcan)->Instance->IR & FDCAN_IR_TFE) != FDCAN_IR_TFE&&timeout )	//等待发送完成
		  //while ((HAL_FDCAN_GetTxFifoFreeLevel(hfdcan) == 0)&&timeout )	//等待发送完成
			{
				timeout--;
			}
            (hfdcan)->Instance->IR &= FDCAN_IR_TFE;								//清除发送完成标志
			if( !timeout ) return 2; 															//发送失败 规定时间内无法发送完成
		}
		psdBuff += sedlen ;																			//发送内容移位 长度更新
		len -= sedlen ;
	}
	return 0; 
}
/*****************************************************
	* 函数功能: CAN1经典配置
    * 输入参数: void
	* 返 回 值: void
	* 说    明：
******************************************************/
void MX_FDCAN1_Init(void)
{
	FDCAN_FilterTypeDef sFilterConfig;
	HAL_FDCAN_DeInit(&hfdcan1);                             //先清除以前的设置
    hfdcan1.Instance = FDCAN1;
    hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;			//CAN时钟不分频
    hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;			//经典CAN模式  
    hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;					//普通模式	
    hfdcan1.Init.AutoRetransmission = DISABLE;				//关闭自动重传，传统模式下关闭
    hfdcan1.Init.TransmitPause = DISABLE;					//关闭传输暂停
    hfdcan1.Init.ProtocolException = ENABLE;				//协议异常处理

    hfdcan1.Init.NominalPrescaler = 4;						//分频系数
    hfdcan1.Init.NominalSyncJumpWidth = 1;					//重新同步跳跃宽度
    hfdcan1.Init.NominalTimeSeg1 = 11;						//采样点之前标志时间段:2~256
    hfdcan1.Init.NominalTimeSeg2 = 4;						//采样点之前标志时间段:2~128
    //baud = can_clk/DataPrescaler/(DataTimeSeg1+DataTimeSeg2+1)  
    hfdcan1.Init.DataPrescaler = 4;							//数据段预分频				
    hfdcan1.Init.DataSyncJumpWidth = 1;						//数据位同步跳跃宽度 DataSyncJumpWidth需小于DataTimeSeg1 DataTimeSeg2的最小值
    hfdcan1.Init.DataTimeSeg1 = 11;							//采样点之前数据时间段
    hfdcan1.Init.DataTimeSeg2 = 4;							//采样点之后数据时间段

    hfdcan1.Init.StdFiltersNbr = 0;							//标准信息ID滤波器数量
    hfdcan1.Init.ExtFiltersNbr = 1;							//扩展信息ID滤波器数量
    hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;	//发送FIFO序列模式	可选队列和FIFO
    if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)					//初始化
    {
        assert_param(0);
    }
    sFilterConfig.IdType = FDCAN_EXTENDED_ID;				//滤波器配置	拓展帧
    sFilterConfig.FilterIndex = 0;							//滤波器索引
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;			//滤波器类型	传统滤波
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;	//过滤器0关联到FIFO0
    sFilterConfig.FilterID1 = 0x0001;						//32位ID
    sFilterConfig.FilterID2 = 0;							//如果FDCAN配置为传统模式的话，这里是32位掩码 掩码为某bit为1表示标识码对应的bit固定 
    if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)		//滤波器配置
    {
        assert_param(0);
    }

    if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, //拒收远程帧
                                        FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)	
    {
        assert_param(0);
    }
    if (HAL_FDCAN_ActivateNotification(&hfdcan1,FDCAN_IT_GROUP_RX_FIFO0 | FDCAN_IT_GROUP_RX_FIFO1 |
                FDCAN_IT_RX_FIFO0_NEW_MESSAGE| FDCAN_IT_RX_FIFO1_NEW_MESSAGE|FDCAN_IT_BUS_OFF, 0) != HAL_OK)
    {
        assert_param(0);
    }

    if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_GROUP_RX_FIFO0 | FDCAN_IT_GROUP_RX_FIFO1|
            FDCAN_IT_RX_FIFO0_NEW_MESSAGE|FDCAN_IT_RX_FIFO1_NEW_MESSAGE,
          FDCAN_TX_BUFFER0 |FDCAN_TX_BUFFER1|FDCAN_TX_BUFFER2 ) != HAL_OK)
    {
    assert_param(0);
    }

    TxHeader.Identifier = 0x4321;								//发送帧配置 32位ID
    TxHeader.IdType = FDCAN_EXTENDED_ID;						//扩展ID
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;					//数据帧
    //TxHeader.DataLength = FDCAN_DLC_BYTES_8;				    //数据长度 非固定 发送时再配置
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;						//打开速率切换
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;						//传统CAN模式  
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;			//无发送事件
    TxHeader.MessageMarker = 0;									//用于识别Tx消息状态。取值范围为0 ~ 0xFF之间的数字
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)					//启动CAN模块
    {
        assert_param(0);
    }
}

/*****************************************************
	* 函数功能: CAN2经典配置
    * 输入参数: void
	* 返 回 值: void
	* 说    明：
******************************************************/
void MX_FDCAN2_Init(void)
{
	FDCAN_FilterTypeDef sFilterConfig;
    hfdcan2.Instance = FDCAN2;
    hfdcan2.Init.ClockDivider = FDCAN_CLOCK_DIV1;				    //CAN时钟不分频
    hfdcan2.Init.FrameFormat = FDCAN_FRAME_CLASSIC;					//经典CAN模式  
    hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;						    //普通模式	
    hfdcan2.Init.AutoRetransmission = DISABLE;					    //关闭自动重传，传统模式下关闭
    hfdcan2.Init.TransmitPause = DISABLE;							//关闭传输暂停
    hfdcan2.Init.ProtocolException = ENABLE;					    //协议异常处理

    hfdcan2.Init.NominalPrescaler = 4;								//分频系数
    hfdcan2.Init.NominalSyncJumpWidth = 1;							//重新同步跳跃宽度
    hfdcan2.Init.NominalTimeSeg1 = 11;								//采样点之前标志时间段:2~256
    hfdcan2.Init.NominalTimeSeg2 = 4;								//采样点之前标志时间段:2~128
    //baud = can_clk/DataPrescaler/(DataTimeSeg1+DataTimeSeg2+1)  
    hfdcan2.Init.DataPrescaler = 4;									//数据段预分频				
    hfdcan2.Init.DataSyncJumpWidth = 1;								//数据位同步跳跃宽度 DataSyncJumpWidth需小于DataTimeSeg1 DataTimeSeg2的最小值
    hfdcan2.Init.DataTimeSeg1 = 11;									//采样点之前数据时间段
    hfdcan2.Init.DataTimeSeg2 = 4;									//采样点之后数据时间段

    hfdcan2.Init.StdFiltersNbr = 0;									//标准信息ID滤波器数量
    hfdcan2.Init.ExtFiltersNbr = 1;									//扩展信息ID滤波器数量
    hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;	        //发送FIFO序列模式	可选队列和FIFO
    if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK)							//初始化
    {
    //assert_param(0);
    }
    sFilterConfig.IdType = FDCAN_EXTENDED_ID;						//滤波器配置	拓展帧
    sFilterConfig.FilterIndex = 0;									//滤波器索引
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;				    //滤波器类型	数据帧
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;		    //过滤器0关联到FIFO1
    sFilterConfig.FilterID1 = 0x1130;
    sFilterConfig.FilterID2 = 0xFFEF;
    if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig) != HAL_OK)	//滤波器配置
    {
    //assert_param(0);
    }

    if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_REJECT, FDCAN_REJECT, //拒收远程帧
                                        FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)	
    {
    //assert_param(0);
    }
    if (HAL_FDCAN_ActivateNotification(&hfdcan2,FDCAN_IT_GROUP_RX_FIFO0 | FDCAN_IT_GROUP_RX_FIFO1 |
                FDCAN_IT_RX_FIFO0_NEW_MESSAGE| FDCAN_IT_RX_FIFO1_NEW_MESSAGE|FDCAN_IT_BUS_OFF, 0) != HAL_OK)
    {
    //assert_param(0);
    }

    if (HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_GROUP_RX_FIFO0 | FDCAN_IT_GROUP_RX_FIFO1|
            FDCAN_IT_RX_FIFO0_NEW_MESSAGE|FDCAN_IT_RX_FIFO1_NEW_MESSAGE,
          FDCAN_TX_BUFFER0 |FDCAN_TX_BUFFER1|FDCAN_TX_BUFFER2 ) != HAL_OK)
    {
    //assert_param(0);
    }
    /* Prepare Tx Header */
    TxHeader.Identifier = 0x4321;									    //32位ID
    TxHeader.IdType = FDCAN_EXTENDED_ID;								//扩展ID
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;							//数据帧
    //TxHeader.DataLength = FDCAN_DLC_BYTES_8;						    //数据长度 非固定 发送时再配置
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;							    //打开速率切换
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;								//传统CAN模式  
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;					//无发送事件
    TxHeader.MessageMarker = 0;											//用于识别Tx消息状态。取值范围为0 ~ 0xFF之间的数字
    if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK)							//启动CAN模块
    {
    //assert_param(0);
    }
}
/*****************************************************
	* 函数功能: CAN1数据发送
    * 输入参数: 帧ID exfid	| 数据地址pbuff	|数据长度len
	* 返 回 值: uint8_t 0 为发送成功	非0表示发送失败
	* 说    明：发送超时需配置
******************************************************/
uint8_t FDCAN1_SendMsg( unsigned char * pbuff , unsigned short len )
{
	return HAL_FDCANx_SendMsg( &hfdcan1 , &TxHeader , SOUR_ADDRESS , pbuff , len );
}
/*****************************************************
	* 函数功能: CAN2数据发送
    * 输入参数: 帧ID exfid	| 数据地址pbuff	|数据长度len
	* 返 回 值: uint8_t 0 为发送成功	非0表示发送失败
	* 说    明：发送超时需配置
******************************************************/
uint8_t FDCAN2_SendMsg( unsigned char * pbuff , unsigned short len )
{
	return HAL_FDCANx_SendMsg( &hfdcan2 , &TxHeader , SOUR_ADDRESS , pbuff , len );
}

/*****************************************************
	* 函数功能: CAN2接收数据回调函数
	//CAN2已设置ID过滤   只通过	ID：0x1130	ID：0x1120
    * 输入参数: 
	* 返 回 值: 
	* 说    明：
******************************************************/
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{	
  if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET)
  {
    Loop:
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &RxHeader, RxData) != HAL_OK)
    {

    }
    if (RxHeader.IdType == FDCAN_EXTENDED_ID)					
    {
        RxHeader.DataLength = RxHeader.DataLength>>16;
        SafeBuf_Write(&gCanDealMsg.gCanRecSafeBuf, RxData,RxHeader.DataLength);
    }    
    if(RxHeader.DataLength != 0)//gCanRxData.DataLength不会发生变化   左移的时候数据
    goto Loop;
  }
}

// CANH - CANL
void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
	if( hfdcan->Instance == FDCAN2 )
	{
		MX_FDCAN2_Init();
	}
	if( hfdcan->Instance == FDCAN1 )
	{
		MX_FDCAN1_Init();
	}
}

void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes)
{
	if( hfdcan->Instance == FDCAN2 )
	{
		
	}
	if( hfdcan->Instance == FDCAN1 )
	{
		
	}
}
