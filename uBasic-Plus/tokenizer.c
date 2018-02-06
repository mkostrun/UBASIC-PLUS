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
 */

#include "config.h"
#include "tokenizer.h"

static char const *ptr, *nextptr;

static char const *prog;

#define MAX_NUMLEN 8

struct keyword_token
{
  char *keyword;
  uint8_t token;
};

static uint8_t current_token = TOKENIZER_ERROR;

static const struct keyword_token keywords[] =
{
#if defined(VARIABLE_TYPE_STRING)
// new string-related statements and functions
  {"left$",                   TOKENIZER_LEFT$},
  {"right$",                  TOKENIZER_RIGHT$},
  {"mid$",                    TOKENIZER_MID$},
  {"str$",                    TOKENIZER_STR$},
  {"chr$",                    TOKENIZER_CHR$},
  {"val",                     TOKENIZER_VAL},
  {"len",                     TOKENIZER_LEN},
  {"instr",                   TOKENIZER_INSTR},
  {"asc",                     TOKENIZER_ASC},
#endif
// end of string additions
  {"let", TOKENIZER_LET},
  {"println", TOKENIZER_PRINTLN},
  {"print", TOKENIZER_PRINT},
  {"if", TOKENIZER_IF},
  {"then", TOKENIZER_THEN},
  {"else", TOKENIZER_ELSE},
  {"for", TOKENIZER_FOR},
#if defined(UBASIC_SCRIPT_HAVE_TICTOC)
  {"toc", TOKENIZER_TOC},
#endif
#if defined(UBASIC_SCRIPT_HAVE_INPUT_FROM_SERIAL)
  {"input", TOKENIZER_INPUT},
#endif
  {"to", TOKENIZER_TO},
  {"next", TOKENIZER_NEXT},
  {"goto", TOKENIZER_GOTO},
  {"gosub", TOKENIZER_GOSUB},
  {"return", TOKENIZER_RETURN},
  {"call", TOKENIZER_CALL},
  {"end", TOKENIZER_END},
// CHDK inspired additions: Start
#if defined(UBASIC_SCRIPT_HAVE_SLEEP)
  {"sleep", TOKENIZER_SLEEP},
#endif
#if defined(VARIABLE_TYPE_ARRAY)
  {"dim", TOKENIZER_DIM},
#endif
#if defined(UBASIC_SCRIPT_HAVE_TICTOC)
  {"tic", TOKENIZER_TIC},
#endif
#if defined(UBASIC_SCRIPT_HAVE_HARDWARE_EVENTS)
  {"hw_event", TOKENIZER_HWE},
#endif
#if defined(UBASIC_SCRIPT_HAVE_RANDOM_NUMBER_GENERATOR)
  {"ran", TOKENIZER_RAN},
#endif
#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
  {"sqrt", TOKENIZER_SQRT},
  {"sin",  TOKENIZER_SIN},
  {"cos", TOKENIZER_COS},
  {"tan", TOKENIZER_TAN},
  {"exp", TOKENIZER_EXP},
  {"ln", TOKENIZER_LN},
  #if defined(UBASIC_SCRIPT_HAVE_RANDOM_NUMBER_GENERATOR)
  {"uniform", TOKENIZER_UNIFORM},
  #endif
  {"floor", TOKENIZER_FLOOR},
  {"ceil", TOKENIZER_CEIL},
  {"round", TOKENIZER_ROUND},
  {"pow", TOKENIZER_POWER},
#endif
#if defined(UBASIC_SCRIPT_HAVE_GPIO_CHANNELS)
  {"gpio", TOKENIZER_GPIO},
#endif
#ifdef UBASIC_SCRIPT_HAVE_PWM_CHANNELS
  {"pwm", TOKENIZER_PWM},
#endif
// CHDK inspired additions: End
  {NULL, TOKENIZER_ERROR}
};

