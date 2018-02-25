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

/* Includes: Generic ----------------------------------------------------------*/
#include "config.h"
#include "ubasic.h"
#include "tokenizer.h"

/**
  * uBASIC Global and Exported Variables: Start
  * 
  */
volatile _Status ubasic_status = {.bit.notInitialized=1};
uint8_t   input_how=0;

/**
 * uBASIC Global and Exported Variables: End
 *
 */

#if defined(VARIABLE_TYPE_ARRAY)
static VARIABLE_TYPE  arrays_data[VARIABLE_TYPE_ARRAY] = {0};
static int16_t        free_arrayptr = 0;
static int16_t        arrayvariable[MAX_VARNUM] = {-1};
#endif


static char const *program_ptr;

#define MAX_GOSUB_STACK_DEPTH 10
static uint16_t gosub_stack[MAX_GOSUB_STACK_DEPTH];
static uint8_t gosub_stack_ptr;

#define MAX_FOR_STACK_DEPTH 4
struct for_state {
  uint16_t line_after_for;
  uint8_t  for_variable;
  VARIABLE_TYPE to;
  VARIABLE_TYPE step;
};
static struct for_state for_stack[MAX_FOR_STACK_DEPTH];
static uint8_t for_stack_ptr;

#define MAX_IF_STACK_DEPTH  4
static int16_t if_stack[MAX_IF_STACK_DEPTH];
static uint8_t if_stack_ptr;

#define MAX_WHILE_STACK_DEPTH 4
struct while_state {
  uint16_t line_while;
  int16_t line_after_endwhile;
};
static struct while_state while_stack[MAX_WHILE_STACK_DEPTH];
static uint8_t while_stack_ptr;

VARIABLE_TYPE variables[MAX_VARNUM];

static VARIABLE_TYPE relation(void);
static void numbered_line_statement(void);
static void statement(void);
static char string[MAX_STRINGLEN];

#if defined(VARIABLE_TYPE_STRING)
static char stringbuffer[MAX_BUFFERLEN];
static uint16_t freebufptr = 0;
static char *stringvariables[MAX_SVARNUM];
static const char nullstring[] = "\0";
static char* sexpr(void);
static char* scpy(char *);
static char* sconcat(char *, char *);
static char* sleft(char *, uint16_t); 
static char* sright(char *,uint16_t);
static char* smid(char *, uint16_t, uint16_t);
static char* sstr(uint16_t);
static char* schr(uint16_t);
static uint8_t sinstr(uint16_t, char*, char*);
#endif

#if defined(UBASIC_SCRIPT_HAVE_INPUT_FROM_SERIAL)
static uint8_t input_varnum;
static uint8_t input_type;
#if defined(VARIABLE_TYPE_ARRAY)
VARIABLE_TYPE  input_array_index;
#endif
#endif

#if defined(UBASIC_SCRIPT_HAVE_STORE_VARS_IN_FLASH)
static VARIABLE_TYPE recall_statement(void);
#endif

/*---------------------------------------------------------------------------*/
void ubasic_clear_variables()
{
  int16_t i;
  for (i=0; i<MAX_VARNUM; i++)
  {
    variables[i] = 0;
#if defined(VARIABLE_TYPE_ARRAY)
    arrayvariable[i] = -1;
#endif
  }

#if defined(VARIABLE_TYPE_ARRAY)
  free_arrayptr = 0;
  for (i=0; i<VARIABLE_TYPE_ARRAY; i++)
  {
    arrays_data[i] = 0;
  }
#endif

#if defined(VARIABLE_TYPE_STRING)
  string[0] = 0;
  freebufptr = 0;
  for (i=0; i<MAX_SVARNUM; i++)
    stringvariables[i] = scpy((char *)nullstring);
#endif
}

/*---------------------------------------------------------------------------*/
void ubasic_load_program(const char *program)
{
  for_stack_ptr = gosub_stack_ptr = 0;
  if (ubasic_status.bit.notInitialized)
  {
    ubasic_clear_variables();
  }
  ubasic_status.byte= 0x00;
  if (program)
  {
    program_ptr = program;
    tokenizer_init(program);
    ubasic_status.bit.isRunning = 1;
  }
}
/*---------------------------------------------------------------------------*/
static uint8_t accept(VARIABLE_TYPE token)
{

  if(token != tokenizer_token())
  {
    tokenizer_error_print(token);
    return 1;
  }

  tokenizer_next();
  return 0;
}
// string additions

/*---------------------------------------------------------------------------*/
static void accept_cr()
{
  while ( (tokenizer_token() != TOKENIZER_EOL) &&
           (tokenizer_token() != TOKENIZER_ERROR) &&
           (tokenizer_token() != TOKENIZER_ENDOFINPUT) )
  {
    tokenizer_next();
  }

  if (tokenizer_token() == TOKENIZER_EOL)
    tokenizer_next();
}

#if defined(VARIABLE_TYPE_STRING)

/*---------------------------------------------------------------------------*/
uint8_t string_space_check(uint16_t l)
{
   // returns true if not enough room for new string
  uint8_t i;
  i = ((MAX_BUFFERLEN - freebufptr) <= (l + 2)); // +2 to play it safe
  if (i)
  {
    ubasic_status.bit.isRunning = 0;
    ubasic_status.bit.Error = 1;
  }
  return i;
}

/*---------------------------------------------------------------------------*/
void garbage_collect(void)
{
  uint16_t totused = 0;
  uint16_t i;
  char *temp;
  char *tp;

  if (freebufptr < GBGCHECK)
     return;

  for (i=0; i< MAX_SVARNUM; i++)
  {
    // calculate used space
    totused += strlen(stringvariables[i]) + 1;
  }
  temp = malloc(totused); // alloc temporary space to store vars
  tp = temp;
  for (i=0; i< MAX_SVARNUM; i++)
  {
    // copy used strings to temporary store
    strcpy(tp, stringvariables[i]);
    tp += strlen(tp) + 1;
  }

  freebufptr = 0;
  tp = temp;
  for (i=0; i< MAX_SVARNUM; i++)
  {
    //copy back to buffer
    stringvariables[i] = scpy(tp);
    tp+= strlen(tp) + 1;
  }

  free(temp); // free temp space
 }
/*---------------------------------------------------------------------------*/
static char* scpy(char *s1) // return a copy of s1
{
  uint16_t bp = freebufptr;
  uint16_t l;

  l = strlen(s1);

  if (string_space_check(l))
    return (char*)nullstring;

  strcpy(stringbuffer+bp, s1);

  freebufptr = bp + l + 1;

  return (stringbuffer+bp);
}
   
