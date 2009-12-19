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

/* this is needed to make sure that tractlets that are really just
   a bunch of vertices piled up on each other are not subjected
   to any kind of frenet frame calculation */
#define MIN_FRENET_LEN 0.05

void
_pushTenInv(pushContext *pctx, push_t *inv, push_t *ten) {
  push_t tmp=0.0, det;

  if (2 == pctx->dimIn) {
    tmp = ten[6];
    ten[6] = 1.0;
  }
  TEN_T_INV(inv, ten, det);
  if (2 == pctx->dimIn) {
    ten[6] = tmp;
    inv[6] = 0.0;
  }
  return;
}

void
_pushProbe(pushTask *task, pushPoint *point) {
  push_t eval[3], sum;

  gageProbeSpace(task->gctx, point->pos[0], point->pos[1], point->pos[2],
                 AIR_FALSE, AIR_TRUE);
  TEN_T_COPY(point->ten, task->tenAns);
  /* _pushTenInv(task->pctx, point->inv, point->ten); */
  TEN_T_COPY(point->inv, task->invAns);
  tenEIGENSOLVE(eval, NULL, point->ten);
  /* sadly, the fact that tenAnisoCalc_f exists only for floats is part
     of the motivation for hard-wiring the aniso measure to Cl1 */
  sum = eval[0] + eval[1] + eval[2];
  point->aniso = (eval[0] - eval[1])/(sum + FLT_EPSILON);
  ELL_3V_COPY(point->cnt, task->cntAns);
  return;
}

int
_pushThingTotal(pushContext *pctx) {
  unsigned int binIdx, numThing;

  numThing = 0;
  for (binIdx=0; binIdx<pctx->numBin; binIdx++) {
    numThing += pctx->bin[binIdx].numThing;
  }
  return numThing;
}

int
_pushPointTotal(pushContext *pctx) {
  unsigned int binIdx, thingIdx, numPoint;
  pushBin *bin;

  numPoint = 0;
  for (binIdx=0; binIdx<pctx->numBin; binIdx++) {
    bin = pctx->bin + binIdx;
    for (thingIdx=0; thingIdx<bin->numThing; thingIdx++) {
      numPoint += bin->thing[thingIdx]->numVert;
    }
  }
  return numPoint;
}

int
pushOutputGet(Nrrd *nPosOut, Nrrd *nTenOut, Nrrd *nStnOut,
              pushContext *pctx) {
  char me[]="pushOutputGet", err[BIFF_STRLEN];
  unsigned int binIdx, pointRun, numPoint, thingRun, numThing,
    pointIdx, thingIdx, *stnOut;
  int E;
  push_t *posOut, *tenOut;
  pushBin *bin;
  pushThing *thing;
  pushPoint *point;

  numPoint = _pushPointTotal(pctx);
  numThing = _pushThingTotal(pctx);
  E = AIR_FALSE;
  if (nPosOut) {
    E |= nrrdMaybeAlloc_va(nPosOut, push_nrrdType, 2,
                           AIR_CAST(size_t, 2 == pctx->dimIn ? 2 : 3),
                           AIR_CAST(size_t, numPoint));
  }
  if (nTenOut) {
    E |= nrrdMaybeAlloc_va(nTenOut, push_nrrdType, 2, 
                           AIR_CAST(size_t, 2 == pctx->dimIn ? 4 : 7),
                           AIR_CAST(size_t, numPoint));
  }
  if (nStnOut) {
    E |= nrrdMaybeAlloc_va(nStnOut, nrrdTypeUInt, 2,
                           AIR_CAST(size_t, 3),
                           AIR_CAST(size_t, numThing));
  }
  if (E) {
    sprintf(err, "%s: trouble allocating outputs", me);
    biffMove(PUSH, err, NRRD); return 1;
  }
  posOut = nPosOut ? (push_t*)(nPosOut->data) : NULL;
  tenOut = nTenOut ? (push_t*)(nTenOut->data) : NULL;
  stnOut = nStnOut ? (unsigned int*)(nStnOut->data) : NULL;

  thingRun = 0;
  pointRun = 0;
  for (binIdx=0; binIdx<pctx->numBin; binIdx++) {
    bin = pctx->bin + binIdx;
    for (thingIdx=0; thingIdx<bin->numThing; thingIdx++) {
      thing = bin->thing[thingIdx];
      if (stnOut) {
        ELL_3V_SET(stnOut + 3*thingRun,
                   pointRun, thing->numVert, thing->seedIdx);
      }
      for (pointIdx=0; pointIdx<thing->numVert; pointIdx++) {
        point = thing->vert + pointIdx;
        if (2 == pctx->dimIn) {
          if (posOut) {
            ELL_2V_SET(posOut + 2*pointRun,
                       point->pos[0], point->pos[1]);
          }
          /*    (0)         (0)
           *     1  2  3     1  2
           *        4  5        3
           *           6            */
          if (tenOut) {
            ELL_4V_SET(tenOut + 4*pointRun,
                       point->ten[0], point->ten[1],
                       point->ten[2], point->ten[4]);
          }
        } else {
          if (posOut) {
            ELL_3V_SET(posOut + 3*pointRun,
                       point->pos[0], point->pos[1], point->pos[2]);
          }
          if (tenOut) {
            TEN_T_COPY(tenOut + 7*pointRun, point->ten);
          }
        }
        pointRun++;
      }
      thingRun++;
    }
  }

  return 0;
}

