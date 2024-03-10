#ifndef __DELAY_H
#define __DELAY_H 			   
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
void delay_init();
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);

#ifdef __cplusplus
}
#endif
#endif





























