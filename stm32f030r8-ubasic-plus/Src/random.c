#include "main.h"


#undef CRC_SALT_I
#undef CRC_SALT_F

// #define  CRC_SALT_I   0xBADA55E5
// #define  CRC_SALT_F   0xBADA55E5

#if defined(USE_STM32F0XX_NUCLEO) || defined(USE_STM32F4XX_NUCLEO)

#ifdef USE_STM32F0XX_NUCLEO
#include "stm32f0xx_hal_adc.h"
#if defined(CRC_SALT_I)
  #include "stm32f0xx_hal_crc.h"
  #include "stm32f0xx_hal_crc_ex.h"
#endif
#endif

#ifdef USE_STM32F4XX_NUCLEO
#include "stm32f4xx_hal_adc.h"
#if defined(CRC_SALT_I)
  #include "stm32f4xx_hal_crc.h"
  #include "stm32f4xx_hal_crc_ex.h"
#endif
#endif


static uint8_t init_trn=0;
ADC_HandleTypeDef hadc;
#if defined(CRC_SALT_I)
CRC_HandleTypeDef hcrc;
#endif

static void open_trand_crc(void)
{
  __HAL_RCC_ADC1_CLK_ENABLE();

  // Initialize ADC 14MHz RC
  __HAL_RCC_HSI14_ENABLE();
  __HAL_RCC_HSI14ADC_ENABLE();
  while (!__HAL_RCC_GET_FLAG(RCC_FLAG_HSI14RDY));
  /**
   * Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
   */
  ADC_ChannelConfTypeDef sConfig;
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_BACKWARD;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**
   * Configure for the selected ADC regular channel to be converted.
   */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**
   * Enable CRC clock
   */
#if defined(CRC_SALT_I)
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
#endif
}

static void close_trand_crc(void)
{
  /* disable non-needed services */
  __HAL_RCC_ADC1_CLK_DISABLE();
#if defined(CRC_SALT_I)
  __HAL_RCC_CRC_CLK_DISABLE();
#endif
}

uint32_t RandomUInt32(uint8_t size)
{
  uint32_t val=0;

  if (!init_trn)
    open_trand_crc();

  for (uint8_t k=0; k<4; k++)
  for (uint8_t i=0; i<(size>>1); i++)
  {
    /* Start ADC1 Software Conversion */
    if (HAL_ADC_Start(&hadc) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    /* wait for conversion complete */
    while (!__HAL_ADC_GET_FLAG(&hadc, ADC_FLAG_EOC));

    /* Two LS bits are most likely most random */
    val ^= (HAL_ADC_GetValue(&hadc) & 0x00000003) << (2*i);

    //clear EOC flag
    __HAL_ADC_CLEAR_FLAG(&hadc, ADC_FLAG_EOC);
  }
#if defined(CRC_SALT_I)
  val = HAL_CRC_Accumulate(&hcrc, &val, 1);
  /* Add salt */
  val = CRC_SALT_F;
  val = HAL_CRC_Accumulate(&hcrc, &val, 1);
#endif
  return val;
}
#endif

 
