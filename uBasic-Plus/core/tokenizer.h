/*
 * Copyright (c) 2006, Adam Dunkels
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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
  
 /*
 * Modified to support simple string variables and functions by David Mitchell
 * November 2008.
 * Changes and additions are marked 'string additions' throughout
 *
 * Modified to support Plus extension by Marijan Kostrun 2018.
 * 
 */
 
#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include "config.h"

enum {
/*0*/  TOKENIZER_ERROR,
/*1*/  TOKENIZER_ENDOFINPUT,
/*2*/  TOKENIZER_NUMBER,
#if defined(VARIABLE_TYPE_STRING)
/*3*/  TOKENIZER_STRING,
#endif
/*4*/  TOKENIZER_VARIABLE,
#if defined(VARIABLE_TYPE_STRING)
  // string additions - must be here and in this order  
/*5*/  TOKENIZER_STRINGVARIABLE,
/*6*/  TOKENIZER_PRINT$,
/*7*/  TOKENIZER_LEFT$,
/*8*/  TOKENIZER_RIGHT$,
/*9*/  TOKENIZER_MID$,
/*10*/  TOKENIZER_STR$,
/*11*/  TOKENIZER_CHR$,
/*12*/  TOKENIZER_VAL,
/*13*/  TOKENIZER_LEN,
/*14*/  TOKENIZER_INSTR,
/*15*/  TOKENIZER_ASC,
#endif
// end of string additions
/*16*/  TOKENIZER_LET,
/*17*/  TOKENIZER_PRINTLN,
/*18*/  TOKENIZER_PRINT,
/*19*/  TOKENIZER_IF,
/*20*/  TOKENIZER_THEN,
/*21*/  TOKENIZER_ELSE,
/*22*/  TOKENIZER_ENDIF,
/*23*/  TOKENIZER_FOR,
/*24*/  TOKENIZER_TO,
/*25*/  TOKENIZER_NEXT,
/*26*/  TOKENIZER_STEP,
/*27*/  TOKENIZER_WHILE,
/*28*/  TOKENIZER_ENDWHILE,
/*29*/  TOKENIZER_GOTO,
/*30*/  TOKENIZER_GOSUB,
/*31*/  TOKENIZER_RETURN,
/*32*/  TOKENIZER_END,
/*33*/  TOKENIZER_COMMA,
/*34*/  TOKENIZER_PLUS,
/*35*/  TOKENIZER_MINUS,
/*36*/  TOKENIZER_AND,
/*37*/  TOKENIZER_OR,
/*38*/  TOKENIZER_ASTR,
/*39*/  TOKENIZER_SLASH,
/*40*/  TOKENIZER_MOD,
/*41*/  TOKENIZER_LEFTPAREN,
/*42*/  TOKENIZER_RIGHTPAREN,
/*43*/  TOKENIZER_LT,
/*44*/  TOKENIZER_GT,
/*45*/  TOKENIZER_EQ,
/*46*/  TOKENIZER_EOL,
//
// Plus : Start
//
/*47*/  TOKENIZER_NE,
/*48*/  TOKENIZER_GE,
/*49*/  TOKENIZER_LE,
/*50*/  TOKENIZER_LAND,
/*51*/  TOKENIZER_LOR,
/*52*/  TOKENIZER_LNOT,
/*53*/  TOKENIZER_NOT,
/*54*/  TOKENIZER_PRINT_HEX,
/*55*/  TOKENIZER_PRINT_DEC,
#if defined(UBASIC_SCRIPT_HAVE_INPUT_FROM_SERIAL)
/*56*/  TOKENIZER_INPUT,
#endif
#if defined(UBASIC_SCRIPT_HAVE_SLEEP)
/*57*/  TOKENIZER_SLEEP,
#endif
#if defined(UBASIC_SCRIPT_HAVE_GPIO_CHANNELS)
/*58*/  TOKENIZER_PINMODE,
/*59*/  TOKENIZER_DREAD,
/*60*/  TOKENIZER_DWRITE,
#endif
#if defined(VARIABLE_TYPE_ARRAY)
/*61*/  TOKENIZER_DIM,
/*62*/  TOKENIZER_ARRAYVARIABLE,
#endif
#if defined(UBASIC_SCRIPT_HAVE_RANDOM_NUMBER_GENERATOR)
/*63*/  TOKENIZER_RAN,
#if defined(UBASIC_SCRIPT_HAVE_TICTOC)
/*64*/  TOKENIZER_TIC,
/*65*/  TOKENIZER_TOC,
#endif
#endif
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
/*66*/  TOKENIZER_INT,
/*67*/  TOKENIZER_FLOAT,
/*68*/  TOKENIZER_SQRT,
/*69*/  TOKENIZER_SIN,
/*70*/  TOKENIZER_COS,
/*71*/  TOKENIZER_TAN,
/*72*/  TOKENIZER_EXP,
/*73*/  TOKENIZER_LN,
  #if defined(UBASIC_SCRIPT_HAVE_RANDOM_NUMBER_GENERATOR)
/*74*/  TOKENIZER_UNIFORM,
  #endif
/*75*/  TOKENIZER_ABS,
/*76*/  TOKENIZER_FLOOR,
/*77*/  TOKENIZER_CEIL,
/*78*/  TOKENIZER_ROUND,
/*79*/  TOKENIZER_POWER,
#endif
#if defined(UBASIC_SCRIPT_HAVE_HARDWARE_EVENTS)
/*80*/  TOKENIZER_HWE,
#endif
#if defined(UBASIC_SCRIPT_HAVE_PWM_CHANNELS)
/*81*/  TOKENIZER_PWMCONF,
/*82*/  TOKENIZER_PWM,
#endif
#if defined(UBASIC_SCRIPT_HAVE_ANALOG_READ)
/*83*/  TOKENIZER_AREADCONF,
/*84*/  TOKENIZER_AREAD,
#endif
/*85*/  TOKENIZER_LABEL,
/*86*/  TOKENIZER_COLON,
  //
  // Plus: End
  // 
};

void tokenizer_init(const char *program);
void tokenizer_next(void);
uint8_t tokenizer_token(void);
VARIABLE_TYPE tokenizer_num(void);
VARIABLE_TYPE tokenizer_int(void);

#ifdef FIXEDPT_FBITS
VARIABLE_TYPE tokenizer_float(void);
#endif

uint8_t tokenizer_variable_num(void);

uint8_t tokenizer_finished(void);
void tokenizer_error_print(VARIABLE_TYPE token);

#if defined(VARIABLE_TYPE_STRING)
void tokenizer_string(char *dest, uint8_t len);
// string addition
int8_t tokenizer_stringlookahead(void);
// end of string addition
#endif

void tokenizer_label(char *dest, uint8_t len);
uint16_t  tokenizer_save_offset(void);
void      tokenizer_jump_offset(uint16_t);
uint16_t  tokenizer_line_number(void);

// string addition
#endif /* __TOKENIZER_H__ */
