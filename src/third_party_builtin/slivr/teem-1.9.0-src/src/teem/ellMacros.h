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

#ifndef ELLMACROS_HAS_BEEN_INCLUDED
#define ELLMACROS_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*
******** ELL_SWAP2, ELL_SWAP3
**
** used to interchange 2 or 3 values, using the given temp variable
*/
#define ELL_SWAP2(a, b, t)    ((t)=(a),(a)=(b),(b)=(t))
#define ELL_SWAP3(a, b, c, t) ((t)=(a),(a)=(b),(b)=(c),(c)=(t))

/*
******** ELL_SORT3
**
** sorts v0, v1, v2 in descending order, using given temp variable t,
*/
#define ELL_SORT3(v0, v1, v2, t)             \
  if (v0 > v1) {                             \
    if (v1 < v2) {                           \
      if (v0 > v2) { ELL_SWAP2(v1, v2, t); } \
      else { ELL_SWAP3(v0, v2, v1, t); }     \
    }                                        \
  }                                          \
  else {                                     \
    if (v1 > v2) {                           \
      if (v0 > v2) { ELL_SWAP2(v0, v1, t); } \
      else { ELL_SWAP3(v0, v1, v2, t); }     \
    }                                        \
    else {                                   \
      ELL_SWAP2(v0, v2, t);                  \
    }                                        \
  }

/*
******** ELL_MAX3_IDX
**
** returns 0, 1, 2, to indicate which of the three arguments is largest
*/
#define ELL_MAX3_IDX(v0, v1, v2) \
  (v0 > v1                       \
   ? (v1 > v2                    \
      ? 0                        \
      : (v0 > v2                 \
         ? 0                     \
         : 2))                   \
   : (v2 > v1                    \
      ? 2                        \
      : 1))

/*
******** ELL_MIN3_IDX
**
** returns 0, 1, 2, to indicate which of the three arguments is smallest
*/
#define ELL_MIN3_IDX(v0, v1, v2) \
  (v0 < v1                       \
   ? (v1 < v2                    \
      ? 0                        \
      : (v0 < v2                 \
         ? 0                     \
         : 2))                   \
   : (v2 < v1                    \
      ? 2                        \
      : 1))

#define ELL_2V_SET(v, a, b) \
  ((v)[0]=(a), (v)[1]=(b))

#define ELL_2V_DOT(v1, v2) ((v1)[0]*(v2)[0] + (v1)[1]*(v2)[1])

#define ELL_2V_LEN(v) (sqrt(ELL_2V_DOT((v),(v))))

#define ELL_2V_SCALE(v2, a, v1) \
  ((v2)[0] = (a)*(v1)[0],       \
   (v2)[1] = (a)*(v1)[1])

#define ELL_2V_NORM(v2, v1, length) \
  (length = ELL_2V_LEN(v1), ELL_2V_SCALE(v2, 1.0/length, v1))

#define _ELL_2M_DET(a,b,c,d) ((a)*(d) - (b)*(c))

#define ELL_2M_DET(m) _ELL_2M_DET((m)[0],(m)[1],(m)[2],(m)[3])

/*
** the 3x3 matrix-related macros assume that the matrix indexing is:
** 0  1  2
** 3  4  5
** 6  7  8
*/

#define ELL_3V_SET(v, a, b, c) \
  ((v)[0] = (a), (v)[1] = (b), (v)[2] = (c))

#define ELL_3V_SET_TT(v, TT, a, b, c) \
  ((v)[0] = AIR_CAST(TT, (a)), \
   (v)[1] = AIR_CAST(TT, (b)), \
   (v)[2] = AIR_CAST(TT, (c)))

#define ELL_3V_GET(a, b, c, v) \
  ((a) = (v)[0], (b) = (v)[1], (c) = (v)[2])

#define ELL_3V_EQUAL(a, b) \
  ((a)[0] == (b)[0] && (a)[1] == (b)[1] && (a)[2] == (b)[2])

#define ELL_3V_COPY(v2, v1) \
  ((v2)[0] = (v1)[0], (v2)[1] = (v1)[1], (v2)[2] = (v1)[2])

#define ELL_3V_COPY_TT(v2, TYPE, v1) \
  ((v2)[0] = AIR_CAST(TYPE, (v1)[0]), \
   (v2)[1] = AIR_CAST(TYPE, (v1)[1]), \
   (v2)[2] = AIR_CAST(TYPE, (v1)[2]))

#define ELL_3V_INCR(v2, v1) \
  ((v2)[0] += (v1)[0],      \
   (v2)[1] += (v1)[1],      \
   (v2)[2] += (v1)[2])

#define ELL_3V_LERP(v3, w, v1, v2)            \
  ((v3)[0] = AIR_LERP((w), (v1)[0], (v2)[0]), \
   (v3)[1] = AIR_LERP((w), (v1)[1], (v2)[1]), \
   (v3)[2] = AIR_LERP((w), (v1)[2], (v2)[2]))

#define ELL_3V_ADD2(v3, v1, v2) \
  ((v3)[0] = (v1)[0] + (v2)[0], \
   (v3)[1] = (v1)[1] + (v2)[1], \
   (v3)[2] = (v1)[2] + (v2)[2])

#define ELL_3V_ADD3(v4, v1, v2, v3)       \
  ((v4)[0] = (v1)[0] + (v2)[0] + (v3)[0], \
   (v4)[1] = (v1)[1] + (v2)[1] + (v3)[1], \
   (v4)[2] = (v1)[2] + (v2)[2] + (v3)[2])

#define ELL_3V_ADD4(v5, v1, v2, v3, v4)       \
  ((v5)[0] = (v1)[0] + (v2)[0] + (v3)[0] + (v4)[0], \
   (v5)[1] = (v1)[1] + (v2)[1] + (v3)[1] + (v4)[1], \
   (v5)[2] = (v1)[2] + (v2)[2] + (v3)[2] + (v4)[2])

#define ELL_3V_SUB(v3, v1, v2)  \
  ((v3)[0] = (v1)[0] - (v2)[0], \
   (v3)[1] = (v1)[1] - (v2)[1], \
   (v3)[2] = (v1)[2] - (v2)[2])

#define ELL_3V_DOT(v1, v2) \
  ((v1)[0]*(v2)[0] + (v1)[1]*(v2)[1] + (v1)[2]*(v2)[2])

#define ELL_3V_SCALE(v2, a, v1) \
  ((v2)[0] = (a)*(v1)[0],       \
   (v2)[1] = (a)*(v1)[1],       \
   (v2)[2] = (a)*(v1)[2])

#define ELL_3V_SCALE_TT(v2, TT, a, v1)   \
  ((v2)[0] = AIR_CAST(TT, (a)*(v1)[0]), \
   (v2)[1] = AIR_CAST(TT, (a)*(v1)[1]), \
   (v2)[2] = AIR_CAST(TT, (a)*(v1)[2]))

#define ELL_3V_SCALE_INCR(v2, s0, v0) \
  ((v2)[0] += (s0)*(v0)[0], \
   (v2)[1] += (s0)*(v0)[1], \
   (v2)[2] += (s0)*(v0)[2])

#define ELL_3V_SCALE_INCR_TT(v2, TT, s0, v0) \
  ((v2)[0] += AIR_CAST(TT, (s0)*(v0)[0]), \
   (v2)[1] += AIR_CAST(TT, (s0)*(v0)[1]), \
   (v2)[2] += AIR_CAST(TT, (s0)*(v0)[2]))

#define ELL_3V_SCALE_ADD2(v2, s0, v0, s1, v1) \
  ((v2)[0] = (s0)*(v0)[0] + (s1)*(v1)[0],     \
   (v2)[1] = (s0)*(v0)[1] + (s1)*(v1)[1],     \
   (v2)[2] = (s0)*(v0)[2] + (s1)*(v1)[2])