int
_pushPairwiseForce(pushContext *pctx, push_t fvec[3], pushForce *force,
                   pushPoint *myPoint, pushPoint *herPoint) {
  char me[]="_pushPairwiseForce", err[BIFF_STRLEN];
  push_t inv[7], dot;
  float haveDist, restDist, mm, fix, mag,
    D[3], nD[3], lenD, lenDsqr,
    U[3], nU[3], lenU, 
    V[3], lenV;

  /* in case lenD > maxDist */
  ELL_3V_SET(fvec, 0, 0, 0);

  ELL_3V_SUB(D, herPoint->pos, myPoint->pos);
  lenDsqr = ELL_3V_DOT(D, D);
  if (lenDsqr < FLT_EPSILON) {
    /* myPoint and herPoint are overlapping */
    fprintf(stderr, "%s: myPos == herPos == (%g,%g,%g)\n", me,
            myPoint->pos[0], myPoint->pos[1], myPoint->pos[2]);
    return 0;
  }
  if (lenDsqr >= (pctx->maxDist)*(pctx->maxDist)) {
    /* too far away to influence each other */
    return 0;
  }

  lenD = AIR_CAST(push_t, sqrt(lenDsqr));
  ELL_3V_SCALE(nD, 1.0f/lenD, D);
  /*
  TEN_T_SCALE_ADD2(ten,
                   0.5f, myPoint->ten,
                   0.5f, herPoint->ten);
  _pushTenInv(pctx, inv, ten);
  */

  TEN_T_SCALE_ADD2(inv,
                   0.5f, myPoint->inv,
                   0.5f, herPoint->inv);

  TEN_TV_MUL(U, inv, D);
  ELL_3V_NORM_TT(nU, push_t, U, lenU);
  dot = ELL_3V_DOT(nU, nD);
  haveDist = dot*lenD;
  restDist = AIR_CAST(push_t, dot*2.0*pctx->scale*lenD/lenU);
  mag = force->func(haveDist, restDist,
                    AIR_CAST(push_t, pctx->scale), force->parm);
  ELL_3V_SCALE(fvec, mag, nU);
  
  if ((0 && pctx->verbose && ELL_3V_LEN(fvec))) {
    fprintf(stderr, "%s:  me --------------------- (of thing %d)\n", me,
            myPoint->thing->ttaagg);
    fprintf(stderr, "  myTen: (%g) %g %g %g   %g %g   %g\n",
            myPoint->ten[0],
            myPoint->ten[1], myPoint->ten[2], myPoint->ten[3],
            myPoint->ten[4], myPoint->ten[5],
            myPoint->ten[6]);
    fprintf(stderr, "  myPos: %g %g %g\n",
            myPoint->pos[0], myPoint->pos[1], myPoint->pos[2]);
    fprintf(stderr, "%s: her ----- (of thing %d)\n", me,
            herPoint->thing->ttaagg);
    fprintf(stderr, " herTen: (%g) %g %g %g   %g %g   %g\n",
            herPoint->ten[0],
            herPoint->ten[1], herPoint->ten[2], herPoint->ten[3],
            herPoint->ten[4], herPoint->ten[5],
            herPoint->ten[6]);
    fprintf(stderr, " herPos: %g %g %g\n",
            herPoint->pos[0], herPoint->pos[1], herPoint->pos[2]);
    fprintf(stderr, "%s: nD = %g %g %g, \n lenD = %g (maxDist = %g)\n", me,
            nD[0], nD[1], nD[2], lenD, pctx->maxDist);
    fprintf(stderr, "   ----> fvec = %g %g %g\n", fvec[0], fvec[1], fvec[2]);
  }
  
  if (pctx->driftCorrect) {
    TEN_TV_MUL(V, myPoint->inv, D);
    lenV = AIR_CAST(push_t, ELL_3V_LEN(V));
    /* dc-0: mm = 2*dot*pctx->scale*(1.0/lenV - 1.0/lenU);
       fix = (1 - mm)/(1 + mm); */
    /* dc-1: mm = 2*dot*pctx->scale*(1.0/lenV - 1.0/lenU);
       fix = (1 + mm)/(1 - mm); */
    /* dc-2: seems to work for gaussian; still drifting w/ charge;
       but *reverse* drift for cotan!! 
       mm = 4*dot*pctx->scale*(1.0/lenV - 1.0/lenU);
       fix = (1 + mm)/(1 - mm); */
    /* 
    ** ----- this is probably correct, based on
    ** ----- tests with the one-ramp.nrrd dataset
    */
    mm = AIR_CAST(push_t, 2.0*dot*pctx->scale*(1.0/lenU - 1.0/lenV));
    mm = AIR_MAX(mm, -0.95f);
    if (mm <= -1) {
      sprintf(err, "%s: invalid mm <= -1 from:\n"
              "%g == 2*(dot=%g)*(scale=%g)*(1.0/lenU=%g - 1.0/lenV=%g = %g)",
              me, mm, dot, pctx->scale, lenU, lenV,
              1.0/lenU - 1.0/lenV);
      biffAdd(PUSH, err); return 1;
    }
    fix = AIR_CAST(push_t, sqrt((1 - mm)/(1 + mm)));
    if (pctx->verbose) {
      fprintf(stderr, "   dcfix: mm = %f -> fix = %f\n", mm, fix);
    }
    ELL_3V_SCALE(fvec, fix, fvec);
  }
  
  if (0 && pctx->verbose && ELL_3V_LEN(fvec)) {
    fprintf(stderr, "   dcfix -> fvec = %g %g %g\n",
            fvec[0], fvec[1], fvec[2]);
  }
  return 0;
}

