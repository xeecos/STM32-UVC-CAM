/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : usb_istr.c
* Author             : MCD Application Team
* Version            : V2.2.0
* Date               : 06/13/2008
* Description        : ISTR events interrupt service routines
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
#include "usb_prop.h"
#include "usb_pwr.h"
#include "usb_istr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint16_t wIstr;  /* ISTR register last read value */
volatile uint8_t bIntPackSOF = 0;  /* SOFs received between 2 consecutive packets */

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* function pointers to non-control endpoints service routines */

/* Extern variables ----------------------------------------------------------*/
// extern void(*pEpInt_IN[7])(void);    /*  Handles IN  interrupts   */
void (*pEpInt_IN[7])(void) =
{
	EP1_IN_Callback,
	EP2_IN_Callback,
	EP3_IN_Callback,
	EP4_IN_Callback,
	EP5_IN_Callback,
	EP6_IN_Callback,
	EP7_IN_Callback,
};
void (*pEpInt_OUT[7])(void) =
{
	EP1_OUT_Callback,
	EP2_OUT_Callback,
	EP3_OUT_Callback,
	EP4_OUT_Callback,
	EP5_OUT_Callback,
	EP6_OUT_Callback,
	EP7_OUT_Callback,
};


/*******************************************************************************
* Function Name  : USB_Istr
* Description    : STR events interrupt service routine
* Input          :
* Output         :
* Return         :
*******************************************************************************/
void USB_Istr (void)
{
	wIstr = _GetISTR ();

#if (IMR_MSK & ISTR_RESET)			// USB复位请求中断
	if (wIstr & ISTR_RESET & wInterrupt_Mask)
	{
		_SetISTR ((uint16_t)CLR_RESET);	// 清除复位中断标志
		Device_Property.Reset ();	// 进入到复位中断( Joystick_Reset )
#ifdef RESET_CALLBACK
		RESET_Callback ();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_DOVR)
	if (wIstr & ISTR_DOVR & wInterrupt_Mask)
	{
		_SetISTR ((uint16_t)CLR_DOVR);
#ifdef DOVR_CALLBACK
		DOVR_Callback ();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_ERR)
	if (wIstr & ISTR_ERR & wInterrupt_Mask)
	{
		_SetISTR ((uint16_t)CLR_ERR);
#ifdef ERR_CALLBACK
		ERR_Callback ();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_WKUP)
	if (wIstr & ISTR_WKUP & wInterrupt_Mask)
	{
		_SetISTR ((uint16_t)CLR_WKUP);
		Resume (RESUME_EXTERNAL);
#ifdef WKUP_CALLBACK
		WKUP_Callback ();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_SUSP)
	if (wIstr & ISTR_SUSP & wInterrupt_Mask)
	{

		/* check if SUSPEND is possible */
		if (fSuspendEnabled)
		{
			Suspend ();
		} else
		{
			/* if not possible then resume after xx ms */
			Resume (RESUME_LATER);
		}
		/* clear of the ISTR bit must be done after setting of CNTR_FSUSP */
		_SetISTR ((uint16_t)CLR_SUSP);
#ifdef SUSP_CALLBACK
		SUSP_Callback ();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_SOF)
	if (wIstr & ISTR_SOF & wInterrupt_Mask)
	{
		_SetISTR ((uint16_t)CLR_SOF);
		bIntPackSOF++;

#ifdef SOF_CALLBACK
		SOF_Callback ();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_ESOF)
	if (wIstr & ISTR_ESOF & wInterrupt_Mask)
	{
		_SetISTR ((uint16_t)CLR_ESOF);
		/* resume handling timing is made with ESOFs */
		Resume (RESUME_ESOF); /* request without change of the machine state */

#ifdef ESOF_CALLBACK
		ESOF_Callback ();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_CTR)			// 正确传输中断
	if (wIstr & ISTR_CTR & wInterrupt_Mask)
	{
		/* servicing of the endpoint correct transfer interrupt */
		/* clear of the CTR flag into the sub */
		CTR_LP ();
#ifdef CTR_CALLBACK
		CTR_Callback ();
#endif
	}
#endif
} /* USB_Istr */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

