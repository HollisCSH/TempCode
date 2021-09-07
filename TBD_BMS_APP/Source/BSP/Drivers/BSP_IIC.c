/****************************************************************************/
/* 	File    	BSP_IIC.c 			 
 * 	Author		Hollis
 *	Notes		ʹ��BSPʱ���stm32g0xx_hal_conf.h��Ӧ�ĺ�
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08		first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_IIC.h"

/* ˽�к궨�� ----------------------------------------------------------------*/
#define IIC_TimeOut 50         //��ȡIIC״̬��ʱʱ��
/* ˽�б��� ------------------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
uint16  gAFE_IIC_COMM_TIME = 0; //ͨѶ��ʱ��ʱ
uint16  gACC_IIC_COMM_TIME = 0; //ͨѶ��ʱ��ʱ
//typedef struct
//{
//    uint8_t *buff;
//    uint8_t bufflen;
//}_AFE_READ_DATA;
//_AFE_READ_DATA sAFE_READ_DATA = {0}; 
/* ???? ------------------------------------------------------------------*/
/* ???? ------------------------------------------------------------------*/

/*****************************************************
	* ��������: I2C�˿ڡ�ʱ�ӡ��жϳ�ʼ��
	* �������: I2C_HandleTypeDef i2c����
	* �� �� ֵ: ��
	* ˵    �������ຯ���ڵ���HAL_XX_Init ������
******************************************************/
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(hi2c->Instance==I2C1)
	{
		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**I2C1 GPIO Configuration
		PB6     ------> I2C1_SCL
		PB7     ------> I2C1_SDA
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF6_I2C1;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* Peripheral clock enable */
		__HAL_RCC_I2C1_CLK_ENABLE();
		HAL_NVIC_SetPriority(I2C1_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(I2C1_IRQn);
	}
	else if(hi2c->Instance==I2C2)
	{
		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**I2C1 GPIO Configuration
		PB13     ------> I2C2_SCL
		PB14     ------> I2C2_SDA
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF6_I2C2;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		__HAL_RCC_I2C2_CLK_ENABLE();
		HAL_NVIC_SetPriority(I2C2_3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(I2C2_3_IRQn);
	}
}

/*****************************************************
	* ��������: I2C�˿ڡ�ʱ�ӡ��ж� �ض�
	* �������: I2C_HandleTypeDef ����
	* �� �� ֵ: ��
	* ˵    �������ຯ���ڵ���HAL_XX_DeInit ������
******************************************************/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
	if(hi2c->Instance==I2C1)
	{
		__HAL_RCC_I2C1_CLK_DISABLE();
		HAL_NVIC_DisableIRQ(I2C1_IRQn); 
		//PB6     ------> I2C1_SCL
		//PB7     ------> I2C1_SDA
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7 );
	}
	else if(hi2c->Instance==I2C2)
	{
		__HAL_RCC_I2C2_CLK_DISABLE();
		HAL_NVIC_DisableIRQ(I2C2_3_IRQn); 
		//PB13     ------> I2C2_SCL
		//PB14     ------> I2C2_SDA
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13 | GPIO_PIN_14 );
	}
}


/*****************************************************
	* ��������: IIC�������� 400k
	* �������: void
	* �� �� ֵ: void
	* ˵    ����
******************************************************/
void MX_I2C1_Init(void)
{
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x403032CA;	//		0x00400B27 400 	0x10707DBC 100	0x403032CA	50	0x90103EFF	20
	hi2c1.Init.OwnAddress1 = SH367309_WRITE_ADDR;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		assert_param(0);                  //���� ������ʾ
	}
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
		assert_param(0);                  //���� ������ʾ
	}
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
	{
		assert_param(0);                  //���� ������ʾ
	}
	HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);
}

/*****************************************************
	* ��������: IIC�������� 400k
	* �������: void
	* �� �� ֵ: void
	* ˵    ����
******************************************************/
void MX_I2C2_Init(void)
{
	hi2c2.Instance = I2C2;
	hi2c2.Init.Timing = 0x90103EFF;	//		0x00400B27 400 	0x10707DBC 100	0x403032CA	50	0x90103EFF	20
	hi2c2.Init.OwnAddress1 = DA213_WRITE_ADDR;
	hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c2.Init.OwnAddress2 = 0;
	hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c2) != HAL_OK)
	{
		assert_param(0);                  //���� ������ʾ
	}
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
		assert_param(0);                  //���� ������ʾ
	}
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
	{
		assert_param(0);                  //���� ������ʾ
	}
	HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C2);
}

/*****************************************************
	* ��������: IIC��ʼ��
	* �������: void
	* �� �� ֵ: void
	* ˵    ���������ⲿ������ĺ�����
******************************************************/
void BSPI2C_Init(I2C_TypeDef *I2cHandle)
{
	if(I2cHandle == I2C1)
		MX_I2C1_Init();
	else if(I2cHandle == I2C2)
		MX_I2C2_Init();
}

