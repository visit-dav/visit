/*
** $RCSfile: debugutil-qt.h,v $
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
** $Id: debugutil-qt.h,v 1.1 2009/04/02 00:39:23 rcook Exp $
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

#include <stdio.h>

  // Note: To use this, must be compiled with Qt and have Qt += network in project file
#include "QTcpSocket"

  /*useful macro -- printf the fmt string onto s, modulo current function varargs */
#define myvasprintf(s, fmt) \
  va_list ap;\
  va_start(ap, fmt);\
  vsprintf(s,fmt,ap);\
  va_end(ap)

#define vafprintf(lvl, s, fmt)                   \
  va_list ap;\
  va_start(ap, fmt);\
  vfprintf(s,fmt,ap);\
  va_end(ap)

#define dbfprintf(stream,level,msg)            \
    real_dbfprintf(stream, level, __FILE__, __LINE__, msg)

void real_dbfprintf(FILE *stream, int level, QString filename, int lineno, QString msg);

#define dbprintf(level,msg) dbfprintf(stderr,level,msg)

char *datestring(void);

int  dbg_isverbose(void); 
void dbg_setverbose(int verbose); // default is 0

// prepend the date to each message, down to the second
void dbg_useDateString(bool use=true); 
// prepend the given text to every message
void dbg_prependString(QString text);
// prepend the filename and line number to each message
void dbg_useFilesAndLines(bool use=true); 

// every message at or below the given level will go over the file
// returns true if file is created successfully, false if not
bool dbg_setlogfile(QString name, int level=0); 
// every message at or below the given level will go over the socket
void dbg_setSocket(QTcpSocket *socket, int level=0); 


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

#endif


