#ifndef __CLI_H__
#define __CLI_H__

#include "config.h"

#define UBASIC_CLI_IDLE       0
#define UBASIC_CLI_LOADED     1
#define UBASIC_CLI_RUNNING    2
#define UBASIC_CLI_PROG       3

uint8_t is_cmd(char *statement, const char * cmd);

#endif