#define THING_SIZE(pctx, thg) \
  (1 + (thg)->len/(2*(pctx)->meanEval*(pctx)->scale))

push_t
_pushThingMass(pushContext *pctx, pushThing *thg) {

  return AIR_CAST(push_t, pctx->mass*THING_SIZE(pctx, thg));
}

push_t
_pushThingPointCharge(pushContext *pctx, pushThing *thg) {

  return AIR_CAST(push_t, THING_SIZE(pctx, thg)/thg->numVert);
}

int
_pushForceSample(pushContext *pctx, unsigned int sx, unsigned int sy) {
  Nrrd *ntmp;
  double *data;
  unsigned int xi, yi, hi;
  push_t fsum[3], fvec[3];
  pushPoint _probe, *probe, *her;
  pushBin *bin, **neigh;

  probe = &_probe;
  ntmp = nrrdNew();
  nrrdMaybeAlloc_va(ntmp, nrrdTypeDouble, 3,
                    AIR_CAST(size_t, 3),
                    AIR_CAST(size_t, sx),
                    AIR_CAST(size_t, sy));
  data = (double*)ntmp->data;
  probe->pos[2] = 0.0;
  fprintf(stderr, "sampling force field"); fflush(stderr);
  for (yi=0; yi<sy; yi++) {
    fprintf(stderr, " %d/%d", yi, sy);
    probe->pos[1] = AIR_CAST(push_t,
                             AIR_AFFINE(0, yi, sy-1,
                                        pctx->minPos[1], pctx->maxPos[1]));
    for (xi=0; xi<sx; xi++) {
      probe->pos[0] = AIR_CAST(push_t,
                               AIR_AFFINE(0, xi, sx-1,
                                          pctx->minPos[0], pctx->maxPos[0]));
      _pushProbe(pctx->task[0], probe);
      bin = _pushBinLocate(pctx, probe->pos);
      neigh = bin->neighbor;
      ELL_3V_SET(fsum, 0, 0, 0);
      do {
        for (hi=0; hi<(*neigh)->numPoint; hi++) {
          her = (*neigh)->point[hi];
          _pushPairwiseForce(pctx, fvec, pctx->force, probe, her);
          ELL_3V_INCR(fsum, fvec);
        }
        neigh++;
      } while (*neigh);
      ELL_3V_COPY(data+ 3*(xi + sx*yi), fsum);
    }
  }
  fprintf(stderr, " done.\n");
  ntmp->axis[1].min = pctx->minPos[0];
  ntmp->axis[1].max = pctx->maxPos[0];
  ntmp->axis[2].min = pctx->minPos[1];
  ntmp->axis[2].max = pctx->maxPos[1];
  nrrdSave("pray.nrrd", ntmp, NULL);
  
  return 0;
}

