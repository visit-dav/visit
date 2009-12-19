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

#include "push.h"
#include "privatePush.h"

/* 
** _pushTensorFieldSetup sets:
**** pctx->dimIn
**** pctx->nten
**** pctx->ninv
**** pctx->nmask
** and checks mask range
*/
int
_pushTensorFieldSetup(pushContext *pctx) {
  char me[]="_pushTensorFieldSetup", err[BIFF_STRLEN];
  Nrrd *seven[7], *two[2];
  NrrdRange *nrange;
  airArray *mop;
  Nrrd *ntmp;
  int E;
  float *_ten, *_inv;
  push_t ten[7], inv[7];
  size_t ii, NN;

  mop = airMopNew();
  ntmp = nrrdNew();
  airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);
  E = AIR_FALSE;
  pctx->nten = nrrdNew();
  pctx->ninv = nrrdNew();
  pctx->nmask = nrrdNew();
  if (3 == pctx->nin->dim) {
    /* input is 2D array of 2D tensors */
    pctx->dimIn = 2;
    for (ii=0; ii<7; ii++) {
      if (ii < 2) {
        two[ii] = nrrdNew();
        airMopAdd(mop, two[ii], (airMopper)nrrdNuke, airMopAlways);
      }
      seven[ii] = nrrdNew();
      airMopAdd(mop, seven[ii], (airMopper)nrrdNuke, airMopAlways);
    }
    /*    (0)         (0)
     *     1  2  3     1  2
     *        4  5        3
     *           6            */
    if (!E) E |= nrrdSlice(seven[0], pctx->nin, 0, 0);
    if (!E) E |= nrrdSlice(seven[1], pctx->nin, 0, 1);
    if (!E) E |= nrrdSlice(seven[2], pctx->nin, 0, 2);
    if (!E) E |= nrrdArithUnaryOp(seven[3], nrrdUnaryOpZero, seven[0]);
    if (!E) E |= nrrdSlice(seven[4], pctx->nin, 0, 3);
    if (!E) E |= nrrdArithUnaryOp(seven[5], nrrdUnaryOpZero, seven[0]);
    if (!E) E |= nrrdArithUnaryOp(seven[6], nrrdUnaryOpZero, seven[0]);
    if (!E) E |= nrrdJoin(two[0], (const Nrrd *const *)seven, 7, 0, AIR_TRUE);
    if (!E) E |= nrrdCopy(two[1], two[0]);
    if (!E) E |= nrrdJoin(ntmp, (const Nrrd *const *)two, 2, 3, AIR_TRUE);
    if (!E) E |= nrrdConvert(pctx->nten, ntmp, nrrdTypeFloat);
  } else {
    /* input was already 3D */
    pctx->dimIn = 3;
    E = nrrdConvert(pctx->nten, pctx->nin, nrrdTypeFloat);
  }
  
  /* set up ninv from nten */
  if (!E) E |= nrrdCopy(pctx->ninv, pctx->nten);
  if (E) {
    sprintf(err, "%s: trouble creating 3D tensor input", me);
    biffMove(PUSH, err, NRRD); airMopError(mop); return 1;
  }
  _ten = (float*)pctx->nten->data;
  _inv = (float*)pctx->ninv->data;
  NN = nrrdElementNumber(pctx->nten)/7;
  for (ii=0; ii<NN; ii++) {
    TEN_T_COPY(ten, _ten);
    _pushTenInv(pctx, inv, ten);
    TEN_T_COPY(_inv, inv);
    _ten += 7;
    _inv += 7;
  }

  if (!E) E |= nrrdSlice(pctx->nmask, pctx->nten, 0, 0);
  if (E) {
    sprintf(err, "%s: trouble creating mask", me);
    biffMove(PUSH, err, NRRD); airMopError(mop); return 1;
  }
  nrange = nrrdRangeNewSet(pctx->nmask, nrrdBlind8BitRangeFalse);
  airMopAdd(mop, nrange, (airMopper)nrrdRangeNix, airMopAlways);
  if (AIR_ABS(1.0 - nrange->max) > 0.005) {
    sprintf(err, "%s: tensor mask max %g not close 1.0", me, nrange->max);
    biffAdd(PUSH, err); airMopError(mop); return 1;
  }

  pctx->nten->axis[1].spacing = (AIR_EXISTS(pctx->nten->axis[1].spacing)
                                 ? pctx->nten->axis[1].spacing
                                 : 1.0);
  pctx->nten->axis[2].spacing = (AIR_EXISTS(pctx->nten->axis[2].spacing)
                                 ? pctx->nten->axis[2].spacing
                                 : 1.0);
  pctx->nten->axis[3].spacing = (AIR_EXISTS(pctx->nten->axis[3].spacing)
                                 ? pctx->nten->axis[3].spacing
                                 : 1.0);
  pctx->ninv->axis[1].spacing = pctx->nten->axis[1].spacing;
  pctx->ninv->axis[2].spacing = pctx->nten->axis[2].spacing;
  pctx->ninv->axis[3].spacing = pctx->nten->axis[3].spacing;
  pctx->nmask->axis[0].spacing = pctx->nten->axis[1].spacing;
  pctx->nmask->axis[1].spacing = pctx->nten->axis[2].spacing;
  pctx->nmask->axis[2].spacing = pctx->nten->axis[3].spacing;
  pctx->nten->axis[1].center = nrrdCenterCell;
  pctx->nten->axis[2].center = nrrdCenterCell;
  pctx->nten->axis[3].center = nrrdCenterCell;
  pctx->ninv->axis[1].center = nrrdCenterCell;
  pctx->ninv->axis[2].center = nrrdCenterCell;
  pctx->ninv->axis[3].center = nrrdCenterCell;
  pctx->nmask->axis[0].center = nrrdCenterCell;
  pctx->nmask->axis[1].center = nrrdCenterCell;
  pctx->nmask->axis[2].center = nrrdCenterCell;

  airMopOkay(mop); 
  return 0;
}

