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
extern volatile _Status ubasic_status;
// uint16_t    current_line=0;

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
  {"let ", TOKENIZER_LET},
  {"println ", TOKENIZER_PRINTLN},
  {"print ", TOKENIZER_PRINT},
  {"if", TOKENIZER_IF},
  {"then", TOKENIZER_THEN},
  {"else", TOKENIZER_ELSE},
  {"endif", TOKENIZER_ENDIF},
#if defined(UBASIC_SCRIPT_HAVE_TICTOC)
  {"toc", TOKENIZER_TOC},
#endif
#if defined(UBASIC_SCRIPT_HAVE_INPUT_FROM_SERIAL)
  {"input", TOKENIZER_INPUT},
#endif
  {"for ", TOKENIZER_FOR},
  {"to ", TOKENIZER_TO},
  {"next ", TOKENIZER_NEXT},
  {"step ", TOKENIZER_STEP},
  {"while", TOKENIZER_WHILE},
  {"endwhile", TOKENIZER_ENDWHILE},
  {"goto ", TOKENIZER_GOTO},
  {"gosub ", TOKENIZER_GOSUB},
  {"return", TOKENIZER_RETURN},
  {"end", TOKENIZER_END},
#if defined(UBASIC_SCRIPT_HAVE_SLEEP)
  {"sleep", TOKENIZER_SLEEP},
#endif
#if defined(VARIABLE_TYPE_ARRAY)
  {"dim ", TOKENIZER_DIM},
#endif
#if defined(UBASIC_SCRIPT_HAVE_TICTOC)
  {"tic", TOKENIZER_TIC},
#endif
#if defined(UBASIC_SCRIPT_HAVE_HARDWARE_EVENTS)
  {"flag", TOKENIZER_HWE},
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
  {"abs", TOKENIZER_ABS},
  {"floor", TOKENIZER_FLOOR},
  {"ceil", TOKENIZER_CEIL},
  {"round", TOKENIZER_ROUND},
  {"pow", TOKENIZER_POWER},
#endif
#if defined(UBASIC_SCRIPT_HAVE_GPIO_CHANNELS)
  {"pinmode", TOKENIZER_PINMODE},
  {"dread", TOKENIZER_DREAD},
  {"dwrite", TOKENIZER_DWRITE},
#endif
#ifdef UBASIC_SCRIPT_HAVE_PWM_CHANNELS
  {"awrite_conf", TOKENIZER_PWMCONF},
  {"awrite", TOKENIZER_PWM},
#endif
#if defined(UBASIC_SCRIPT_HAVE_ANALOG_READ)
  {"aread_conf", TOKENIZER_AREADCONF},
  {"aread", TOKENIZER_AREAD},
#endif
  {"hex ", TOKENIZER_PRINT_HEX},
  {"dec ", TOKENIZER_PRINT_DEC},
  { ":", TOKENIZER_COLON},
#if defined(UBASIC_SCRIPT_HAVE_STORE_VARS_IN_FLASH)
  { "store", TOKENIZER_STORE},
  { "recall", TOKENIZER_RECALL},
