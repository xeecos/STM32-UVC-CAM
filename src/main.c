#include "delay.h"
#include "usart.h"
#include "sys.h"
#include "bf3003.h"
#include "usb_init.h"

int main()
{
    SetSysClockToHSE();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    MY_NVIC_PriorityGroupConfig(2);
    Delay_Init();
    Usart_Init();
    BF3003_Init();
    USB_Init();

    while (1) 
    {
        BF3003_Handle();
    }
}