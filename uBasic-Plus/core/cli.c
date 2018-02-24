#include "cli.h"
#include "ubasic.h"
#include "../hardware/usart.h"

/* Example Scripts for demo command ---------------------------------------------------------*/
const char welcome_msg[]=
"\
Welcome to uBasic-Plus for STM32 by M.Kostrun\n\
Expands upon uBasic by A.Dunkels, uBasic with string by D.Mitchell,\n\
and uBasic for CHDK by P.d'Angelo\n>";

#if defined(UBASIC_SCRIPT_HAVE_DEMO_SCRIPTS)
static const char *program[] = {

"\
println 'Demo 1 - Warm-up';\
gosub l1;\
for i = 1 to 2;\
for j = 1 to 2;\
println 'i,j=',i,j;\
next j;\
next i;\
println 'Demo 1 Completed';\
end;\
:l1 \
println 'subroutine';\
return;",

"\
println 'Demo 2 - ubasic with strings';\
a$= 'abcdefghi';\
b$='123456789';\
println 'Length of a$=', len(a$);\
println 'Length of b$=', len(b$);\
if len(a$) = len(b$) then println 'same length';\
if a$ = b$ then println 'same string'\n\
c$=left$(a$+ b$,12);\
println c$;\
c$=right$(a$+b$, 12);\
println c$;\
c$=mid$(a$+b$, 8,8);\
println c$;\
c$=str$(13+42);\
println c$;\
println len(c$);\
println len('this' + 'that');\
c$ = chr$(34);\
println 'c$=' c$;\
j = asc(c$);\
println 'j=' j;\
println val('12345');\
i=instr(3, '123456789', '67');\
println 'position of 67 in 123456789 is', i;\
println mid$(a$,2,2)+'xyx';\
println 'Demo 2 Completed';",

"\
println 'Demo 3 - Plus';\
tic(1);\
for i = 1 to 2;\
  j = i + 0.25 + 1/2;\
  println 'j=' j;\
  k = sqrt(2*j) + ln(4*i) + cos(i+j) + sin(j);\
  println 'k=' k;\
next i;\
:repeat \
  if (toc(1)<=300) then goto repeat;\
for i = 1 to 2;\
println 'ran(' i ')=' ran;\
next i;\
for i = 1 to 2;\
println 'uniform(' i ')=' uniform;\
next i;\
for i = 1 to 2;\
x = 10 * uniform;\
println 'x=' x;\
println 'floor(x)=' floor(x);\
println 'ceil(x)=' ceil(x);\
println 'round(x)=' round(x);\
println 'x^3=' pow(x,3);\
next i;\
println 'Digital Write Test';\
pinmode(0xc0,-1,0);\
pinmode(0xc1,-1,0);\
pinmode(0xc2,-1,0);\
pinmode(0xc3,-1,0);\
for j = 0 to 2;\
  dwrite(0xc0,(j % 2));\
  dwrite(0xc1,(j % 2));\
  dwrite(0xc2,(j % 2));\
  dwrite(0xc3,(j % 2));\
  sleep(0.5);\
next j;\
println 'Press the Blue Button or type kill!';\
:presswait \
  if (flag(1)==0) then goto presswait;\
tic(1);\
println 'Blue Button pressed!';\
:deprwait \
  if (flag(2)==0) then goto deprwait;\
println 'duration =' toc(1);\
println 'Blue Button de-pressed!';\
println 'Demo 3 Completed';\
end;",

"\
println 'Demo 4 - Input with timeouts';\
dim a@(5);\
for i = 1 to 5;\
  print '?';\
  input a@(i),10000;\
next i;\
println 'end of input';\
for i = 1 to 5;\
  println 'a(' i ') = ' a@(i);\
next i;\
println 'Demo 4 Completed';\
end",

"\
println 'Demo 5 - analog inputs and arrays';\
aread_conf(7,16);\
for i = 1 to 5;\
  x = aread(16);\
  y = aread(17);\
  println 'VREF,TEMP=', x, y;\
next i;\
for i = 1 to 1;\
  n = floor(10 * uniform) + 2 ;\
  dim b@(n);\
  for j = 1 to n;\
    b@(j) = ran;\
    println 'b@(' j ')=' b@(j);\
  next j;\
next i;\
println 'Demo 5 Completed';\
end;",


"\
println 'Demo 6: Multiline if, while';\
println 'Test If: 1';\
for i=1 to 10 step 0.125;\
  x = uniform;\
  if (x>=0.5) then;\
    println x, 'is greater then 0.5';\
  else;\
    println x, 'is smaller then 0.5';\
  endif;\
  println 'i=' i;\
next i;\
println 'End of If-test 1';\
println 'Test While: 1';\
i=10;\
while ((i>=0)&&(uniform<=0.9));\
  i = i - 0.125;\
  println 'i =', i;\
endwhile;\
println 'End of While-test 1';\
println 'Demo 6 Completed';\
end",

"\
println 'Demo 7: Analog Read or Kill';\
y=0;\
:startover \
  x = aread(10);\
  if (abs(x-y)>20) then;\
    y = x;\
    println 'x=',x;\
  endif;\
  sleep (0.2);\
goto startover;\
end",

"\
println 'Demo 8: analog write (PWM) 4-Channel Test';\
p = 65536;\
for k = 1 to 10;\
  p = p/2;\
  awrite_conf(p,4096);\
  println 'prescaler = ' p;\
  for i = 1 to 10;\
    for j = 1 to 4;\
      awrite(j,4095*uniform);\
    next j;\
    println '    analog write = ' awrite(1),awrite(2),awrite(3),awrite(4);\
    sleep(5);\
  next i;\
next k;\
awrite(1,0);\
awrite(2,0);\
awrite(3,0);\
awrite(4,0);\
println 'Demo 8 Completed';\
end",

"\
clear;\
println 'Demo 9: store/recall with FLASH';\
if (recall(x)==0) then;\
  println 'generating x';\
  x = uniform;\
  store(x);\
endif;\
println 'stored: x=' x;\
if (recall(y@)==0) then;\
  println 'generating y';\
  dim y@(10);\
  for i=1 to 10;\
    y@(i) = uniform;\
  next i;\
  store(y@);\
endif;\
println 'stored: y@';\
for i=1 to 10;\
  println '  y@('i')=' y@(i);\
next i;\
if (recall(s$)==0) then;\
  println 'generating s';\
  s$='what is going on?';\
  store(s$);\
endif;\
println 'stored: s$',s$;\
println 'Demo 9 Completed';\
end"
};
#endif

