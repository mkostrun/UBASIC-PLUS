# UBASIC PLUS

## UBASIC

uBasic is a small non-interactive interpreter of the programming language BASIC
which offers a subset of the commands that are part of the language:
- 26 integer variables, named 'a' through 'z'
- *if/then/else* with support for single symbol logical operators (>,<,=)
- *for/next*
- *let*
- *goto*
- *gosub*
- *print*
- '\n' (new line) as end-of-line character


uBasic was written by Adam Dunkels circa 2006, and the code is available at github: https://github.com/adamdunkels/ubasic.


## UBASIC WITH STRINGS

uBasic with strings is a enhanced version of uBasic which adds support for strings:
- 26 string variables, named a$-z$, string literals, variables and expressions can
be used in assignments and print statements string expressions can be compared
for equality in if statements string expressions can be concatenated using '+'
- *left$(s$, i)*
    returns the first i characters of s$
- *mid$(s$, i, j)*
    returns the substring of length j starting at position i in s$
- *right(s$, i, j)*
    returns the last i characters of s$
- *str$(i)*
    returns the integer value i as a string - opposite of val()
- *chr$(i)*
    returns the i'th ASCII character - opposite of asc()
- *val(s$)*
    returns the numeric value of the string of digits s$ - opposite of str()
- *len$(s$)*
    returns the number of characters in s$
- *instr$(t$, s$)*
    returns the position of the string t$ in s$ (or 0)
- *asc(s$)*
    returns the ASCII code for the first character in s$ - opposite of val()

uBasic with strings was written by David Mitchell circa 2008, and the code is
available from the author's web site: http://www.zenoshrdlu.com/kapstuff/zsubasic.html


## UBASIC PLUS

uBasic Plus combines 'uBasic with strings' with elements of 'CHDK' version of uBasic.
For CHDK please see http://chdk.wikia.com/wiki/CHDK_Scripting_Cross_Reference_Page.
It adds the following functionalities:
- No more line numbers:

Instead CHDK-style labels are used, *:some_label*.
However, unlike CHDK, when these labels are refered to in goto/gosub they are not
inside quotation marks. Also, internally the lines are not numbered,
rather pointers are used, so returns - from gosub, for/next, if/then/else, while/endwhile -
are very fast as there are no line searches.
- config.h, configuration which allows user to select which of the features below
(if any) they want in their uBasic build.
- Fixep point floats are implemented through Fixed Point Math Library for C
by Ivan Voras and Tim Hartnick, https://sourceforge.net/projects/fixedptc.
The libary is enhanced with str_fixedpt function, which converts a string to fixed point float.
- flow control
  - two character logical operators supported (<>,<=,>=,==,&&,||)
  - complex logical expressions supported with use of brackets
  - multi-line If-command (CHDK-style)
  - while/endwhile
- Strings can be encompassed by single or double quotations
- Besides '\n', ';' can also be used as end-of-line character
- println
      at the end of printout add an empty line '\n'
- Two character logical operators are also implemented 
- 26 fixed point arrays, named a@ to z@, dynamically allocated using DIM command,
and accessed through index, e.g., a@(i)
- *sleep(f)*,
      where f is fixed point number in seconds. sleep is NOT executed inside the BASIC interpreter
- *input {a,a$,a@(i)}, timeout*
      wait until timeout for input through serial port, external functions for serial-availabe and serial-read have to be supplied - see documentation in config.h
- ran
      system random number generator based on the external function - see documentation in config.h
- *tic(n), a=toc(n)*
      rlabplus-type six timers for measuring the passed time from different breakpoints in the script
- *sqrt, sin, cos, tan, exp, ln, pow*
      fixed point arithmetic single argument functions from fixed point math library
- *floor, ceil, round, abs*
      fixed point float to fixed point integer arithmetic functions
- *uniform*
      system fixed point random number generator in the range 0.000 to 0.999.
- *gpio(pin,state), a=gpio(pin)*
      direct control over digital pins. Tied to hardware specific functions as described in config.h
- *pwm(channel,value), pwm(channel)*
      direct control over output pins that support PWM on the micro-controller. Tied to hardware functions described in config.h
- *hw_event(channel)*
      access to flags that can be set outside BASIC interpreter, e.g., using interrupts, that are available to BASIC scripts for flow control.
- *aread(channel)*
      read input from analog channels 0:18 on the microcontroller. However, only some of the channels might be
      available.
- *UBASIC-PLUS* can be used as direct command interpreter for quick computations or setting the state of the hardware.

  ```
  ... gpio(1,1)
  ```
  will set the channel 1 gpio to 1 from the command line.
- *does not fit in anything above*

  In the scripts the numbers can be entered either directly as,
  ```
  ... x = 0xaabbccdd
  ```
  or
  ```
  ... x = 0b1101001
  ```
  or
  ```
  ... x = 123456[d,D,L,l]
  ```
  if default conversion to fixed point integer is not needed.

  The values can be printed directly as
  ```
  ... println [hex,dec] x 
  ```
  that is, as hexadecimal or decimal integer, if default printout,
  that of fixed point integer, is not needed.





The uBasic-Plus comprise of six files config.h, fixedptc.h, tokenizer.c,
tokenizer.h, ubasic.c  and  ubasic.h.
As an example implementation of the hardware related functions (random number generation,
gpio, hardware events, sleep and tic/toc) the development boards STM32F030-Nucleo64 and
STM32F051-Discovery are used in combination with CubeMX created system libraries.


