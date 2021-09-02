/****************************************************************************/
/* 	File    	BSP_IIC.c 			 
 * 	Author		Hollis
 *	Notes			使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_IIC.h"
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

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


/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x90103EFF;	//		0x00400B27 400 	0x10707DBC 100	0x403032CA	50	0x90103EFF	20
  hi2c1.Init.OwnAddress1 = SH367309_WRITE_ADDR;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    assert_param(0);
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    assert_param(0);
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    assert_param(0);
  }
  /** I2C Fast mode Plus enable
  */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);
}

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
    assert_param(0);
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    assert_param(0);
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    assert_param(0);
  }
  /** I2C Fast mode Plus enable
  */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C2);
}

void MX_I2C1_REAR(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
		while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
    {
    }
		HAL_I2C_Mem_Read_IT( &hi2c1, I2C_ADDRESS, REG_ADDRESS, I2C_MEMADD_SIZE_16BIT, pData, Size);
}
void MX_I2C1_WRITE(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
		while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
    {
    }
		HAL_I2C_Mem_Write_IT( &hi2c1, SH367309_WRITE_ADDR, REG_ADDRESS, I2C_MEMADD_SIZE_16BIT, pData, Size);
}


void I2C_REAR(I2C_TypeDef *hi2c , uint32_t RegAdress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
		I2C_HandleTypeDef *hi2cx;
		if(hi2c == AFE_IIC_CH)
		{
			hi2cx = &hi2c1;
			while (HAL_I2C_GetState(hi2cx) != HAL_I2C_STATE_READY)
			{
			}
			HAL_I2C_Mem_Read_IT( hi2cx, SH367309_WRITE_ADDR, RegAdress, I2C_MEMADD_SIZE_16BIT, pData, Size);
		}
		else if(hi2c == ACC_IIC_CH)
		{
			hi2cx = &hi2c2;
			while (HAL_I2C_GetState(hi2cx) != HAL_I2C_STATE_READY)
			{
			}
			HAL_I2C_Mem_Read_IT( hi2cx, DA213_WRITE_ADDR, RegAdress, I2C_MEMADD_SIZE_8BIT, pData, Size);
		}
		else return;
		
//		if(HAL_I2C_Mem_Read( &hi2c1, SH367309_WRITE_ADDR, RegAdress, I2C_MEMADD_SIZE_16BIT, pData, Size, Timeout)!= HAL_OK)
//		{
//			/* Error_Handler() function is called when error occurs. */
//			Error_Handler();
//		}
}
void I2C_WRITE(I2C_TypeDef *hi2c ,uint32_t RegAdress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
		I2C_HandleTypeDef *hi2cx;
		if(hi2c == AFE_IIC_CH)
		{
			hi2cx = &hi2c1;
			while (HAL_I2C_GetState(hi2cx) != HAL_I2C_STATE_READY)
			{
			}
			HAL_I2C_Mem_Write_IT( hi2cx, SH367309_WRITE_ADDR, RegAdress, I2C_MEMADD_SIZE_16BIT, pData, Size);	
		}
		else if(hi2c == ACC_IIC_CH)
		{
			hi2cx = &hi2c2;
			while (HAL_I2C_GetState(hi2cx) != HAL_I2C_STATE_READY)
			{
			}
			HAL_I2C_Mem_Write_IT( hi2cx, DA213_WRITE_ADDR, RegAdress, I2C_MEMADD_SIZE_8BIT, pData, Size);	
		}
		else return;
//		if(HAL_I2C_Mem_Write( &hi2c1, SH367309_WRITE_ADDR, RegAdress, I2C_MEMADD_SIZE_16BIT, pData, Size, Timeout)!= HAL_OK)
//		{
//			/* Error_Handler() function is called when error occurs. */
//			Error_Handler();
//		}	   
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
  /* Toggle LED4: Transfer in transmission process is correct */
  //HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); 
	
}
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
  /* Toggle LED4: Transfer in reception process is correct */
  //HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); 
}