#define ELL_3V_SCALE_INCR2(v2, s0, v0, s1, v1) \
  ((v2)[0] += (s0)*(v0)[0] + (s1)*(v1)[0],     \
   (v2)[1] += (s0)*(v0)[1] + (s1)*(v1)[1],     \
   (v2)[2] += (s0)*(v0)[2] + (s1)*(v1)[2])

#define ELL_3V_SCALE_ADD3(v3, s0, v0, s1, v1, s2, v2)     \
  ((v3)[0] = (s0)*(v0)[0] + (s1)*(v1)[0] + (s2)*(v2)[0],  \
   (v3)[1] = (s0)*(v0)[1] + (s1)*(v1)[1] + (s2)*(v2)[1],  \
   (v3)[2] = (s0)*(v0)[2] + (s1)*(v1)[2] + (s2)*(v2)[2])

#define ELL_3V_SCALE_ADD4(v4, s0, v0, s1, v1, s2, v2, s3, v3)           \
  ((v4)[0] = (s0)*(v0)[0] + (s1)*(v1)[0] + (s2)*(v2)[0] + (s3)*(v3)[0], \
   (v4)[1] = (s0)*(v0)[1] + (s1)*(v1)[1] + (s2)*(v2)[1] + (s3)*(v3)[1], \
   (v4)[2] = (s0)*(v0)[2] + (s1)*(v1)[2] + (s2)*(v2)[2] + (s3)*(v3)[2])

#define ELL_3V_SCALE_ADD6(v6, s0, v0, s1, v1, s2, v2,     \
                              s3, v3, s4, v4, s5, v5)     \
  ((v6)[0] = (s0)*(v0)[0] + (s1)*(v1)[0] + (s2)*(v2)[0]   \
           + (s3)*(v3)[0] + (s4)*(v4)[0] + (s5)*(v5)[0],  \
   (v6)[1] = (s0)*(v0)[1] + (s1)*(v1)[1] + (s2)*(v2)[1]   \
           + (s3)*(v3)[1] + (s4)*(v4)[1] + (s5)*(v5)[1],  \
   (v6)[2] = (s0)*(v0)[2] + (s1)*(v1)[2] + (s2)*(v2)[2]   \
           + (s3)*(v3)[2] + (s4)*(v4)[2] + (s5)*(v5)[2])

#define ELL_3V_SCALE_INCR3(v3, s0, v0, s1, v1, s2, v2)     \
  ((v3)[0] += (s0)*(v0)[0] + (s1)*(v1)[0] + (s2)*(v2)[0],  \
   (v3)[1] += (s0)*(v0)[1] + (s1)*(v1)[1] + (s2)*(v2)[1],  \
   (v3)[2] += (s0)*(v0)[2] + (s1)*(v1)[2] + (s2)*(v2)[2])

#define ELL_3V_LEN(v) (sqrt(ELL_3V_DOT((v),(v))))

#define ELL_3V_DIST(a, b)                    \
  sqrt(((a)[0] - (b)[0])*((a)[0] - (b)[0]) + \
       ((a)[1] - (b)[1])*((a)[1] - (b)[1]) + \
       ((a)[2] - (b)[2])*((a)[2] - (b)[2]))

#define ELL_3V_NORM(v2, v1, length) \
  (length = ELL_3V_LEN(v1), ELL_3V_SCALE(v2, 1.0/length, v1))

#define ELL_3V_NORM_TT(v2, TT, v1, length) \
  (length = AIR_CAST(TT, ELL_3V_LEN(v1)), \
   ELL_3V_SCALE_TT(v2, TT, 1.0/length, v1))

#define ELL_3V_CROSS(v3, v1, v2) \
  ((v3)[0] = (v1)[1]*(v2)[2] - (v1)[2]*(v2)[1], \
   (v3)[1] = (v1)[2]*(v2)[0] - (v1)[0]*(v2)[2], \
   (v3)[2] = (v1)[0]*(v2)[1] - (v1)[1]*(v2)[0])

#define ELL_3V_MIN(v3,v1,v2) (         \
  (v3)[0] = AIR_MIN((v1)[0], (v2)[0]), \
  (v3)[1] = AIR_MIN((v1)[1], (v2)[1]), \
  (v3)[2] = AIR_MIN((v1)[2], (v2)[2]))

#define ELL_3V_MAX(v3,v1,v2) (         \
  (v3)[0] = AIR_MAX((v1)[0], (v2)[0]), \
  (v3)[1] = AIR_MAX((v1)[1], (v2)[1]), \
  (v3)[2] = AIR_MAX((v1)[2], (v2)[2]))

#define ELL_3V_EXISTS(v) \
   (AIR_EXISTS((v)[0]) && AIR_EXISTS((v)[1]) && AIR_EXISTS((v)[2]))

#define ELL_3V_AFFINE(v,i,x,I,o,O) ( \
  (v)[0] = AIR_AFFINE((i)[0],(x)[0],(I)[0],(o)[0],(O)[0]), \
  (v)[1] = AIR_AFFINE((i)[1],(x)[1],(I)[1],(o)[1],(O)[1]), \
  (v)[2] = AIR_AFFINE((i)[2],(x)[2],(I)[2],(o)[2],(O)[2]))

#define ELL_3V_ABS(v2,v1) ( \
  (v2)[0] = AIR_ABS((v1)[0]), \
  (v2)[1] = AIR_ABS((v1)[1]), \
  (v2)[2] = AIR_ABS((v1)[2]))

#define ELL_3M_EQUAL(m1, m2) \
  ((m1)[0] == (m2)[0] &&     \
   (m1)[1] == (m2)[1] &&     \
   (m1)[2] == (m2)[2] &&     \
   (m1)[3] == (m2)[3] &&     \
   (m1)[4] == (m2)[4] &&     \
   (m1)[5] == (m2)[5] &&     \
   (m1)[6] == (m2)[6] &&     \
   (m1)[7] == (m2)[7] &&     \
   (m1)[8] == (m2)[8])

#define ELL_3M_SCALE(m2, s, m1) \
  (ELL_3V_SCALE((m2)+0, (s), (m1)+0), \
   ELL_3V_SCALE((m2)+3, (s), (m1)+3), \
   ELL_3V_SCALE((m2)+6, (s), (m1)+6))

#define ELL_3M_SCALE_ADD2(m2, s0, m0, s1, m1) \
  (ELL_3V_SCALE_ADD2((m2)+0, (s0), (m0)+0, (s1), (m1)+0), \
   ELL_3V_SCALE_ADD2((m2)+3, (s0), (m0)+3, (s1), (m1)+3), \
   ELL_3V_SCALE_ADD2((m2)+6, (s0), (m0)+6, (s1), (m1)+6))

#define ELL_3M_ADD2(m3, m1, m2) \
  ((m3)[0] = (m1)[0] + (m2)[0],  \
   (m3)[1] = (m1)[1] + (m2)[1],  \
   (m3)[2] = (m1)[2] + (m2)[2],  \
   (m3)[3] = (m1)[3] + (m2)[3],  \
   (m3)[4] = (m1)[4] + (m2)[4],  \
   (m3)[5] = (m1)[5] + (m2)[5],  \
   (m3)[6] = (m1)[6] + (m2)[6],  \
   (m3)[7] = (m1)[7] + (m2)[7],  \
   (m3)[8] = (m1)[8] + (m2)[8])

#define ELL_3M_SUB(m3, m1, m2) \
  ((m3)[0] = (m1)[0] - (m2)[0],  \
   (m3)[1] = (m1)[1] - (m2)[1],  \
   (m3)[2] = (m1)[2] - (m2)[2],  \
   (m3)[3] = (m1)[3] - (m2)[3],  \
   (m3)[4] = (m1)[4] - (m2)[4],  \
   (m3)[5] = (m1)[5] - (m2)[5],  \
   (m3)[6] = (m1)[6] - (m2)[6],  \
   (m3)[7] = (m1)[7] - (m2)[7],  \
   (m3)[8] = (m1)[8] - (m2)[8])

