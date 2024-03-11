#include "delay.h"
#include "usart.h"
#include "sys.h"
#include "usb_init.h"
#include "hw_config.h"

int main()
{

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;              // enable the clock to GPIO
    GPIOA->CRH &= 0X0FFFFFFF;
    GPIOA->CRH |= GPIO_CRH_MODE12_0;
    MY_NVIC_PriorityGroupConfig(2);
    Stm32_Clock_Init(9);
    // SetSysClockToHSE();
    // SystemCoreClockUpdate();
    // printf("SystemCoreClock:%ld\n",SystemCoreClock);
    delay_init();

    usart_init();
	// USB配置
    Set_USBClock();
    USB_Init();
    USB_Interrupts_Config();
    
    // GPIOB->CRH &= 0X0FFFFFFF;
    // GPIOB->CRH |= GPIO_CRH_MODE15_0;   // set pins to be general purpose output

    while (1) 
    {
        delay_ms(1000);
        // GPIOB->ODR ^= (1<<15);  // toggle diodes
        printf("ack\n");

    }
}

/* HAL */
// void SystemClock_Config(void)
// {
//     RCC_ClkInitTypeDef RCC_ClkInitStruct;
//     RCC_OscInitTypeDef RCC_OscInitStruct;

//     /* Enable HSE Oscillator and activate PLL with HSE as source */
//     RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
//     RCC_OscInitStruct.HSEState        = RCC_HSE_ON;
//     RCC_OscInitStruct.HSEPredivValue  = RCC_HSE_PREDIV_DIV1;
//     RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//     RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
//     RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
//     HAL_RCC_OscConfig(&RCC_OscInitStruct);
  
//     /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
//         clocks dividers */
//     RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
//     RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
//     RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
//     RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
//     RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
//     HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
// }
// #include "stm32f1xx_ll_exti.h"
// int main()
// {
//     HAL_Init();
//     SystemClock_Config();

//     usart_init();

//     Set_USBClock();
//     USB_Init();
//     // USB_Interrupts_Config();
//     LL_EXTI_InitTypeDef EXTI_InitStruct;
//     EXTI_InitStruct.Line_0_31 = EXTI_LINE_18;
// 	EXTI_InitStruct.Mode = EXTI_MODE_INTERRUPT;
// 	EXTI_InitStruct.Trigger = EXTI_TRIGGER_RISING;	//上升沿
// 	EXTI_InitStruct.LineCommand = ENABLE;
// 	LL_EXTI_Init(&EXTI_InitStruct);

//     HAL_NVIC_SetPriorityGrouping(2);
//     HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 1, 0);
//     HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);

//     HAL_NVIC_SetPriorityGrouping(2);
//     HAL_NVIC_SetPriority(USBWakeUp_IRQn, 0, 0);
//     HAL_NVIC_EnableIRQ(USBWakeUp_IRQn);

//     while (1) 
//     {
//         delay_ms(1000);
//         printf("ack\n");
//     }
//     return 0;
// }
/* SPL */
// #include "stm32f10x.h"
// #define RCC_GPIOC RCC_APB2Periph_GPIOC
// #define LEDBUILDIN GPIO_Pin_13
// #define LEDPORT GPIOC
// int main(void)
// {
// 	RCC_APB2PeriphClockCmd(RCC_GPIOC, ENABLE);

// 	GPIO_InitTypeDef GPIO_InitStructure;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
// 	GPIO_InitStructure.GPIO_Pin = LEDBUILDIN;
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(LEDPORT, &GPIO_InitStructure);


// 	while (1)
// 	{
// 		GPIO_ResetBits(LEDPORT, LEDBUILDIN);
// 	}
// }