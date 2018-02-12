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
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "ubasic.h"

/* Includes ------------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
_MainStatus main_status = { .byte = 0x00 };

static const char *program[] = {

"\
;\
;\
println 'start of test 1';\
gosub l1;\
for i = 1 to 2;\
for j = 1 to 2;\
println 'i,j=',i,j;\
next j;\
next i;\
println 'end of test 1';\
end;\
:l1 \
println 'subroutine';\
return;",

"\
println 'start of test 2';\
a$= 'abcdefghi';\
b$='123456789';\
println 'Length of a$=', len(a$);\
println 'Length of b$=', len(b$);\
if len(a$) = len(b$) then println 'same length';\
if a$ = b$ then println 'same string'\n\
c$=left$(a$+ b$,12);\
println c$;\
c$=right$(a$+b$, 12);\
println c$;\
c$=mid$(a$+b$, 8,8);\
println c$;\
c$=str$(13+42);\
println c$;\
println len(c$);\
println len('this' + 'that');\
c$ = chr$(34);\
println 'c$=' c$;\
j = asc(c$);\
println 'j=' j;\
println val('12345');\
i=instr(3, '123456789', '67');\
println 'position of 67 in 123456789 is', i;\
println mid$(a$,2,2)+'xyx';\
println 'end of test 2';",

"\
println 'start of test 3';\
for i = 1 to 2;\
println 'i=',i;\
gosub test;\
next i;\
j=1.79;\
println 'j=' j;\
println 'end of test 3';\
end;\
:test \
println 'subroutine - sleep';\
sleep(1);\
return;",

"\
println 'start of test 4';\
tic(1);\
for i = 1 to 2;\
j = i + 0.25 + 1/2;\
println 'j=' j;\
k = sqrt(2*j) + ln(4*i) + cos(i+j) + sin(j);\
println 'k=' k;\
next i;\
:repeat \
if toc(1)<=300 then goto repeat;\
for i = 1 to 2;\
println 'ran(' i ')=' ran;\
next i;\
for i = 1 to 2;\
println 'uniform(' i ')=' uniform;\
next i;\
for i = 1 to 2;\
x = 10 * uniform;\
println 'x=' x;\
println 'floor(x)=' floor(x);\
println 'ceil(x)=' ceil(x);\
println 'round(x)=' round(x);\
println 'x^3=' pow(x,3);\
next i;\
println 'Analog Write Test';\
for i = 1 to 5;\
for j = 1 to 4;\
pwm(j,4095*uniform);\
next j;\
sleep(0.5);\
println 'pwm=' pwm(1),pwm(2),pwm(3),pwm(4);\
next i;\
pwm(1,0);\
pwm(2,0);\
pwm(3,0);\
pwm(4,0);\
println 'GPIO 1:4 Test';\
for i = 1 to 1;\
for j = 0 to 2;\
gpio(i,(j % 2));\
sleep(0.5);\
next j;\
next i;\
println 'gpio(1)=' gpio(1);\
println 'gpio(2)=' gpio(2);\
println 'Press the Blue Button!';\
:presswait \
if hw_event(1)=0 then goto presswait;\
tic(1);\
println 'Blue Button pressed!';\
:deprwait \
if hw_event(2)=0 then goto deprwait;\
println 'duration =' toc(1);\
println 'Blue Button de-pressed!';\
println 'end of test 4';\
end;",

"\
println 'start of test 5';\
dim a@(5);\
for i = 1 to 5;\
print '?';\
input a@(i),10000;\
next i;\
println 'end of input';\
for i = 1 to 5;\
println 'a(' i ') = ' a@(i);\
next i;\
println 'end of test 5';\
end",

"\
println 'start of test 6';\
for i = 1 to 100;\
x = aread(16);\
y = aread(17);\
println 'VREF,TEMP=', x, y;\
next i;\
for i = 1 to 1;\
n = floor(10 * uniform) + 2 ;\
dim b@(n);\
for j = 1 to n;\
b@(j) = ran;\
println 'b@(' j ')=' b@(j);\
next j;\
next i;\
println 'end of test 6';\
end;",

"\
println 'start of test 7';\
println 'Test If:';\
for i=1 to 10 step 0.125;\
  x = uniform;\
  if x>0.5 then;\
    println x, 'is greater then 0.5';\
  else;\
    println x, 'is smaller then 0.5';\
  endif;\
  println i;\
next i;\
println 'Test While:';\
while i>=0;\
  i = i - 0.125;\
  println 'i =', i;\
endwhile;\
end"
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
  char statement[64];
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
  MX_ADC_Init();
#if defined(USE_CRC_FOR_RANDOM_NUMBER_GENERATION)
  MX_CRC_Init();
#endif

  MX_GPIO_Init();

  /* Initialize UART2 - circular buffer for RX */
  MX_UART2_Configuration();

  /* Initialize Push Button - see also sw.c */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

  for (uint8_t i=0; i<1; i++)
  {
    ubasic_load_program( program[6] );
    ubasic_clear_variables();
    do
    {
      ubasic_run_program();
    }
    while(!ubasic_finished());
  }

  /* USER CODE BEGIN WHILE */
  ubasic_clear_variables();
  print_serial(">");

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
