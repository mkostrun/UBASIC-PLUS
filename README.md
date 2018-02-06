UBASIC




uBasic is a small non-interactive interpreter of the programming language BASIC
which offers a subset of the commands that are part of the language:
- 26 integer variables, named 'a' through 'z'
- if/then/else, with support for single symbol logical operators (>,<,=)
- for/next
- let
- goto
- gosub
- print
- 'new line' as end-of-line character
It also supports some mathematical expressions for integer variables, where
the variables can have only single character names.
It was written by Adam Dunkels circa 2006, and the code is available at github:

https://github.com/adamdunkels/ubasic.




UBASIC WITH STRINGS

uBasic with strings is a enhanced version of uBasic which adds support for strings:
- 26 string variables, named a$-z$, string literals, variables and expressions can
be used in assignments and print statements string expressions can be compared
for equality in if statements string expressions can be concatenated using '+'
- left$(s$, i)
    returns the first i characters of s$
- mid$(s$, i, j)
    returns the substring of length j starting at position i in s$
- right(s$, i, j),
    returns the last i characters of s$
- str$(i),
    returns the integer value i as a string - opposite of val()
- chr$(i),
    returns the i'th ASCII character - opposite of asc()
- val(s$),
    returns the numeric value of the string of digits s$ - opposite of str()
- len$(s$),
    returns the number of characters in s$
- instr$(t$, s$),
    returns the position of the string t$ in s$ (or 0)
- asc(s$),
    returns the ASCII code for the first character in s$ - opposite of val()
    uBasic with strings was written by David Mitchell circa 2008, and the code is
    available from the author's web site:

http://www.zenoshrdlu.com/kapstuff/zsubasic.html




UBASIC PLUS

uBasic Plus is a further enhanced version of 'uBasic with strings' which adds
the following functionalities:
- config.h, configuration which allows user to select which features they want
in their uBasic build
- Fixed Point Math Library for C by Ivan Voras and Tim Hartnick, from
https://sourceforge.net/projects/fixedptc. Their libary is enhanced with a function
str_fixedpt, which converts a string to fixed point integer.
- Strings can be encompassed by single or double quotations
- Besides '\n', ';' can also be used as end-of-line character
- println
    at the end of printout add an empty line '\n'
- Two character logical operators are also implemented (<>,<=,>=,==)
- 26 fixed point arrays, named a@ to z@, dynamically allocated using DIM command,
and accessed through index, e.g., a@(i)
- sleep(f),
    where f is fixed point number in seconds. sleep is NOT executed inside
    the BASIC interpreter
- input {a,a$,a@(i)}, timeout
    wait until timeout for input through serial port, external functions for
    serial-availabe and serial-read have to be supplied - see documentation in config.h
- ran
    system random number generator based on the external function - see documentation
    in config.h
- tic(n), a=toc(n)
    rlabplus type six timers for measuring the passed time from different breakpoints
    in the script
- sqrt, sin, cos, tan, exp, ln, pow
    fixed point arithmetic single argument functions from fixed point math library
- floor, ceil, round
    fixed point float to fixed point integer arithmetic functions
- uniform
    system fixed point random number generator in the range 0.000 to 0.999.
- gpio(pin,state), gpio(pin)
    direct control over digital pins. Tied to hardware specific functions as described
    in config.h
- pwm(channel,value)
    direct control over output pins that support PWM on the micro-controller. Tied to
    hardware functions described in config.h
- hw_event(channel)
    access to flags that can be set outside BASIC interpreter, e.g., using interrupts,
    that are available to BASIC scripts for flow control.

The uBasic-Plus comprise of five six files config.h, fixedptc.h, tokenizer.c,
tokenizer.h, ubasic.c  and  ubasic.h.
As an example implementation of the hardware related functions (random number generation,
gpio, hardware events, sleep and tic/toc) the development boards STM32F030-Nucleo64 and
STM32F051-Discovery are used in combination with CubeMX created system libraries.




UBASIC PLUS SCRIPT EXAMPLES


1st example - warm up

9 println 'start of test 1'
10 gosub 100
20 for i = 1 to 20
21 for j = 1 to 20
30 println 'i,j=',i,j
31 next j
40 next i
50 println 'end of test 1'
60 end
100 println 'subroutine'
110 return


