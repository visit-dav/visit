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

#include "mite.h"
#include "privateMite.h"

miteRender *
_miteRenderNew(void) {
  miteRender *mrr;

  mrr = (miteRender *)calloc(1, sizeof(miteRender));
  if (mrr) {
    mrr->rmop = airMopNew();
    if (!mrr->rmop) {
      airFree(mrr);
      return NULL;
    }
    mrr->ntxf = NULL;
    mrr->ntxfNum = 0;
    mrr->sclPvlIdx = -1;
    mrr->vecPvlIdx = -1;
    mrr->tenPvlIdx = -1;
    mrr->normalSpec = gageItemSpecNew();
    airMopAdd(mrr->rmop, mrr->normalSpec,
              (airMopper)gageItemSpecNix, airMopAlways);
    mrr->shadeSpec = miteShadeSpecNew();
    airMopAdd(mrr->rmop, mrr->shadeSpec,
              (airMopper)miteShadeSpecNix, airMopAlways);
    mrr->time0 = AIR_NAN;
    GAGE_QUERY_RESET(mrr->queryMite);
    mrr->queryMiteNonzero = AIR_FALSE;
  }
  return mrr;
}

miteRender *
_miteRenderNix(miteRender *mrr) {
  
  if (mrr) {
    airMopOkay(mrr->rmop);
    airFree(mrr);
  }
  return NULL;
}

int 
miteRenderBegin(miteRender **mrrP, miteUser *muu) {
  char me[]="miteRenderBegin", err[BIFF_STRLEN];
  gagePerVolume *pvl;
  int E, T, thr, pvlIdx;
  gageQuery queryScl, queryVec, queryTen;
  gageItemSpec isp;
  unsigned int axi;
 
  if (!(mrrP && muu)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(MITE, err); return 1;
  }
  if (_miteUserCheck(muu)) {
    sprintf(err, "%s: problem with user-set parameters", me);
    biffAdd(MITE, err); return 1;
  }
  if (!( *mrrP = _miteRenderNew() )) {
    sprintf(err, "%s: couldn't alloc miteRender", me);
    biffAdd(MITE, err); return 1;
  }
  if (_miteNtxfAlphaAdjust(*mrrP, muu)) {
    sprintf(err, "%s: trouble copying and alpha-adjusting txfs", me);
    biffAdd(MITE, err); return 1;
  }

  GAGE_QUERY_RESET(queryScl);
  GAGE_QUERY_RESET(queryVec);
  GAGE_QUERY_RESET(queryTen);
  GAGE_QUERY_RESET((*mrrP)->queryMite);
  for (T=0; T<muu->ntxfNum; T++) {
    for (axi=1; axi<muu->ntxf[T]->dim; axi++) {
      miteVariableParse(&isp, muu->ntxf[T]->axis[axi].label);
      miteQueryAdd(queryScl, queryVec, queryTen, (*mrrP)->queryMite, &isp);
    }
  }
  miteVariableParse((*mrrP)->normalSpec, muu->normalStr);
  miteQueryAdd(queryScl, queryVec, queryTen, (*mrrP)->queryMite,
               (*mrrP)->normalSpec);
  miteShadeSpecParse((*mrrP)->shadeSpec, muu->shadeStr);
  miteShadeSpecQueryAdd(queryScl, queryVec, queryTen, (*mrrP)->queryMite,
                        (*mrrP)->shadeSpec);
  (*mrrP)->queryMiteNonzero = GAGE_QUERY_NONZERO((*mrrP)->queryMite);

  E = 0;
  pvlIdx = 0;
  if (muu->nsin) {
    if (!E) E |= !(pvl = gagePerVolumeNew(muu->gctx0, muu->nsin, gageKindScl));
    if (!E) E |= gageQuerySet(muu->gctx0, pvl, queryScl);
    if (!E) E |= gagePerVolumeAttach(muu->gctx0, pvl);
    if (!E) (*mrrP)->sclPvlIdx = pvlIdx++;
  }
  if (muu->nvin) {
    if (!E) E |= !(pvl = gagePerVolumeNew(muu->gctx0, muu->nvin, gageKindVec));
    if (!E) E |= gageQuerySet(muu->gctx0, pvl, queryVec);
    if (!E) E |= gagePerVolumeAttach(muu->gctx0, pvl);
    if (!E) (*mrrP)->vecPvlIdx = pvlIdx++;
  }
  if (muu->ntin) {
    if (!E) E |= !(pvl = gagePerVolumeNew(muu->gctx0, muu->ntin, tenGageKind));
    if (!E) E |= gageQuerySet(muu->gctx0, pvl, queryTen);
    if (!E) E |= gagePerVolumeAttach(muu->gctx0, pvl);
    if (!E) (*mrrP)->tenPvlIdx = pvlIdx++;
  }
  if (!E) E |= gageKernelSet(muu->gctx0, gageKernel00,
                             muu->ksp[gageKernel00]->kernel,
                             muu->ksp[gageKernel00]->parm);
  if (!E) E |= gageKernelSet(muu->gctx0, gageKernel11,
                             muu->ksp[gageKernel11]->kernel,
                             muu->ksp[gageKernel11]->parm);
  if (!E) E |= gageKernelSet(muu->gctx0, gageKernel22,
                             muu->ksp[gageKernel22]->kernel,
                             muu->ksp[gageKernel22]->parm);
  if (!E) E |= gageUpdate(muu->gctx0);
  if (E) {
    sprintf(err, "%s: gage trouble", me);
    biffMove(MITE, err, GAGE); return 1;
  }
  fprintf(stderr, "!%s: kernel support = %d^3 samples\n",
          me, 2*muu->gctx0->radius);
  
  if (nrrdMaybeAlloc_va(muu->nout, mite_nt, 3,
                        AIR_CAST(size_t, 5) /* RGBAZ */ ,
                        AIR_CAST(size_t, muu->hctx->imgSize[0]),
                        AIR_CAST(size_t, muu->hctx->imgSize[1]))) {
    sprintf(err, "%s: nrrd trouble", me);
    biffMove(MITE, err, NRRD);
    return 1;
  }
  muu->nout->axis[1].center = nrrdCenterCell;
  muu->nout->axis[1].min = muu->hctx->cam->uRange[0];
  muu->nout->axis[1].max = muu->hctx->cam->uRange[1];
  muu->nout->axis[2].center = nrrdCenterCell;
  muu->nout->axis[2].min = muu->hctx->cam->vRange[0];
  muu->nout->axis[2].max = muu->hctx->cam->vRange[1];

  for (thr=0; thr<muu->hctx->numThreads; thr++) {
    (*mrrP)->tt[thr] = miteThreadNew();
    if (!((*mrrP)->tt[thr])) {
      sprintf(err, "%s: couldn't allocate thread[%d]", me, thr);
      biffAdd(MITE, err); return 1;
    }
    airMopAdd((*mrrP)->rmop, (*mrrP)->tt[thr],
              (airMopper)miteThreadNix, airMopAlways);
  }

  (*mrrP)->time0 = airTime();
  return 0;
}

int
miteRenderEnd(miteRender *mrr, miteUser *muu) {
  int thr;
  double samples;

  muu->rendTime = airTime() - mrr->time0;
  samples = 0;
  for (thr=0; thr<muu->hctx->numThreads; thr++) {
    samples += mrr->tt[thr]->samples;
  }
  muu->sampRate = samples/(1000.0*muu->rendTime);
  _miteRenderNix(mrr);
  return 0;
}
