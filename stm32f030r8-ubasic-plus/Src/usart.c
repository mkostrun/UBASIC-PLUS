/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "cube_hal.h"
#include "main.h"
#include "gpio.h"

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

UART_HandleTypeDef huart2;
__IO ITStatus UART_TX_Completed=RESET, UART_RX_Available=RESET;

void print_serial(char * msg);

void MX_UART2_Configuration(void)
{
  huart2.Instance        = USART2;
  huart2.Init.BaudRate   = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits   = UART_STOPBITS_1;
  huart2.Init.Parity     = UART_PARITY_NONE;
  huart2.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  huart2.Init.Mode       = UART_MODE_TX_RX;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  __HAL_LOCK(&huart2); /* Process Locked */
  huart2.rx_last = huart2.rx_first = huart2.rx_available = 0;
  huart2.ErrorCode = HAL_UART_ERROR_NONE;
  huart2.RxState = HAL_UART_STATE_BUSY_RX;
  __HAL_UNLOCK(&huart2);  /* Process Unlocked */

  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  SET_BIT(huart2.Instance->CR3, USART_CR3_EIE);
  /* Enable the UART Parity Error and Data Register not empty Interrupts */
  SET_BIT(huart2.Instance->CR1, USART_CR1_PEIE | USART_CR1_RXNEIE);

  /* USART2 interrupt Init */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);

  HAL_Delay(50);
  print_serial("\n");
}

/**
 * @brief  Initializes UART MSP.
 * @param  UART_HandleTypeDef* pUARTHandle
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef* pUARTHandle)
{
  if (pUARTHandle->Instance == USART2)
  {
    GPIO_InitTypeDef GPIO_InitStruct;

    /* USER CODE BEGIN USART2_MspInit 1 */
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Enable USART2 clock */
    __HAL_RCC_USART2_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = GPIO_PIN_2;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_Delay(50);
  }
}


uint8_t UART2_Transmit(uint8_t * msg, uint16_t lenmsg)
{
  UART_TX_Completed = RESET;
  return HAL_UART_Transmit_IT(&huart2, msg, lenmsg);
}

/* USER CODE BEGIN 1 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* Set transmission flag: trasfer complete*/
  UART_TX_Completed = SET;
}

/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/
/* USER CODE BEGIN 1 */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_Cir_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}


void print_serial(char * msg)
{
  UART2_Transmit((uint8_t*) msg, strlen(msg));
  while (UART_TX_Completed==RESET);
}

uint8_t serial_input_available(uint16_t timeout)
{
  uint8_t n = HAL_UART_Cir_Available_IT(&huart2);

  if (!n)
    return 0;

  if (HAL_UART_Cir_Last_Time_IT(&huart2)<timeout)
    return 0;

  return n;
}

uint8_t serial_input (char * buffer, uint8_t len, uint16_t timeout)
{
  uint8_t n = serial_input_available(timeout);
  if (!n)
    return 0;

  uint16_t i = 0;
  while (HAL_UART_Cir_Available_IT(&huart2))
  {
    buffer[i] = HAL_UART_Cir_RetrieveReceivedChar_IT(&huart2);
    i++;

    if (i==len)
      break;
  }

  while (buffer[i-1]=='\n')
    i--;
  while (buffer[i-1]=='\r')
    i--;
  buffer[i] = '\0';

  return i;
}



/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
