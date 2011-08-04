#include "VisItFortran.h"

const char *VISIT_F77NULLSTRING = "NULLSTRING";

char *visit_fstring_copy_to_cstring(const char *src, int len)
{
    char *newstr = NULL;

    int clen = strlen(VISIT_F77NULLSTRING);
    if(clen > len)
        clen = len;

    if(src == NULL || strncmp(src, VISIT_F77NULLSTRING, clen) == 0)
        newstr = NULL;
    else
    {
        char *cptr = NULL;
        newstr = (char *)malloc(len+1);
        memcpy(newstr, src, len);
        newstr[len] = '\0';

        /* Fortran can add spaces to the end of long strings so remove the spaces */
        cptr = newstr + len - 1;
        while(cptr >= newstr && *cptr == ' ')
            *cptr-- = '\0';
    }

    return newstr;
}

void
visit_cstring_to_fstring(const char *src, char *dest, int len)
{
    int i;
    char *c = dest;
    for(i = 0; i < len; ++i)
    {
        if(*src != '\0')
            *dest++ = *src++;
        else
            *dest = ' ';
    }
}