/*****************************************************
	* ��������: IIC�ض�
	* �������: void
	* �� �� ֵ: void
	* ˵    ���������ⲿ������ĺ�����
******************************************************/
void BSPI2C_DeInit(I2C_TypeDef *I2cHandle)
{
	if(I2cHandle == I2C1)
		HAL_I2C_DeInit(&hi2c1);
	else if(I2cHandle == I2C2)
		HAL_I2C_DeInit(&hi2c2);
}

/*****************************************************
	* ��������: IIC��ȡ�Ĵ�������
	* �������: *hi2c , �Ĵ�����ַ RegAdress, ���ָ�� *pData, ���ݳ��� Size
	* �� �� ֵ: ����0���óɹ�
	* ˵    �����жϷ��ͣ����óɹ��Ƿ��ͳɹ����ȷ�������ж�
******************************************************/
uint8_t BSP_I2C_MEM_REAR(I2C_TypeDef *hi2c , uint32_t RegAdress, uint8_t *pData, uint16_t Size)//���óɹ�����0
{
	//I2C_HandleTypeDef *hi2cx;
    uint8_t timeout = 0;
	if(hi2c == SH367309_IIC_CHANNEL)
	{
		I2C_HandleTypeDef *hi2cx = &hi2c1;
		while (HAL_I2C_GetState(hi2cx) != HAL_I2C_STATE_READY)
		{
            timeout++;
            if(timeout >= IIC_TimeOut )
            {
                gSHAFEData.IICState = IIC_Sta_ERR;
                return 1;
            }
		}
		gSHAFEData.IICState = IIC_Sta_Rd_Busy;
        gAFE_IIC_COMM_TIME = 10;
		HAL_I2C_Mem_Read_IT( hi2cx, SH367309_WRITE_ADDR, RegAdress, I2C_MEMADD_SIZE_16BIT, pData, Size);
		#ifdef IIC_DEBUG
		_UN_NB_Printf("SH367309 READ: REG :%02X %d\n",RegAdress,Size);
		#endif
	}
	else if(hi2c == DA213_IIC_CHANNEL)
	{
		I2C_HandleTypeDef *hi2cx = &hi2c2;
		while (HAL_I2C_GetState(hi2cx) != HAL_I2C_STATE_READY)
		{
            timeout++;
            if(timeout >= IIC_TimeOut )
            {
                gAccVar.IICState = IIC_Sta_ERR;
                return 1;
            }
		}
		gAccVar.IICState = IIC_Sta_Rd_Busy;
		HAL_I2C_Mem_Read_IT ( hi2cx, DA213_WRITE_ADDR, RegAdress, I2C_MEMADD_SIZE_8BIT, pData, Size);
		#ifdef IIC_DEBUG
		_UN_NB_Printf("DA213 READ: REG :%02X %d\n",RegAdress,Size);
		#endif
	}
	else return 1;
	return 0;
}

