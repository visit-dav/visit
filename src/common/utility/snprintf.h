#ifndef SNPRINTF_H
#define SNPRINTF_H
#include <stdio.h>

/* Define the appropriate macro. */
#if defined(_WIN32)
#define SNPRINTF _snprintf
#else
#define SNPRINTF snprintf
#endif

#endif
