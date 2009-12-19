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

pushThing *
pushThingNew(unsigned int numVert) {
  static int ttaagg=0; 
  pushThing *thg;
  unsigned int idx;

  if (!( numVert >= 1 )) {
    thg = NULL;
  } else {
    thg = (pushThing *)calloc(1, sizeof(pushThing));
    if (thg) {
      thg->ttaagg = ttaagg++;
      thg->point.thing = thg;
      thg->numVert = numVert;
      if (1 == numVert) {
        thg->vert = &(thg->point);
      } else {
        thg->vert = (pushPoint *)calloc(numVert, sizeof(pushPoint));
        for (idx=0; idx<numVert; idx++) {
          thg->vert[idx].thing = thg;
        }
      }
      thg->len = 0;
      thg->seedIdx = 0;
    }
  }
  return thg;
}

pushThing *
pushThingNix(pushThing *thg) {
  
  if (thg) {
    if (thg->vert != &(thg->point)) {
      thg->vert = (pushPoint *)airFree(thg->vert);
    }
    airFree(thg);
  }
  return NULL;
}

void
pushBinInit(pushBin *bin, unsigned int incr) {

  bin->numThing = 0;
  bin->thing = NULL;
  bin->thingArr = airArrayNew((void**)&(bin->thing), &(bin->numThing),
                              sizeof(pushThing *), incr);
  bin->numPoint = 0;
  bin->point = NULL;
  bin->pointArr = airArrayNew((void**)&(bin->point), &(bin->numPoint),
                              sizeof(pushPoint *), incr);
  /* airArray callbacks are tempting but super confusing .... */
  bin->neighbor = NULL;
  return;
}

/*
** bins own the "thing" they contain, when you nix a bin, you nix the
** the things inside, but not the points (they belong to things)
*/
void
pushBinDone(pushBin *bin) {
  unsigned int idx;

  bin->pointArr = airArrayNuke(bin->pointArr);
  for (idx=0; idx<bin->numThing; idx++) {
    bin->thing[idx] = pushThingNix(bin->thing[idx]);
  }
  bin->thingArr = airArrayNuke(bin->thingArr);
  bin->neighbor = (pushBin **)airFree(bin->neighbor);
  return;
}


pushContext *
pushContextNew(void) {
  pushContext *pctx;
  unsigned int si, pi;

  pctx = (pushContext *)calloc(1, sizeof(pushContext));
  if (pctx) {
    pctx->nin = NULL;
    pctx->npos = NULL;
    pctx->nstn = NULL;
    pctx->drag = 0.1;
    pctx->preDrag = 1.0;
    pctx->step = 0.01;
    pctx->mass = 1.0;
    pctx->scale = 0.2;
    pctx->nudge = 0.0;
    pctx->wall = 0.1;
    pctx->margin = 0.3;
    pctx->tlThresh = 0.0;
    pctx->tlSoft = 0.0;
    pctx->minMeanVel = 0.0;
    pctx->seed = 42;
    pctx->binIncr = 30;
    pctx->numThing = 0;
    pctx->numThread = 1;
    pctx->numStage = 0;
    pctx->minIter = 0;
    pctx->maxIter = 0;
    pctx->snap = 0;
    pctx->singleBin = AIR_FALSE;
    pctx->driftCorrect = AIR_TRUE;
    pctx->verbose = 0;
    pctx->force = NULL;
    pctx->ksp00 = nrrdKernelSpecNew();
    pctx->ksp11 = nrrdKernelSpecNew();
    for (si=0; si<PUSH_STAGE_MAXNUM; si++) {
      for (pi=0; pi<PUSH_STAGE_PARM_MAXNUM; pi++) {
        pctx->stageParm[si][pi] = AIR_NAN;
      }
      pctx->process[si] = _pushProcessDummy;
    }
    pctx->nten = NULL;
    pctx->ninv = NULL;
    pctx->nmask = NULL;
    pctx->gctx = NULL;
    pctx->tpvl = NULL;
    pctx->fctx = NULL;
    pctx->dimIn = 0;
    /* binsEdge and numBin are found later */
    pctx->binsEdge = pctx->numBin = 0;
    pctx->finished = AIR_FALSE;
    pctx->stageIdx = pctx->binIdx = 0;
    pctx->bin = NULL;
    pctx->maxDist = AIR_NAN;
    ELL_3V_SET(pctx->minPos, AIR_NAN, AIR_NAN, AIR_NAN);
    ELL_3V_SET(pctx->maxPos, AIR_NAN, AIR_NAN, AIR_NAN);
    pctx->meanVel = 0;
    pctx->time0 = pctx->time1 = 0;
    pctx->task = NULL;
    pctx->binMutex = NULL;
    pctx->stageBarrierA = NULL;
    pctx->stageBarrierB = NULL;
    pctx->time = 0;
    pctx->iter = 0;
    pctx->noutPos = NULL;
    pctx->noutTen = NULL;
  }
  return pctx;
}

pushContext *
pushContextNix(pushContext *pctx) {
  
  if (pctx) {
    /* weirdness: we don't manage the pushForce- caller (perhaps hest) does */
    pctx->ksp00 = nrrdKernelSpecNix(pctx->ksp00);
    pctx->ksp11 = nrrdKernelSpecNix(pctx->ksp11);
    airFree(pctx);
  }
  return NULL;
}