/*
** _pushGageSetup sets:
**** pctx->gctx
**** pctx->minPos
**** pctx->maxPos
*/
int
_pushGageSetup(pushContext *pctx) {
  char me[]="_pushGageSetup", err[BIFF_STRLEN];
  gagePerVolume *mpvl;
  int E;

  pctx->gctx = gageContextNew();
  E = AIR_FALSE;
  /* set up tensor probing */
  if (!E) E |= !(pctx->tpvl = gagePerVolumeNew(pctx->gctx,
                                               pctx->nten, tenGageKind));
  if (!E) E |= gagePerVolumeAttach(pctx->gctx, pctx->tpvl);
  if (!E) E |= gageKernelSet(pctx->gctx, gageKernel00,
                             pctx->ksp00->kernel, pctx->ksp00->parm);
  if (!E) E |= gageQueryItemOn(pctx->gctx, pctx->tpvl, tenGageTensor);
  /* set up tensor inverse probing */
  if (!E) E |= !(pctx->ipvl = gagePerVolumeNew(pctx->gctx,
                                               pctx->ninv, tenGageKind));
  if (!E) E |= gagePerVolumeAttach(pctx->gctx, pctx->ipvl);
  if (!E) E |= gageQueryItemOn(pctx->gctx, pctx->ipvl, tenGageTensor);
  /* set up mask gradient probing */
  if (!E) E |= !(mpvl = gagePerVolumeNew(pctx->gctx,
                                         pctx->nmask, gageKindScl));
  if (!E) E |= gagePerVolumeAttach(pctx->gctx, mpvl);
  if (!E) E |= gageKernelSet(pctx->gctx, gageKernel11,
                             pctx->ksp11->kernel, pctx->ksp11->parm);
  if (!E) E |= gageQueryItemOn(pctx->gctx, mpvl, gageSclGradVec);
  if (!E) E |= gageUpdate(pctx->gctx);
  if (E) {
    sprintf(err, "%s: trouble setting up gage", me);
    biffMove(PUSH, err, GAGE); return 1;
  }
  gageParmSet(pctx->gctx, gageParmRequireAllSpacings, AIR_TRUE);
  ELL_3V_SCALE(pctx->minPos, -1, pctx->gctx->shape->volHalfLen);
  ELL_3V_SCALE(pctx->maxPos, 1, pctx->gctx->shape->volHalfLen);

  return 0;
}

