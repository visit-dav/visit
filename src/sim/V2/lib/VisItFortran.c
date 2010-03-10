#include "VisItFortran.h"

const char *VISIT_F77NULLSTRING = "NULLSTRING";

static const char *fcdtocp_ptr = NULL;
static char *fcdtocp_data = NULL;

char *visit_fstring_copy_string(const char *src, int len)
{
    char *cptr = NULL, *newstr = (char *)malloc(len+1);
    memcpy(newstr, src, len);
    newstr[len] = '\0';

    /* Fortran can add spaces to the end of long strings so remove the spaces */
    cptr = newstr + len - 1;
    while(cptr >= newstr && *cptr == ' ')
        *cptr-- = '\0';

    return newstr;
}

char *visit_fstring_to_cstring(const char *ptr, int len)
{
    if((ptr != fcdtocp_ptr) ||
       (fcdtocp_data != NULL && strncmp(fcdtocp_data, ptr, len) != 0))
    {
        fcdtocp_ptr = ptr;
        FREE(fcdtocp_data);
        fcdtocp_data = visit_fstring_copy_string(ptr, len);
    }

    return fcdtocp_data;
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
