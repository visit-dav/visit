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

#ifndef NRRD_DEFINES_HAS_BEEN_INCLUDED
#define NRRD_DEFINES_HAS_BEEN_INCLUDED

#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

/* feel free to set these to higher values and recompile */
#define NRRD_DIM_MAX 16            /* Max array dimension (nrrd->dim) */
#define NRRD_SPACE_DIM_MAX 8       /* Max dimension of "space" around array 
                                      (nrrd->spaceDim) */

#define NRRD_EXT_NRRD   ".nrrd"
#define NRRD_EXT_NHDR   ".nhdr"
#define NRRD_EXT_PGM    ".pgm"
#define NRRD_EXT_PPM    ".ppm"
#define NRRD_EXT_PNG    ".png"
#define NRRD_EXT_VTK    ".vtk"
#define NRRD_EXT_TEXT   ".txt"
#define NRRD_EXT_EPS    ".eps"

#define NRRD_KERNEL_PARMS_NUM 8    /* max # arguments to a kernel-
                                      this is weird: it isn't the max
                                      of any of the NrrdKernels
                                      defined by the nrrd library
                                      (that is more like 3), but is
                                      the max number of parms of any
                                      NrrdKernel used by anyone using
                                      Teem, such as in gage.
                                      Enforcing one global max
                                      simplifies implementation. */

/* 
** For the 64-bit integer types (not standard except in C99), we try
** to use the names for the _MIN and _MAX values which are used in C99
** (as well as gcc) such as LLONG_MAX.
** 
** If these aren't defined, we try the ones used on SGI such as
** LONGLONG_MAX.
**
** If these aren't defined either, we go wild and define something
** ourselves (which just happen to be the values defined in C99), with
** total disregard to what the architecture and compiler actually
** support.  These values are tested, however, by nrrdSanity().
*/

#ifdef LLONG_MAX
#  define NRRD_LLONG_MAX LLONG_MAX
#else
#  ifdef LONGLONG_MAX
#    define NRRD_LLONG_MAX LONGLONG_MAX
#  else
#    define NRRD_LLONG_MAX AIR_LLONG(9223372036854775807)
#  endif
#endif

#ifdef LLONG_MIN
#  define NRRD_LLONG_MIN LLONG_MIN
#else
#  ifdef LONGLONG_MIN
#    define NRRD_LLONG_MIN LONGLONG_MIN
#  else
#    define NRRD_LLONG_MIN (-NRRD_LLONG_MAX-AIR_LLONG(1))
#  endif
#endif

#ifdef ULLONG_MAX
#  define NRRD_ULLONG_MAX ULLONG_MAX
#else
#  ifdef ULONGLONG_MAX
#    define NRRD_ULLONG_MAX ULONGLONG_MAX
#  else
#    define NRRD_ULLONG_MAX AIR_ULLONG(18446744073709551615)
#  endif
#endif

/*
** Chances are, you shouldn't mess with these
*/

#define NRRD_COMMENT_CHAR '#'
#define NRRD_FILENAME_INCR 32
#define NRRD_COMMENT_INCR 16
#define NRRD_KEYVALUE_INCR 32
#define NRRD_LIST_FLAG "LIST"
#define NRRD_PNM_COMMENT "# NRRD>"    /* this is designed to be robust against
                                         the mungling that xv does, but no
                                         promises for any other image
                                         programs */

#define NRRD_PNG_FIELD_KEY "NRRD"     /* this is the key used for getting nrrd
                                         fields into/out of png comments */
#define NRRD_PNG_COMMENT_KEY "NRRD#"  /* this is the key used for getting nrrd
                                         comments into/out of png comments */

#define NRRD_UNKNOWN  "???"           /* how to represent something unknown in
                                         a field of the nrrd header, when it
                                         being unknown is not an error */
#define NRRD_NONE "none"              /* like NRRD_UNKNOWN, but with an air
                                         of certainty */

#ifdef __cplusplus
}
#endif

#endif /* NRRD_DEFINES_HAS_BEEN_INCLUDED */
