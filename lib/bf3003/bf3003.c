#include "stm32f10x.h" // Device header
// #include "stm32f1xx_hal.h"
#include "sccb.h"
#include "delay.h"
#include "usart.h"
#include "bf3003.h"

/* 0xDC --- 写地址   ***   0XDD ---读地址 */
#define BF3003_ADDRESS      0xDC

#define BF3003_PID_BME              0xFC
#define BF3003_VER_BME              0xFD

#define REGS_COUNT	38
uint8_t regs[REGS_COUNT][2] = {
	{BF3003_COM7, 0b10000000},
	{BF3003_COM2, 0b00000001},
	/*
	Common control 2
	Bit[7:6]: vclk output drive capability
		00:1x 01:1.5x 10:2.5x 11:3x
	Bit[5]:Tri-state option for output data at power down period
		0:tri-state at this period
		1:No tri-state at this period
	Bit[4]:Tri-state option for output clock at power down period
		0:tri-state at this period
		1:No tri-state at this period
	Bit[3:2]: hsync output drive capability
		00:1x 01:1.5x 10:2.5x 11:3x
	when drivesel=0
	Bit[1:0]: data&clk&Hsync output drive capability
		00:1x 01:1.5x 10:2.5x 11:3x
	when drivesel=1
		Bit[1:0]: data output drive capability
		00:1x 01:1.5x 10:2.5x 11:3x
	*/
	{BF3003_COM3, 0b00000000},
	/*
	Bit[7]:PROCRSS RAW selection
		0: process raw from ycbcr to rgb conversion in datformat
		1: process raw from color interpolation(deniose,gamma,lsc is selectable)
	Bit[6]:Output data MSB and LSB swap
	Bit[5:4]:PROCESS RAW sequence(when 0x0c[7]=0):
		00: (LINE0:BGBG/LINE1:GRGR)
		01: (LINE0:GBGB/LINE1:RGRG)
		10: (LINE0:GRGR/LINE1:BGBG)
		11: (LINE0:RGRG/LINE1:GBGB)
	Bit[3]:
		0:no HREF when VSYNC_DAT=0;
		1:always has HREF no matter VSYNC_DAT=0 or not;
	Bit[2]:DATA ahead 1 clk(YUV MCLK,RawData PCLK) or not
	Bit[1]:HREF ahead 1 clk(YUV MCLK,RawData PCLK) or not
	Bit[0]:HREF ahead 0.5 clk(YUV MCLK,RawData PCLK) or not
	0x0c[1:0]: Internal use only
	*/
	{BF3003_CLKRC, 0x0},
	/*
	Mclk_div control
	Bit[7：2]: Internal use only
	Bit[1:0]:Internal MCLK pre-scalar
		00:divided by 1 F(MCLK)=F(pll output clock)
		01:divided by 2 F(MCLK)=F(pll output clock)/2
		10:divided by 4 F(MCLK)=F(pll output clock)/4
		11: no clocking, digital stand by mode(all clocks freeze)
	*/
	{BF3003_COM7, 0b00000100},
	/*
		Bit[7]: SCCB Register Reset
			0: No change
			1: Resets all registers to default values 
		Bit[6]: Reserved
		Bit[5]: (when 0x4a =03h)0: row 1/2 sub,1: output input image.
		Bit[4]: 1/2 digital subsample Selection(only for YUV422/RGB565/RGB555/RGB444 output).
		Bit[3]: data selection
			0:normal(YUV422/RGB565/RGB555/RGB444/BAYER RAW/PRO RAW)
			1:CCIR656 output enable(for TV)
		Bit[2]: YUV422/RGB565/RGB555/RGB444 Selection.
		Bit[1]: Reserved.
		Bit[0]: Raw RGB Selection.
			{0x12[2],0x12[0]}
			00: YUV422
			01: Bayer RAW
			10: RGB565/RGB555/RGB444(use with 0x3a) 
			11: Process RAW (use with 0x0c[7]) 
 	*/
	{BF3003_TSLB, 0b00000001},
	/*
		if YUV422 is selected,the Sequence is:
		Bit[1:0]:Output YUV422 Sequence
		00: YUYV, 01: YVYU
		10: UYVY, 11: VYUY
		if RGB565/RGB555/RGB444 is selected,the Sequence is:
		Bit[4:0]:Output RGB565/RGB555/RGB444 Sequence
		RGB565:
		00h: R5G3H,G3LB5 01h: B5G3H,G3LR5
		02h: B5R3H,R2LG6 03h: R5B3H,B2LG6
		04h: G3HB5,R5G3L 05h: G3LB5,R5G3H
		06h: G3HR5,B5G3L 07h: G3LR5,B5G3H
		08h: G6B2H,B3LR5 09h: G6R2H,R3LB5
	*/
	{BF3003_COM8, 0b00001111},
	/*
		Auto mode Contrl
		Bit[7:6] reserved
		Bit[5:4]：Sensitivity enable,
		Bit[5]: 0:manual adjust , 1 :auto adjust
		Bit[4]: when manual adjust,write 1, high sensitivity
		write 0, low sensitivity
		Bit[4]: select which gain to be used,when short int_tim
		adjust:
		0: use glb_gain_short
		1: use glb_gain
		Bit[3]: Reserved.
		Bit[2]: AGC Enable. 0:OFF , 1: ON.
		Bit[1]: AWB Enable. 0:OFF , 1: ON.
		Bit[0]: AEC Enable. 0:OFF , 1: ON. 
	*/
	{BF3003_COM10, 0b00000010},
	/* BF3003_COM10
		Bit[7]: Reserved
		Bit[6]: 0:HREF, 1:HSYNC
		Bit[5]: 0:VSYNC_IMAGE, 1:VSYNC_DAT
		Bit[4]: VCLK reverse
		Bit[3]: HREF option, 0:active high, 1:active low.
		Bit[2]: Reserved
		Bit[1]: VSYNC option, 0:active low, 1:active high.
		Bit[0]: HSYNC option, 0:active high, 1:active low.
	*/
	{BF3003_VHREF, 0b0},
	{BF3003_HSTART, 0x2},
	{BF3003_HSTOP, 0xA2},
	{BF3003_VSTART, 0x0},
	{BF3003_VSTOP, 0x78},
	{BF3003_PLLCTL, 0x2A},
	{BF3003_EXHCH,0x02},
	{BF3003_EXHCL,0xA0},
	{BF3003_AE_MODE, 0b11000000},
	/*
	Bit[7]: AE mode select:
	0: use Y (from color space module).
	1: use rawdata (from gamma module), (when special effect in color interpolation module is selected,0x80[7] must set
	to be 1'b1)
	Bit[6]: INT_TIM lower than INT_STEP_5060 or not:
	0: limit int_tim>=step(no flicker)
	1: int_tim can be less than 1*int_step(existing flicker).
	Bit[5:4]: center window select:
	vga and ntsc mode 00: 512*384(full) ,256*192(1/2sub when normal mode)
	and ntsc have no sub 
	01: 384*288(full) ,192*144(1/2sub when normal mode)
	10 : 288*216(full) ,144*108(1/2sub when normal mode)
	11: 216*160(full) ,108*80 (1/2sub when normal mode)
	pal mode 00: 512*448
	01: 384*336
	10: 288*256
	11: 216*192
	Bit[3:1]: weight select: weight_sel region1 region2 region3 region4
	000: 1/4 1/4 1/4 1/4
	001: 1/2 1/4 1/8 1/8
	010: 5/8 1/8 1/8 1/8
	011: 3/8 3/8 1/8 1/8
	100: 3/4 1/4 0 0
	101: 5/8 3/8 0 0
	110: 1/2 1/2 0 0
	111: 1 0 0 0
	Bit[0]: Banding filter value select
	0: Select {0x89[5],0x9E[7:0]} as Banding Filter Value.
	1: Select {0x89[4],0x9D[7:0]} as Banding Filter Value
	*/
	{BF3003_TEST_MODE, 0b00000000},
	/*
	BIT[7] : 
		1: test pattern enable
		0: bypass test pattern
	BIT[6:5]: 
		00: output color bar pattern
		01: output gradual pattern
		1x: output manual write R/G/B
	BIT[4] : 0:vertical pattern, 1:horizontal pattern
	BIT[3:0]: gradual gray pattern mode control 
	*/
	{BF3003_MODE_SEL, 0b0},
	{BF3003_SUBSAMPLE, 0b0},
	{BF3003_BLUE_GAIN, 0x19},
	{BF3003_RED_GAIN, 0x15},
	{BF3003_GREEN_GAIN, 0x33},
	{BF3003_DICOM1, 0x80},
	/*Bit[7]: YCBCR RANGE select
		0: YCBCR 0~255
		1: Y 16~235, CBCR 16~240
	Bit[6]: Negative image enable
		0: Normal image, 1: Negative image
	Bit[5]: UV output value select.
		0: output normal value
		1: output fixed value set in MANU and MANV
	Bit[4]:U、V dither when ycbcr mode/R、B dither when rgb
	mode:
		0: low 2 bits, 1: low 3bits
	Bit[3]:Y dither when ycbcr mode/G dither when rgb mode:
		0: low 2 bits, 1: low 3bits
	Bit[2]:Y dither enable
	Bit[1]:U、V dither enable
	Bit[0]:RGB dither enable 
	*/
	{BF3003_INT_MEAN_H, 0x32},
	{BF3003_INT_TIM_MIN, 0x0},
	{BF3003_INT_TIM_HI, 0x00},
	{BF3003_INT_TIM_LO, 0x06},
	{BF3003_INT_TIM_MAX_HI, 0xff},
	{BF3003_INT_TIM_MAX_LO, 0xff},
	{0, 0},
};
/* @brief 下面几个函数主要是用于便捷读取串口电平 */
uint8_t BF3003_VS(void)
{
    return 1;//GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_12);
}
uint8_t BF3003_HREF(void)
{
    return 1;//GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_13);
}
uint8_t BF3003_PCLK(void)
{
    return 1;//GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_14);
}
/*
 * @brief    BF3003引脚初始化函数，在BF3003_Init() 里触发，不需外部调用
 * @param  无
 * @retval 无
 */
