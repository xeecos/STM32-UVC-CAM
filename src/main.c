#include "delay.h"
#include "usart.h"
#include "sys.h"
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
        FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
        FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_1;
    #endif

    RCC->CFGR &= ~RCC_CFGR_PLLMULL_Msk; //Clear

    // RCC->CFGR &= ~RCC_CFGR_USBPRE; //USBclk=PLLclk/1.5=48Mhz
    
    /* HCLK = SYSCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1; 
    /* PCLK2 = HCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1; 
    /* PCLK1 = HCLK / 2 */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV2;

    // RCC->CR |= RCC_CR_HSEON;
    
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9);
    /* Wait till PLL is ready */
    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;
    while((RCC->CR & RCC_CR_PLLRDY) == 0)
    {
    }

    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    //Wait until PLL system source is active
    while((RCC->CFGR & RCC_CFGR_SWS) != 0x8);
  }
  else
  { /* If HSE fails to start-up, the application will have wrong clock 
         configuration. User can add here some code to deal with this error */
         printf("HSE fails\n");
  }  
}

int main()
{
    MY_NVIC_PriorityGroupConfig(2);

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