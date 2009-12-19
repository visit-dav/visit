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


#include "ten.h"
#include "privateTen.h"

int tenVerbose = 0;

/*
******** tenTensorCheck()
**
** describes if the given nrrd could be a diffusion tensor dataset,
** either the measured DWI data or the calculated tensor data.
**
** We've been using 7 floats for BOTH kinds of tensor data- both the
** measured DWI and the calculated tensor matrices.  The measured data
** comes as one anatomical image and 6 DWIs.  For the calculated tensors,
** in addition to the 6 matrix components, we keep a "threshold" value
** which is based on the sum of all the DWIs, which describes if the
** calculated tensor means anything or not.
** 
** useBiff controls if biff is used to describe the problem
*/
int
tenTensorCheck(const Nrrd *nin, int wantType, int want4D, int useBiff) {
  char me[]="tenTensorCheck", err[256];
  
  if (!nin) {
    sprintf(err, "%s: got NULL pointer", me);
    if (useBiff) biffAdd(TEN, err); return 1;
  }
  if (wantType) {
    if (nin->type != wantType) {
      sprintf(err, "%s: wanted type %s, got type %s", me,
              airEnumStr(nrrdType, wantType),
              airEnumStr(nrrdType, nin->type));
      if (useBiff) biffAdd(TEN, err); return 1;
    }
  }
  else {
    if (!(nin->type == nrrdTypeFloat || nin->type == nrrdTypeShort)) {
      sprintf(err, "%s: need data of type float or short", me);
      if (useBiff) biffAdd(TEN, err); return 1;
    }
  }
  if (want4D && !(4 == nin->dim)) {
    sprintf(err, "%s: given dimension is %d, not 4", me, nin->dim);
    if (useBiff) biffAdd(TEN, err); return 1;
  }
  if (!(7 == nin->axis[0].size)) {
    sprintf(err, "%s: axis 0 has size " _AIR_SIZE_T_CNV ", not 7", 
            me, nin->axis[0].size);
    if (useBiff) biffAdd(TEN, err); return 1;
  }
  return 0;
}

int
tenMeasurementFrameReduce(Nrrd *nout, const Nrrd *nin) {
  char me[]="tenMeasurementFrameReduce", err[BIFF_STRLEN];
  double MF[9], MFT[9], tenMeasr[9], tenWorld[9];
  float *tdata;
  size_t ii, nn;
  unsigned int si, sj;
  
  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
    sprintf(err, "%s: ", me);
    biffAdd(TEN, err); return 1;
  }
  if (3 != nin->spaceDim) {
    sprintf(err, "%s: input nrrd needs 3-D (not %u-D) space dimension",
            me, nin->spaceDim);
    biffAdd(TEN, err); return 1;
  }
  /*
   [0]  [1]  [2]     [0][0]   [1][0]   [2][0]
   [3]  [4]  [5]  =  [0][1]   [1][1]   [2][1]
   [6]  [7]  [8]     [0][2]   [1][2]   [2][2]
  */
  MF[0] = nin->measurementFrame[0][0];
  MF[1] = nin->measurementFrame[1][0];
  MF[2] = nin->measurementFrame[2][0];
  MF[3] = nin->measurementFrame[0][1];
  MF[4] = nin->measurementFrame[1][1];
  MF[5] = nin->measurementFrame[2][1];
  MF[6] = nin->measurementFrame[0][2];
  MF[7] = nin->measurementFrame[1][2];
  MF[8] = nin->measurementFrame[2][2];
  if (!ELL_3M_EXISTS(MF)) {
    sprintf(err, "%s: 3x3 measurement frame doesn't exist", me);
    biffAdd(TEN, err); return 1;
  }
  ELL_3M_TRANSPOSE(MFT, MF);

  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: trouble with initial copy", me);
      biffAdd(TEN, err); return 1;
    }
  }
  nn = nrrdElementNumber(nout)/nout->axis[0].size;
  tdata = (float*)(nout->data);
  for (ii=0; ii<nn; ii++) {
    TEN_T2M(tenMeasr, tdata);
    ell_3m_mul_d(tenWorld, MF, tenMeasr);
    ell_3m_mul_d(tenWorld, tenWorld, MFT);
    TEN_M2T_TT(tdata, float, tenWorld);
    tdata += 7;
  }
  for (si=0; si<NRRD_SPACE_DIM_MAX; si++) {
    for (sj=0; sj<NRRD_SPACE_DIM_MAX; sj++) {
      nout->measurementFrame[si][sj] = AIR_NAN;
    }
  }

  return 0;
}