int
_pushForce(pushTask *task, int myBinIdx,
           const push_t parm[PUSH_STAGE_PARM_MAXNUM]) {
  char me[]="_pushForce", err[BIFF_STRLEN];
  pushBin *myBin, *herBin, **neighbor;
  pushPoint *myPoint, *herPoint;
  pushThing *myThing;
  unsigned int myThingIdx, myPointIdx, herPointIdx, ci;
  push_t myCharge, herCharge,
    len, dir[3], drag, fvec[3];

  AIR_UNUSED(parm);
  /* fprintf(stderr, "!%s: bingo 0 %d\n", me, myBinIdx); */

  myBin = task->pctx->bin + myBinIdx;

  /* initialize forces for things in this bin.  This is necessary
     for tractlets in this bin, and redundant (with following
     loop) for points in this bin */
  for (myThingIdx=0; myThingIdx<myBin->numThing; myThingIdx++) {
    myThing = myBin->thing[myThingIdx];
    ELL_3V_SET(myThing->point.frc, 0, 0, 0);
  }

  /* fprintf(stderr, "!%s: bingo 1 %d\n", me, myBinIdx); */

  /* compute pair-wise forces between all points in this bin,
     and all points in all neighboring bins */
  for (myPointIdx=0; myPointIdx<myBin->numPoint; myPointIdx++) {
    myPoint = myBin->point[myPointIdx];
    myCharge = myPoint->charge;
    ELL_3V_SET(myPoint->frc, 0, 0, 0);

    neighbor = myBin->neighbor;
    while ((herBin = *neighbor)) {
      for (herPointIdx=0; herPointIdx<herBin->numPoint; herPointIdx++) {
        herPoint = herBin->point[herPointIdx];
        if (myPoint->thing == herPoint->thing) {
          /* there are no intra-thing forces */
          continue;
        }
        herCharge = herPoint->charge;
        /*
        task->pctx->verbose = (myPoint->thing->ttaagg == 398);
        */
        if (_pushPairwiseForce(task->pctx, fvec, task->pctx->force,
                               myPoint, herPoint)) {
          sprintf(err, "%s: myPoint (thing %d) vs herPoint (thing %d)", me,
                  myPoint->thing->ttaagg, herPoint->thing->ttaagg);
          biffAdd(PUSH, err); return 1;
        }

        ELL_3V_SCALE_INCR(myPoint->frc, myCharge*herCharge, fvec);
        if (!ELL_3V_EXISTS(myPoint->frc)) {
          sprintf(err, "%s: point (thing %d) frc -> NaN from point (thing %d)"
                  " w/ fvec (%g,%g,%g)",
                  me, myPoint->thing->ttaagg,
                  herPoint->thing->ttaagg, fvec[0], fvec[1], fvec[2]);
          biffAdd(PUSH, err); return 1;
        }
        if (task->pctx->verbose) {
          fprintf(stderr, "   ... myPoint->frc = %g %g %g\n", 
                  myPoint->frc[0], myPoint->frc[1], myPoint->frc[2]);
        }

        /* POST */
        if (0 && 400 == task->pctx->iter 
            && 192 == myPoint->thing->ttaagg
            && ELL_3V_LEN(fvec)) {
          fprintf(stderr, "%f,%f += %f,%f <- %f, %f <- (% 4d) %f,%f\n",
                  myPoint->frc[0], myPoint->frc[1],
                  fvec[0], fvec[1],
                  myCharge, herCharge, 
                  herPoint->thing->ttaagg,
                  herPoint->pos[0], herPoint->pos[1]);
          if (!AIR_EXISTS(myPoint->frc[0])) {
            exit(1);
          }
        }

      }
      neighbor++;
    }
  
    /* fprintf(stderr, "!%s: bingo 1.3 %d\n", me, myBinIdx); */
    /* each point in this thing also potentially experiences wall forces */
    if (task->pctx->wall) {
      for (ci=0; ci<=2; ci++) {
        len = AIR_CAST(push_t, myPoint->pos[ci] - task->pctx->minPos[ci]);
        if (len < 0) {
          myPoint->frc[ci] += AIR_CAST(push_t, -task->pctx->wall*len);
        } else {
          len = AIR_CAST(push_t, task->pctx->maxPos[ci] - myPoint->pos[ci]);
          if (len < 0) {
            myPoint->frc[ci] += AIR_CAST(push_t, task->pctx->wall*len);
          }
        }
      }
    }
  } /* for myPointIdx ... */

  /* fprintf(stderr, "!%s: bingo 2 %d\n", me, myBinIdx); */

  /* drag and nudging are computed per-thing, not per-point */
  for (myThingIdx=0; myThingIdx<myBin->numThing; myThingIdx++) {
    myThing = myBin->thing[myThingIdx];
    if (task->pctx->minIter
        && task->pctx->iter < task->pctx->minIter) {
      drag = AIR_CAST(push_t,
                      AIR_AFFINE(0, task->pctx->iter, task->pctx->minIter,
                                 task->pctx->preDrag, task->pctx->drag));
    } else {
      drag = AIR_CAST(push_t, task->pctx->drag);
    }
    ELL_3V_SCALE_INCR(myThing->point.frc, -drag, myThing->point.vel);
    if (task->pctx->nudge) {
      ELL_3V_NORM_TT(dir, push_t, myThing->point.pos, len);
      if (len) {
        ELL_3V_SCALE_INCR_TT(myThing->point.frc, push_t,
                             -task->pctx->nudge*len, dir);
      }
    }
  }

  /* fprintf(stderr, "!%s: bingo 100\n", me); */
  return 0;
}

