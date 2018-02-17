#include "cli.h"

uint8_t is_cmd(char *statement, const char * cmd)
{
  if (strlen(statement)<strlen(cmd))
    return 0;

  uint8_t i;
  for (i=0; i<strlen(cmd); i++)
  {
    if ( statement[i] != cmd[i] )
      return 0;
  }

  return 1;
}
