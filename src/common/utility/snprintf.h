// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
