#ifndef _CONFIG_H_
#define _CONFIG_H_

/*-
 * Copyright (c) 2018, Marijan Kostrun <mkostrun@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>


//
// Undef it all
//

/* Storage and arithmetics */
#undef  VARIABLE_STORAGE_INT16
#undef  VARIABLE_STORAGE_INT32
#undef  VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8
#undef  VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10
#undef  VARIABLE_TYPE_STRING
#undef  VARIABLE_TYPE_ARRAY

/* Microcontroller related functionality */
#undef  UBASIC_SCRIPT_HAVE_RANDOM_NUMBER_GENERATOR
#undef  UBASIC_SCRIPT_HAVE_PWM_CHANNELS
#undef  UBASIC_SCRIPT_HAVE_GPIO
#undef  UBASIC_SCRIPT_HAVE_TICTOC
#undef  UBASIC_SCRIPT_HAVE_SLEEP
#undef  UBASIC_SCRIPT_HAVE_HARDWARE_EVENTS
#undef  UBASIC_SCRIPT_PRINT_TO_SERIAL
#undef  UBASIC_SCRIPT_HAVE_INPUT_FROM_SERIAL
#undef  UBASIC_SCRIPT_HAVE_ANALOG_READ
/**
  *
  *   UBASIC-PLUS: Start
  *
  */

/* default storage for all numeric values */
#define VARIABLE_STORAGE_INT32

/* defines the representation of floating point numbers as fixed points:
    this is to allow UBASIC to run on Cortex M0 processors which do not
    support Floating Point Arithmetic in hardware (they emulate it which
    consumes lots of flash memory) */
#define VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8

/* This many one-letter variables UBASIC supports */
#define MAX_VARNUM 26

/* have numeric arrays and set their storage to this many VARIABLE_TYPE entries */
#define VARIABLE_TYPE_ARRAY 64

/* have strings and related functions */
#define VARIABLE_TYPE_STRING

/* can go to sleep: leave UBASIC for other stuff while waiting for timer to expire */
#define  UBASIC_SCRIPT_HAVE_SLEEP

/* have microcontroller support for PWM: specify how many channels */
#define  UBASIC_SCRIPT_HAVE_PWM_CHANNELS  (4)

/* have internal timer channels available through rlab-like toc(ch) functions */
#define  UBASIC_SCRIPT_HAVE_TICTOC

/* support for random number generator by micro-controller */
#define  UBASIC_SCRIPT_HAVE_RANDOM_NUMBER_GENERATOR

/* support for direct access to pin inputs and ooutputs */
#define  UBASIC_SCRIPT_HAVE_GPIO_CHANNELS (4)

/* support flags in BASIC that change on hardware events:
    for STM32F0XX nucleo and discovery boards
   source of the events is push-button
*/
#define  UBASIC_SCRIPT_HAVE_HARDWARE_EVENTS

/* have a standard print to serial console function */
#define UBASIC_SCRIPT_PRINT_TO_SERIAL

/* how is input function supported ? */
#define UBASIC_SCRIPT_HAVE_INPUT_FROM_SERIAL

/* support for analog inputs */
#define  UBASIC_SCRIPT_HAVE_ANALOG_READ

/**
  *
  *   UBASIC-PLUS: End
  *
  */



/*
 * Selectively load header files based on the ocnfiguration above.
 * Remember MC Hammer:
 *                      CAN'T TOUCH THIS!
 *
 */
#if defined(VARIABLE_STORAGE_INT32)

  #define VARIABLE_TYPE int32_t

  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)

    #define FIXEDPT_BITS 32

    #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8)
      #define FIXEDPT_WBITS 24
    #elif defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      #define FIXEDPT_WBITS 22
    #else
      #error "Only 24.8 and 22.10 floats are currently supported"
    #endif

    #include "fixedptc.h"

  #endif

#elif defined(VARIABLE_STORAGE_INT16)

  #define VARIABLE_TYPE int16_t
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
    #error "Fixed Point Floats are Supported for 32bit Storage Only!"
  #endif

#else

  #error "Only INT32 and INT16 variable types are supported."

#endif

