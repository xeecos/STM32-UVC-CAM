#include "config.h"
#include "usb_init.h"
#include "usart.h"
#include "delay.h"
#include "bf3003.h"
int main()
{

    uart_init(72, 115200);
    delay_init(32);
    // USB_Init();
    // BF3003_Init();
    while(1)
    {
        printf("hello\n");
        delay_ms(500);
    }
    return 0;
}