/*---------------------------------------------------------------------------*/
static char* sconcat(char *s1, char*s2)
{
  // return the concatenation of s1 and s2
  uint16_t bp = freebufptr;
  uint16_t rp = bp;
  uint16_t l1, l2;
  l1 = strlen(s1);
  l2 = strlen(s2);
  if (string_space_check(l1+l2))
     return (char*)nullstring;

  strcpy((stringbuffer+bp), s1);
  bp += l1;
  if (l1 == MAX_STRINGVARLEN)
  {
    freebufptr = bp + 1;
    return (stringbuffer + rp); 
  }

  l2 = strlen(s2);
  strcpy((stringbuffer+bp), s2);
  if (l1 + l2 > MAX_STRINGVARLEN)
  {
    l2 = MAX_STRINGVARLEN - l1;
    // truncate
    *(stringbuffer + bp + l2) = '\0';
  }   
  freebufptr = bp + l2 + 1;
  return (stringbuffer+rp);
}
/*---------------------------------------------------------------------------*/
static char* sleft(char *s1, uint16_t l) // return the left l chars of s1
{
   uint16_t bp = freebufptr;
   uint16_t rp = bp;
   uint16_t i;
   if (l<1) 
     return scpy((char*)nullstring);
   if (string_space_check(l))
     return (char*)nullstring;
   if (strlen(s1) <=l) {
      return scpy(s1);
   } else {
      for (i=0; i<l; i++) {
       *(stringbuffer +bp) = *s1;
     bp++;
     s1++;
    }
    *(stringbuffer + bp) = '\0';
    freebufptr = bp+1;
  
   }
   return stringbuffer + rp;
}
/*---------------------------------------------------------------------------*/
static char* sright(char *s1, uint16_t l) // return the right l chars of s1
{
  uint16_t bp = freebufptr;
  uint16_t rp = bp;
  uint16_t i, j;
   j = strlen(s1);
   if (l<1) 
     return scpy((char*)nullstring);
   if (string_space_check(l))
     return (char*)nullstring;
   if (j <= l) {
      return scpy(s1);
   } else {
      for (i=0; i<l; i++) {
       *(stringbuffer + bp) = *(s1 + j-l);
     bp++;
     s1++;
    }
    *(stringbuffer + bp) = '\0';
    freebufptr = bp+1;
  
   }
   return stringbuffer + rp;
}
/*---------------------------------------------------------------------------*/
static char* smid(char *s1, uint16_t l1, uint16_t l2) // return the l2 chars of s1 starting at offset l1
{
  uint16_t bp = freebufptr;
  uint16_t rp = bp;
  uint16_t i, j;
   j = strlen(s1);
   if (l2<1 || l1>j) 
     return scpy((char*)nullstring);
   if (string_space_check(l2))
     return (char*)nullstring;
   if (l2 > j-l1)
     l2 = j-l1;
   for (i=l1; i<l1+l2; i++) {
      *(stringbuffer + bp) = *(s1 + l1 -1);
      bp++;
      s1++;
   }
   *(stringbuffer + bp) = '\0';
   freebufptr = bp+1;
   return stringbuffer + rp;
}
/*---------------------------------------------------------------------------*/
static char* sstr(uint16_t j) // return the integer j as a string
{
  uint16_t bp = freebufptr;
  uint16_t rp = bp;
   if (string_space_check(10))
     return (char*)nullstring;
   sprintf((stringbuffer+bp),"%d",j);
   freebufptr = bp + strlen(stringbuffer+bp) + 1;
   return stringbuffer + rp;
}
/*---------------------------------------------------------------------------*/
static char* schr(uint16_t j) // return the character whose ASCII code is j
{
  uint16_t bp = freebufptr;
  uint16_t rp = bp;

   if (string_space_check(1))
     return (char*)nullstring;
   sprintf((stringbuffer+bp),"%c",j);
   freebufptr = bp + 2;
   return stringbuffer + rp;
}
/*---------------------------------------------------------------------------*/
static uint8_t sinstr(uint16_t j, char *s, char *s1) // return the position of s1 in s (or 0)
{
   char *p;
   p = strstr(s+j, s1);
   if (p == NULL)
      return 0;
   return (p - s + 1);
}

/*---------------------------------------------------------------------------*/
char* sfactor()
{
  // string form of factor
  char *r=0, *s=0;

  VARIABLE_TYPE i, j;

  switch(tokenizer_token())
  {
    case TOKENIZER_LEFTPAREN:
      accept(TOKENIZER_LEFTPAREN);
      r = sexpr();
      accept(TOKENIZER_RIGHTPAREN);
      break;

    case TOKENIZER_STRING:
      tokenizer_string(string, sizeof(string));
      r = scpy(string);
      accept(TOKENIZER_STRING);
      break;

    case TOKENIZER_LEFT$:
      accept(TOKENIZER_LEFT$);
      accept(TOKENIZER_LEFTPAREN);
      s = sexpr();
      accept(TOKENIZER_COMMA);
      i =relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      i = fixedpt_toint(i);
  #endif
      r = sleft(s,i);
      accept(TOKENIZER_RIGHTPAREN);
      break;

    case TOKENIZER_RIGHT$:
      accept(TOKENIZER_RIGHT$);
      accept(TOKENIZER_LEFTPAREN);
      s = sexpr();
      accept(TOKENIZER_COMMA);
      i =relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      i = fixedpt_toint(i);
  #endif
      r = sright(s,i);
      accept(TOKENIZER_RIGHTPAREN);
      break;

    case TOKENIZER_MID$:
      accept(TOKENIZER_MID$);
      accept(TOKENIZER_LEFTPAREN);
      s = sexpr();
      accept(TOKENIZER_COMMA);
      i =relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      i = fixedpt_toint(i);
  #endif
      if (tokenizer_token() == TOKENIZER_COMMA)
      {
        accept(TOKENIZER_COMMA);
        j =relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
        j = fixedpt_toint(j);
  #endif
      }
      else
      {
        j = 999; // ensure we get all of it
      }
      r = smid(s,i,j);
      accept(TOKENIZER_RIGHTPAREN);
      break;

    case TOKENIZER_STR$:
      accept(TOKENIZER_STR$);
      j =relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      j = fixedpt_toint(j);
  #endif
      r = sstr(j);
      break;

    case TOKENIZER_CHR$:
      accept(TOKENIZER_CHR$);
      j =relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      j = fixedpt_toint(j);
  #endif
      if (j<0 || j>255)
        j = 0;
      r = schr(j);
      break;

    default:
      r = ubasic_get_stringvariable(tokenizer_variable_num());
      accept(TOKENIZER_STRINGVARIABLE);
  }

  return r;
}

/*---------------------------------------------------------------------------*/

static char* sexpr(void) // string form of expr
{
  char *s1, *s2;
  s1 = sfactor();
  uint8_t op = tokenizer_token();
  while(op == TOKENIZER_PLUS)
  {
    tokenizer_next();
    s2 = sfactor();
    s1 = sconcat(s1,s2);
    op = tokenizer_token();
  }
  return s1;
}
/*---------------------------------------------------------------------------*/
static uint8_t slogexpr() // string logical expression
{
   char *s1, *s2;
   uint8_t r = 0;
   s1 = sexpr();
   uint8_t op = tokenizer_token();
   tokenizer_next();
   if(op == TOKENIZER_EQ)
   {
     s2 = sexpr();
     r = (strcmp(s1,s2) == 0);
   }
   return r;
}
// end of string additions
#endif

