# UBASIC PLUS

uBasicPlus is an extension of the uBasic by Adam Dunkels (2006),
https://github.com/adamdunkels/ubasic, based on
'uBasic with strings' by David Mitchell (2008),
http://www.zenoshrdlu.com/kapstuff/zsubasic.html,
and elements of 'CHDK', http://chdk.wikia.com/wiki/CHDK_Scripting_Cross_Reference_Page.

## uBasic-Plus Interpreter

- UBASIC, which can be a small non-interactive interpreter of
the programming language BASIC which offers a subset of the commands
that are part of the language,
26 integer variables, named 'a' through 'z',
*if/then/else* with support for single symbol logical operators (>,<,=),
*for/next*, *let*, *goto*, *gosub*, *print* and '\n' (new line) as end-of-line character.

- uBasic with strings, which adds support for strings:
26 string variables, named a$-z$, string literals, variables and expressions can
be used in assignments and print statements string expressions can be compared
for equality in if statements string expressions can be concatenated using '+',
and a number of string functions: *left$*, *mid$*, *right*, *str$*, *chr$*,
*val*, *len$*, *instr$*, *asc*.

- No more line numbers:
Instead CHDK-style labels are used, *:some_label* . However, unlike CHDK, when these labels are refered to in goto/gosub they are not
inside quotation marks. Labels can use '_' in their names.
Also, internally the lines are not numbered,
rather pointers are used, so returns - from gosub, for/next, if/then/else, while/endwhile -
are very fast as there are no line searches.

- Strings can be encompassed by single or double quotations. Tokenizer can identify labels.

- End of line characters *'\n'* and *';'*

- *config.h*

  Allows user to select which of the features below (if any) they want in their uBasic build.

- Fixep point floats are implemented through Fixed Point Math Library for C
by Ivan Voras and Tim Hartnick, https://sourceforge.net/projects/fixedptc.
The libary is enhanced with str_fixedpt function, which converts a string to fixed point float.

- flow control
  - more logical operators supported (<>,<=,>=,==,&&,||,!)
  - complex logical expressions supported with use of brackets
  - multi-line If/then/else/endif-command (CHDK-style)
  - while/endwhile

- *input {a,a$,a@(i)}, timeout_ms*

   Wait at most until *timeout_ms* for input from the serial port. Wait is not executed
inside the interpreter.
Relies on external functions for serial-available and serial-read to be supplied as
documented in *config.h* .


- 26 fixed point arrays, a@ to z@, dynamically allocated using DIM command,
  ```
  dim a@(5);
  for i = 1 to 5;
    input a@(i),10000;
  next i;
  ```
  In this example code expects 5 array entries to be entered through serial port, and
waits at most 10sec for each input to complete.
In the scripts or from the input line the numbers can be entered either directly
as hex values,
  ```
  ... x = 0xaabbccdd
  ```
  or as binary values,
  ```
  ... x = 0b1101001
  ```
  or as decimal values,
  ```
  ... x = 123456[d,D,L,l]
  ```
  if the default fixed point float is not desired.


- *println*

  Same as *print* but adds an empty line at the end. Additional identifiers *hex* or *dec*
can be used to print the number as (hex)adecimal if fixed point floats are not desired, as
in this example:
  ```
  ... println [hex,dec] x 
  ```

- *sleep(f)*

  Wait *f* seconds, which can be fixed point float.
This is not executed inside the BASIC interpreter.


- *ran, uniform*

  System random number generators based on the external function as documented in *config.h* .
On STM32 boards listed below, *ran* and *uniform* use two lowest bits after the analog read from
the internal temperature sensor. From multiple calls required number of random bits is collected.

    - *ran* - generates random positive integer in fixed point float representation.

    - *uniform* - generates random fixed point float in the range 0 to 0.999.


- *tic(n), a=toc(n)*

  rlabplus-type six timers for measuring the passed time from different breakpoints in the script

- *sqrt, sin, cos, tan, exp, ln, pow*

  fixed point arithmetic single argument functions from the fixed point math library.

- *floor, ceil, round, abs*

  fixed point float to fixed point integer arithmetic functions.


- *pinmode(pin,mode,speed), a=dread(pin), dwrite(pin,state)*

  Allows direct control over digital pins.

  - pin: 

    The pin designation follows their hardware
names, 0xa0:0xaf for pins 0 through 15 on port A, and so forth. It is up to user to verify
that particular pins are available for digital read/write operations.
Tied to hardware specific functions as described in *config.h* .

  - mode: STM32 specific

    Output: -2 for open drain, -1 for push-pull

    Input:  0 for no pull, 1 for push-up, 2 for push-down.

  - speed: STM specific

    0 for low, 1 for medium and 2 for high frequency.

- *awrite_conf(prescaler,period), awrite(channel,value), awrite(channel)*

  direct control over output pins that support PWM on the micro-controller.
The *value* has to be in the range 0 to *period*-1.
Tied to hardware functions described in config.h


- *i = flag(channel)*

  Allow flags that can be set outside BASIC interpreter, e.g., using interrupts,