#define ELL_3M_COPY(m2, m1) \
  (ELL_3V_COPY((m2)+0, (m1)+0), \
   ELL_3V_COPY((m2)+3, (m1)+3), \
   ELL_3V_COPY((m2)+6, (m1)+6))

#define ELL_3M_COPY_TT(m2, TYPE, m1) \
  (ELL_3V_COPY_TT((m2)+0, TYPE, (m1)+0), \
   ELL_3V_COPY_TT((m2)+3, TYPE, (m1)+3), \
   ELL_3V_COPY_TT((m2)+6, TYPE, (m1)+6))

#define ELL_3M_IDENTITY_SET(m) \
  (ELL_3V_SET((m)+0,  1 ,  0 ,  0), \
   ELL_3V_SET((m)+3,  0 ,  1 ,  0), \
   ELL_3V_SET((m)+6,  0 ,  0 ,  1))

#define ELL_3M_EXISTS(m) \
  (ELL_3V_EXISTS((m) + 0) \
   && ELL_3V_EXISTS((m) + 3) \
   && ELL_3V_EXISTS((m) + 6))

#define ELL_3M_ZERO_SET(m) \
  (ELL_3V_SET((m)+0,  0 ,  0 ,  0), \
   ELL_3V_SET((m)+3,  0 ,  0 ,  0), \
   ELL_3V_SET((m)+6,  0 ,  0 ,  0))

#define ELL_3M_DIAG_SET(m, a, b, c) \
  ((m)[0] = (a), (m)[4] = (b), (m)[8] = (c))

#define ELL_3M_TRANSPOSE(m2, m1) \
  ((m2)[0] = (m1)[0],            \
   (m2)[1] = (m1)[3],            \
   (m2)[2] = (m1)[6],            \
   (m2)[3] = (m1)[1],            \
   (m2)[4] = (m1)[4],            \
   (m2)[5] = (m1)[7],            \
   (m2)[6] = (m1)[2],            \
   (m2)[7] = (m1)[5],            \
   (m2)[8] = (m1)[8])

#define ELL_3M_TRANSPOSE_IP(m, t) \
  (ELL_SWAP2((m)[1],(m)[3],(t)),  \
   ELL_SWAP2((m)[2],(m)[6],(t)),  \
   ELL_SWAP2((m)[5],(m)[7],(t)))

#define ELL_3M_TRACE(m) ((m)[0] + (m)[4] + (m)[8])

#define ELL_3M_FROB(m) \
  (sqrt(ELL_3V_DOT((m)+0, (m)+0) + \
        ELL_3V_DOT((m)+3, (m)+3) + \
        ELL_3V_DOT((m)+6, (m)+6)))

#define _ELL_3M_DET(a,b,c,d,e,f,g,h,i) \
  (  (a)*(e)*(i) \
   + (d)*(h)*(c) \
   + (g)*(b)*(f) \
   - (g)*(e)*(c) \
   - (d)*(b)*(i) \
   - (a)*(h)*(f))

#define ELL_3M_DET(m) _ELL_3M_DET((m)[0],(m)[1],(m)[2],\
                                  (m)[3],(m)[4],(m)[5],\
                                  (m)[6],(m)[7],(m)[8])

#define ELL_3MV_COL0_GET(v, m) \
  (ELL_3V_SET((v), (m)[0], (m)[3], (m)[6]))

#define ELL_3MV_COL1_GET(v, m) \
  (ELL_3V_SET((v), (m)[1], (m)[4], (m)[7]))

#define ELL_3MV_COL2_GET(v, m) \
  (ELL_3V_SET((v), (m)[2], (m)[5], (m)[8]))

#define ELL_3MV_ROW0_GET(v, m) \
  (ELL_3V_SET((v), (m)[0], (m)[1], (m)[2]))

#define ELL_3MV_ROW1_GET(v, m) \
  (ELL_3V_SET((v), (m)[3], (m)[4], (m)[5]))

#define ELL_3MV_ROW2_GET(v, m) \
  (ELL_3V_SET((v), (m)[6], (m)[7], (m)[8]))

#define ELL_3MV_COL0_SET(m, v) \
  (ELL_3V_GET((m)[0], (m)[3], (m)[6], (v)))

#define ELL_3MV_COL1_SET(m, v) \
  (ELL_3V_GET((m)[1], (m)[4], (m)[7], (v)))

#define ELL_3MV_COL2_SET(m, v) \
  (ELL_3V_GET((m)[2], (m)[5], (m)[8], (v)))

#define ELL_3MV_ROW0_SET(m, v) \
  (ELL_3V_GET((m)[0], (m)[1], (m)[2], (v)))

#define ELL_3MV_ROW1_SET(m, v) \
  (ELL_3V_GET((m)[3], (m)[4], (m)[5], (v)))

#define ELL_3MV_ROW2_SET(m, v) \
  (ELL_3V_GET((m)[6], (m)[7], (m)[8], (v)))

#define ELL_3MV_OUTER(m, v1, v2) \
  (ELL_3V_SCALE((m)+0, (v1)[0], (v2)), \
   ELL_3V_SCALE((m)+3, (v1)[1], (v2)), \
   ELL_3V_SCALE((m)+6, (v1)[2], (v2)))

#define ELL_3MV_OUTER_ADD(m, v1, v2) \
  (ELL_3V_SCALE_INCR((m)+0, (v1)[0], (v2)), \
   ELL_3V_SCALE_INCR((m)+3, (v1)[1], (v2)), \
   ELL_3V_SCALE_INCR((m)+6, (v1)[2], (v2)))

#define ELL_3MV_MUL(v2, m, v1) \
  ((v2)[0] = (m)[0]*(v1)[0] + (m)[1]*(v1)[1] + (m)[2]*(v1)[2], \
   (v2)[1] = (m)[3]*(v1)[0] + (m)[4]*(v1)[1] + (m)[5]*(v1)[2], \
   (v2)[2] = (m)[6]*(v1)[0] + (m)[7]*(v1)[1] + (m)[8]*(v1)[2])

#define ELL_3MV_MUL_TT(v2, TT, m, v1) \
  ((v2)[0] = AIR_CAST(TT, (m)[0]*(v1)[0] + (m)[1]*(v1)[1] + (m)[2]*(v1)[2]), \
   (v2)[1] = AIR_CAST(TT, (m)[3]*(v1)[0] + (m)[4]*(v1)[1] + (m)[5]*(v1)[2]), \
   (v2)[2] = AIR_CAST(TT, (m)[6]*(v1)[0] + (m)[7]*(v1)[1] + (m)[8]*(v1)[2]))

#define ELL_3MV_TMUL(v2, m, v1) \
  ((v2)[0] = (m)[0]*(v1)[0] + (m)[3]*(v1)[1] + (m)[6]*(v1)[2], \
   (v2)[1] = (m)[1]*(v1)[0] + (m)[4]*(v1)[1] + (m)[7]*(v1)[2], \
   (v2)[2] = (m)[2]*(v1)[0] + (m)[5]*(v1)[1] + (m)[8]*(v1)[2])

