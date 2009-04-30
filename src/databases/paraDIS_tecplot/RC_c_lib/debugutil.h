/*
** $RCSfile: debugutil.h,v $
** $Name:  $
**
** ASCI Visualization Project 
**
** Lawrence Livermore National Laboratory
** Information Management and Graphics Group
** P.O. Box 808, Mail Stop L-561
** Livermore, CA 94551-0808
**
** For information about this project see:
**     http://www.llnl.gov/sccd/lc/img/
**
**      or contact: asciviz@llnl.gov
**
** For copyright and disclaimer information see:
**      $(ASCIVIS_ROOT)/copyright_notice_1.txt
**
**     or man llnl_copyright
**
** $Id: debugutil.h,v 1.9 2008/09/28 01:02:23 rcook Exp $
**
*/
/*
**
**  Abstract:  Calls to support debug print strings
**
**  Author:
**
*/

#ifndef _debugutil_h
#define _debugutil_h

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

  /*useful macro -- printf the fmt string onto s, modulo current function varargs */
#define myvasprintf(s, fmt) \
  va_list ap;\
  va_start(ap, fmt);\
  vsprintf(s,fmt,ap);\
  va_end(ap)

#define vafprintf(s, fmt) \
  va_list ap;\
  va_start(ap, fmt);\
  vfprintf(s,fmt,ap);\
  va_end(ap)

void dbfprintf(FILE *stream, int level, const char *fmt, ...);
void dbprintf(int level, const char *fmt, ...);
char *datestring(void);

int dbg_setfile(const char *dbgfile); 
int  dbg_isverbose(void);
void dbg_setverbose(int verbose);

void dbg_stderr(char *fmt, ...);
void dbg_maskstderr(int mask,char *fmt, ...);
  

#define DBGPRINTF    dbg_stderr
#define DBGMPRINTF    dbg_maskstderr
#define DBGVERBOSE    dbg_isverbose


#define varprint(debug, format, __var) if (debug) {printf(#__var" is '"format"'\n", __var); fflush(stdout);}
#define    badvalue(format, __var, astring, errval) if (1) {printf("Bad value of "#__var" while reading file "astring": '"#format"'\n", __var);  fflush(stdout); return errval;}
#define vectprint(debug, array, length) if (debug) {\
      int __i=0; \
      printf("printing array "#array":\n");  \
      for (__i=0; __i<length; __i++) \
         printf("%g\t", (float)(array[__i])); \
      printf("\n");\
      fflush(stdout); \
   }

#ifdef __cplusplus
}
#endif
#endif