/*
** _pushFiberSetup sets:
**** pctx->fctx
*/
int
_pushFiberSetup(pushContext *pctx) {
  char me[]="_pushFiberSetup", err[BIFF_STRLEN];
  int E;

  pctx->fctx = tenFiberContextNew(pctx->nten);
  if (!pctx->fctx) { 
    sprintf(err, "%s: couldn't create fiber context", me);
    biffMove(PUSH, err, TEN); return 1;
  }
  E = AIR_FALSE;
  if (!E) E |= tenFiberStopSet(pctx->fctx, tenFiberStopNumSteps,
                               pctx->tlNumStep);
  if (!E) E |= tenFiberStopSet(pctx->fctx, tenFiberStopAniso,
                               tenAniso_Cl1,
                               pctx->tlThresh - pctx->tlSoft);
  if (!E) E |= tenFiberTypeSet(pctx->fctx, tenFiberTypeEvec1);
  if (!E) E |= tenFiberKernelSet(pctx->fctx,
                                 pctx->ksp00->kernel, pctx->ksp00->parm);
  if (!E) E |= tenFiberIntgSet(pctx->fctx, tenFiberIntgRK4);
  /* if (!E) E |= tenFiberIntgSet(pctx->fctx, tenFiberIntgEuler); */
  if (!E) E |= tenFiberParmSet(pctx->fctx, tenFiberParmStepSize, pctx->tlStep);
  if (!E) E |= tenFiberAnisoSpeedSet(pctx->fctx, tenAniso_Cl1,
                                     1 /* lerp */ ,
                                     pctx->tlThresh /* thresh */,
                                     pctx->tlSoft);
  if (!E) E |= tenFiberUpdate(pctx->fctx);
  if (E) {
    sprintf(err, "%s: trouble setting up fiber context", me);
    biffMove(PUSH, err, TEN); return 1;
  }
  return 0;
}

pushTask *
_pushTaskNew(pushContext *pctx, int threadIdx) {
  pushTask *task;

  task = (pushTask *)calloc(1, sizeof(pushTask));
  if (task) {
    task->pctx = pctx;
    task->gctx = gageContextCopy(pctx->gctx);
    task->fctx = tenFiberContextCopy(pctx->fctx);
    /* 
    ** HEY: its a limitation in gage that we have to know a priori
    ** the ordering of per-volumes in the context ...
    */
    task->tenAns = gageAnswerPointer(task->gctx, task->gctx->pvl[0],
                                     tenGageTensor);
    task->invAns = gageAnswerPointer(task->gctx, task->gctx->pvl[1],
                                     tenGageTensor);
    task->cntAns = gageAnswerPointer(task->gctx, task->gctx->pvl[2],
                                     gageSclGradVec);
    if (threadIdx) {
      task->thread = airThreadNew();
    }
    task->threadIdx = threadIdx;
    task->numThing = 0;
    task->sumVel = 0;
    task->vertBuff = (double*)calloc(3*(1 + 2*pctx->tlNumStep),
                                     sizeof(double));
    task->returnPtr = NULL;
  }
  return task;
}

pushTask *
_pushTaskNix(pushTask *task) {

  if (task) {
    task->gctx = gageContextNix(task->gctx);
    task->fctx = tenFiberContextNix(task->fctx);
    if (task->threadIdx) {
      task->thread = airThreadNix(task->thread);
    }
    task->vertBuff = (double *)airFree(task->vertBuff);
    airFree(task);
  }
  return NULL;
}

/*
** _pushTaskSetup sets:
**** pctx->task
**** pctx->task[]
*/
int
_pushTaskSetup(pushContext *pctx) {
  char me[]="_pushTaskSetup", err[BIFF_STRLEN];
  unsigned int tidx;

  pctx->task = (pushTask **)calloc(pctx->numThread, sizeof(pushTask *));
  if (!(pctx->task)) {
    sprintf(err, "%s: couldn't allocate array of tasks", me);
    biffAdd(PUSH, err); return 1;
  }
  for (tidx=0; tidx<pctx->numThread; tidx++) {
    pctx->task[tidx] = _pushTaskNew(pctx, tidx);
    if (!(pctx->task[tidx])) {
      sprintf(err, "%s: couldn't allocate task %d", me, tidx);
      biffAdd(PUSH, err); return 1;
    }
  }
  return 0;
}

