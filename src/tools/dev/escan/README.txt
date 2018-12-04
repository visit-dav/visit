Here is how to build the exception scanner

flex rules
gcc -o escan -I. exception_scanner.c lex.yy.c -lfl
