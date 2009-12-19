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

#ifndef TENMACROS_HAS_BEEN_INCLUDED
#define TENMACROS_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* 
******** TEN_T2M, TEN_M2T
**
** for going between 7-element list and 9-element matrix
** representations of a symmetric tensor
**
** the ordering of the tensor elements is assumed to be:
**
** threshold        0
** Dxx Dxy Dxz      1   2   3
** Dxy Dyy Dyz  =  (2)  4   5
** Dxz Dyz Dzz     (3) (5)  6 
**
** As in ell, the matrix ordering is given by:
**
**   0  1  2
**   3  4  5
**   6  7  8
**
** Note that TEN_M2T does NOT set the threshold element (index 0),
** and that the threshold value plays no role in TEN_T2M.
*/

#define TEN_T2M(m, t) ( \
   (m)[0] = (t)[1], (m)[1] = (t)[2], (m)[2] = (t)[3], \
   (m)[3] = (t)[2], (m)[4] = (t)[4], (m)[5] = (t)[5], \
   (m)[6] = (t)[3], (m)[7] = (t)[5], (m)[8] = (t)[6] )

#define TEN_M2T(t, m) ( \
   (t)[1] = (m)[0], (t)[2] = (m)[1], (t)[3] = (m)[2], \
                    (t)[4] = (m)[4], (t)[5] = (m)[5], \
                                     (t)[6] = (m)[8] )

#define TEN_M2T_TT(t, TT, m) ( \
   (t)[1] = AIR_CAST(TT, (m)[0]), \
   (t)[2] = AIR_CAST(TT, (m)[1]), \
   (t)[3] = AIR_CAST(TT, (m)[2]), \
   (t)[4] = AIR_CAST(TT, (m)[4]), \
   (t)[5] = AIR_CAST(TT, (m)[5]), \
   (t)[6] = AIR_CAST(TT, (m)[8]))

#define TEN_TV_MUL(v2, t, v1) \
  ((v2)[0] = (t)[1]*(v1)[0] + (t)[2]*(v1)[1] + (t)[3]*(v1)[2], \
   (v2)[1] = (t)[2]*(v1)[0] + (t)[4]*(v1)[1] + (t)[5]*(v1)[2], \
   (v2)[2] = (t)[3]*(v1)[0] + (t)[5]*(v1)[1] + (t)[6]*(v1)[2])

#define TEN_T_EXISTS(t) ( \
    AIR_EXISTS((t)[0]) \
 && AIR_EXISTS((t)[1]) \
 && AIR_EXISTS((t)[2]) \
 && AIR_EXISTS((t)[3]) \
 && AIR_EXISTS((t)[4]) \
 && AIR_EXISTS((t)[5]) \
 && AIR_EXISTS((t)[6]) )

#define TEN_T_SET(t, conf, a, b, c, d, e, f) ( \
   (t)[0] = (conf), \
   (t)[1] = (a), (t)[2] = (b), (t)[3] = (c), \
                 (t)[4] = (d), (t)[5] = (e), \
                               (t)[6] = (f) )

#define TEN_T_SET_TT(t, TT, conf, a, b, c, d, e, f) ( \
   (t)[0] = AIR_CAST(TT, (conf)), \
   (t)[1] = AIR_CAST(TT, (a)), \
   (t)[2] = AIR_CAST(TT, (b)), \
   (t)[3] = AIR_CAST(TT, (c)), \
   (t)[4] = AIR_CAST(TT, (d)), \
   (t)[5] = AIR_CAST(TT, (e)), \
   (t)[6] = AIR_CAST(TT, (f)))

#define TEN_T_COPY(d, s) ( \
   (d)[0] = (s)[0], \
   (d)[1] = (s)[1], \
   (d)[2] = (s)[2], \
   (d)[3] = (s)[3], \
   (d)[4] = (s)[4], \
   (d)[5] = (s)[5], \
   (d)[6] = (s)[6] )

#define TEN_T_COPY_TT(d, TT, s) ( \
   (d)[0] = AIR_CAST(TT, (s)[0]), \
   (d)[1] = AIR_CAST(TT, (s)[1]), \
   (d)[2] = AIR_CAST(TT, (s)[2]), \
   (d)[3] = AIR_CAST(TT, (s)[3]), \
   (d)[4] = AIR_CAST(TT, (s)[4]), \
   (d)[5] = AIR_CAST(TT, (s)[5]), \
   (d)[6] = AIR_CAST(TT, (s)[6]) )