/*
** _pushBinSetup sets:
**** pctx->maxDist, pctx->minEval, pctx->maxEval
**** pctx->binsEdge, pctx->numBin
**** pctx->bin
**** pctx->bin[]
*/
int
_pushBinSetup(pushContext *pctx) {
  char me[]="_pushBinSetup", err[BIFF_STRLEN];
  float eval[3], *tdata;
  unsigned int ii, nn, count;

  /* ------------------------ find maxEval and set up binning */
  nn = nrrdElementNumber(pctx->nten)/7;
  tdata = (float*)pctx->nten->data;
  pctx->maxEval = 0;
  pctx->meanEval = 0;
  count = 0;
  for (ii=0; ii<nn; ii++) {
    tenEigensolve_f(eval, NULL, tdata);
    if (tdata[0] > 0.5) {
      /* HEY: this limitation may be a bad idea */
      count++;
      pctx->meanEval += eval[0];
      pctx->maxEval = AIR_MAX(pctx->maxEval, eval[0]);
    }
    tdata += 7;
  }
  pctx->meanEval /= count;
  pctx->maxDist = pctx->force->maxDist(AIR_CAST(push_t, pctx->scale),
                                       AIR_CAST(push_t, pctx->maxEval),
                                       pctx->force->parm);
  if (pctx->singleBin) {
    pctx->binsEdge = 1;
    pctx->numBin = 1;
  } else {
    pctx->binsEdge = (int)floor((2.0 + 2*pctx->margin)/pctx->maxDist);
    fprintf(stderr, "!%s: maxEval=%g -> maxDist=%g -> binsEdge=%d\n",
            me, pctx->maxEval, pctx->maxDist, pctx->binsEdge);
    if (!(pctx->binsEdge >= 1)) {
      fprintf(stderr, "!%s: fixing binsEdge %d to 1\n", me, pctx->binsEdge);
      pctx->binsEdge = 1;
    }
    pctx->numBin = pctx->binsEdge*pctx->binsEdge*(2 == pctx->dimIn ? 
                                                  1 : pctx->binsEdge);
  }
  pctx->bin = (pushBin *)calloc(pctx->numBin, sizeof(pushBin));
  if (!( pctx->bin )) {
    sprintf(err, "%s: trouble allocating bin arrays", me);
    biffAdd(PUSH, err); return 1;
  }
  for (ii=0; ii<pctx->numBin; ii++) {
    pushBinInit(pctx->bin + ii, pctx->binIncr);
  }
  pushBinAllNeighborSet(pctx);

  return 0;
}