/*---------------------------------------------------------------------------*/
static VARIABLE_TYPE varfactor(void)
{
  VARIABLE_TYPE r;
  r = ubasic_get_variable(tokenizer_variable_num());
  accept(TOKENIZER_VARIABLE);
  return r;
}
/*---------------------------------------------------------------------------*/
static VARIABLE_TYPE factor(void)
{
  VARIABLE_TYPE r;
  // string function additions
  VARIABLE_TYPE i, j;
#if defined(VARIABLE_TYPE_ARRAY)
  uint8_t varnum;
#endif
#if defined(VARIABLE_TYPE_STRING)
  char *s, *s1;
#endif

  switch(tokenizer_token())
  {
#if defined(VARIABLE_TYPE_STRING)

    case TOKENIZER_LEN:
      accept(TOKENIZER_LEN);
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      r = fixedpt_fromint(strlen(sexpr()));
  #else
      r = strlen(sexpr());
  #endif
      break;


    case TOKENIZER_VAL:
      accept(TOKENIZER_VAL);
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      s1 = sexpr();
      r = str_fixedpt(s1, strlen(s1), 3);
  #else
      r = atoi(sexpr());
  #endif
      break;


    case TOKENIZER_ASC:
      accept(TOKENIZER_ASC);
      s = sexpr();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      r = fixedpt_fromint(*s);
  #else
      r = *s;
  #endif
      break;


    case TOKENIZER_INSTR:
      accept(TOKENIZER_INSTR);
      accept(TOKENIZER_LEFTPAREN);
      j = 1;
      if (tokenizer_token() == TOKENIZER_NUMBER)
      {
        j = tokenizer_num();
        accept(TOKENIZER_NUMBER);
        accept(TOKENIZER_COMMA);
      }
      if (j <1)
        return 0;
      s = sexpr();
      accept(TOKENIZER_COMMA);
      s1 = sexpr();
      accept(TOKENIZER_RIGHTPAREN);
      r = sinstr(j, s, s1);
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      r = fixedpt_fromint(r);
  #endif
      break;
      // end of string additions
#endif

    case TOKENIZER_MINUS:
      accept(TOKENIZER_MINUS);
      r = - factor();
      break;

    case TOKENIZER_LNOT:
      accept(TOKENIZER_LNOT);
      r = ! relation();
      break;

    case TOKENIZER_NOT:
      accept(TOKENIZER_LNOT);
      r = ~ relation();
      break;

#if defined(UBASIC_SCRIPT_HAVE_TICTOC)
    case TOKENIZER_TOC:
      accept(TOKENIZER_TOC);
      accept(TOKENIZER_LEFTPAREN);
      r =relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      r = fixedpt_toint(r);
  #endif
      if (r == 2)
        r = ubasic_script_tic1_ms;
      else if (r == 3)
        r = ubasic_script_tic2_ms;
      else if (r == 4)
        r = ubasic_script_tic3_ms;
      else if (r == 5)
        r = ubasic_script_tic4_ms;
      else if (r == 6)
        r = ubasic_script_tic5_ms;
      else
        r = ubasic_script_tic0_ms;
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      r = fixedpt_fromint(r);
      accept(TOKENIZER_RIGHTPAREN);
  #endif
      break;
#endif


#if defined(UBASIC_SCRIPT_HAVE_HARDWARE_EVENTS)
    case TOKENIZER_HWE:
      accept(TOKENIZER_HWE);
      accept(TOKENIZER_LEFTPAREN);
      r = relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      r = fixedpt_toint(r);
  #endif
      if (r)
      {
        if ( hw_event & (1<<(r-1)) )
        {
          hw_event -= 0x01<<(r-1);
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
          r = FIXEDPT_ONE;
  #endif
        }
        else
          r = 0;
      }
      accept(TOKENIZER_RIGHTPAREN);
      break;
#endif /* #if defined(UBASIC_SCRIPT_HAVE_HARDWARE_EVENTS) */


#if defined(UBASIC_SCRIPT_HAVE_RANDOM_NUMBER_GENERATOR)
    case TOKENIZER_RAN:
      accept(TOKENIZER_RAN);
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      r = RandomUInt32(FIXEDPT_WBITS);
      r = fixedpt_fromint(r);
  #else
      r = RandomUInt32(32);
  #endif
      if (r<0)
        r=-r;
      break;
#endif

    case TOKENIZER_ABS:
      accept(TOKENIZER_ABS);
      accept(TOKENIZER_LEFTPAREN);
      r = relation();
      if (r<0)
        r = -r;
      accept(TOKENIZER_RIGHTPAREN);
      break;


#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
    case TOKENIZER_POWER:
      accept(TOKENIZER_POWER);
      accept(TOKENIZER_LEFTPAREN);
      // argument:
      i = relation();
      accept(TOKENIZER_COMMA);
      // exponent
      j = relation();
      r = fixedpt_pow(i,j);
      accept(TOKENIZER_RIGHTPAREN);
      break;


    case TOKENIZER_FLOAT:
      r = tokenizer_float(); /* 24.8 deciman number */
      accept(TOKENIZER_FLOAT);
      break;


    case TOKENIZER_SQRT:
      accept(TOKENIZER_SQRT);
      accept(TOKENIZER_LEFTPAREN);
      r = fixedpt_sqrt( relation() );
      accept(TOKENIZER_RIGHTPAREN);
      break;


    case TOKENIZER_SIN:
      accept(TOKENIZER_SIN);
      accept(TOKENIZER_LEFTPAREN);
      r = fixedpt_sin( relation() );
      accept(TOKENIZER_RIGHTPAREN);
      break;


    case TOKENIZER_COS:
      accept(TOKENIZER_COS);
      accept(TOKENIZER_LEFTPAREN);
      r = fixedpt_cos( relation() );
      accept(TOKENIZER_RIGHTPAREN);
      break;


    case TOKENIZER_TAN:
      accept(TOKENIZER_TAN);
      accept(TOKENIZER_LEFTPAREN);
      r = fixedpt_tan( relation() );
      accept(TOKENIZER_RIGHTPAREN);
      break;


    case TOKENIZER_EXP:
      accept(TOKENIZER_EXP);
      accept(TOKENIZER_LEFTPAREN);
      r = fixedpt_exp( relation() );
      accept(TOKENIZER_RIGHTPAREN);
      break;


    case TOKENIZER_LN:
      accept(TOKENIZER_LN);
      accept(TOKENIZER_LEFTPAREN);
      r = fixedpt_ln( relation() );
      accept(TOKENIZER_RIGHTPAREN);
      break;


  #if defined(UBASIC_SCRIPT_HAVE_RANDOM_NUMBER_GENERATOR)
    case TOKENIZER_UNIFORM:
      accept(TOKENIZER_UNIFORM);
      r = RandomUInt32(FIXEDPT_FBITS) & FIXEDPT_FMASK;
      break;
  #endif


    case TOKENIZER_FLOOR:
      accept(TOKENIZER_FLOOR);
      accept(TOKENIZER_LEFTPAREN);
      r = relation();
      if (r>=0)
      {
        r = r & (~FIXEDPT_FMASK);
      }
      else
      {
        uint32_t f = (r & FIXEDPT_FMASK);
        r = r & (~FIXEDPT_FMASK);
        if (f>0)
          r -= FIXEDPT_ONE;
      }
      accept(TOKENIZER_RIGHTPAREN);
      break;


    case TOKENIZER_CEIL:
      accept(TOKENIZER_CEIL);
      accept(TOKENIZER_LEFTPAREN);
      r = relation();
      if (r>=0)
      {
        uint32_t f = (r & FIXEDPT_FMASK);
        r = r & (~FIXEDPT_FMASK);
        if (f>0)
          r += FIXEDPT_ONE;
      }
      else
      {
        r = r & (~FIXEDPT_FMASK);
      }
      accept(TOKENIZER_RIGHTPAREN);
      break;


    case TOKENIZER_ROUND:
      accept(TOKENIZER_ROUND);
      accept(TOKENIZER_LEFTPAREN);
      r = relation();
      uint32_t f = (r & FIXEDPT_FMASK);
      if (r>=0)
      {
        r = r & (~FIXEDPT_FMASK);
        if (f>=FIXEDPT_ONE_HALF)
          r += FIXEDPT_ONE;
      }
      else
      {
        r = r & (~FIXEDPT_FMASK);
        if (f<=FIXEDPT_ONE_HALF)
          r -= FIXEDPT_ONE;
      }
      accept(TOKENIZER_RIGHTPAREN);
      break;
#endif /* #if defined(VARIABLE_TYPE_FLOAT_AS ... */

    case TOKENIZER_INT:
      r = tokenizer_int();
      accept(TOKENIZER_INT);
      break;

    case TOKENIZER_NUMBER:
      r = tokenizer_num();
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      r = fixedpt_fromint(r);
#endif
      accept(TOKENIZER_NUMBER);
      break;


#ifdef UBASIC_SCRIPT_HAVE_PWM_CHANNELS
    case TOKENIZER_PWM:
      accept(TOKENIZER_PWM);
      accept(TOKENIZER_LEFTPAREN);
      // single argument: channel
      j = relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      j = fixedpt_toint(j);
  #endif
      if (j <1 || j>UBASIC_SCRIPT_HAVE_PWM_CHANNELS)
      {
        r = -1;
      }
      else
      {
        r = dutycycle_pwm_ch[j-1];
      }
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      r = fixedpt_fromint(r);
  #endif
      accept(TOKENIZER_RIGHTPAREN);
      break;
#endif


#if defined(UBASIC_SCRIPT_HAVE_ANALOG_READ)
    case TOKENIZER_AREAD:
      accept(TOKENIZER_AREAD);
      accept(TOKENIZER_LEFTPAREN);
      // single argument: channel as hex value
      j = relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      j = fixedpt_toint(j);
  #endif
      r = analogRead(j);
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      r = fixedpt_fromint(r);
  #endif
      accept(TOKENIZER_RIGHTPAREN);
      break;
#endif


    case TOKENIZER_LEFTPAREN:
      accept(TOKENIZER_LEFTPAREN);
      r = relation();
      accept(TOKENIZER_RIGHTPAREN);
      break;


#if defined(VARIABLE_TYPE_ARRAY)
    case TOKENIZER_ARRAYVARIABLE:
      varnum = tokenizer_variable_num();
      accept(TOKENIZER_ARRAYVARIABLE);
      j = relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      j = fixedpt_toint(j);
  #endif
      r = ubasic_get_arrayvariable(varnum , (uint16_t) j);
      break;
#endif

#if defined(UBASIC_SCRIPT_HAVE_GPIO_CHANNELS)
    case TOKENIZER_DREAD:
      accept(TOKENIZER_LEFTPAREN);
      r = relation();
      r = digitalRead(r);
      accept(TOKENIZER_RIGHTPAREN);
      break;
#endif /* UBASIC_SCRIPT_HAVE_GPIO_CHANNELS */

#if defined(UBASIC_SCRIPT_HAVE_STORE_VARS_IN_FLASH)
    case TOKENIZER_RECALL:
      r = recall_statement();
      break;
#endif

    default:
      r = varfactor();
      break;
  }

  return r;
}

/*---------------------------------------------------------------------------*/

