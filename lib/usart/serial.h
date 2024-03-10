#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void serial_init(void);
void uart_log(const char*msg);

#ifdef __cplusplus
}
#endif