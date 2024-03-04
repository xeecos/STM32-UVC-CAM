#include "usb_init.h"
#include "usart.h"
#include "delay.h"
int main()
{
    uart_init(36, 115200);
    delay_init(32);
    USB_Init();
    while(1)
    {
        printf("hello\n");
        delay_ms(500);
    }
    return 0;
}