static VARIABLE_TYPE term(void)
{
  VARIABLE_TYPE f1, f2;
#if defined(VARIABLE_TYPE_STRING)
  if (tokenizer_stringlookahead())
  {
    f1 = slogexpr();
  }
  else
#endif
  {
    f1 = factor();
    VARIABLE_TYPE op = tokenizer_token();
    while (op == TOKENIZER_ASTR || op == TOKENIZER_SLASH || op == TOKENIZER_MOD)
    {
      tokenizer_next();
      f2 = factor();
      switch(op)
      {

        case TOKENIZER_ASTR:
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
          f1 = fixedpt_xmul(f1,f2);
#else
          f1 = f1 * f2;
#endif
          break;

        case TOKENIZER_SLASH:
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
          f1 = fixedpt_xdiv(f1,f2);
#else
          f1 = f1 / f2;
#endif
          break;

        case TOKENIZER_MOD:
          f1 = f1 % f2;
          break;
      }
      op = tokenizer_token();
    }
  }
  return f1;
}
/*---------------------------------------------------------------------------*/
static VARIABLE_TYPE relation(void)
{
  VARIABLE_TYPE r1, r2;

  r1 = (VARIABLE_TYPE) term();

  VARIABLE_TYPE op = tokenizer_token();

  while ( op == TOKENIZER_LT || op == TOKENIZER_LE ||
          op == TOKENIZER_GT || op == TOKENIZER_GE ||
          op == TOKENIZER_EQ || op == TOKENIZER_NE ||
          op == TOKENIZER_LAND || op == TOKENIZER_LOR ||
          op == TOKENIZER_PLUS || op == TOKENIZER_MINUS ||
          op == TOKENIZER_AND || op == TOKENIZER_OR )
  {
    tokenizer_next();
    r2 = (VARIABLE_TYPE) term();

    switch(op)
    {
      case TOKENIZER_LE:
        r1 = (r1 <= r2);
        break;

      case TOKENIZER_LT:
        r1 = (r1 < r2);
        break;

      case TOKENIZER_GT:
        r1 = (r1 > r2);
        break;

      case TOKENIZER_GE:
        r1 = (r1 >= r2);
        break;

      case TOKENIZER_EQ:
        r1 = (r1 == r2);
        break;

      case TOKENIZER_NE:
        r1 = (r1 != r2);
        break;

      case TOKENIZER_LAND:
        r1 = (r1 && r2);
        break;

      case TOKENIZER_LOR:
        r1 = (r1 || r2);
        break;

      case TOKENIZER_PLUS:
        r1 = r1 + r2;
        break;

      case TOKENIZER_MINUS:
        r1 = r1 - r2;
        break;

      case TOKENIZER_AND:
        r1 = ((int32_t) r1) & ((int32_t) r2);
        break;

      case TOKENIZER_OR:
        r1 = ((int32_t) r1) | ((int32_t) r2);
        break;
    }
    op = tokenizer_token();
  }

  return r1;
}

// TODO: error handling?
uint8_t jump_label(char * label)
{
  char currLabel[MAX_LABEL_LEN] = { '\0' };

  tokenizer_init(program_ptr);

  while (tokenizer_token() != TOKENIZER_ENDOFINPUT)
  {
    tokenizer_next();

    if (tokenizer_token() == TOKENIZER_COLON)
    {
      tokenizer_next();
      if (tokenizer_token() == TOKENIZER_LABEL)
      {
        tokenizer_label(currLabel, sizeof(currLabel));
        if(strcmp(label, currLabel) == 0)
        {
          accept(TOKENIZER_LABEL);
          return 1;
        }
      }
    }
  }

  if (tokenizer_token() == TOKENIZER_ENDOFINPUT)
    return 0;

  return 1;
}


static void gosub_statement(void)
{
  accept(TOKENIZER_GOSUB);

  if(tokenizer_token() == TOKENIZER_LABEL)
  {
    // copy label
    tokenizer_label(string, sizeof(string));
    tokenizer_next();

    // check for the end of line 
    while (tokenizer_token() == TOKENIZER_EOL)
      tokenizer_next();
 //     accept_cr();

    if(gosub_stack_ptr < MAX_GOSUB_STACK_DEPTH)
    {
      /*    tokenizer_line_number_inc();*/
      //gosub_stack[gosub_stack_ptr] = tokenizer_line_number();
      gosub_stack[gosub_stack_ptr] = tokenizer_save_offset();
      gosub_stack_ptr++;
      jump_label(string);
      return;
    }
  }

  tokenizer_error_print(TOKENIZER_GOSUB);
  ubasic_status.bit.isRunning = 0;
  ubasic_status.bit.Error = 1;
}


static void return_statement(void)
{
  accept(TOKENIZER_RETURN);
  if(gosub_stack_ptr > 0)
  {
    gosub_stack_ptr--;
    //jump_line(gosub_stack[gosub_stack_ptr]);
    tokenizer_jump_offset(gosub_stack[gosub_stack_ptr]);
    return;
  }

  tokenizer_error_print(TOKENIZER_RETURN);
  ubasic_status.bit.isRunning = 0;
  ubasic_status.bit.Error = 1;
}


static void goto_statement(void)
{
  accept(TOKENIZER_GOTO);

  if(tokenizer_token() == TOKENIZER_LABEL)
  {
    tokenizer_label(string, sizeof(string));
    tokenizer_next();
    jump_label(string);
    return;
  }

  tokenizer_error_print(TOKENIZER_GOTO);
  ubasic_status.bit.isRunning = 0;
  ubasic_status.bit.Error = 1;
}


/*---------------------------------------------------------------------------*/
#ifdef UBASIC_SCRIPT_HAVE_PWM_CHANNELS
static void pwm_statement(void)
{
  VARIABLE_TYPE j,r;

  accept(TOKENIZER_PWM);

  accept(TOKENIZER_LEFTPAREN);

  // first argument: channel
  j = relation();
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
  j = fixedpt_toint(j);
#endif
  if (j <1 || j>4)
    return;

  accept(TOKENIZER_COMMA);

  // second argument: value
  r = relation();
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
  r = fixedpt_toint(r);
#endif
  accept(TOKENIZER_RIGHTPAREN);

  if (j>=1 && j<=UBASIC_SCRIPT_HAVE_PWM_CHANNELS)
  {
    analogWrite(j, r);
  }

  accept_cr();
  return;
}

static void pwmconf_statement(void)
{
  VARIABLE_TYPE j,r;

  accept(TOKENIZER_PWMCONF);
  accept(TOKENIZER_LEFTPAREN);
  // first argument: prescaler 0...
  j = relation();
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
  j = fixedpt_toint(j);
#endif
  if (j<0)
    j = 0;

  accept(TOKENIZER_COMMA);
  r = relation();
  // second argument: period
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
  r = fixedpt_toint(r);
#endif
  analogWriteConfig(j,r);
  r = 0;
  accept(TOKENIZER_RIGHTPAREN);
  accept_cr();
}


#endif

#if defined(UBASIC_SCRIPT_HAVE_ANALOG_READ)
static void areadconf_statement(void)
{
  VARIABLE_TYPE j,r;

  accept(TOKENIZER_AREADCONF);
  accept(TOKENIZER_LEFTPAREN);
  // first argument: sampletime 0...7 on STM32
  j = relation();
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
  j = fixedpt_toint(j);
#endif
  if (j<0)
    j = 0;
  if (j>7)
    j = 7;
  accept(TOKENIZER_COMMA);
  r = relation();
  // second argument: number of analog sample to average from 
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
  r = fixedpt_toint(r);
#endif
  analogReadConfig(j,r);
  accept(TOKENIZER_RIGHTPAREN);
  accept_cr();
}
#endif

#if defined(UBASIC_SCRIPT_HAVE_GPIO_CHANNELS)
static void pinmode_statement(void)
{
  VARIABLE_TYPE i,j,r;
  accept(TOKENIZER_PINMODE);
  accept(TOKENIZER_LEFTPAREN);

  // channel - should be entered as 0x..
  i = relation();
  if (i < 0xa0 || i > 0xff)
    return;

  accept(TOKENIZER_COMMA);

  // mode
  j = relation();

#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
  j = fixedpt_toint(j);
#endif

  if (j<-2)
    j = -1;
  if (j>2)
    j = 0;

  accept(TOKENIZER_COMMA);

  // speed
  r = relation();
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
  r = fixedpt_toint(r);
#endif
  if (r<0 || r>2)
    r = 0;

  accept(TOKENIZER_RIGHTPAREN);

  pinMode((uint8_t) i, (int8_t) j, (int8_t) r);

  accept_cr();

  return;
}


static void dwrite_statemet(void)
{
  VARIABLE_TYPE j,r;
  accept(TOKENIZER_DWRITE);
  accept(TOKENIZER_LEFTPAREN);

  j = relation();

  accept(TOKENIZER_COMMA);

  r = relation();
  if (r)
    r = 0x01;

  accept(TOKENIZER_RIGHTPAREN);

  r = digitalWrite(j,r);
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
  r = fixedpt_fromint(r);
#endif

  accept_cr();

  return;
}

#endif /* UBASIC_SCRIPT_HAVE_ANALOG_READ */


