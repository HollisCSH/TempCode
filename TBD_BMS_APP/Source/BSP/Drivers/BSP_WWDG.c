/****************************************************************************/
/* 	File    	BSP_WWDG.c 			 
 * 	Author		Hollis
 *	Notes			使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_WWDG.h"
#define WWDG_WINDOW    0x7F
#define WWDG_COUNTER   0x7F
WWDG_HandleTypeDef hwwdg;


void HAL_WWDG_MspInit(WWDG_HandleTypeDef* hwwdg)
{
  if(hwwdg->Instance==WWDG)
  {
  /* USER CODE BEGIN WWDG_MspInit 0 */

  /* USER CODE END WWDG_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_WWDG_CLK_ENABLE();
  /* USER CODE BEGIN WWDG_MspInit 1 */

  /* USER CODE END WWDG_MspInit 1 */
  }

}
void HAL_WWDG_MspDeInit(WWDG_HandleTypeDef* hwwdg)
{
  if(hwwdg->Instance==WWDG)
  {
  /* USER CODE BEGIN WWDG_MspInit 0 */

  /* USER CODE END WWDG_MspInit 0 */
    /* Peripheral clock enable */
//    __HAL_RCC_WWDG_CLK_ENABLE();
//		__HAL_RCC_WWDG_CLK_DISABLE();
  /* USER CODE BEGIN WWDG_MspInit 1 */

  /* USER CODE END WWDG_MspInit 1 */
  }

}
/**
  * @brief WWDG Initialization Function
  * @param None
  * @retval None
  */
void MX_WWDG_Init(void)
{

  /* USER CODE BEGIN WWDG_Init 0 */
     /*  Default WWDG Configuration:
      1] Set WWDG counter to 0x7F  and window to 0x50
      2] Set Prescaler to WWDG_PRESCALER_8

      Timing calculation:
      a) WWDG clock counter period (in ms) = (4096 * WWDG_PRESCALER_8) / (PCLK1 / 1000)
                                           = 0,512 ms   8.192
      b) WWDG timeout (in ms) = (0x7F + 1) * 0,512
                              ~= 65,53 ms
			 b) WWDG timeout (in ms) = (0x7F -0x40 + 1) * 0,512
                               = 32.768ms
      => After refresh, WWDG will expires after 65,53 ms and generate reset if
      counter is not reloaded.
      c) Time to enter inside window
      Window timeout (in ms) = (127 - 80 + 1) * 0,512
                             = 24,58 ms 
	     Window timeout (in ms) = (127 - 80 + 1) * 0,512
                             = 24,58 ms 
	
	*/
  /* USER CODE END WWDG_Init 0 */

  /* USER CODE BEGIN WWDG_Init 1 */

  /* USER CODE END WWDG_Init 1 */
//	__HAL_RCC_CLEAR_RESET_FLAGS();
  hwwdg.Instance = WWDG;
  hwwdg.Init.Prescaler = WWDG_PRESCALER_32;
  hwwdg.Init.Window = WWDG_WINDOW;
  hwwdg.Init.Counter = WWDG_COUNTER;
  hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;
  if (HAL_WWDG_Init(&hwwdg) != HAL_OK)
  {
    assert_param(0);                  //断言 错误提示
  }
}

void  FeedWDog(void)
{
	if (HAL_WWDG_Refresh(&hwwdg) != HAL_OK)
	{
		assert_param(0);                  //断言 错误提示
	}	
}
