#ifndef __BF3003_H
#define __BF3003_H


void BF3003_Init(void);

void BF3003_GetPic(void);

void BF3003_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t BF3003_ReadReg(uint8_t RegAddress);

#endif