static void print_statement(uint8_t println)
{
  uint8_t print_how=0; /*0-xp, 1-hex, 2-oct, 3-dec, 4-bin*/
  // string additions
  if (println)
    accept(TOKENIZER_PRINTLN);
  else
    accept(TOKENIZER_PRINT);

  do
  {
    if (tokenizer_token() == TOKENIZER_PRINT_HEX)
    {
      tokenizer_next();
      print_how = 1;
    }
    else if (tokenizer_token() == TOKENIZER_PRINT_DEC)
    {
      tokenizer_next();
      print_how = 2;
    }

#if defined(VARIABLE_TYPE_STRING)
    if(tokenizer_token() == TOKENIZER_STRING)
    {
      tokenizer_string(string, sizeof(string));
      tokenizer_next();
    }
    else
#endif
    if(tokenizer_token() == TOKENIZER_COMMA)
    {
      sprintf(string, " ");
      tokenizer_next();
    }
    else
    {
#if defined(VARIABLE_TYPE_STRING)
      if (tokenizer_stringlookahead())
      {
        sprintf(string, "%s", sexpr());
      }
      else
#endif
      {
        if (print_how == 1)
        {
          sprintf(string, "%lx", (uint32_t) relation());
        }
        else if (print_how == 2)
        {
          sprintf(string, "%ld", relation());
        }
        else
        {
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
          fixedpt_str( relation(), string, FIXEDPT_FBITS/3 );
#else
          sprintf(string, "%ld", relation());
#endif
        }
      }
    // end of string additions
    }
    print_serial(string);
  }
  while ( tokenizer_token() != TOKENIZER_EOL &&
          tokenizer_token() != TOKENIZER_ENDOFINPUT );

  // printf("\n");
  if (println)
    print_serial("\n");

  accept_cr();
}
/*---------------------------------------------------------------------------*/
static void endif_statement(void)
{
  if(if_stack_ptr > 0)
  {
    accept(TOKENIZER_ENDIF);
    accept(TOKENIZER_EOL);
    if_stack_ptr--;
    return;
  }
  tokenizer_error_print(TOKENIZER_IF);
  ubasic_status.bit.isRunning = 0;
  ubasic_status.bit.Error = 1;
  return;
}

static void if_statement(void)
{
  uint8_t else_cntr, endif_cntr, f_nt, f_sl;

  accept(TOKENIZER_IF);

  VARIABLE_TYPE r = relation();

  if (accept(TOKENIZER_THEN))
  {
    tokenizer_error_print(TOKENIZER_IF);
    ubasic_status.bit.isRunning = 0;
    ubasic_status.bit.Error = 1;
    return;
  }

  if (tokenizer_token() == TOKENIZER_EOL)
  {
    // Multi-line IF-Statement
    // CR after then -> multiline IF-Statement
    if(if_stack_ptr < MAX_IF_STACK_DEPTH)
    {
      if_stack[if_stack_ptr] = r;
      if_stack_ptr++;
    }
    else
    {
      tokenizer_error_print(TOKENIZER_IF);
      ubasic_status.bit.isRunning = 0;
      ubasic_status.bit.Error = 1;
      return;
    }
    accept(TOKENIZER_EOL);
    if(r)
      return;
    else
    {
      else_cntr=endif_cntr=0; // number of else/endif possible in current nesting
      f_nt=f_sl=0; // f_nt flag for additional next token, f_fs flag single line

      while ( (
                (tokenizer_token() != TOKENIZER_ELSE && tokenizer_token() != TOKENIZER_ENDIF) ||
                  else_cntr || endif_cntr ) &&
                tokenizer_token() != TOKENIZER_ENDOFINPUT )
      {
          f_nt=0;

          // nested if
          if( tokenizer_token() == TOKENIZER_IF)
          {
            else_cntr+=1;
            endif_cntr+=1;
            f_sl=0;
          }

          if( tokenizer_token() == TOKENIZER_THEN)
          {
            f_nt=1;
            tokenizer_next();
            if (tokenizer_token() != TOKENIZER_EOL)
              f_sl=1;
          }

          if(tokenizer_token() == TOKENIZER_ELSE)
          {
            else_cntr--;
            if (else_cntr<0)
            {
              tokenizer_error_print(TOKENIZER_IF);
              ubasic_status.bit.isRunning = 0;
              ubasic_status.bit.Error = 1;
              return;
            }
          }

          if(!f_sl && (tokenizer_token() == TOKENIZER_ENDIF))
          {
            endif_cntr--;
            if (endif_cntr != else_cntr)
              else_cntr--;
          }
          else
          {
            if (f_sl && (tokenizer_token() == TOKENIZER_EOL))
            {
              f_sl=0;
              endif_cntr--;
              if (endif_cntr != else_cntr)
                else_cntr--;
            }
            else
            {
              if (tokenizer_token()==TOKENIZER_ENDIF)
              {
                tokenizer_error_print(TOKENIZER_IF);
                ubasic_status.bit.isRunning = 0;
                ubasic_status.bit.Error = 1;
                return;
              }
            }
          }
          if (!f_nt)
            tokenizer_next();
        }

        if(tokenizer_token() == TOKENIZER_ELSE)
          return;
      }
      endif_statement();
  }
  else
  {
    // Single-line IF-Statement
    if(r)
    {
      statement();
    }
    else
    {
      do
      {
        tokenizer_next();
      }
      while( tokenizer_token() != TOKENIZER_ELSE &&
             tokenizer_token() != TOKENIZER_EOL &&
             tokenizer_token() != TOKENIZER_ENDOFINPUT);
      if(tokenizer_token() == TOKENIZER_ELSE)
      {
        accept(TOKENIZER_ELSE);
        tokenizer_next();
        statement();
      }
      else
        accept_cr();
    }
  }
}

static void else_statement(void)
{
  VARIABLE_TYPE r=0;
  uint8_t endif_cntr, f_nt;

  accept(TOKENIZER_ELSE);

  if(if_stack_ptr > 0)
  {
    r = if_stack[if_stack_ptr-1];
  }
  else
  {
    tokenizer_error_print(TOKENIZER_ELSE);
    ubasic_status.bit.isRunning = 0;
    ubasic_status.bit.Error = 1;
    return;
  }

  if (tokenizer_token() == TOKENIZER_EOL)
  {
    accept(TOKENIZER_EOL);
    if(!r)
      return;
    else
    {
      endif_cntr=0;
      while( ( (tokenizer_token() != TOKENIZER_ENDIF ) || endif_cntr) &&
                tokenizer_token() != TOKENIZER_ENDOFINPUT )
      {
        f_nt=0;
        if( tokenizer_token() == TOKENIZER_IF)
          endif_cntr+=1;
        if( tokenizer_token() == TOKENIZER_THEN)
        {
          tokenizer_next();
          // then followed by CR -> multi line
          if (tokenizer_token() == TOKENIZER_EOL)
          {
            f_nt=1;
          }
          else
          { // single line
            endif_cntr--;
            while ( tokenizer_token() != TOKENIZER_ENDIF &&
                    tokenizer_token() != TOKENIZER_EOL &&
                    tokenizer_token() != TOKENIZER_ENDOFINPUT)
            {
              tokenizer_next();
            }
            if (tokenizer_token()==TOKENIZER_ENDIF)
            {
              tokenizer_error_print(TOKENIZER_ELSE);
              ubasic_status.bit.isRunning = 0;
              ubasic_status.bit.Error = 1;
              return;
            }
          }
        }
        if( tokenizer_token() == TOKENIZER_ENDIF)
          endif_cntr--;
        if (!f_nt)
          tokenizer_next();
      }
    }
    endif_statement();
    return;
  }
  tokenizer_error_print(TOKENIZER_ELSE);
  ubasic_status.bit.isRunning = 0;
  ubasic_status.bit.Error = 1;
  return;
}


/*---------------------------------------------------------------------------*/
static void let_statement(void)
{
  uint8_t varnum;
  if (tokenizer_token() == TOKENIZER_VARIABLE)
  {
    varnum = tokenizer_variable_num();
    accept(TOKENIZER_VARIABLE);
    if (!accept(TOKENIZER_EQ))
      ubasic_set_variable(varnum, relation());
  }
#if defined(VARIABLE_TYPE_STRING)
  // string additions here
  else if (tokenizer_token() == TOKENIZER_STRINGVARIABLE)
  {
    varnum = tokenizer_variable_num();
    accept(TOKENIZER_STRINGVARIABLE);
    if (!accept(TOKENIZER_EQ))
      ubasic_set_stringvariable(varnum, sexpr());
  }
  // end of string additions
#endif
#if defined(VARIABLE_TYPE_ARRAY)
  else if (tokenizer_token() == TOKENIZER_ARRAYVARIABLE)
  {
    varnum = tokenizer_variable_num();
    accept(TOKENIZER_ARRAYVARIABLE);

    accept(TOKENIZER_LEFTPAREN);
    VARIABLE_TYPE idx = relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
    idx = fixedpt_toint( idx );
  #endif
    accept(TOKENIZER_RIGHTPAREN);
    if (!accept(TOKENIZER_EQ))
      ubasic_set_arrayvariable(varnum, (uint16_t) idx, relation());
  }
#endif

  accept_cr();
}