#endif
  { "clear", TOKENIZER_CLEAR},
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
    if (*(ptr+1) == '&')
    {
      if (offset)
        *offset += 1;
      return TOKENIZER_LAND;
    }
    return TOKENIZER_AND;
  }
  else if(*ptr == '|')
  {
    if (*(ptr+1) == '|')
    {
      if (offset)
        *offset += 1;
      return TOKENIZER_LOR;
    }
    return TOKENIZER_OR;
  }
  else if(*ptr == '*')
  {
    return TOKENIZER_ASTR;
  }
  else if(*ptr == '!')
  {
    return TOKENIZER_LNOT;
  }
  else if(*ptr == '~')
  {
    return TOKENIZER_NOT;
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
  uint8_t i,j;

  // eat all whitespace
  while(*ptr == ' ' || *ptr == '\t' || *ptr == '\r')
    ptr++;

  if(*ptr == 0)
  {
    return TOKENIZER_ENDOFINPUT;
  }

  uint8_t have_decdot=0, i_dot=0;
  if ( (ptr[0]=='0') && ((ptr[1]=='x')||(ptr[1]=='X')) )
  {
    // is it HEX
    nextptr = ptr + 2;
    while (1)
    {
      if (*nextptr>='0' && *nextptr<='9')
      {
        nextptr++;
        continue;
      }
      if ( (*nextptr>='a') && (*nextptr<='f') )
      {
        nextptr++;
        continue;
      }
      if ( (*nextptr>='A') && (*nextptr<='F') )
      {
        nextptr++;
        continue;
      }
      return TOKENIZER_INT;
    }
  }
  else if ( (ptr[0]=='0') && ((ptr[1]=='b')||(ptr[1]=='B')) )
  {
    // is it BIN
    nextptr = ptr + 2;
    while (*nextptr=='0' || *nextptr=='1')
      nextptr++;
    return TOKENIZER_INT;
  }
  else if( isdigit(*ptr) || (*ptr=='.') )
  {
    // is it
    //    FLOAT (digits with at most one decimal point)
    // or is it
    //    DEC (digits without decimal point which ends in d,D,L,l)
    nextptr = ptr;
    have_decdot = 0;
    i_dot = 0;
    while (1)
    {
      if (*nextptr>='0' && *nextptr<='9')
      {
        nextptr++;
        if (have_decdot)
          i_dot++;
        continue;
      }
      if (*nextptr=='.')
      {
        nextptr++;
        have_decdot++;
        if (have_decdot>1)
          return TOKENIZER_ERROR;
        continue;
      }
      if (*nextptr=='d' || *nextptr=='D' || *nextptr=='l' || *nextptr=='L')
        return TOKENIZER_INT;

  #if defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_24_8) || defined(VARIABLE_TYPE_FLOAT_AS_FIXEDPT_22_10)
      if (i_dot)
        return TOKENIZER_FLOAT;
  #endif

      return TOKENIZER_NUMBER;
    }
  }
  else if ( (j = singlechar_or_operator(&i)) )
  {
    nextptr = ptr + i;
//     if (j == TOKENIZER_EOL)
//       current_line++;
    return j;
  }
#if defined(VARIABLE_TYPE_STRING)
  else if (( *ptr == '"' || *ptr == '\'') && (*(ptr-1) != '\\') )
  {
    i = *ptr;
    nextptr = ptr;
    do
    {
      ++nextptr;
      if ((*nextptr=='\0')||(*nextptr=='\n')||(*nextptr==';'))
        return TOKENIZER_ERROR;
    }
    while(*nextptr != i || *(nextptr-1) == '\\');

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

  /**
    * what is left after this point we call a label as long as
    * it starts with "_" or a..z
    * and contains only digits and letters
    */
  i = 0;
  j = 0;
  if(*ptr == '_' || (*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <= 'Z') )
  {
    nextptr = ptr;
    while(1)
    {
      if ( *nextptr == '_' )
      {
        j++;
        nextptr++;
        continue;
      }
      if ( (*nextptr>='0') && (*nextptr<='9') )
      {
        i++;
        nextptr++;
        continue;
      }
      if ( (*nextptr>='a') && (*nextptr<='z') )
      {
        i++;
        nextptr++;
        continue;
      }
      if ( (*nextptr>='A') && (*nextptr<='Z') )
      {
        i++;
        nextptr++;
        continue;
      }

      if (j>0 || i>1)
      {
        return TOKENIZER_LABEL;
      }

      if (i == 1)
      {
#if defined(VARIABLE_TYPE_STRING)
        if (*(ptr+1) == '$')
        {
          nextptr++;
          return TOKENIZER_STRINGVARIABLE;
        }
#endif

#if defined(VARIABLE_TYPE_ARRAY)
        if (*(ptr+1) == '@')
        {
          nextptr++;
          return TOKENIZER_ARRAYVARIABLE;
        }
#endif
        return TOKENIZER_VARIABLE;
      }
      break;
    }
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
//   current_line = 1;
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
  uint8_t *c = (uint8_t *) ptr;
  VARIABLE_TYPE rval=0;

  while (1)
  {
    if (*c<'0' || *c>'9')
      break;

    rval *= 10;
    rval += (*c - '0');
    c++;
  }

  return rval;
}


VARIABLE_TYPE tokenizer_int(void)
{
  uint8_t *c = (uint8_t *) ptr;
  VARIABLE_TYPE rval=0;
  if ( (*c=='0') && (*(c+1)=='x' || *(c+1)=='X') )
  {
    c+= 2;
    while (1)
    {
      if (*c>='0' && *c<='9')
      {
        rval <<= 4;
        rval += (*c - '0');
        c++;
        continue;
      }
      if ((*c>='a') && (*c<='f'))
      {
        rval <<= 4;
        rval += (*c - 87); // 87 = 'a' - 10
        c++;
        continue;
      }
      if ((*c>='A') && (*c<='F'))
      {
        rval <<= 4;
        rval += (*c - 55);// 55 = 'A' - 10
        c++;
        continue;
      }
      break;
    }
    return rval;
  }
  if ( (*c=='0') && (*(c+1)=='b' || *(c+1)=='B') )
  {
    c+= 2;
    while (1)
    {
      if (*c=='0' || *c=='1')
      {
        rval <<= 1;
        rval += (*c - '0');
        c++;
        continue;
      }
      break;
    }
    return rval;
  }

  return tokenizer_num();
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

  /** figure out the quote used for strings
    * ignore escaped string-quotes
    */
  string_end = (char *) ptr;
  do
  {
    string_end++;

    string_end = strchr(string_end, quote_char);
    if(string_end == NULL)
    {
      return;
    }
  }
  while ( *(string_end - 1) == '\\');

  string_len = string_end - ptr - 1;
  if(len < string_len)
  {
    string_len = len;
  }
  memcpy(dest, ptr + 1, string_len);
  dest[string_len] = 0;
  return;
}
#endif

void tokenizer_label(char *dest, uint8_t len)
{
  char *string_end = (char *) nextptr;
  uint8_t string_len;

  if(tokenizer_token() != TOKENIZER_LABEL)
  {
    return;
  }

  for (string_len=0; string_len < string_end - ptr; string_len++)
  {
    if (  (*(ptr+string_len)=='_') ||
             ((*(ptr+string_len)>='0') && (*(ptr+string_len)<='9')) ||
             ((*(ptr+string_len)>='A') && (*(ptr+string_len)<='Z')) ||
             ((*(ptr+string_len)>='a') && (*(ptr+string_len)<='z')) )
      continue;
    break;
  }
  memcpy(dest, ptr, string_len);
  dest[string_len] = 0;
}

/*---------------------------------------------------------------------------*/
void tokenizer_error_print(VARIABLE_TYPE token)
{
  char msg[10];
//   if (ubasic_status.bit.isRunning == 1)
//   {
//     print_serial("Line ");
//     sprintf(msg,"%d:", current_line);
//     print_serial(msg);
//   }
  print_serial("Err");
  sprintf(msg,"[%u]:", (uint8_t) token);
  print_serial(msg);
  print_serial((char*)ptr-1);
  print_serial("\n");
}
/*---------------------------------------------------------------------------*/
uint8_t tokenizer_finished(void)
{
  if (ubasic_status.bit.isRunning == 1)
    return ((*ptr == 0) || (current_token == TOKENIZER_ENDOFINPUT));

  return ((*ptr == 0) || (current_token == TOKENIZER_ENDOFINPUT) || (ubasic_status.bit.Error == 1));
}
/*---------------------------------------------------------------------------*/
uint8_t tokenizer_variable_num(void)
{
  if ((*ptr >= 'a' && *ptr <= 'z'))
    return (((uint8_t) *ptr) - 'a');

  if ((*ptr >= 'A' && *ptr <= 'Z'))
    return (((uint8_t) *ptr) - 'A');

  return 0xff;
}
/*---------------------------------------------------------------------------*/

// uint16_t tokenizer_line_number(void)
// {
//   return current_line;
// }

uint16_t  tokenizer_save_offset(void)
{
  return (ptr - prog);
}

void      tokenizer_jump_offset(uint16_t offset)
{
  ptr = (prog + offset);
  current_token = get_next_token();
  while ( (current_token==TOKENIZER_EOL) && !tokenizer_finished() )
    tokenizer_next();
  return;
}


