#include "stm32f10x.h" // Device header
#include "stm32f1xx_hal.h"
#include "sccb.h"
#include "delay.h"
#include "usart.h"
#include "bf3003.h"

/* 0xDC --- 写地址   ***   0XDD ---读地址 */
#define BF3003_ADDRESS      0xDC

#define BF3003_CHIP_VERSION			0x3000
#define BF3003_CHIP_ID				0x2402

uint8_t regs[] = {
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
    return GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_12);
}
uint8_t BF3003_HREF(void)
{
    return GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_13);
}
uint8_t BF3003_PCLK(void)
{
    return GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_14);
}
/*
 * @brief    BF3003引脚初始化函数，在BF3003_Init() 里触发，不需外部调用
 * @param  无
 * @retval 无
 */
void BF3003_Pin_Init()
{
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /* VS HREF 和 PCLK IO口初始化 */
    GPIO_InitTypeDef GPIO_InitStruct;                                   /* 结构体定义 */
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14; /* PB12,PB13,PB14 */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                          /* 上拉输入 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;                      /*  50MHz*/
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    /* D0-D7 IO口初始化 */

    GPIO_InitStruct.Pin = 0xFF00;                     /* PB8-PB15 */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;           /* 上拉输入 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;     /*  50MHz*/
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
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
 * @brief 参照其他教程的寄存器设置
 */
void BF3003_RegExample(void)
{
    BF3003_WriteReg(0x3a, 0x04);
    BF3003_WriteReg(0x40, 0xd0);
    BF3003_WriteReg(0x12, 0x14);

    BF3003_WriteReg(0x32, 0x80);
    BF3003_WriteReg(0x17, 0x16);
    BF3003_WriteReg(0x18, 0x04);
    BF3003_WriteReg(0x19, 0x02);
    BF3003_WriteReg(0x1a, 0x7b);
    BF3003_WriteReg(0x03, 0x06);

    BF3003_WriteReg(0x0c, 0x00);
    BF3003_WriteReg(0x15, 0x00);
    BF3003_WriteReg(0x3e, 0x00);
    BF3003_WriteReg(0x70, 0x3a);
    BF3003_WriteReg(0x71, 0x35);
    BF3003_WriteReg(0x72, 0x11);
    BF3003_WriteReg(0x73, 0x00);

    BF3003_WriteReg(0xa2, 0x02);
    BF3003_WriteReg(0x11, 0x81);
    BF3003_WriteReg(0x7a, 0x20);
    BF3003_WriteReg(0x7b, 0x1c);
    BF3003_WriteReg(0x7c, 0x28);

    BF3003_WriteReg(0x7d, 0x3c);
    BF3003_WriteReg(0x7e, 0x55);
    BF3003_WriteReg(0x7f, 0x68);
    BF3003_WriteReg(0x80, 0x76);
    BF3003_WriteReg(0x81, 0x80);

    BF3003_WriteReg(0x82, 0x88);
    BF3003_WriteReg(0x83, 0x8f);
    BF3003_WriteReg(0x84, 0x96);
    BF3003_WriteReg(0x85, 0xa3);
    BF3003_WriteReg(0x86, 0xaf);

    BF3003_WriteReg(0x87, 0xc4);
    BF3003_WriteReg(0x88, 0xd7);
    BF3003_WriteReg(0x89, 0xe8);
    BF3003_WriteReg(0x13, 0xe0);
    BF3003_WriteReg(0x00, 0x00);

    BF3003_WriteReg(0x10, 0x00);
    BF3003_WriteReg(0x0d, 0x00);
    BF3003_WriteReg(0x14, 0x28);
    BF3003_WriteReg(0xa5, 0x05);
    BF3003_WriteReg(0xab, 0x07);

    BF3003_WriteReg(0x24, 0x75);
    BF3003_WriteReg(0x25, 0x63);
    BF3003_WriteReg(0x26, 0xA5);
    BF3003_WriteReg(0x9f, 0x78);
    BF3003_WriteReg(0xa0, 0x68);

    BF3003_WriteReg(0xa1, 0x03);
    BF3003_WriteReg(0xa6, 0xdf);
    BF3003_WriteReg(0xa7, 0xdf);
    BF3003_WriteReg(0xa8, 0xf0);
    BF3003_WriteReg(0xa9, 0x90);

    BF3003_WriteReg(0xaa, 0x94);
    BF3003_WriteReg(0x13, 0xe5);
    BF3003_WriteReg(0x0e, 0x61);
    BF3003_WriteReg(0x0f, 0x4b);
    BF3003_WriteReg(0x16, 0x02);

    BF3003_WriteReg(0x1e, 0x37);
    BF3003_WriteReg(0x21, 0x02);
    BF3003_WriteReg(0x22, 0x91);
    BF3003_WriteReg(0x29, 0x07);
    BF3003_WriteReg(0x33, 0x0b);

    BF3003_WriteReg(0x35, 0x0b);
    BF3003_WriteReg(0x37, 0x1d);
    BF3003_WriteReg(0x38, 0x71);
    BF3003_WriteReg(0x39, 0x2a);
    BF3003_WriteReg(0x3c, 0x78);

    BF3003_WriteReg(0x4d, 0x40);
    BF3003_WriteReg(0x4e, 0x20);
    BF3003_WriteReg(0x69, 0x00);
    BF3003_WriteReg(0x6b, 0x40);
    BF3003_WriteReg(0x74, 0x19);
    BF3003_WriteReg(0x8d, 0x4f);

    BF3003_WriteReg(0x8e, 0x00);
    BF3003_WriteReg(0x8f, 0x00);
    BF3003_WriteReg(0x90, 0x00);
    BF3003_WriteReg(0x91, 0x00);
    BF3003_WriteReg(0x92, 0x00);

    BF3003_WriteReg(0x96, 0x00);
    BF3003_WriteReg(0x9a, 0x80);
    BF3003_WriteReg(0xb0, 0x84);
    BF3003_WriteReg(0xb1, 0x0c);
    BF3003_WriteReg(0xb2, 0x0e);

    BF3003_WriteReg(0xb3, 0x82);
    BF3003_WriteReg(0xb8, 0x0a);
    BF3003_WriteReg(0x43, 0x14);
    BF3003_WriteReg(0x44, 0xf0);
    BF3003_WriteReg(0x45, 0x34);

    BF3003_WriteReg(0x46, 0x58);
    BF3003_WriteReg(0x47, 0x28);
    BF3003_WriteReg(0x48, 0x3a);
    BF3003_WriteReg(0x59, 0x88);
    BF3003_WriteReg(0x5a, 0x88);

    BF3003_WriteReg(0x5b, 0x44);
    BF3003_WriteReg(0x5c, 0x67);
    BF3003_WriteReg(0x5d, 0x49);
    BF3003_WriteReg(0x5e, 0x0e);
    BF3003_WriteReg(0x64, 0x04);
    BF3003_WriteReg(0x65, 0x20);

    BF3003_WriteReg(0x66, 0x05);
    BF3003_WriteReg(0x94, 0x04);
    BF3003_WriteReg(0x95, 0x08);
    BF3003_WriteReg(0x6c, 0x0a);
    BF3003_WriteReg(0x6d, 0x55);

    BF3003_WriteReg(0x4f, 0x80);
    BF3003_WriteReg(0x50, 0x80);
    BF3003_WriteReg(0x51, 0x00);
    BF3003_WriteReg(0x52, 0x22);
    BF3003_WriteReg(0x53, 0x5e);
    BF3003_WriteReg(0x54, 0x80);

    BF3003_WriteReg(0x09, 0x03);

    BF3003_WriteReg(0x6e, 0x11);
    BF3003_WriteReg(0x6f, 0x9f);
    BF3003_WriteReg(0x55, 0x00);
    BF3003_WriteReg(0x56, 0x40);
    BF3003_WriteReg(0x57, 0x40);
    BF3003_WriteReg(0x6a, 0x40);
    BF3003_WriteReg(0x01, 0x40);
    BF3003_WriteReg(0x02, 0x40);
    BF3003_WriteReg(0x13, 0xe7);
    BF3003_WriteReg(0x15, 0x00);

    BF3003_WriteReg(0x58, 0x9e);

    BF3003_WriteReg(0x41, 0x08);
    BF3003_WriteReg(0x3f, 0x00);
    BF3003_WriteReg(0x75, 0x05);
    BF3003_WriteReg(0x76, 0xe1);
    BF3003_WriteReg(0x4c, 0x00);
    BF3003_WriteReg(0x77, 0x01);
    BF3003_WriteReg(0x3d, 0xc2);
    BF3003_WriteReg(0x4b, 0x09);
    BF3003_WriteReg(0xc9, 0x60);
    BF3003_WriteReg(0x41, 0x38);

    BF3003_WriteReg(0x34, 0x11);
    BF3003_WriteReg(0x3b, 0x02);

    BF3003_WriteReg(0xa4, 0x89);
    BF3003_WriteReg(0x96, 0x00);
    BF3003_WriteReg(0x97, 0x30);
    BF3003_WriteReg(0x98, 0x20);
    BF3003_WriteReg(0x99, 0x30);
    BF3003_WriteReg(0x9a, 0x84);
    BF3003_WriteReg(0x9b, 0x29);
    BF3003_WriteReg(0x9c, 0x03);
    BF3003_WriteReg(0x9d, 0x4c);
    BF3003_WriteReg(0x9e, 0x3f);
    BF3003_WriteReg(0x78, 0x04);

    BF3003_WriteReg(0x79, 0x01);
    BF3003_WriteReg(0xc8, 0xf0);
    BF3003_WriteReg(0x79, 0x0f);
    BF3003_WriteReg(0xc8, 0x00);
    BF3003_WriteReg(0x79, 0x10);
    BF3003_WriteReg(0xc8, 0x7e);
    BF3003_WriteReg(0x79, 0x0a);
    BF3003_WriteReg(0xc8, 0x80);
    BF3003_WriteReg(0x79, 0x0b);
    BF3003_WriteReg(0xc8, 0x01);
    BF3003_WriteReg(0x79, 0x0c);
    BF3003_WriteReg(0xc8, 0x0f);
    BF3003_WriteReg(0x79, 0x0d);
    BF3003_WriteReg(0xc8, 0x20);
    BF3003_WriteReg(0x79, 0x09);
    BF3003_WriteReg(0xc8, 0x80);
    BF3003_WriteReg(0x79, 0x02);
    BF3003_WriteReg(0xc8, 0xc0);
    BF3003_WriteReg(0x79, 0x03);
    BF3003_WriteReg(0xc8, 0x40);
    BF3003_WriteReg(0x79, 0x05);
    BF3003_WriteReg(0xc8, 0x30);
    BF3003_WriteReg(0x79, 0x26);
    BF3003_WriteReg(0x09, 0x00);
}
/*
 * @brief    寄存器初始化
 */
void BF3003_Configure(void)
{
    BF3003_WriteReg(0x12, 0x80); // 寄存器复位,所有寄存器复位为初始默认值
    delay_ms(100);

    BF3003_RegExample(); /* 参照其他教程的寄存器设置，具体寄存器对应的功能暂未弄清 */

    BF3003_WriteReg(0x11, 0x9F); /* 输入时钟32分频 */
    BF3003_WriteReg(0x6b, 0x00); /* 设置输入时钟倍频为1 */
    /* PCLK频率：F=4MHz/32*1=0.125MHz */
    /* 经初步测试，PCLK<=0.8MHz时能够正常获取图像数据不丢失 */

    /*设置测试图案输出  这里设置的是输出八色彩条*/
    BF3003_WriteReg(0x70, 0x3A);
    BF3003_WriteReg(0x71, 0xB5);
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
            frame[640 * i + j] = GPIOA->IDR & 0xFF;
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

    delay_ms(1000);
}