int
_pushThingPointBe(pushTask *task, pushThing *thing, pushBin *oldBin) {
  char me[]="_pushThingPointBe", err[BIFF_STRLEN];
  unsigned int vertIdx;

  if (1 == thing->numVert) {
    /* its already a point, so no points have to be nullified
       in the bins.  The point may be rebinned later, but so what. */
  } else {
    /* have to nullify, but not remove (because it wouldn't be
       thread-safe) any outstanding pointers to this point in bins */
    for (vertIdx=0; vertIdx<thing->numVert; vertIdx++) {
      if (_pushBinPointNullify(task->pctx, NULL, thing->vert + vertIdx)) {
        sprintf(err, "%s(%d): couldn't nullify vertex %d of thing %p",
                me, task->threadIdx,
                vertIdx, AIR_CAST(void*, thing));
        biffAdd(PUSH, err); return 1;
      }
    }
    /* the now-single point does have to be binned */
    _pushBinPointAdd(task->pctx, oldBin, &(thing->point));
    thing->point.charge = _pushThingPointCharge(task->pctx, thing);
    /* free vertex info */
    airFree(thing->vert);
    thing->vert = &(thing->point);
    thing->numVert = 1;
    thing->len = 0;
    thing->seedIdx = 0;
  }
  return 0;
}