void BF3003_Pin_Init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStruct;  
    /* XCLK初始化 */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;  
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST,ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    TIM_TimeBaseInitStructure.TIM_Prescaler = 18 - 1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 4 - 1;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    // 72000000  / (TIM_Period + 1) / (TIM_Prescaler + 1)
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 2;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;

    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    TIM_Cmd(TIM3, ENABLE);

    /* VSYNC HREF PIXCLK初始化 */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* D0-D7 IO口初始化 */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}
/*
 * @brief    BF3003写寄存器
 * @param  写入寄存器的地址
 * @param  写入的内容（一个字节数据）
 * @retval 无
 */
void BF3003_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    SCCB_Start();
    SCCB_SendByte(BF3003_ADDRESS);
    SCCB_ReceiveAck(); // SCCB_ReceiveAck()==0说明上一步执行成功，下面的两个也是

    SCCB_SendByte(RegAddress);
    SCCB_ReceiveAck();

    SCCB_SendByte(Data);
    SCCB_ReceiveAck();

    SCCB_Stop();
}
/*
 * @brief    BF3003读取寄存器
 * @param  读取寄存器的地址
 * @retval 相应地址的寄存器的数据
 */
uint8_t BF3003_ReadReg(uint8_t RegAddress)
{
    uint8_t Data;

    SCCB_Start();
    SCCB_SendByte(BF3003_ADDRESS);
    SCCB_ReceiveAck(); // SCCB_ReceiveAck()==0说明上一步执行成功，下面的两个也是
    SCCB_SendByte(RegAddress);
    SCCB_ReceiveAck();
    SCCB_Stop(); // 这里的STOP很必要！！！！是SCCB不同于I2C的地方！！！

    SCCB_Start();
    SCCB_SendByte(BF3003_ADDRESS | 0x01);
    SCCB_ReceiveAck();
    Data = SCCB_ReceiveByte();
    SCCB_SendNA();
    SCCB_Stop();

    return Data;
}
/*
 * @brief    寄存器初始化
 */