to be used in flow control.
  ```
  :waithere
      if (flag(1)==0) then goto waithere;
  ```
  In this example the script sits in this loop until the hardware event flag no. 1
gets set by external process. Importantly, after the interpreter recognizes that the
flag has been set, it immediately resets it so the subsequent calls will return 0
until the flag is set again externally.

  Importantly, while waiting for the flag, the microcontrolled does not sit inside the interpreter.

- *aread_conf(duration,nreads), i = aread(channel)*

  Read input from analog channels 0:18 from the microcontroller. The pin assignments of
the analog inputs for some microcontrollers is given in hardware examples below.
Some channels may not be available.

  - duration: STM specific

    0 through 7 for varius preprogrammend duration of analog sampling and conversion, higher
the value longer the duration.

  - nreads: value reported to the user will be an average of so many readings.


- *clear*

  Clears all variables, arrays and strings. It is good practice to put it as the
first line of the script.


- *store(x[x$,x@]), i=recall(x[x$,x@])*

  Store and recall variable, string or array in FLASH. In that way variable can survive
reboot of the device. The single page from FLASH is used as a scratch space, so when
there is no more space left on the page, the entire page is erased.


## uBasic-Plus *Command Line Interface* 

The Command Line Interface (CLI) supports the following commands:

- *prog*

  The CLI enters the line input mode. Everything entered on the serial port is considered
as a sequential line in the script. Repeated execution of *prog* erases the previous script.

- *run*

  The CLI executes what ever has been assembled from multiple inputs as a BASIC script.

- *cat*

  Print the script that has been assembed so far.

- *save*

  exit the *prog* mode.

- *edit*

  enter the *prog* mode. Subsequent lines of input will be attached to already existing
script.

- *demo N*

  execute demo script number N.

- *kill*

  Stops the script if it is being executed, and returns control to command prompt.

- If in *prog* mode, every typed line is added to the script until *save* or *run* is
executed.

- If not in *prog* mode, every typed line is executed as its own script, but the
uBasic-Plus internal storage is not erased in between the executions.

The uBasic-Plus comprise of six files config.h, fixedptc.h, tokenizer.c,
tokenizer.h, ubasic.c  and  ubasic.h.
As an example implementation of the hardware related functions (random number generation,
gpio, hardware events, sleep and tic/toc) the development boards STM32F030-Nucleo64 and
STM32F051-Discovery are used in combination with CubeMX created system libraries.


## UBASIC PLUS SCRIPT DEMOS


### Demo 1 - warm up
```
println 'Demo 1 - Warm-up'
gosub l1
for i = 1 to 2
  for j = 1 to 2
    println 'i,j=',i,j
  next j
next i
println 'Demo 1 Completed'
end
  :l1
println 'subroutine'
return
```

### Demo 2 - 'uBasic with strings' by David Mitchell
```
println 'Demo 2'
a$= 'abcdefghi'
b$='123456789'
println 'Length of a$=', len(a$)
println 'Length of b$=', len(b$)
if len(a$) = len(b$) then println 'same length'
if a$ = b$ then println 'same string'
c$=left$(a$+ b$,12)
println c$
c$=right$(a$+b$, 12)
println c$
c$=mid$(a$+b$, 8,8)
println c$
c$=str$(13+42)
println c$
println len(c$)
println len('this' + 'that')
c$ = chr$(34)
println 'c$=' c$
j = asc(c$)
println 'j=' j
println val('12345')
i=instr(3, '123456789', '67')
println 'position of 67 in 123456789 is', i
println mid$(a$,2,2)+'xyx'
println 'Demo 2 Completed'
end
```

### Demo 3 - uBasic-Plus is here
```
println 'Demo 3 - Plus'
tic(1)
for i = 1 to 2
  j = i + 0.25 + 1/2
  println 'j=' j
  k = sqrt(2*j) + ln(4*i) + cos(i+j) + sin(j)
  println 'k=' k
next i
:repeat
  if toc(1)<=300 then goto repeat
for i = 1 to 2
println 'ran(' i ')=' ran
next i
for i = 1 to 2
println 'uniform(' i ')=' uniform
next i
for i = 1 to 2
x = 10 * uniform
println 'x=' x
println 'floor(x)=' floor(x)
println 'ceil(x)=' ceil(x)
println 'round(x)=' round(x)
println 'x^3=' pow(x,3)
next i
println 'Digital Write Test'
pinmode(0xc0,-1,0)
pinmode(0xc1,-1,0)
pinmode(0xc2,-1,0)
pinmode(0xc3,-1,0)
for j = 0 to 2
  dwrite(0xc0,(j % 2))
  dwrite(0xc1,(j % 2))
  dwrite(0xc2,(j % 2))
  dwrite(0xc3,(j % 2))
  sleep(0.5)
next j
println 'Press the Blue Button or type kill!'
:presswait
  if flag(1)=0 then goto presswait
tic(1)
println 'Blue Button pressed!'
:deprwait
  if flag(2)=0 then goto deprwait
println 'duration =' toc(1)
println 'Blue Button de-pressed!'
println 'Demo 3 Completed'
end
```