#define ELL_3M_MUL(m3, m1, m2)                                    \
  ((m3)[0] = (m1)[0]*(m2)[0] + (m1)[1]*(m2)[3] + (m1)[2]*(m2)[6], \
   (m3)[1] = (m1)[0]*(m2)[1] + (m1)[1]*(m2)[4] + (m1)[2]*(m2)[7], \
   (m3)[2] = (m1)[0]*(m2)[2] + (m1)[1]*(m2)[5] + (m1)[2]*(m2)[8], \
                                                                  \
   (m3)[3] = (m1)[3]*(m2)[0] + (m1)[4]*(m2)[3] + (m1)[5]*(m2)[6], \
   (m3)[4] = (m1)[3]*(m2)[1] + (m1)[4]*(m2)[4] + (m1)[5]*(m2)[7], \
   (m3)[5] = (m1)[3]*(m2)[2] + (m1)[4]*(m2)[5] + (m1)[5]*(m2)[8], \
                                                                  \
   (m3)[6] = (m1)[6]*(m2)[0] + (m1)[7]*(m2)[3] + (m1)[8]*(m2)[6], \
   (m3)[7] = (m1)[6]*(m2)[1] + (m1)[7]*(m2)[4] + (m1)[8]*(m2)[7], \
   (m3)[8] = (m1)[6]*(m2)[2] + (m1)[7]*(m2)[5] + (m1)[8]*(m2)[8])

#define ELL_3M_MUL_TT(m3, TT, m1, m2)                                    \
  ((m3)[0] = AIR_CAST(TT, (m1)[0]*(m2)[0]+(m1)[1]*(m2)[3]+(m1)[2]*(m2)[6]), \
   (m3)[1] = AIR_CAST(TT, (m1)[0]*(m2)[1]+(m1)[1]*(m2)[4]+(m1)[2]*(m2)[7]), \
   (m3)[2] = AIR_CAST(TT, (m1)[0]*(m2)[2]+(m1)[1]*(m2)[5]+(m1)[2]*(m2)[8]), \
                                                                  \
   (m3)[3] = AIR_CAST(TT, (m1)[3]*(m2)[0]+(m1)[4]*(m2)[3]+(m1)[5]*(m2)[6]), \
   (m3)[4] = AIR_CAST(TT, (m1)[3]*(m2)[1]+(m1)[4]*(m2)[4]+(m1)[5]*(m2)[7]), \
   (m3)[5] = AIR_CAST(TT, (m1)[3]*(m2)[2]+(m1)[4]*(m2)[5]+(m1)[5]*(m2)[8]), \
                                                                  \
   (m3)[6] = AIR_CAST(TT, (m1)[6]*(m2)[0]+(m1)[7]*(m2)[3]+(m1)[8]*(m2)[6]), \
   (m3)[7] = AIR_CAST(TT, (m1)[6]*(m2)[1]+(m1)[7]*(m2)[4]+(m1)[8]*(m2)[7]), \
   (m3)[8] = AIR_CAST(TT, (m1)[6]*(m2)[2]+(m1)[7]*(m2)[5]+(m1)[8]*(m2)[8]))

#define ELL_3M_INV(m2, m1, det)                                   \
  ((det) = ELL_3M_DET(m1),                                        \
   (m2)[0] =  _ELL_2M_DET((m1)[4],(m1)[5],(m1)[7],(m1)[8])/(det), \
   (m2)[1] = -_ELL_2M_DET((m1)[1],(m1)[2],(m1)[7],(m1)[8])/(det), \
   (m2)[2] =  _ELL_2M_DET((m1)[1],(m1)[2],(m1)[4],(m1)[5])/(det), \
   (m2)[3] = -_ELL_2M_DET((m1)[3],(m1)[5],(m1)[6],(m1)[8])/(det), \
   (m2)[4] =  _ELL_2M_DET((m1)[0],(m1)[2],(m1)[6],(m1)[8])/(det), \
   (m2)[5] = -_ELL_2M_DET((m1)[0],(m1)[2],(m1)[3],(m1)[5])/(det), \
   (m2)[6] =  _ELL_2M_DET((m1)[3],(m1)[4],(m1)[6],(m1)[7])/(det), \
   (m2)[7] = -_ELL_2M_DET((m1)[0],(m1)[1],(m1)[6],(m1)[7])/(det), \
   (m2)[8] =  _ELL_2M_DET((m1)[0],(m1)[1],(m1)[3],(m1)[4])/(det))

#define ELL_3M_SCALE_SET(m, x, y, z)  \
  (ELL_3V_SET((m)+ 0, (x),  0 ,  0 ), \
   ELL_3V_SET((m)+ 3,  0 , (y),  0 ), \
   ELL_3V_SET((m)+ 6,  0 ,  0 , (z)))

#define ELL_3M_ROTATE_X_SET(m, th)               \
  (ELL_3V_SET((m)+ 0,  1 ,     0    ,     0   ), \
   ELL_3V_SET((m)+ 3,  0 ,  cos(th) , -sin(th)), \
   ELL_3V_SET((m)+ 6,  0 , +sin(th) ,  cos(th)))

#define ELL_3M_ROTATE_Y_SET(m, th)               \
  (ELL_3V_SET((m)+ 0,  cos(th) ,  0 , +sin(th)), \
   ELL_3V_SET((m)+ 3,     0    ,  1 ,     0   ), \
   ELL_3V_SET((m)+ 6, -sin(th) ,  0 ,  cos(th)))

#define ELL_3M_ROTATE_Z_SET(m, th)               \
  (ELL_3V_SET((m)+ 0,  cos(th) , -sin(th) ,  0), \
   ELL_3V_SET((m)+ 3, +sin(th) ,  cos(th) ,  0), \
   ELL_3V_SET((m)+ 6,     0    ,     0    ,  1))

/*
** the 4x4 matrix-related macros assume that the matrix indexing is:
**
**  0   1   2   3
**  4   5   6   7
**  8   9  10  11
** 12  13  14  15
*/

#define ELL_4V_SET(v, a, b, c, d) \
  ((v)[0] = (a), (v)[1] = (b), (v)[2] = (c), (v)[3] = (d))

#define ELL_4V_SET_TT(v, TT, a, b, c, d) \
  ((v)[0] = AIR_CAST(TT, (a)), \
   (v)[1] = AIR_CAST(TT, (b)), \
   (v)[2] = AIR_CAST(TT, (c)), \
   (v)[3] = AIR_CAST(TT, (d)))

#define ELL_4V_GET(a, b, c, d, v) \
  ((a) = (v)[0], (b) = (v)[1], (c) = (v)[2], (d) = (v)[3])

#define ELL_4V_COPY(v2, v1) \
  ((v2)[0] = (v1)[0],       \
   (v2)[1] = (v1)[1],       \
   (v2)[2] = (v1)[2],       \
   (v2)[3] = (v1)[3])

#define ELL_4V_COPY_TT(v2, TT, v1)  \
  ((v2)[0] = AIR_CAST(TT, (v1)[0]), \
   (v2)[1] = AIR_CAST(TT, (v1)[1]), \
   (v2)[2] = AIR_CAST(TT, (v1)[2]), \
   (v2)[3] = AIR_CAST(TT, (v1)[3]))

#define ELL_4V_ADD2(v3, v1, v2)  \
  ((v3)[0] = (v1)[0] + (v2)[0], \
   (v3)[1] = (v1)[1] + (v2)[1], \
   (v3)[2] = (v1)[2] + (v2)[2], \
   (v3)[3] = (v1)[3] + (v2)[3])

#define ELL_4V_SUB(v3, v1, v2)  \
  ((v3)[0] = (v1)[0] - (v2)[0], \
   (v3)[1] = (v1)[1] - (v2)[1], \
   (v3)[2] = (v1)[2] - (v2)[2], \
   (v3)[3] = (v1)[3] - (v2)[3])

#define ELL_4V_DOT(v1, v2) \
  ((v1)[0]*(v2)[0] + (v1)[1]*(v2)[1] + (v1)[2]*(v2)[2] + (v1)[3]*(v2)[3])

#define ELL_4V_SCALE(v2, a, v1) \
  ((v2)[0] = (v1)[0]*a, (v2)[1] = (v1)[1]*a, \
   (v2)[2] = (v1)[2]*a, (v2)[3] = (v1)[3]*a)

