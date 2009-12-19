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

#include "gage.h"
#include "privateGage.h"

/*
** _gageSclTable
**
** the static array of item information for the scalar kind. 
**
** General notes about setting these things up: 
** - Yes, you'll need more than 80 columns of display
** - You do need to explicitily initialize all the prerequisite elements,
** because the compiler will initialize them to zero (which is a valid
** item value)
*/
gageItemEntry
_gageSclTable[GAGE_SCL_ITEM_MAX+1] = {
  /* enum value        len,deriv,  prereqs,                                       parent item,    parent index,  needData */
  {gageSclValue,         1,  0,  {-1, -1, -1, -1, -1},                                    -1,              -1,  0},
  {gageSclGradVec,       3,  1,  {-1, -1, -1, -1, -1},                                    -1,              -1,  0},
  {gageSclGradMag,       1,  1,  {gageSclGradVec, -1, -1, -1, -1},                        -1,              -1,  0},
  {gageSclNormal,        3,  1,  {gageSclGradVec, gageSclGradMag, -1, -1, -1},            -1,              -1,  0},
  {gageSclNPerp,         9,  1,  {gageSclNormal, -1, -1, -1, -1},                         -1,              -1,  0},
  {gageSclHessian,       9,  2,  {gageSclHessian, -1, -1, -1, -1},                        -1,              -1,  0},
  {gageSclLaplacian,     1,  2,  {gageSclHessian, -1, -1, -1, -1},                        -1,              -1,  0},
  {gageSclHessFrob,      1,  2,  {gageSclHessian, -1, -1, -1, -1},                        -1,              -1,  0},
  {gageSclHessEval,      3,  2,  {gageSclHessian, -1, -1, -1, -1},                        -1,              -1,  0},
  {gageSclHessEval0,     1,  2,  {gageSclHessEval, -1, -1, -1, -1},                       gageSclHessEval,  0,  0},
  {gageSclHessEval1,     1,  2,  {gageSclHessEval, -1, -1, -1, -1},                       gageSclHessEval,  1,  0},
  {gageSclHessEval2,     1,  2,  {gageSclHessEval, -1, -1, -1, -1},                       gageSclHessEval,  2,  0},
  {gageSclHessEvec,      9,  2,  {gageSclHessian, gageSclHessEval, -1, -1, -1},           -1,              -1,  0},
  {gageSclHessEvec0,     3,  2,  {gageSclHessEvec, -1, -1, -1, -1},                       gageSclHessEvec,  0,  0},
  {gageSclHessEvec1,     3,  2,  {gageSclHessEvec, -1, -1, -1, -1},                       gageSclHessEvec,  3,  0},
  {gageSclHessEvec2,     3,  2,  {gageSclHessEvec, -1, -1, -1, -1},                       gageSclHessEvec,  6,  0},
  {gageScl2ndDD,         1,  2,  {gageSclHessian, gageSclNormal, -1, -1, -1},             -1,              -1,  0},
  {gageSclGeomTens,      9,  2,  {gageSclHessian, gageSclNPerp, gageSclGradMag, -1, -1},  -1,              -1,  0},
  {gageSclK1,            1,  2,  {gageSclTotalCurv, gageSclShapeTrace, -1, -1, -1},       -1,              -1,  0},
  {gageSclK2,            1,  2,  {gageSclTotalCurv, gageSclShapeTrace, -1, -1, -1},       -1,              -1,  0},
  {gageSclTotalCurv,     1,  2,  {gageSclGeomTens, -1, -1, -1, -1},                       -1,              -1,  0},
  {gageSclShapeTrace,    1,  2,  {gageSclGeomTens, -1, -1, -1, -1},                       -1,              -1,  0},
  {gageSclShapeIndex,    1,  2,  {gageSclK1, gageSclK2, -1, -1, -1},                      -1,              -1,  0},
  {gageSclMeanCurv,      1,  2,  {gageSclK1, gageSclK2, -1, -1, -1},                      -1,              -1,  0},
  {gageSclGaussCurv,     1,  2,  {gageSclK1, gageSclK2, -1, -1, -1},                      -1,              -1,  0},
  {gageSclCurvDir1,      3,  2,  {gageSclGeomTens, gageSclK1, gageSclK2, -1, -1},         -1,              -1,  0},
  {gageSclCurvDir2,      3,  2,  {gageSclGeomTens, gageSclK1, gageSclK2, -1, -1},         -1,              -1,  0},
  {gageSclFlowlineCurv,  1,  2,  {gageSclGeomTens, -1, -1, -1, -1},                       -1,              -1,  0},
  {gageSclMedian,        1,  0,  {-1, -1, -1, -1, -1},                                    -1,              -1,  0}
};

