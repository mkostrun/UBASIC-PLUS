#ifndef __CLI_H__
#define __CLI_H__

#include "config.h"

#define UBASIC_CLI_IDLE       0
#define UBASIC_CLI_LOADED     1
#define UBASIC_CLI_RUNNING    2
#define UBASIC_CLI_PROG       3

#define UBASIC_SCRIPT_SIZE_MAX  (1024)
#define UBASIC_STATEMENT_SIZE_MAX  (64)

extern const char welcome_msg[];
void ubasic_cli(void);

#endif
