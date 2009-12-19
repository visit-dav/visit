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

/*

http://www.mathworks.com/access/helpdesk/help/toolbox/curvefit/ch_fitt5.html#40515

*/

/* ---------------------------------------------- */

char
_tenEstimateMethodStr[][AIR_STRLEN_SMALL] = {
  "(unknown tenEstimateMethod)",
  "LLS",
  "WLS",
  "NLS"
};

char
_tenEstimateMethodDesc[][AIR_STRLEN_MED] = {
  "unknown tenEstimateMethod",
  "linear least-squares fit of log(DWI)",
  "weighted least-squares fit of log(DWI)",
  "non-linear least-squares fit of DWI"
};

airEnum
_tenEstimateMethod = {
  "tenEstimateMethod",
  TEN_ESTIMATE_METHOD_MAX,
  _tenEstimateMethodStr, NULL,
  _tenEstimateMethodDesc,
  NULL, NULL,
  AIR_FALSE
};
airEnum *
tenEstimateMethod= &_tenEstimateMethod;

/* ---------------------------------------------- */

int
_tenGaussian(double *retP, double m, double t, double s) {
  char me[]="_tenGaussian", err[BIFF_STRLEN];
  double diff, earg, den;

  if (!retP) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  diff = (m-t)/2;
  earg = -diff*diff/2;
  den = s*sqrt(2*AIR_PI);
  *retP = exp(earg)/den;
  if (!AIR_EXISTS(*retP)) {
    sprintf(err, "%s: m=%g, t=%g, s=%g", me, m, t, s);
    biffAdd(TEN, err); 
    sprintf(err, "%s: diff=%g, earg=%g, den=%g", me, diff, earg, den);
    biffAdd(TEN, err);
    sprintf(err, "%s: failed with ret = exp(%g)/%g = %g/%g = %g",
            me, earg, den, exp(earg), den, *retP);
    biffAdd(TEN, err);
    *retP = AIR_NAN; return 1;
  }
  return 0;
}

int
_tenRicianTrue(double *retP,
               double m /* measured */, 
               double t /* truth */, 
               double s /* sigma */) {
  char me[]="_tenRicianTrue", err[BIFF_STRLEN];
  double mos, moss, mos2, tos2, tos, ss, earg, barg;

  if (!retP) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }

  mos = m/s;
  moss = mos/s;
  tos = t/s;
  ss = s*s;
  mos2 = mos*mos;
  tos2 = tos*tos;
  earg = -(mos2 + tos2)/2;
  barg = mos*tos;
  *retP = exp(earg)*airBesselI0(barg)*moss;

  if (!AIR_EXISTS(*retP)) {
    sprintf(err, "%s: m=%g, t=%g, s=%g", me, m, t, s);
    biffAdd(TEN, err);
    sprintf(err, "%s: mos=%g, moss=%g, tos=%g, ss=%g",
            me, mos, moss, tos, ss);
    biffAdd(TEN, err);
    sprintf(err, "%s: mos2=%g, tos2=%g, earg=%g, barg=%g", me,
            mos2, tos2, earg, barg);
    biffAdd(TEN, err);
    sprintf(err, "%s: failed: ret=exp(%g)*bessi0(%g)*%g = %g * %g * %g = %g",
            me, earg, barg, moss, exp(earg), airBesselI0(barg), moss, *retP);
    biffAdd(TEN, err);
    *retP = AIR_NAN; return 1;
  }
  return 0;
}

int
_tenRicianSafe(double *retP, double m, double t, double s) {
  char me[]="_tenRicianSafe", err[BIFF_STRLEN];
  double diff, ric, gau, neer=10, faar=20;
  int E;

  if (!retP) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }

  diff = AIR_ABS(m-t)/s;
  E = 0;
  if (diff < neer) {
    if (!E) E |= _tenRicianTrue(retP, m, t, s);
  } else if (diff < faar) {
    if (!E) E |= _tenRicianTrue(&ric, m, t, s);
    if (!E) E |= _tenGaussian(&gau, m, t, s);
    if (!E) *retP = AIR_AFFINE(neer, diff, faar, ric, gau);
  } else {
    if (!E) E |= _tenGaussian(retP, m, t, s);
  }
  if (E) {
    sprintf(err, "%s: failed with m=%g, t=%g, s=%g -> diff=%g",
            me, m, t, s, diff);
    biffAdd(TEN, err); *retP = AIR_NAN; return 1;
  }
  return 0;
}

int
_tenRician(double *retP,
           double m /* measured */, 
           double t /* truth */, 
           double s /* sigma */) {
  char me[]="_tenRician", err[BIFF_STRLEN];
  double tos, ric, gau, loSignal=4.0, hiSignal=8.0;
  int E;

  if (!retP) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( m >= 0 && t >= 0 && s > 0 )) {
    sprintf(err, "%s: got bad args: m=%g t=%g s=%g", me, m, t, s);
    biffAdd(TEN, err); *retP = AIR_NAN; return 1;
  }

  tos = t/s;
  E = 0;
  if (tos < loSignal) {
    if (!E) E |= _tenRicianSafe(retP, m, t, s);
  } else if (tos < hiSignal) {
    if (!E) E |= _tenRicianSafe(&ric, m, t, s);
    if (!E) E |= _tenGaussian(&gau, m, t, s);
    if (!E) *retP = AIR_AFFINE(loSignal, tos, hiSignal, ric, gau);
  } else {
    if (!E) E |= _tenGaussian(retP, m, t, s);
  }
  if (E) {
    sprintf(err, "%s: failed with m=%g, t=%g, s=%g -> tos=%g",
            me, m, t, s, tos);
    biffAdd(TEN, err); *retP = AIR_NAN; return 1;
  }
  return 0;
}

enum {
  flagUnknown,
  flagEstimateMethod,
  flagBInfo,
  flagAllNum,
  flagDwiNum,
  flagAllAlloc,
  flagDwiAlloc,
  flagAllSet,
  flagDwiSet,
  flagWght,
  flagEmat,
  flagLast
};

void
_tenEstimateOutputInit(tenEstimateContext *tec) {

  tec->estimatedB0 = AIR_NAN;
  TEN_T_SET(tec->ten, AIR_NAN,
            AIR_NAN, AIR_NAN, AIR_NAN,
            AIR_NAN, AIR_NAN,
            AIR_NAN);
  tec->conf = AIR_NAN;
  tec->mdwi = AIR_NAN;
  tec->time = AIR_NAN;
  tec->errorDwi = AIR_NAN;
  tec->errorLogDwi = AIR_NAN;
}

tenEstimateContext *
tenEstimateContextNew() {
  tenEstimateContext *tec;
  unsigned int fi;

  tec = AIR_CAST(tenEstimateContext *, malloc(sizeof(tenEstimateContext)));
  if (tec) {
    tec->bValue = AIR_NAN;
    tec->valueMin = AIR_NAN;
    tec->sigma = AIR_NAN;
    tec->dwiConfThresh = AIR_NAN;
    tec->dwiConfSoft = AIR_NAN;
    tec->_ngrad = NULL;
    tec->_nbmat = NULL;
    tec->all_f = NULL;
    tec->all_d = NULL;
    tec->simulate = AIR_FALSE;
    tec->estimateMethod = tenEstimateMethodUnknown;
    tec->estimateB0 = AIR_TRUE;
    tec->recordTime = AIR_FALSE;
    tec->recordErrorDwi = AIR_FALSE;
    tec->recordErrorLogDwi = AIR_FALSE;
    tec->verbose = 0;
    tec->progress = AIR_FALSE;
    tec->WLSIterNum = 3;
    for (fi=flagUnknown+1; fi<flagLast; fi++) {
      tec->flag[fi] = AIR_FALSE;
    }
    tec->allNum = 0;
    tec->dwiNum = 0;
    tec->nbmat = nrrdNew();
    tec->nwght = nrrdNew();
    tec->nemat = nrrdNew();
    tec->knownB0 = AIR_NAN;
    tec->all = NULL;
    tec->bnorm = NULL;
    tec->allTmp = NULL;
    tec->dwiTmp = NULL;
    tec->dwi = NULL;
    _tenEstimateOutputInit(tec);
  }
  return tec;
}

