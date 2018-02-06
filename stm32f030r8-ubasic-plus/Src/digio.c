
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_hal.h"

int8_t digio_out_ch[4] = {-1,-1,-1,-1};
int8_t digio_in_ch[4] = {-1,-1,-1,-1};

void DIGIO_ConfGpio(uint8_t ch, uint8_t mode, int8_t pull)
{
  if (ch<1 || ch>4)
    return;

  /** Configure GPIO pins :
    * PC0 PC1 PC2 PC3 as digital inputs (mode=0) or outputs (mode=1)
    */
  if (mode == 1)
  {
    if (digio_out_ch[ch-1] < 0)
    {
      GPIO_InitTypeDef GPIO_InitStruct;

      /* GPIO Ports Clock Enable */
      __HAL_RCC_GPIOC_CLK_ENABLE();

      if (ch==1)
        GPIO_InitStruct.Pin = GPIO_PIN_0;
      else if (ch==2)
        GPIO_InitStruct.Pin = GPIO_PIN_1;
      else if (ch==3)
        GPIO_InitStruct.Pin = GPIO_PIN_2;
      else if (ch==4)
        GPIO_InitStruct.Pin = GPIO_PIN_3;

      if (pull == 0)
        GPIO_InitStruct.Pull = GPIO_NOPULL;
      else if (pull < 0)
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
      else if (pull > 0)
        GPIO_InitStruct.Pull = GPIO_PULLUP;

      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      digio_out_ch[ch-1] = 0;
      digio_in_ch[ch-1]  = -1;

      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    }
  }
  else
  {
    if (digio_in_ch[ch-1] < 0)
    {
      GPIO_InitTypeDef GPIO_InitStruct;

      /* GPIO Ports Clock Enable */
      __HAL_RCC_GPIOC_CLK_ENABLE();

      if (ch==1)
        GPIO_InitStruct.Pin = GPIO_PIN_0;
      else if (ch==2)
        GPIO_InitStruct.Pin = GPIO_PIN_1;
      else if (ch==3)
        GPIO_InitStruct.Pin = GPIO_PIN_2;
      else if (ch==4)
        GPIO_InitStruct.Pin = GPIO_PIN_3;

      if (pull == 0)
        GPIO_InitStruct.Pull = GPIO_NOPULL;
      else if (pull < 0)
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
      else if (pull > 0)
        GPIO_InitStruct.Pull = GPIO_PULLUP;

      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      digio_in_ch[ch-1]  = 0;
      digio_out_ch[ch-1] = -1;

      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    }
  }
}

void DIGIO_GetSetGpio(uint8_t ch)
{
  if (ch<1 || ch>4)
    return;

  if (digio_out_ch[ch-1] < 0)
  {
    if (digio_in_ch[ch-1] < 0)
      return;

    /* Update input on ch */
    if (ch==1)
      digio_in_ch[0] = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0);
    else if (ch==2)
      digio_in_ch[1] = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);
    else if (ch==3)
      digio_in_ch[2] = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2);
    else
      digio_in_ch[3] = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3);
  }
  else
  {
    /* Update output on ch */
    GPIO_PinState PinState = (digio_out_ch[ch-1] > 0);
    if (ch==1)
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, PinState);
    else if (ch==2)
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, PinState);
    else if (ch==3)
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, PinState);
    else
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, PinState);
  }

  return;
}