int
_pushThingTractletBe(pushTask *task, pushThing *thing, pushBin *oldBin) {
  char me[]="_pushThingTractletBe", err[BIFF_STRLEN];
  unsigned int vertIdx, startIdx, endIdx, numVert;
  int tret;
  double seed[3], tmp;

  /* NOTE: the seed point velocity remains as the tractlet velocity */

  ELL_3V_COPY(seed, thing->point.pos);
  tret = tenFiberTraceSet(task->fctx, NULL, 
                          task->vertBuff, task->pctx->tlNumStep,
                          &startIdx, &endIdx, seed);
  if (tret) {
    sprintf(err, "%s(%d): fiber tracing failed", me, task->threadIdx);
    biffMove(PUSH, err, TEN); return 1;
  }
  if (task->fctx->whyNowhere) {
    sprintf(err, "%s(%d): fiber tracing got nowhere: %d == %s\n", me,
            task->threadIdx,
            task->fctx->whyNowhere,
            airEnumDesc(tenFiberStop, task->fctx->whyNowhere));
    biffAdd(PUSH, err); return 1;
  }
  numVert = endIdx - startIdx + 1;
  if (!( numVert >= 3 )) {
    sprintf(err, "%s(%d): numVert only %d < 3", me, task->threadIdx, numVert);
    biffAdd(PUSH, err); return 1;
  }

  /* remember the length */
  thing->len = 
    AIR_CAST(push_t, task->fctx->halfLen[0] + task->fctx->halfLen[1]);

  /* allocate tractlet vertices as needed */
  if (numVert != thing->numVert) {
    if (1 == thing->numVert) {
      /* it used to be a point, nullify old bin's pointer to it */
      if (_pushBinPointNullify(task->pctx, oldBin, &(thing->point))) {
        sprintf(err, "%s(%d): couldn't nullify former point %p of thing %p",
                me, task->threadIdx,
                AIR_CAST(void*, &(thing->point)), AIR_CAST(void*, thing));
        biffAdd(PUSH, err); return 1;
      }
    } else {
      /* it used to be a tractlet (but w/ different numVert); verts still
         have the position information so that we can recover the old bins */
      for (vertIdx=0; vertIdx<thing->numVert; vertIdx++) {
        if (_pushBinPointNullify(task->pctx, NULL, thing->vert + vertIdx)) {
          sprintf(err, "%s(%d): couldn't nullify old vert %d %p of thing %p",
                  me, task->threadIdx, vertIdx,
                  AIR_CAST(void*, thing->vert + vertIdx),
                  AIR_CAST(void*, thing));
          biffAdd(PUSH, err); return 1;
        }
      }
      airFree(thing->vert);
    }
    thing->vert = (pushPoint*)calloc(numVert, sizeof(pushPoint));
    thing->numVert = numVert;
    /* put tractlet points into last bin we were, since we can do so in a 
       thread-safe way; later they will be re-binned */
    for (vertIdx=0; vertIdx<thing->numVert; vertIdx++) {
      _pushBinPointAdd(task->pctx, oldBin, thing->vert + vertIdx);
    }
  }

  /* copy from fiber tract vertex buffer */
  for (vertIdx=0; vertIdx<numVert; vertIdx++) {
    thing->vert[vertIdx].thing = thing;
    ELL_3V_COPY_TT(thing->vert[vertIdx].pos, push_t,
                   task->vertBuff + 3*(startIdx + vertIdx));
    _pushProbe(task, thing->vert + vertIdx);
    thing->vert[vertIdx].charge = _pushThingPointCharge(task->pctx, thing);
  }
  thing->seedIdx = task->pctx->tlNumStep - startIdx;

  /* compute tangent at all vertices */
  if (task->pctx->tlFrenet && thing->len > MIN_FRENET_LEN) {
    ELL_3V_SUB(thing->vert[0].tan, thing->vert[1].pos, thing->vert[0].pos);
    ELL_3V_NORM_TT(thing->vert[0].tan, push_t,
                   thing->vert[0].tan, tmp);
    for (vertIdx=1; vertIdx<numVert-1; vertIdx++) {
      ELL_3V_SUB(thing->vert[vertIdx].tan,
                 thing->vert[vertIdx+1].pos,
                 thing->vert[vertIdx-1].pos);
      ELL_3V_NORM_TT(thing->vert[vertIdx].tan, push_t,
                     thing->vert[vertIdx].tan, tmp);
    }
    ELL_3V_SUB(thing->vert[numVert-1].tan,
             thing->vert[numVert-1].pos,
               thing->vert[numVert-2].pos);
    ELL_3V_NORM_TT(thing->vert[numVert-1].tan, push_t,
                   thing->vert[numVert-1].tan, tmp);
    
    /* compute "normal" at all vertices */
    for (vertIdx=1; vertIdx<numVert-1; vertIdx++) {
      ELL_3V_CROSS(thing->vert[vertIdx].nor,
                   thing->vert[vertIdx+1].tan,
                   thing->vert[vertIdx-1].tan);
      ELL_3V_NORM_TT(thing->vert[vertIdx].nor, push_t,
                     thing->vert[vertIdx].nor, tmp);
      tmp = ELL_3V_LEN(thing->vert[vertIdx].nor);
      if (!AIR_EXISTS(tmp)) {
        fprintf(stderr, "(%d) (%g,%g,%g) X (%g,%g,%g) = "
                "%g %g %g --> %g\n", vertIdx,
                (thing->vert[vertIdx+1].tan)[0],
                (thing->vert[vertIdx+1].tan)[1],
                (thing->vert[vertIdx+1].tan)[2],
                (thing->vert[vertIdx-1].tan)[0],
                (thing->vert[vertIdx-1].tan)[1],
                (thing->vert[vertIdx-1].tan)[2],
                thing->vert[vertIdx].nor[0],
                thing->vert[vertIdx].nor[1],
                thing->vert[vertIdx].nor[2],
                ELL_3V_LEN(thing->vert[vertIdx].nor));
        exit(1);
      }
    }
    ELL_3V_COPY(thing->vert[0].nor, thing->vert[1].nor);
    ELL_3V_COPY(thing->vert[numVert-1].nor, thing->vert[numVert-2].nor);
  }

  return 0;
}

