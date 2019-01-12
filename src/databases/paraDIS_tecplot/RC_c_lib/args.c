/* $Id: args.c,v 1.19 2008/11/03 23:48:35 rcook Exp $ */
#include "args.h"
#include "debugutil.h"
#include <stdio.h>
#include <string.h> 
static int gNumArgs = 0;
static argt *gArgs = NULL;
static int gIgnoreBadArgs = 0; 
static int gArgDebug = 0; 

/*------------------------------------------------------------------*/
void debug_args(int iYesNo ){
  gArgDebug = iYesNo; 
}
/*------------------------------------------------------------------*/
void print_args(int argc, char *argv[] ){
  int i = 0 ;
  printf("Program called as: "); 
  while (i<argc) {
    printf("%s ", argv[i]); 
    ++i; 
  }
  printf("\n");
  return ;
}

/*------------------------------------------------------------------*/
void arg_ignore_bad_args(int iYesNo) { gIgnoreBadArgs = iYesNo; }

/*------------------------------------------------------------------*/
void arg_expect_args(argt args[], int numargs) {
  gNumArgs = numargs;
  if (gArgs) free(gArgs);
  gArgs = (argt*)malloc(sizeof(argt)* gNumArgs);
  memcpy(gArgs, args, sizeof(argt)*gNumArgs);
  return;
}

/*------------------------------------------------------------------*/
argt *arg_find_entry(char *flag){
  int argnum = 0;
  while (argnum < gNumArgs){
    if (!strcmp(gArgs[argnum]._flag, flag)) 
      return &gArgs[argnum];
    ++argnum;
  }
  
  return NULL;
}

/*------------------------------------------------------------------*/
void print_arg_entry(argt *iEntry) {
  fprintf(stderr, "arg entry: \n"); 
  fprintf(stderr, " iEntry->_type = %d\n", iEntry->_type); 
  fprintf(stderr, " iEntry->_flag = %s\n", iEntry->_flag); 
  fprintf(stderr, " iEntry->_numvals = %d\n", iEntry->_numvals); 
  switch(iEntry->_type) {
  case   BOOL_TYPE:  /* must be type long**/
  case   INT_TYPE:   /* must be type int* */
    if (!iEntry->_values) fprintf(stderr, "(empty)\n"); 
    vectprint(1, ((int*)iEntry->_values), iEntry->_numvals); 
    break;
  case   LONG_TYPE:   /* must be type long* */
  case   LONG_ARRAY_TYPE:   /* must be type long* */
    if (!iEntry->_values) fprintf(stderr, "(empty)\n"); 
    vectprint(1, ((long*)iEntry->_values), iEntry->_numvals); 
    break;
  case   FLOAT_TYPE:   /* must be type long* */
  case   FLOAT_ARRAY_TYPE:   /* must be type long* */
    if (!iEntry->_values) fprintf(stderr, "(empty)\n"); 
    vectprint(1, ((float*)iEntry->_values), iEntry->_numvals); 
    break;
  case   DOUBLE_TYPE: /* must be type double* */
  case   DOUBLE_ARRAY_TYPE: /* must be type double* */
    if (!iEntry->_values) fprintf(stderr, "(empty)\n"); 
    vectprint(1, ((long*)iEntry->_values), iEntry->_numvals); 
    break;
  case   STRING_TYPE:  /* must be type (char *) */
    fprintf(stderr, " iEntry->_values = %s\n", (char*)(iEntry->_values)); 
    break;
  }
  fprintf(stderr, "\n"); 
  return; 
}

/*! 
   "consume" the requested arg in argv by 
   shifting all later args one to the left 
*/ 
void ConsumeArg(int *argc, char *argv[], int position) {
  /*  fprintf(stderr, "Before ConsumeArg(%d, argv, %d): ", argc, position); 
      printargv(argc, argv); */
  if (position == 0 && *argc == 1) {
    argv[0] = NULL; 
  } else {
    while (position < *argc) {
      argv[position] = argv[position+1];
      ++position; 
    }
  }
  (*argc)--; 
  /* fprintf(stderr, "After ConsumeArg: "); 
     printargv(argc, argv); */
  return; 
}
  
