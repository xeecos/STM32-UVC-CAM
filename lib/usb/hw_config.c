/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : hw_config.c
* Author             : MCD Application Team
* Version            : V2.2.0
* Date               : 06/13/2008
* Description        : Hardware Configuration & Setup
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"

/*******************************************************************************
* Function Name  : Enter_LowPowerMode.
* Description    : Power-off system clocks and power while entering suspend mode.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void)
{
	/* Set the device state to suspend */
	bDeviceState = SUSPENDED;
	/* Request to enter STOP mode with regulator in low power mode */
	//PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
}

/*******************************************************************************
* Function Name  : Leave_LowPowerMode.
* Description    : Restores system clocks and power while exiting suspend mode.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Leave_LowPowerMode(void)
{
	DEVICE_INFO *pInfo = &Device_Info;


	/* Set the device state to the correct state */
	if (pInfo->Current_Configuration != 0)
	{
		/* Device configured */
		bDeviceState = CONFIGURED;
	} else
	{
		bDeviceState = ATTACHED;
	}
}


//配置USB时钟,USBclk=48Mhz
void Set_USBClock(void)
{
	RCC->CFGR &= ~RCC_CFGR_USBPRE; //USBclk=PLLclk/1.5=48Mhz	    
	RCC->APB1ENR |= RCC_APB1ENR_USBEN; //USB时钟使能					 
}


//USB中断配置
void USB_Interrupts_Config(void)
{
	EXTI->IMR |= EXTI_RTSR_TR18;//  开启线18上的中断
	EXTI->RTSR |= EXTI_RTSR_TR18;//line 18上事件上升降沿触发	 
	// MY_NVIC_Init(0, 1, USB_HP_CAN1_TX_IRQn, 2);//组2，优先级次之 
	// MY_NVIC_Init(1, 0, USB_LP_CAN1_RX0_IRQn, 2);//组2，优先级次之 
	// MY_NVIC_Init(0, 1, USBWakeUp_IRQn, 2);     //组2，优先级最高	
	NVIC_InitTypeDef nvic_init;
    nvic_init.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    nvic_init.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_init.NVIC_IRQChannelSubPriority = 0;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);
    nvic_init.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
    nvic_init.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvic_init);
    nvic_init.NVIC_IRQChannel = USBWakeUp_IRQn;
    NVIC_Init(&nvic_init); 	 
}

/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Get_SerialNum(void)
{
	uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

	Device_Serial0 = *(uint32_t*)(0x1FFFF7E8);
	Device_Serial1 = *(uint32_t*)(0x1FFFF7EC);
	Device_Serial2 = *(uint32_t*)(0x1FFFF7F0);

	if (Device_Serial0 != 0)
	{
		Camera_StringSerial[2] = (uint8_t)(Device_Serial0 & 0x000000FF);
		Camera_StringSerial[4] = (uint8_t)((Device_Serial0 & 0x0000FF00) >> 8);
		Camera_StringSerial[6] = (uint8_t)((Device_Serial0 & 0x00FF0000) >> 16);
		Camera_StringSerial[8] = (uint8_t)((Device_Serial0 & 0xFF000000) >> 24);

		Camera_StringSerial[10] = (uint8_t)(Device_Serial1 & 0x000000FF);
		Camera_StringSerial[12] = (uint8_t)((Device_Serial1 & 0x0000FF00) >> 8);
		Camera_StringSerial[14] = (uint8_t)((Device_Serial1 & 0x00FF0000) >> 16);
		Camera_StringSerial[16] = (uint8_t)((Device_Serial1 & 0xFF000000) >> 24);

		Camera_StringSerial[18] = (uint8_t)(Device_Serial2 & 0x000000FF);
		Camera_StringSerial[20] = (uint8_t)((Device_Serial2 & 0x0000FF00) >> 8);
		Camera_StringSerial[22] = (uint8_t)((Device_Serial2 & 0x00FF0000) >> 16);
		Camera_StringSerial[24] = (uint8_t)((Device_Serial2 & 0xFF000000) >> 24);
	}
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