#define TEN_T_DET(t) ( \
  (t)[1]*((t)[4]*(t)[6] - (t)[5]*(t)[5]) \
  + (t)[2]*((t)[5]*(t)[3] - (t)[2]*(t)[6]) \
  + (t)[3]*((t)[2]*(t)[5] - (t)[3]*(t)[4]))

#define TEN_T_INV(i, t, det)                                 \
  ((det) = TEN_T_DET(t),                                     \
   (i)[0] = (t)[0],                                          \
   (i)[1] =  _ELL_2M_DET((t)[4],(t)[5],(t)[5],(t)[6])/(det), \
   (i)[2] = -_ELL_2M_DET((t)[2],(t)[5],(t)[3],(t)[6])/(det), \
   (i)[3] =  _ELL_2M_DET((t)[2],(t)[4],(t)[3],(t)[5])/(det), \
   (i)[4] =  _ELL_2M_DET((t)[1],(t)[3],(t)[3],(t)[6])/(det), \
   (i)[5] = -_ELL_2M_DET((t)[1],(t)[2],(t)[3],(t)[5])/(det), \
   (i)[6] =  _ELL_2M_DET((t)[1],(t)[2],(t)[2],(t)[4])/(det))

#define TEN_T_DOT(A, B) ( \
  (A)[1]*(B)[1] + 2*(A)[2]*(B)[2] + 2*(A)[3]*(B)[3] \
                +   (A)[4]*(B)[4] + 2*(A)[5]*(B)[5] \
                                  +   (A)[6]*(B)[6] )

#define TEN_T_NORM(A) (sqrt(TEN_T_DOT(A,A)))

#define TEN_T_SCALE(a, s, b) ( \
   (a)[0] = (b)[0],               \
   (a)[1] = (s)*(b)[1],           \
   (a)[2] = (s)*(b)[2],           \
   (a)[3] = (s)*(b)[3],           \
   (a)[4] = (s)*(b)[4],           \
   (a)[5] = (s)*(b)[5],           \
   (a)[6] = (s)*(b)[6])

#define TEN_T_SCALE_INCR(a, s, b) ( \
   (a)[0] = (b)[0],               \
   (a)[1] += (s)*(b)[1],          \
   (a)[2] += (s)*(b)[2],          \
   (a)[3] += (s)*(b)[3],          \
   (a)[4] += (s)*(b)[4],          \
   (a)[5] += (s)*(b)[5],          \
   (a)[6] += (s)*(b)[6])

#define TEN_T_SCALE_INCR2(a, s, b, t, c) ( \
   (a)[0] = AIR_MIN((b)[0], (c)[0]),  \
   (a)[1] += (s)*(b)[1] + (t)*(c)[1], \
   (a)[2] += (s)*(b)[2] + (t)*(c)[2], \
   (a)[3] += (s)*(b)[3] + (t)*(c)[3], \
   (a)[4] += (s)*(b)[4] + (t)*(c)[4], \
   (a)[5] += (s)*(b)[5] + (t)*(c)[5], \
   (a)[6] += (s)*(b)[6] + (t)*(c)[6])

#define TEN_T_SCALE_ADD2(a, s, b, t, c) ( \
   (a)[0] = AIR_MIN((b)[0], (c)[0]),  \
   (a)[1] = (s)*(b)[1] + (t)*(c)[1],  \
   (a)[2] = (s)*(b)[2] + (t)*(c)[2],  \
   (a)[3] = (s)*(b)[3] + (t)*(c)[3],  \
   (a)[4] = (s)*(b)[4] + (t)*(c)[4],  \
   (a)[5] = (s)*(b)[5] + (t)*(c)[5],  \
   (a)[6] = (s)*(b)[6] + (t)*(c)[6])

#define TEN_T3V_MUL(b, t, a) (                            \
  (b)[0] = (t)[1]*(a)[0] + (t)[2]*(a)[1] + (t)[3]*(a)[2], \
  (b)[1] = (t)[2]*(a)[0] + (t)[4]*(a)[1] + (t)[5]*(a)[2], \
  (b)[2] = (t)[3]*(a)[0] + (t)[5]*(a)[1] + (t)[6]*(a)[2])

#define TEN_T3V_OUTER(t, a) (                                             \
  (t)[0] = 1.0,                                                           \
  (t)[1] = (a)[0]*(a)[0], (t)[2] = (a)[0]*(a)[1], (t)[3] = (a)[0]*(a)[2], \
                          (t)[4] = (a)[1]*(a)[1], (t)[5] = (a)[1]*(a)[2], \
                                                  (t)[6] = (a)[2]*(a)[2])

#ifdef __cplusplus
}
#endif

#endif /* TENMACROS_HAS_BEEN_INCLUDED */