/*****************************************************
	* ��������: IICд��Ĵ�������
	* �������: *hi2c , �Ĵ�����ַ RegAdress, ���ָ�� *pData, ���ݳ��� Size
	* �� �� ֵ: ����0���óɹ�
	* ˵    �����жϷ��ͣ����óɹ��Ƿ��ͳɹ����ȷ�������ж�
******************************************************/
uint8_t BSP_I2C_MEM_WRITE(I2C_TypeDef *hi2c ,uint32_t RegAdress, uint8_t *pData, uint16_t Size)//���÷��ͳɹ�����0
{
	I2C_HandleTypeDef *hi2cx;
    uint8_t timeout = 0;
	if(hi2c == SH367309_IIC_CHANNEL)
	{
		hi2cx = &hi2c1;
		while (HAL_I2C_GetState(hi2cx) != HAL_I2C_STATE_READY)
		{
            timeout++;
            if(timeout >= IIC_TimeOut )
            {
                gSHAFEData.IICState = IIC_Sta_ERR;
                return 1;
            }
		}
		gSHAFEData.IICState = IIC_Sta_Rd_Busy;
        gAFE_IIC_COMM_TIME = 100;
		HAL_I2C_Mem_Write_IT( hi2cx, SH367309_WRITE_ADDR, RegAdress, I2C_MEMADD_SIZE_8BIT, pData, Size);
		#ifdef IIC_DEBUG      
		_UN_NB_Printf("SH367309 W: REG :%02X\n",RegAdress);
		#endif
	}
	else if(hi2c == DA213_IIC_CHANNEL)
	{
		hi2cx = &hi2c2;
		while (HAL_I2C_GetState(hi2cx) != HAL_I2C_STATE_READY)
		{
            timeout++;
            if(timeout >= IIC_TimeOut )
            {
                gAccVar.IICState = IIC_Sta_ERR;
                return 1;
            }
		}
		gAccVar.IICState = IIC_Sta_Wr_Busy;
		HAL_I2C_Mem_Write_IT( hi2cx, DA213_WRITE_ADDR, RegAdress, I2C_MEMADD_SIZE_8BIT, pData, Size);
		#ifdef IIC_DEBUG
		_UN_NB_Printf("DA213 W: REG :%02X\n",RegAdress); 
		#endif      
	}
	else return 1;

	#ifdef IIC_DEBUG
	for(int sl_temp = 0 ; sl_temp < Size ; sl_temp++)
	{
		_UN_NB_Printf(" %02X ",pData[sl_temp]);
	}
	_UN_NB_Printf("\n");
	#endif
	return 0;
}
//uint32_t BSPIICRead( I2C_TypeDef *hi2cx, uint16_t addr, uint8_t *pstr, uint16_t cnt)
//{
//		I2C_HandleTypeDef *hi2c;
//		if(hi2cx == SH367309_IIC_CHANNEL)
//		{
//			hi2c = &hi2c1;
//			gSHAFEData.IICState = IIC_Sta_Rd_Busy;
//		}
//		else if(hi2cx == DA213_IIC_CHANNEL)
//		{
//			hi2c = &hi2c2;
//			gAccVar.IICState = IIC_Sta_Rd_Busy;
//		}
//		while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
//		{
//		}
//		HAL_I2C_Master_Receive_IT(hi2c, addr, pstr , cnt);
//		return 1;
//}
//uint32_t BSPIICWrite(I2C_TypeDef *hi2cx, uint16_t addr, uint8_t *pstr, uint16_t cnt)
//{
//		I2C_HandleTypeDef *hi2c;
//		if(hi2cx == SH367309_IIC_CHANNEL)
//		{
//			hi2c = &hi2c1;
//			gSHAFEData.IICState = IIC_Sta_Wr_Busy;
//		}
//		else if(hi2cx == DA213_IIC_CHANNEL)
//		{
//			hi2c = &hi2c2;
//			gAccVar.IICState = IIC_Sta_Wr_Busy;
//		}

//		while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
//		{
//		}
//		HAL_I2C_Master_Transmit_IT(hi2c, addr, pstr , cnt);
//		return 1;	
//}

/*****************************************************
	* ��������: I2Cֱ�Ӷ�ȡ ������
	* �������: I2C_HandleTypeDef
	* �� �� ֵ: 
	* ˵    ����
******************************************************/
uint32_t BSPIICWriteAndRead(I2C_TypeDef *hi2cx, uint16_t addr, uint8_t *pwstr, uint16_t wcnt, uint8_t *prstr, uint16_t rcnt)
{
		I2C_HandleTypeDef *hi2c;
        uint8_t timeout = 0;
		uint16_t MemAddress;
		uint16_t MemAddSize;
		if(wcnt == I2C_MEMADD_SIZE_8BIT)
		{
			MemAddress = pwstr[0];
			MemAddSize = I2C_MEMADD_SIZE_8BIT ;
		}
		else if(wcnt == I2C_MEMADD_SIZE_16BIT)
		{
			MemAddress = ((uint16_t)pwstr[0]<<8) | (pwstr[1]&0x00FF);
			MemAddSize = I2C_MEMADD_SIZE_16BIT ;
		}
		else return 0;	
		if(hi2cx == SH367309_IIC_CHANNEL)
		{
			hi2c = &hi2c1;
			while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
			{
                timeout++;
                if(timeout >= IIC_TimeOut )
                {
                    gSHAFEData.IICState = IIC_Sta_ERR;
                    return 1;
                }            
			}
			gSHAFEData.IICState = IIC_Sta_Rd_Busy;
            if(rcnt>0x30)
                gAFE_IIC_COMM_TIME = 320;               //��ʱʱ��ϳ�
            else if(rcnt>0x10)
                gAFE_IIC_COMM_TIME = 200;               //��ʱʱ��ϳ�
            else
                gAFE_IIC_COMM_TIME = 120;               //��ʱʱ��ϳ�
			HAL_I2C_Mem_Read_IT( hi2c, addr, MemAddress, MemAddSize, prstr, rcnt);	
		}
		else if(hi2cx == DA213_IIC_CHANNEL)
		{
			hi2c = &hi2c2;
			while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
			{
                timeout++;
                if(timeout >= IIC_TimeOut )
                {
                    gAccVar.IICState = IIC_Sta_ERR;
                    return 1;
                }                
			}
			gAccVar.IICState = IIC_Sta_Rd_Busy;
			HAL_I2C_Mem_Read_IT( hi2c, addr, MemAddress, MemAddSize, prstr, rcnt);	
		}
		else return 0;	
		return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void IICTickHandler(void)   //1ms   IICͨѶ��ʱ
{
    if((gSHAFEData.IICState == IIC_Sta_Rd_Busy) || (gSHAFEData.IICState == IIC_Sta_Wr_Busy))
    {
        gAFE_IIC_COMM_TIME--;
        if(gAFE_IIC_COMM_TIME <= 1 )
        {
            gAFE_IIC_COMM_TIME = 1;
            gSHAFEData.IICState = IIC_Sta_ERR;
        }
    }
//    else gAFE_IIC_COMM_TIME = 100;
    
    if((gAccVar.IICState == IIC_Sta_Rd_Busy) || (gAccVar.IICState == IIC_Sta_Wr_Busy))
    {
        gACC_IIC_COMM_TIME++;
        if(gACC_IIC_COMM_TIME >=50)
        {
            gACC_IIC_COMM_TIME = 50;
            gAccVar.IICState = IIC_Sta_ERR;
        }
    }    
    else gACC_IIC_COMM_TIME = 0;
//gAFE_IIC_COMM_TIME = 0;
//gACC_IIC_COMM_TIME = 0;
}
/*****************************************************
	* ��������: I2C������ɻص�����
	* �������: I2C_HandleTypeDef
	* �� �� ֵ: void
	* ˵    ����ϵͳ���� ��stm32g0xx_it.c���жϺ�������
******************************************************/
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *I2cHandle)	//IICͨѶ��MEM ���ͻص�
{
	if(I2cHandle->Instance == SH367309_IIC_CHANNEL)
	{
		gSHAFEData.IICState = IIC_Sta_Done;
        gAFE_IIC_COMM_TIME = 0;
		#ifdef IIC_DEBUG
		_UN_NB_Printf("SH cbW\n");  
		#endif    
	}
	else if(I2cHandle->Instance == DA213_IIC_CHANNEL)
	{
		gAccVar.IICState = IIC_Sta_Done;
        gACC_IIC_COMM_TIME = 0;
		#ifdef IIC_DEBUG
		_UN_NB_Printf("DA213 cbW\n");
		#endif
	}
}