2nd example by David Mitchell for his 'uBasic with strings'

1 println 'start of test 2'
2 a$= 'abcdefghi'
3 b$='123456789'
5 println 'Length of a$=', len(a$)
6 println 'Length of b$=', len(b$)
7 if len(a$) = len(b$) then println 'same length'
8 if a$ = b$ then println 'same string'
9 c$=left$(a$+ b$,12)
10 println c$
11 c$=right$(a$+b$, 12)
12 println c$
13 c$=mid$(a$+b$, 8,8)
14 println c$
15 c$=str$(13+42)
16 println c$
17 println len(c$)
18 println len('this' + 'that')
19 c$ = chr$(34)
20 println 'c$=' c$
21 j = asc(c$)
22 println 'j=' j
23 println val('12345')
24 i=instr(3, '123456789', '67')
24 println 'position of 67 in 123456789 is', i
25 println mid$(a$,2,2)+'xyx'
30 println 'end of test 2'


3rd example - deme of Plus

10 println 'start of test 3'
11 tic(1)
12 for i = 1 to 20
13 j = i + 0.25 + 1/2
14 println 'j=' j
15 k = sqrt(2*j) + ln(4*i) + cos(i+j) + sin(j)
16 println 'k=' k
20 next i
21 if toc(1)<=300 then goto 21
31 for i = 1 to 20
32 println 'ran(' i ')=' ran
33 next i
40 for i = 1 to 20
41 println 'uniform(' i ')=' uniform
42 next i
50 for i = 1 to 20
51 x = 10 * uniform
52 println 'x=' x
53 println 'floor(x)=' floor(x)
54 println 'ceil(x)=' ceil(x)
55 println 'round(x)=' round(x)
56 println 'x^3=' pow(x,3)
57 next i
60 println 'Analog Write Test'
61 for i = 1 to 10
62 for j = 1 to 4
63 pwm(j,4095*uniform)
64 next j
65 sleep(1)
66 println 'pwm=' pwm(1),pwm(2),pwm(3),pwm(4)
70 next i
71 pwm(1,0)
72 pwm(2,0)
73 pwm(3,0)
74 pwm(4,0)
80 println 'GPIO 1:4 Test'
81 for i = 1 to 4
82 for j = 0 to 2
84 gpio(i,(j % 2))
85 sleep(1)
86 next j
87 next i
88 println 'gpio(1)=' gpio(1)
89 println 'gpio(2)=' gpio(2)
90 println 'Press the Push button!'
91 if hw_event(1)=0 then goto 91
92 tic(1)
93 println 'Push button pressed!'
94 if hw_event(2)=0 then goto 94
95 println 'duration =' toc(1)
96 println 'Push button de-pressed!'
98 println 'end of test 3'
99 end


4th example - arrays

10 println 'start of test 4'
11 for i = 1 to 100
12 n = floor(10 * uniform) + 2
12 dim b@(n)
21 for j = 1 to n
22 b@(j) = ran
30 println 'b@(' j ')=' b@(j)
31 next j
40 next i
50 println 'end of test 4'
60 end;",


5th example - input values but wait only for 2sec

10 println 'start of test 6'
11 dim a@(5)
11 for i = 1 to 5
12 print '?'
13 input a@(i),2000
14 next i
15 println 'end of input'
20 for i = 1 to 5
21 println 'a@(' i ') = ' a@(i)
22 next i
50 println 'end of test 6'
60 end




UBASIC PLUS HARDWARE IMPLEMENTATION EXAMPLES


STM32F030R8-Nucleo64

- Arm Cortex M0 64kb flash, 8kb ram, 48MHz frequency
- GPIO: Pins C0,C1,C2,C3
- PWM:  Timer 3 Channels 1 (A6), 2 (A7), 3 (B0) and 4 (B1)
- Hardware Events:
    Push Button 1 (C13)
        Two events: button pressed - hw_event(1), and
                    button depressed - hw_event(2)
- sleep, tic/toc:
    through SysTick_Handler() that is by default configured for
    1ms interrupts
- firmware footprint with all features enabled and 6 test scripts (bytes):
    29628 flash, 552 data, 2776 bss