tenEstimateContext *
tenEstimateContextNix(tenEstimateContext *tec) {

  if (tec) {
    nrrdNuke(tec->nbmat);
    nrrdNuke(tec->nwght);
    nrrdNuke(tec->nemat);
    airFree(tec->all);
    airFree(tec->bnorm);
    airFree(tec->allTmp);
    airFree(tec->dwiTmp);
    airFree(tec->dwi);
    airFree(tec);
  }
  return NULL;
}

void
tenEstimateVerboseSet(tenEstimateContext *tec,
                      int verbose) {
  if (tec) {
    tec->verbose = verbose;
  }
  return;
}

int
tenEstimateMethodSet(tenEstimateContext *tec, int estimateMethod) {
  char me[]="tenEstimateMethodSet", err[BIFF_STRLEN];

  if (!tec) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenEstimateMethod, estimateMethod)) {
    sprintf(err, "%s: estimateMethod %d not valid", me, estimateMethod);
    biffAdd(TEN, err); return 1;
  }

  if (tec->estimateMethod != estimateMethod) {
    tec->estimateMethod = estimateMethod;
    tec->flag[flagEstimateMethod] = AIR_TRUE;
  }
  
  return 0;
}

int
tenEstimateSigmaSet(tenEstimateContext *tec, double sigma) {
  char me[]="tenEstimateSigmaSet", err[BIFF_STRLEN];

  if (!tec) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!(AIR_EXISTS(sigma) && sigma >= 0.0)) {
    sprintf(err, "%s: given sigma (%g) not existent and >= 0.0", me, sigma);
    biffAdd(TEN, err); return 1;
  }

  tec->sigma = sigma;

  return 0;
}

int
tenEstimateValueMinSet(tenEstimateContext *tec, double valueMin) {
  char me[]="tenEstimateValueMinSet", err[BIFF_STRLEN];

  if (!tec) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!(AIR_EXISTS(valueMin) && valueMin > 0.0)) {
    sprintf(err, "%s: given valueMin (%g) not existant and > 0.0",
            me, valueMin);
    biffAdd(TEN, err); return 1;
  }

  tec->valueMin = valueMin;

  return 0;
}

int
tenEstimateGradientsSet(tenEstimateContext *tec,
                        const Nrrd *ngrad, double bValue, int estimateB0) {
  char me[]="tenEstimateGradientsSet", err[BIFF_STRLEN];

  if (!(tec && ngrad)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!AIR_EXISTS(bValue)) {
    sprintf(err, "%s: given b value doesn't exist", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenGradientCheck(ngrad, nrrdTypeDefault, 7)) {
    sprintf(err, "%s: problem with gradient list", me);
    biffAdd(TEN, err); return 1;
  }

  tec->bValue = bValue;
  tec->_ngrad = ngrad;
  tec->_nbmat = NULL;
  tec->estimateB0 = estimateB0;

  tec->flag[flagBInfo] = AIR_TRUE;
  return 0;
}

int
tenEstimateBMatricesSet(tenEstimateContext *tec,
                        const Nrrd *nbmat, double bValue, int estimateB0) {
  char me[]="tenEstimateBMatricesSet", err[BIFF_STRLEN];

  if (!(tec && nbmat)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!AIR_EXISTS(bValue)) {
    sprintf(err, "%s: given b value doesn't exist", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenBMatrixCheck(nbmat, nrrdTypeDefault, 7)) {
    sprintf(err, "%s: problem with b-matrix list", me);
    biffAdd(TEN, err); return 1;
  }

  tec->bValue = bValue;
  tec->_ngrad = NULL;
  tec->_nbmat = nbmat;
  tec->estimateB0 = estimateB0;

  tec->flag[flagBInfo] = AIR_TRUE;
  return 0;
}

int
tenEstimateThresholdSet(tenEstimateContext *tec,
                        double thresh, double soft) {
  char me[]="tenEstimateThresholdSet", err[BIFF_STRLEN];

  if (!tec) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!(AIR_EXISTS(thresh) && AIR_EXISTS(soft))) {
    sprintf(err, "%s: not both threshold (%g) and softness (%g) exist", me,
            thresh, soft);
    biffAdd(TEN, err); return 1;
  }

  tec->dwiConfThresh = thresh;
  tec->dwiConfSoft = soft;

  return 0;
}

int
_tenEstimateCheck(tenEstimateContext *tec) {
  char me[]="_tenEstimateCheck", err[BIFF_STRLEN];

  if (!tec) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( AIR_EXISTS(tec->valueMin) && tec->valueMin > 0.0)) {
    sprintf(err, "%s: need a positive valueMin set (not %g)",
            me, tec->valueMin);
    biffAdd(TEN, err); return 1;
  }
  if (!tec->simulate) {
    if (!AIR_EXISTS(tec->bValue)) {
      sprintf(err, "%s: b-value not set", me);
      biffAdd(TEN, err); return 1;
    }
    if (airEnumValCheck(tenEstimateMethod, tec->estimateMethod)) {
      sprintf(err, "%s: estimation method not set", me);
      biffAdd(TEN, err); return 1;
    }
    if (!(AIR_EXISTS(tec->dwiConfThresh) && AIR_EXISTS(tec->dwiConfSoft))) {
      sprintf(err, "%s: not both threshold (%g) and softness (%g) exist", me,
              tec->dwiConfThresh, tec->dwiConfSoft);
      biffAdd(TEN, err); return 1;
    }
  }
  if (!( tec->_ngrad || tec->_nbmat )) {
    sprintf(err, "%s: need to set either gradients or B-matrices", me);
    biffAdd(TEN, err); return 1;
  }

  return 0;
}

int
_tenEstimateNumUpdate(tenEstimateContext *tec) {
  char me[]="_tenEstimateNumUpdate", err[BIFF_STRLEN];
  unsigned int newAllNum, newDwiNum, allIdx;
  double (*lup)(const void *, size_t), gg[3], bb[6];

  if (tec->flag[flagBInfo]) {
    if (tec->_ngrad) {
      newAllNum = AIR_CAST(unsigned int, tec->_ngrad->axis[1].size);
      lup = nrrdDLookup[tec->_ngrad->type];
    } else {
      newAllNum = AIR_CAST(unsigned int, tec->_nbmat->axis[1].size);
      lup = nrrdDLookup[tec->_nbmat->type];
    }
    if (tec->allNum != newAllNum) {
      tec->allNum = newAllNum;
      tec->flag[flagAllNum] = AIR_TRUE;
    }

    /* HEY: this should be its own update function */
    airFree(tec->bnorm);
    tec->bnorm = AIR_CAST(double *, calloc(tec->allNum, sizeof(double)));
    if (!tec->bnorm) {
      sprintf(err, "%s: couldn't allocate bnorm vec %u\n", me, tec->allNum);
      biffAdd(TEN, err); return 1;
    }
    for (allIdx=0; allIdx<tec->allNum; allIdx++) {
      if (tec->_ngrad) {
        gg[0] = lup(tec->_ngrad->data, 0 + 3*allIdx);
        gg[1] = lup(tec->_ngrad->data, 1 + 3*allIdx);
        gg[2] = lup(tec->_ngrad->data, 2 + 3*allIdx);
        bb[0] = gg[0]*gg[0];
        bb[1] = gg[1]*gg[0];
        bb[2] = gg[2]*gg[0];
        bb[3] = gg[1]*gg[1];
        bb[4] = gg[2]*gg[1];
        bb[5] = gg[2]*gg[2];
      } else {
        bb[0] = lup(tec->_nbmat->data, 0 + 6*allIdx);
        bb[1] = lup(tec->_nbmat->data, 1 + 6*allIdx);
        bb[2] = lup(tec->_nbmat->data, 2 + 6*allIdx);
        bb[3] = lup(tec->_nbmat->data, 3 + 6*allIdx);
        bb[4] = lup(tec->_nbmat->data, 4 + 6*allIdx);
        bb[5] = lup(tec->_nbmat->data, 5 + 6*allIdx);
      }
      tec->bnorm[allIdx] = sqrt(bb[0]*bb[0] + 2*bb[1]*bb[1] + 2*bb[2]*bb[2]
                                + bb[3]*bb[3] + 2*bb[4]*bb[4]
                                + bb[5]*bb[5]);
    }
    
    if (tec->estimateB0) {
      newDwiNum = tec->allNum;
    } else {
      newDwiNum = 0;
      for (allIdx=0; allIdx<tec->allNum; allIdx++) {
        newDwiNum += (0.0 != tec->bnorm[allIdx]);
      }
    }
    if (tec->dwiNum != newDwiNum) {
      tec->dwiNum = newDwiNum;
      tec->flag[flagDwiNum] = AIR_TRUE;
    }
    if (!tec->estimateB0 && (tec->allNum == tec->dwiNum)) {
      sprintf(err, "%s: don't want to estimate B0, but all values are DW", me);
      biffAdd(TEN, err); return 1;
    }
  }
  return 0;
}

