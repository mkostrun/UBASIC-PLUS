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

/*
 * Modified to support fixed point arithmetic, and number of math and io and
 * hardware functions by Marijan Kostrun, January 2018.
 * Added config.h file that allows detailed configuration of the UBASIC.
 */


#ifndef __UBASIC_H__
#define __UBASIC_H__

#include "config.h"

void ubasic_init(const char *program);
void ubasic_var_init();
void ubasic_run(void);
uint8_t ubasic_finished(void);

VARIABLE_TYPE ubasic_get_variable(uint8_t varnum);
void ubasic_set_variable(uint8_t varum, VARIABLE_TYPE value);

#if defined(VARIABLE_TYPE_ARRAY)
void ubasic_dim_arrayvariable(uint8_t varnum, uint16_t size);
void ubasic_set_arrayvariable(uint8_t varnum, uint16_t idx,  VARIABLE_TYPE value);
VARIABLE_TYPE ubasic_get_arrayvariable(uint8_t varnum, uint16_t idx);
#endif

#if defined(VARIABLE_TYPE_STRING)
// string addition
char* ubasic_get_stringvariable(uint8_t);
void  ubasic_set_stringvariable(uint8_t, char *);
// end of string addition
#endif

#endif /* __UBASIC_H__ */
