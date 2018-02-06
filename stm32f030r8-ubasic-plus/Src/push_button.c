/*
 * push_button.c
 *
 *  Created on: Jan 30, 2018
 *      Author: mk
 */
#include "cube_hal.h"

__IO uint8_t pb_status=0, hw_event=0;

/**
 * @brief  This function handles PUSH BUTTON interrupt request.
 * @param  None
 * @retval None
 */
void EXTI4_15_IRQHandler(void)
{
  /* Make sure that interrupt flag is set */
  if(__HAL_GPIO_EXTI_GET_IT(USER_BUTTON_EXTI_LINE) != RESET)
  {
    /* Do your stuff when PB12 is changed */
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Configure Button pin as input with External interrupt */
    GPIO_InitStruct.Pin = USER_BUTTON_PIN;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;

    if (pb_status)
    {
      GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
      pb_status = 0;
      hw_event |= 0x02; // pressed
    }
    else
    {
      GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
      pb_status = 1;
      hw_event |= 0x01; // depressed
    }
    HAL_GPIO_Init(USER_BUTTON_GPIO_PORT, &GPIO_InitStruct);

    /* Clear interrupt flag */
    __HAL_GPIO_EXTI_CLEAR_IT(USER_BUTTON_EXTI_LINE);
  }
}

