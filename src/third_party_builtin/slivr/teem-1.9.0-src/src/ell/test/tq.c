/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
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


#include <stdio.h>
#include "../ell.h"

char *me;

#define CA 1

int
main(int argc, char *argv[]) {
  float angleA_f, axisA_f[3], angleB_f, axisB_f[3],
    qA_f[4], qB_f[4], qC_f[4],
    mat3A_f[9], mat4A_f[16], mat3B_f[9], mat4B_f[16], mat3C_f[9], mat4C_f[16],
    pntA_f[4], pntB_f[4], pntC_f[4];
  double angleA_d, axisA_d[3], angleB_d, axisB_d[3],
    qA_d[4], qB_d[4], qC_d[4],
    mat3A_d[9], mat4A_d[16], mat3B_d[9], mat4B_d[16], mat3C_d[9], mat4C_d[16],
    pntA_d[4], pntB_d[4], pntC_d[4];

  int I, N;
  double tmp, det, frob;

  me = argv[0];
  N = 100000;

  for (I=0; I<N; I++) {
    /* make a rotation (as a quaternion) */
    ELL_3V_SET(axisA_f, 2*airDrandMT()-1, 2*airDrandMT()-1, 2*airDrandMT()-1);
    ELL_3V_NORM(axisA_f, axisA_f, tmp); /* yea, not uniform, so what */
    angleA_f = AIR_PI*(2*airDrandMT()-1);
    ell_aa_to_q_f(qA_f, angleA_f, axisA_f);

    /* convert to AA and back, and back */
    angleB_f = ell_q_to_aa_f(axisB_f, qA_f);
    if (ELL_3V_DOT(axisB_f, axisA_f) < 0) {
      ELL_3V_SCALE(axisB_f, -1, axisB_f);
      angleB_f *= -1;
    }
    ELL_3V_SUB(axisA_f, axisA_f, axisB_f);
    printf(" aa -> q -> aa error: %g, %g\n",
           CA + AIR_ABS(angleA_f - angleB_f), CA + ELL_3V_LEN(axisA_f));

    /* convert to 3m and back, and back */
    ell_q_to_3m_f(mat3A_f, qA_f);
    ell_3m_to_q_f(qB_f, mat3A_f);
    if (ELL_4V_DOT(qA_f, qB_f) < 0) {
      ELL_4V_SCALE(qB_f, -1, qB_f);
    }
    ELL_4V_SUB(qC_f, qA_f, qB_f);
    ELL_Q_TO_3M(mat3B_f, qA_f);
    ELL_3M_SUB(mat3C_f, mat3B_f, mat3A_f);
    printf(" q -> 3m -> q error: %g, %g\n",
           CA + ELL_4V_LEN(qC_f), CA + ELL_3M_FROB(mat3C_f));

    /* convert to 4m and back, and back */
    ell_q_to_4m_f(mat4A_f, qA_f);
    ell_4m_to_q_f(qB_f, mat4A_f);
    if (ELL_4V_DOT(qA_f, qB_f) < 0) {
      ELL_4V_SCALE(qB_f, -1, qB_f);
    }
    ELL_4V_SUB(qC_f, qA_f, qB_f);
    ELL_Q_TO_4M(mat4B_f, qA_f);
    ELL_4M_SUB(mat4C_f, mat4B_f, mat4A_f);
    printf(" q -> 4m -> q error: %g, %g\n",
           CA + ELL_4V_LEN(qC_f), CA + ELL_4M_FROB(mat4C_f));

    /* make a point that we'll rotate */
    ELL_3V_SET(pntA_f, 2*airDrandMT()-1, 2*airDrandMT()-1, 2*airDrandMT()-1);
    
    /* effect rotation in two different ways, and compare results */
    ELL_3MV_MUL(pntB_f, mat3A_f, pntA_f);
    ell_q_3v_rotate_f(pntC_f, qA_f, pntA_f);
    ELL_3V_SUB(pntA_f, pntB_f, pntC_f);
    printf("      rotation error = %g\n", CA + ELL_3V_LEN(pntA_f));

    /* mix up inversion with conversion */
    ell_3m_inv_f(mat3C_f, mat3A_f);
    ell_3m_to_q_f(qB_f, mat3C_f);
    ell_q_mul_f(qC_f, qA_f, qB_f);
    if (ELL_4V_DOT(qA_f, qC_f) < 0) {
      ELL_4V_SCALE(qC_f, -1, qC_f);
    }
    printf("    inv mul = %g %g %g %g\n", qC_f[0], 
           CA + qC_f[1], CA + qC_f[2], CA + qC_f[3]);
    ell_q_inv_f(qC_f, qB_f);
    ELL_4V_SUB(qC_f, qB_f, qB_f);
    printf("    inv diff = %g %g %g %g\n", CA + qC_f[0], 
           CA + qC_f[1], CA + qC_f[2], CA + qC_f[3]);

    /* exp and log */
    ell_q_log_f(qC_f, qA_f);
    ell_q_log_f(qB_f, qC_f);
    ell_q_exp_f(qC_f, qB_f);
    ell_q_exp_f(qB_f, qC_f);
    ELL_4V_SUB(qC_f, qB_f, qA_f);
    printf("    exp/log diff = %g %g %g %g\n", CA + qC_f[0], 
           CA + qC_f[1], CA + qC_f[2], CA + qC_f[3]);

    /* pow, not very exhaustive */
    ell_q_to_3m_f(mat3A_f, qA_f);
    ell_3m_post_mul_f(mat3A_f, mat3A_f);
    ell_3m_post_mul_f(mat3A_f, mat3A_f);
    ell_q_pow_f(qB_f, qA_f, 4);
    ell_q_to_3m_f(mat3B_f, qB_f);
    ELL_3M_SUB(mat3B_f, mat3B_f, mat3A_f);
    printf("   pow diff = %g\n", CA + ELL_3M_FROB(mat3B_f));
    if (ELL_3M_FROB(mat3B_f) > 2) {
      printf("  start q = %g %g %g %g\n", qA_f[0], qA_f[1], qA_f[2], qA_f[3]);
      angleA_f = ell_q_to_aa_f(axisA_f, qA_f);
      printf("  --> aa = %g  (%g %g %g)\n", angleA_f, 
             axisA_f[0], axisA_f[1], axisA_f[2]);
      printf("   q^3 = %g %g %g %g\n", qB_f[0], qB_f[1], qB_f[2], qB_f[3]);
      angleA_f = ell_q_to_aa_f(axisA_f, qB_f);
      printf("  --> aa = %g  (%g %g %g)\n", angleA_f, 
             axisA_f[0], axisA_f[1], axisA_f[2]);
      exit(1);
    }

    /* make sure it looks like a rotation matrix */
    ell_q_to_3m_f(mat3A_f, qA_f);
    det = ELL_3M_DET(mat3A_f);
    frob = ELL_3M_FROB(mat3A_f);
    ELL_3M_TRANSPOSE(mat3B_f, mat3A_f);
    ell_3m_inv_f(mat3C_f, mat3A_f);
    ELL_3M_SUB(mat3C_f, mat3B_f, mat3C_f);
    printf("      det = %g; size = %g; err = %g\n", det, frob*frob/3,
           CA + ELL_3M_FROB(mat3C_f));
    
  }

  exit(0);
}
