#include "main.h"
#include "../core/config.h"
#include "./eeprom.h"

#if defined(USE_STM32F0XX_NUCLEO) || defined(USE_STM32F0XX_DISCOVERY)

  #if !defined(PAGE)
    #define PAGE (63)
  #endif

#endif

static uint32_t ee_start_address;
static uint32_t ee_free_address;
static uint32_t ee_used_bytes=0;
static uint8_t  ee_initialized=0;


/**
 * @brief  Erases PAGE and writes RECEIVE_DATA header to first half-word of the PAGE
 * @param  None
 * @retval Status of the last operation (Flash write or erase) done during
 *         EEPROM formating or HAL_OK
 */
static HAL_StatusTypeDef EE_Format(void)
{
  HAL_StatusTypeDef FlashStatus = HAL_OK;
  uint32_t SectorError = 0;
  FLASH_EraseInitTypeDef pEraseInit;

  pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
  pEraseInit.PageAddress = PAGE_BASE_ADDRESS(PAGE);
  pEraseInit.NbPages = 1;

  /* Erase page */
  FlashStatus = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
  /* If erase operation was failed, a Flash error code is returned */
  if (FlashStatus != HAL_OK)
  {
    return FlashStatus;
  }

  /* Write RECEIVE_DATA at page base address */
  HAL_FLASH_Unlock();
  FlashStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, PAGE_BASE_ADDRESS(PAGE),
                                  RECEIVE_DATA);
  HAL_FLASH_Lock();

  /* If program operation was failed, a Flash error code is returned */
  if (FlashStatus != HAL_OK)
  {
    return FlashStatus;
  }

  ee_used_bytes = sizeof(RECEIVE_DATA);
  ee_free_address = ee_start_address = PAGE_BASE_ADDRESS(PAGE) + ee_used_bytes;

  return HAL_OK;
}


/**
  * @brief  EE_WriteVariable
  *     @arg NAME: Index of variable  Name-'a'
  *     @arg Vartype: 0-4byte (fixed point) float, 1-string of nonzero length, 2-(fp)f array
  *     @arg Datalen: length of data
  *     @arg dataptr: pointer to data storage
  * @retval None
  */
void EE_WriteVariable(uint8_t Name, uint8_t Vartype,
                      uint8_t datalen_bytes, uint8_t *dataptr)
{
  /* What is variable headless header:
   *  [7:6] -> variable type: 0-4byte (fixed point) float, 1-string of nonzero length, 2-(fp)f array
   *  [5:0] -> variable name index as in Name-'a', for Name=a,b,c..z
   *
   */
  uint8_t i;

  EE_HEADER_TYPE headless_header = EE_HEADER(Vartype,Name+1);

  EE_FULLHEADER_TYPE  header = (EE_FULLHEADER_TYPE) headless_header; // as LSB thus first in FLASH

  /* do we have enough room to write the data ? */
  if (datalen_bytes + ee_free_address + 2 > PAGE_END_ADDRESS(PAGE))
  {
    /* if not, just erase the entire page */
    EE_Format();
  }
  else
  {
    /* check if this variable has been written to flash before. If that happened
     * then it was written only once:
     *    If it did appear before, zero-it: FLASH will accept second write as long as these
     *    are all zeros !  */
    uint32_t PageCheckAddress = ee_start_address;
    while (PageCheckAddress < ee_free_address)
    {
      EE_FULLHEADER_TYPE  full_header = (*(__IO EE_FULLHEADER_TYPE *)PageCheckAddress);
      EE_HEADER_TYPE check_header = EE_FULLHEADER_HEADER(full_header);
      EE_HEADER_TYPE len          = EE_FULLHEADER_DATALEN(full_header)>>1;
      if ( headless_header == check_header )
      {
        HAL_FLASH_Unlock();
#if defined(EE_ERASE_FULL)
        // the headless header describes possibly the same variable,
        // perhaps with different length if string or array
        for (i=0; i<(len + (sizeof(EE_FULLHEADER_TYPE)>>1)); i++)
        {
          HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, PageCheckAddress, 0x0000);
          PageCheckAddress += 2;
        }
#elif defined(EE_ERASE_VARNAME_FROM_FULLHEADER)
  #if defined(EE_HEADER_FORMAT_16)
          HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, PageCheckAddress,
                            EE_FULLHEADER_NO_VARNAME);
  #elif  defined(EE_HEADER_FORMAT_16)
          HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, PageCheckAddress,
                            EE_FULLHEADER_NO_VARNAME);
  #endif
#endif
        HAL_FLASH_Lock();
        // update ee_start_page if blanking was done at the beginning of the flash
        if (ee_start_address + ((len+1)<<1) == PageCheckAddress)
          ee_start_address = PageCheckAddress;
        ee_used_bytes -= (len+1)<<1;
        break;
      }

      PageCheckAddress += (len<<1) + sizeof(EE_FULLHEADER_TYPE);
    }
  }

  
  if ((Vartype == UBASIC_VAR_FLOAT) || (Vartype == UBASIC_VAR_ARRAY))
  {
    HAL_FLASH_Unlock();
#if defined(EE_HEADER_FORMAT_16)
    header |= (datalen_bytes << 8); // LSB first then MSB
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ee_free_address, header);
#elif defined(EE_HEADER_FORMAT_32)
  #error "I dont know what is your header!"
