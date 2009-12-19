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

tenEMBimodalParm* 
tenEMBimodalParmNew() {
  tenEMBimodalParm *biparm;
  
  biparm = (tenEMBimodalParm*)calloc(1, sizeof(tenEMBimodalParm));
  if (biparm) {
    biparm->minProb = 0.0001;
    biparm->minProb2 = 0.0001;
    biparm->minDelta = 0.00001;
    biparm->minFraction = 0.05;  /* 5% */
    biparm->minConfidence = 0.7;
    biparm->maxIteration = 200;
    biparm->verbose = AIR_FALSE;

    biparm->histo = NULL;
    biparm->pp1 = biparm->pp2 = NULL;
    biparm->vmin = biparm->vmax = AIR_NAN;
    biparm->N = 0;
  }
  return biparm;
}

tenEMBimodalParm* 
tenEMBimodalParmNix(tenEMBimodalParm *biparm) {

  if (biparm) {
    biparm->histo = (double *)airFree(biparm->histo);
    biparm->pp1 = (double *)airFree(biparm->pp1);
    biparm->pp2 = (double *)airFree(biparm->pp2);
  }
  airFree(biparm);
  return NULL;
}

int
_tenEMBimodalInit(tenEMBimodalParm *biparm, const Nrrd *_nhisto) {
  char me[]="_tenEMBimodalInit", err[BIFF_STRLEN];
  int i, median;
  Nrrd *nhisto;
  double medianD, sum;
  airArray *mop;

  if (!( biparm->maxIteration > 5 )) {
    sprintf(err, "%s: biparm->maxIteration = %d too small", me, 
            biparm->maxIteration);
    biffAdd(TEN, err); return 1;
  }
  
  mop = airMopNew();
  nhisto = nrrdNew();
  airMopAdd(mop, nhisto, (airMopper)nrrdNuke, airMopOnError);
  airMopAdd(mop, nhisto, (airMopper)nrrdNix, airMopOnOkay);
  if (nrrdConvert(nhisto, _nhisto, nrrdTypeDouble)) {
    sprintf(err, "%s: trouble converting histogram to double", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  biparm->N = nhisto->axis[0].size;
  biparm->histo = (double*)(nhisto->data);
  biparm->vmin = (AIR_EXISTS(nhisto->axis[0].min)
                  ? nhisto->axis[0].min
                  : -0.5);
  biparm->vmax = (AIR_EXISTS(nhisto->axis[0].max)
                  ? nhisto->axis[0].max
                  : biparm->N - 0.5);

  (nrrdMeasureLine[nrrdMeasureHistoMedian])
    (&medianD, nrrdTypeDouble,
     biparm->histo, nrrdTypeDouble, biparm->N,
     AIR_NAN, AIR_NAN);
  (nrrdMeasureLine[nrrdMeasureSum])
    (&sum, nrrdTypeDouble,
     biparm->histo, nrrdTypeDouble, biparm->N,
     AIR_NAN, AIR_NAN);
  for (i=0; i<biparm->N; i++) {
    biparm->histo[i] /= sum;
  }
  if (!AIR_EXISTS(medianD)) {
    sprintf(err, "%s: got empty histogram? (median calculation failed)", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  median = (int)medianD;

  biparm->pp1 = (double*)calloc(biparm->N, sizeof(double));
  biparm->pp2 = (double*)calloc(biparm->N, sizeof(double));
  if (!( biparm->pp1 && biparm->pp2 )) {
    sprintf(err, "%s: couldn't allocate posterior prob. buffers", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }

  /* get mean and stdv of bins below median */
  (nrrdMeasureLine[nrrdMeasureHistoMean])
    (&(biparm->mean1), nrrdTypeDouble, 
     biparm->histo, nrrdTypeDouble, median,
     AIR_NAN, AIR_NAN);
  (nrrdMeasureLine[nrrdMeasureHistoSD])
    (&(biparm->stdv1), nrrdTypeDouble, 
     biparm->histo, nrrdTypeDouble, median,
     AIR_NAN, AIR_NAN);

  /* get mean (shift upwards by median) and stdv of bins above median */
  (nrrdMeasureLine[nrrdMeasureHistoMean])
    (&(biparm->mean2), nrrdTypeDouble, 
     biparm->histo + median, nrrdTypeDouble, biparm->N - median,
     AIR_NAN, AIR_NAN);
  (nrrdMeasureLine[nrrdMeasureHistoSD])
    (&(biparm->stdv2), nrrdTypeDouble, 
     biparm->histo + median, nrrdTypeDouble, biparm->N - median,
     AIR_NAN, AIR_NAN);

  biparm->mean2 += median;
  biparm->fraction1 = 0.5;

  if (biparm->verbose) {
    fprintf(stderr, "%s: median = %d\n", me, median);
    fprintf(stderr, "%s: m1, s1 = %g, %g; m2, s2 = %g, %g\n", me,
            biparm->mean1, biparm->stdv1,
            biparm->mean2, biparm->stdv2);
  }
  
  airMopOkay(mop);
  return 0;
}

void
_tenEMBimodalBoost(double *pp1P, double *pp2P, double b) {
  double p1, p2, tmp;
  int sw=AIR_FALSE;

  if (*pp1P < *pp2P) {
    ELL_SWAP2(*pp1P, *pp2P, tmp);
    sw = AIR_TRUE;
  }
  p1 = 1 - pow(1 - *pp1P, b);
  p2 = 1 - p1;
  if (sw) {
    *pp1P = p2;
    *pp2P = p1;
  } else {
    *pp1P = p1;
    *pp2P = p2;
  }
}

/*
** what is posterior probability that measured value x comes from
** material 1 and 2, stored in pp1 and pp2
*/
void
_tenEMBimodalPP(tenEMBimodalParm *biparm) {
  int i;
  double g1, g2, pp1, pp2, f1, min;
  
  min = (1 == biparm->stage 
         ? biparm->minProb 
         : biparm->minProb2);
  f1 = biparm->fraction1;
  for (i=0; i<biparm->N; i++) {
    g1 = airGaussian(i, biparm->mean1, biparm->stdv1);
    g2 = airGaussian(i, biparm->mean2, biparm->stdv2);
    if (g1 <= min && g2 <= min) {
      pp1 = pp2 = 0;
    } else {
      pp1 = f1*g1 / (f1*g1 + (1-f1)*g2);
      pp2 = 1 - pp1;
    }
    biparm->pp1[i] = pp1;
    biparm->pp2[i] = pp2;
  }

  if (biparm->verbose > 1) {
    Nrrd *ntmp = nrrdNew();
    nrrdWrap_va(ntmp, biparm->pp1, nrrdTypeDouble, 1,
                AIR_CAST(size_t, biparm->N));
    nrrdSave("pp1.nrrd", ntmp, NULL);
    nrrdWrap_va(ntmp, biparm->pp2, nrrdTypeDouble, 1,
                AIR_CAST(size_t, biparm->N));
    nrrdSave("pp2.nrrd", ntmp, NULL);
    nrrdNix(ntmp);
  }

  return;
}

double
_tenEMBimodalNewFraction1(tenEMBimodalParm *biparm) {
  int i;
  double pp1, pp2, h, sum1, sum2;

  sum1 = sum2 = 0.0;
  for (i=0; i<biparm->N; i++) {
    pp1 = biparm->pp1[i];
    pp2 = biparm->pp2[i];
    h = biparm->histo[i];
    sum1 += pp1*h;
    sum2 += pp2*h;
  }
  return sum1/(sum1 + sum2);
}

void
_tenEMBimodalNewMean(double *m1P, double *m2P,
                     tenEMBimodalParm *biparm) {
  int i;
  double pp1, pp2, h, sum1, isum1, sum2, isum2;
  
  sum1 = isum1 = sum2 = isum2 = 0.0;
  for (i=0; i<biparm->N; i++) {
    pp1 = biparm->pp1[i];
    pp2 = biparm->pp2[i];
    h = biparm->histo[i];
    isum1 += i*pp1*h;
    isum2 += i*pp2*h;
    sum1 += pp1*h;
    sum2 += pp2*h;
  }
  *m1P = isum1/sum1;
  *m2P = isum2/sum2;
}

void
_tenEMBimodalNewSigma(double *s1P, double *s2P,
                      double m1, double m2, 
                      tenEMBimodalParm *biparm) {
  int i;
  double pp1, pp2, h, sum1, isum1, sum2, isum2;
  
  sum1 = isum1 = sum2 = isum2 = 0.0;
  for (i=0; i<biparm->N; i++) {
    pp1 = biparm->pp1[i];
    pp2 = biparm->pp2[i];
    h = biparm->histo[i];
    isum1 += (i-m1)*(i-m1)*pp1*h;
    isum2 += (i-m2)*(i-m2)*pp2*h;
    sum1 += pp1*h;
    sum2 += pp2*h;
  }
  *s1P = sqrt(isum1/sum1);
  *s2P = sqrt(isum2/sum2);
}

void
_tenEMBimodalSaveImage(tenEMBimodalParm *biparm) {
  char name[AIR_STRLEN_MED];
  Nrrd *nh, *nm, *nhi, *nmi, *ni;
  NrrdRange *range;
  const Nrrd *nhmhi[3];
  double *m, max;
  int i;
  
  nh = nrrdNew();
  nm = nrrdNew();
  nhi = nrrdNew();
  nmi = nrrdNew();
  ni = nrrdNew();
  nrrdWrap_va(nh, biparm->histo, nrrdTypeDouble, 1,
              AIR_CAST(size_t, biparm->N));
  range = nrrdRangeNewSet(nh, nrrdBlind8BitRangeFalse);
  max = range->max*1.1;
  nrrdRangeNix(range);
  nrrdCopy(nm, nh);
  m = (double*)(nm->data);
  for (i=0; i<biparm->N; i++) {
    m[i] = biparm->fraction1*airGaussian(i, biparm->mean1, biparm->stdv1);
    m[i] += (1-biparm->fraction1)*airGaussian(i, biparm->mean2, biparm->stdv2);
  }
  nrrdHistoDraw(nmi, nm, 400, AIR_FALSE, max);
  nrrdHistoDraw(nhi, nh, 400, AIR_FALSE, max);
  ELL_3V_SET(nhmhi, nhi, nmi, nhi);
  nrrdJoin(ni, nhmhi, 3, 0, AIR_TRUE);
  sprintf(name, "%04d-%d.png", biparm->iteration, biparm->stage);
  nrrdSave(name, ni, NULL);
  nh = nrrdNix(nh);
  nm = nrrdNuke(nm);
  nhi = nrrdNuke(nhi);
  nmi = nrrdNuke(nmi);
  ni = nrrdNuke(ni);
  return;
}


int
_tenEMBimodalIterate(tenEMBimodalParm *biparm) {
  char me[]="_tenEMBimodalIterate";
  double om1, os1, om2, os2, of1, m1, s1, m2, s2, f1;

  /* copy old values */
  om1 = biparm->mean1;
  os1 = biparm->stdv1;
  of1 = biparm->fraction1;
  om2 = biparm->mean2;
  os2 = biparm->stdv2;

  /* find new values, and calculate delta */
  _tenEMBimodalPP(biparm);
  f1 = _tenEMBimodalNewFraction1(biparm);
  /*   if (1 == biparm->stage) { */
    _tenEMBimodalNewMean(&m1, &m2, biparm);
    /*   } */
  _tenEMBimodalNewSigma(&s1, &s2, m1, m2, biparm);

  biparm->delta = ((fabs(m1 - om1) + fabs(m2 - om2)
                    + fabs(s1 - os1) + fabs(s2 - os2))/biparm->N
                   + fabs(f1 - of1));
  
  /* set new values */
  biparm->mean1 = m1;
  biparm->stdv1 = s1;
  biparm->fraction1 = f1;
  biparm->mean2 = m2;
  biparm->stdv2 = s2;

  if (biparm->verbose) {
    fprintf(stderr, "%s(%d:%d):\n", me, biparm->stage, biparm->iteration);
    fprintf(stderr, "  m1, s1 = %g, %g\n", m1, s1);
    fprintf(stderr, "  m2, s2 = %g, %g\n", m2, s2);
    fprintf(stderr, "  f1 = %g ; delta = %g\n", f1, biparm->delta);
  }
  if (biparm->verbose > 1) {
    _tenEMBimodalSaveImage(biparm);
  }
  return 0;
}

int
_tenEMBimodalConfThresh(tenEMBimodalParm *biparm) {
  char me[]="_tenEMBimodalConfThresh", err[BIFF_STRLEN];
  double m1, s1, m2, s2, f1, f2, A, B, C, D, t1, t2;

  biparm->confidence = ((biparm->mean2 - biparm->mean1)
                        / (biparm->stdv1 + biparm->stdv2));
  m1 = biparm->mean1;
  s1 = biparm->stdv1;
  f1 = biparm->fraction1;
  m2 = biparm->mean2;
  s2 = biparm->stdv2;
  f2 = 1 - f1;
  A = s1*s1 - s2*s2;
  B = 2*(m1*s2*s2 - m2*s1*s1);
  C = s1*s1*m2*m2 - s2*s2*m1*m1 + 4*s1*s1*s2*s2*log(s2*f1/(s1*f2));
  D = B*B - 4*A*C;
  if (D < 0) {
    sprintf(err, "%s: threshold descriminant went negative (%g)", me, D);
    biffAdd(TEN, err); return 1;
  }
  t1 = (-B + sqrt(D))/(2*A);
  if (AIR_IN_OP(m1, t1, m2)) {
    biparm->threshold = t1;
  } else {
    t2 = (-B - sqrt(D))/(2*A);
    if (AIR_IN_OP(m1, t2, m2)) {
      biparm->threshold = t2;
    } else {
      sprintf(err, "%s: neither computed threshold %g,%g inside open interval "
              "between means (%g,%g)", me, t1, t2, m1, m2);
      biffAdd(TEN, err); return 1;
    }
  }

  if (biparm->verbose) {
    fprintf(stderr, "%s: conf = %g, thresh = %g\n", me, 
            biparm->confidence, biparm->threshold);
  }
  return 0;
}

int
_tenEMBimodalCheck(tenEMBimodalParm *biparm) {
  char me[]="_tenEMBimodalCheck", err[BIFF_STRLEN];

  if (!( biparm->confidence > biparm->minConfidence )) {
    sprintf(err, "%s: confidence %g went below threshold %g", me,
            biparm->confidence, biparm->minConfidence);
    biffAdd(TEN, err); return 1;
  }
  if (!( biparm->stdv1 > 0 && biparm->stdv2 > 0 )) {
    sprintf(err, "%s: stdv of material 1 (%g) or 2 (%g) went negative", me,
            biparm->stdv1, biparm->stdv2);
    biffAdd(TEN, err); return 1;
  }
  if (!( biparm->mean1 > 0 && biparm->mean1 < biparm->N-1
         && biparm->mean2 > 0 && biparm->mean2 < biparm->N-1 )) {
    sprintf(err, "%s: mean of material 1 (%g) or 2 (%g) went outside "
            "given histogram range [0 .. %d]", me,
            biparm->mean1, biparm->mean2, biparm->N-1);
    biffAdd(TEN, err); return 1;
  }
  if (biparm->fraction1 < biparm->minFraction) {
    sprintf(err, "%s: material 1 fraction (%g) fell below threshold %g", me,
            biparm->fraction1, biparm->minFraction);
    biffAdd(TEN, err); return 1;
  }
  if (1 - biparm->fraction1 < biparm->minFraction) {
    sprintf(err, "%s: material 2 fraction (%g) fell below threshold %g", me,
            1 - biparm->fraction1, biparm->minFraction);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

int
tenEMBimodal(tenEMBimodalParm *biparm, const Nrrd *_nhisto) {
  char me[]="tenEMBimodal", err[BIFF_STRLEN];
  int done, _iter;
  
  if (!(biparm && _nhisto)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( 1 == _nhisto->dim )) {
    sprintf(err, "%s: histogram must be 1-D, not %d-D", me, _nhisto->dim);
    biffAdd(TEN, err); return 1;
  }

  if (_tenEMBimodalInit(biparm, _nhisto)) {
    sprintf(err, "%s: trouble initializing parameters", me);
    biffAdd(TEN, err); return 1;
  }

  done = AIR_FALSE;
  biparm->iteration = 0;
  for (biparm->stage = 1; 
       biparm->stage <= (biparm->twoStage ? 2 : 1);
       biparm->stage++) {
    for (_iter=0; 
         biparm->iteration <= biparm->maxIteration; 
         biparm->iteration++, _iter++) {
      if (_tenEMBimodalIterate(biparm)    /* sets delta */
          || _tenEMBimodalConfThresh(biparm)
          || _tenEMBimodalCheck(biparm)) {
        sprintf(err, "%s: problem with fitting (iter=%d)", me,
                biparm->iteration);
        biffAdd(TEN, err); return 1;
      }
      if (biparm->delta < biparm->minDelta
          && (!biparm->twoStage || 1 == biparm->stage || _iter > 10) ) {
        done = AIR_TRUE;
        break;
      }
    }
  }
  if (!done) {
    sprintf(err, "%s: didn't converge after %d iterations", me, 
            biparm->maxIteration);
    biffAdd(TEN, err); return 1;
  }
  
  return 0;
}
