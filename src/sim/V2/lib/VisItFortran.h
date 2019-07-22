// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_FORTRAN_H
#define VISIT_FORTRAN_H
#include <stdlib.h>
#include <string.h>

/* F77_ID - attempt a uniform naming of FORTRAN 77 functions which
 *        - gets around loader naming conventions
 *        -
 *        - F77_ID(foo_, foo, FOO)(x, y, z)
 */
#ifndef F77_ID
#ifdef _INT_MAX_46              /* UNICOS */
#define F77_ID(x_, x, X)  X
#endif
#ifdef _H_LIMITS                /* AIX */
#define F77_ID(x_, x, X)  x
#endif
#ifdef _INCLUDE_HPUX_SOURCE     /* HPUX */
#define F77_ID(x_, x, X)  x
#endif
#ifdef VISIT_BLUE_GENE_Q        /* BlueGene/Q */
#define F77_ID(x_, x, X)  x
#endif
#ifndef F77_ID
#define F77_ID(x_, x, X)  x_
#endif
#endif

/* Support strings from Fortran */
#define FORTRAN           int
#define VISIT_F77STRING   char*

#define COPY_FORTRAN_STRING(dest, src, srclen) \
    dest = visit_fstring_copy_to_cstring(src,((srclen==NULL)?0:(*srclen)));

#define ALLOC(T,N) (T*)malloc(sizeof(T) * (N))
#define FREE(ptr)  if(ptr != NULL) free(ptr);

extern const char *VISIT_F77NULLSTRING;
char *visit_fstring_copy_to_cstring(const char *src, int len);
void visit_cstring_to_fstring(const char *src, char *dest, int len);

#endif
