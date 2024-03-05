#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include "sccb.h"

/*
  * @brief  修改SCL的电平
  * @param  0或者1
  * @retval 无
*/
void SCCB_W_SCL(uint8_t BitValue)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)BitValue);
    delay_us(10);
}
/*
  * @brief  修改SDA的电平
  * @param  0或者1
  * @retval 无
*/
void SCCB_W_SDA(uint8_t BitValue)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_11, (BitAction)BitValue);
    delay_us(10);
}
/*
  * @brief  读取SDA的电平
  * @param  无
  * @retval 0或者1
*/
uint8_t SCCB_R_SDA(void)
{
    uint8_t BitValue;
    BitValue = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
    delay_us(10);
    return BitValue;
}
/*
  * @brief  SCCB初始化
  * @param  无
  * @retval 无
*/
void SCCB_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11);
}
/*
  * @brief  产生SCCB开始信号
  * @param  无
  * @retval 无
*/
void SCCB_Start(void)
{
    SCCB_W_SDA(1);
    delay_us(10);
    SCCB_W_SCL(1);
    delay_us(2);
    SCCB_W_SDA(0);
    delay_us(2);
    SCCB_W_SCL(0);
}
/*
  * @brief  产生SCCB结束信号
  * @param  无
  * @retval 无
*/
void SCCB_Stop(void)
{
    SCCB_W_SDA(0);
    SCCB_W_SCL(1);
    SCCB_W_SDA(1);
}
/*
  * @brief    SCCB发送一个字节
  * @param  一个字节数据
  * @retval 无
*/
void SCCB_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i ++)
    {    
        delay_us(2);
        SCCB_W_SDA(Byte & (0x80 >> i));
        delay_us(2);
        SCCB_W_SCL(1);
        delay_us(2);
        SCCB_W_SCL(0);
        delay_us(2);
    }
}
/*
  * @brief    SCCB接收一个字节
  * @param  无
  * @retval 接收到的字节
*/
uint8_t SCCB_ReceiveByte(void)
{
    uint8_t i, Byte = 0x00;
    SCCB_W_SDA(1);
    for (i = 0; i < 8; i ++)
    {
        SCCB_W_SCL(1);
        if (SCCB_R_SDA() == 1){Byte |= (0x80 >> i);}
        SCCB_W_SCL(0);
        delay_us(1);
    }
    return Byte;
}
/*
  * @brief    SCCB发送NA信号
  * @param  无
  * @retval 无
*/
void SCCB_SendNA()
{
    SCCB_W_SDA(1);
    SCCB_W_SCL(1);
    SCCB_W_SCL(0);
    SCCB_W_SDA(0);//new
}
/*
  * @brief    SCCB接收Ack应答
  * @param  无
  * @retval 接收到的应答，若数据成功发送，应答为0，反之为1
*/
uint8_t SCCB_ReceiveAck(void)
{
    uint8_t AckBit;
    SCCB_W_SDA(1);
    SCCB_W_SCL(1);
    AckBit = SCCB_R_SDA();
    SCCB_W_SCL(0);
    return AckBit;
}