#if defined(VARIABLE_TYPE_ARRAY)
static void dim_statement(void)
{
  VARIABLE_TYPE size=0;

  accept (TOKENIZER_DIM);

  // array addition here
  uint8_t  varnum = tokenizer_variable_num();

// 
  accept (TOKENIZER_ARRAYVARIABLE);

//   accept(TOKENIZER_LEFTPAREN);
  size = relation();

#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
  size = fixedpt_toint( size );
#endif

  ubasic_dim_arrayvariable(varnum, size);

//   accept(TOKENIZER_RIGHTPAREN);
  accept_cr();

// end of array additions
}
#endif

/*---------------------------------------------------------------------------*/
static void next_statement(void)
{
  accept(TOKENIZER_NEXT);

  uint8_t var = tokenizer_variable_num();

  accept(TOKENIZER_VARIABLE);

  if(for_stack_ptr > 0 && var == for_stack[for_stack_ptr - 1].for_variable)
  {
    VARIABLE_TYPE value = ubasic_get_variable(var) + for_stack[for_stack_ptr - 1].step;
    ubasic_set_variable(var, value);

    if ( ( (for_stack[for_stack_ptr - 1].step > 0) &&
            (value <= for_stack[for_stack_ptr - 1].to) ) ||
         ( (for_stack[for_stack_ptr - 1].step < 0) &&
            (value >= for_stack[for_stack_ptr - 1].to) ) )
    {
      //jump_line(for_stack[for_stack_ptr - 1].line_after_for);
      tokenizer_jump_offset(for_stack[for_stack_ptr - 1].line_after_for);
    }
    else
    {
      for_stack_ptr--;
      accept_cr();
    }
    return;
  }

  tokenizer_error_print(TOKENIZER_FOR);
  ubasic_status.bit.isRunning = 0;
  ubasic_status.bit.Error = 1;
}

/*---------------------------------------------------------------------------*/

static void for_statement(void)
{
  uint8_t for_variable;
  VARIABLE_TYPE to;

  accept(TOKENIZER_FOR);
  for_variable = tokenizer_variable_num();
  accept(TOKENIZER_VARIABLE);
  accept(TOKENIZER_EQ);
  ubasic_set_variable(for_variable, relation());
  accept(TOKENIZER_TO);
  to = relation();

#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
  VARIABLE_TYPE step = FIXEDPT_ONE;
#else
  VARIABLE_TYPE step = 1;
#endif
  if (tokenizer_token() == TOKENIZER_STEP)
  {
    accept(TOKENIZER_STEP);
    step = relation();
  }
  accept_cr();

  if(for_stack_ptr < MAX_FOR_STACK_DEPTH)
  {
    //for_stack[for_stack_ptr].line_after_for = tokenizer_line_number();
    for_stack[for_stack_ptr].line_after_for = tokenizer_save_offset();
    for_stack[for_stack_ptr].for_variable = for_variable;
    for_stack[for_stack_ptr].to = to;
    for_stack[for_stack_ptr].step = step;
    for_stack_ptr++;
    return;
  }

  tokenizer_error_print(TOKENIZER_FOR);
  ubasic_status.bit.isRunning = 0;
  ubasic_status.bit.Error = 1;
}

/*---------------------------------------------------------------------------*/

static void end_statement(void)
{
  accept(TOKENIZER_END);
  ubasic_status.bit.isRunning = 0;
  ubasic_status.bit.Error = 0;
}

#if defined(UBASIC_SCRIPT_HAVE_SLEEP)
static void sleep_statement(void)
{
  accept(TOKENIZER_SLEEP);
  VARIABLE_TYPE f = relation();
  if (f > 0)
  {
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
    ubasic_script_sleeping_ms = fixedpt_toint(f * 1000);
#else
    ubasic_script_sleeping_ms = (uint32_t) f;
#endif
  }
  else
    ubasic_script_sleeping_ms = 0;

  accept_cr();
}
#endif

#if defined(UBASIC_SCRIPT_HAVE_TICTOC)
static void tic_statement(void)
{
  accept(TOKENIZER_TIC);

  accept(TOKENIZER_LEFTPAREN);

  VARIABLE_TYPE f = relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
    f = fixedpt_toint(f);
  #endif

  if (f == 2)
    ubasic_script_tic1_ms = 0;
  else if (f == 3)
    ubasic_script_tic2_ms = 0;
  else if (f == 4)
    ubasic_script_tic3_ms = 0;
  else if (f == 5)
    ubasic_script_tic4_ms = 0;
  else if (f == 6)
    ubasic_script_tic5_ms = 0;
  else
    ubasic_script_tic0_ms = 0;

  accept(TOKENIZER_RIGHTPAREN);
  accept_cr();
}
#endif

#if defined(UBASIC_SCRIPT_HAVE_INPUT_FROM_SERIAL)
static void input_statement_wait (void)
{
  input_how = 0;
  accept(TOKENIZER_INPUT);
  
  if (tokenizer_token() == TOKENIZER_PRINT_HEX)
  {
    tokenizer_next();
    input_how = 1;
  }
  else if (tokenizer_token() == TOKENIZER_PRINT_DEC)
  {
    tokenizer_next();
    input_how = 2;
  }

  if (tokenizer_token() == TOKENIZER_VARIABLE)
  {
    input_varnum = tokenizer_variable_num();
    accept(TOKENIZER_VARIABLE);
    input_type = 0;
  }
  #if defined(VARIABLE_TYPE_STRING)
  // string additions here
  else if (tokenizer_token() == TOKENIZER_STRINGVARIABLE)
  {
    input_varnum = tokenizer_variable_num();
    accept(TOKENIZER_STRINGVARIABLE);
    input_type = 1;
  }
  // end of string additions
  #endif
  #if defined(VARIABLE_TYPE_ARRAY)
  else if (tokenizer_token() == TOKENIZER_ARRAYVARIABLE)
  {
    input_varnum = tokenizer_variable_num();
    accept(TOKENIZER_ARRAYVARIABLE);

    accept(TOKENIZER_LEFTPAREN);
    input_array_index = relation();
    #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
    input_array_index = fixedpt_toint( input_array_index );
    #endif
    accept(TOKENIZER_RIGHTPAREN);
    input_type = 2;
  }
  #endif

  // get next token:
  //    CR
  // or
  //    , timeout
  if(tokenizer_token() == TOKENIZER_COMMA)
  {
    accept(TOKENIZER_COMMA);
    VARIABLE_TYPE r = relation();
  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
    r = fixedpt_toint( r );
  #endif
    if (r>0)
    {
      ubasic_script_wait_for_input_ms = r;
    }
  }

  accept_cr();

  ubasic_status.bit.WaitForSerialInput = 1;
}

static void serial_input_completed(void)
{
  // transfer serial input buffer to 'buf' only if something
  // has been received.
  // otherwise leave the variable content unchanged.
  if (serial_input(string,MAX_STRINGLEN)>0)
  {
    if ( (input_type == 0)
  #if defined(VARIABLE_TYPE_ARRAY)
        || (input_type == 2)
  #endif
      )
    {
      VARIABLE_TYPE r;
      if ((input_how == 1)||(input_how == 2))
      {
        r = atoi(string);
      }
      else
      {
      // process number
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
        r = str_fixedpt(string,MAX_STRINGLEN,FIXEDPT_FBITS>>1);
#else
        r = atoi(string);
#endif
      }

      if (input_type == 0)
      {
        ubasic_set_variable(input_varnum, r);
      }
  #if defined(VARIABLE_TYPE_ARRAY)
      else if (input_type == 2)
      {
        ubasic_set_arrayvariable(input_varnum, input_array_index, r);
      }
  #endif
    }
  #if defined(VARIABLE_TYPE_STRING)
    else if (input_type == 1)
    {
      ubasic_set_stringvariable(input_varnum, string);
    }
  #endif 
  }

  ubasic_status.bit.WaitForSerialInput = 0;
}

#endif /* #if defined(UBASIC_SCRIPT_HAVE_INPUT_FROM_SERIAL) */


