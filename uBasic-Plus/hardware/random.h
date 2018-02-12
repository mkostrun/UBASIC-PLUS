#ifndef __RANDOM_H
#define __RANDOM_H

void MX_ADC_Init(void);
void MX_CRC_Init(void);
uint8_t SelectChannelADC(uint8_t channel);
uint16_t AnalogRead(void);
uint32_t RandomUInt32(uint8_t size);

#endif