/*---------------------------------------------------------------------------*/
static uint8_t singlechar_or_operator(uint8_t *offset)
{
  if (offset)
    *offset=1;

  if ((*ptr == '\n') || (*ptr == ';'))
  {
    return TOKENIZER_EOL;
  }
  else if(*ptr == ',')
  {
    return TOKENIZER_COMMA;
  }
  else if(*ptr == '+')
  {
    return TOKENIZER_PLUS;
  }
  else if(*ptr == '-')
  {
    return TOKENIZER_MINUS;
  }
  else if(*ptr == '&')
  {
    return TOKENIZER_AND;
  }
  else if(*ptr == '|')
  {
    return TOKENIZER_OR;
  }
  else if(*ptr == '*')
  {
    return TOKENIZER_ASTR;
  }
  else if(*ptr == '/')
  {
    return TOKENIZER_SLASH;
  }
  else if(*ptr == '%')
  {
    return TOKENIZER_MOD;
  }
  else if(*ptr == '(')
  {
    return TOKENIZER_LEFTPAREN;
  }
  else if(*ptr == ')')
  {
    return TOKENIZER_RIGHTPAREN;
  }
  else if(*ptr == '<')
  {
    if (ptr[1] == '=')
    {
      if (offset)
        *offset += 1;
      return TOKENIZER_LE;
    }
    else if (ptr[1] == '>')
    {
      if (offset)
        *offset += 1;
      return TOKENIZER_NE;
    }
    return TOKENIZER_LT;
  }
  else if(*ptr == '>')
  {
    if (ptr[1] == '=')
    {
      if (offset)
        *offset += 1;
      return TOKENIZER_GE;
    }
    return TOKENIZER_GT;
  }
  else if(*ptr == '=')
  {
    if (ptr[1] == '=')
      if (offset)
        *offset += 1;
    return TOKENIZER_EQ;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static uint8_t get_next_token(void)
{
  struct keyword_token const *kt;
  uint8_t i;

  // eat all whitespace
  while(*ptr == ' ' || *ptr == '\t' || *ptr == '\r')
    ptr++;

  if(*ptr == 0)
  {
    return TOKENIZER_ENDOFINPUT;
  }

#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
  uint8_t have_decdot=0, i_dot=0;
  if( isdigit(*ptr) || (*ptr=='.') )
  {
    for(i=0; i<MAX_NUMLEN; i++)
    {
      if (ptr[i]=='.')
      {
        i_dot = i;
        have_decdot++;
        if (have_decdot>1)
          return TOKENIZER_ERROR;
        continue;
      }
      if(!isdigit(ptr[i]))
      {
        if(i > 0)
        {
          nextptr = ptr + i;
          if (have_decdot && i_dot < i-1)
            return TOKENIZER_FLOAT;
          else
            return TOKENIZER_NUMBER;
        }
        else
        {
          return TOKENIZER_ERROR;
        }
      }
      if(!isdigit(ptr[i]))
      {
        return TOKENIZER_ERROR;
      }
    }
    return TOKENIZER_ERROR;
  }
#else
  if (isdigit(*ptr))
  {
    for(i=0; i<MAX_NUMLEN; i++)
    {
      if(!isdigit(ptr[i]))
      {
        if(i > 0)
        {
          nextptr = ptr + i;
          return TOKENIZER_NUMBER;
        }
        else
        {
          return TOKENIZER_ERROR;
        }
      }
      if(!isdigit(ptr[i]))
      {
        return TOKENIZER_ERROR;
      }
    }
    return TOKENIZER_ERROR;
  }
#endif
  else if(singlechar_or_operator(&i))
  {
    nextptr = ptr + i;
    return singlechar_or_operator(NULL);
  }
#if defined(VARIABLE_TYPE_STRING)
  else if(*ptr == '"' || *ptr == '\'')
  {
    nextptr = ptr;
    do
    {
      ++nextptr;
      if ((*nextptr=='\0')||(*nextptr=='\n')||(*nextptr==';'))
        return TOKENIZER_ERROR;
    }
    while(*nextptr != '"' && *nextptr != '\'');
    ++nextptr;
    return TOKENIZER_STRING;
  }
#endif
  else
  {
    /* Check for keywords: */
   for(kt = keywords; kt->keyword != NULL; ++kt)
   {
     if(strncmp(ptr, kt->keyword, strlen(kt->keyword)) == 0) {
       nextptr = ptr + strlen(kt->keyword);
       return kt->token;
     }
   }
  }
  if(*ptr >= 'a' && *ptr <= 'z')
  {
#if defined(VARIABLE_TYPE_STRING)
    // string addition
    if (*(ptr+1) == '$')
    {
      nextptr = ptr + 2;
      return TOKENIZER_STRINGVARIABLE;
    }
    // end of string addition
#endif

#if defined(VARIABLE_TYPE_ARRAY)
    if (*(ptr+1) == '@')
    {
      nextptr = ptr + 2;
      return TOKENIZER_ARRAYVARIABLE;
    }
#endif

    nextptr = ptr + 1;
    return TOKENIZER_VARIABLE;
  }

  return TOKENIZER_ERROR;
}
/*---------------------------------------------------------------------------*/
#if defined(VARIABLE_TYPE_STRING)
int8_t tokenizer_stringlookahead( void )
{
  // return 1 (true) if next 'defining' token is string not integer
  char * saveptr = (char *)ptr;
  char * savenextptr = (char *)nextptr;
  uint8_t token = current_token;
  int8_t si = -1;

  while (si == -1)
  {
    if (token == TOKENIZER_EOL || token == TOKENIZER_ENDOFINPUT)
      si = 0;
    else if (token == TOKENIZER_NUMBER || token == TOKENIZER_VARIABLE || token == TOKENIZER_FLOAT)
      si = 0; // number or numeric var
    else if (token == TOKENIZER_PLUS)
      si = si;
    else if (token == TOKENIZER_STRING)
      si = 1;
    else if (token >= TOKENIZER_STRINGVARIABLE && token <= TOKENIZER_CHR$)
      si = 1;
    else if (token > TOKENIZER_CHR$)
      si = 0; // numeric function

    token = get_next_token();
  }
  ptr = saveptr;
  nextptr = savenextptr;
  return si; 
}
#endif
/*---------------------------------------------------------------------------*/
void tokenizer_init(const char *program)
{
  ptr = program;
  prog = program;
  current_token = get_next_token();
}
/*---------------------------------------------------------------------------*/
uint8_t tokenizer_token(void)
{
  return current_token;
}
/*---------------------------------------------------------------------------*/
void tokenizer_next(void)
{

  if(tokenizer_finished())
  {
    return;
  }

  ptr = nextptr;

  while(*ptr == ' ')
  {
    ++ptr;
  }

  current_token = get_next_token();
  return;
}
/*---------------------------------------------------------------------------*/
VARIABLE_TYPE tokenizer_num(void)
{
  VARIABLE_TYPE rval;
  rval = atoi(ptr);
  return rval;
}

#if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
/*---------------------------------------------------------------------------*/
VARIABLE_TYPE tokenizer_float(void)
{
  return str_fixedpt((char*)ptr, nextptr-ptr, FIXEDPT_FBITS>>1);
}
#endif

#if defined(VARIABLE_TYPE_STRING)
/*---------------------------------------------------------------------------*/
void tokenizer_string(char *dest, uint8_t len)
{
  char *string_end, quote_char;
  uint8_t string_len;

  if(tokenizer_token() != TOKENIZER_STRING)
  {
    return;
  }
  quote_char = *ptr;

  string_end = strchr(ptr + 1, quote_char);
  if(string_end == NULL)
  {
    return;
  }
  string_len = string_end - ptr - 1;
  if(len < string_len)
  {
    string_len = len;
  }
  memcpy(dest, ptr + 1, string_len);
  dest[string_len] = 0;
}
#endif
/*---------------------------------------------------------------------------*/
void tokenizer_error_print(void)
{
  // printf("%s", string);
  print_serial("Err:");
  print_serial((char*)ptr);
}
/*---------------------------------------------------------------------------*/
uint8_t tokenizer_finished(void)
{
  return *ptr == 0 || current_token == TOKENIZER_ENDOFINPUT;
}
/*---------------------------------------------------------------------------*/
uint8_t tokenizer_variable_num(void)
{
  return *ptr - 'a';
}
/*---------------------------------------------------------------------------*/