#define ELL_4V_SCALE_ADD2(v2, s0, v0, s1, v1) \
  ((v2)[0] = (s0)*(v0)[0] + (s1)*(v1)[0],    \
   (v2)[1] = (s0)*(v0)[1] + (s1)*(v1)[1],    \
   (v2)[2] = (s0)*(v0)[2] + (s1)*(v1)[2],    \
   (v2)[3] = (s0)*(v0)[3] + (s1)*(v1)[3])

#define ELL_4V_SCALE_ADD3(v, s0, v0, s1, v1, s2, v2)    \
  ((v)[0] = (s0)*(v0)[0] + (s1)*(v1)[0] + (s2)*(v2)[0], \
   (v)[1] = (s0)*(v0)[1] + (s1)*(v1)[1] + (s2)*(v2)[1], \
   (v)[2] = (s0)*(v0)[2] + (s1)*(v1)[2] + (s2)*(v2)[2], \
   (v)[3] = (s0)*(v0)[3] + (s1)*(v1)[3] + (s2)*(v2)[3])

#define ELL_4V_SCALE_ADD4(v, s0, v0, s1, v1, s2, v2, s3, v3)           \
  ((v)[0] = (s0)*(v0)[0] + (s1)*(v1)[0] + (s2)*(v2)[0] + (s3)*(v3)[0], \
   (v)[1] = (s0)*(v0)[1] + (s1)*(v1)[1] + (s2)*(v2)[1] + (s3)*(v3)[1], \
   (v)[2] = (s0)*(v0)[2] + (s1)*(v1)[2] + (s2)*(v2)[2] + (s3)*(v3)[2], \
   (v)[3] = (s0)*(v0)[3] + (s1)*(v1)[3] + (s2)*(v2)[3] + (s3)*(v3)[3])

#define ELL_4V_SCALE_INCR(v2, s0, v0) \
  ((v2)[0] += (s0)*(v0)[0], \
   (v2)[1] += (s0)*(v0)[1], \
   (v2)[2] += (s0)*(v0)[2], \
   (v2)[3] += (s0)*(v0)[3])

#define ELL_4V_LEN(v) (sqrt(ELL_4V_DOT((v),(v))))

#define ELL_4V_NORM(v2, v1, length) \
  (length = ELL_4V_LEN(v1), ELL_4V_SCALE(v2, 1.0/length, v1))

#define ELL_4V_EXISTS(v) \
   (AIR_EXISTS((v)[0]) && AIR_EXISTS((v)[1]) \
    && AIR_EXISTS((v)[2]) && AIR_EXISTS((v)[3]))

#define ELL_4M_EQUAL(m1, m2) \
  ((m1)[ 0] == (m2)[ 0] &&   \
   (m1)[ 1] == (m2)[ 1] &&   \
   (m1)[ 2] == (m2)[ 2] &&   \
   (m1)[ 3] == (m2)[ 3] &&   \
   (m1)[ 4] == (m2)[ 4] &&   \
   (m1)[ 5] == (m2)[ 5] &&   \
   (m1)[ 6] == (m2)[ 6] &&   \
   (m1)[ 7] == (m2)[ 7] &&   \
   (m1)[ 8] == (m2)[ 8] &&   \
   (m1)[ 9] == (m2)[ 9] &&   \
   (m1)[10] == (m2)[10] &&   \
   (m1)[11] == (m2)[11] &&   \
   (m1)[12] == (m2)[12] &&   \
   (m1)[13] == (m2)[13] &&   \
   (m1)[14] == (m2)[14] &&   \
   (m1)[15] == (m2)[15])

#define ELL_4M_ADD2(m3, m1, m2)            \
  (ELL_4V_ADD2((m3)+ 0, (m1)+ 0, (m2)+ 0), \
   ELL_4V_ADD2((m3)+ 4, (m1)+ 4, (m2)+ 4), \
   ELL_4V_ADD2((m3)+ 8, (m1)+ 8, (m2)+ 8), \
   ELL_4V_ADD2((m3)+12, (m1)+12, (m2)+12))

#define ELL_4M_SUB(m3, m1, m2)            \
  (ELL_4V_SUB((m3)+ 0, (m1)+ 0, (m2)+ 0), \
   ELL_4V_SUB((m3)+ 4, (m1)+ 4, (m2)+ 4), \
   ELL_4V_SUB((m3)+ 8, (m1)+ 8, (m2)+ 8), \
   ELL_4V_SUB((m3)+12, (m1)+12, (m2)+12))

#define ELL_4M_SCALE(m2, a, m1)         \
  (ELL_4V_SCALE((m2)+ 0, (a), (m1)+ 0), \
   ELL_4V_SCALE((m2)+ 4, (a), (m1)+ 4), \
   ELL_4V_SCALE((m2)+ 8, (a), (m1)+ 8), \
   ELL_4V_SCALE((m2)+12, (a), (m1)+12))

#define ELL_4M_COPY(m2, m1)     \
  (ELL_4V_COPY((m2)+ 0, (m1)+ 0), \
   ELL_4V_COPY((m2)+ 4, (m1)+ 4), \
   ELL_4V_COPY((m2)+ 8, (m1)+ 8), \
   ELL_4V_COPY((m2)+12, (m1)+12))

#define ELL_4M_COPY_TT(m2, TT, m1)     \
  (ELL_4V_COPY_TT((m2)+ 0, TT, (m1)+ 0), \
   ELL_4V_COPY_TT((m2)+ 4, TT, (m1)+ 4), \
   ELL_4V_COPY_TT((m2)+ 8, TT, (m1)+ 8), \
   ELL_4V_COPY_TT((m2)+12, TT, (m1)+12))

#define ELL_4M_TRANSPOSE(m2, m1) \
  ((m2)[ 0] = (m1)[ 0],          \
   (m2)[ 1] = (m1)[ 4],          \
   (m2)[ 2] = (m1)[ 8],          \
   (m2)[ 3] = (m1)[12],          \
   (m2)[ 4] = (m1)[ 1],          \
   (m2)[ 5] = (m1)[ 5],          \
   (m2)[ 6] = (m1)[ 9],          \
   (m2)[ 7] = (m1)[13],          \
   (m2)[ 8] = (m1)[ 2],          \
   (m2)[ 9] = (m1)[ 6],          \
   (m2)[10] = (m1)[10],          \
   (m2)[11] = (m1)[14],          \
   (m2)[12] = (m1)[ 3],          \
   (m2)[13] = (m1)[ 7],          \
   (m2)[14] = (m1)[11],          \
   (m2)[15] = (m1)[15])

#define ELL_4M_TRANSPOSE_TT(m2, TT, m1) \
  ((m2)[ 0] = AIR_CAST(TT, (m1)[ 0]),   \
   (m2)[ 1] = AIR_CAST(TT, (m1)[ 4]),   \
   (m2)[ 2] = AIR_CAST(TT, (m1)[ 8]),   \
   (m2)[ 3] = AIR_CAST(TT, (m1)[12]),   \
   (m2)[ 4] = AIR_CAST(TT, (m1)[ 1]),   \
   (m2)[ 5] = AIR_CAST(TT, (m1)[ 5]),   \
   (m2)[ 6] = AIR_CAST(TT, (m1)[ 9]),   \
   (m2)[ 7] = AIR_CAST(TT, (m1)[13]),   \
   (m2)[ 8] = AIR_CAST(TT, (m1)[ 2]),   \
   (m2)[ 9] = AIR_CAST(TT, (m1)[ 6]),   \
   (m2)[10] = AIR_CAST(TT, (m1)[10]),   \
   (m2)[11] = AIR_CAST(TT, (m1)[14]),   \
   (m2)[12] = AIR_CAST(TT, (m1)[ 3]),   \
   (m2)[13] = AIR_CAST(TT, (m1)[ 7]),   \
   (m2)[14] = AIR_CAST(TT, (m1)[11]),   \
   (m2)[15] = AIR_CAST(TT, (m1)[15]))