/*****************************************************
	* ��������: I2C������ɻص�����
	* �������: I2C_HandleTypeDef
	* �� �� ֵ: void
	* ˵    ����ϵͳ���� ��stm32g0xx_it.c���жϺ�������
******************************************************/
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *I2cHandle)	//IICͨѶ��MEM ���ջص�
{
	if(I2cHandle->Instance == SH367309_IIC_CHANNEL)
	{
		gSHAFEData.IICState = IIC_Sta_Done;
        gAFE_IIC_COMM_TIME = 0;
		#ifdef IIC_DEBUG
		_UN_NB_Printf("SH cbR\n");
		#endif
	}
	else if(I2cHandle->Instance == DA213_IIC_CHANNEL)
	{
		gAccVar.IICState = IIC_Sta_Done;
        gACC_IIC_COMM_TIME = 0;
		#ifdef IIC_DEBUG
		_UN_NB_Printf("DA213 cbR\n");  
		#endif
	}
}
//void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *I2cHandle)	//IICͨѶ ���ͻص�
//{
//  if(I2cHandle->Instance == SH367309_IIC_CHANNEL)
//	{
//		gSHAFEData.IICState = IIC_Sta_Done;
//	}
//	else if(I2cHandle->Instance == DA213_IIC_CHANNEL)
//	{
//		gAccVar.IICState = IIC_Sta_Done;
//	}	
//}
//void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *I2cHandle)	//IICͨѶ ���ջص�
//{
//  if(I2cHandle->Instance == SH367309_IIC_CHANNEL)
//	{
//		gSHAFEData.IICState = IIC_Sta_Done;
//	}
//	else if(I2cHandle->Instance == DA213_IIC_CHANNEL)
//	{
//		gAccVar.IICState = IIC_Sta_Done;
//	}	
//}

//=============================================================================================
//��������	: I2C_TransferReturn_TypeDef BSPIICChannalStateGet(uint8_t ch)
//�������	: ch: IIC ͨ�����
//�������	: IIC�շ�״̬
//��̬����	: void
//��	��	: ��ѯIICͨ���շ�״̬
//ע	��	:
//=============================================================================================
int8_t BSPIIC_StateGet(I2C_TypeDef *hi2cx)
{
	if(hi2cx == SH367309_IIC_CHANNEL)
	{
		return gSHAFEData.IICState;	//������������ģ�����޸�
	}
	else if(hi2cx == DA213_IIC_CHANNEL) 
	{
		return gAccVar.IICState;
	}
	return IIC_Sta_ERR;
}



