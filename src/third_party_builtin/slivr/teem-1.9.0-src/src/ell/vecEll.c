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


#include "ell.h"

#define PERP \
  idx = 0; \
  if (b[0]*b[0] < b[1]*b[1]) \
    idx = 1; \
  if (b[idx]*b[idx] < b[2]*b[2]) \
    idx = 2; \
  switch (idx) { \
  case 0: \
    ELL_3V_SET(a, b[1] - b[2], -b[0], b[0]); \
    break; \
  case 1: \
    ELL_3V_SET(a, -b[1], b[0] - b[2], b[1]); \
    break; \
  case 2: \
    ELL_3V_SET(a, -b[2], b[2], b[0] - b[1]); \
    break; \
  }

/*
******** ell_3v_perp_f()
**
** Given a 3-vector, produce one which is perpendicular.
** Output length won't be same as input length, but it will always
** be non-zero, if input length is non-zero.   This does NOT try to
** produce a unit-length vector, regardless of the length of the input.
*/
void
ell_3v_perp_f(float a[3], float b[3]) {
  int idx;
  PERP;
}

/*
******** ell_3v_perp_d()
**
** same as above, but for doubles
*/
void
ell_3v_perp_d(double a[3], double b[3]) {
  int idx;
  PERP;
}

void
ell_3mv_mul_f(float v2[3], float m[9], float v1[3]) {
  ELL_3MV_MUL(v2, m, v1);
}

void
ell_3mv_mul_d(double v2[3], double m[9], double v1[3]) {
  ELL_3MV_MUL(v2, m, v1);
}

void
ell_4mv_mul_f(float v2[4], float m[16], float v1[4]) {
  ELL_4MV_MUL(v2, m, v1);
}

void
ell_4mv_mul_d(double v2[4], double m[16], double v1[4]) {
  ELL_4MV_MUL(v2, m, v1);
}