/*---------------------------------------------------------------------------*/
static void while_statement(void)
{
  VARIABLE_TYPE r;
  int8_t  while_cntr;
  uint16_t while_offset;

  // this is where we jump to after 'endwhile'
  while_offset = tokenizer_save_offset();

  accept(TOKENIZER_WHILE);

  if(while_stack_ptr == MAX_WHILE_STACK_DEPTH)
  {
    tokenizer_error_print(TOKENIZER_WHILE);
    ubasic_status.bit.isRunning = 0;
    ubasic_status.bit.Error = 1;
  }

  // this makes sure that the jump to the same while is ignored
  if ( (while_stack_ptr == 0)  ||
        ( (while_stack_ptr > 0) &&
        (while_stack[while_stack_ptr-1].line_while != while_offset) ) )
  {
    while_stack[while_stack_ptr].line_while = while_offset;
    while_stack[while_stack_ptr].line_after_endwhile = -1; // we don't know it yet
    while_stack_ptr++;
  }

  r = relation();

  if(while_stack_ptr==0)
  {
    tokenizer_error_print(TOKENIZER_WHILE);
    ubasic_status.bit.isRunning = 0;
    ubasic_status.bit.Error = 1;
    return;
  }

  if (r)
  {
    accept_cr();
    return;
  }

  if (while_stack[while_stack_ptr-1].line_after_endwhile > 0)
  {
    // we have traversed while loop once to its end already.
    // thus we know where the loop ends. we just use that to jump there.
    tokenizer_jump_offset(while_stack[while_stack_ptr-1].line_after_endwhile);
  }
  else
  {
    // first time the loop is entered the condition is not satisfied,
    // so we gobble the lines until we reach the matching endwhile
    while_cntr=0;
    while ( (tokenizer_token() != TOKENIZER_ENDWHILE  || while_cntr ) &&
             (tokenizer_token() != TOKENIZER_ENDOFINPUT)  )
    {
      if (tokenizer_token() == TOKENIZER_WHILE)
        while_cntr+=1;
      if (tokenizer_token() == TOKENIZER_ENDWHILE)
        while_cntr-=1;
      tokenizer_next();
    }
    while_stack_ptr--;
    accept(TOKENIZER_ENDWHILE);
    accept(TOKENIZER_EOL);
  }

  return;
}
/*---------------------------------------------------------------------------*/
static void endwhile_statement(void)
{
  accept(TOKENIZER_ENDWHILE);
  if(while_stack_ptr > 0)
  {
    // jump_line(while_stack[while_stack_ptr-1]);
    if (while_stack[while_stack_ptr-1].line_after_endwhile==-1)
    {
      while_stack[while_stack_ptr-1].line_after_endwhile = tokenizer_save_offset();
    }
    tokenizer_jump_offset(while_stack[while_stack_ptr-1].line_while);
    return;
  }

  tokenizer_error_print(TOKENIZER_FOR);
  ubasic_status.bit.isRunning = 0;
  ubasic_status.bit.Error = 1;
}
/*---------------------------------------------------------------------------*/

#if defined(UBASIC_SCRIPT_HAVE_STORE_VARS_IN_FLASH)

static VARIABLE_TYPE recall_statement(void)
{
  VARIABLE_TYPE rval=0;

  static uint8_t varnum;

  accept(TOKENIZER_RECALL);
  accept(TOKENIZER_LEFTPAREN);

  if (tokenizer_token() == TOKENIZER_VARIABLE)
  {
    varnum = tokenizer_variable_num();
    accept(TOKENIZER_VARIABLE);
    EE_ReadVariable( varnum, 0, (uint8_t *) &variables[varnum], (uint8_t *) &rval );
    rval >>= 2;
  }
#if defined(VARIABLE_TYPE_STRING)
  else if (tokenizer_token() == TOKENIZER_STRINGVARIABLE)
  {
    varnum = tokenizer_variable_num();
    accept(TOKENIZER_STRINGVARIABLE);
    char dummy_s[MAX_STRINGLEN] = {0};
    EE_ReadVariable( varnum, 1, (uint8_t *) dummy_s, (uint8_t *) &rval );
    if (rval > 0)
    {
      stringvariables[varnum] = scpy((char *)dummy_s);
    }
    else
      stringvariables[varnum] = scpy((char *)nullstring);
    garbage_collect();
  }
#endif
#if defined(VARIABLE_TYPE_ARRAY)
  else if (tokenizer_token() == TOKENIZER_ARRAYVARIABLE)
  {
    varnum = tokenizer_variable_num();
    accept(TOKENIZER_ARRAYVARIABLE);
    VARIABLE_TYPE dummy_a[VARIABLE_TYPE_ARRAY+1];
    EE_ReadVariable(varnum, 2, (uint8_t *) dummy_a, (uint8_t *) &rval );
    if (rval > 0)
    {
      rval >>= 2;
      ubasic_dim_arrayvariable(varnum, rval);
      for (uint8_t i=0; i<rval; i++)
        ubasic_set_arrayvariable(varnum, i+1,  dummy_a[i]);
    }
  }
#endif

  accept(TOKENIZER_RIGHTPAREN);
  return rval;
}

static void store_statement(void)
{
  static uint8_t varnum;

  accept(TOKENIZER_STORE);
  accept(TOKENIZER_LEFTPAREN);

  if (tokenizer_token() == TOKENIZER_VARIABLE)
  {
    varnum = tokenizer_variable_num();
    accept(TOKENIZER_VARIABLE);
    EE_WriteVariable( varnum, 0, 4, (uint8_t *) &variables[varnum] );
  }
  #if defined(VARIABLE_TYPE_STRING)
  // string additions here
  else if (tokenizer_token() == TOKENIZER_STRINGVARIABLE)
  {
    varnum = tokenizer_variable_num();
    accept(TOKENIZER_STRINGVARIABLE);
    EE_WriteVariable( varnum, 1, strlen(stringvariables[varnum]),
                      (uint8_t *) stringvariables[varnum] );
  }
  // end of string additions
  #endif
  #if defined(VARIABLE_TYPE_ARRAY)
  else if (tokenizer_token() == TOKENIZER_ARRAYVARIABLE)
  {
    varnum = tokenizer_variable_num();
    accept(TOKENIZER_ARRAYVARIABLE);
    EE_WriteVariable(varnum, 2, 4 * (arrays_data[arrayvariable[varnum]] & 0x0000ffff),
                     (uint8_t *) &arrays_data[arrayvariable[varnum] + 1] );
  }
  #endif

  accept(TOKENIZER_RIGHTPAREN);
  accept_cr();
}
/*---------------------------------------------------------------------------*/
#endif


/*---------------------------------------------------------------------------*/
static void statement(void)
{
  VARIABLE_TYPE token = tokenizer_token();
  uint8_t println=0;

  if (ubasic_status.bit.Error)
    return;

  switch(token)
  {
    case TOKENIZER_EOL:
      accept(TOKENIZER_EOL);
      break;

    case TOKENIZER_PRINTLN:
      println=1;
    case TOKENIZER_PRINT:
      print_statement(println);
      break;

    case TOKENIZER_IF:
      if_statement();
      break;

    case TOKENIZER_ELSE:
      else_statement();
      break;

    case TOKENIZER_ENDIF:
      endif_statement();
      break;

    case TOKENIZER_GOTO:
      goto_statement();
      break;

    case TOKENIZER_GOSUB:
      gosub_statement();
      break;

    case TOKENIZER_RETURN:
      return_statement();
      break;

    case TOKENIZER_FOR:
      for_statement();
      break;

    case TOKENIZER_NEXT:
      next_statement();
      break;

    case TOKENIZER_WHILE:
      while_statement();
      break;

    case TOKENIZER_ENDWHILE:
      endwhile_statement();
      break;


    case TOKENIZER_END:
      end_statement();
      break;

    case TOKENIZER_LET:
      accept(TOKENIZER_LET); /* Fall through: Nothing to do! */
    case TOKENIZER_VARIABLE:
#if defined(VARIABLE_TYPE_STRING)
    // string addition
    case TOKENIZER_STRINGVARIABLE:
    // end of string addition
#endif
#if defined(VARIABLE_TYPE_ARRAY)
    case TOKENIZER_ARRAYVARIABLE:
#endif
      let_statement();
      break;

#if defined(UBASIC_SCRIPT_HAVE_INPUT_FROM_SERIAL)
    case TOKENIZER_INPUT:
      input_statement_wait();
      break;
#endif

#if defined(UBASIC_SCRIPT_HAVE_SLEEP)
    case TOKENIZER_SLEEP:
      sleep_statement();
      break;
#endif

#if defined(VARIABLE_TYPE_ARRAY)
    case TOKENIZER_DIM:
      dim_statement();
      break;
#endif

#if defined(UBASIC_SCRIPT_HAVE_TICTOC)
    case TOKENIZER_TIC:
      tic_statement();
      break;
#endif

#ifdef UBASIC_SCRIPT_HAVE_PWM_CHANNELS
    case TOKENIZER_PWM:
      pwm_statement();
      break;

    case TOKENIZER_PWMCONF:
      pwmconf_statement();
      break;
#endif

#if defined(UBASIC_SCRIPT_HAVE_ANALOG_READ)
    case TOKENIZER_AREADCONF:
      areadconf_statement();
      break;
#endif

#if defined(UBASIC_SCRIPT_HAVE_GPIO_CHANNELS)
    case TOKENIZER_PINMODE:
      pinmode_statement();
      break;

    case TOKENIZER_DWRITE:
      dwrite_statemet();
      break;
#endif /* UBASIC_SCRIPT_HAVE_GPIO_CHANNELS */

#if defined(UBASIC_SCRIPT_HAVE_STORE_VARS_IN_FLASH)
    case TOKENIZER_STORE:
      store_statement();
      break;

    case TOKENIZER_RECALL:
      recall_statement();
      break;
#endif /* UBASIC_SCRIPT_HAVE_STORE_VARS_IN_FLASH */

    case TOKENIZER_CLEAR:
      ubasic_clear_variables();
      accept_cr();
      break;


    default:
      tokenizer_error_print(token);
      ubasic_status.bit.isRunning = 0;
      ubasic_status.bit.Error = 1;
  }
}
/*---------------------------------------------------------------------------*/
static void numbered_line_statement(void)
{
  while (1)
  {
    if (tokenizer_token() == TOKENIZER_COLON)
    {
      accept(TOKENIZER_COLON);
      if (accept(TOKENIZER_LABEL))
        return;
      continue;
    }
    break;
  }

  statement();

  return;
}

