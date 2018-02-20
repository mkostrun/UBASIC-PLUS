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
#include "gpio.h"
#include "../uBasic-Plus/hardware/usart.h"
#include "../uBasic-Plus/hardware/random.h"

/* USER CODE BEGIN Includes */
#include "ubasic.h"

/* Includes ------------------------------------------------------------------*/
#include "cli.h"
uint8_t cli_state=UBASIC_CLI_IDLE;

/* Private variables ---------------------------------------------------------*/
_MainStatus main_status = { .byte = 0x00 };

static const char *program[] = {

"\
println 'Demo 1 - Warm-up';\
gosub l1;\
for i = 1 to 2;\
for j = 1 to 2;\
println 'i,j=',i,j;\
next j;\
next i;\
println 'Demo 1 Completed';\
end;\
:l1 \
println 'subroutine';\
return;",

"\
println 'Demo 2 - ubasic with strings';\
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
println 'Demo 2 Completed';",

"\
println 'Demo 3 - Plus';\
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
println 'Digital Write Test';\
pinmode(0xc0,-1,0);\
pinmode(0xc1,-1,0);\
pinmode(0xc2,-1,0);\
pinmode(0xc3,-1,0);\
for j = 0 to 2;\
  dwrite(0xc0,(j % 2));\
  dwrite(0xc1,(j % 2));\
  dwrite(0xc2,(j % 2));\
  dwrite(0xc3,(j % 2));\
  sleep(0.5);\
next j;\
println 'Press the Blue Button or type kill!';\
:presswait \
  if hw_event(1)=0 then goto presswait;\
tic(1);\
println 'Blue Button pressed!';\
:deprwait \
  if hw_event(2)=0 then goto deprwait;\
println 'duration =' toc(1);\
println 'Blue Button de-pressed!';\
println 'Demo 3 Completed';\
end;",

"\
println 'Demo 4 - Input with timeouts';\
dim a@(5);\
for i = 1 to 5;\
  print '?';\
  input a@(i),10000;\
next i;\
println 'end of input';\
for i = 1 to 5;\
  println 'a(' i ') = ' a@(i);\
next i;\
println 'Demo 4 Completed';\
end",

"\
println 'Demo 5 - analog inputs and arrays';\
aread_conf(7,16);\
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
println 'Demo 5 Completed';\
end;",


"\
println 'Demo 6: Multiline if, while';\
println 'Test If: 1';\
for i=1 to 10 step 0.125;\
  x = uniform;\
  if (x>=0.5) then;\
    println x, 'is greater then 0.5';\
  else;\
    println x, 'is smaller then 0.5';\
  endif;\
  println 'i=' i;\
next i;\
println 'End of If-test 1';\
println 'Test While: 1';\
i=10;\
while ((i>=0)&&(uniform<=0.9));\
  i = i - 0.125;\
  println 'i =', i;\
endwhile;\
println 'End of While-test 1';\
println 'Demo 6 Completed';\
end",

"\
println 'Demo 7: Analog Read or Kill';\
y=0;\
:startover \
  x = aread(10);\
  if (abs(x-y)>20) then;\
    y = x;\
    println 'x=',x;\
  endif;\
  sleep (0.2);\
goto startover;\
end",

"\
println 'Demo 8: analog write (PWM) 4-Channel Test';\
p = 65536;\
for k = 1 to 10;\
  p = p/2;\
  awrite_conf(p,4096);\
  println 'prescaler = ' p;\
  for i = 1 to 10;\
    for j = 1 to 4;\
      awrite(j,4095*uniform);\
    next j;\
    println '    analog write = ' awrite(1),awrite(2),awrite(3),awrite(4);\
    sleep(5);\
  next i;\
next k;\
awrite(1,0);\
awrite(2,0);\
awrite(3,0);\
awrite(4,0);\
end"
};


/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
char welcome_msg[]=
"\
Welcome to uBasic-Plus for STM32 by M.Kostrun\n\
Expands upon uBasic by A.Dunkels, uBasic with string by D.Mitchell,\n\
and uBasic for CHDK by Pablo d'Angelo\n>";
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */
/* USER CODE BEGIN 0 */

/* Private variables ---------------------------------------------------------*/
char script[1024];
char statement[64];

/* USER CODE END 0 */

int main(void)
{
  /* USER CODE BEGIN 1 */
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


  print_serial(welcome_msg);
  while (1)
  {
    if ( (cli_state == UBASIC_CLI_LOADED) ||
          (cli_state == UBASIC_CLI_RUNNING) )
    {
      ubasic_run_program();
      cli_state = UBASIC_CLI_RUNNING;
      if (ubasic_finished())
      {
        cli_state = UBASIC_CLI_IDLE;
        print_serial("\n>");
      }
      else if (!ubasic_waiting_for_input())
      {
        if (serial_input_available())
        {
          serial_input(statement,sizeof(statement));
          if (is_cmd(statement,"kill"))
          {
            // enter programming mode
            print_serial("killed\n>");
            cli_state = UBASIC_CLI_IDLE;
            ubasic_load_program(NULL);
            continue;
          }
        }
      }
    }

    if (cli_state != UBASIC_CLI_RUNNING)
    {
      if (serial_input_available())
      {
        serial_input(statement,sizeof(statement));

        if (is_cmd(statement,"prog"))
        {
          // enter programming mode
          script[0] = 0;
          print_serial("Enter your script. Type 'run' to execute!\n>");
          cli_state = UBASIC_CLI_PROG;
          continue;
        }
        else if (is_cmd(statement,"run"))
        {
          // run script
          print_serial("run\n");
          if (strlen(script) > 0)
          {
            ubasic_load_program( script );
            cli_state = UBASIC_CLI_LOADED;
          }
          continue;
        }
        else if (is_cmd(statement,"cat"))
        {
          // list script
          print_serial("cat\n");
          if (strlen(script)>0)
          {
            print_serial(script);
            print_serial("\n>");
          }
          continue;
        }
        else if (is_cmd(statement,"save"))
        {
          // save script: exit PROG mode
          print_serial("save\n>");
          if (strlen(script)>0)
          {
            cli_state = UBASIC_CLI_IDLE;
          }
          continue;
        }
        else if (is_cmd(statement,"edit"))
        {
          // edit script: re-enter PROG mode
          print_serial("edit\n>");
          if (strlen(script)>0)
          {
            cli_state = UBASIC_CLI_PROG;
          }
          continue;
        }
        else if (is_cmd(statement,"demo"))
        {
          // run script
          print_serial(statement);
          print_serial("\n");
          char *s = &statement[4];
          while (*s==' ') ++s;
          uint8_t idx = *s - '0';
          ubasic_load_program( program[idx-1] );
          cli_state = UBASIC_CLI_LOADED;
          continue;
        }

        if (cli_state == UBASIC_CLI_PROG)
        {
          // add statement to the script
          if (strlen(script)>0)
          {
            while (script[strlen(script)-1]==' ' || script[strlen(script)-1]=='\t')
              script[strlen(script)-1]='\0';
            if (script[strlen(script)-1] != '\n' && script[strlen(script)-1] != ';')
              sprintf(&script[strlen(script)], "\n");
          }
          sprintf(&script[strlen(script)], statement);
          print_serial(statement);
          print_serial("\n>");
        }
        else
        {
          // prepare statement for execution
          if (strlen(statement) > 0)
          {
            print_serial(statement);
            print_serial("\n>");
            ubasic_load_program( statement );
            cli_state = UBASIC_CLI_LOADED;
          }
        }
      }
    }
  }


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