char
_gageSclStr[][AIR_STRLEN_SMALL] = {
  "(unknown gageScl)",
  "value",
  "gradient vector",
  "gradient magnitude",
  "normalized gradient",
  "tangent projector",
  "Hessian",
  "Laplacian",
  "Frob(Hessian)",
  "Hessian eigenvalues",
  "Hessian eigenvalue[0]",
  "Hessian eigenvalue[1]",
  "Hessian eigenvalue[2]",
  "Hessian eigenvectors",
  "Hessian eigenvector[0]",
  "Hessian eigenvector[1]",
  "Hessian eigenvector[2]",
  "2nd DD along gradient",
  "geometry tensor",
  "kappa1",
  "kappa2",
  "total curvature",
  "shape trace",
  "shape index",
  "mean curvature",
  "Gaussian curvature",
  "1st curvature direction",
  "2nd curvature direction",
  "flowline curvature",
  "median"
};

char
_gageSclDesc[][AIR_STRLEN_MED] = {
  "unknown gageScl query",
  "reconstructed scalar data value",
  "gradient vector, un-normalized",
  "gradient magnitude (length of gradient vector)",
  "projection into tangent (perp space of normal)",
  "normalized gradient vector",
  "3x3 Hessian matrix",
  "Laplacian",
  "Frobenius norm of Hessian",
  "Hessian's eigenvalues",
  "Hessian's 1st eigenvalue",
  "Hessian's 2nd eigenvalue",
  "Hessian's 3rd eigenvalue",
  "Hessian's eigenvectors",
  "Hessian's 1st eigenvector",
  "Hessian's 2nd eigenvector",
  "Hessian's 3rd eigenvector",
  "2nd directional derivative along gradient",
  "geometry tensor",
  "1st principal curvature (K1)",
  "2nd principal curvature (K2)",
  "total curvature (L2 norm of K1, K2)",
  "shape trace = (K1+K2)/(total curvature)",
  "Koenderink's shape index",
  "mean curvature = (K1+K2)/2",
  "gaussian curvature = K1*K2",
  "1st principal curvature direction",
  "2nd principal curvature direction",
  "curvature of normal streamline",
  "median of iv3 cache (not weighted by any filter (yet))"
};

int
_gageSclVal[] = {
  gageSclUnknown,
  gageSclValue,
  gageSclGradVec,
  gageSclGradMag,
  gageSclNormal,
  gageSclNPerp,
  gageSclHessian,
  gageSclLaplacian,
  gageSclHessFrob,
  gageSclHessEval,
  gageSclHessEval0,
  gageSclHessEval1,
  gageSclHessEval2,
  gageSclHessEvec,
  gageSclHessEvec0,
  gageSclHessEvec1,
  gageSclHessEvec2,
  gageScl2ndDD,
  gageSclGeomTens,
  gageSclK1,
  gageSclK2,
  gageSclTotalCurv,
  gageSclShapeTrace,
  gageSclShapeIndex,
  gageSclMeanCurv,
  gageSclGaussCurv,
  gageSclCurvDir1,
  gageSclCurvDir2,
  gageSclFlowlineCurv,
  gageSclMedian
};

#define GS_V   gageSclValue
#define GS_GV  gageSclGradVec
#define GS_GM  gageSclGradMag
#define GS_N   gageSclNormal
#define GS_NP  gageSclNPerp
#define GS_H   gageSclHessian
#define GS_L   gageSclLaplacian
#define GS_HF  gageSclHessFrob
#define GS_HA  gageSclHessEval
#define GS_HA0 gageSclHessEval0
#define GS_HA1 gageSclHessEval1
#define GS_HA2 gageSclHessEval2
#define GS_HE  gageSclHessEvec
#define GS_HE0 gageSclHessEvec0
#define GS_HE1 gageSclHessEvec1
#define GS_HE2 gageSclHessEvec2
#define GS_2D  gageScl2ndDD
#define GS_GT  gageSclGeomTens
#define GS_K1  gageSclK1
#define GS_K2  gageSclK2
#define GS_TC  gageSclTotalCurv
#define GS_ST  gageSclShapeTrace
#define GS_SI  gageSclShapeIndex
#define GS_MC  gageSclMeanCurv
#define GS_GC  gageSclGaussCurv
#define GS_C1  gageSclCurvDir1
#define GS_C2  gageSclCurvDir2
#define GS_FC  gageSclFlowlineCurv
#define GS_MD  gageSclMedian

