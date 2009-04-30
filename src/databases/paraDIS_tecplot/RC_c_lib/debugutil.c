/*
** $RCSfile: debugutil.c,v $
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
** $Id: debugutil.c,v 1.8 2008/09/28 01:02:23 rcook Exp $
**
*/
/*
**
**  Abstract:
**
**  Author:
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "debugutil.h"
#include <time.h>
#include <sys/time.h>

static  int    iVerbose = 0;
static    int    iCheck = 0;

static FILE *gDebugFile = NULL; 
#define INITDEBUGFILE if (!gDebugFile) gDebugFile = stderr

char *datestring(void) {
  static char timebuf[32]; 
  time_t now;

  now = time(NULL);
  strftime(timebuf, 31, "%d/%m/%y %H:%M:%S", (struct tm *)localtime(&now));
  return timebuf;
}

void dbfprintf(FILE *stream, int level, const char *fmt, ...){
  if(iVerbose >= level){
    vafprintf(stream, fmt);
    /*    va_list ap;
      va_start(ap, fmt);
      vfprintf(stream,fmt,ap);
      va_end(ap);
    */
  }    
  return;

}

void dbprintf(int level, const char *fmt, ...){
  if(iVerbose >= level){
    INITDEBUGFILE; 
    vafprintf(gDebugFile, fmt);
    /*    va_list ap;
      va_start(ap, fmt);
      vfprintf(gDebugFile,fmt,ap);
      va_end(ap);
    */
  }    
  return;
}

static int check_verbose(void)
{
    if (!iCheck) {
        if (getenv("DEBUG_VERBOSE")) {
            iVerbose = atoi(getenv("DEBUG_VERBOSE"));
        }
        iCheck = 1;
    }
    return(iVerbose);
}

int dbg_isverbose(void)
{
    return(check_verbose());
}


void dbg_setverbose(int verbose)
{
    iVerbose = verbose;
    iCheck = 1;
}

void dbg_stderr(char *fmt, ...) 
{
  /*va_list ap;*/

    if (check_verbose()) {
      vafprintf(stderr, fmt);
      /*        va_start(ap, fmt);
            vfprintf(stderr,fmt,ap);
            va_end(ap);
      */
    }
}

void dbg_maskstderr(int mask, char *fmt, ...) 
{
  /*    va_list ap;*/

    if (check_verbose() & mask) {
      vafprintf(stderr, fmt);
      /*    va_start(ap, fmt);
        vfprintf(stderr,fmt,ap);
        va_end(ap);*/
    }
}

/* return 0 on failure, 1 on success */
int dbg_setfile(const char *dbgfile) {
  INITDEBUGFILE; 
  if (!dbgfile) return 0; 
  FILE *tmpf = fopen(dbgfile, "w"); 
  if (!tmpf) return 0; 
  gDebugFile = tmpf; 
  return 1;
}
  