#endif
    ee_free_address += sizeof(EE_FULLHEADER_TYPE);
    ee_used_bytes += sizeof(EE_FULLHEADER_TYPE);

    uint16_t *Data = (uint16_t *) dataptr;
    for (i=0; i<(datalen_bytes>>1); i++)
    {
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ee_free_address, Data[i]);
      ee_free_address += 2;
      ee_used_bytes += 2;
    }
    HAL_FLASH_Lock();
  }
  else if (Vartype == UBASIC_VAR_STRING)
  {
    /* calculate padding for strings:
     *    odd strlen get '\0' at the end, while
     *    even strlen get two '\0' at the end.
    */
#if defined(EE_HEADER_FORMAT_16)
    if (datalen_bytes % 2)
    {
      header |= ((datalen_bytes+1) << 8); // LSB first then MSB
    }
    else
    {
      header |= ((datalen_bytes+2) << 8); // LSB first then MSB
    }
#else
  #error "I dont know what is your header!"
#endif
    HAL_FLASH_Unlock();

#if defined(EE_HEADER_FORMAT_16)
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ee_free_address, header);
#elif defined(EE_HEADER_FORMAT_32)
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, ee_free_address, header);
#endif
    ee_free_address += sizeof(EE_FULLHEADER_TYPE);
    ee_used_bytes += sizeof(EE_FULLHEADER_TYPE);

    uint16_t Data;
    for (i=0; i<(datalen_bytes>>1); i++)
    {
      Data = dataptr[2*i] | dataptr[2*i+1]<<8;
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ee_free_address, Data);
      ee_free_address += 2;
      ee_used_bytes += 2;
    }

    if (datalen_bytes % 2)
    {
      Data = dataptr[2*i];
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ee_free_address, Data);
    }
    else
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ee_free_address, 0x0000);

    HAL_FLASH_Lock();
    ee_free_address += 2;
    ee_used_bytes += 2;
  }
}

void EE_ReadVariable(uint8_t Name, uint8_t Vartype, uint8_t *dataptr, uint8_t *datalen)
{
  if (!datalen)
    return;

  *datalen = 0;

  if (!dataptr)
    return;

  if (Name == 0xff)
    return;

  EE_HEADER_TYPE headless_header = EE_HEADER(Vartype,Name+1);

  uint32_t PageCheckAddress = ee_start_address;

  while (PageCheckAddress < ee_free_address)
  {
    EE_FULLHEADER_TYPE header = (*(__IO EE_FULLHEADER_TYPE *) PageCheckAddress);
    EE_HEADER_TYPE check_header = EE_FULLHEADER_HEADER(header);
    EE_HEADER_TYPE len          = EE_FULLHEADER_DATALEN(header);

    if ( headless_header == check_header )
    {
      PageCheckAddress += sizeof(EE_FULLHEADER_TYPE);
      for (uint8_t i=0; i<len; i++)
      {
        dataptr[i] = (*(__IO uint8_t*)(PageCheckAddress));
        PageCheckAddress++;
      }
      *datalen = len;
      return;
    }

    PageCheckAddress += sizeof(EE_FULLHEADER_TYPE) + len;
  }

  return;
}


/**
  * @brief  Initialize the flash page used for storage of variables
  * @param  None.
  * @retval None.
  */