#define UBASIC_STATUS_RUN   0x80
#define UBASIC_STATUS_MASK_NOT_RUNNING 0x7f
#define UBASIC_STATUS_IDLE  0x00

/* define a structure with bit fields */
typedef union
{
  uint8_t byte;
  struct
  {
    uint8_t bit0 : 1;
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t WaitForSerialInput : 1;
    uint8_t Error       : 1;
    uint8_t isRunning   : 1;
  } bit;
} _Status;

#define MAX_STRINGLEN     40
#define MAX_LABEL_LEN     10

#if defined(VARIABLE_TYPE_STRING)
#define MAX_STRINGVARLEN  64
#define MAX_BUFFERLEN     128
#define GBGCHECK          100
#define MAX_SVARNUM       26
#endif


//
// What it means to support SLEEP:
//    An interrupt routine has to exist which checks the value of
//        'ubasic_script_sleeping_ms'
//    and decrease it by one every ms. As long as this is nonzero
//        ubasic_run()
//    will return immediately and not execute BASIC script.
//    The sleep() sets this to requested value of ms to sleep.
#if defined(UBASIC_SCRIPT_HAVE_SLEEP)
extern volatile uint32_t ubasic_script_sleeping_ms;
#endif


// What it means to support PWM:
#if defined(UBASIC_SCRIPT_HAVE_PWM_CHANNELS)
extern int16_t dutycycle_pwm_ch[UBASIC_SCRIPT_HAVE_PWM_CHANNELS];
void pwm_UpdateDutyCycle(uint8_t ch, int16_t dutycycle);
#endif


//
// What it means to support tic(n) toc(n):
//    In the same interrupt routine that is executed every ms,
//    these variables are increased by one each ms.
//        tic(n)
//    sets the n-th variable to 0, while
//        toc(n)
//    returns how many ms has passed since tic(n) was called.
#if defined(UBASIC_SCRIPT_HAVE_TICTOC)
extern volatile uint32_t ubasic_script_tic0_ms;
extern volatile uint32_t ubasic_script_tic1_ms;
extern volatile uint32_t ubasic_script_tic2_ms;
extern volatile uint32_t ubasic_script_tic3_ms;
extern volatile uint32_t ubasic_script_tic4_ms;
extern volatile uint32_t ubasic_script_tic5_ms;
#endif


#if defined(UBASIC_SCRIPT_HAVE_HARDWARE_EVENTS)
extern volatile uint8_t pb_status, hw_event;
#endif


#if defined(UBASIC_SCRIPT_HAVE_GPIO_CHANNELS)
void DIGIO_ConfGpio(uint8_t ch, uint8_t mode, int8_t pull);
void DIGIO_GetSetGpio(uint8_t ch);
extern int8_t digio_out_ch[UBASIC_SCRIPT_HAVE_GPIO_CHANNELS];
extern int8_t digio_in_ch[UBASIC_SCRIPT_HAVE_GPIO_CHANNELS];
#endif


#if defined(UBASIC_SCRIPT_PRINT_TO_SERIAL)
void print_serial(char * msg);
#else
void print_serial(char * msg){;}
#endif

#if defined(UBASIC_SCRIPT_HAVE_INPUT_FROM_SERIAL)
#define UBASIC_SERIAL_INPUT_MS  50
uint8_t serial_input_available();
uint8_t serial_input (char * buffer, uint8_t len);
extern volatile uint32_t ubasic_script_wait_for_input_ms;
extern uint8_t ubasic_script_wait_for_input_expired;
#endif

#if defined(UBASIC_SCRIPT_HAVE_RANDOM_NUMBER_GENERATOR) || defined(UBASIC_SCRIPT_HAVE_ANALOG_READ)
uint32_t  RandomUInt32(uint8_t size);
#if defined(UBASIC_SCRIPT_HAVE_ANALOG_READ)
uint8_t   SelectChannelADC(uint8_t channel);
uint16_t  AnalogRead(void);
#endif /* UBASIC_SCRIPT_HAVE_ANALOG_READ */
#endif /* UBASIC_SCRIPT_HAVE_RANDOM_NUMBER_GENERATOR || UBASIC_SCRIPT_HAVE_ANALOG_READ */


#endif /* #ifndef _CONFIG_H_ */
