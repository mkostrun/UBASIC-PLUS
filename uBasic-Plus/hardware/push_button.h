#ifndef __PUSH_BUTTON_H
#define __PUSH_BUTTON_H

extern __IO uint8_t pb_status, hw_event;

#if defined(USE_STM32F0XX_DISCOVERY)
void  PB_Init(void);
#endif

#endif
