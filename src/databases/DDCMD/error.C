#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "error.h"
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 15:49:45 EDT 2008
//    va_arg doesn't support enums; if one is passed in, it's promoted
//    to an int.  So here, we retrieve an int then cast it to the enum.
//
static char message[1024];
void error_action(const char *start, ...)
{
    va_list ap;
    enum ACTION action;
    int line; 
    char *string;
    strncpy(message, start, 1024);
    va_start(ap, start);
    string = va_arg(ap, char *);
    while (string != NULL)
    {
        strncat(message, " ", 1024);
        strncat(message, string, 1024);
        string = va_arg(ap, char *);
    }
    printf("\nMessage:%s\n\n", message);
    string = va_arg(ap, char *);
    fprintf(stderr, "Error in routine %s\n", string);
    string = va_arg(ap, char *);
    fprintf(stderr, "in file %s ", string);
    line = va_arg(ap, int);
    fprintf(stderr, "at line %d\n", line);
        int int_action;
    int_action = va_arg(ap, int);
        action = (enum ACTION)(int_action);
    switch (action)
    {
    case CONTINUE:
        fprintf(stderr, "Program will CONTINUE.\n");
        va_end(ap);
        return;
    case ABORT:
        fprintf(stderr, "Program will ABORT.\n");
        va_end(ap);
        exit(1);
    default:
        fprintf(stderr, "Program will ABORT because of unknown action.\n");
        va_end(ap);
        exit(1);
    }
}
