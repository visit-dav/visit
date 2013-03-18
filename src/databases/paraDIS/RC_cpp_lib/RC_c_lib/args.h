/* utilities for args parsing */
#ifndef args_h
#define args_h
#ifdef __cplusplus
extern "C" {
#endif
#ifdef _NEVER
} // to make emacs happy
#endif 
#include <stdlib.h>

/* 
   example how to use (typical usage): 
   
   long useX=0, verbosity=0; 
   double cruftLevel=0.5;
   char inputfile[1024] = "defaultvalue"; 
   double location[3] = {0}; 
   char **unparsed_args = NULL; 

   argt args[5] = {
   {BOOL_TYPE, "-useX", 1, &useX}, 
   {LONG_TYPE, "-verbosity", 1, &verbosity}, 
   {DOUBLE_TYPE, "-cruftiness", 1, &cruftLevel}, 
   {FLOAT_TYPE, "-loc", 3, location}, <------- note no ampersand (reference)
   {STRING_TYPE, "-in", 1024, inputfile} <------- note no ampersand (reference)
   }
   arg_expect_args(args, 5);
   arg_ignore_bad_args(1); <---- you probably want this on
   arg_parse_args(-1, argv, 1, &unparsed_args);  <-----typical usage: parse all args starting from argv[1]
   ---  OR: ----
   arg_parse_args(-1, argv, 1, NULL);  <-----if you don't care about unparsed args
*/
   
typedef long args_bool_t; 
typedef int args_int_t; 
typedef long args_long_t; 
typedef double args_double_t;
typedef char *args_string_t; 

typedef enum {
  BOOL_TYPE,   /* must be type long**/
  INT_TYPE, /*must be type int*  */ 
  LONG_TYPE,   /* must be type long* */
  LONG_ARRAY_TYPE,   /* DEPRECATED -- same as LONG_TYPE must be type long* */
  FLOAT_TYPE, /* must be type float* */
  FLOAT_ARRAY_TYPE,   /* DEPRECATED -- same as FLOAT_TYPE must be type float* */
  DOUBLE_TYPE, /* must be type double* */
  DOUBLE_ARRAY_TYPE,   /* DEPRECATED -- same as DOUBLE_TYPE must be type double* */
  STRING_TYPE  /* must be type (char *) */
} argtype;

typedef struct argt {
  argtype _type;
  char _flag[64];
  int _numvals; /* for arrays of values */
  void *_values; /* see enum for actual type */ 
} argt;


void debug_args(int iYesNo ); /* debugging unexpected behaviors parse_args */
void print_args(int argc, char *argv[] );
void arg_expect_args(argt args[], int numargs);
void arg_ignore_bad_args(int iYesNo);


/*!
  arg_parse_args
   return 0 on error, 1 if everything's OK.  
   parse argv, looking for flags set using arg_expect_args, starting at argv[1].
   Continue until no more valid args are found.  
   Removes all parsed args and options from argv and decrements numargs approprately.  Bad args are ignored, unless gIgnoreBadArgs is false; 
*/
int arg_parse_args(int *argc, char *argv[]);
#ifdef __cplusplus
}
#endif

#endif