/*------------------------------------------------------------------*/
/*!
  arg_parse_args
   return 0 on error, 1 if everything's OK.  
   parse argv, looking for flags set using arg_expect_args, starting at argv[1].
   Continue until no more valid args are found.  
   Removes all parsed args and options from argv and decrements numargs approprately.  Bad args are ignored, unless gIgnoreBadArgs is false; 
*/
int arg_parse_args(int *argc, char *argv[]){ 
  int argnum=1;
  argt*entry=NULL;
  if (!gArgs) {
    dbprintf(1, "Error parsing args:  arg_parse_args called without calling  arg_expect_args\n"); 
    return 0; 
  }
  
  while (argnum < *argc) {
    char *theArg = argv[argnum];     
    if (theArg[0] != '-') {
      argnum++;
      continue;  
    }
    entry = arg_find_entry(theArg);
    if (!entry){
      if (gIgnoreBadArgs) {
        argnum++;
        continue;   
      }
      else {
        fprintf(stderr, "Error parsing args:  bad arg: \"%s\"\n", theArg); 
        return 0;
      }
    } /* end bad flag code */ 
    else {
      ConsumeArg(argc, argv, argnum); 
      if (entry->_type != BOOL_TYPE && argnum == *argc) {
        fprintf(stderr, "ERROR -- arg_parse_args -- missing argument to %s\n", entry->_flag); 
        return 0; 
      }
      if (gArgDebug) fprintf(stderr, "entry type %d\n", entry->_type); 
      switch (entry->_type){
      case BOOL_TYPE:    
        *((long*)entry->_values)=1;
        break;
      case INT_TYPE:
        {
          int *values = (int*)entry->_values; 
          int valnum=entry->_numvals; while (valnum--) {
            if (argnum == *argc) {
              fprintf(stderr, "ERROR -- arg_parse_args -- missing argument to %s\n", entry->_flag); 
              return 0; 
            }
            *values++ = atoi(argv[argnum]);
            ConsumeArg(argc, argv, argnum); 
          }    
        }    
        break;        
      case LONG_TYPE:    
      case LONG_ARRAY_TYPE:
        {
          long *values = (long*)entry->_values; 
          int valnum=entry->_numvals; while (valnum--) {
            if (argnum == *argc) {
              fprintf(stderr, "ERROR -- arg_parse_args -- missing argument to %s\n", entry->_flag); 
              return 0; 
            }
            *values++ = strtol(argv[argnum], NULL, 10);
            ConsumeArg(argc, argv, argnum); 
          }    
        }    
        break;
      case FLOAT_TYPE:    
      case FLOAT_ARRAY_TYPE:
        {
          float *values = (float*)entry->_values; 
          int valnum=entry->_numvals; while (valnum--) {
            if (argnum == *argc) {
              fprintf(stderr, "ERROR -- arg_parse_args -- missing argument to %s\n", entry->_flag); 
              return 0; 
            }
            *values++ = strtod(argv[argnum], NULL);
            ConsumeArg(argc, argv, argnum); 
          }    
        }    
        break;
      case DOUBLE_TYPE:
      case DOUBLE_ARRAY_TYPE:
        {
          double *values = (double*)entry->_values; 
          int valnum=entry->_numvals; while (--valnum) {
            if (argnum == *argc) {
              fprintf(stderr, "ERROR -- arg_parse_args -- missing argument to %s\n", entry->_flag); 
              return 0; 
            }
            *values++ = strtod(argv[argnum], NULL);
            ConsumeArg(argc, argv, argnum); 
          }    
        }    
        break;
      case STRING_TYPE:
        strcpy((char*)entry->_values, argv[argnum]);
        ConsumeArg(argc, argv, argnum); 
        break;
      default:
        fprintf(stderr, "ERROR -- args.c: Can't identify my own type (%d)!\n", 
                (int)entry->_type);
        return 0;
        break;
      }
      if (gArgDebug) print_arg_entry(entry);  
    }
  }
  return 1; 
}

