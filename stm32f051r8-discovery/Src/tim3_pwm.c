/**
  ******************************************************************************
  * File Name          : TIM.c
  * Description        : This file provides code for the configuration
  *                      of the TIM instances.
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
#include "main.h"

/* USER CODE BEGIN 0 */
#define TIM3_PWM1_PSC 16
#define TIM3_PWM1_PERIOD_CLK 4096

TIM_HandleTypeDef htim3;
TIM_OC_InitTypeDef sConfigOC;

int16_t
    dutycycle_pwm_ch[4] = {-1,-1,-1,-1};

static uint8_t
    init_completed=0;

/* USER CODE END 0 */

/* TIM3 init function */
void pwm_Init(uint8_t ch)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  if (dutycycle_pwm_ch[ch-1] >= 0)
    return;

  if (!init_completed)
  {
    TIM_MasterConfigTypeDef sMasterConfig;

    memset(&htim3, 0, sizeof(htim3));
    memset(&sMasterConfig, 0, sizeof(sMasterConfig));
    memset(&sConfigOC, 0, sizeof(sConfigOC));

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = TIM3_PWM1_PSC;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = TIM3_PWM1_PERIOD_CLK;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    init_completed=1;
  }


  if ( (ch==1) && (dutycycle_pwm_ch[0] < 0))
  {
    /* TIM3 GPIO Configuration
     * PA6     ------> TIM3_CH1
     */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    dutycycle_pwm_ch[0] = 0;
  }

  if ( (ch==2) && (dutycycle_pwm_ch[1] < 0))
  {
    /* TIM3 GPIO Configuration
     * PA7     ------> TIM3_CH2
      */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    dutycycle_pwm_ch[1] = 0;
  }

  if ( (ch==3) && (dutycycle_pwm_ch[2] < 0))
  {
    /* TIM3 GPIO Configuration
     * PB0     ------> TIM3_CH3
     */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    dutycycle_pwm_ch[2] = 0;
  }

  if ( (ch==4) && (dutycycle_pwm_ch[3] < 0))
  {
    /* TIM3 GPIO Configuration
     * PB1     ------> TIM3_CH4
     */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    dutycycle_pwm_ch[3] = 0;
  }
}

void pwm_UpdateDutyCycle(uint8_t ch, int16_t dutycycle)
{
  if (dutycycle < 0)
    return;

  if (dutycycle_pwm_ch[ch-1] < 0)
    pwm_Init(ch);

  if (dutycycle_pwm_ch[ch-1] != dutycycle)
  {
    dutycycle_pwm_ch[ch-1] = dutycycle;

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = dutycycle_pwm_ch[ch-1];
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    if (ch==1)
    {
      if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
      {
        _Error_Handler(__FILE__, __LINE__);
      }
      HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
    }
    else if (ch==2)
    {
      if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
      {
        _Error_Handler(__FILE__, __LINE__);
      }
      HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
    }
    else if (ch==3)
    {
      if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
      {
        _Error_Handler(__FILE__, __LINE__);
      }
      HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);
    }
    else if (ch==4)
    {
      if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
      {
        _Error_Handler(__FILE__, __LINE__);
      }
      HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_4);
    }
  }
}



void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* tim_pwmHandle)
{

  if(tim_pwmHandle->Instance==TIM3)
  {
    /* USER CODE BEGIN TIM3_MspInit 0 */

    /* USER CODE END TIM3_MspInit 0 */
    /* TIM3 clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();
    /* USER CODE BEGIN TIM3_MspInit 1 */

    /* USER CODE END TIM3_MspInit 1 */
  }
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* tim_pwmHandle)
{

  if(tim_pwmHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspDeInit 0 */

  /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();
  /* USER CODE BEGIN TIM3_MspDeInit 1 */

  /* USER CODE END TIM3_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
