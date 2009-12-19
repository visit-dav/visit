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

void
ell_3m_mul_f(float m3[9], float _m1[9], float _m2[9]) {
  float m1[9], m2[9];

  ELL_3M_COPY(m1, _m1);
  ELL_3M_COPY(m2, _m2);
  ELL_3M_MUL(m3, m1, m2);
}

void
ell_3m_mul_d(double m3[9], double _m1[9], double _m2[9]) {
  double m1[9], m2[9];

  ELL_3M_COPY(m1, _m1);
  ELL_3M_COPY(m2, _m2);
  ELL_3M_MUL(m3, m1, m2);
}

void
ell_3m_pre_mul_f(float _m[9], float x[9]) {
  float m[9];
  ELL_3M_MUL(m, _m, x);
  ELL_3M_COPY(_m, m);
}

void
ell_3m_pre_mul_d(double _m[9], double x[9]) {
  double m[9];
  ELL_3M_MUL(m, _m, x);
  ELL_3M_COPY(_m, m);
}

void
ell_3m_post_mul_f(float _m[9], float x[9]) {
  float m[9];
  ELL_3M_MUL(m, x, _m);
  ELL_3M_COPY(_m, m);
}

void
ell_3m_post_mul_d(double _m[9], double x[9]) {
  double m[9];
  ELL_3M_MUL(m, x, _m);
  ELL_3M_COPY(_m, m);
}

float
ell_3m_det_f(float m[9]) {
  return ELL_3M_DET(m);
}

double
ell_3m_det_d(double m[9]) {
  return ELL_3M_DET(m);
}

void
ell_3m_inv_f(float i[9], float m[9]) {
  float det;

  ELL_3M_INV(i, m, det);
}

void
ell_3m_inv_d(double i[9], double m[9]) {
  double det;

  ELL_3M_INV(i, m, det);
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

void
ell_4m_mul_f(float m3[16], float m1[16], float m2[16]) {
  ELL_4M_MUL(m3, m1, m2);
}

void
ell_4m_mul_d(double m3[16], double m1[16], double m2[16]) {
  ELL_4M_MUL(m3, m1, m2);
}

void
ell_4m_pre_mul_f(float _m[16], float x[16]) {
  float m[16];
  ELL_4M_MUL(m, _m, x);
  ELL_4M_COPY(_m, m);
}

void
ell_4m_pre_mMul_d(double _m[16], double x[16]) {
  double m[16];
  ELL_4M_MUL(m, _m, x);
  ELL_4M_COPY(_m, m);
}

void
ell_4m_post_mul_f(float _m[16], float x[16]) {
  float m[16];
  ELL_4M_MUL(m, x, _m);
  ELL_4M_COPY(_m, m);
}

void
ell_4m_post_mul_d(double _m[16], double x[16]) {
  double m[16];
  ELL_4M_MUL(m, x, _m);
  ELL_4M_COPY(_m, m);
}

float
ell_4m_det_f(float m[16]) {
  return ELL_4M_DET(m);
}

double
ell_4m_det_d(double m[16]) {
  return ELL_4M_DET(m);
}

#define _4INV \
  det = ELL_4M_DET(m); \
  i[ 0] =  _ELL_3M_DET((m)[ 5],(m)[ 6],(m)[ 7], \
                       (m)[ 9],(m)[10],(m)[11], \
                       (m)[13],(m)[14],(m)[15])/det; \
  i[ 1] = -_ELL_3M_DET((m)[ 1],(m)[ 2],(m)[ 3], \
                       (m)[ 9],(m)[10],(m)[11], \
                       (m)[13],(m)[14],(m)[15])/det; \
  i[ 2] =  _ELL_3M_DET((m)[ 1],(m)[ 2],(m)[ 3], \
                       (m)[ 5],(m)[ 6],(m)[ 7], \
                       (m)[13],(m)[14],(m)[15])/det; \
  i[ 3] = -_ELL_3M_DET((m)[ 1],(m)[ 2],(m)[ 3], \
                       (m)[ 5],(m)[ 6],(m)[ 7], \
                       (m)[ 9],(m)[10],(m)[11])/det; \
  i[ 4] = -_ELL_3M_DET((m)[ 4],(m)[ 6],(m)[ 7], \
                       (m)[ 8],(m)[10],(m)[11], \
                       (m)[12],(m)[14],(m)[15])/det; \
  i[ 5] =  _ELL_3M_DET((m)[ 0],(m)[ 2],(m)[ 3], \
                       (m)[ 8],(m)[10],(m)[11], \
                       (m)[12],(m)[14],(m)[15])/det; \
  i[ 6] = -_ELL_3M_DET((m)[ 0],(m)[ 2],(m)[ 3], \
                       (m)[ 4],(m)[ 6],(m)[ 7], \
                       (m)[12],(m)[14],(m)[15])/det; \
  i[ 7] =  _ELL_3M_DET((m)[ 0],(m)[ 2],(m)[ 3], \
                       (m)[ 4],(m)[ 6],(m)[ 7], \
                       (m)[ 8],(m)[10],(m)[11])/det; \
  i[ 8] =  _ELL_3M_DET((m)[ 4],(m)[ 5],(m)[ 7], \
                       (m)[ 8],(m)[ 9],(m)[11], \
                       (m)[12],(m)[13],(m)[15])/det; \
  i[ 9] = -_ELL_3M_DET((m)[ 0],(m)[ 1],(m)[ 3], \
                       (m)[ 8],(m)[ 9],(m)[11], \
                       (m)[12],(m)[13],(m)[15])/det; \
  i[10] =  _ELL_3M_DET((m)[ 0],(m)[ 1],(m)[ 3], \
                       (m)[ 4],(m)[ 5],(m)[ 7], \
                       (m)[12],(m)[13],(m)[15])/det; \
  i[11] = -_ELL_3M_DET((m)[ 0],(m)[ 1],(m)[ 3], \
                       (m)[ 4],(m)[ 5],(m)[ 7], \
                       (m)[ 8],(m)[ 9],(m)[11])/det; \
  i[12] = -_ELL_3M_DET((m)[ 4],(m)[ 5],(m)[ 6], \
                       (m)[ 8],(m)[ 9],(m)[10], \
                       (m)[12],(m)[13],(m)[14])/det; \
  i[13] =  _ELL_3M_DET((m)[ 0],(m)[ 1],(m)[ 2], \
                       (m)[ 8],(m)[ 9],(m)[10], \
                       (m)[12],(m)[13],(m)[14])/det; \
  i[14] = -_ELL_3M_DET((m)[ 0],(m)[ 1],(m)[ 2], \
                       (m)[ 4],(m)[ 5],(m)[ 6], \
                       (m)[12],(m)[13],(m)[14])/det; \
  i[15] =  _ELL_3M_DET((m)[ 0],(m)[ 1],(m)[ 2], \
                       (m)[ 4],(m)[ 5],(m)[ 6], \
                       (m)[ 8],(m)[ 9],(m)[10])/det

void
ell_4m_inv_f(float i[16], float m[16]) {
  float det;

  _4INV;
}

void
ell_4m_inv_d(double i[16], double m[16]) {
  double det;

  _4INV;
}