void
_pushPrintForce(pushContext *pctx, pushThing *thing) {
  int posI[3], frcI[3];
  double pos[3], frc[3];
  unsigned int vi;

#define TMP_3V_SCALE(v2, a, v1) \
  ((v2)[0] = (int)((a)*(v1)[0]),       \
   (v2)[1] = (int)((a)*(v1)[1]),       \
   (v2)[2] = (int)((a)*(v1)[2]))

  AIR_UNUSED(pctx);
  TMP_3V_SCALE(posI, 1000000, thing->point.pos);
  ELL_3V_SCALE(pos, 1.0/1000000, posI);
  TMP_3V_SCALE(frcI, 1000000, thing->point.frc);
  ELL_3V_SCALE(frc, 1.0/1000000, frcI);
  fprintf(stderr, "% 4d@(% 6.6f,% 6.6f)(% 6.6f,% 6.6f)",
          thing->ttaagg, pos[0], pos[1], frc[0], frc[1]);
  for (vi=0; vi<thing->numVert; vi++) {
    TMP_3V_SCALE(frcI, 1000000, thing->vert[vi].frc);
    ELL_3V_SCALE(frc, 1.0/1000000, frcI);
    fprintf(stderr, "--(% 6.6f,% 6.6f)", frc[0], frc[1]);
  }
  fprintf(stderr, "\n");

#undef TMP_3V_SCALE
}

