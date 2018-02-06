/*
 * push_button.c
 *
 *  Created on: Jan 30, 2018
 *      Author: mk
 */
#include "main.h"

__IO uint8_t pb_status=0, hw_event=0;

#if defined(USE_STM32F0XX_DISCOVERY)
/**
  * @brief  This function handles PUSH BUTTON on PA0 interrupt request.
  * @param  None
  * @retval None
  */
void PB_Init(void)
{
  GPIO_InitTypeDef gpioinitstruct;

  /* Enable the BUTTON Clock */
  USER_BUTTON_GPIO_CLK_ENABLE();

  gpioinitstruct.Pin = USER_BUTTON_PIN;
  gpioinitstruct.Pull = GPIO_NOPULL;
  gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;

  /* Configure Button pin as input with External interrupt */
  gpioinitstruct.Mode = GPIO_MODE_IT_RISING;

  HAL_GPIO_Init(USER_BUTTON_GPIO_PORT, &gpioinitstruct);

  /* Enable and set Button EXTI Interrupt to the lowest priority */
  HAL_NVIC_SetPriority( (IRQn_Type)(USER_BUTTON_EXTI_IRQn), 0x03, 0x00);
  HAL_NVIC_EnableIRQ( (IRQn_Type)(USER_BUTTON_EXTI_IRQn) );
}

/**
 * @brief  Push Button DeInit.
 * @param  Button Button to be configured
 *   This parameter should be: BUTTON_USER
 * @note PB DeInit does not disable the GPIO clock
 * @retval None
 */
void PB_DeInit(void)
{
  GPIO_InitTypeDef gpio_init_structure;

  gpio_init_structure.Pin = USER_BUTTON_PIN;
  HAL_NVIC_DisableIRQ( (IRQn_Type)(USER_BUTTON_EXTI_IRQn) );
  HAL_GPIO_DeInit(USER_BUTTON_GPIO_PORT, gpio_init_structure.Pin);
}

void EXTI0_1_IRQHandler(void)
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
      GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
      pb_status = 0;
      hw_event |= 0x02; // pressed
    }
    else
    {
      GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
      pb_status = 1;
      hw_event |= 0x01; // depressed
    }
    HAL_GPIO_Init(USER_BUTTON_GPIO_PORT, &GPIO_InitStruct);

    /* Clear interrupt flag */
    __HAL_GPIO_EXTI_CLEAR_IT(USER_BUTTON_EXTI_LINE);
  }
}
#endif

