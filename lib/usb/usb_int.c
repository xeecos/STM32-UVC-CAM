/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : usb_int.c
* Author             : MCD Application Team
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : Endpoint CTR (Low and High) interrupt's service routines
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
uint16_t SaveRState;
uint16_t SaveTState;

#include "usb_desc.h"

#define CAMERA_SIZ_STREAMHD			0
extern uint8_t frame[2][640 * 1];
extern int8_t frameIdx;
extern uint16_t lineIdx;
uint8_t sendbuf[PACKET_SIZE];			// 发送数据缓冲区
uint32_t sendsize = 0;					// 已发送字节数
uint16_t frameSize = 640*1;	
uint16_t lastLineIdx;
// int8_t lastFrameId = -1;
void dataMemcpy(const uint8_t* src, uint8_t* dst, uint8_t len)
{
	uint8_t i = 0;
	for (i = 0; i < len; ++i)
	{
		dst[i] = src[i];
	}
}
void UsbCamera_SendImage(void)
{
	if(lineIdx<1)
	{
		sendsize = 0;
		sendbuf[0] = 0xff;
		sendbuf[1] = 0xff;
		ToggleDTOG_RX(ENDP1); 
		if (GetENDPOINT(ENDP1) & EP_DTOG_RX)
		{
			// User use buffer0
			UserToPMABufferCopy(sendbuf, ENDP1_BUF0Addr, 2);
			SetEPDblBuf0Count(ENDP1, EP_DBUF_IN, 2);
		} else{
			// User use buffer1
			UserToPMABufferCopy(sendbuf, ENDP1_BUF1Addr, 2);
			SetEPDblBuf1Count(ENDP1, EP_DBUF_IN, 2);
		}
		SetEPTxStatus(ENDP1, EP_TX_VALID);
		return;
	}
	uint8_t datalen = 0;
	frameIdx = 1-(lineIdx%2);
	if (sendsize==0)
	{
		datalen = PACKET_SIZE;
		sendsize = datalen;
		lastLineIdx = lineIdx;
	} 
	else if(sendsize<frameSize)
	{
		datalen = PACKET_SIZE;
		if (sendsize + datalen >= frameSize)
		{
			datalen = frameSize - sendsize;
		}
		sendsize += datalen;
	}
	ToggleDTOG_RX(ENDP1); 
	if (GetENDPOINT(ENDP1) & EP_DTOG_RX)
	{
		// User use buffer0
		UserToPMABufferCopy(frame[frameIdx] + sendsize - datalen, ENDP1_BUF0Addr, datalen);
		SetEPDblBuf0Count(ENDP1, EP_DBUF_IN, datalen);
	} else{
		// User use buffer1
		UserToPMABufferCopy(frame[frameIdx] + sendsize - datalen, ENDP1_BUF1Addr, datalen);
		SetEPDblBuf1Count(ENDP1, EP_DBUF_IN, datalen);
	}
	SetEPTxStatus(ENDP1, EP_TX_VALID);		// 允许数据发送
	if(sendsize>=frameSize)
	{
		if(lineIdx!=lastLineIdx)
		{
			sendsize = 0;
		}
	}
}
/* Extern variables ----------------------------------------------------------*/
// extern void(*pEpInt_IN[7])(void);    /*  Handles IN  interrupts   */
void (*pEpInt_IN[7])(void) =
{
	UsbCamera_SendImage,
	EP2_IN_Callback,
	EP3_IN_Callback,
	EP4_IN_Callback,
	EP5_IN_Callback,
	EP6_IN_Callback,
	EP7_IN_Callback,
};

