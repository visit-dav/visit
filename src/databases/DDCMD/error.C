#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "error.h"

#include <DebugStream.h>
#include <InvalidFilesException.h>

//  Modifications:
//    Jeremy Meredith, Thu Aug  7 15:49:45 EDT 2008
//    va_arg doesn't support enums; if one is passed in, it's promoted
//    to an int.  So here, we retrieve an int then cast it to the enum.
//
//    Brad Whitlock, Mon Oct  8 16:17:30 PDT 2012
//    I modified this code to print to the debug logs and to throw an 
//    exception instead of exiting.
//
static char message[2048];
void error_action(const char *start, ...)
{
    va_list ap;
    enum ACTION action;
    int line; 
    char *str;
    strncpy(message, start, 1024);
    va_start(ap, start);
    str = va_arg(ap, char *);
    while (str != NULL)
    {
        strncat(message, " ", 1024);
        strncat(message, str, 1024);
        str = va_arg(ap, char *);
    }
    debug5 << "Message:" << message << endl << endl;
    str = va_arg(ap, char *);
    debug5 << "Error in routine " << str;
    str = va_arg(ap, char *);
    debug5 << " in file " << str;
    line = va_arg(ap, int);
    debug5 << " at line " << line << endl;
        int int_action;
    int_action = va_arg(ap, int);
        action = (enum ACTION)(int_action);
    va_end(ap);

    switch (action)
    {
    case CONTINUE:
        debug5 << "Program will CONTINUE.\n";
        return;
    default:
        EXCEPTION1(InvalidFilesException, str);
    }
}