void BF3003_Configure(void)
{
    uint8_t ver = 0;
	while (1)
	{
		ver = BF3003_ReadReg(BF3003_PID_BME);
		printf("BF3003 Probe:%x\n", ver);
		if (ver == 0x30)
		{
			break;
		}
		Delay_Ms(1000);
	}
	for (int i = 0; i < REGS_COUNT; i++)
	{
		if (regs[i][0] == 0)
			break;
		BF3003_WriteReg(regs[i][0], regs[i][1]);
		Delay_Ms(10);
	}
}
/*
 * @brief    BF3003初始化
 * @param  无
 * @retval 无
 */
void BF3003_Init(void)
{
    BF3003_Pin_Init();  // 引脚初始化
    SCCB_Init();        // SCCB初始化
    BF3003_Configure(); // 寄存器预设
}

void BF3003_Handle(void)
{
    // BF3003_GetPic();
}
/*
 * @brief    读取图像信息并发送至电脑显示
 * @param  无
 * @retval 无
 */
uint8_t frame[320 * 40];
void BF3003_GetPic(void)
{
    uint16_t i, j;
    while (BF3003_VS() == 0)
        ; /* 保证进入一个新的帧时序，而不是在帧时序的一半进入 */
    while (BF3003_VS() == 1)
        ;
    for (i = 0; i < 240; i++)
    {
        while (BF3003_HREF() == 0)
            ;
        for (j = 0; j < 320 * 2; j++)
        {
            while (BF3003_PCLK() == 0)
                ;
            frame[640 * i + j] = (GPIOB->IDR >> 8)&0xff;
            while (BF3003_PCLK() == 1)
                ;
        }
        if (i == 19)
            break;
        while (BF3003_HREF() == 1)
            ;
    }
    while (BF3003_VS() == 0)
        ;

    // Serial_SendByte(0x01);
    // Serial_SendByte(0xFE);
    // for (i = 0; i < 320 * 40; i++)
    // {
    //     Serial_SendByte(frame[i]);
    // }
    // Serial_SendByte(0xFE);
    // Serial_SendByte(0x01);

    Delay_Ms(1000);
}