#include "delay.h"
#include "usart.h"
#include "sys.h"
#include "usb_init.h"
#include "bf3003.h"

int main()
{
    SetSysClockToHSE();
    Delay_Init();
    Usart_Init();
    BF3003_Init();
    USB_Init();
    GPIOA->CRH &= 0X0FFFFFFF;
    GPIOA->CRH |= GPIO_CRH_MODE12_0;
    // GPIOB->CRH &= 0X0FFFFFFF;
    // GPIOB->CRH |= GPIO_CRH_MODE15_0;   // set pins to be general purpose output

    while (1) 
    {
        BF3003_Handle();
        // GPIOB->ODR ^= (1<<15);  // toggle diodes
        // printf("ack\n");

    }
}