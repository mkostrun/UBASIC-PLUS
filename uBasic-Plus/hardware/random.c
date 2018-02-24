#include "main.h"


#undef CRC_SALT_I
#undef CRC_SALT_F

// #define  CRC_SALT_I   0xBADA55E5
// #define  CRC_SALT_F   0xBADA55E5

#if defined(USE_STM32F0XX_NUCLEO) || defined(USE_STM32F4XX_NUCLEO) || defined(USE_STM32F0XX_DISCOVERY)

#ifdef USE_STM32F0XX_DISCOVERY
#include "stm32f0xx_hal_conf.h"
#endif

#ifdef USE_STM32F4XX_NUCLEO
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal_adc.h"
#if defined(CRC_SALT_I)
  #include "stm32f4xx_hal_crc.h"
  #include "stm32f4xx_hal_crc_ex.h"
#endif
#endif


#if defined(CRC_SALT_I)
CRC_HandleTypeDef hcrc;

void MX_CRC_Init(void)
{
  /**
   * Enable CRC clock
   */
  if (main_status.bit.bit0 == 1)
    return;

  __HAL_RCC_CRC_CLK_ENABLE();
  hcrc.Instance = CRC;
  hcrc.Init.CRCLength = CRC_POLYLENGTH_32B;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
  hcrc.Init.InitValue = CRC_SALT_I;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}
#endif


ADC_HandleTypeDef       hadc;
ADC_ChannelConfTypeDef  sConfig;
static uint8_t ch_prev = 0xff, nreads=1, stime=0, stime_prev=8;


void MX_ADC_Init(void)
{
  RCC_OscInitTypeDef        RCC_OscInitStructure;

  /* Note: In case of usage of asynchronous clock derived from ADC dedicated  */
  /*       HSI RC oscillator 14MHz, with ADC setting                          */
  /*       "hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1",            */
  /*       the clock source has to be enabled at RCC top level using function */
  /*       "HAL_RCC_OscConfig()" (see comments in stm32f0_hal_adc.c header)   */

  /* Enable asynchronous clock source of ADCx */
  /* (place oscillator HSI14 under control of the ADC) */
  HAL_RCC_GetOscConfig(&RCC_OscInitStructure);
  RCC_OscInitStructure.OscillatorType = RCC_OSCILLATORTYPE_HSI14;
  RCC_OscInitStructure.HSI14CalibrationValue = RCC_HSI14CALIBRATION_DEFAULT;
  RCC_OscInitStructure.HSI14State = RCC_HSI14_ADC_CONTROL;
  HAL_RCC_OscConfig(&RCC_OscInitStructure);

  /* Enable clock of ADC1 peripheral */
  __HAL_RCC_ADC1_CLK_ENABLE();

  /**
    * Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
    */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler          = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution              = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign               = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode            = DISABLE;
  hadc.Init.EOCSelection            = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait        = DISABLE;
  hadc.Init.LowPowerAutoPowerOff    = DISABLE;
  hadc.Init.ContinuousConvMode      = DISABLE;
  hadc.Init.DiscontinuousConvMode   = DISABLE;
  hadc.Init.ExternalTrigConv        = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge    = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests   = DISABLE;
  hadc.Init.Overrun                 = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  return;
}

void analogReadConfig(uint8_t sampletime, uint8_t nr)
{
  if (nr)
    nreads = nr;

  stime = stime % 8; /* there are 8 sampling time options for STM32 */
  return;
}


int16_t analogRead(uint8_t ch)
{
  int16_t val=0;
  uint32_t dummy=0;
  uint8_t i=0;

  if (ch != ch_prev)
  {
    // clear old configuration 
    sConfig.Rank = ADC_RANK_NONE;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
      return -2;

    // start new configuration
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;

    if (ch<0x12)
    {
      sConfig.Channel = ch;
    }
#if defined(ADC_CHANNEL_VBAT)
    else if (ch==0x12)
    {
      sConfig.Channel = ADC_CHANNEL_VBAT;
    }
#endif
    else
      return -1;

    ch_prev = ch;
    i = 1;
  }

  if (stime_prev != stime)
  {
    if (stime==0)
    {
      sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    }
    else if (stime==1)
    {
      sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
    }
    else if (stime==2)
    {
      sConfig.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
    }
    else if (stime==3)
    {
      sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
    }
    else if (stime==4)
    {
      sConfig.SamplingTime = ADC_SAMPLETIME_41CYCLES_5;
    }
    else if (stime==5)
    {
      sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
    }
    else if (stime==6)
    {
      sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    }
    else if (stime==7)
    {
      sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    }

    stime_prev = stime;
    i = 1;
  }

  if (i)
  {
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
      return -3;
  }

  /* Start ADC1 Software Conversion */
  for (i=0; i<nreads; i++)
  {
    if (HAL_ADC_Start(&hadc) != HAL_OK)
    {
      Error_Handler();
    }

    /* wait for conversion complete */
    while (!__HAL_ADC_GET_FLAG(&hadc, ADC_FLAG_EOC));

    /* Two LS bits are most likely most random */
    dummy += HAL_ADC_GetValue(&hadc) ;

    //clear EOC flag
    __HAL_ADC_CLEAR_FLAG(&hadc, ADC_FLAG_EOC);
  }

  val = dummy / nreads;
  return val;
}


uint32_t RandomUInt32(uint8_t size)
{
  uint32_t val=0, temp_val=0;

  analogReadConfig(0,1); /* fastest readout, no averaging */

  for (uint8_t k=0; k<4; k++)
  {
    temp_val = 0;
    for (uint8_t i=0; i<(size>>1); i++)
    {
      /* Two LS bits are most likely most random */
      temp_val |= (analogRead(0x10) & 0x00000003) << (2*i);
    }
    val ^= temp_val;
  }
  #if defined(CRC_SALT_I)
  val = HAL_CRC_Accumulate(&hcrc, &val, 1);
  /* Add salt */
  val = CRC_SALT_F;
  val = HAL_CRC_Accumulate(&hcrc, &val, 1);
  #endif
  return val;
}

#endif /* #if defined(USE_STM32F0XX_NUCLEO) || defined(USE_STM32F4XX_NUCLEO) || defined(USE_STM32F0XX_DISCOVERY) */

 
