/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "nrrd.h"
#include "privateNrrd.h"

/* 
** making these typedefs here allows us to use one token for both
** constructing function names, and for specifying argument types
*/
typedef signed char CH;
typedef unsigned char UC;
typedef signed short SH;
typedef unsigned short US;
/* Microsoft apparently uses 'IN' as a keyword, so we changed 'IN' to 'JN'. */
typedef signed int JN;
typedef unsigned int UI;
typedef airLLong LL;
/* ui64 to double conversion is not implemented, sorry */
#if _MSC_VER < 1300
typedef airLLong UL;
#else
typedef airULLong UL;
#endif
typedef float FL;
typedef double DB;
typedef size_t IT;
/* typedef long double LD; */

/*
** I don't think that I can get out of defining this macro twice,
** because of the rules of C preprocessor macro expansion.  If
** you can figure out a way to not use two identical macros, then
** email me (gk@bwh.harvard.edu) and I'll send you money for dinner.
**
** >>> MAP1 and MAP2 need to be identical <<<
*/

#define MAP1(F, A) \
F(A, CH) \
F(A, UC) \
F(A, SH) \
F(A, US) \
F(A, JN) \
F(A, UI) \
F(A, LL) \
F(A, UL) \
F(A, FL) \
F(A, DB)
/* F(A, LD) */

#define MAP2(F, A) \
F(A, CH) \
F(A, UC) \
F(A, SH) \
F(A, US) \
F(A, JN) \
F(A, UI) \
F(A, LL) \
F(A, UL) \
F(A, FL) \
F(A, DB)
/* F(A, LD) */

/* 
** _nrrdConv<Ta><Tb>()
** 
** given two arrays, a and b, of different types (Ta and Tb) but equal
** size N, _nrrdConvTaTb(a, b, N) will copy all the values from b into
** a, thereby effecting the same type-conversion as one gets with a
** cast.  See K+R Appendix A6 (pg. 197) for the details of what that
** entails.  There are plenty of situations where the results are
** "undefined" (assigning -23 to an unsigned char); the point here is
** simply to make available on arrays all the same behavior you can
** get from scalars.
*/
#define CONV_DEF(TA, TB) \
void _nrrdConv##TA##TB(TA *a, const TB *b, IT N) { while (N--) a[N]=(TA)b[N]; }

/* 
** the individual converter's appearance in the array initialization,
** using the cast to the "CF" typedef defined below
*/
#define CONV_LIST(TA, TB) (CF)_nrrdConv##TA##TB,

/* 
** the brace-delimited list of all converters _to_ type TYPE 
*/
#define CONVTO_LIST(_dummy_, TYPE) {NULL, MAP2(CONV_LIST, TYPE) NULL},



/*
** This is where the actual emitted code starts ...
*/



/*
** This typedef makes the definition of _nrrdConv[][] shorter
*/
typedef void (*CF)(void *, const void *, IT);

/* 
** Define all 100 of the individual converters. 
*/
MAP1(MAP2, CONV_DEF)

/* 
** Initialize the whole converter array.
** 
** This generates one incredibly long line of text, which hopefully will not
** break a stupid compiler with assumptions about line-length...
*/
CF _nrrdConv[NRRD_TYPE_MAX+1][NRRD_TYPE_MAX+1] = {
{NULL}, 
MAP1(CONVTO_LIST, _dummy_)
{NULL}
};
