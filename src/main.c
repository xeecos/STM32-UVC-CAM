// // #include "stm32f1xx.h"
// // #include "config.h"
// // #include "sys.h"
// // #include "usb_init.h"
// // #include "usart.h"
// // #include "delay.h"
// // #include "bf3003.h"

// // int main()
// // {
// //     usart_init();
// //     write('i');
// //     write('\n');
// //     delay_init(72);
// //     // USB_Init();
// //     // BF3003_Init();
// //     write('i');
// //     write('\n');
// //     while(1)
// //     {
// //         printf("hi!\n");
// //         delay_ms(500);
// //     }
// //     return 0;
// // }
// #include <Arduino.h>

// #include "usbd_video.h"
// #include "usbd_core.h"
// #include "usb_init.h"
// #include "serial.h"
#include "delay.h"
#include "usart.h"
#include "stm32f1xx.h"
#include "usb_init.h"
#include "hw_config.h"
static void SetSysClockToHSE(void)
{
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;
  
  /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/    
  /* Enable HSE */    
  RCC->CR |= ((uint32_t)RCC_CR_HSEON);
 
  /* Wait till HSE is ready and if Time out is reached exit */
  do
  {
    HSEStatus = RCC->CR & RCC_CR_HSERDY;
    StartUpCounter++;  
  } while((HSEStatus == 0) && (StartUpCounter != 0x0500));

  if ((RCC->CR & RCC_CR_HSERDY) != RESET)
  {
    HSEStatus = (uint32_t)0x01;
  }
  else
  {
    HSEStatus = (uint32_t)0x00;
  }  

  if (HSEStatus == (uint32_t)0x01)
  {

    #if !defined STM32F10X_LD_VL && !defined STM32F10X_MD_VL && !defined STM32F10X_HD_VL
        /* Enable Prefetch Buffer */
        FLASH->ACR |= FLASH_ACR_PRFTBE;

        /* Flash 0 wait state */
        FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);

    #ifndef STM32F10X_CL
        FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_0;
    #else
        if (HSE_VALUE <= 24000000)
        {
        FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_0;
        }
        else
        {
        FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_1;
        }
    #endif /* STM32F10X_CL */
    #endif

    /* HCLK = SYSCLK */
    RCC->CFGR &= ~RCC_CFGR_PLLMULL_Msk; //Clear
    RCC->CFGR |= (0x7UL << RCC_CFGR_PLLMULL_Pos);

	RCC->CFGR &= ~RCC_CFGR_USBPRE; //USBclk=PLLclk/1.5=48Mhz
    RCC->CFGR &= ~RCC_CFGR_HPRE;
    //APB1 divider=2
    RCC->CFGR &= ~RCC_CFGR_PPRE1;
    RCC->CFGR |= RCC_CFGR_PPRE1_2;
    //APB2 divider=1
    RCC->CFGR &= ~RCC_CFGR_PPRE2;
    //ADC prescalar = 12
    RCC->CFGR &= ~RCC_CFGR_ADCPRE;
    RCC->CFGR |= RCC_CFGR_ADCPRE_1;

    RCC->CR |= RCC_CR_HSEON;
    RCC->CFGR |= RCC_CFGR_PLLSRC;
    /* Wait till PLL is ready */
    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;
    while((RCC->CR & RCC_CR_PLLRDY) == 0)
    {
    }

    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_1;
    //Wait until PLL system source is active
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1);
  }
  else
  { /* If HSE fails to start-up, the application will have wrong clock 
         configuration. User can add here some code to deal with this error */
         printf("HSE fails\n");
  }  
}

int main()
{
    SetSysClockToHSE();
    
    delay_init();

    usart_init();
	// USB配置
	Set_USBClock();
	USB_Init();
    USB_Interrupts_Config();

    // RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;              // enable the clock to GPIO
    // GPIOB->CRH &= 0X0FFFFFFF;
    // GPIOB->CRH |= GPIO_CRH_MODE15_0;   // set pins to be general purpose output

    while (1) 
    {
        delay_ms(1000);
        // GPIOB->ODR ^= (1<<15);  // toggle diodes
        printf("ack\n");

    }
}

// void setup()
// {
//     serial_init();
//     
//     // delay(1000);
//     // USB_Interrupts_Config();
//     // Set_USBClock();
//     // USB_Init();
//     // pinMode(PB15,OUTPUT);
// }
// void loop()
// {
//     // digitalWrite(PB15,LOW);
//     delay(1000);
//     uart_log("ack");
// }