char
_gageSclStrEqv[][AIR_STRLEN_SMALL] = {
  "v", "val", "value", 
  "gv", "gvec", "gradvec", "grad vec", "gradient vector",
  "gm", "gmag", "gradmag", "grad mag", "gradient magnitude",
  "gn", "n", "normal", "gnorm", "normg", "norm", "normgrad", \
       "norm grad", "normalized gradient",
  "np", "nperp", 
  "h", "hess", "hessian",
  "l", "lapl", "laplacian",
  "hf",
  "heval", "h eval", "hessian eval", "hessian eigenvalues",
  "heval0",
  "heval1",
  "heval2",
  "hevec", "h evec", "hessian evec", "hessian eigenvectors",
  "hevec0",
  "hevec1",
  "hevec2",
  "2d", "2dd", "2nddd", "2nd", "2nd dd", "2nd dd along gradient",
  "gt", "gten", "geoten", "geomten", "geometry tensor",
  "k1", "kap1", "kappa1",
  "k2", "kap2", "kappa2",
  "total curv", "totalcurv", "total curvature", "tc", "cv", "curvedness",
  "st", "shape trace",
  "si", "shape index",
  "mc", "mcurv", "meancurv", "mean curvature",
  "gc", "gcurv", "gausscurv", "gaussian curvature",
  "cdir1", "c dir1", "curvdir1", "curv dir1", "curvature direction 1",
  "cdir2", "c dir2", "curvdir2", "curv dir2", "curvature direction 2",
  "fc", "flowlinecurv", "flowline curv", "flowline curvature",
  "med", "median",
  ""
};

int
_gageSclValEqv[] = {
  GS_V, GS_V, GS_V,
  GS_GV, GS_GV, GS_GV, GS_GV, GS_GV, 
  GS_GM, GS_GM, GS_GM, GS_GM, GS_GM,
  GS_N,  GS_N, GS_N, GS_N, GS_N, GS_N, GS_N, GS_N, GS_N,
  GS_NP, GS_NP,
  GS_H, GS_H, GS_H, 
  GS_L, GS_L, GS_L, 
  GS_HF,
  GS_HA, GS_HA, GS_HA, GS_HA, 
  GS_HA0,
  GS_HA1,
  GS_HA2,
  GS_HE, GS_HE, GS_HE, GS_HE, 
  GS_HE0,
  GS_HE1,
  GS_HE2,
  GS_2D, GS_2D, GS_2D, GS_2D, GS_2D, GS_2D,
  GS_GT, GS_GT, GS_GT, GS_GT, GS_GT, 
  GS_K1, GS_K1, GS_K1,
  GS_K2, GS_K2, GS_K2,
  GS_TC, GS_TC, GS_TC, GS_TC, GS_TC, GS_TC,
  GS_ST, GS_ST,
  GS_SI, GS_SI,
  GS_MC, GS_MC, GS_MC, GS_MC,
  GS_GC, GS_GC, GS_GC, GS_GC,
  GS_C1, GS_C1, GS_C1, GS_C1, GS_C1,
  GS_C2, GS_C2, GS_C2, GS_C2, GS_C2,
  GS_FC, GS_FC, GS_FC, GS_FC,
  GS_MD, GS_MD
};

airEnum
_gageScl = {
  "gageScl",
  GAGE_SCL_ITEM_MAX+1,
  _gageSclStr, _gageSclVal,
  _gageSclDesc,
  _gageSclStrEqv, _gageSclValEqv,
  AIR_FALSE
};
airEnum *const
gageScl = &_gageScl;

gageKind
_gageKindScl = {
  "scalar",
  &_gageScl,
  0,
  1,
  GAGE_SCL_ITEM_MAX,
  _gageSclTable,
  _gageSclIv3Print,
  _gageSclFilter,
  _gageSclAnswer,
  NULL, NULL, NULL,
  NULL
};
gageKind *const
gageKindScl = &_gageKindScl;