extern void(*pEpInt_OUT[7])(void);   /*  Handles OUT interrupts   */

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : CTR_LP.
* Description    : Low priority Endpoint Correct Transfer interrupt's service
*                  routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CTR_LP(void)
{
	uint32_t wEPVal = 0;
	/* stay in loop while pending ints */
	while (((wIstr = _GetISTR()) & ISTR_CTR) != 0)
	{
		_SetISTR((uint16_t)CLR_CTR); /* clear CTR flag */
		/* extract highest priority endpoint number */
		EPindex = (uint8_t)(wIstr & ISTR_EP_ID);
		if (EPindex == 0)
		{
			/* Decode and service control endpoint interrupt */
			/* calling related service routine */
			/* (Setup0_Process, In0_Process, Out0_Process) */

			/* save RX & TX status */
			/* and set both to NAK */
			SaveRState = _GetEPRxStatus(ENDP0);
			SaveTState = _GetEPTxStatus(ENDP0);
			_SetEPRxStatus(ENDP0, EP_RX_NAK);
			_SetEPTxStatus(ENDP0, EP_TX_NAK);

			/* DIR bit = origin of the interrupt */

			if ((wIstr & ISTR_DIR) == 0)
			{
				/* DIR = 0 */
				/* DIR = 0      => IN  int */
				/* DIR = 0 implies that (EP_CTR_TX = 1) always  */

				_ClearEP_CTR_TX(ENDP0);
				In0_Process();

				/* before terminate set Tx & Rx status */
				_SetEPRxStatus(ENDP0, SaveRState);
				_SetEPTxStatus(ENDP0, SaveTState);
				return;
			} else
			{
				/* DIR = 1 */

				/* DIR = 1 & CTR_RX       => SETUP or OUT int */
				/* DIR = 1 & (CTR_TX | CTR_RX) => 2 int pending */

				wEPVal = _GetENDPOINT(ENDP0);
				if ((wEPVal & EP_CTR_TX) != 0)
				{
					_ClearEP_CTR_TX(ENDP0);
					In0_Process();
					/* before terminate set Tx & Rx status */
					_SetEPRxStatus(ENDP0, SaveRState);
					_SetEPTxStatus(ENDP0, SaveTState);
					return;
				} else if ((wEPVal &EP_SETUP) != 0)
				{
					_ClearEP_CTR_RX(ENDP0); /* SETUP bit kept frozen while CTR_RX = 1 */
					Setup0_Process();
					/* before terminate set Tx & Rx status */
					_SetEPRxStatus(ENDP0, SaveRState);
					_SetEPTxStatus(ENDP0, SaveTState);
					return;
				}

				else if ((wEPVal & EP_CTR_RX) != 0)
				{
					_ClearEP_CTR_RX(ENDP0);
					Out0_Process();
					/* before terminate set Tx & Rx status */
					_SetEPRxStatus(ENDP0, SaveRState);
					_SetEPTxStatus(ENDP0, SaveTState);
					return;
				}
			}
		}/* if(EPindex == 0) */
		else
		{
			/* Decode and service non control endpoints interrupt  */

			/* process related endpoint register */
			wEPVal = _GetENDPOINT(EPindex);
			printf("CTR_LP:%x %d\n",wEPVal, EPindex);
			if ((wEPVal & EP_CTR_RX) != 0)
			{
				/* clear int flag */
				_ClearEP_CTR_RX(EPindex);

				/* call OUT service function */
				(*pEpInt_OUT[EPindex - 1])();

			} /* if((wEPVal & EP_CTR_RX) */

			if ((wEPVal & EP_CTR_TX) != 0)
			{
				/* clear int flag */
				_ClearEP_CTR_TX(EPindex);
				/* call IN service function */
				(*pEpInt_IN[EPindex - 1])();
			} /* if((wEPVal & EP_CTR_TX) != 0) */

		}/* if(EPindex == 0) else */

	}/* while(...) */
}

/*******************************************************************************
* Function Name  : CTR_HP.
* Description    : High Priority Endpoint Correct Transfer interrupt's service
*                  routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CTR_HP(void)
{
	uint32_t wEPVal = 0;

	while (((wIstr = _GetISTR()) & ISTR_CTR) != 0)
	{
		_SetISTR((uint16_t)CLR_CTR); /* clear CTR flag */
		/* extract highest priority endpoint number */
		EPindex = (uint8_t)(wIstr & ISTR_EP_ID);
		/* process related endpoint register */
		wEPVal = _GetENDPOINT(EPindex);
		if ((wEPVal & EP_CTR_RX) != 0)
		{
			/* clear int flag */
			_ClearEP_CTR_RX(EPindex);

			/* call OUT service function */
			(*pEpInt_OUT[EPindex - 1])();

		} /* if((wEPVal & EP_CTR_RX) */
		else if ((wEPVal & EP_CTR_TX) != 0)
		{
			/* clear int flag */
			_ClearEP_CTR_TX(EPindex);

			/* call IN service function */
			(*pEpInt_IN[EPindex - 1])();


		} /* if((wEPVal & EP_CTR_TX) != 0) */

	}/* while(...) */
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
