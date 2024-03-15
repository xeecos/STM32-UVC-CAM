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
#include "srcjpg.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


//配置USB时钟,USBclk=48Mhz
void Set_USBClock(void)
{
	RCC->CFGR &= ~RCC_CFGR_USBPRE; //USBclk=PLLclk/1.5=48Mhz	    
	RCC->APB1ENR |= RCC_APB1ENR_USBEN; //USB时钟使能					 
}

/*******************************************************************************
* Function Name  : Enter_LowPowerMode.
* Description    : Power-off system clocks and power while entering suspend mode.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void)
{
	printf("SUSPENDED\n");
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
	printf("leave SUSPENDED\n");
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



//USB中断配置
void USB_Interrupts_Config(void)
{
    MY_NVIC_PriorityGroupConfig(2);
	EXTI->IMR |= EXTI_RTSR_TR18;//  开启线18上的中断
	EXTI->RTSR |= EXTI_RTSR_TR18;//line 18上事件上升降沿触发	 
	MY_NVIC_Init(1, 0, USB_HP_CAN1_TX_IRQn, 2);//组2，优先级次之 
	MY_NVIC_Init(1, 0, USB_LP_CAN1_RX0_IRQn, 2);//组2，优先级次之 
	MY_NVIC_Init(0, 0, USBWakeUp_IRQn, 2);     //组2，优先级最高	 	 
}

///*******************************************************************************
//* Function Name : UsbCamera_Send.
//* Description   : 初始化编码器
//* Input         : 
//* Output        : 
//* Return value  : 
//*******************************************************************************/
//void UsbCamera_StreamStart(void)
//{
//	//u32 datalen = 0;
//
//	//初始化UVC payload head
//	sendbuf[0] = 0x02;		//HLF为2(标头长度字段指定了标头的长度，单位为字节)
//	sendbuf[1] = 0x01;	
//	// 初始化单帧数据传输计数
//	sendsize = 0;
//
//	//// 计算发送数据长度
//	//datalen = PACKET_SIZE - 2;
//	//myMemcpy(sbuf, sendbuf + 2, datalen);
//	//UserToPMABufferCopy(sendbuf, ENDP1_BUF0Addr, PACKET_SIZE);
//	//_SetEPDblBuf0Count(ENDP1, EP_DBUF_IN, PACKET_SIZE);
//	//sendsize = datalen;
//
//	// 收到IN令牌后发送数据
//	_SetEPTxStatus(ENDP1, EP_TX_VALID);
//}


/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Get_SerialNum(void)
{
	u32 Device_Serial0, Device_Serial1, Device_Serial2;

	Device_Serial0 = *(u32*)(0x1FFFF7E8);
	Device_Serial1 = *(u32*)(0x1FFFF7EC);
	Device_Serial2 = *(u32*)(0x1FFFF7F0);

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