int
_tenEstimateAllAllocUpdate(tenEstimateContext *tec) {
  char me[]="_tenEstimateAllAllocUpdate", err[BIFF_STRLEN];

  if (tec->flag[flagAllNum]) {
    airFree(tec->all);
    airFree(tec->allTmp);
    tec->all = AIR_CAST(double *, calloc(tec->allNum, sizeof(double)));
    tec->allTmp = AIR_CAST(double *, calloc(tec->allNum, sizeof(double)));
    if (!( tec->all && tec->allTmp )) {
      sprintf(err, "%s: couldn't allocate \"all\" arrays (length %u)", me,
              tec->allNum);
      biffAdd(TEN, err); return 1;
    }
    tec->flag[flagAllAlloc] = AIR_TRUE;
  }
  return 0;
}

int
_tenEstimateDwiAllocUpdate(tenEstimateContext *tec) {
  char me[]="_tenEstimateDwiAllocUpdate", err[BIFF_STRLEN];
  size_t size[2];
  int E;

  if (tec->flag[flagDwiNum]) {
    airFree(tec->dwi);
    airFree(tec->dwiTmp);
    tec->dwi = AIR_CAST(double *, calloc(tec->dwiNum, sizeof(double)));
    tec->dwiTmp = AIR_CAST(double *, calloc(tec->dwiNum, sizeof(double)));
    if (!(tec->dwi && tec->dwiTmp)) {
      sprintf(err, "%s: couldn't allocate DWI arrays (length %u)", me,
              tec->dwiNum);
      biffAdd(TEN, err); return 1;
    }
    E = 0;
    if (!E) size[0] = (tec->estimateB0 ? 7 : 6);
    if (!E) size[1] = tec->dwiNum;
    if (!E) E |= nrrdMaybeAlloc_nva(tec->nbmat, nrrdTypeDouble, 2, size);
    if (!E) size[0] = tec->dwiNum;
    if (!E) size[1] = tec->dwiNum;
    if (!E) E |= nrrdMaybeAlloc_nva(tec->nwght, nrrdTypeDouble, 2, size);
    if (E) {
      sprintf(err, "%s: couldn't allocate dwi nrrds", me);
      biffMove(TEN, err, NRRD); return 1;
    }
    /* nrrdSave("0-nbmat.txt", tec->nbmat, NULL); */
    tec->flag[flagDwiAlloc] = AIR_TRUE;
  }
  return 0;
}

int
_tenEstimateAllSetUpdate(tenEstimateContext *tec) {
  /* char me[]="_tenEstimateAllSetUpdate", err[BIFF_STRLEN]; */
  /* unsigned int allIdx, dwiIdx; */

  if (tec->flag[flagAllAlloc]
      || tec->flag[flagDwiNum]) {

  }
  return 0;
}

int
_tenEstimateDwiSetUpdate(tenEstimateContext *tec) {
  /* char me[]="_tenEstimateDwiSetUpdate", err[BIFF_STRLEN]; */
  double (*lup)(const void *, size_t I), gg[3], *bmat;
  unsigned int allIdx, dwiIdx, bmIdx;
  
  if (tec->flag[flagAllNum]
      || tec->flag[flagDwiAlloc]) {
    if (tec->_ngrad) {
      lup = nrrdDLookup[tec->_ngrad->type];
    } else {
      lup = nrrdDLookup[tec->_nbmat->type];
    }
    dwiIdx = 0;
    bmat = AIR_CAST(double*, tec->nbmat->data);
    for (allIdx=0; allIdx<tec->allNum; allIdx++) {
      if (tec->estimateB0 || tec->bnorm[allIdx]) {
        if (tec->_ngrad) {
          gg[0] = lup(tec->_ngrad->data, 0 + 3*allIdx);
          gg[1] = lup(tec->_ngrad->data, 1 + 3*allIdx);
          gg[2] = lup(tec->_ngrad->data, 2 + 3*allIdx);
          bmat[0] = gg[0]*gg[0];
          bmat[1] = gg[1]*gg[0];
          bmat[2] = gg[2]*gg[0];
          bmat[3] = gg[1]*gg[1];
          bmat[4] = gg[2]*gg[1];
          bmat[5] = gg[2]*gg[2];
        } else {
          for (bmIdx=0; bmIdx<6; bmIdx++) {
            bmat[bmIdx] = lup(tec->_nbmat->data, bmIdx + 6*allIdx);
          }
        }
        bmat[1] *= 2.0;
        bmat[2] *= 2.0;
        bmat[4] *= 2.0;
        if (tec->estimateB0) {
          bmat[6] = -1;
        }
        bmat += tec->nbmat->axis[0].size;
        dwiIdx++;
      }
    }
  }
  return 0;
}

int
_tenEstimateWghtUpdate(tenEstimateContext *tec) {
  /* char me[]="_tenEstimateWghtUpdate", err[BIFF_STRLEN]; */
  unsigned int dwiIdx;
  double *wght;
  
  wght = AIR_CAST(double *, tec->nwght->data);
  if (tec->flag[flagDwiAlloc]
      || tec->flag[flagEstimateMethod]) {
    
    /* HEY: this is only useful for linear LS, no? */
    for (dwiIdx=0; dwiIdx<tec->dwiNum; dwiIdx++) {
      wght[dwiIdx + tec->dwiNum*dwiIdx] = 1.0;
    }
    
    tec->flag[flagEstimateMethod] = AIR_FALSE;
    tec->flag[flagWght] = AIR_TRUE;
  }
  return 0;
}

int
_tenEstimateEmatUpdate(tenEstimateContext *tec) {
  char me[]="tenEstimateEmatUpdate", err[BIFF_STRLEN];
  
  if (tec->flag[flagDwiSet]
      || tec->flag[flagWght]) {
    
    if (!tec->simulate) {
      /* HEY: ignores weights! */
      if (ell_Nm_pseudo_inv(tec->nemat, tec->nbmat)) {
        sprintf(err, "%s: trouble pseudo-inverting %ux%u B-matrix", me,
                AIR_CAST(unsigned int, tec->nbmat->axis[1].size),
                AIR_CAST(unsigned int, tec->nbmat->axis[0].size));
        biffMove(TEN, err, ELL); return 1;
      }
    }
    /*
    nrrdSave("nbmat.txt", tec->nbmat, NULL);
    nrrdSave("nemat.txt", tec->nemat, NULL);
    */
    tec->flag[flagDwiSet] = AIR_FALSE;
    tec->flag[flagWght] = AIR_FALSE;
  }
  return 0;
}

