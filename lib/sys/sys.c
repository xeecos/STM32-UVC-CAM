#include "sys.h" 

void SetSysClockToHSE(void)
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

    RCC->CFGR &= ~RCC_CFGR_PLLMULL; //Clear
    // RCC->CFGR &= ~RCC_CFGR_USBPRE; //USBclk=PLLclk/1.5=48Mhz
    
    /* HCLK = SYSCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1; 
    /* PCLK2 = HCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1; 
    /* PCLK1 = HCLK / 2 */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV2;

    // RCC->CR |= RCC_CR_HSEON;
    
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL18);
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

























