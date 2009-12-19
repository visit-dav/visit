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

#include "coil.h"

int
coilVolumeCheck(const Nrrd *nin, const coilKind *kind) {
  char me[]="coilVolumeCheck", err[BIFF_STRLEN];
  unsigned int baseDim;
  
  if (!(nin && kind)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(COIL, err); return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    sprintf(err, "%s: can only operate on scalar types, not %s", me,
            airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(COIL, err); return 1;
  }
  baseDim = (1 == kind->valLen ? 0 : 1);
  if (3 + baseDim != nin->dim) {
    sprintf(err, "%s: dim of input must be 3+%d (3 + baseDim), not %d", 
            me, baseDim, nin->dim);
    biffAdd(COIL, err); return 1;
  }

  return 0;
}

coilContext *
coilContextNew() {
  coilContext *cctx;
  
  cctx = (coilContext *)calloc(1, sizeof(coilContext));
  if (cctx) {
    cctx->nin = NULL;
    cctx->radius = coilDefaultRadius;
    cctx->numThreads = 1;
    ELL_3V_SET(cctx->spacing, AIR_NAN, AIR_NAN, AIR_NAN);
    cctx->nvol = NULL;
    cctx->finished = AIR_FALSE;
    cctx->task = NULL;
    cctx->nextSliceMutex = NULL;
    cctx->filterBarrier = NULL;
    cctx->updateBarrier = NULL;
  }
  return cctx;
}

int
coilContextAllSet(coilContext *cctx, const Nrrd *nin,
                  const coilKind *kind, const coilMethod *method,
                  unsigned int radius, unsigned int numThreads, int verbose,
                  double parm[COIL_PARMS_NUM]) {
  char me[]="coilContextAllSet", err[BIFF_STRLEN];
  int someExist, allExist, baseDim, pi;
  size_t size[NRRD_DIM_MAX], sx, sy, sz;
  double xsp, ysp, zsp;
  airArray *mop;
  
  cctx->verbose = verbose;
  if (!( cctx && nin && kind && method )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(COIL, err); return 1;
  }
  if (coilVolumeCheck(nin, kind)) {
    sprintf(err, "%s: input volume not usable as %s", me, kind->name);
    biffAdd(COIL, err); return 1;
  }
  if (!( radius >= 1 && numThreads >= 1 )) {
    sprintf(err, "%s: radius (%d) not >= 1 or numThreads (%d) not >= 1", me,
            radius, numThreads);
    biffAdd(COIL, err); return 1;
  }
  if (!( AIR_IN_OP(coilMethodTypeUnknown, method->type,
                   coilMethodTypeLast) )) {
    sprintf(err, "%s: method->type %d not valid", me, method->type);
    biffAdd(COIL, err); return 1;
  }
  
  if (!kind->filter[method->type]) {
    sprintf(err, "%s: sorry, %s filtering not available on %s kind",
            me, method->name, kind->name);
    biffAdd(COIL, err); return 1;
  }

  /* warn if we can't do the multiple threads user wants */
  if (numThreads > 1 && !airThreadCapable && airThreadNoopWarning) {
    fprintf(stderr, "%s: WARNING: this Teem not thread capable: using 1 "
            "thread, not %d\n", me, numThreads);
    numThreads = 1;
  }
  
  mop = airMopNew();

  /* set parms */
  for (pi=0; pi<method->numParm; pi++) {
    if (!AIR_EXISTS(parm[pi])) {
      sprintf(err, "%s: parm[%d] (need %d) doesn't exist",
              me, pi, method->numParm);
      biffAdd(COIL, err); airMopError(mop); return 1;
    }
    cctx->parm[pi] = parm[pi];
  }

  /* set sizes and spacings */
  baseDim = (1 == kind->valLen ? 0 : 1);
  sx = nin->axis[0 + baseDim].size;
  sy = nin->axis[1 + baseDim].size;
  sz = nin->axis[2 + baseDim].size;
  if (sz < numThreads) {
    fprintf(stderr, "%s: wanted %d threads but volume only has "
            _AIR_SIZE_T_CNV " slices, using " _AIR_SIZE_T_CNV 
            " threads instead\n", me, numThreads, sz, sz);
    numThreads = sz;
  }
  ELL_3V_SET(cctx->size, sx, sy, sz);
  xsp = nin->axis[0 + baseDim].spacing;
  ysp = nin->axis[1 + baseDim].spacing;
  zsp = nin->axis[2 + baseDim].spacing;
  someExist = AIR_EXISTS(xsp) || AIR_EXISTS(ysp) || AIR_EXISTS(zsp);
  allExist = AIR_EXISTS(xsp) && AIR_EXISTS(ysp) && AIR_EXISTS(zsp);
  if (!( someExist )) {
    fprintf(stderr, "%s: WARNING: assuming unit spacing for all axes\n", me);
    xsp = 1;
    ysp = 1;
    zsp = 1;
  } else {
    if ( !allExist ) {
      sprintf(err, "%s: spacings (%g,%g,%g) not uniformly existant", 
              me, xsp, ysp, zsp);
      biffAdd(COIL, err); airMopError(mop); return 1;
    }
  }
  ELL_3V_SET(cctx->spacing, xsp, ysp, zsp);
  if (cctx->verbose) {
    fprintf(stderr, "%s: spacings: %g %g %g\n", me, 
            cctx->spacing[0], cctx->spacing[1], cctx->spacing[2]);
  }
  
  /* allocate nvol */
  if (0 == baseDim) {
    ELL_4V_SET(size, 2, sx, sy, sz);
  } else {
    ELL_5V_SET(size, kind->valLen, 2, sx, sy, sz);
  }
  cctx->nvol = nrrdNew();
  if (nrrdMaybeAlloc_nva(cctx->nvol, coil_nrrdType, 4 + baseDim, size)) {
    sprintf(err, "%s: couldn't allocate internal processing volume", me);
    biffMove(COIL, err, NRRD); airMopError(mop); return 1;
  }
  airMopAdd(mop, cctx->nvol, (airMopper)nrrdNuke, airMopOnError);
  
  cctx->nin = nin;
  cctx->kind = kind;
  cctx->method = method;
  cctx->radius = radius;
  cctx->numThreads = numThreads;

  airMopOkay(mop);
  return 0;
}

/*
******** coilOutputGet
**
** slice the present intermediate volume to get an output.  
**
** No, this does not do quantization or rounding to match the input 
** type (of cctx->nin).  The reason is that after filtering, it is often
** the case that subtle differences in values emerge, and it may be
** reckless to dump them back into the limited type or value range
** that they started with.  That sort of operation should be under
** explicit user control.
*/
int
coilOutputGet(Nrrd *nout, coilContext *cctx) {
  char me[]="coilOutputGet", err[BIFF_STRLEN];
  int baseDim;

  if (!(nout && cctx)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(COIL, err); return 1;
  }
  baseDim = (1 == cctx->kind->valLen ? 0 : 1);
  if (nrrdSlice(nout, cctx->nvol, baseDim, 0)
      || nrrdAxisInfoCopy(nout, cctx->nin, NULL, NRRD_AXIS_INFO_NONE)) {
    sprintf(err, "%s: trouble getting output", me);
    biffMove(COIL, err, NRRD); return 1;
  }
  return 0;
}

coilContext *
coilContextNix(coilContext *cctx) {

  if (cctx) {
    /* thread machinery destroyed with coilFinish() */
    cctx->nvol = nrrdNuke(cctx->nvol);
    airFree(cctx);
  }
  return NULL;
}