int
tenEstimateUpdate(tenEstimateContext *tec) {
  char me[]="tenEstimateUpdate", err[BIFF_STRLEN];

  if (_tenEstimateCheck(tec)
      || _tenEstimateNumUpdate(tec)
      || _tenEstimateAllAllocUpdate(tec)
      || _tenEstimateDwiAllocUpdate(tec)
      || _tenEstimateAllSetUpdate(tec)
      || _tenEstimateDwiSetUpdate(tec)
      || _tenEstimateWghtUpdate(tec)
      || _tenEstimateEmatUpdate(tec)) {
    sprintf(err, "%s: problem updating", me);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

/*
** from given tec->all_f or tec->all_d (whichever is non-NULL), sets:
** tec->all[],
** tec->dwi[]
** tec->knownB0, if !tec->estimateB0, 
** tec->mdwi,
** tec->conf (from tec->mdwi)
*/
void
_tenEstimateValuesSet(tenEstimateContext *tec) {
  unsigned int allIdx, dwiIdx, B0Num;
  double normSum;

  if (!tec->estimateB0) {
    tec->knownB0 = 0;
  } else {
    tec->knownB0 = AIR_NAN;
  }
  normSum = 0;
  tec->mdwi = 0;
  B0Num = 0;
  dwiIdx = 0;
  for (allIdx=0; allIdx<tec->allNum; allIdx++) {
    tec->all[allIdx] = (tec->all_f 
                        ? tec->all_f[allIdx]
                        : tec->all_d[allIdx]);
    tec->mdwi += tec->bnorm[allIdx]*tec->all[allIdx];
    normSum += tec->bnorm[allIdx];
    if (tec->estimateB0 || tec->bnorm[allIdx]) {
      tec->dwi[dwiIdx++] = tec->all[allIdx];
    } else {
      tec->knownB0 += tec->all[allIdx];
      B0Num += 1;
    }
  }
  if (!tec->estimateB0) {
    tec->knownB0 /= B0Num;
  }
  tec->mdwi /= normSum;
  if (tec->dwiConfSoft > 0) {
    tec->conf = AIR_AFFINE(-1, airErf((tec->mdwi - tec->dwiConfThresh)
                                      /tec->dwiConfSoft), 1,
                           0, 1);
  } else {
    tec->conf = tec->mdwi > tec->dwiConfThresh;
  }
  return;
}

/*
** ASSUMES THAT dwiTmp[] has been stuff with all values simulated from model
*/
double
_tenEstimateErrorDwi(tenEstimateContext *tec) {
  unsigned int dwiIdx;
  double err, diff;
  
  err = 0;
  for (dwiIdx=0; dwiIdx<tec->dwiNum; dwiIdx++) {
    diff = tec->dwi[dwiIdx] - tec->dwiTmp[dwiIdx];
    err += diff*diff;
  }
  err /= tec->dwiNum;
  return err;
}
double
_tenEstimateErrorLogDwi(tenEstimateContext *tec) {
  unsigned int dwiIdx;
  double err, diff;
  
  err = 0;
  for (dwiIdx=0; dwiIdx<tec->dwiNum; dwiIdx++) {
    diff = (log(AIR_MAX(tec->valueMin, tec->dwi[dwiIdx])) 
            - log(AIR_MAX(tec->valueMin, tec->dwiTmp[dwiIdx])));
    err += diff*diff;
  }
  err /= tec->dwiNum;
  return err;
}

/*
** sets:
** tec->dwiTmp[]
** and sets of all of them, regardless of estimateB0
*/
int
_tenEstimate1TensorSimulateSingle(tenEstimateContext *tec,
                                  double sigma, double bValue, double B0,
                                  const double ten[7]) {
  char me[]="_tenEstimate1TensorSimulateSingle", err[BIFF_STRLEN];
  unsigned int dwiIdx, jj;
  double nr, ni, vv;
  const double *bmat;

  if (!( ten && ten )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( AIR_EXISTS(sigma) && sigma >= 0 
         && AIR_EXISTS(bValue) && AIR_EXISTS(B0) )) {
    sprintf(err, "%s: got bad args: sigma %g, bValue %g, B0 %g\n", me,
            sigma, bValue, B0);
    biffAdd(TEN, err); return 1;
  }

  bmat = AIR_CAST(const double *, tec->nbmat->data);
  for (dwiIdx=0; dwiIdx<tec->dwiNum; dwiIdx++) {
    vv = 0;
    for (jj=0; jj<6; jj++) {
      vv += bmat[jj]*ten[1+jj];
    }
    /*
    fprintf(stderr, "!%s: sigma = %g, bValue = %g, B0 = %g\n", me,
            sigma, bValue, B0);
    fprintf(stderr, "!%s[%u]: bmat=(%g %g %g %g %g %g)."
            "ten=(%g %g %g %g %g %g)\n",
            me, dwiIdx, 
            bmat[0], bmat[1], bmat[2], bmat[3], bmat[4], bmat[5],
            ten[1], ten[2], ten[3], ten[4], ten[5], ten[6]);
    fprintf(stderr, "!%s: %g * exp(- %g * %g) = %g * exp(%g) = "
            "%g * %g = ... \n", me,
            B0, bValue, vv, B0, -bValue*vv, B0, exp(-bValue*vv));
    */
    /* need AIR_MAX(0, vv) because B:D might be negative */
    vv = B0*exp(-bValue*AIR_MAX(0, vv));
    /*
    fprintf(stderr, "!%s: vv = %g\n", me, vv);
    */
    if (sigma > 0) {
      airNormalRand(&nr, &ni);
      nr *= sigma;
      ni *= sigma;
      vv = sqrt((vv+nr)*(vv+nr) + ni*ni);
    }
    tec->dwiTmp[dwiIdx] = vv;
    if (!AIR_EXISTS(tec->dwiTmp[dwiIdx])) {
      fprintf(stderr, "**********************************\n");
      
    }
    /*
      if (tec->verbose) {
      fprintf(stderr, "%s: dwi[%u] = %g\n", me, dwiIdx, tec->dwiTmp[dwiIdx]);
      }
    */
    bmat += tec->nbmat->axis[0].size;
  }
  return 0;
}

int
tenEstimate1TensorSimulateSingle_f(tenEstimateContext *tec,
                                   float *simval,
                                   float sigma, float bValue, float B0,
                                   const float _ten[7]) {
  char me[]="tenEstimate1TensorSimulateSingle_f", err[BIFF_STRLEN];
  unsigned int allIdx, dwiIdx;
  double ten[7];

  if (!(tec && simval && _ten)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }

  TEN_T_COPY(ten, _ten);
  if (_tenEstimate1TensorSimulateSingle(tec, sigma, bValue, B0, ten)) {
    sprintf(err, "%s: ", me);
    biffAdd(TEN, err); return 1;
  }
  dwiIdx = 0;
  for (allIdx=0; allIdx<tec->allNum; allIdx++) {
    if (tec->estimateB0 || tec->bnorm[allIdx]) {
      simval[allIdx] = AIR_CAST(float, tec->dwiTmp[dwiIdx++]);
    } else {
      simval[allIdx] = B0;
    }
  }
  return 0;
}

int
tenEstimate1TensorSimulateSingle_d(tenEstimateContext *tec,
                                   double *simval,
                                   double sigma, double bValue, double B0,
                                   const double ten[7]) {
  char me[]="tenEstimate1TensorSimulateSingle_d", err[BIFF_STRLEN];
  unsigned int allIdx, dwiIdx;
  
  if (!(tec && simval && ten)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( AIR_EXISTS(sigma) && sigma >= 0 
         && AIR_EXISTS(bValue) && AIR_EXISTS(B0) )) {
    sprintf(err, "%s: got bad bargs sigma %g, bValue %g, B0 %g\n", me,
            sigma, bValue, B0);
    biffAdd(TEN, err); return 1;
  }

  if (_tenEstimate1TensorSimulateSingle(tec, sigma, bValue, B0, ten)) {
    sprintf(err, "%s: ", me);
    biffAdd(TEN, err); return 1;
  }
  dwiIdx = 0;
  for (allIdx=0; allIdx<tec->allNum; allIdx++) {
    if (tec->estimateB0 || tec->bnorm[allIdx]) {
      simval[allIdx] = tec->dwiTmp[dwiIdx++];
    } else {
      simval[allIdx] = B0;
    }
  }
  return 0;
}

int
tenEstimate1TensorSimulateVolume(tenEstimateContext *tec,
                                 Nrrd *ndwi, 
                                 double sigma, double bValue,
                                 const Nrrd *nB0, const Nrrd *nten,
                                 int outType) {
  char me[]="tenEstimate1TensorSimulateVolume", err[BIFF_STRLEN];
  size_t sizeTen, sizeX, sizeY, sizeZ, NN, II;
  double (*tlup)(const void *, size_t), (*blup)(const void *, size_t), 
    ten_d[7], *dwi_d, B0;
  float *dwi_f, ten_f[7];
  unsigned int tt;
  int axmap[4], E;
  airArray *mop;

  if (!(tec && ndwi && nB0 && nten)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  /* this should have been done by update(), but why not */
  if (_tenEstimateCheck(tec)) {
    sprintf(err, "%s: problem in given context", me);
    biffAdd(TEN, err); return 1;
  }
  if (!(AIR_EXISTS(sigma) && sigma >= 0.0 
        && AIR_EXISTS(bValue) && bValue >= 0.0)) {
    sprintf(err, "%s: got invalid sigma (%g) or bValue (%g)\n", me,
            sigma, bValue);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(nrrdType, outType)) {
    sprintf(err, "%s: requested output type %d not valid", me, outType);
    biffAdd(TEN, err); return 1;
  }
  if (!( nrrdTypeFloat == outType || nrrdTypeDouble == outType )) {
    sprintf(err, "%s: requested output type (%s) not %s or %s", me,
            airEnumStr(nrrdType, outType),
            airEnumStr(nrrdType, nrrdTypeFloat),
            airEnumStr(nrrdType, nrrdTypeDouble));
    biffAdd(TEN, err); return 1;
  }

  mop = airMopNew();

  sizeTen = nrrdKindSize(nrrdKind3DMaskedSymMatrix);
  sizeX = nten->axis[1].size;
  sizeY = nten->axis[2].size;
  sizeZ = nten->axis[3].size;
  if (!(3 == nB0->dim &&
        sizeX == nB0->axis[0].size &&
        sizeY == nB0->axis[1].size &&
        sizeZ == nB0->axis[2].size)) {
    sprintf(err, "%s: given B0 (%u-D) volume not 3-D " _AIR_SIZE_T_CNV
            "x" _AIR_SIZE_T_CNV "x" _AIR_SIZE_T_CNV, me, nB0->dim,
            sizeX, sizeY, sizeZ);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdMaybeAlloc_va(ndwi, outType, 4,
                     AIR_CAST(size_t, tec->allNum), sizeX, sizeY, sizeZ)) {
    sprintf(err, "%s: couldn't allocate DWI output", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  NN = sizeX * sizeY * sizeZ;
  tlup = nrrdDLookup[nten->type];
  blup = nrrdDLookup[nB0->type];
  dwi_d = AIR_CAST(double *, ndwi->data);
  dwi_f = AIR_CAST(float *, ndwi->data);
  E = 0;
  for (II=0; !E && II<NN; II++) {
    B0 = blup(nB0->data, II);
    if (nrrdTypeDouble == outType) {
      for (tt=0; tt<7; tt++) {
        ten_d[tt] = tlup(nten->data, tt + sizeTen*II);
      }
      E = tenEstimate1TensorSimulateSingle_d(tec, dwi_d, sigma,
                                             bValue, B0, ten_d);
      dwi_d += tec->allNum;
    } else {
      for (tt=0; tt<7; tt++) {
        ten_f[tt] = AIR_CAST(float, tlup(nten->data, tt + sizeTen*II));
      }
      E = tenEstimate1TensorSimulateSingle_f(tec, dwi_f,
                                             AIR_CAST(float, sigma),
                                             AIR_CAST(float, bValue),
                                             AIR_CAST(float, B0),
                                             ten_f);
      dwi_f += tec->allNum;
    }
    if (E) {
      sprintf(err, "%s: failed at sample " _AIR_SIZE_T_CNV, me, II);
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
  }

  ELL_4V_SET(axmap, -1, 1, 2, 3);
  nrrdAxisInfoCopy(ndwi, nten, axmap, NRRD_AXIS_INFO_NONE);
  ndwi->axis[0].kind = nrrdKindList;
  if (nrrdBasicInfoCopy(ndwi, nten,
                        NRRD_BASIC_INFO_ALL ^ NRRD_BASIC_INFO_SPACE)) {
    sprintf(err, "%s:", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

/*
** sets:
** tec->ten[1..6]
** tec->B0, if tec->estimateB0
*/
int
_tenEstimate1Tensor_LLS(tenEstimateContext *tec) {
  char me[]="_tenEstimate1Tensor_LLS", err[BIFF_STRLEN];
  double *emat, tmp, logB0;
  unsigned int ii, jj;
  
  emat = AIR_CAST(double *, tec->nemat->data);
  if (tec->estimateB0) {
    for (ii=0; ii<tec->allNum; ii++) {
      tmp = AIR_MAX(tec->valueMin, tec->all[ii]);
      tec->allTmp[ii] = -log(tmp)/(tec->bValue);
    }
    for (jj=0; jj<7; jj++) {
      tmp = 0;
      for (ii=0; ii<tec->allNum; ii++) {
        tmp += emat[ii + tec->allNum*jj]*tec->allTmp[ii];
      }
      if (jj < 6) {
        tec->ten[1+jj] = tmp;
        if (!AIR_EXISTS(tmp)) {
          sprintf(err, "%s: estimated non-existant tensor coef (%u) %g", 
                  me, jj, tmp);
          biffAdd(TEN, err); return 1;
        }
      } else {
        /* we're on seventh row, for finding B0 */
        tec->estimatedB0 = exp(tec->bValue*tmp);
        tec->estimatedB0 = AIR_MIN(FLT_MAX, tec->estimatedB0);
        if (!AIR_EXISTS(tec->estimatedB0)) {
          sprintf(err, "%s: estimated non-existant B0 %g (b=%g, tmp=%g)", 
                  me, tec->estimatedB0, tec->bValue, tmp);
          biffAdd(TEN, err); return 1;
        }
      }
    }
  } else {
    logB0 = log(AIR_MAX(tec->valueMin, tec->knownB0));
    for (ii=0; ii<tec->dwiNum; ii++) {
      tmp = AIR_MAX(tec->valueMin, tec->dwi[ii]);
      tec->dwiTmp[ii] = (logB0 - log(tmp))/(tec->bValue);
    }
    for (jj=0; jj<6; jj++) {
      tmp = 0;
      for (ii=0; ii<tec->dwiNum; ii++) {
        tmp += emat[ii + tec->dwiNum*jj]*tec->dwiTmp[ii];
      }
      tec->ten[1+jj] = tmp;
    }
  }
  return 0;
}

int
_tenEstimate1Tensor_WLS(tenEstimateContext *tec) {
  char me[]="_tenEstimate1Tensor_WLS", err[BIFF_STRLEN];
  unsigned int dwiIdx, iter;
  double *wght, dwi, sum;

  if (!tec) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }

  wght = AIR_CAST(double *, tec->nwght->data);
  sum = 0;
  for (dwiIdx=0; dwiIdx<tec->dwiNum; dwiIdx++) {
    dwi = tec->dwi[dwiIdx];
    dwi = AIR_MAX(tec->valueMin, dwi);
    sum += dwi*dwi;
  }
  for (dwiIdx=0; dwiIdx<tec->dwiNum; dwiIdx++) {
    dwi = tec->dwi[dwiIdx];
    dwi = AIR_MAX(tec->valueMin, dwi);
    wght[dwiIdx + tec->dwiNum*dwiIdx] = dwi*dwi/sum;
  }
  if (ell_Nm_wght_pseudo_inv(tec->nemat, tec->nbmat, tec->nwght)) {
    sprintf(err, "%s(1): trouble wght-pseudo-inverting %ux%u B-matrix", me,
            AIR_CAST(unsigned int, tec->nbmat->axis[1].size),
            AIR_CAST(unsigned int, tec->nbmat->axis[0].size));
    biffMove(TEN, err, ELL);
    return 1;
  }
  /*
  nrrdSave("wght.txt", tec->nwght, NULL);
  nrrdSave("bmat.txt", tec->nbmat, NULL);
  nrrdSave("emat.txt", tec->nemat, NULL);
  */
  if (_tenEstimate1Tensor_LLS(tec)) {
    sprintf(err, "%s: initial weighted LLS failed", me);
    biffAdd(TEN, err); return 1;
  }

  for (iter=0; iter<tec->WLSIterNum; iter++) {
    /*
    fprintf(stderr, "!%s: bValue = %g, B0 = %g, ten = %g %g %g %g %g %g\n",
            me,
            tec->bValue, (tec->estimateB0 ? tec->estimatedB0 : tec->knownB0), 
            tec->ten[1], tec->ten[2], tec->ten[3], 
            tec->ten[4], tec->ten[5], tec->ten[6]);
    */
    if (_tenEstimate1TensorSimulateSingle(tec, 0.0, tec->bValue, 
                                          (tec->estimateB0 ?
                                           tec->estimatedB0 
                                           : tec->knownB0), tec->ten)) {
      sprintf(err, "%s: iter %u", me, iter);
      biffAdd(TEN, err); return 1;
    }
    for (dwiIdx=0; dwiIdx<tec->dwiNum; dwiIdx++) {
      dwi = tec->dwiTmp[dwiIdx];
      if (!AIR_EXISTS(dwi)) {
        sprintf(err, "%s: bad simulated dwi[%u] == %g (iter %u)",
                me, dwiIdx, dwi, iter);
        biffAdd(TEN, err); return 1;
      }
      wght[dwiIdx + tec->dwiNum*dwiIdx] = AIR_MAX(FLT_MIN, dwi*dwi);
    }
    if (ell_Nm_wght_pseudo_inv(tec->nemat, tec->nbmat, tec->nwght)) {
      /*
      nrrdSave("nbmat.txt", tec->nbmat, NULL);
      nrrdSave("nwght.txt", tec->nwght, NULL);
      */
      sprintf(err, "%s(2): trouble w/ %ux%u B-matrix (iter %u)", me,
              AIR_CAST(unsigned int, tec->nbmat->axis[1].size),
              AIR_CAST(unsigned int, tec->nbmat->axis[0].size), iter);
      biffMove(TEN, err, ELL);
      return 1;
    }
    _tenEstimate1Tensor_LLS(tec);
  }
  
  return 0;
}

int
_tenEstimate1TensorGradient(tenEstimateContext *tec,
                            double *gradB0P, double gradTen[7],
                            double B0, double ten[7],
                            double epsilon,
                            int (*gradientCB)(tenEstimateContext *tec,
                                              double *gradB0P,  double gTen[7],
                                              double B0, double ten[7]),
                            int (*badnessCB)(tenEstimateContext *tec,
                                             double *badP,
                                             double B0, double ten[7])) {
  char me[]="_tenEstimate1TensorGradper", err[BIFF_STRLEN];
  double forwTen[7], backTen[7], forwBad, backBad;
  unsigned int ti;

  if (!( tec && gradB0P && gradTen && badnessCB && ten)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }

  if (gradientCB) {
    if (gradientCB(tec, gradB0P, gradTen, B0, ten)) {
      sprintf(err, "%s: problem with grad callback", me);
      biffAdd(TEN, err); return 1;
    }
  } else {
    /* we find gradient manually */
    gradTen[0] = 0;
    for (ti=0; ti<6; ti++) {
      TEN_T_COPY(forwTen, ten);
      TEN_T_COPY(backTen, ten);
      forwTen[ti+1] += epsilon;
      backTen[ti+1] -= epsilon;
      if (badnessCB(tec, &forwBad, B0, forwTen)
          || badnessCB(tec, &backBad, B0, backTen)) {
        sprintf(err, "%s: trouble at ti=%u", me, ti);
        biffAdd(TEN, err); return 1;
      }
      gradTen[ti+1] = (forwBad - backBad)/(2*epsilon);
    }
  }

  return 0;
}

int
_tenEstimate1TensorDescent(tenEstimateContext *tec,
                           int (*gradientCB)(tenEstimateContext *tec,
                                             double *gradB0,
                                             double gradTen[7],
                                             double B0,
                                             double ten[7]),
                           int (*badnessCB)(tenEstimateContext *tec,
                                            double *badP,
                                            double B0,
                                            double ten[7])) {
  char me[]="_tenEstimate1TensorDescent", err[BIFF_STRLEN];
  double currB0, lastB0, currTen[7], lastTen[7], gradB0, gradTen[7],
    epsilon, 
    stepSize, badInit, bad, badDelta, stepSizeMin = 0.00000000001, badLast;
  unsigned int iter, iterMax = 100000;

  /* start with WLS fit since its probably close */
  _tenEstimate1Tensor_WLS(tec);
  if (tec->verbose) {
    fprintf(stderr, "%s: WLS gave %g %g %g    %g %g    %g\n", me, 
            tec->ten[1], tec->ten[2], tec->ten[3],
            tec->ten[4], tec->ten[5], tec->ten[6]);
  }
  
  if (badnessCB(tec, &badInit,
                (tec->estimateB0 ? tec->estimatedB0 : tec->knownB0), tec->ten)
      || !AIR_EXISTS(badInit)) {
    sprintf(err, "%s: problem getting initial bad", me);
    biffAdd(TEN, err); return 1;
  }
  if (tec->verbose) {
    fprintf(stderr, "\n%s: ________________________________________\n", me);
    fprintf(stderr, "%s: start: badInit = %g ---------------\n", me, badInit);
  }

  epsilon = 0.0000001;
 newepsilon:
  if (_tenEstimate1TensorGradient(tec, &gradB0, gradTen,
                                  (tec->estimateB0 
                                   ? tec->estimatedB0 
                                   : tec->knownB0),
                                  tec->ten, epsilon,
                                  gradientCB, badnessCB)) {
    sprintf(err, "%s: problem getting initial gradient", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( AIR_EXISTS(gradB0) || 0 <= TEN_T_NORM(gradTen) )) {
    sprintf(err, "%s: got bad gradB0 %g or zero-norm tensor grad", me, gradB0);
    biffAdd(TEN, err); return 1;
  }
  if (tec->verbose) {
    fprintf(stderr, "%s: gradTen (%s) = %g %g %g  %g %g   %g\n", me,
            gradientCB ? "analytic" : "cent-diff",
            gradTen[1], gradTen[2], gradTen[3],
            gradTen[4], gradTen[5], gradTen[6]);
  }

  stepSize = 0.1;
  do {
    stepSize /= 10;
    TEN_T_SCALE_ADD2(currTen, 1.0, tec->ten, -stepSize, gradTen);
    if (tec->estimateB0) {
      currB0 = tec->estimatedB0 + -stepSize*gradB0;
    } else {
      currB0 = tec->knownB0;
    }
    if (badnessCB(tec, &bad, currB0, currTen)
        || !AIR_EXISTS(bad)) {
      sprintf(err, "%s: problem getting badness for stepSize", me);
      biffAdd(TEN, err); return 1;
    }
    if (tec->verbose) {
      fprintf(stderr, "%s: ************ stepSize = %g --> bad = %g\n",
              me, stepSize, bad);
    }
  } while (bad > badInit && stepSize > stepSizeMin);

  if (stepSize <= stepSizeMin) {
    if (epsilon > FLT_MIN) {
      epsilon /= 10;
      fprintf(stderr, "%s: re-trying initial step w/ eps %g\n", me, epsilon);
      goto newepsilon;
    } else {
      sprintf(err, "%s: never found a usable step size", me);
      biffAdd(TEN, err); return 1;
    }
  } else if (tec->verbose) {
    sprintf(err, "%s: using step size %g\n", me, stepSize);
  }

  iter = 0;
  badLast = bad;
  do {
    iter++;
    TEN_T_COPY(lastTen, currTen);
    lastB0 = currB0;
    if (0 == (iter % 3)) {
      if (_tenEstimate1TensorGradient(tec, &gradB0, gradTen,
                                      currB0, currTen, stepSize/5,
                                      gradientCB, badnessCB)
          || !AIR_EXISTS(gradB0)) {
        sprintf(err, "%s[%u]: problem getting iter grad", me, iter);
        biffAdd(TEN, err); return 1;
      }
    }
    TEN_T_SCALE_INCR(currTen, -stepSize, gradTen);
    if (tec->estimateB0) {
      currB0 -= stepSize*gradB0;
    }
    if (badnessCB(tec, &bad, currB0, currTen)
        || !AIR_EXISTS(bad)) {
      sprintf(err, "%s[%u]: problem getting badness during grad", me, iter);
      biffAdd(TEN, err); return 1;
    }
    if (tec->verbose) {
      fprintf(stderr, "%s: %u bad = %g\n", me, iter, bad);
    }
    badDelta = bad - badLast;
    badLast = bad;
    if (badDelta > 0) {
      stepSize /= 10;
      if (tec->verbose) {
        fprintf(stderr, "%s: badDelta %g > 0 ---> stepSize = %g\n",
                me, badDelta, stepSize);
      }
      badDelta = -1;  /* bogus improvement for loop continuation */
      TEN_T_COPY(currTen, lastTen);
      currB0 = lastB0;
    }
  } while (iter < iterMax && (iter < 2 || badDelta < -0.00005));
  if (iter >= iterMax) {
    sprintf(err, "%s: didn't converge after %u iterations", me, iter);
    biffAdd(TEN, err); return 1;
  }
  if (tec->verbose) {
    fprintf(stderr, "%s: finished\n", me);
  }

  ELL_6V_COPY(tec->ten+1, currTen+1);
  tec->estimatedB0 = currB0;

  return 0;
}
                            
int
_tenEstimate1Tensor_GradientNLS(tenEstimateContext *tec, 
                                double *gradB0P, double gradTen[7],
                                double currB0, double currTen[7]) {
  char me[]="_tenEstimate1Tensor_GradientNLS", err[BIFF_STRLEN];
  double *bmat, dot, tmp, diff, scl;
  unsigned int dwiIdx;

  if (!(tec && gradB0P && gradTen && currTen)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  *gradB0P = 0;
  TEN_T_SET(gradTen, 0,   0, 0, 0,    0, 0,   0);
  bmat = AIR_CAST(double *, tec->nbmat->data);
  for (dwiIdx=0; dwiIdx<tec->dwiNum; dwiIdx++) {
    dot = ELL_6V_DOT(bmat, currTen+1);
    tmp = currB0*exp(-(tec->bValue)*dot);
    diff = tec->dwi[dwiIdx] - tmp;
    scl = 2*diff*tmp*(tec->bValue);
    ELL_6V_SCALE_INCR(gradTen+1, scl, bmat);
    bmat += tec->nbmat->axis[0].size;
    /* HEY: increment *gradB0P */
  }
  ELL_6V_SCALE_INCR(gradTen+1, 1.0/tec->dwiNum, gradTen+1);
  return 0;
}

int
_tenEstimate1Tensor_BadnessNLS(tenEstimateContext *tec,
                               double *retP,
                               double currB0, double currTen[7]) {
  char me[]="_tenEstimate1Tensor_BadnessNLS", err[BIFF_STRLEN];

  if (!(retP && tec)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err);  return 1;
  }
  if (_tenEstimate1TensorSimulateSingle(tec, 0.0, tec->bValue,
                                        currB0, currTen)) {
    sprintf(err, "%s: ", me);
    biffAdd(TEN, err); return 1;
  }
  if (tec->verbose > 2) {
    unsigned int di;
    fprintf(stderr, "%s: simdwi =", me);
    for (di=0; di<tec->dwiNum; di++) {
      fprintf(stderr, " %g", tec->dwiTmp[di]);
    }
    fprintf(stderr, "\n");
  }
  *retP = _tenEstimateErrorDwi(tec);
  if (tec->verbose > 2) {
    fprintf(stderr, "!%s: badness(%g, (%g) %g %g %g   %g %g  %g) = %g\n", 
            me, currB0, currTen[0],
            currTen[1], currTen[2], currTen[3],
            currTen[4], currTen[5],
            currTen[6], *retP);
  }
  return 0;
}

int
_tenEstimate1Tensor_NLS(tenEstimateContext *tec) {
  char me[]="_tenEstimate1Tensor_NLS", err[BIFF_STRLEN];

  if (_tenEstimate1TensorDescent(tec, 
                                 NULL
                                 /* _tenEstimate1Tensor_GradientNLS */
                                 ,
                                 _tenEstimate1Tensor_BadnessNLS)) {
    sprintf(err, "%s: ", me);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

/*
** sets:
** tec->ten[0] (from tec->conf)
** tec->time, if tec->recordTime
** tec->errorDwi, if tec->recordErrorDwi
** tec->errorLogDwi, if tec->recordErrorLogDwi
*/
int
_tenEstimate1TensorSingle(tenEstimateContext *tec) {
  char me[]="_tenEstimate1TensorSingle", err[BIFF_STRLEN];
  double time0, B0;
  int E;
  
  _tenEstimateOutputInit(tec);
  time0 = tec->recordTime ? airTime() : 0;
  _tenEstimateValuesSet(tec);
  tec->ten[0] = tec->conf;
  switch(tec->estimateMethod) {
  case tenEstimateMethodLLS:
    E = _tenEstimate1Tensor_LLS(tec);
    break;
  case tenEstimateMethodWLS:
    E = _tenEstimate1Tensor_WLS(tec);
    break;
  case tenEstimateMethodNLS:
    E = _tenEstimate1Tensor_NLS(tec);
    break;
  default:
    sprintf(err, "%s: estimation method %d unimplemented",
            me, tec->estimateMethod);
    biffAdd(TEN, err); return 1;
  }
  tec->time = tec->recordTime ? airTime() - time0 : 0;
  if (E) {
    TEN_T_SET(tec->ten, AIR_NAN, 
              AIR_NAN, AIR_NAN, AIR_NAN, 
              AIR_NAN, AIR_NAN, AIR_NAN);
    if (tec->estimateB0) {
      tec->estimatedB0 = AIR_NAN;
    }
    sprintf(err, "%s: estimation failed", me);
    biffAdd(TEN, err); return 1;
  }
  
  if (tec->recordErrorDwi
      || tec->recordErrorLogDwi) {
    B0 = tec->estimateB0 ? tec->estimatedB0 : tec->knownB0;
    if (_tenEstimate1TensorSimulateSingle(tec, 0.0, tec->bValue,
                                          B0, tec->ten)) {
      sprintf(err, "%s: simulation failed", me);
      biffAdd(TEN, err); return 1;
    }
    if (tec->recordErrorDwi) {
      tec->errorDwi = _tenEstimateErrorDwi(tec);
    }
    if (tec->recordErrorLogDwi) {
      tec->errorLogDwi = _tenEstimateErrorLogDwi(tec);
    }
  }

  return 0;
}

int
tenEstimate1TensorSingle_f(tenEstimateContext *tec,
                           float ten[7], const float *all) {
  char me[]="tenEstimate1TensorSingle_f", err[BIFF_STRLEN];

  if (!(tec && ten && all)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }

  tec->all_f = all;
  tec->all_d = NULL;
  fprintf(stderr, "!%s(%u): B0 = %g,%g\n", me, __LINE__,
          tec->knownB0, tec->estimatedB0);
  if (_tenEstimate1TensorSingle(tec)) {
    sprintf(err, "%s: ", me);
    biffAdd(TEN, err); return 1;
  }
  fprintf(stderr, "!%s(%u): B0 = %g,%g\n", me, __LINE__,
          tec->knownB0, tec->estimatedB0);
  TEN_T_COPY_TT(ten, float, tec->ten);

  return 0;
}

int
tenEstimate1TensorSingle_d(tenEstimateContext *tec,
                           double ten[7], const double *all) {
  char me[]="tenEstimate1TensorSingle_d", err[BIFF_STRLEN];

  if (!(tec && ten && all)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }

  tec->all_f = NULL;
  tec->all_d = all;
  if (_tenEstimate1TensorSingle(tec)) {
    sprintf(err, "%s: ", me);
    biffAdd(TEN, err); return 1;
  }
  TEN_T_COPY(ten, tec->ten);
  /*
    fprintf(stderr, "%s: ten = %g   %g %g %g   %g %g   %g\n", me,
            ten[0],
            ten[1], ten[2], ten[3], 
            ten[4], ten[5], 
            ten[6]);
  */
  return 0;
}

int
tenEstimate1TensorVolume4D(tenEstimateContext *tec,
                           Nrrd *nten, Nrrd **nB0P, Nrrd **nterrP,
                           const Nrrd *ndwi, int outType) {
  char me[]="tenEstimate1TensorVolume4D", err[BIFF_STRLEN];
  char doneStr[20];
  size_t sizeTen, sizeX, sizeY, sizeZ, NN, II, tick;
  double *all, ten[7], (*lup)(const void *, size_t),
    (*ins)(void *v, size_t I, double d);
  unsigned int dd;
  airArray *mop;
  int axmap[4];

#if 0
#define NUM 800
  double val[NUM], minVal=0, maxVal=10, arg;
  unsigned int valIdx;
  Nrrd *nval;
  for (valIdx=0; valIdx<NUM; valIdx++) {
    arg = AIR_AFFINE(0, valIdx, NUM-1, minVal, maxVal);
    if (_tenRician(val + valIdx, arg, 1, 1)) {
      sprintf(err, "%s: you are out of luck", me);
      biffAdd(TEN, err); return 1;
    }
  }
  nval = nrrdNew();
  nrrdWrap(nval, val, nrrdTypeDouble, 1, AIR_CAST(size_t, NUM));
  nrrdSave("nval.nrrd", nval, NULL);
  nrrdNix(nval);
#endif

  if (!(tec && nten && ndwi)) {
    /* nerrP and _NB0P can be NULL */
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdCheck(ndwi)) {
    sprintf(err, "%s: DWI volume not valid", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (!( 4 == ndwi->dim && 7 <= ndwi->axis[0].size )) {
    sprintf(err, "%s: DWI volume should be 4-D with axis 0 size >= 7", me);
    biffAdd(TEN, err); return 1;
  }
  if (tec->allNum != ndwi->axis[0].size) {
    sprintf(err, "%s: from %s info, expected %u values per sample, "
            "but have " _AIR_SIZE_T_CNV " in volume", me,
            tec->_ngrad ? "gradient" : "B-matrix", tec->allNum,
            ndwi->axis[0].size);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdTypeBlock == ndwi->type) {
    sprintf(err, "%s: DWI volume has non-scalar type %s", me,
            airEnumStr(nrrdType, ndwi->type));
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(nrrdType, outType)) {
    sprintf(err, "%s: requested output type %d not valid", me, outType);
    biffAdd(TEN, err); return 1;
  }
  if (!( nrrdTypeFloat == outType || nrrdTypeDouble == outType )) {
    sprintf(err, "%s: requested output type (%s) not %s or %s", me,
            airEnumStr(nrrdType, outType),
            airEnumStr(nrrdType, nrrdTypeFloat),
            airEnumStr(nrrdType, nrrdTypeDouble));
    biffAdd(TEN, err); return 1;
  }
  if (nterrP) {
    int recE, recEL;
    recE = !!(tec->recordErrorDwi);
    recEL = !!(tec->recordErrorLogDwi);
    if (1 != recE + recEL) {
      sprintf(err, "%s: requested error volume but need exactly one of "
              "recordErrorDwi, recordErrorLogDwi to be set", me);
      biffAdd(TEN, err); return 1;
    }
  }

  mop = airMopNew();

  sizeTen = nrrdKindSize(nrrdKind3DMaskedSymMatrix);
  sizeX = ndwi->axis[1].size;
  sizeY = ndwi->axis[2].size;
  sizeZ = ndwi->axis[3].size;
  all = AIR_CAST(double *, calloc(tec->allNum, sizeof(double)));
  if (!all) {
    sprintf(err, "%s: couldn't allocate length %u array", me, tec->allNum);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, all, airFree, airMopAlways);

  if (nrrdMaybeAlloc_va(nten, outType, 4,
                        sizeTen, sizeX, sizeY, sizeZ)) {
    sprintf(err, "%s: couldn't allocate tensor output", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  if (nB0P) {
    *nB0P = nrrdNew();
    if (nrrdMaybeAlloc_va(*nB0P, outType, 3,
                          sizeX, sizeY, sizeZ)) {
      sprintf(err, "%s: couldn't allocate B0 output", me);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
    airMopAdd(mop, *nB0P, (airMopper)nrrdNuke, airMopOnError);
    airMopAdd(mop, nB0P, (airMopper)airSetNull, airMopOnError);
  }
  if (nterrP) {
    *nterrP = nrrdNew();
    if (nrrdMaybeAlloc_va(*nterrP, outType, 3,
                          sizeX, sizeY, sizeZ)
        || nrrdBasicInfoCopy(*nterrP, ndwi,
                             NRRD_BASIC_INFO_DATA_BIT
                             | NRRD_BASIC_INFO_TYPE_BIT
                             | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                             | NRRD_BASIC_INFO_DIMENSION_BIT
                             | NRRD_BASIC_INFO_CONTENT_BIT
                             | NRRD_BASIC_INFO_MEASUREMENTFRAME_BIT
                             | NRRD_BASIC_INFO_COMMENTS_BIT
                             | (nrrdStateKeyValuePairsPropagate
                                ? 0
                                : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
      sprintf(err, "%s: couldn't creatting fitting error output", me);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
    ELL_3V_SET(axmap, 1, 2, 3);
    nrrdAxisInfoCopy(*nterrP, ndwi, axmap, NRRD_AXIS_INFO_NONE);
    airMopAdd(mop, *nterrP, (airMopper)nrrdNuke, airMopOnError);
    airMopAdd(mop, nterrP, (airMopper)airSetNull, airMopOnError);
  }
  NN = sizeX * sizeY * sizeZ;
  lup = nrrdDLookup[ndwi->type];
  ins = nrrdDInsert[outType];
  if (tec->progress) {
    fprintf(stderr, "%s:       ", me); 
  }
  fflush(stderr);
  tick = NN / 200;
  for (II=0; II<NN; II++) {
    if (tec->progress && 0 == II%tick) {
      fprintf(stderr, "%s", airDoneStr(0, II, NN-1, doneStr));
    }
    for (dd=0; dd<tec->allNum; dd++) {
      all[dd] = lup(ndwi->data, dd + tec->allNum*II);
    }
    if (tenEstimate1TensorSingle_d(tec, ten, all)) {
      sprintf(err, "%s: failed at sample " _AIR_SIZE_T_CNV, me, II);
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
    ins(nten->data, 0 + sizeTen*II, ten[0]);
    ins(nten->data, 1 + sizeTen*II, ten[1]);
    ins(nten->data, 2 + sizeTen*II, ten[2]);
    ins(nten->data, 3 + sizeTen*II, ten[3]);
    ins(nten->data, 4 + sizeTen*II, ten[4]);
    ins(nten->data, 5 + sizeTen*II, ten[5]);
    ins(nten->data, 6 + sizeTen*II, ten[6]);
    if (nB0P) {
      ins((*nB0P)->data, II, (tec->estimateB0 
                              ? tec->estimatedB0
                              : tec->knownB0));
    }
    if (nterrP) {
      /* this works because above we checked that only one of the 
         tec->record* flags is set */
      if (tec->recordErrorDwi) {
        ins((*nterrP)->data, II, tec->errorDwi);
      } else if (tec->recordErrorLogDwi) {
        ins((*nterrP)->data, II, tec->errorLogDwi);
      }
    }
  }
  if (tec->progress) {
    fprintf(stderr, "%s\n", airDoneStr(0, II, NN-1, doneStr));
  }

  ELL_4V_SET(axmap, -1, 1, 2, 3);
  nrrdAxisInfoCopy(nten, ndwi, axmap, NRRD_AXIS_INFO_NONE);
  nten->axis[0].kind = nrrdKind3DMaskedSymMatrix;
  if (nrrdBasicInfoCopy(nten, ndwi,
                        NRRD_BASIC_INFO_ALL ^ NRRD_BASIC_INFO_SPACE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }

  airMopOkay(mop);
  return 0;
}
