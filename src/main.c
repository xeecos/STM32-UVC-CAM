#include "delay.h"
#include "usart.h"
#include "sys.h"
#include "stm32f1xx.h"
#include "usb_init.h"
#include "hw_config.h"
#include "stm32f1xx_ll_exti.h"
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

// int main()
// {

//     RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;              // enable the clock to GPIO
//     GPIOA->CRH &= 0X0FFFFFFF;
//     GPIOA->CRH |= GPIO_CRH_MODE12_0;
//     MY_NVIC_PriorityGroupConfig(2);

//     SetSysClockToHSE();
//     SystemCoreClockUpdate();
//     printf("SystemCoreClock:%ld\n",SystemCoreClock);
//     delay_init();

//     usart_init();
// 	// USB配置
//     Set_USBClock();
//     USB_Init();
//     USB_Interrupts_Config();
    
//     // GPIOB->CRH &= 0X0FFFFFFF;
//     // GPIOB->CRH |= GPIO_CRH_MODE15_0;   // set pins to be general purpose output

//     while (1) 
//     {
//         delay_ms(1000);
//         // GPIOB->ODR ^= (1<<15);  // toggle diodes
//         printf("ack\n");

//     }
// }

void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState        = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue  = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
  
    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
        clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}
int main()
{
    HAL_Init();
    SystemClock_Config();

    usart_init();

    Set_USBClock();
    USB_Init();
    // USB_Interrupts_Config();
    LL_EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.Line_0_31 = EXTI_LINE_18;
	EXTI_InitStruct.Mode = EXTI_MODE_INTERRUPT;
	EXTI_InitStruct.Trigger = EXTI_TRIGGER_RISING;	//上升沿
	EXTI_InitStruct.LineCommand = ENABLE;
	LL_EXTI_Init(&EXTI_InitStruct);

    HAL_NVIC_SetPriorityGrouping(2);
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);

    HAL_NVIC_SetPriorityGrouping(2);
    HAL_NVIC_SetPriority(USBWakeUp_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USBWakeUp_IRQn);

    while (1) 
    {
        delay_ms(1000);
        printf("ack\n");
    }
    return 0;
}