void EE_Init(void)
{
  if (ee_initialized)
    return;

  if ((*(__IO uint16_t*)PAGE_BASE_ADDRESS(PAGE)) != RECEIVE_DATA)
  {
    EE_Format();
  }
  else
  {
    ee_start_address = (uint32_t)(PAGE_BASE_ADDRESS(PAGE)) + sizeof(RECEIVE_DATA);
#if defined(EE_ERASE_FULL)
    while ( (*(__IO uint16_t*)ee_start_address) == 0x0000 )
      ee_start_address += 2;
#elif defined(EE_ERASE_VARNAME_FROM_FULLHEADER)
    while ( !EE_VARNAME(*(__IO EE_FULLHEADER_TYPE *) ee_start_address) )
      ee_start_address += EE_FULLHEADER_DATALEN( (*(__IO EE_FULLHEADER_TYPE *) ee_start_address) )
          + sizeof(EE_FULLHEADER_TYPE);
#endif
    ee_used_bytes = 0;
    ee_free_address = ee_start_address;
    while (ee_free_address < PAGE_END_ADDRESS(PAGE))
    {
      EE_FULLHEADER_TYPE header = (*(__IO EE_FULLHEADER_TYPE *)ee_free_address);

      if (header == (EE_FULLHEADER_TYPE) -1)
        break;

      /* the block contains variable data in the format
       *    header[0:7]                   -> 1 byte
       *    datalen[0:7]                  -> 1 byte
       *      data[0]....data[datalen-1]  -> datalen bytes
       * use that to scan the used flash memory
      */

#if defined(EE_ERASE_FULL)
      // were this segment blanked: 0's written in it, then just scan over zeros
      if (header == ((EE_FULLHEADER_TYPE)0) )
      {
        do
        {
          ee_free_address += sizeof(EE_FULLHEADER_TYPE);
        }
        while ((*(__IO EE_FULLHEADER_TYPE *) ee_free_address) == (EE_FULLHEADER_TYPE) 0);
        continue;
      }
#endif
      // if instead Varname is blanked, then no variable will match it
      // so we just skip it like any other variable

      // segment is used - determine its length then skip over it
      EE_FULLHEADER_TYPE datalen  = EE_FULLHEADER_DATALEN(*(__IO EE_FULLHEADER_TYPE *)ee_free_address);
      ee_free_address += sizeof(EE_FULLHEADER_TYPE) + datalen;
      if (EE_VARNAME(header))
        ee_used_bytes   += sizeof(EE_FULLHEADER_TYPE) + datalen;
    }
  }

  ee_initialized = 1;
  return;
}


void EE_DumpFlash(void)
{
  if (!ee_initialized)
    return;

  char msg[32] = {0};

  if ( *(__IO uint16_t*)PAGE_BASE_ADDRESS(PAGE) != RECEIVE_DATA)
  {
    return;
  }


  sprintf(msg, "page:  %u\n", PAGE);
  print_serial(msg);
  sprintf(msg, "start: %08lx\n", ee_start_address);
  print_serial(msg);
  sprintf(msg, "free:  %08lx\n", ee_free_address);
  print_serial(msg);
  sprintf(msg, "used:  %lu/%u\n",   ee_used_bytes, FLASH_PAGE_SIZE);
  print_serial(msg);

  uint32_t PageCheckAddress = ee_start_address;

  while (PageCheckAddress < ee_free_address)
  {
    EE_FULLHEADER_TYPE header = (*(__IO EE_FULLHEADER_TYPE *) PageCheckAddress);

#if defined(EE_ERASE_FULL)
    while ( header == (EE_FULLHEADER_TYPE) 0 )
    {
      PageCheckAddress += 2;
      header = (*(__IO EE_FULLHEADER_TYPE *) PageCheckAddress);
    }
#elif defined(EE_ERASE_VARNAME_FROM_FULLHEADER)
    while ( EE_FULLHEADER_VARNAME(header) == (EE_FULLHEADER_TYPE) 0 )
    {
      PageCheckAddress += sizeof(EE_FULLHEADER_TYPE) + EE_FULLHEADER_DATALEN(header);
      header = (*(__IO EE_FULLHEADER_TYPE *) PageCheckAddress);
    }
#endif

    sprintf(msg, "Name: %c", EE_FULLHEADER_VARNAME(header) - 1 + 'a');
    print_serial(msg);
    if (EE_VARTYPE(header) == 0)
    {
      print_serial(" (float)\n");
    }
    else if (EE_VARTYPE(header) == 1)
    {
      print_serial("$ (string)\n");
    }
    else if (EE_VARTYPE(header) == 2)
    {
      print_serial("@ (array)\n");
    }
    sprintf(msg, "  Size:%u+%u bytes\n", sizeof(header), EE_FULLHEADER_DATALEN(header));
    print_serial(msg);

    PageCheckAddress += sizeof(EE_FULLHEADER_TYPE) + EE_FULLHEADER_DATALEN(header);
  }

  return;
}