/*
** _pushThingSetup sets:
**** pctx->numThing (in case pctx->nstn and/or pctx->npos)
**
** This is only called by the master thread
** 
** this should set stuff to be like after an update stage and
** just before the rebinning
*/
int
_pushThingSetup(pushContext *pctx) {
  char me[]="_pushThingSetup", err[BIFF_STRLEN];
  double (*lup)(const void *v, size_t I);
  unsigned int *stn, pointIdx, baseIdx, thingIdx;
  pushThing *thing;

  pctx->numThing = (pctx->nstn
                    ? pctx->nstn->axis[1].size
                    : (pctx->npos
                       ? pctx->npos->axis[1].size
                       : pctx->numThing));
  lup = pctx->npos ? nrrdDLookup[pctx->npos->type] : NULL;
  stn = pctx->nstn ? (unsigned int*)pctx->nstn->data : NULL;
  for (thingIdx=0; thingIdx<pctx->numThing; thingIdx++) {
    if (pctx->nstn) {
      baseIdx = stn[0 + 3*thingIdx];
      thing = pushThingNew(stn[1 + 3*thingIdx]);
      for (pointIdx=0; pointIdx<thing->numVert; pointIdx++) {
        ELL_3V_SET_TT(thing->vert[pointIdx].pos, push_t,
                      lup(pctx->npos->data, 0 + 3*(pointIdx + baseIdx)),
                      lup(pctx->npos->data, 1 + 3*(pointIdx + baseIdx)),
                      lup(pctx->npos->data, 2 + 3*(pointIdx + baseIdx)));
        _pushProbe(pctx->task[0], thing->vert + pointIdx);
        thing->vert[pointIdx].charge = _pushThingPointCharge(pctx, thing);
      }
      thing->seedIdx = stn[2 + 3*thingIdx];
      if (1 < thing->numVert) {
        /* info about seedpoint has to be set separately */
        ELL_3V_SET_TT(thing->point.pos, push_t,
                      lup(pctx->npos->data, 0 + 3*(thing->seedIdx + baseIdx)),
                      lup(pctx->npos->data, 1 + 3*(thing->seedIdx + baseIdx)),
                      lup(pctx->npos->data, 2 + 3*(thing->seedIdx + baseIdx)));
        _pushProbe(pctx->task[0], &(thing->point));
      }
      /*
      fprintf(stderr, "!%s: numThing(%d) = %d\n", "_pushThingSetup",
              thingIdx, thing->numVert);
      */
    } else if (pctx->npos) {
      thing = pushThingNew(1);
      ELL_3V_SET_TT(thing->vert[0].pos, push_t,
                    lup(pctx->npos->data, 0 + 3*thingIdx),
                    lup(pctx->npos->data, 1 + 3*thingIdx),
                    lup(pctx->npos->data, 2 + 3*thingIdx));
      _pushProbe(pctx->task[0], thing->vert + 0);
      thing->vert[0].charge = _pushThingPointCharge(pctx, thing);
    } else {
      thing = pushThingNew(1);
      do {
        thing->vert[0].pos[0] =
          AIR_CAST(push_t, AIR_AFFINE(0.0, airDrandMT(), 1.0,
                                      pctx->minPos[0], pctx->maxPos[0]));
        thing->vert[0].pos[1] =
          AIR_CAST(push_t, AIR_AFFINE(0.0, airDrandMT(), 1.0,
                                      pctx->minPos[1], pctx->maxPos[1]));
        if (2 == pctx->dimIn
            || (3 == pctx->dimIn && 1 == pctx->nin->axis[3].size)) {
          thing->vert[0].pos[2] = 0;
        } else {
          thing->vert[0].pos[2] =
            AIR_CAST(push_t, AIR_AFFINE(0.0, airDrandMT(), 1.0,
                                        pctx->minPos[2], pctx->maxPos[2]));
        }
        _pushProbe(pctx->task[0], thing->vert + 0);
        /* assuming that we're not using some very blurring kernel,
           this will eventually succeed, because we previously checked
           the range of values in the mask */
      } while (thing->vert[0].ten[0] < 0.5);
    }
    for (pointIdx=0; pointIdx<thing->numVert; pointIdx++) {
      if (pushBinPointAdd(pctx, thing->vert + pointIdx)) {
        sprintf(err, "%s: trouble binning vert %d of thing %d", me,
                pointIdx, thingIdx);
        biffAdd(PUSH, err); return 1;
      }
      ELL_3V_SET(thing->vert[pointIdx].vel, 0, 0, 0);
      thing->vert[pointIdx].charge = _pushThingPointCharge(pctx, thing);
    }
    if (pushBinThingAdd(pctx, thing)) {
      sprintf(err, "%s: trouble thing %d", me, thingIdx);
      biffAdd(PUSH, err); return 1;
    }
  }
  /*
  {
    Nrrd *nten, *npos, *nstn;
    char me[]="dammit", err[BIFF_STRLEN], poutS[AIR_STRLEN_MED],
      toutS[AIR_STRLEN_MED], soutS[AIR_STRLEN_MED];
      nten = nrrdNew();
      npos = nrrdNew();
      nstn = nrrdNew();
      sprintf(poutS, "snap-pre.%06d.pos.nrrd", -1);
      sprintf(toutS, "snap-pre.%06d.ten.nrrd", -1);
      sprintf(soutS, "snap-pre.%06d.stn.nrrd", -1);
      if (pushOutputGet(npos, nten, nstn, pctx)) {
        sprintf(err, "%s: couldn't get snapshot for iter %d", me, -1);
        biffAdd(PUSH, err); return 1;
      }
      if (nrrdSave(poutS, npos, NULL)
          || nrrdSave(toutS, nten, NULL)
          || nrrdSave(soutS, nstn, NULL)) {
        sprintf(err, "%s: couldn't save snapshot for iter %d", me, -1);
        biffMove(PUSH, err, NRRD); return 1;
      }
      nten = nrrdNuke(nten);
      npos = nrrdNuke(npos);
      nstn = nrrdNuke(nstn);
  }
  */
  return 0;
}