/* Private variables ---------------------------------------------------------*/
static char script[UBASIC_SCRIPT_SIZE_MAX];
static char statement[UBASIC_STATEMENT_SIZE_MAX];
static uint8_t cli_state=UBASIC_CLI_IDLE;

void ubasic_cli(void)
{

#if defined(UBASIC_SCRIPT_HAVE_STORE_VARS_IN_FLASH)
  EE_Init();
#endif

  if ( (cli_state == UBASIC_CLI_LOADED) || (cli_state == UBASIC_CLI_RUNNING) )
  {
    ubasic_run_program();
    cli_state = UBASIC_CLI_RUNNING;
    if (ubasic_finished())
    {
      cli_state = UBASIC_CLI_IDLE;
      print_serial("\n>");
    }
    else if (!ubasic_waiting_for_input())
    {
      if (serial_input_available())
      {
        serial_input(statement,sizeof(statement));
        if (strstr(statement,"kill"))
        {
            // enter programming mode
          print_serial("killed\n>");
          cli_state = UBASIC_CLI_IDLE;
          ubasic_load_program(NULL);
          return;
        }
      }
    }
  }

  if (cli_state != UBASIC_CLI_RUNNING)
  {
    if (serial_input_available())
    {
      serial_input(statement,sizeof(statement));

      if (strstr(statement,"prog"))
      {
          // enter programming mode
        script[0] = 0;
        print_serial("Enter your script. Type 'run' to execute!\n>");
        cli_state = UBASIC_CLI_PROG;
        return;
      }
      else if (strstr(statement,"run"))
      {
          // run script
        print_serial("run\n");
        if (strlen(script) > 0)
        {
          ubasic_load_program( script );
          cli_state = UBASIC_CLI_LOADED;
        }
        return;
      }
      else if (strstr(statement,"cat"))
      {
          // list script
        print_serial("cat\n");
        if (strlen(script)>0)
        {
          print_numbered_lines(script);
          print_serial("\n");
//             print_serial(script);
//             print_serial("\n");
        }
        print_serial(">");
        return;
      }
      else if (strstr(statement,"save"))
      {
          // save script: exit PROG mode
        print_serial("save\n>");
        if (strlen(script)>0)
        {
          cli_state = UBASIC_CLI_IDLE;
        }
        return;
      }
      else if (strstr(statement,"edit"))
      {
          // edit script: re-enter PROG mode
        print_serial("edit\n>");
        if (strlen(script)>0)
        {
          cli_state = UBASIC_CLI_PROG;
        }
        return;
      }
#if defined(UBASIC_SCRIPT_HAVE_DEMO_SCRIPTS)
      else if (strstr(statement,"demo"))
      {
          // run script
        print_serial(statement);
        print_serial("\n");
        char *s = &statement[4];
        while (*s==' ') ++s;
        uint8_t idx = *s - '0';
        if (idx<10)
        {
          ubasic_load_program( program[idx-1] );
          cli_state = UBASIC_CLI_LOADED;
        }
        else
        {
          print_serial("demo script out of range!\n");
        }
        return;
      }
#endif
#if defined(UBASIC_SCRIPT_HAVE_STORE_VARS_IN_FLASH)
      else if (strstr(statement,"flash"))
      {
        // test write
        print_serial("flash\n");
        EE_DumpFlash();
        print_serial(">");
        return;
      }
#endif
      if (cli_state == UBASIC_CLI_PROG)
      {
          // add statement to the script
          // put ';' at the end of each new line
        if (strlen(script)>0)
        {
          while (script[strlen(script)-1]==' ' || script[strlen(script)-1]=='\t')
            script[strlen(script)-1]='\0';
          if (script[strlen(script)-1] != '\n' && script[strlen(script)-1] != ';')
            sprintf(&script[strlen(script)], "\n");
        }
        char *s = statement;
        while (*s==' ') s++;
        sprintf(&script[strlen(script)], s);
        print_serial(s);
        print_serial("\n>");
      }
      else
      {
          // prepare statement for execution
        if (strlen(statement) > 0)
        {
          print_serial(statement);
          print_serial("\n");
          ubasic_load_program( statement );
          cli_state = UBASIC_CLI_LOADED;
        }
      }
    }
  }

  return;
}