#define ELL_4M_TRANSPOSE_IP(m, t)   \
  (ELL_SWAP2((m)[ 1],(m)[ 4],(t)),  \
   ELL_SWAP2((m)[ 2],(m)[ 8],(t)),  \
   ELL_SWAP2((m)[ 3],(m)[12],(t)),  \
   ELL_SWAP2((m)[ 6],(m)[ 9],(t)),  \
   ELL_SWAP2((m)[ 7],(m)[13],(t)),  \
   ELL_SWAP2((m)[11],(m)[14],(t)))

#define ELL_4MV_ROW0_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 0], (m)[ 1], (m)[ 2], (m)[ 3]))

#define ELL_4MV_ROW1_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 4], (m)[ 5], (m)[ 6], (m)[ 7]))

#define ELL_4MV_ROW2_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 8], (m)[ 9], (m)[10], (m)[11]))

#define ELL_4MV_ROW3_GET(v, m) \
  (ELL_4V_SET((v), (m)[12], (m)[13], (m)[14], (m)[15]))

#define ELL_4MV_COL0_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 0], (m)[ 4], (m)[ 8], (m)[12]))

#define ELL_4MV_COL1_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 1], (m)[ 5], (m)[ 9], (m)[13]))

#define ELL_4MV_COL2_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 2], (m)[ 6], (m)[10], (m)[14]))

#define ELL_4MV_COL3_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 3], (m)[ 7], (m)[11], (m)[15]))

#define ELL_4MV_ROW0_SET(m, v) \
  (ELL_4V_GET((m)[ 0], (m)[ 1], (m)[ 2], (m)[ 3], (v)))

#define ELL_4MV_ROW1_SET(m, v) \
  (ELL_4V_GET((m)[ 4], (m)[ 5], (m)[ 6], (m)[ 7], (v)))

#define ELL_4MV_ROW2_SET(m, v) \
  (ELL_4V_GET((m)[ 8], (m)[ 9], (m)[10], (m)[11], (v)))

#define ELL_4MV_ROW3_SET(m, v) \
  (ELL_4V_GET((m)[12], (m)[13], (m)[14], (m)[15], (v)))

#define ELL_4MV_COL0_SET(m, v) \
  (ELL_4V_GET((m)[ 0], (m)[ 4], (m)[ 8], (m)[12], (v)))

#define ELL_4MV_COL1_SET(m, v) \
  (ELL_4V_GET((m)[ 1], (m)[ 5], (m)[ 9], (m)[13], (v)))

#define ELL_4MV_COL2_SET(m, v) \
  (ELL_4V_GET((m)[ 2], (m)[ 6], (m)[10], (m)[14], (v)))

#define ELL_4MV_COL3_SET(m, v) \
  (ELL_4V_GET((m)[ 3], (m)[ 7], (m)[11], (m)[15], (v)))

#define ELL_4MV_MUL(v2, m, v1)                                              \
  ((v2)[0]=(m)[ 0]*(v1)[0]+(m)[ 1]*(v1)[1]+(m)[ 2]*(v1)[2]+(m)[ 3]*(v1)[3], \
   (v2)[1]=(m)[ 4]*(v1)[0]+(m)[ 5]*(v1)[1]+(m)[ 6]*(v1)[2]+(m)[ 7]*(v1)[3], \
   (v2)[2]=(m)[ 8]*(v1)[0]+(m)[ 9]*(v1)[1]+(m)[10]*(v1)[2]+(m)[11]*(v1)[3], \
   (v2)[3]=(m)[12]*(v1)[0]+(m)[13]*(v1)[1]+(m)[14]*(v1)[2]+(m)[15]*(v1)[3])

#define ELL_4MV_MUL_TT(v2, TT, m, v1)                       \
  ((v2)[0]=AIR_CAST(TT, ((m)[ 0]*(v1)[0]+(m)[ 1]*(v1)[1]    \
                        +(m)[ 2]*(v1)[2]+(m)[ 3]*(v1)[3])), \
   (v2)[1]=AIR_CAST(TT, ((m)[ 4]*(v1)[0]+(m)[ 5]*(v1)[1]    \
                        +(m)[ 6]*(v1)[2]+(m)[ 7]*(v1)[3])), \
   (v2)[2]=AIR_CAST(TT, ((m)[ 8]*(v1)[0]+(m)[ 9]*(v1)[1]    \
                        +(m)[10]*(v1)[2]+(m)[11]*(v1)[3])), \
   (v2)[3]=AIR_CAST(TT, ((m)[12]*(v1)[0]+(m)[13]*(v1)[1]    \
                        +(m)[14]*(v1)[2]+(m)[15]*(v1)[3])))
   
#define ELL_4MV_TMUL(v2, m, v1)                                             \
  ((v2)[0]=(m)[ 0]*(v1)[0]+(m)[ 4]*(v1)[1]+(m)[ 8]*(v1)[2]+(m)[12]*(v1)[3], \
   (v2)[1]=(m)[ 1]*(v1)[0]+(m)[ 5]*(v1)[1]+(m)[ 9]*(v1)[2]+(m)[13]*(v1)[3], \
   (v2)[2]=(m)[ 2]*(v1)[0]+(m)[ 6]*(v1)[1]+(m)[10]*(v1)[2]+(m)[14]*(v1)[3], \
   (v2)[3]=(m)[ 3]*(v1)[0]+(m)[ 7]*(v1)[1]+(m)[11]*(v1)[2]+(m)[15]*(v1)[3])

#define ELL_34V_HOMOG(v2, v1) \
  ((v2)[0] = (v1)[0]/(v1)[3], \
   (v2)[1] = (v1)[1]/(v1)[3], \
   (v2)[2] = (v1)[2]/(v1)[3])

#define ELL_34V_HOMOG_TT(v2, TT, v1) \
  ((v2)[0] = AIR_CAST(TT, (v1)[0]/(v1)[3]), \
   (v2)[1] = AIR_CAST(TT, (v1)[1]/(v1)[3]), \
   (v2)[2] = AIR_CAST(TT, (v1)[2]/(v1)[3]))

#define ELL_4V_HOMOG(v2, v1)  \
  ((v2)[0] = (v1)[0]/(v1)[3], \
   (v2)[1] = (v1)[1]/(v1)[3], \
   (v2)[2] = (v1)[2]/(v1)[3], \
   (v2)[3] = 1.0)

/*
** These macros are intended to be used as aids with homogeneous transforms
*/

#define ELL_4M_COLS_SET(m, a, b, c, d)                 \
  (ELL_4V_SET((m)+ 0, (a)[0], (b)[0], (c)[0], (d)[0]), \
   ELL_4V_SET((m)+ 4, (a)[1], (b)[1], (c)[1], (d)[1]), \
   ELL_4V_SET((m)+ 8, (a)[2], (b)[2], (c)[2], (d)[2]), \
   ELL_4V_SET((m)+12, (a)[3], (b)[3], (c)[3], (d)[3]))

#define ELL_4M_ROWS_SET(m, a, b, c, d)  \
  (ELL_4V_COPY((m)+ 0, a),              \
   ELL_4V_COPY((m)+ 4, b),              \
   ELL_4V_COPY((m)+ 8, c),              \
   ELL_4V_COPY((m)+12, d))

#define ELL_4M_IDENTITY_SET(m) \
  (ELL_4V_SET((m)+ 0,  1 ,  0 ,  0 , 0), \
   ELL_4V_SET((m)+ 4,  0 ,  1 ,  0 , 0), \
   ELL_4V_SET((m)+ 8,  0 ,  0 ,  1 , 0), \
   ELL_4V_SET((m)+12,  0 ,  0 ,  0 , 1))

#define ELL_4M_ZERO_SET(m) \
  (ELL_4V_SET((m)+ 0,  0 ,  0 ,  0 , 0), \
   ELL_4V_SET((m)+ 4,  0 ,  0 ,  0 , 0), \
   ELL_4V_SET((m)+ 8,  0 ,  0 ,  0 , 0), \
   ELL_4V_SET((m)+12,  0 ,  0 ,  0 , 0))

