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

void DIGIO_ConfGpio(uint8_t ch, uint8_t mode, int8_t pull);
void DIGIO_GetSetGpio(uint8_t ch);
extern int8_t digio_out_ch[4];
extern int8_t digio_in_ch[4];

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* DIGIO_H_ */
