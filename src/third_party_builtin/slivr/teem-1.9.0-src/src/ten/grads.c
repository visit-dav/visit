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

tenGradientParm *
tenGradientParmNew(void) {
  tenGradientParm *ret;

  ret = (tenGradientParm *)calloc(1, sizeof(tenGradientParm));
  if (ret) {
    ret->mass = 1;
    ret->charge = 0.2;
    ret->drag = 0.01;
    ret->dt = 0.05;
    ret->jitter = 0.05;
    ret->minVelocity = 0.000001;
    ret->minMean = 0.0001;
    ret->minMeanImprovement = 0.00001;
    ret->srand = AIR_TRUE;
    ret->snap = 0;
    ret->single = AIR_FALSE;
    ret->minIteration = 20;
    ret->maxIteration = 1000000;
  }
  return ret;
}

tenGradientParm *
tenGradientParmNix(tenGradientParm *tgparm) {
  
  airFree(tgparm);
  return NULL;
}


int
tenGradientCheck(const Nrrd *ngrad, int type, unsigned int minnum) {
  char me[]="tenGradientCheck", err[BIFF_STRLEN];
  
  if (nrrdCheck(ngrad)) {
    sprintf(err, "%s: basic validity check failed", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (!( 3 == ngrad->axis[0].size && 2 == ngrad->dim )) {
    sprintf(err, "%s: need a 3xN 2-D array (not a " _AIR_SIZE_T_CNV 
            "x? %u-D array)",
            me, ngrad->axis[0].size, ngrad->dim);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdTypeDefault != type && type != ngrad->type) {
    sprintf(err, "%s: requested type %s but got type %s", me,
            airEnumStr(nrrdType, type), airEnumStr(nrrdType, ngrad->type));
    biffAdd(TEN, err); return 1;
  }
  if (nrrdTypeBlock == ngrad->type) {
    sprintf(err, "%s: sorry, can't use %s type", me, 
            airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(TEN, err); return 1;
  }
  if (!( minnum <= ngrad->axis[1].size )) {
    sprintf(err, "%s: have only " _AIR_SIZE_T_CNV " gradients, "
            "need at least %d",
            me, ngrad->axis[1].size, minnum);
    biffAdd(TEN, err); return 1;
  }

  return 0;
}

/*
******** tenGradientRandom
**
** generates num random unit vectors of type double
*/
int
tenGradientRandom(Nrrd *ngrad, unsigned int num, int srand) {
  char me[]="tenGradientRandom", err[BIFF_STRLEN];
  double *grad, len;
  unsigned int gi;
  
  if (nrrdMaybeAlloc_va(ngrad, nrrdTypeDouble, 2,
                        AIR_CAST(size_t, 3), AIR_CAST(size_t, num))) {
    sprintf(err, "%s: couldn't allocate output", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (srand) {
    airSrandMT(AIR_CAST(unsigned int, airTime()));
  }
  grad = (double*)(ngrad->data);
  for (gi=0; gi<num; gi++) {
    do {
      grad[0] = AIR_AFFINE(0, airDrandMT(), 1, -1, 1);
      grad[1] = AIR_AFFINE(0, airDrandMT(), 1, -1, 1);
      grad[2] = AIR_AFFINE(0, airDrandMT(), 1, -1, 1);
      len = ELL_3V_LEN(grad);
    } while (len > 1 || !len);
    ELL_3V_SCALE(grad, 1.0/len, grad);
    grad += 3;
  }
  return 0;
}

/*
******** tenGradientJitter
**
** moves all gradients by amount dist on tangent plane, in a random
** direction, and then renormalizes
*/
int
tenGradientJitter(Nrrd *nout, const Nrrd *nin, double dist) {
  char me[]="tenGradientJitter", err[BIFF_STRLEN];
  double *grad, perp0[3], perp1[3], len, theta, cc, ss;
  unsigned int gi;

  if (nrrdConvert(nout, nin, nrrdTypeDouble)) {
    sprintf(err, "%s: trouble converting input to double", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (tenGradientCheck(nout, nrrdTypeDouble, 3)) {
    sprintf(err, "%s: didn't get valid gradients", me);
    biffAdd(TEN, err); return 1;
  }
  grad = (double*)(nout->data);
  for (gi=0; gi<nout->axis[1].size; gi++) {
    ELL_3V_NORM(grad, grad, len);
    ell_3v_perp_d(perp0, grad);
    ELL_3V_CROSS(perp1, perp0, grad);
    theta = AIR_AFFINE(0, airDrandMT(), 1, 0, 2*AIR_PI);
    cc = dist*cos(theta);
    ss = dist*sin(theta);
    ELL_3V_SCALE_ADD3(grad, 1.0, grad, cc, perp0, ss, perp1);
    ELL_3V_NORM(grad, grad, len);
    grad += 3;
  }
  
  return 0;
}

void
_tenGradientChangeFind(Nrrd *ndvdt, Nrrd *ndpdt,
                       Nrrd *nvel, Nrrd *npos,
                       tenGradientParm *tgparm) {
  double *dvdt, *dpdt, *vel, *pos, qq, force[3], ff[3],
    pdir[3], mdir[3], plen, mlen, prep, mrep;
  int num, ii, jj;
  
  dvdt = (double *)(ndvdt->data);
  dpdt = (double *)(ndpdt->data);
  vel = (double *)(nvel->data);
  pos = (double *)(npos->data);
  num = ndvdt->axis[1].size;
  qq = (tgparm->charge)*(tgparm->charge);

  for (ii=0; ii<num; ii++) {
    /* find forces acting on ii from position of all other pairs jj */
    ELL_3V_SET(force, 0, 0, 0);
    for (jj=0; jj<num; jj++) {
      if (ii == jj) {
        continue;
      }
      ELL_3V_SUB(pdir, pos + 3*ii, pos + 3*jj);
      ELL_3V_NORM(pdir, pdir, plen);
      prep = qq/(plen*plen);
      if (tgparm->single) {
        mrep = 0;
        ELL_3V_SET(mdir, 0, 0, 0);
      } else {
        ELL_3V_ADD2(mdir, pos + 3*ii, pos + 3*jj);
        ELL_3V_NORM(mdir, mdir, mlen);
        mrep = qq/(mlen*mlen);
      }
      ELL_3V_SCALE_ADD3(ff,
                        prep, pdir,
                        mrep, mdir,
                        -tgparm->drag, vel + 3*ii);
      ELL_3V_ADD2(force, force, ff);
    }
    ELL_3V_SCALE(dvdt + 3*ii, 1.0/tgparm->mass, force);
    ELL_3V_COPY(dpdt + 3*ii, vel + 3*ii);
  }
  return;
}

void
_tenGradientChangeApply(Nrrd *nvel1, Nrrd *npos1,
                        Nrrd *ndvdt, Nrrd *ndpdt, 
                        Nrrd *nvel0, Nrrd *npos0,
                        tenGradientParm *tgparm, double amount) {
  double *dvdt, *dpdt, *vel0, *pos0, *vel1, *pos1, len, dot;
  int ii, num;

  dvdt = (double *)(ndvdt->data);
  dpdt = (double *)(ndpdt->data);
  vel0 = (double *)(nvel0->data);
  pos0 = (double *)(npos0->data);
  vel1 = (double *)(nvel1->data);
  pos1 = (double *)(npos1->data);
  num = ndvdt->axis[1].size;

  for (ii=0; ii<num; ii++) {
    ELL_3V_SCALE_ADD2(vel1,
                      1.0, vel0,
                      amount*tgparm->dt, dvdt);
    ELL_3V_SCALE_ADD2(pos1,
                      1.0, pos0,
                      amount*tgparm->dt, dpdt);
    /* impose constraints: pos is unit-length vector ... */
    ELL_3V_NORM(pos1, pos1, len);
    dot = ELL_3V_DOT(vel1, pos1);
    /* ... and velocity must be tangential */
    ELL_3V_SCALE_ADD2(vel1, 1.0, vel1, -dot, pos1);
    vel1 += 3;
    pos1 += 3;
    vel0 += 3;
    pos0 += 3;
    dvdt += 3;
    dpdt += 3;
  }
  
  return;
}

double
_tenGradientMeanVelocity(Nrrd *nvel) {
  double *vel, mv;
  int ii, num;

  vel = (double*)(nvel->data);
  num = nvel->axis[1].size;
  mv = 0;
  for (ii=0; ii<num; ii++) {
    mv += ELL_3V_LEN(vel + 3*ii);
  }
  mv /= num;
  return mv;
}

double
_tenGradientParty(double *grad, int num) {
  double mean[3];
  int ii;
  
  ELL_3V_SET(mean, 0, 0, 0);
  for (ii=0; ii<num; ii++) {
    if (airRandInt(2)) {
      ELL_3V_SCALE(grad + 3*ii, -1, grad + 3*ii);
    }
    ELL_3V_INCR(mean, grad + 3*ii);
  }
  ELL_3V_SCALE(mean, 1.0/num, mean);
  return ELL_3V_LEN(mean);
}

int
tenGradientMeanMinimize(Nrrd *nout, const Nrrd *nin,
                        tenGradientParm *tgparm) {
  char me[]="tenGradientMeanMinimize", err[BIFF_STRLEN];
  int num;
  double *grad, len, lastLen, improv;

  if (!nout || tenGradientCheck(nin, nrrdTypeUnknown, 2)) {
    sprintf(err, "%s: got NULL pointer or invalid input", me);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdConvert(nout, nin, nrrdTypeDouble)) {
    sprintf(err, "%s: can't initialize output with input", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  num = nout->axis[1].size;
  grad = (double*)(nout->data);
   
  lastLen = _tenGradientParty(grad, num);
  do {
    do {
      len = _tenGradientParty(grad, num);
    } while (len > lastLen);
    improv = lastLen - len;
    lastLen = len;
    fprintf(stderr, "%s: improvement: %g  (mean length = %g)\n",
            me, improv, len);
  } while (improv > tgparm->minMeanImprovement
           && len > tgparm->minMean);
  
  return 0;
}

/*
******** tenGradientDistribute
**
** takes the given list of gradients, normalizes their lengths,
** optionally jitters their positions, does point repulsion, and then
** selects a combination of directions with minimum vector sum.
*/
int
tenGradientDistribute(Nrrd *nout, const Nrrd *nin,
                      tenGradientParm *tgparm) {
  char me[]="tenGradientDistribute", err[BIFF_STRLEN], *serr,
    filename[AIR_STRLEN_SMALL];
  unsigned int gi, iter;
  airArray *mop;
  Nrrd *nvel, *npos, *nveltmp, *npostmp, *ndvdt, *ndpdt;
  double *grad, len, meanVelocity;

  if (!nout || tenGradientCheck(nin, nrrdTypeUnknown, 2) || !tgparm) {
    sprintf(err, "%s: got NULL pointer or invalid input", me);
    biffAdd(TEN, err); return 1;
  }

  if (nrrdConvert(nout, nin, nrrdTypeDouble)) {
    sprintf(err, "%s: can't initialize output with input", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  grad = (double*)(nout->data);
  for (gi=0; gi<nout->axis[1].size; gi++) {
    ELL_3V_NORM(grad, grad, len);
    grad += 3;
  }
  if (tgparm->jitter) {
    if (tenGradientJitter(nout, nout, tgparm->jitter)) {
      sprintf(err, "%s: problem jittering input", me);
      biffAdd(TEN, err); return 1;
    }
  }

  mop = airMopNew();
  ndvdt = nrrdNew();
  ndpdt = nrrdNew();
  nvel = nrrdNew();
  npos = nrrdNew();
  nveltmp = nrrdNew();
  npostmp = nrrdNew();
  airMopAdd(mop, ndvdt, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ndpdt, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nvel, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, npos, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nveltmp, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, npostmp, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdCopy(ndvdt, nout)
      || nrrdCopy(ndpdt, nout)
      || nrrdCopy(nvel, nout)
      || nrrdCopy(npos, nout)
      || nrrdCopy(nveltmp, nout)
      || nrrdCopy(npostmp, nout)) {
    sprintf(err, "%s: trouble allocating temp buffers", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }

  /*
    while mean velocity is small:
    - compute forces based on positions and velocities
    - compute change in velocity (by f=ma  -->  a=f/m)
    - compute change in position (by definition)
    - apply HALF those changes to create temp world
    - compute forces based on positions and velocities
    - compute change in velocity (by f=ma)
    - compute change in position (by definition)
    - apply those changes to the real world
  */
  /* npos is already initialized (via nrrdCopy) */
  memset(nvel->data, 0, nrrdElementSize(nvel)*nrrdElementNumber(nvel));
  meanVelocity = _tenGradientMeanVelocity(nvel);
  for (iter=0;
       (iter < tgparm->minIteration 
        || (iter < tgparm->maxIteration
            && meanVelocity > tgparm->minVelocity)); 
       iter++) {
    _tenGradientChangeFind(ndvdt, ndpdt,
                           nvel, npos, tgparm);
    _tenGradientChangeApply(nveltmp, npostmp,
                            ndvdt, ndpdt, 
                            nvel, npos, tgparm, 0.5);
    _tenGradientChangeFind(ndvdt, ndpdt,
                           nveltmp, npostmp, tgparm);
    _tenGradientChangeApply(nvel, npos,
                            ndvdt, ndpdt, 
                            nvel, npos, tgparm, 1.0);
    meanVelocity = _tenGradientMeanVelocity(nvel);
    if (tgparm->snap) {
      if (!(iter % tgparm->snap)) {
        sprintf(filename, "%05d.nrrd", iter/tgparm->snap);
        fprintf(stderr, "%s: %d: meanVelocity = %g; saving %s\n",
                me, iter, meanVelocity, filename);
        if (nrrdSave(filename, npos, NULL)) {
          serr = biffGetDone(NRRD);
          fprintf(stderr, "%s: iter=%d, couldn't save snapshot:\n%s"
                  "continuing ...\n", me, iter, serr);
          free(serr);
        }
      }
    } else {
      if (!(iter % 1000)) {
        fprintf(stderr, "%s: iteration = %d: meanVelocity = %g\n",
                me, iter, meanVelocity);
      }
    }
  }

  fprintf(stderr, "%s: optimizing balance ... \n", me);
  if (tenGradientMeanMinimize(nout, npos, tgparm)) {
    sprintf(err, "%s: failed to minimize vector sum of gradients", me);
    biffAdd(TEN, err); return 1;
  }

  airMopOkay(mop); 
  return 0;
}

int
tenGradientGenerate(Nrrd *nout, unsigned int num, tenGradientParm *tgparm) {
  char me[]="tenGradientGenerate", err[BIFF_STRLEN];
  Nrrd *nin;
  airArray *mop;

  if (!(nout && tgparm)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( num >= 3 )) {
    sprintf(err, "%s: can generate minimum of 3 gradient directions "
            "(not %d)", me, num);
    biffAdd(TEN, err); return 1;
  }
  mop = airMopNew();
  nin = nrrdNew();
  airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);

  if (tenGradientRandom(nin, num, tgparm->srand)
      || tenGradientDistribute(nout, nin, tgparm)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
