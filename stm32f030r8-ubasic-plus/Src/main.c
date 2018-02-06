/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
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
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_conf.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "ubasic.h"

/* Includes ------------------------------------------------------------------*/

extern __IO uint32_t ubasic_script_sleeping_ms;
extern uint8_t sleep_for_input;

static const char *program[] = {

"\
9 println 'start of test 1';\
10 gosub 100;\
20 for i = 1 to 2;\
21 for j = 1 to 2;\
30 println 'i,j=',i,j;\
31 next j;\
40 next i;\
50 println 'end of test 1';\
60 end;\
100 println 'subroutine';\
110 return;",

"\
1 println 'start of test 2';\
2 a$= 'abcdefghi';\
3 b$='123456789';\
5 println 'Length of a$=', len(a$);\
6 println 'Length of b$=', len(b$);\
7 if len(a$) = len(b$) then println 'same length';\
8 if a$ = b$ then println 'same string'\n\
9 c$=left$(a$+ b$,12);\
10 println c$;\
11 c$=right$(a$+b$, 12);\
12 println c$;\
13 c$=mid$(a$+b$, 8,8);\
14 println c$;\
15 c$=str$(13+42);\
16 println c$;\
17 println len(c$);\
18 println len('this' + 'that');\
19 c$ = chr$(34);\
20 println 'c$=' c$;\
21 j = asc(c$);\
22 println 'j=' j;\
23 println val('12345');\
24 i=instr(3, '123456789', '67');\
24 println 'position of 67 in 123456789 is', i;\
25 println mid$(a$,2,2)+'xyx';\
30 println 'end of test 2';",

"\
10 println 'start of test 3';\
11 for i = 1 to 2;\
12 println 'i=',i;\
13 gosub 100;\
14 next i;\
15 j=1.79;\
16 println 'j=' j;\
50 println 'end of test 3';\
60 end;\
100 println 'subroutine - sleep';\
101 sleep(1);\
102 return;",

"\
10 println 'start of test 4';\
11 tic(1);\
12 for i = 1 to 2;\
13 j = i + 0.25 + 1/2;\
14 println 'j=' j;\
15 k = sqrt(2*j) + ln(4*i) + cos(i+j) + sin(j);\
16 println 'k=' k;\
20 next i;\
21 if toc(1)<=300 then goto 21;\
31 for i = 1 to 2;\
32 println 'ran(' i ')=' ran;\
33 next i;\
40 for i = 1 to 2;\
41 println 'uniform(' i ')=' uniform;\
42 next i;\
50 for i = 1 to 2;\
51 x = 10 * uniform;\
52 println 'x=' x;\
53 println 'floor(x)=' floor(x);\
54 println 'ceil(x)=' ceil(x);\
55 println 'round(x)=' round(x);\
56 println 'x^3=' pow(x,3);\
57 next i;\
60 println 'Analog Write Test';\
61 for i = 1 to 2;\
62 for j = 1 to 4;\
63 pwm(j,4095*uniform);\
64 next j;\
65 sleep(0.5);\
66 println 'pwm=' pwm(1),pwm(2),pwm(3),pwm(4);\
70 next i;\
71 pwm(1,0);\
72 pwm(2,0);\
73 pwm(3,0);\
74 pwm(4,0);\
80 println 'GPIO 1:4 Test';\
81 for i = 1 to 4;\
82 for j = 0 to 2;\
84 gpio(i,(j % 2));\
85 sleep(0.5);\
86 next j;\
87 next i;\
88 println 'gpio(1)=' gpio(1);\
89 println 'gpio(2)=' gpio(2);\
90 println 'Press the Push button!';\
91 if hw_event(1)=0 then goto 91;\
92 tic(1);\
93 println 'Push button pressed!';\
94 if hw_event(2)=0 then goto 94;\
95 println 'duration =' toc(1);\
96 println 'Push button de-pressed!';\
98 println 'end of test 4';\
99 end;",

"\
10 println 'start of test 1';\
11 for i = 1 to 100;\
12 n = floor(10 * uniform) + 2 ;\
12 dim b@(n);\
21 for j = 1 to n;\
22 b@(j) = ran;\
30 println 'b@(' j ')=' b@(j);\
31 next j;\
40 next i;\
50 println 'end of test 1';\
60 end;",

"\
10 println 'start of test 6';\
11 dim a@(5);\
11 for i = 1 to 5;\
12 print '?';\
13 input a@(i),2000;\
14 next i;\
15 println 'end of input';\
20 for i = 1 to 5;\
21 println 'a(' i ') = ' a@(i);\
22 next i;\
50 println 'end of test 6';\
60 end"
};


/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */
/* USER CODE BEGIN 0 */

/* Private variables ---------------------------------------------------------*/


/* USER CODE END 0 */

int main(void)
{
  /* USER CODE BEGIN 1 */
  uint32_t counter=0;
  uint32_t time_last_ms = HAL_GetTick();
  /* USER CODE END 1 */

  /* BASIC VARS BEGIN */
  /* BASIC VARS END */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  /* Initialize UART2 - circular buffer for RX */
  MX_UART2_Configuration();

  /* Initialize Push Button - see also sw.c */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

  /**
    *   basic welcome message
    */
  for (uint8_t i=0; i<1; i++)
  {
    ubasic_init( program[3] );
    ubasic_var_init();
    do
    {
      ubasic_run();
    }
    while(!ubasic_finished());
  }

  while (1)
  {
    /**
      *   basic interpreter
      */
  } /* while (1) */

} /* main() */

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r;", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */


/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
