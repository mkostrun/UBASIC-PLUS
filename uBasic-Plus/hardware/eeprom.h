/*
 * eeprom.h
 *
 *  Created on: Jan 31, 2018
 *      Author: mk
 */
#ifndef __EEPROM_H
#define __EEPROM_H

/* Includes ------------------------------------------------------------------*/
#if defined(USE_STM32F4XX_NUCLEO)
  #include "stm32f4xx_hal_conf.h"
  #if !defined(FLASH_PAGE_SIZE)
    #define FLASH_PAGE_SIZE   (uint32_t)0x4000  /* Page size = 16KByte */
  #endif
  #define VOLTAGE_RANGE     (uint8_t)VOLTAGE_RANGE_3
#endif

#if defined(USE_STM32F0XX_NUCLEO) || defined(USE_STM32F0XX_DISCOVERY)
  #include "stm32f0xx_hal_conf.h"
  #if !defined(FLASH_PAGE_SIZE)
    #define FLASH_PAGE_SIZE   (uint32_t)0x0400  /* Page size = 1KByte */
  #endif
#endif

/* Exported constants --------------------------------------------------------*/

/* EEPROM emulation firmware error codes */
#define EE_OK      (uint32_t)HAL_OK
#define EE_ERROR   (uint32_t)HAL_ERROR
#define EE_BUSY    (uint32_t)HAL_BUSY
#define EE_TIMEOUT (uint32_t)HAL_TIMEOUT

/* No valid page define */
#define NO_VALID_PAGE         ((uint16_t)0x00AB)

/* Page status definitions */
#define ERASED                ((uint16_t)0xFFFF)     /* Page is empty */
#define RECEIVE_DATA          ((uint16_t)0xEEEE)     /* Page is marked to receive data */
#define VALID_PAGE            ((uint16_t)0x0000)     /* Page containing valid data */

/* Valid pages in read and write defines */
#define READ_FROM_VALID_PAGE  ((uint8_t)0x00)
#define WRITE_IN_VALID_PAGE   ((uint8_t)0x01)

/* Page full define */
#define PAGE_FULL             ((uint8_t)0x80)

/* Exported Macros ------------------------------------------------------- */

/* FLASH page base and end address */
#define PAGE_BASE_ADDRESS(p)  ( (uint32_t) (0x08000000 + FLASH_PAGE_SIZE * (p)      ) )
#define PAGE_END_ADDRESS(p)   ( (uint32_t) (0x08000000 + FLASH_PAGE_SIZE * (p+1) - 1) )

/**
  * uBasicPlus:
  */
#undef  EE_HEADER_FORMAT_16
#undef  EE_HEADER_FORMAT_32
#undef  EE_ERASE_FULL
#undef  EE_ERASE_VARNAME_FROM_FULLHEADER

#define EE_HEADER_FORMAT_16
#define EE_ERASE_FULL
#define UBASIC_VAR_FLOAT  (0)
#define UBASIC_VAR_STRING (1)
#define UBASIC_VAR_ARRAY  (2)

#if defined(EE_HEADER_FORMAT_16)
/* General concept:
 *    Uses not-so-much advertised feature that to flash one can write
 *    twice (or more) as long as there are bits available that have not
 *    been set to 0. This is called blanking.
 *    The flash is utilized as follows: data is written to the end of
 *    space in flash, and previous write of the same 'variable' is located
 *    and blanked out.
 *    The data is written sequentially in blocks starting with header, where the
 *    header comprise of the variable name and description (1 byte - big
 *    enough for uBasicPlus variables, string and arrays), and length in
 *    bytes of the data (for uBasicPlus 255 bytes suffices).
 *    This is followed by the data.
 *      Header:
 *          headless header:
 *              [7:6]   Variable type
 *              [5:0]   Variable Name, inUbasic '$' - 'a' where $ \in a:z
 *          length of data block in bytes
 *              [7:0]   datalen (always even number in uBasicPlus)
 *        Importantly, the choice of variable type, name, and datalen are
 *        such that the header can never be 0xffff.
 *
 *      Data:
 *          byte0, ..... byte[datalen - 1]
 *
 *
 *     Search if variable has already been written before is fast, as only
 *     the header is used to identify the content of the data block, and
 *     if this is not the right one, datalen is used to skip to the next
 *     datablock.
 *     Blanked 
 *     If the previous instance if found then the entire datablock is blanked out.
 */
  #define EE_HEADER_TYPE  uint8_t
  #define EE_VARTYPE(x)   ( ((EE_HEADER_TYPE) x) >> 6    )
  #define EE_VARNAME(x)   ( ((EE_HEADER_TYPE) x) &  0x3f )
  #define EE_HEADER(typ,nam) ( ( (((EE_HEADER_TYPE)typ) & 0x03) << 6) | (((EE_HEADER_TYPE)nam) & 0x3f) )
  #define EE_FULLHEADER_TYPE  uint16_t
  #define EE_FULLHEADER(h,len) ( ((EE_FULLHEADER_TYPE)h) | (((EE_FULLHEADER_TYPE)len)<<8) )
  #define EE_FULLHEADER_NO_VARNAME  ( (EE_FULLHEADER_TYPE) 0xff00)
  #define EE_FULLHEADER_DATALEN(h)  ( (EE_FULLHEADER_TYPE) h >> 8 )
  #define EE_FULLHEADER_HEADER(h)   ( (EE_FULLHEADER_TYPE) h & 0x00ff )
  #define EE_FULLHEADER_VARNAME(h)  ( (EE_FULLHEADER_TYPE) h & 0x003f )
  #define EE_FULLHEADER_VARTYPE(h)  ( ((EE_FULLHEADER_TYPE) h & 0x003f) >> 6 )

#elif defined(EE_HEADER_FORMAT_32)
/*
 * Put your own definition of header here assuming 32 bits are available
 */
  #define EE_HEADER_TYPE  uint16_t
  #define EE_VARTYPE(x)   ( ((EE_HEADER_TYPE) x) >> 6)
  #define EE_VARNAME(x)   ( ((EE_HEADER_TYPE) x) &  0x3f )
  #define EE_HEADER(typ,nam) ( ( (((EE_HEADER_TYPE)typ) & 0x03) << 6) | (((EE_HEADER_TYPE)nam) & 0x3f) )
  #define EE_FULLHEADER_TYPE  uint32_t
  #define EE_FULLHEADER(h,len) ( ((EE_FULLHEADER_TYPE)h) | (((EE_FULLHEADER_TYPE)len)<<16) )

  #if defined(EE_ERASE_VARNAME_FROM_FULLHEADER)
    #define EE_FULLHEADER_NO_VARNAME ((EE_FULLHEADER_TYPE) 0xff00)
  #endif
#endif


/* Exported functions ------------------------------------------------------- */
void EE_Init(void);
void EE_WriteVariable(uint8_t Name, uint8_t Vartype, uint8_t datalen_bytes, uint8_t *dataptr);
void EE_ReadVariable(uint8_t Name, uint8_t Vartype, uint8_t *dataptr, uint8_t *datalen);
void EE_DumpFlash(void);
#endif /* __EEPROM_H */