### Demo 4 - input array entries in 10 sec time
```
println 'Demo 4 - Input with timeouts'
dim a@(5)
for i = 1 to 5
  print '?'
  input a@(i),10000
next i
println 'end of input'
for i = 1 to 5
  println 'a(' i ') = ' a@(i)
next i
println 'Demo 4 Completed'
end
```

### Demo 5 - analog read with arrays
```
println 'Demo 5 - analog inputs and arrays';
for i = 1 to 100;
  x = aread(16);
  y = aread(17);
  println 'VREF,TEMP=', x, y;
next i;
for i = 1 to 1;
  n = floor(10 * uniform) + 2 ;
  dim b@(n);
  for j = 1 to n;
    b@(j) = ran;
    println 'b@(' j ')=' b@(j);
  next j;
next i;
println 'Demo 5 Completed';
end;
```

### Demo 6 - if/then/else/endif and while/endwhile
```
println 'Demo 6: Multiline if, while'
println 'Test If: 1'
for i=1 to 10 step 0.125
  x = uniform
  if (x>=0.5) then
    println x, 'is greater then 0.5'
  else
    println x, 'is smaller then 0.5'
  endif
  println 'i=' i
next i
println 'End of If-test 1'
println 'Test While: 1'
i=10
while ((i>=0)&&(uniform<=0.9))
  i = i - 0.125
  println 'i =', i
endwhile
println 'End of While-test 1'
println 'Demo 6 Completed'
end
```

### Demo 7 - kill test
```
println 'Demo 7: Analog Read or Kill'
y=0
:startover
  x = aread(10)
  if (abs(x-y)>20) then
    y = x
    println 'x=',x
  endif
  sleep (0.2)
  goto startover
end
```


### Demo 8 - PWM Test
```
println 'Demo 8: analog write (PWM) 4-Channel Test'
p = 65536
for k = 1 to 10
  p = p/2
  awrite_conf(p,4096)
  println 'prescaler = ' p
  for i = 1 to 10
    for j = 1 to 4
      awrite(j,4095*uniform)
    next j
    println '    analog write = ' awrite(1),awrite(2),awrite(3),awrite(4)
    sleep(5)
  next i
next k
awrite(1,0)
awrite(2,0)
awrite(3,0)
awrite(4,0)
end
```

### Demo 9 - Store/recall in FLASH Test
```
clear
println 'Demo 9: store/recall with FLASH'
if (recall(x)==0) then
  println 'generating x'
  x = uniform
  store(x)
endif
println 'stored: x=' x
if (recall(y@)==0) then
  println 'generating y'
  dim y@(10)
  for i=1 to 10
    y@(i) = uniform
  next i
  store(y@)
endif
println 'stored: y@'
for i=1 to 10
  println '  y@('i')=' y@(i)
next i
if (recall(s$)==0) then
  println 'generating s'
  s$='what is going on?'
  store(s$)
endif
println 'stored: s$',s$
println 'Demo 9 Completed'
println 'Please run it once more'
end
```

## UBASIC PLUS HARDWARE SUPPORT

- STM32F0

  For both boards a HAL UART library with RX using circular buffer was implemented and tested.
The library provides Arduino-type serial-available and serial-read functions. The library is
the same for both STM32F0 boards. The library is located in Drivers folder and should replace
the same by STM.


### STM32F030R8-Nucleo64

About the board:
- Arm Cortex M0 64kb flash, 8kb ram, 48MHz frequency
- GPIO: PC0, PC1, PC2, PC3
- PWM:  Timer 3 Channels 1 (PA6), 2 (PA7), 3 (PB0) and 4 (PB1)
- Analog Input: On STM32F0 these Channels are mapped to pins as follows 0:7 as A0:A7,
8:9 as B0:B1, and 10:15 as C0:C5. Channel 16 is the temperature, and 17 the VREF.
- Hardware Events:
  - Blue Push Button 1 (PC13) with two events:
    - button pressed - flag(1), and
    - button depressed - flag(2)
- sleep, tic/toc:

  through *SysTick_Handler()* which counts in 1ms increments through interrupts

- Serial Port

  through USB connector used for debugging (UART2 on PA2/PA3)


### STM32F051R8-Discovery

About the board:
- Arm Cortex M0 64kb flash, 8kb ram, 48MHz frequency
- GPIO: PC0, PC1, PC2, PC3
- PWM:  Timer 3 Channels 1 (PC6), 2 (PC7), 3 (PC8/LED4-blue) and 4 (PC9/LED3-green)
- Analog Input: On STM32F0 these Channels are mapped to pins as follows 0:7 as A0:A7,
8:9 as B0:B1, and 10:15 as C0:C5. Internal channel 16 is the temperature, and 17 the VREF.
- Analog output: Channel 1.
- Hardware Events:
  - Blue Push Button 1 (PA0) with two events:
    - button pressed - flag(1), and
    - button depressed - flag(2)
- sleep, tic/toc:

  through *SysTick_Handler()* which counts in 1ms increments through interrupts

- Serial Port

  requires an additional USB/TTL cable (UART2 using PA2/PA3 on the discovery board)



Firmware footprint with all features enabled and 8 demo scripts (bytes):
37872 flash, 548 data and 3940 bss.