## UBASIC PLUS SCRIPT EXAMPLES


### 1st example - warm up
```
println 'start of test 1';
gosub label1;
for i = 1 to 20;
  for j = 1 to 20;
    println 'i,j=',i,j;
  next j;
next i;
println 'end of test 1';
end;
:label1 
println 'subroutine';
return;
```

### 2nd example by David Mitchell for his 'uBasic with strings'
```
println 'start of test 2';
a$= 'abcdefghi';
b$='123456789';
println 'Length of a$=', len(a$);
println 'Length of b$=', len(b$);
if len(a$) = len(b$) then println 'same length';
if a$ = b$ then println 'same string';
c$=left$(a$+ b$,12);
println c$;
c$=right$(a$+b$, 12);
println c$;
c$=mid$(a$+b$, 8,8);
println c$;
c$=str$(13+42);
println c$;
println len(c$);
println len('this' + 'that');
c$ = chr$(34);
println 'c$=' c$;
j = asc(c$);
println 'j=' j;
println val('12345');
i=instr(3, '123456789', '67');
println 'position of 67 in 123456789 is', i;
println mid$(a$,2,2)+'xyx';
println 'end of test 2';
end;
```

### 3rd example - demonstration of Plus capabilities
```
println 'start of test 3';
tic(1);
for i = 1 to 2;
  j = i + 0.25 + 1/2;
  println 'j=' j;
  k = sqrt(2*j) + ln(4*i) + cos(i+j) + sin(j);
  println 'k=' k;
next i;
:repeat 
if toc(1)<=300 then goto repeat;
for i = 1 to 2;
  println 'ran(' i ')=' ran;
next i;
for i = 1 to 2;
  println 'uniform(' i ')=' uniform;
next i;
for i = 1 to 2;
  x = 10 * uniform;
  println 'x=' x;
  println 'floor(x)=' floor(x);
  println 'ceil(x)=' ceil(x);
  println 'round(x)=' round(x);
  println 'x^3=' pow(x,3);
next i;
println 'Analog Write Test';
for i = 1 to 5;
  for j = 1 to 4;
    pwm(j,4095*uniform);
  next j;
  sleep(0.5);
  println 'pwm=' pwm(1),pwm(2),pwm(3),pwm(4);
next i;
pwm(1,0);
pwm(2,0);
pwm(3,0);
pwm(4,0);
println 'GPIO 1:4 Test';
for i = 1 to 1;
  for j = 0 to 2;
    gpio(i,(j % 2));
    sleep(0.5);
  next j;
next i;
println 'gpio(1)=' gpio(1);
println 'gpio(2)=' gpio(2);
println 'Press the Blue Button!';
:presswait 
  if hw_event(1)=0 then goto presswait;
tic(1);
println 'Blue Button pressed!';
:deprwait 
  if hw_event(2)=0 then goto deprwait;
println 'duration =' toc(1);
println 'Blue Button de-pressed!';
println 'end of test 3';
end;
```

### 4th example - input array entries in 10 sec time
```
println 'start of test 4';
dim a@(5);
for i = 1 to 5;
  print '?';
  input a@(i),10000;
next i;
println 'end of input';
for i = 1 to 5;
  println 'a(' i ') = ' a@(i);
next i;
println 'end of test 4';
end
```

### 5th example - analog read with arrays
```
println 'start of test 5';
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
println 'end of test 5';
end;
```

### 6th example - if/then/else/endif and while/endwhile
```
println 'start of test 6';
println 'Test If:';
for i=1 to 10 step 0.125;
  x = uniform;
  if x>0.5 then;
    println x, 'is greater then 0.5';
  else;
    println x, 'is smaller then 0.5';
  endif;
  println i;
next i;
println 'Test While:';
while i>=0;
  i = i - 0.125;
  println 'i =', i;
endwhile;
println 'end of test 6';
end
```


## UBASIC PLUS HARDWARE IMPLEMENTATION EXAMPLES

For both boards a TX with circular buffer was implemented and tested.
These files are provided in the Drivers/HAL.. directory and are the
same for both boards.


### STM32F030R8-Nucleo64

About the board:
- Arm Cortex M0 64kb flash, 8kb ram, 48MHz frequency
- GPIO: PC0, PC1, PC2, PC3
- PWM:  Timer 3 Channels 1 (PA6), 2 (PA7), 3 (PB0) and 4 (PB1)
- Analog Input: On STM32F0 these Channels are mapped to pins as follows 0:7 as A0:A7,
8:9 as B0:B1, and 10:15 as C0:C5. Channel 16 is the temperature, and 17 the VREF.
- Hardware Events:
  - Blue Push Button 1 (PC13) with two events:
    - button pressed - hw_event(1), and
    - button depressed - hw_event(2)
- sleep, tic/toc:

  through *SysTick_Handler()* which counts in 1ms increments through interrupts

- Serial Port

  through USB connector used for debugging (UART2 on PA2/PA3)

- firmware footprint with all features enabled and 6 test scripts (bytes):
  32080 flash, 552 data, 2768 bss.

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
    - button pressed - hw_event(1), and
    - button depressed - hw_event(2)
- sleep, tic/toc:

  through *SysTick_Handler()* which counts in 1ms increments through interrupts

- Serial Port

  requires an additional USB/TTL cable (UART2 using PA2/PA3 on the discovery board)

- firmware footprint with all features enabled and 7 test scripts (bytes):
  32416 flash, 556 data, 2828 bss.
