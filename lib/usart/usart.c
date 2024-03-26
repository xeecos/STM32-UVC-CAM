#include "usart.h"
#include "stm32f10x.h"
#include "sys.h"

int _write(int fd, char *ptr, int len)  
{  
  for(int i=0;i<len;i++)
  {
	while ((USART1->SR & 0X40) == 0);
	USART1->DR = ptr[i];
  }
  return len;
}
// 串口1中断服务程序
// 注意,读取USARTx->SR能避免莫名其妙的错误
uint8_t USART_RX_BUF[USART_REC_LEN]; // 接收缓冲,最大USART_REC_LEN个字节.
// 接收状态
// bit15，	接收完成标志
// bit14，	接收到0x0d
// bit13~0，	接收到的有效字节数目
uint16_t USART_RX_STA = 0; // 接收状态标记

void USART1_IRQHandler(void)
{
	uint8_t res;
#ifdef OS_CRITICAL_METHOD // 如果OS_CRITICAL_METHOD定义了,说明使用ucosII了.
	OSIntEnter();
#endif
	if (USART1->SR & (1 << 5)) // 接收到数据
	{
		res = USART1->DR;
		if ((USART_RX_STA & 0x8000) == 0) // 接收未完成
		{
			if (USART_RX_STA & 0x4000) // 接收到了0x0d
			{
				if (res != 0x0a)
					USART_RX_STA = 0; // 接收错误,重新开始
				else
					USART_RX_STA |= 0x8000; // 接收完成了
			}
			else // 还没收到0X0D
			{
				if (res == 0x0d)
					USART_RX_STA |= 0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA & 0X3FFF] = res;
					USART_RX_STA++;
					if (USART_RX_STA > (USART_REC_LEN - 1))
						USART_RX_STA = 0; // 接收数据错误,重新开始接收
				}
			}
		}
	}
#ifdef OS_CRITICAL_METHOD // 如果OS_CRITICAL_METHOD定义了,说明使用ucosII了.
	OSIntExit();
#endif
}
void Usart_Init()
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;  
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	uint16_t mantissa;
	uint16_t fraction;	   
	float temp = (float)(8*9*1000000)/(115200*16);//得到USARTDIV
	mantissa = temp;				 //得到整数部分
	fraction = (temp-mantissa)*16; //得到小数部分	 
    mantissa <<= 4;
	mantissa += fraction; 
	// RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;   //使能PORTA口时钟  
	GPIOA->CRH &= 0XFFFFF00F;//IO状态设置
	GPIOA->CRH |= 0X000008B0;//IO状态设置
		  
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;  //使能串口时钟 
	RCC->APB2RSTR |= RCC_APB2ENR_USART1EN;   //复位串口1
	RCC->APB2RSTR &= ~RCC_APB2ENR_USART1EN;//停止复位	   	   
	//波特率设置
 	USART1->BRR = mantissa; // 波特率设置	 
	//使能接收中断
	USART1->CR1 |= 1<<8;
	USART1->CR1 |= 1<<5;   //RXNE 接收缓冲区非空中断使能	
	USART1->CR1 |= 0X200C;  //1位停止,无校验位.
	// MY_NVIC_Init(1, 0, USART1_IRQn, 2);//组2，最低优先级 
	NVIC_InitTypeDef nvic_init;
    nvic_init.NVIC_IRQChannel = USART1_IRQn;
    nvic_init.NVIC_IRQChannelPreemptionPriority = 1;
    nvic_init.NVIC_IRQChannelSubPriority = 0;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);
}