#define ELL_4M_SCALE_SET(m, x, y, z)     \
  (ELL_4V_SET((m)+ 0, (x),  0 ,  0 , 0), \
   ELL_4V_SET((m)+ 4,  0 , (y),  0 , 0), \
   ELL_4V_SET((m)+ 8,  0 ,  0 , (z), 0), \
   ELL_4V_SET((m)+12,  0 ,  0 ,  0 , 1))

#define ELL_4M_TRANSLATE_SET(m, x, y, z) \
  (ELL_4V_SET((m)+ 0,  1 ,  0 ,  0 , (x)), \
   ELL_4V_SET((m)+ 4,  0 ,  1 ,  0 , (y)), \
   ELL_4V_SET((m)+ 8,  0 ,  0 ,  1 , (z)), \
   ELL_4V_SET((m)+12,  0 ,  0 ,  0 ,  1))

#define ELL_4M_ROTATE_X_SET(m, th)                   \
  (ELL_4V_SET((m)+ 0,  1 ,     0    ,     0    , 0), \
   ELL_4V_SET((m)+ 4,  0 ,  cos(th) , -sin(th) , 0), \
   ELL_4V_SET((m)+ 8,  0 , +sin(th) ,  cos(th) , 0), \
   ELL_4V_SET((m)+12,  0 ,     0    ,     0    , 1))

#define ELL_4M_ROTATE_Y_SET(m, th)                   \
  (ELL_4V_SET((m)+ 0,  cos(th) ,  0 , +sin(th) , 0), \
   ELL_4V_SET((m)+ 4,     0    ,  1 ,     0    , 0), \
   ELL_4V_SET((m)+ 8, -sin(th) ,  0 ,  cos(th) , 0), \
   ELL_4V_SET((m)+12,     0    ,  0 ,     0    , 1))

#define ELL_4M_ROTATE_Z_SET(m, th)                   \
  (ELL_4V_SET((m)+ 0,  cos(th) , -sin(th) ,  0 , 0), \
   ELL_4V_SET((m)+ 4, +sin(th) ,  cos(th) ,  0 , 0), \
   ELL_4V_SET((m)+ 8,     0    ,     0    ,  1 , 0), \
   ELL_4V_SET((m)+12,     0    ,     0    ,  0 , 1))

#define ELL_4M_MUL(n, l, m)                                                 \
  ((n)[ 0]=(l)[ 0]*(m)[ 0]+(l)[ 1]*(m)[ 4]+(l)[ 2]*(m)[ 8]+(l)[ 3]*(m)[12], \
   (n)[ 1]=(l)[ 0]*(m)[ 1]+(l)[ 1]*(m)[ 5]+(l)[ 2]*(m)[ 9]+(l)[ 3]*(m)[13], \
   (n)[ 2]=(l)[ 0]*(m)[ 2]+(l)[ 1]*(m)[ 6]+(l)[ 2]*(m)[10]+(l)[ 3]*(m)[14], \
   (n)[ 3]=(l)[ 0]*(m)[ 3]+(l)[ 1]*(m)[ 7]+(l)[ 2]*(m)[11]+(l)[ 3]*(m)[15], \
                                                                            \
   (n)[ 4]=(l)[ 4]*(m)[ 0]+(l)[ 5]*(m)[ 4]+(l)[ 6]*(m)[ 8]+(l)[ 7]*(m)[12], \
   (n)[ 5]=(l)[ 4]*(m)[ 1]+(l)[ 5]*(m)[ 5]+(l)[ 6]*(m)[ 9]+(l)[ 7]*(m)[13], \
   (n)[ 6]=(l)[ 4]*(m)[ 2]+(l)[ 5]*(m)[ 6]+(l)[ 6]*(m)[10]+(l)[ 7]*(m)[14], \
   (n)[ 7]=(l)[ 4]*(m)[ 3]+(l)[ 5]*(m)[ 7]+(l)[ 6]*(m)[11]+(l)[ 7]*(m)[15], \
                                                                            \
   (n)[ 8]=(l)[ 8]*(m)[ 0]+(l)[ 9]*(m)[ 4]+(l)[10]*(m)[ 8]+(l)[11]*(m)[12], \
   (n)[ 9]=(l)[ 8]*(m)[ 1]+(l)[ 9]*(m)[ 5]+(l)[10]*(m)[ 9]+(l)[11]*(m)[13], \
   (n)[10]=(l)[ 8]*(m)[ 2]+(l)[ 9]*(m)[ 6]+(l)[10]*(m)[10]+(l)[11]*(m)[14], \
   (n)[11]=(l)[ 8]*(m)[ 3]+(l)[ 9]*(m)[ 7]+(l)[10]*(m)[11]+(l)[11]*(m)[15], \
                                                                            \
   (n)[12]=(l)[12]*(m)[ 0]+(l)[13]*(m)[ 4]+(l)[14]*(m)[ 8]+(l)[15]*(m)[12], \
   (n)[13]=(l)[12]*(m)[ 1]+(l)[13]*(m)[ 5]+(l)[14]*(m)[ 9]+(l)[15]*(m)[13], \
   (n)[14]=(l)[12]*(m)[ 2]+(l)[13]*(m)[ 6]+(l)[14]*(m)[10]+(l)[15]*(m)[14], \
   (n)[15]=(l)[12]*(m)[ 3]+(l)[13]*(m)[ 7]+(l)[14]*(m)[11]+(l)[15]*(m)[15])

#define ELL_34M_EXTRACT(m, l) \
  ((m)[0] = (l)[ 0], (m)[1] = (l)[ 1], (m)[2] = (l)[ 2], \
   (m)[3] = (l)[ 4], (m)[4] = (l)[ 5], (m)[5] = (l)[ 6], \
   (m)[6] = (l)[ 8], (m)[7] = (l)[ 9], (m)[8] = (l)[10])

#define ELL_43M_INSET(l, m) \
  ((l)[ 0] = (m)[0], (l)[ 1] = (m)[1], (l)[ 2] = (m)[2], (l)[ 3] = 0, \
   (l)[ 4] = (m)[3], (l)[ 5] = (m)[4], (l)[ 6] = (m)[5], (l)[ 7] = 0, \
   (l)[ 8] = (m)[6], (l)[ 9] = (m)[7], (l)[10] = (m)[8], (l)[11] = 0, \
   (l)[12] =   0   , (l)[13] =   0   , (l)[14] =   0   , (l)[15] = 1)

#define ELL_4M_FROB(m) \
  (sqrt(ELL_4V_DOT((m)+ 0, (m)+ 0) + \
        ELL_4V_DOT((m)+ 4, (m)+ 4) + \
        ELL_4V_DOT((m)+ 8, (m)+ 8) + \
        ELL_4V_DOT((m)+12, (m)+12)))

#define ELL_4M_DET(m) \
  (  (m)[ 0] * _ELL_3M_DET((m)[ 5], (m)[ 6], (m)[ 7], \
                           (m)[ 9], (m)[10], (m)[11], \
                           (m)[13], (m)[14], (m)[15]) \
   - (m)[ 1] * _ELL_3M_DET((m)[ 4], (m)[ 6], (m)[ 7], \
                           (m)[ 8], (m)[10], (m)[11], \
                           (m)[12], (m)[14], (m)[15]) \
   + (m)[ 2] * _ELL_3M_DET((m)[ 4], (m)[ 5], (m)[ 7], \
                           (m)[ 8], (m)[ 9], (m)[11], \
                           (m)[12], (m)[13], (m)[15]) \
   - (m)[ 3] * _ELL_3M_DET((m)[ 4], (m)[ 5], (m)[ 6], \
                           (m)[ 8], (m)[ 9], (m)[10], \
                           (m)[12], (m)[13], (m)[14]))