int
_pushUpdate(pushTask *task, int binIdx,
            const push_t parm[PUSH_STAGE_PARM_MAXNUM]) {
  char me[]="_pushUpdate", err[BIFF_STRLEN];
  int ret, inside;
  unsigned int thingIdx, vertIdx;
  double step, mass, *minPos, *maxPos;
  push_t fTNB[3], binorm[3], fvec[3];
  pushBin *bin;
  pushThing *thing;
  pushPoint *point, *seedPoint;

  AIR_UNUSED(parm);
  step = task->pctx->step;
  bin = task->pctx->bin + binIdx;
  minPos = task->pctx->minPos;
  maxPos = task->pctx->maxPos;
  for (thingIdx=0; thingIdx<bin->numThing; thingIdx++) {
    thing = bin->thing[thingIdx];
    /*
    task->pctx->verbose = (thing->ttaagg == 29);
    */
    /* convert per-vertex forces on tractlet to total force */
    if (thing->numVert > 1) {
      ELL_3V_SET(fvec, 0, 0, 0);
      if (task->pctx->tlFrenet && thing->len > MIN_FRENET_LEN) {
        ELL_3V_SET(fTNB, 0, 0, 0);
        for (vertIdx=0; vertIdx<thing->numVert; vertIdx++) {
          point = thing->vert + vertIdx;
          ELL_3V_CROSS(binorm, point->tan, point->nor);
          fTNB[0] += ELL_3V_DOT(point->frc, point->tan);
          fTNB[1] += ELL_3V_DOT(point->frc, point->nor);
          fTNB[2] += ELL_3V_DOT(point->frc, binorm);
        }
        seedPoint = thing->vert + thing->seedIdx;
        ELL_3V_CROSS(binorm, seedPoint->tan, seedPoint->nor);
        ELL_3V_SCALE_INCR(fvec, fTNB[0], seedPoint->tan);
        ELL_3V_SCALE_INCR(fvec, fTNB[1], seedPoint->nor);
        ELL_3V_SCALE_INCR(fvec, fTNB[2], binorm);
      } else {
        for (vertIdx=0; vertIdx<thing->numVert; vertIdx++) {
          point = thing->vert + vertIdx;
          ELL_3V_INCR(fvec, point->frc);
        }
      }
      /* we have to add this on ("INCR") and not just set it,
         because the drag and nudge forces were already stored
         here during the force stage */
      /* ELL_3V_SCALE_INCR(thing->point.frc, 1.0/thing->numVert, fvec); */
      ELL_3V_INCR(thing->point.frc, fvec);
    }

    /* POST */
    if (task->pctx->verbose) {
      fprintf(stderr, "final: %f,%f\n", 
              thing->point.frc[0], thing->point.frc[1]);
    }

    /* update dynamics: applies equally to points and tractlets */
    mass = _pushThingMass(task->pctx, thing);
    if (task->pctx->verbose) {
      fprintf(stderr, "vel(%f,%f) * step(%f) -+-> pos(%f,%f)\n", 
              thing->point.vel[0], thing->point.vel[0], step,
              thing->point.pos[0], thing->point.pos[1]);
      fprintf(stderr, "frc(%f,%f) * step(%f)/mass(%f) (%f) -+-> vel(%f,%f)\n", 
              thing->point.frc[0], thing->point.frc[0], step, mass, step/mass,
              thing->point.vel[0], thing->point.vel[1]);
    }
    ELL_3V_SCALE_INCR_TT(thing->point.pos, push_t,
                         step, thing->point.vel);
    if (2 == task->pctx->dimIn
        || (3 == task->pctx->dimIn && 1 == task->pctx->nin->axis[3].size)) {
      thing->point.pos[2] = 0;
    }
    ELL_3V_SCALE_INCR_TT(thing->point.vel, push_t,
                         step/mass, thing->point.frc);
    if (task->pctx->verbose) {
      fprintf(stderr, "thing %d: pos(%f,%f); vel(%f,%f)\n",
              thing->ttaagg,
              thing->point.pos[0], thing->point.pos[1],
              thing->point.vel[0], thing->point.vel[0]);
      fprintf(stderr, "sumVel = %f ---> ", task->sumVel);
    }
    task->sumVel += ELL_3V_LEN(thing->point.vel);
    if (task->pctx->verbose) {
      fprintf(stderr, "%f (exists %d)\n", task->sumVel,
              AIR_EXISTS(task->sumVel));
    }
    if (!AIR_EXISTS(task->sumVel)) {
      sprintf(err, "%s(%d): sumVel went NaN (from vel (%g,%g,%g), from force "
              "(%g,%g,%g)) on thing %d (%d verts) %p of bin %d", 
              me, task->threadIdx,
              thing->point.vel[0],
              thing->point.vel[1],
              thing->point.vel[2],
              thing->point.frc[0],
              thing->point.frc[1],
              thing->point.frc[2],
              thing->ttaagg, thing->numVert, AIR_CAST(void*, thing), binIdx);
      biffAdd(PUSH, err); return 1;
    }
    task->numThing += 1;
    /* while _pushProbe clamps positions to inside domain before
       calling gageProbe, we can exert no such control over the gageProbe
       called within tenFiberTraceSet.  So for now, things turn to points
       as soon as they leave the domain, even if they are still inside
       the margin.  This sucks */
    inside = (AIR_IN_OP(minPos[0], thing->point.pos[0], maxPos[0]) &&
              AIR_IN_OP(minPos[1], thing->point.pos[1], maxPos[1]) &&
              AIR_IN_OP(minPos[2], thing->point.pos[2], maxPos[2]));
    /* sample field at new point location */
    _pushProbe(task, &(thing->point));
    /* be a point or tractlet, depending on anisotropy (and location) */
    if (inside && (thing->point.aniso 
                   >= (task->pctx->tlThresh - task->pctx->tlSoft))) {
      ret = _pushThingTractletBe(task, thing, bin);
    } else {
      ret = _pushThingPointBe(task, thing, bin);
    }
    if (ret) {
      sprintf(err, "%s(%d): trouble updating thing %d %p of bin %d",
              me, task->threadIdx,
              thing->ttaagg, AIR_CAST(void*, thing), binIdx);
      biffAdd(PUSH, err); return 1;
    }
  } /* for thingIdx */
  return 0;
}

