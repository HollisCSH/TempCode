/****************************************************************************/
/* 	File    	BSP_GPIO.c 			 
 * 	Author		Hollis
 *	Notes		使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 				
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08		first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_GPIO.h"
#include "BSP_EXIT.h"

/*****************************************************
	* 函数功能: GPIO端口 时钟 中断初始化
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void BSPGPIOInit(void)
{
	GPIO_InitTypeDef  gpioinitstruct;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();	//端口A初始化
	gpioinitstruct.Pin = CAN_5V_PIN | CTL_AFE_PIN | SH367309_ON_PIN | VCC_4G_CTL_PIN | FLASH_CS_PIN | PRE_DSG_EN_PIN;
	gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &gpioinitstruct);
	
	__HAL_RCC_GPIOB_CLK_ENABLE();	//端口B初始化
	gpioinitstruct.Pin = PRE_DSG_NTC_PIN | MOS_TVS_NTC_PIN ;
	gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &gpioinitstruct);
	
	__HAL_RCC_GPIOC_CLK_ENABLE();	//端口C初始化
	gpioinitstruct.Pin = FLASH_POW_EN_PIN | VPRO_AFE_PIN | PWR_EN_14V_5V_PIN ;
	gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOC, &gpioinitstruct);
	
	__HAL_RCC_GPIOD_CLK_ENABLE();	//端口D初始化
	gpioinitstruct.Pin = DSG_EN_PIN | FUSE_DET_EN_PIN | VCC_MODULE_PIN;
	gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	
	HAL_GPIO_Init(GPIOD, &gpioinitstruct);
  
	GPIO_DefaultInit();				//端口输入初始值配置
	////////////////////////////////以上为输出配置  以下为输入配置
	gpioinitstruct.Pin = MODULE_SHORT_PIN ;
	gpioinitstruct.Mode = GPIO_MODE_INPUT;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	
	HAL_GPIO_Init(GPIOA, &gpioinitstruct);
  
	gpioinitstruct.Pin = HW_REV0_PIN | HW_REV1_PIN | FUSE_DET_PIN ;
	gpioinitstruct.Mode = GPIO_MODE_INPUT;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	
	HAL_GPIO_Init(GPIOD, &gpioinitstruct);
	////////////////////////////////中断配置
	BSP_EXIT_Init();				//外部中断
}
/*****************************************************
	* 函数功能: GPIO端口 关断
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void BSPGPIODeInit(void)
{
//	__HAL_RCC_GPIOA_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOA, CAN_5V_PIN | CTL_AFE_PIN | SH367309_ON_PIN | VCC_4G_CTL_PIN | PRE_DSG_EN_PIN | FLASH_CS_PIN );

//	__HAL_RCC_GPIOB_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOB, PRE_DSG_NTC_PIN | MOS_TVS_NTC_PIN );
	
//	__HAL_RCC_GPIOC_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOC, FLASH_POW_EN_PIN | VPRO_AFE_PIN | PWR_EN_14V_5V_PIN );
	
//	__HAL_RCC_GPIOD_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOD, DSG_EN_PIN | FUSE_DET_EN_PIN | VCC_MODULE_PIN );
}

/*****************************************************
	* 函数功能: GPIO端口 默认状态
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void GPIO_DefaultInit(void)
{
	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_RESET);					//默认值 低电平 CAN通讯原边电源供电使能	高电平有效
	HAL_GPIO_WritePin(CTL_AFE_PORT, CTL_AFE_PIN, GPIO_PIN_RESET);				//默认值 低电平 拉低之后强制关闭充放电MOS
	HAL_GPIO_WritePin(SH367309_ON_PORT, SH367309_ON_PIN, GPIO_PIN_SET);		  	//默认值 高电平 默认拉高，拉低之后AFE进入仓运模式（休眠2uA）
	HAL_GPIO_WritePin(VCC_4G_CTL_PORT, VCC_4G_CTL_PIN, GPIO_PIN_RESET);			//默认值 低电平 拉高后给内部模块供电，拉低后关闭给内部模块的供电
	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);				//默认值 低电平 通讯之前先拉低  结束后拉高
	
	HAL_GPIO_WritePin(PRE_DSG_NTC_PORT, PRE_DSG_NTC_PIN, GPIO_PIN_SET);		  	//默认值 高电平 无预放时拉高
	HAL_GPIO_WritePin(MOS_TVS_NTC_PORT, MOS_TVS_NTC_PIN, GPIO_PIN_SET);		  	//默认值 高电平 TVS、MOS端的NTC检测控制，拉低有效
	
	HAL_GPIO_WritePin(FLASH_POW_EN_PORT, FLASH_POW_EN_PIN, GPIO_PIN_SET);	  	//默认值 高电平 Flash供电，不擦写Flash时默认输出高电平 不供电
	HAL_GPIO_WritePin(VPRO_AFE_PORT, VPRO_AFE_PIN, GPIO_PIN_RESET);				//默认值 低电平 默认拉低，给AFE写EEPROM参数时必须拉高
	HAL_GPIO_WritePin(PWR_EN_14V_5V_PORT, PWR_EN_14V_5V_PIN, GPIO_PIN_RESET);	//默认值 低电平 MCU唤醒后立即拉高，进休眠前延时100mS拉低
	
	HAL_GPIO_WritePin(DSG_EN_PORT, DSG_EN_PIN, GPIO_PIN_RESET);					//默认值 低电平 Flash供电，放电管驱动加速控制++++++++++++++++++++++++++未确定
	HAL_GPIO_WritePin(FUSE_DET_EN_PORT, FUSE_DET_EN_PIN, GPIO_PIN_RESET);		//默认值 低电平 默认拉低，检测FUSE状态时拉高
	HAL_GPIO_WritePin(VCC_MODULE_PORT, VCC_MODULE_PIN, GPIO_PIN_RESET);			//默认值 低电平 外部4G通信供电,等于P+电压，高电平使能------------------
	HAL_GPIO_WritePin(PRE_DSG_EN_PORT, PRE_DSG_EN_PIN, GPIO_PIN_RESET);			//默认值 低电平 预放电开关，默认拉低
	
}
/*****************************************************
	* 函数功能: 使能CAN通讯芯片 供电
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：管脚描述错误，实际控制为3.3V
******************************************************/
void CAN_3V_ENABLE(void)
{
	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(PWR_EN_14V_5V_PORT, PWR_EN_14V_5V_PIN, GPIO_PIN_SET);
}