#define ELL_Q_MUL(q3, q1, q2)                                            \
  ELL_4V_SET((q3),                                                       \
  (q1)[0]*(q2)[0] - (q1)[1]*(q2)[1] - (q1)[2]*(q2)[2] - (q1)[3]*(q2)[3], \
  (q1)[0]*(q2)[1] + (q1)[1]*(q2)[0] + (q1)[2]*(q2)[3] - (q1)[3]*(q2)[2], \
  (q1)[0]*(q2)[2] - (q1)[1]*(q2)[3] + (q1)[2]*(q2)[0] + (q1)[3]*(q2)[1], \
  (q1)[0]*(q2)[3] + (q1)[1]*(q2)[2] - (q1)[2]*(q2)[1] + (q1)[3]*(q2)[0])

#define ELL_Q_CONJ(q2, q1) \
  ELL_4V_SET((q2), (q1)[0], -(q1)[1], -(q1)[2], -(q1)[3])

#define ELL_Q_INV(i, q, n)                                            \
 (n = ELL_4V_DOT(q, q),                                               \
  ELL_4V_SET((i), (q)[0]/(n), -(q)[1]/(n), -(q)[2]/(n), -(q)[3]/(n)))

#define ELL_Q_TO_3M(m, q)                                                   \
 (ELL_3V_SET((m)+0,                                                         \
             (q)[0]*(q)[0] + (q)[1]*(q)[1] - (q)[2]*(q)[2] - (q)[3]*(q)[3], \
             2*((q)[1]*(q)[2] - (q)[0]*(q)[3]),                             \
             2*((q)[1]*(q)[3] + (q)[0]*(q)[2])),                            \
  ELL_3V_SET((m)+3,                                                         \
             2*((q)[1]*(q)[2] + (q)[0]*(q)[3]),                             \
             (q)[0]*(q)[0] - (q)[1]*(q)[1] + (q)[2]*(q)[2] - (q)[3]*(q)[3], \
             2*((q)[2]*(q)[3] - (q)[0]*(q)[1])),                            \
  ELL_3V_SET((m)+6,                                                         \
             2*((q)[1]*(q)[3] - (q)[0]*(q)[2]),                             \
             2*((q)[2]*(q)[3] + (q)[0]*(q)[1]),                             \
             (q)[0]*(q)[0] - (q)[1]*(q)[1] - (q)[2]*(q)[2] + (q)[3]*(q)[3]))

#define ELL_Q_TO_4M(m, q)                                                   \
 (ELL_4V_SET((m)+0,                                                         \
             (q)[0]*(q)[0] + (q)[1]*(q)[1] - (q)[2]*(q)[2] - (q)[3]*(q)[3], \
             2*((q)[1]*(q)[2] - (q)[0]*(q)[3]),                             \
             2*((q)[1]*(q)[3] + (q)[0]*(q)[2]),                             \
             0),                                                            \
  ELL_4V_SET((m)+4,                                                         \
             2*((q)[1]*(q)[2] + (q)[0]*(q)[3]),                             \
             (q)[0]*(q)[0] - (q)[1]*(q)[1] + (q)[2]*(q)[2] - (q)[3]*(q)[3], \
             2*((q)[2]*(q)[3] - (q)[0]*(q)[1]),                             \
             0),                                                            \
  ELL_4V_SET((m)+8,                                                         \
             2*((q)[1]*(q)[3] - (q)[0]*(q)[2]),                             \
             2*((q)[2]*(q)[3] + (q)[0]*(q)[1]),                             \
             (q)[0]*(q)[0] - (q)[1]*(q)[1] - (q)[2]*(q)[2] + (q)[3]*(q)[3], \
             0),                                                            \
  ELL_4V_SET((m)+12, 0, 0, 0, 1))

#define ELL_5V_SET(v, a, b, c, d, e) \
  ((v)[0]=(a), (v)[1]=(b), (v)[2]=(c), (v)[3]=(d), (v)[4]=(e))

#define ELL_5V_COPY(v, w) \
  ((v)[0]=(w)[0], (v)[1]=(w)[1], (v)[2]=(w)[2], (v)[3]=(w)[3], (v)[4]=(w)[4])

#define ELL_6V_SET(v, a, b, c, d, e, f) \
  ((v)[0]=(a), (v)[1]=(b), (v)[2]=(c), (v)[3]=(d), (v)[4]=(e), (v)[5]=(f))

#define ELL_6V_COPY(v2, v1) \
  ((v2)[0] = (v1)[0],       \
   (v2)[1] = (v1)[1],       \
   (v2)[2] = (v1)[2],       \
   (v2)[3] = (v1)[3],       \
   (v2)[4] = (v1)[4],       \
   (v2)[5] = (v1)[5])       \

#define ELL_6V_SCALE_INCR2(v2, s0, v0, s1, v1) \
  ((v2)[0] += (s0)*(v0)[0] + (s1)*(v1)[0],     \
   (v2)[1] += (s0)*(v0)[1] + (s1)*(v1)[1],     \
   (v2)[2] += (s0)*(v0)[2] + (s1)*(v1)[2],     \
   (v2)[3] += (s0)*(v0)[3] + (s1)*(v1)[3],     \
   (v2)[4] += (s0)*(v0)[4] + (s1)*(v1)[4],     \
   (v2)[5] += (s0)*(v0)[5] + (s1)*(v1)[5])

#define ELL_6V_SCALE_INCR(v2, a, v1) \
  ((v2)[0] += (a)*(v1)[0],       \
   (v2)[1] += (a)*(v1)[1],       \
   (v2)[2] += (a)*(v1)[2],       \
   (v2)[3] += (a)*(v1)[3],       \
   (v2)[4] += (a)*(v1)[4],       \
   (v2)[5] += (a)*(v1)[5])

#define ELL_6V_SCALE(v2, a, v1) \
  ((v2)[0] = (a)*(v1)[0],       \
   (v2)[1] = (a)*(v1)[1],       \
   (v2)[2] = (a)*(v1)[2],       \
   (v2)[3] = (a)*(v1)[3],       \
   (v2)[4] = (a)*(v1)[4],       \
   (v2)[5] = (a)*(v1)[5])

#define ELL_6V_DOT(v1, v2) \
  ((v1)[0]*(v2)[0] +       \
   (v1)[1]*(v2)[1] +       \
   (v1)[2]*(v2)[2] +       \
   (v1)[3]*(v2)[3] +       \
   (v1)[4]*(v2)[4] +       \
   (v1)[5]*(v2)[5])

#define ELL_9V_SET(v, a, b, c, d, e, f, g, h, i) \
  ((v)[0]=(a), (v)[1]=(b), (v)[2]=(c), \
   (v)[3]=(d), (v)[4]=(e), (v)[5]=(f), \
   (v)[6]=(g), (v)[7]=(h), (v)[8]=(i))

#define ELL_9V_COPY(v2, v1) \
  ((v2)[0] = (v1)[0],       \
   (v2)[1] = (v1)[1],       \
   (v2)[2] = (v1)[2],       \
   (v2)[3] = (v1)[3],       \
   (v2)[4] = (v1)[4],       \
   (v2)[5] = (v1)[5],       \
   (v2)[6] = (v1)[6],       \
   (v2)[7] = (v1)[7],       \
   (v2)[8] = (v1)[8])

#define ELL_9V_DOT(v1, v2) \
  ((v1)[0]*(v2)[0] +       \
   (v1)[1]*(v2)[1] +       \
   (v1)[2]*(v2)[2] +       \
   (v1)[3]*(v2)[3] +       \
   (v1)[4]*(v2)[4] +       \
   (v1)[5]*(v2)[5] +       \
   (v1)[6]*(v2)[6] +       \
   (v1)[7]*(v2)[7] +       \
   (v1)[8]*(v2)[8])

#ifdef __cplusplus
   }
#endif

#endif /* ELLMACROS_HAS_BEEN_INCLUDED */
