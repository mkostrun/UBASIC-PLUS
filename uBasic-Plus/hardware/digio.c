
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** Configure GPIO pins :
  *     ch = 0xa0:0xff, same exclusions apply
  *     mode =  0 - input - no pull
  *             1 - input - pull up
  *             2 - input - pull down
  *            -1 - output - pushpull
  *            -2 - output - open drain
  *     freq =  0 - low
  *             1 - medium
  *             2 - high
  */
void pinMode(uint8_t ch, int8_t mode, uint8_t freq)
{
  uint32_t pin  = 1 << (ch & 0x0f) ;
  uint8_t port  = (ch>>4) & 0x0f;

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  if (port == 0x0a)
  {
    __HAL_RCC_GPIOA_CLK_ENABLE();
  }
  else if (port == 0x0b)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  }
  else if (port == 0x0c)
  {
    __HAL_RCC_GPIOC_CLK_ENABLE();
  }
  else if (port == 0x0d)
  {
    __HAL_RCC_GPIOD_CLK_ENABLE();
  }
  else if (port == 0x0f)
  {
    __HAL_RCC_GPIOF_CLK_ENABLE();
  }

  GPIO_InitStruct.Pin = pin;

  if (mode == -2)
  {
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  }
  else if (mode < 0)
  {
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  }
  else
  {
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    if (mode == 1)
      GPIO_InitStruct.Pull = GPIO_PULLUP;
    else if (mode == 2)
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    else
      GPIO_InitStruct.Pull = GPIO_NOPULL;
  }

  if (freq == 1)
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  else if (freq == 2)
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  else
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  if (port == 0x0a)
  {
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
  else if (port == 0x0b)
  {
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
  else if (port == 0x0c)
  {
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  }
  else if (port == 0x0d)
  {
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  }
  else if (port == 0x0f)
  {
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  }
}

int8_t digitalRead(uint8_t ch)
{
  int PinState = -1;
  uint32_t pin  = 1 << (ch & 0x0f) ;
  uint8_t port  = (ch>>4) & 0x0f;

    /* Update input on ch */
  if (port == 0x0a)
  {
    PinState = HAL_GPIO_ReadPin(GPIOA, pin);
  }
  else if (port == 0x0b)
  {
    PinState = HAL_GPIO_ReadPin(GPIOB, pin);
  }
  else if (port == 0x0c)
  {
    PinState = HAL_GPIO_ReadPin(GPIOC, pin);
  }
  else if (port == 0x0d)
  {
    PinState = HAL_GPIO_ReadPin(GPIOD, pin);
  }
  else if (port == 0x0f)
  {
    PinState = HAL_GPIO_ReadPin(GPIOF, pin);
  }

  return PinState;
}


int8_t digitalWrite(uint8_t ch, uint8_t PinState)
{
  uint32_t pin  = 1 << (ch & 0x0f) ;
  uint8_t port  = (ch>>4) & 0x0f;

  if (port == 0x0a)
  {
    HAL_GPIO_WritePin(GPIOA, pin, PinState);
  }
  else if (port == 0x0b)
  {
    HAL_GPIO_WritePin(GPIOB, pin, PinState);
  }
  else if (port == 0x0c)
  {
    HAL_GPIO_WritePin(GPIOC, pin, PinState);
  }
  else if (port == 0x0d)
  {
    HAL_GPIO_WritePin(GPIOD, pin, PinState);
  }
  else if (port == 0x0f)
  {
    HAL_GPIO_WritePin(GPIOF, pin, PinState);
  }
  else
    return -1;

  return PinState;
}

