#include "delay.h"
#include "usart.h"
#include "sys.h"
#include "bf3003.h"
#include "usb_init.h"

int main()
{
    SetSysClockToHSE();
    Delay_Init();
    Usart_Init();
    BF3003_Init();
    USB_Init();

    while (1) 
    {
        // BF3003_Handle();
    }
}