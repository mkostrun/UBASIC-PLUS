/*
 * digio.h
 *
 *  Created on: Jan 31, 2018
 *      Author: mk
 */

#ifndef DIGIO_H_
#define DIGIO_H_

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */
void
    pinMode(uint8_t ch, int8_t mode, uint8_t freq);

int8_t
    digitalWrite(uint8_t ch, uint8_t PinState);

int8_t
    digitalRead(uint8_t ch);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* DIGIO_H_ */
