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



/*
******** ell_cubic(): 
**
** finds real roots of x^3 + A*x^2 + B*x + C.
**
** records the found real roots in the given root array. 
**
** returns information about the roots according to ellCubicRoot enum,
** the set the following values in given root[] array:
**   ellCubicRootSingle: root[0], root[1] == root[2] == AIR_NAN
**   ellCubicRootTriple: root[0] == root[1] == root[2]
**   ellCubicRootSingleDouble: single root[0]; double root[1] == root[2]
**                          or double root[0] == root[1], single root[2]
**   ellCubicRootThree: root[0], root[1], root[2]
**
** The values stored in root[] are, in a change from the past, sorted
** in descending order!  No need to sort them any more!
**
** This does NOT use biff
*/
int
ell_cubic(double root[3], double A, double B, double C, int newton) {
  char me[]="ell_cubic";
  double epsilon = 1.0E-11, AA, Q, R, QQQ, D, sqrt_D, der,
    u, v, x, theta, t, sub;

  sub = A/3.0;
  AA = A*A;
  Q = (AA/3.0 - B)/3.0;
  R = (-2.0*A*AA/27.0 + A*B/3.0 - C)/2.0;
  QQQ = Q*Q*Q;
  D = R*R - QQQ;
  if (D < -epsilon) {
    /* three distinct roots- this is the most common case, it has 
       been tested the most, its code should go first */
    theta = acos(R/sqrt(QQQ))/3.0;
    t = 2*sqrt(Q);
    /* yes, these are sorted, because the C definition of acos says
       that it returns values in in [0, pi] */
    root[0] = t*cos(theta) - sub;
    root[1] = t*cos(theta - 2*AIR_PI/3.0) - sub;
    root[2] = t*cos(theta + 2*AIR_PI/3.0) - sub;
    /*
    if (!AIR_EXISTS(root[0])) {
      fprintf(stderr, "%s: %g %g %g --> nan!!!\n", me, A, B, C);
    }
    */
    return ell_cubic_root_three;
  }
  else if (D > epsilon) {
    double nr, fnr;
    /* one real solution, except maybe also a "rescued" double root */
    sqrt_D = sqrt(D);
    u = airCbrt(sqrt_D+R);
    v = -airCbrt(sqrt_D-R);
    x = u+v - sub;
    if (!newton) {
      root[0] = x;
      root[1] = root[2] = AIR_NAN;
      return ell_cubic_root_single;
    }

    /* else refine x, the known root, with newton-raphson, so as to get the 
       most accurate possible calculation for nr, the possible new root */
    x -= (der = (3*x + 2*A)*x + B, ((x/der + A/der)*x + B/der)*x + C/der);
    x -= (der = (3*x + 2*A)*x + B, ((x/der + A/der)*x + B/der)*x + C/der);
    x -= (der = (3*x + 2*A)*x + B, ((x/der + A/der)*x + B/der)*x + C/der);
    x -= (der = (3*x + 2*A)*x + B, ((x/der + A/der)*x + B/der)*x + C/der);
    x -= (der = (3*x + 2*A)*x + B, ((x/der + A/der)*x + B/der)*x + C/der);
    x -= (der = (3*x + 2*A)*x + B, ((x/der + A/der)*x + B/der)*x + C/der);
    nr = -(A + x)/2.0;
    fnr = ((nr + A)*nr + B)*nr + C;  /* the polynomial evaluated at nr */
    /*
    if (ell_debug) {
      fprintf(stderr, "%s: root = %g -> %g, nr=% 20.15f\n"
              "   fnr=% 20.15f\n", me,
              x, (((x + A)*x + B)*x + C), nr, fnr);
    }
    */
    if (fnr < -epsilon || fnr > epsilon) {
      root[0] = x;
      root[1] = root[2] = AIR_NAN;
      return ell_cubic_root_single;
    }
    else {
      if (ell_debug) {
        fprintf(stderr, "%s: rescued double root:% 20.15f\n", me, nr);
      } 
      if (x > nr) {
        root[0] = x;
        root[1] = nr;
        root[2] = nr;
      } else {
        root[0] = nr;
        root[1] = nr;
        root[2] = x;
      }
      return ell_cubic_root_single_double;
    }
  } 
  else {
    /* else D is in the interval [-epsilon, +epsilon] */
    if (R < -epsilon || epsilon < R) {
      /* one double root and one single root */
      u = airCbrt(R);
      if (u > 0) {
        root[0] = 2*u - sub;
        root[1] = -u - sub;
        root[2] = -u - sub;
      } else {
        root[0] = -u - sub;
        root[1] = -u - sub;
        root[2] = 2*u - sub;
      }
      return ell_cubic_root_single_double;
    } 
    else {
      /* one triple root */
      root[0] = root[1] = root[2] = -sub;
      return ell_cubic_root_triple;
    }
  }
  /* shouldn't ever get here */
  /* return ell_cubic_root_unknown; */
}