/*****************************************************
	* 函数功能: 禁止CAN通讯芯片 断电
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：管脚描述错误，实际控制为3.3V
******************************************************/
void CAN_3V_DISABLE(void)
{
	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(PWR_EN_14V_5V_PORT, PWR_EN_14V_5V_PIN, GPIO_PIN_RESET);
}
/*****************************************************
	* 函数功能: 使能14V电源输出
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void PWR_14V_5V_ENABLE(void)
{
	HAL_GPIO_WritePin(PWR_EN_14V_5V_PORT, PWR_EN_14V_5V_PIN, GPIO_PIN_SET);
}

/*****************************************************
	* 函数功能: 禁止14V电源输出
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void PWR_14V_5V_DISABLE(void)
{
	HAL_GPIO_WritePin(PWR_EN_14V_5V_PORT, PWR_EN_14V_5V_PIN, GPIO_PIN_RESET);
}
/*****************************************************
	* 函数功能: 使能FLASH通讯芯片 供电
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void FLASH_POWER_ENABLE(void)
{
	HAL_GPIO_WritePin(FLASH_POW_EN_PORT, FLASH_POW_EN_PIN, GPIO_PIN_RESET);	//低电平打开通电
}

/*****************************************************
	* 函数功能: 禁止FLASH通讯芯片 断电
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void FLASH_POWER_DISABLE(void)
{
	HAL_GPIO_WritePin(FLASH_POW_EN_PORT, FLASH_POW_EN_PIN, GPIO_PIN_SET);
}

/*****************************************************
	* 函数功能: 使能FLASH芯片 通讯
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void FLASH_CS_ENABLE(void)
{
	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);
}

/*****************************************************
	* 函数功能: 禁止FLASH芯片 通讯
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void FLASH_CS_DISABLE(void)
{
	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);
}
/*****************************************************
	* 函数功能: AFE参数读写使能控制输出默认拉低，
				给AFE写EEPROM参数时必须拉高	读取SH367309的参数拉低
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void VPRO_AFE_HI(void)
{
//	HAL_GPIO_WritePin(VPRO_AFE_PORT, VPRO_AFE_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(VPRO_AFE_PORT, VPRO_AFE_PIN, GPIO_PIN_SET);
}
void VPRO_AFE_LOW(void)
{
//	HAL_GPIO_WritePin(VPRO_AFE_PORT, VPRO_AFE_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(VPRO_AFE_PORT, VPRO_AFE_PIN, GPIO_PIN_RESET);
}
/////////////////////////////////////////////////////////////////
void SH367309_ON_ENABLE(void)
{
	HAL_GPIO_WritePin(SH367309_ON_PORT, SH367309_ON_PIN, GPIO_PIN_SET);
}

void SH367309_ON_DISABLE(void)	//运仓模式
{
	HAL_GPIO_WritePin(SH367309_ON_PORT, SH367309_ON_PIN, GPIO_PIN_RESET);
}


/*****************************************************
  * 函数功能: 设置IO为输入状态
  * 输入参数: 具体端口号
  * 返 回 值: 无
  * 说    明：无
******************************************************/
void GPIO_SET_INPUT(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef gpioinitstruct;
    gpioinitstruct.Pin = GPIO_Pin ;
    gpioinitstruct.Mode = GPIO_MODE_INPUT;
    gpioinitstruct.Pull = GPIO_NOPULL;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	
    HAL_GPIO_Init(GPIOx, &gpioinitstruct);
}
/*****************************************************
  * 函数功能: 设置IO为上升沿中断
  * 输入参数: 具体端口号
  * 返 回 值: 无
  * 说    明：无
******************************************************/
void GPIO_SET_RISE_EXIT(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef gpioinitstruct;
    gpioinitstruct.Pin = GPIO_Pin;	                //外置模块接入中断
    gpioinitstruct.Pull = GPIO_NOPULL;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpioinitstruct.Mode = GPIO_MODE_IT_RISING;		//上升沿触发	
    HAL_GPIO_Init(GPIOx, &gpioinitstruct);
    
    if((GPIO_Pin == GPIO_PIN_0)||(GPIO_Pin == GPIO_PIN_1))
    {
        HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0x03, 0);
        HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);       
    }
    else if((GPIO_Pin == GPIO_PIN_2)||(GPIO_Pin == GPIO_PIN_3))
    {
        HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0x03, 0);
        HAL_NVIC_EnableIRQ(EXTI2_3_IRQn); 
    }
    else
    {
        HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0x03, 0);
        HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);    
    }
}

