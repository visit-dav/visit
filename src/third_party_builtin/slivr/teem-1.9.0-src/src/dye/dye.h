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

#ifndef DYE_HAS_BEEN_INCLUDED
#define DYE_HAS_BEEN_INCLUDED

#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(dye_EXPORTS) || defined(teem_EXPORTS)
#    define DYE_EXPORT extern __declspec(dllexport)
#  else
#    define DYE_EXPORT extern __declspec(dllimport)
#  endif
#define DYE_EXPORT2 DYE_EXPORT2
#else /* TEEM_STATIC || UNIX */
# if __GNUC__ >= 4 && (defined(dye_EXPORTS) || defined(teem_EXPORTS))
#  define DYE_EXPORT __attribute__ ((visibility("default")))
# else
#  define DYE_EXPORT extern
# endif
#define DYE_EXPORT2 extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DYE dyeBiffKey

enum {
  dyeSpaceUnknown,        /* 0: nobody knows */
  dyeSpaceHSV,            /* 1: single hexcone */
  dyeSpaceHSL,            /* 2: double hexcone */
  dyeSpaceRGB,            /* 3: obscure, deprecated */
  dyeSpaceXYZ,            /* 4: perceptual primaries */
  dyeSpaceLAB,            /* 5: 1976 CIE (L*a*b*) (based on Munsell) */
  dyeSpaceLUV,            /* 6: 1976 CIE (L*u*v*) */
  dyeSpaceLast
};
#define DYE_MAX_SPACE 6

#define DYE_VALID_SPACE(spc) \
  (AIR_IN_OP(dyeSpaceUnknown, (spc), dyeSpaceLast))

typedef struct {
  float val[2][3];        /* room for two colors: two triples of floats */
  float xWhite, yWhite;   /* chromaticity for white point */
  signed char spc[2],     /* the spaces the two colors belong to */
    ii;                   /* which (0 or 1) of the two values is current */
} dyeColor;

/* methodsDye.c */
DYE_EXPORT2 const char *dyeBiffKey;
DYE_EXPORT2 char dyeSpaceToStr[][AIR_STRLEN_SMALL];
DYE_EXPORT int dyeStrToSpace(char *str);
DYE_EXPORT dyeColor *dyeColorInit(dyeColor *col);
DYE_EXPORT dyeColor *dyeColorSet(dyeColor *col, int space, 
                                 float v0, float v1, float v2);
DYE_EXPORT int dyeColorGet(float *v0P, float *v1P, float *v2P, dyeColor *col);
DYE_EXPORT int dyeColorGetAs(float *v0P, float *v1P, float *v2P, 
                             dyeColor *col, int space);
DYE_EXPORT dyeColor *dyeColorNew();
DYE_EXPORT dyeColor *dyeColorCopy(dyeColor *c1, dyeColor *c0);
DYE_EXPORT dyeColor *dyeColorNix(dyeColor *col);
DYE_EXPORT int dyeColorParse(dyeColor *col, char *str);
DYE_EXPORT char *dyeColorSprintf(char *str, dyeColor *col);

/* convertDye.c */
typedef void (*dyeConverter)(float*, float*, float*, float, float, float);
DYE_EXPORT void dyeRGBtoHSV(float *H, float *S, float *V,
                            float  R, float  G, float  B);
DYE_EXPORT void dyeHSVtoRGB(float *R, float *G, float *B,
                            float  H, float  S, float  V);
DYE_EXPORT void dyeRGBtoHSL(float *H, float *S, float *L,
                            float  R, float  G, float  B);
DYE_EXPORT void dyeHSLtoRGB(float *R, float *G, float *B,
                            float  H, float  S, float  L);
DYE_EXPORT void dyeRGBtoXYZ(float *X, float *Y, float *Z,
                            float  R, float  G, float  B);
DYE_EXPORT void dyeXYZtoRGB(float *R, float *G, float *B,
                            float  X, float  Y, float  Z);
DYE_EXPORT void dyeXYZtoLAB(float *L, float *A, float *B,
                            float  X, float  Y, float  Z);
DYE_EXPORT void dyeXYZtoLUV(float *L, float *U, float *V,
                            float  X, float  Y, float  Z);
DYE_EXPORT void dyeLABtoXYZ(float *X, float *Y, float *Z,
                            float  L, float  A, float  B);
DYE_EXPORT void dyeLUVtoXYZ(float *X, float *Y, float *Z,
                            float  L, float  U, float  V);
DYE_EXPORT2 dyeConverter dyeSimpleConvert[DYE_MAX_SPACE+1][DYE_MAX_SPACE+1];
DYE_EXPORT int dyeConvert(dyeColor *col, int space);

#ifdef __cplusplus
}
#endif

#endif /* DYE_HAS_BEEN_INCLUDED */