int
tenExpand(Nrrd *nout, const Nrrd *nin, double scale, double thresh) {
  char me[]="tenExpand", err[BIFF_STRLEN];
  size_t N, I, sx, sy, sz;
  float *seven, *nine;

  if (!( nout && nin && AIR_EXISTS(thresh) )) {
    sprintf(err, "%s: got NULL pointer or non-existant threshold", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
    sprintf(err, "%s: ", me);
    biffAdd(TEN, err); return 1;
  }

  sx = nin->axis[1].size;
  sy = nin->axis[2].size;
  sz = nin->axis[3].size;
  N = sx*sy*sz;
  if (nrrdMaybeAlloc_va(nout, nrrdTypeFloat, 4, 
                        AIR_CAST(size_t, 9), sx, sy, sz)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  for (I=0; I<=N-1; I++) {
    seven = (float*)(nin->data) + I*7;
    nine = (float*)(nout->data) + I*9;
    if (seven[0] < thresh) {
      ELL_3M_ZERO_SET(nine);
      continue;
    }
    TEN_T2M(nine, seven);
    ELL_3M_SCALE(nine, AIR_CAST(float, scale), nine);
  }
  if (nrrdAxisInfoCopy(nout, nin, NULL,
                       NRRD_AXIS_INFO_SIZE_BIT | NRRD_AXIS_INFO_KIND_BIT)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (nrrdBasicInfoCopy(nout, nin,
                        NRRD_BASIC_INFO_ALL ^ NRRD_BASIC_INFO_SPACE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  /* Tue Sep 13 18:36:45 EDT 2005: why did I do this?
  nout->axis[0].label = (char *)airFree(nout->axis[0].label);
  nout->axis[0].label = airStrdup("matrix");
  */
  nout->axis[0].kind = nrrdKind3DMatrix;

  return 0;
}

int
tenShrink(Nrrd *tseven, const Nrrd *nconf, const Nrrd *tnine) {
  char me[]="tenShrink", err[BIFF_STRLEN];
  size_t I, N, sx, sy, sz;
  float *seven, *conf, *nine;
  
  if (!(tseven && tnine)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( nrrdTypeFloat == tnine->type &&
         4 == tnine->dim &&
         9 == tnine->axis[0].size )) {
    sprintf(err, "%s: type not %s (was %s) or dim not 4 (was %d) "
            "or first axis size not 9 (was " _AIR_SIZE_T_CNV ")", me,
            airEnumStr(nrrdType, nrrdTypeFloat),
            airEnumStr(nrrdType, tnine->type),
            tnine->dim, tnine->axis[0].size);
    biffAdd(TEN, err); return 1;
  }
  sx = tnine->axis[1].size;
  sy = tnine->axis[2].size;
  sz = tnine->axis[3].size;
  if (nconf) {
    if (!( nrrdTypeFloat == nconf->type &&
           3 == nconf->dim &&
           sx == nconf->axis[0].size &&
           sy == nconf->axis[1].size &&
           sz == nconf->axis[2].size )) {
      sprintf(err, "%s: confidence type not %s (was %s) or dim not 3 (was %d) "
              "or dimensions didn't match tensor volume", me,
              airEnumStr(nrrdType, nrrdTypeFloat),
              airEnumStr(nrrdType, nconf->type),
              nconf->dim);
      biffAdd(TEN, err); return 1;
    }
  }
  if (nrrdMaybeAlloc_va(tseven, nrrdTypeFloat, 4,
                        AIR_CAST(size_t, 7), sx, sy, sz)) {
    sprintf(err, "%s: trouble allocating output", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  seven = (float *)tseven->data;
  conf = nconf ? (float *)nconf->data : NULL;
  nine = (float *)tnine->data;
  N = sx*sy*sz;
  for (I=0; I<N; I++) {
    TEN_M2T(seven, nine);
    seven[0] = conf ? conf[I] : 1.0f;
    seven += 7;
    nine += 9;
  }
  if (nrrdAxisInfoCopy(tseven, tnine, NULL,
                       NRRD_AXIS_INFO_SIZE_BIT | NRRD_AXIS_INFO_KIND_BIT)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (nrrdBasicInfoCopy(tseven, tnine,
                        NRRD_BASIC_INFO_ALL ^ NRRD_BASIC_INFO_SPACE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  tseven->axis[0].label = (char *)airFree(tseven->axis[0].label);
  tseven->axis[0].label = airStrdup("tensor");

  return 0;
}

/*
******** tenEigensolve_f
**
** uses ell_3m_eigensolve_d to get the eigensystem of a single tensor
** disregards the confidence value t[0]
**
** return is same as ell_3m_eigensolve_d, which is same as ell_cubic
**
** NOTE: Even in the post-Teem-1.7 switch from column-major to 
** row-major- its still the case that the eigenvectors are at
** evec+0, evec+3, evec+6: this means that they USED to be the
** "columns" of the matrix, and NOW they're the rows.  
**
** This does NOT use biff
*/
int
tenEigensolve_f(float _eval[3], float _evec[9], const float t[7]) {
  double m[9], eval[3], evec[9], trc, iso[9];
  int ret;
  
  TEN_T2M(m, t);
  trc = ELL_3M_TRACE(m)/3.0;
  ELL_3M_IDENTITY_SET(iso);
  ELL_3M_SCALE_SET(iso, -trc, -trc, -trc);
  ELL_3M_ADD2(m, m, iso);
  if (_evec) {
    ret = ell_3m_eigensolve_d(eval, evec, m, AIR_TRUE);
    if (tenVerbose > 4) {
      fprintf(stderr, "---- cubic ret = %d\n", ret);
      fprintf(stderr, "tensor = {\n");
      fprintf(stderr, "    % 15.7f,\n", t[1]);
      fprintf(stderr, "    % 15.7f,\n", t[2]);
      fprintf(stderr, "    % 15.7f,\n", t[3]);
      fprintf(stderr, "    % 15.7f,\n", t[4]);
      fprintf(stderr, "    % 15.7f,\n", t[5]);
      fprintf(stderr, "    % 15.7f}\n", t[6]);
      fprintf(stderr, "roots = %d:\n", ret);
      fprintf(stderr, "    % 31.15f\n", trc + eval[0]);
      fprintf(stderr, "    % 31.15f\n", trc + eval[1]);
      fprintf(stderr, "    % 31.15f\n", trc + eval[2]);
    }
    ELL_3V_SET_TT(_eval, float, eval[0] + trc, eval[1] + trc, eval[2] + trc);
    ELL_3M_COPY_TT(_evec, float, evec);
    if (ell_cubic_root_single_double == ret) {
      /* this was added to fix a stupid problem with very nearly
         isotropic glyphs, used for demonstration figures */
      if (eval[0] == eval[1]) {
        ELL_3V_CROSS(_evec+6, _evec+0, _evec+3);
      } else {
        ELL_3V_CROSS(_evec+0, _evec+3, _evec+6);
      }
    }
    if ((tenVerbose > 1) && _eval[2] < 0) {
      fprintf(stderr, "tenEigensolve_f -------------\n");
      fprintf(stderr, "% 15.7f % 15.7f % 15.7f\n", 
              t[1], t[2], t[3]);
      fprintf(stderr, "% 15.7f % 15.7f % 15.7f\n", 
              t[2], t[4], t[5]);
      fprintf(stderr, "% 15.7f % 15.7f % 15.7f\n", 
              t[3], t[5], t[6]);
      fprintf(stderr, " --> % 15.7f % 15.7f % 15.7f\n",
              _eval[0], _eval[1], _eval[2]);
    }
  } else {
    /* caller only wants eigenvalues */
    ret = ell_3m_eigenvalues_d(eval, m, AIR_TRUE);
    ELL_3V_SET_TT(_eval, float, eval[0] + trc, eval[1] + trc, eval[2] + trc);
  }    
  return ret;
}

int
tenEigensolve_d(double _eval[3], double evec[9], const double t[7]) {
  double m[9], eval[3], trc, iso[9];
  int ret;
  
  TEN_T2M(m, t);
  trc = ELL_3M_TRACE(m)/3.0;
  ELL_3M_SCALE_SET(iso, -trc, -trc, -trc);
  ELL_3M_ADD2(m, m, iso);
  if (evec) {
    ret = ell_3m_eigensolve_d(eval, evec, m, AIR_TRUE);
    ELL_3V_SET(_eval, eval[0] + trc, eval[1] + trc, eval[2] + trc);
    if (ell_cubic_root_single_double == ret) {
      /* this was added to fix a stupid problem with very nearly
         isotropic glyphs, used for demonstration figures */
      if (eval[0] == eval[1]) {
        ELL_3V_CROSS(evec+6, evec+0, evec+3);
      } else {
        ELL_3V_CROSS(evec+0, evec+3, evec+6);
      }
    }
  } else {
    /* caller only wants eigenvalues */
    ret = ell_3m_eigenvalues_d(eval, m, AIR_TRUE);
    ELL_3V_SET(_eval, eval[0] + trc, eval[1] + trc, eval[2] + trc);
  }    
  return ret;
}



/*  lop A
    fprintf(stderr, "###################################  I = %d\n", (int)I);
    tenEigensolve(teval, tevec, out);
    fprintf(stderr, "evals: (%g %g %g) %g %g %g --> %g %g %g\n", 
            AIR_ABS(eval[0] - teval[0]) + 1,
            AIR_ABS(eval[1] - teval[1]) + 1,
            AIR_ABS(eval[2] - teval[2]) + 1,
            eval[0], eval[1], eval[2], 
            teval[0], teval[1], teval[2]);
    fprintf(stderr, "   tevec lens: %g %g %g\n", ELL_3V_LEN(tevec+3*0),
            ELL_3V_LEN(tevec+3*1), ELL_3V_LEN(tevec+3*2));
    ELL_3V_CROSS(tmp1, evec+3*0, evec+3*1); tmp2[0] = ELL_3V_LEN(tmp1);
    ELL_3V_CROSS(tmp1, evec+3*0, evec+3*2); tmp2[1] = ELL_3V_LEN(tmp1);
    ELL_3V_CROSS(tmp1, evec+3*1, evec+3*2); tmp2[2] = ELL_3V_LEN(tmp1);
    fprintf(stderr, "   evec[0] = %g %g %g\n", 
            (evec+3*0)[0], (evec+3*0)[1], (evec+3*0)[2]);
    fprintf(stderr, "   evec[1] = %g %g %g\n",
            (evec+3*1)[0], (evec+3*1)[1], (evec+3*1)[2]);
    fprintf(stderr, "   evec[2] = %g %g %g\n",
            (evec+3*2)[0], (evec+3*2)[1], (evec+3*2)[2]);
    fprintf(stderr, "   evec crosses: %g %g %g\n",
            tmp2[0], tmp2[1], tmp2[2]);
    ELL_3V_CROSS(tmp1, tevec+3*0, tevec+3*1); tmp2[0] = ELL_3V_LEN(tmp1);
    ELL_3V_CROSS(tmp1, tevec+3*0, tevec+3*2); tmp2[1] = ELL_3V_LEN(tmp1);
    ELL_3V_CROSS(tmp1, tevec+3*1, tevec+3*2); tmp2[2] = ELL_3V_LEN(tmp1);
    fprintf(stderr, "   tevec[0] = %g %g %g\n", 
            (tevec+3*0)[0], (tevec+3*0)[1], (tevec+3*0)[2]);
    fprintf(stderr, "   tevec[1] = %g %g %g\n",
            (tevec+3*1)[0], (tevec+3*1)[1], (tevec+3*1)[2]);
    fprintf(stderr, "   tevec[2] = %g %g %g\n",
            (tevec+3*2)[0], (tevec+3*2)[1], (tevec+3*2)[2]);
    fprintf(stderr, "   tevec crosses: %g %g %g\n",
            tmp2[0], tmp2[1], tmp2[2]);
    if (tmp2[1] < 0.5) {
      fprintf(stderr, "(panic)\n");
      exit(0);
    }
*/

void
tenMakeOne_f(float ten[7], float conf, float eval[3], float evec[9]) {
  double tmpMat1[9], tmpMat2[9], diag[9], evecT[9];

  ELL_3M_ZERO_SET(diag);
  ELL_3M_DIAG_SET(diag, eval[0], eval[1], eval[2]);
  ELL_3M_TRANSPOSE(evecT, evec);
  ELL_3M_MUL(tmpMat1, diag, evec);
  ELL_3M_MUL(tmpMat2, evecT, tmpMat1);
  ten[0] = conf;
  TEN_M2T_TT(ten, float, tmpMat2);
  return;
}

/*
******** tenMake
**
** create a tensor nrrd from nrrds of confidence, eigenvalues, and
** eigenvectors
*/
int
tenMake(Nrrd *nout, const Nrrd *nconf, const Nrrd *neval, const Nrrd *nevec) {
  char me[]="tenTensorMake", err[BIFF_STRLEN];
  size_t I, N, sx, sy, sz;
  float *out, *conf, *eval, *evec;
  int map[4];
  /* float teval[3], tevec[9], tmp1[3], tmp2[3]; */

  if (!(nout && nconf && neval && nevec)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdCheck(nconf) || nrrdCheck(neval) || nrrdCheck(nevec)) {
    sprintf(err, "%s: didn't get three valid nrrds", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (!( 3 == nconf->dim && nrrdTypeFloat == nconf->type )) {
    sprintf(err, "%s: first nrrd not a confidence volume "
            "(dim = %d, not 3; type = %s, not %s)", me,
            nconf->dim, airEnumStr(nrrdType, nconf->type),
            airEnumStr(nrrdType, nrrdTypeFloat));
    biffAdd(TEN, err); return 1;
  }
  sx = nconf->axis[0].size;
  sy = nconf->axis[1].size;
  sz = nconf->axis[2].size;
  if (!( 4 == neval->dim && 4 == nevec->dim &&
         nrrdTypeFloat == neval->type &&
         nrrdTypeFloat == nevec->type )) {
    sprintf(err, "%s: second and third nrrd aren't both 4-D (%d and %d) "
            "and type %s (%s and %s)",
            me, neval->dim, nevec->dim,
            airEnumStr(nrrdType, nrrdTypeFloat),
            airEnumStr(nrrdType, neval->type),
            airEnumStr(nrrdType, nevec->type));
    biffAdd(TEN, err); return 1;
  }
  if (!( 3 == neval->axis[0].size &&
         sx == neval->axis[1].size &&
         sy == neval->axis[2].size &&
         sz == neval->axis[3].size )) {
    sprintf(err, "%s: second nrrd sizes wrong: (" 
            _AIR_SIZE_T_CNV "," _AIR_SIZE_T_CNV "," 
            _AIR_SIZE_T_CNV "," _AIR_SIZE_T_CNV ") not (3," 
            _AIR_SIZE_T_CNV "," _AIR_SIZE_T_CNV "," _AIR_SIZE_T_CNV ")",
            me, neval->axis[0].size, neval->axis[1].size,
            neval->axis[2].size, neval->axis[3].size,
            sx, sy, sz);
    biffAdd(TEN, err); return 1;
  }
  if (!( 9 == nevec->axis[0].size &&
         sx == nevec->axis[1].size &&
         sy == nevec->axis[2].size &&
         sz == nevec->axis[3].size )) {
    sprintf(err, "%s: third nrrd sizes wrong: (" 
            _AIR_SIZE_T_CNV "," _AIR_SIZE_T_CNV "," 
            _AIR_SIZE_T_CNV "," _AIR_SIZE_T_CNV ") not (9," 
            _AIR_SIZE_T_CNV "," _AIR_SIZE_T_CNV "," _AIR_SIZE_T_CNV ")",
            me, nevec->axis[0].size, nevec->axis[1].size,
            nevec->axis[2].size, nevec->axis[3].size,
            sx, sy, sz);
    biffAdd(TEN, err); return 1;
  }

  /* finally */
  if (nrrdMaybeAlloc_va(nout, nrrdTypeFloat, 4,
                        AIR_CAST(size_t, 7), sx, sy, sz)) {
    sprintf(err, "%s: couldn't allocate output", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  N = sx*sy*sz;
  conf = (float *)(nconf->data);
  eval = (float *)neval->data;
  evec = (float *)nevec->data;
  out = (float *)nout->data;
  for (I=0; I<N; I++) {
    tenMakeOne_f(out, conf[I], eval, evec);
    /* lop A */
    out += 7;
    eval += 3;
    evec += 9;
  }
  ELL_4V_SET(map, -1, 0, 1, 2);
  if (nrrdAxisInfoCopy(nout, nconf, map, NRRD_AXIS_INFO_SIZE_BIT)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  nout->axis[0].label = (char *)airFree(nout->axis[0].label);
  nout->axis[0].label = airStrdup("tensor");
  if (nrrdBasicInfoCopy(nout, nconf,
                        NRRD_BASIC_INFO_DATA_BIT
                        | NRRD_BASIC_INFO_TYPE_BIT
                        | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                        | NRRD_BASIC_INFO_DIMENSION_BIT
                        | NRRD_BASIC_INFO_CONTENT_BIT
                        | NRRD_BASIC_INFO_COMMENTS_BIT
                        | (nrrdStateKeyValuePairsPropagate
                           ? 0
                           : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    sprintf(err, "%s:", me);
    biffMove(TEN, err, NRRD); return 1;
  }

  return 0;
}

int
tenSlice(Nrrd *nout, const Nrrd *nten, unsigned int axis,
         size_t pos, unsigned int dim) {
  Nrrd *nslice, *ncoeff[4];
  int ci[4];
  char me[]="tenSlice", err[BIFF_STRLEN];
  airArray *mop;

  if (!(nout && nten)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nten, nrrdTypeDefault, AIR_TRUE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a valid tensor field", me);
    biffAdd(TEN, err); return 1;
  }
  if (!(2 == dim || 3 == dim)) {
    sprintf(err, "%s: given dim (%d) not 2 or 3", me, dim);
    biffAdd(TEN, err); return 1;
  }
  if (!( axis <= 2 )) {
    sprintf(err, "%s: axis %u not in valid range [0,1,2]", me, axis);
    biffAdd(TEN, err); return 1;
  }
  if (!( pos < nten->axis[1+axis].size )) {
    sprintf(err, "%s: slice position " _AIR_SIZE_T_CNV 
            " not in valid range [0.." _AIR_SIZE_T_CNV "]", me,
            pos, nten->axis[1+axis].size-1);
    biffAdd(TEN, err); return 1;
  }

  /*
  ** threshold        0
  ** Dxx Dxy Dxz      1   2   3
  ** Dxy Dyy Dyz  =  (2)  4   5
  ** Dxz Dyz Dzz     (3) (5)  6 
  */
  mop = airMopNew();
  airMopAdd(mop, nslice=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (3 == dim) {
    if (nrrdSlice(nslice, nten, axis+1, pos)
        || nrrdAxesInsert(nout, nslice, axis+1)) {
      sprintf(err, "%s: trouble making slice", me);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
  } else {
    airMopAdd(mop, ncoeff[0]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    airMopAdd(mop, ncoeff[1]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    airMopAdd(mop, ncoeff[2]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    airMopAdd(mop, ncoeff[3]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    switch(axis) {
    case 0:
      ELL_4V_SET(ci, 0, 4, 5, 6);
      break;
    case 1:
      ELL_4V_SET(ci, 0, 1, 3, 6);
      break;
    case 2:
      ELL_4V_SET(ci, 0, 1, 2, 4);
      break;
    default:
      sprintf(err, "%s: axis %d bogus", me, axis);
      biffAdd(TEN, err); airMopError(mop); return 1;
      break;
    }
    if (nrrdSlice(nslice, nten, axis+1, pos)
        || nrrdSlice(ncoeff[0], nslice, 0, ci[0])
        || nrrdSlice(ncoeff[1], nslice, 0, ci[1])
        || nrrdSlice(ncoeff[2], nslice, 0, ci[2])
        || nrrdSlice(ncoeff[3], nslice, 0, ci[3])
        || nrrdJoin(nout, (const Nrrd **)ncoeff, 4, 0, AIR_TRUE)) {
      sprintf(err, "%s: trouble collecting coefficients", me);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
  }

  airMopOkay(mop);
  return 0;
}

#define Txx (ten[1])
#define Txy (ten[2])
#define Txz (ten[3])
#define Tyy (ten[4])
#define Tyz (ten[5])
#define Tzz (ten[6])

/*
** eval and evec are given merely as space into which
** we may compute the eigensystem
*/
void
tenInvariantGradients_d(double mu1[7],
                        double mu2[7], double *mu2Norm,
                        double skw[7], double *skwNorm,
                        double ten[7]) {
  double eval[3], evec[9];
  double mu1Dot, mu2Dot, dot, mev, third, matR[9],
    matA[9], matB[9], norm, epsilon;

  /* largest allowable dot product betwen two normals */
  epsilon = 0.00001;

  TEN_T_SET(mu1, ten[0],
            0.57735027, 0, 0,      /* sqrt(1/3) = 0.57735027 */
            0.57735027, 0, 
            0.57735027);
  TEN_T_SET(mu2, ten[0],
            2*Txx - Tyy - Tzz, 3*Txy, 3*Txz,
            2*Tyy - Txx - Tzz, 3*Tyz,
            2*Tzz - Txx - Tyy);
  *mu2Norm = TEN_T_NORM(mu2);
  if (*mu2Norm < epsilon) {
    /* they gave us a diagonal matrix */
    TEN_T_SET(mu2, ten[0],
              0.816496581, 0, 0,   /* sqrt(2/3) = 0.816496581 */
              -0.408248290, 0,     /* sqrt(1/6) = 0.408248290 */
              -0.408248290);
    TEN_T_SET(skw, ten[0],
              0, 0, 0,
              0.707106781, 0,      /* sqrt(1/2) = 0.707106781 */
              -0.707106781);
    *skwNorm = 0;  /* have to invent a value, this works */
  } else {
    /* we have some variance */
    TEN_T_SCALE(mu2, 1.0/(*mu2Norm), mu2);
    /* make damn sure mu2 is orthogonal to mu1 */
    dot = TEN_T_DOT(mu2, mu1);
    if (AIR_ABS(dot) > epsilon) {
      TEN_T_SCALE_INCR(mu2, -dot, mu1);
      norm = TEN_T_NORM(mu2);
      TEN_T_SCALE(mu2, 1.0/(norm), mu2);
    }
    TEN_T_SET(skw, ten[0],
              Tyy*Tzz - Tyz*Tyz, Txz*Tyz - Txy*Tzz, Txy*Tyz - Txz*Tyy,
              Txx*Tzz - Txz*Txz, Txy*Txz - Tyz*Txx,
              Txx*Tyy - Txy*Txy);
    mu1Dot = TEN_T_DOT(skw, mu1);
    mu2Dot = TEN_T_DOT(skw, mu2);
    TEN_T_SCALE_INCR2(skw, -mu1Dot, mu1, -mu2Dot, mu2);
    *skwNorm = TEN_T_NORM(skw);
    if (*skwNorm < epsilon) {
      /* skw is at an extremum, have to diagonalize */
      tenEigensolve_d(eval, evec, ten);
      mev = (eval[0] + eval[1] + eval[2])/3;
      third = ((eval[0] - mev)*(eval[0] - mev)*(eval[0] - mev)
               + (eval[1] - mev)*(eval[1] - mev)*(eval[1] - mev)
               + (eval[2] - mev)*(eval[2] - mev)*(eval[2] - mev))/3;
      if (third > 0) {
        /* skw is positive: linear: eval[1] = eval[2] */
        ELL_3V_SET(matA + 0*3, 0, 0, 0);
        ELL_3V_SET(matA + 1*3, 0, 0.707106781, 0);
        ELL_3V_SET(matA + 2*3, 0, 0, -0.707106781);
      } else {
        /* skw is negative: planar: eval[0] = eval[1] */
        ELL_3V_SET(matA + 0*3, 0.707106781, 0, 0);
        ELL_3V_SET(matA + 1*3, 0, -0.707106781, 0);
        ELL_3V_SET(matA + 2*3, 0, 0, 0);
      }
      ELL_3M_TRANSPOSE(matR, evec);
      ELL_3M_MUL(matB, matA, evec);
      ELL_3M_MUL(matA, matR, matB);
      TEN_M2T(skw, matA);
      /* not sure why this last orgthonalization against mu2 is needed,
         but I got some slop otherwise : mu2 . skw = 0.001 or so */
      dot = TEN_T_DOT(mu2, skw);
      TEN_T_SCALE_INCR(skw, -dot, mu2);
      norm = TEN_T_NORM(skw);
      TEN_T_SCALE(skw, 1.0/(norm), skw);
    } else {
      /* skw not at extremum */
      TEN_T_SCALE(skw, 1.0/(*skwNorm), skw);
      /* make damn sure skw is orthogonal to mu1 ... */
      dot = TEN_T_DOT(skw, mu1);
      if (AIR_ABS(dot) > epsilon) {
        TEN_T_SCALE_INCR(skw, -dot, mu1);
      }
      /* ... and to mu2 */
      dot = TEN_T_DOT(skw, mu2);
      if (AIR_ABS(dot) > epsilon) {
        TEN_T_SCALE_INCR(skw, -dot, mu2);
      }
      norm = TEN_T_NORM(skw);
      TEN_T_SCALE(skw, 1.0/(norm), skw);
    }
  }
  if (fabs(TEN_T_DOT(skw, mu1)) > 0.7) {
    fprintf(stderr, "tenShapeGradients_d : PANIC\n");
    fprintf(stderr, "dots = %g %g %g\n",
            TEN_T_DOT(mu1, mu2),
            TEN_T_DOT(mu1, skw),
            TEN_T_DOT(mu2, skw));
    fprintf(stderr, "ten = (%g) %g %g %g   %g %g   %g\n",
            ten[0],
            ten[1], ten[2], ten[3],
            ten[4], ten[5],
            ten[6]);
    fprintf(stderr, "mu2Norm = %g; mu2 = (%g) %g %g %g   %g %g   %g\n",
            *mu2Norm,
            mu2[0],
            mu2[1], mu2[2], mu2[3],
            mu2[4], mu2[5],
            mu2[6]);
    fprintf(stderr, "skwNorm = %g; skw = (%g) %g %g %g   %g %g   %g\n",
            *skwNorm,
            skw[0],
            skw[1], skw[2], skw[3],
            skw[4], skw[5],
            skw[6]);
    exit(0);
  }

  return;
}

/*
** eval and evec must be pre-computed and given to us
*/
void
tenRotationTangents_d(double phi1[7],
                      double phi2[7],
                      double phi3[7],
                      double evec[9]) {
  double outA[9], outB[9], mat[9];

  if (phi1) {
    phi1[0] = 1.0;
    ELL_3MV_OUTER(outA, evec + 1*3, evec + 2*3);
    ELL_3MV_OUTER(outB, evec + 2*3, evec + 1*3);
    ELL_3M_SCALE_ADD2(mat, 0.7071068, outA, 0.7071068, outB);
    TEN_M2T(phi1, mat);
  }

  if (phi2) {
    phi2[0] = 1.0;
    ELL_3MV_OUTER(outA, evec + 0*3, evec + 2*3);
    ELL_3MV_OUTER(outB, evec + 2*3, evec + 0*3);
    ELL_3M_SCALE_ADD2(mat, 0.7071068, outA, 0.7071068, outB);
    TEN_M2T(phi2, mat);
  }

  if (phi3) {
    phi3[0] = 1.0;
    ELL_3MV_OUTER(outA, evec + 0*3, evec + 1*3);
    ELL_3MV_OUTER(outB, evec + 1*3, evec + 0*3);
    ELL_3M_SCALE_ADD2(mat, 0.7071068, outA, 0.7071068, outB);
    TEN_M2T(phi3, mat);
  }
  
  return;
}

void
tenInv_f(float inv[7], float ten[7]) {
  float det;

  TEN_T_INV(inv, ten, det);
}

void
tenInv_d(double inv[7], double ten[7]) {
  double det;

  TEN_T_INV(inv, ten, det);
}