/*---------------------------------------------------------------------------*/
void ubasic_run_program(void)
{
  if (ubasic_status.bit.isRunning==0)
    return;
  if (ubasic_status.bit.Error==1)
    return;

#if defined(UBASIC_SCRIPT_HAVE_SLEEP)
  if (ubasic_script_sleeping_ms)
    return;
#endif

#if defined(UBASIC_SCRIPT_HAVE_INPUT_FROM_SERIAL)
  if (ubasic_status.bit.WaitForSerialInput)
  {
    if (serial_input_available()==0)
    {
      if (ubasic_script_wait_for_input_ms > 0)
        return;
    }
    serial_input_completed();
  }
#endif

#if defined(VARIABLE_TYPE_STRING)
  // string additions
  garbage_collect();
  // end of string additions
#endif

  if(tokenizer_finished())
  {
    return;
  }

  numbered_line_statement();
}

/*---------------------------------------------------------------------------*/
uint8_t ubasic_execute_statement(char * stmt)
{
  ubasic_status.byte= 0;

  program_ptr = stmt;
  for_stack_ptr = gosub_stack_ptr = 0;
  tokenizer_init(stmt);

  do
  {
#if defined(VARIABLE_TYPE_STRING)
    garbage_collect();
#endif

    statement();

    if (ubasic_status.bit.Error)
      break;

#if defined(UBASIC_SCRIPT_HAVE_INPUT_FROM_SERIAL)
    while (ubasic_status.bit.WaitForSerialInput)
    {
      if (serial_input_available()==0)
      {
        if (ubasic_script_wait_for_input_ms > 0)
          continue;
      }
      serial_input_completed();
    }
#endif

#if defined(UBASIC_SCRIPT_HAVE_SLEEP)
    while (ubasic_script_sleeping_ms);
#endif

  }
  while (!tokenizer_finished());

  return ubasic_status.byte;
}

/*---------------------------------------------------------------------------*/
uint8_t ubasic_waiting_for_input(void)
{
  return (ubasic_status.bit.WaitForSerialInput);
}

uint8_t ubasic_finished(void)
{
  return (tokenizer_finished() || ubasic_status.bit.isRunning == 0 );
}

/*---------------------------------------------------------------------------*/

void ubasic_set_variable(uint8_t varnum, VARIABLE_TYPE value)
{
  if(varnum > 0 && varnum <= MAX_VARNUM)
  {
    variables[varnum] = value;
  }
}

/*---------------------------------------------------------------------------*/

VARIABLE_TYPE ubasic_get_variable(uint8_t varnum)
{
  if(varnum > 0 && varnum <= MAX_VARNUM)
  {
    return variables[varnum];
  }
  return 0;
}

#if defined(VARIABLE_TYPE_STRING)
//
// string additions
// 
/*---------------------------------------------------------------------------*/
void ubasic_set_stringvariable(uint8_t svarnum, char *svalue)
{
  if(svarnum <MAX_SVARNUM)
  {
    stringvariables[svarnum] = svalue;
  }
}

/*---------------------------------------------------------------------------*/

char * ubasic_get_stringvariable(uint8_t varnum)
{
  if(varnum < MAX_SVARNUM)
  {
    return stringvariables[varnum];
  }
  return scpy((char*)nullstring);
}
//
// end of string additions
//
#endif

#if defined(VARIABLE_TYPE_ARRAY)
//
// array additions: works only for VARIABLE_TYPE 32bit
//  array storage:
//    1st entry:   [ 31:16 , 15:0]
//                  varnum   size
//    entries 2 through size+1 are the array elements
//  could work for 16bit values as well
/*---------------------------------------------------------------------------*/
void ubasic_dim_arrayvariable(uint8_t varnum, int16_t newsize)
{
  if(varnum >= MAX_VARNUM)
    return;

  int16_t oldsize;
  int16_t  current_location;

_attach_at_the_end:

  current_location = arrayvariable[varnum];
  if (current_location == -1)
  {
    /* does the array fit in the available memory? */
    if (free_arrayptr+newsize+1 < VARIABLE_TYPE_ARRAY)
    {
      current_location = free_arrayptr;
      arrayvariable[varnum] = current_location;
      arrays_data[current_location] = (varnum<<16) | newsize;
      free_arrayptr += newsize + 1;
      return;
    }
    return; /* failed to allocate*/
  }
  else
  {
    oldsize = arrays_data[current_location];
  }

  /* if size of the array is the same as earlier allocated then do nothing */
  if (oldsize == newsize)
  {
    return;
  }

  /* if this is the last array in arrays_data, just modify the boundary */
  if (current_location + oldsize + 1 == free_arrayptr)
  {
    if (free_arrayptr - current_location + newsize < VARIABLE_TYPE_ARRAY)
    {
      arrays_data[current_location] = (varnum<<16) | newsize;
      free_arrayptr += newsize - oldsize;
      arrays_data[free_arrayptr] = 0;
      return;
    }

    /* failed to allocate memory */
    arrayvariable[varnum] = -1;
    return;
  }

  /* Array has been allocated before. It is not the last array */
  /* Thus we have to go over all arrays above the current location, and shift them down */
  arrayvariable[varnum] = -1;
  int16_t  next_location;
  uint16_t mov_size, mov_varnum;
  next_location = current_location + oldsize + 1;
  do
  {
    mov_varnum = (arrays_data[next_location]>>16);
    mov_size   =  arrays_data[next_location];

    for (uint8_t i=0; i<=mov_size; i++)
    {
      arrays_data[current_location + i] = arrays_data[next_location + i];
      arrays_data[next_location + i] = 0;
    }
    arrayvariable[mov_varnum] = current_location;
    next_location = next_location + mov_size + 1;
    current_location = current_location + mov_size + 1;
    arrays_data[current_location] = 0;
  }
  while (arrays_data[next_location]>0);
  free_arrayptr = current_location;

  /** now the array should be added to the end of the list:
      if there is space do it! */
  goto _attach_at_the_end;
}

void ubasic_set_arrayvariable(uint8_t varnum, uint16_t idx,  VARIABLE_TYPE value)
{
  uint16_t size = (uint16_t) arrays_data[arrayvariable[varnum]];
  if ((size < idx)||(idx<1))
    return;

  arrays_data[arrayvariable[varnum] + idx] = value;
}

VARIABLE_TYPE ubasic_get_arrayvariable(uint8_t varnum, uint16_t idx)
{
  uint16_t size = (uint16_t) arrays_data[arrayvariable[varnum]];
  if ((idx>=0)&&(idx<=size))
    return (VARIABLE_TYPE) arrays_data[arrayvariable[varnum] + idx];
  return -1;
}
#endif
/*---------------------------------------------------------------------------*/
