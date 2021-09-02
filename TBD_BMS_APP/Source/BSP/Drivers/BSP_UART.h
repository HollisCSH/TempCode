/****************************************************************************/
/* 	File    	BSP_UART.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_UART_H__
#define __BSP_UART_H__
#include <stdint.h>
#include <stdio.h>
#include "main.h"

#define UART_DEBUG				LPUART2
#define UART_DEBUG_BAUD 		115200

#define UART_WIFI_DEBUG			LPUART1
#define UART_WIFI_DEBUG_BAUD 	115200
#define UART_WIFI_RX_PORT     	GPIOB
#define UART_WIFI_RX_PIN      	GPIO_PIN_10
#define UART_WIFI_TX_PORT     	GPIOB
#define UART_WIFI_TX_PIN      	GPIO_PIN_11
#define UART_WIFI_IRQn        	USART3_4_5_6_LPUART1_IRQn

#define UART_GPS				USART1
#define UART_GPS_BAUD 	      	9600
#define UART_GPS_RX_PORT      	GPIOC
#define UART_GPS_RX_PIN       	GPIO_PIN_5
#define UART_GPS_TX_PORT      	GPIOB
#define UART_GPS_TX_PIN       	GPIO_PIN_4
#define UART_GPS_IRQn         	USART1_IRQn


/*****************************************************
	* ��������: ���ڳ�ʼ��
    * �������: USART_TypeDef ���� uint32_t baud ������
    * �� �� ֵ: ��
    * ˵    ������
******************************************************/
void BSPUART_Init(USART_TypeDef *UartHandle , uint32_t baud);

/*****************************************************
	* ��������: ���ڹس�ʼ��
    * �������: USART_TypeDef ����
    * �� �� ֵ: ��
    * ˵    ������
******************************************************/
void BSPUART_DeInit(USART_TypeDef *UartHandle);

/*****************************************************
    * ��������: UART���ݷ��ͺ���
    * �������: ���ں� *UartHandle ����ָ�� *Data , ���ݳ��� DataLen
    * �� �� ֵ: ��
    * ˵    ������
******************************************************/
void LPUART_DRV_SendData(USART_TypeDef *UartHandle , uint8_t *Data , uint8_t DataLen);

#ifndef BMS_ENABLE_NB_IOT
#define		_UN_NB_Printf(fmt,...)		printf(fmt,##__VA_ARGS__)
#else
#define		_UN_NB_Printf(fmt,...)
#endif 
//#define		_UN_NB_Printf(fmt,...)		printf(fmt,##__VA_ARGS__)   //û��NBʱprintf��LPUART1����wifiͬ����ʹ��NB ��ӡʹ��LPUART2

#endif /* __BSP_UART_H__ */
