/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : usb_init.c
* Author             : MCD Application Team
* Version            : V2.2.0
* Date               : 06/13/2008
* Description        : Initialization routines & global variables
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* The number of current endpoint, it will be used to specify an endpoint */
u8 EPindex;
/* The number of current device, it is an index to the Device_Table */
/* u8	Device_no; */
/* Points to the DEVICE_INFO structure of current device */
/* The purpose of this register is to speed up the execution */
DEVICE_INFO *pInformation;
/* Points to the DEVICE_PROP structure of current device */
/* The purpose of this register is to speed up the execution */
DEVICE_PROP *pProperty;
/* Temporary save the state of Rx & Tx status. */
/* Whenever the Rx or Tx state is changed, its value is saved */
/* in this variable first and will be set to the EPRB or EPRA */
/* at the end of interrupt process */
u16	SaveState ;
u16  wInterrupt_Mask;
DEVICE_INFO	Device_Info;
USER_STANDARD_REQUESTS  *pUser_Standard_Requests;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

//初始化USB
void USB_Init(void)
{
	pInformation = &Device_Info;
	//typedef enum _CONTROL_STATE
	//{
	//	WAIT_SETUP,       /* 0 */
	//	SETTING_UP,       /* 1 */
	//	IN_DATA,          /* 2 */
	//	OUT_DATA,         /* 3 */
	//	LAST_IN_DATA,     /* 4 */
	//	LAST_OUT_DATA,    /* 5 */
	//	WAIT_STATUS_IN,   /* 7 */
	//	WAIT_STATUS_OUT,  /* 8 */
	//	STALLED,          /* 9 */
	//	PAUSE             /* 10 */
	//} CONTROL_STATE;    /* The state machine states of a control pipe */

	pInformation->ControlState = IN_DATA;

	pProperty = &Device_Property;
	pUser_Standard_Requests = &User_Standard_Requests;
	/* Initialize devices one by one */
	pProperty->Init();
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
