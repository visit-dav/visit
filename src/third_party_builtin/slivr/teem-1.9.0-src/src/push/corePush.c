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

int
_pushProcessDummy(pushTask *task, int bin, 
                  const push_t parm[PUSH_STAGE_PARM_MAXNUM]) {
  char me[]="_pushProcessDummy";
  unsigned int i, j;

  AIR_UNUSED(parm);
  fprintf(stderr, "%s(%d): dummy processing bin %d (stage %d)\n", me,
          task->threadIdx, bin, task->pctx->stageIdx);
  j = 0;
  for (i=0; i<=1000000*(1+task->threadIdx); i++) {
    j += i;
  }
  for (i=0; i<=1000000*(1+task->threadIdx); i++) {
    j -= i;
  }
  for (i=0; i<=1000000*(1+task->threadIdx); i++) {
    j += i;
  }
  for (i=0; i<=1000000*(1+task->threadIdx); i++) {
    j -= i;
  }
  return 0;
}

/*
** this is run once per task (thread), per stage
*/
int
_pushStageRun(pushTask *task, int stageIdx) {
  char me[]="_pushStageRun", err[BIFF_STRLEN];
  unsigned int binIdx;
  
  while (task->pctx->binIdx < task->pctx->numBin) {
    if (task->pctx->numThread > 1) {
      airThreadMutexLock(task->pctx->binMutex);
    }
    do {
      binIdx = task->pctx->binIdx;
      if (task->pctx->binIdx < task->pctx->numBin) {
        task->pctx->binIdx++;
      }
    } while (binIdx < task->pctx->numBin
             && 0 == task->pctx->bin[binIdx].numThing
             && 0 == task->pctx->bin[binIdx].numPoint);
    if (task->pctx->numThread > 1) {
      airThreadMutexUnlock(task->pctx->binMutex);
    }

    if (binIdx == task->pctx->numBin) {
      /* no more bins to process */
      break;
    }

    if (task->pctx->process[stageIdx](task, binIdx,
                                      task->pctx->stageParm[stageIdx])) {
      sprintf(err, "%s(%d): had trouble running stage %d", me,
              task->threadIdx, stageIdx);
      biffAdd(PUSH, err); return 1;
    }
  }
  return 0;
}

/* the main loop for each worker thread */
void *
_pushWorker(void *_task) {
  char me[]="_pushWorker", *err;
  pushTask *task;
  
  task = (pushTask *)_task;

  while (1) {
    if (task->pctx->verbose > 1) {
      fprintf(stderr, "%s(%d): waiting to check finished\n",
              me, task->threadIdx);
    }
    /* pushFinish sets finished prior to the barriers */
    airThreadBarrierWait(task->pctx->stageBarrierA);
    if (task->pctx->finished) {
      if (task->pctx->verbose > 1) {
        fprintf(stderr, "%s(%d): done!\n", me, task->threadIdx);
      }
      break;
    }
    /* else there's work to do ... */
    
    if (task->pctx->verbose > 1) {
      fprintf(stderr, "%s(%d): starting to run stage %d\n",
              me, task->threadIdx, task->pctx->stageIdx);
    }
    if (_pushStageRun(task, task->pctx->stageIdx)) {
      err = biffGetDone(PUSH);
      fprintf(stderr, "%s: task %d trouble with stage %d:\n%s", me,
              task->threadIdx, task->pctx->stageIdx, err);
      /* HEY: we should be using the "finished" mechanism to
         shut the whole production down */
    }
    airThreadBarrierWait(task->pctx->stageBarrierB);
  }

  return _task;
}

int
_pushContextCheck(pushContext *pctx) {
  char me[]="_pushContextCheck", err[BIFF_STRLEN];
  unsigned int sidx;
  int nul;
  
  if (!pctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(PUSH, err); return 1;
  }
  if (!( AIR_IN_CL(1, pctx->numStage, PUSH_STAGE_MAXNUM) )) {
    sprintf(err, "%s: pctx->numStage (%d) outside valid range [1,%d]", me,
            pctx->numStage, PUSH_STAGE_MAXNUM);
    biffAdd(PUSH, err); return 1;
  }
  nul = AIR_FALSE;
  for (sidx=0; sidx<pctx->numStage; sidx++) {
    nul |= !(pctx->process[sidx]);
  }
  if (nul) {
    sprintf(err, "%s: one or more of the process functions was NULL", me);
    biffAdd(PUSH, err); return 1;
  }
  if (!( pctx->numThing >= 1 )) {
    sprintf(err, "%s: pctx->numThing (%d) not >= 1\n", me, pctx->numThing);
    biffAdd(PUSH, err); return 1;
  }
  if (!( pctx->numThread >= 1 )) {
    sprintf(err, "%s: pctx->numThread (%d) not >= 1\n", me, pctx->numThread);
    biffAdd(PUSH, err); return 1;
  }
  if (!( AIR_IN_CL(1, pctx->numThread, PUSH_THREAD_MAXNUM) )) {
    sprintf(err, "%s: pctx->numThread (%d) outside valid range [1,%d]", me,
            pctx->numThread, PUSH_THREAD_MAXNUM);
    biffAdd(PUSH, err); return 1;
  }

  if (nrrdCheck(pctx->nin)) {
    sprintf(err, "%s: got a broken input nrrd", me);
    biffMove(PUSH, err, NRRD); return 1;
  }
  if (!( (3 == pctx->nin->dim && 4 == pctx->nin->axis[0].size) 
         || (4 == pctx->nin->dim && 7 == pctx->nin->axis[0].size) )) {
    sprintf(err, "%s: input doesn't look like 2D or 3D masked tensors", me);
    biffAdd(PUSH, err); return 1;
  }

  if (pctx->npos) {
    if (nrrdCheck(pctx->npos)) {
      sprintf(err, "%s: got a broken position nrrd", me);
      biffMove(PUSH, err, NRRD); return 1;
    }
    if (!( 2 == pctx->npos->dim 
           && 3 == pctx->npos->axis[0].size )) {
      sprintf(err, "%s: position nrrd not 2-D 3-by-N", me);
      biffAdd(PUSH, err); return 1;
    }
  }
  if (pctx->nstn) {
    if (!pctx->npos) {
      sprintf(err, "%s: can't have start/num nrrd w/out position nrrd", me);
      biffAdd(PUSH, err); return 1;
    }
    if (nrrdCheck(pctx->nstn)) {
      sprintf(err, "%s: got a broken start/num nrrd", me);
      biffMove(PUSH, err, NRRD); return 1;
    }
    if (!( 2 == pctx->nstn->dim 
           && nrrdTypeInt == pctx->nstn->type
           && 3 == pctx->nstn->axis[0].size )) {
      sprintf(err, "%s: start/num nrrd not 2-D 3-by-N array of %ss", me,
              airEnumStr(nrrdType, nrrdTypeInt));
      biffAdd(PUSH, err); return 1;
    }
  }
  return 0;
}

int
pushStart(pushContext *pctx) {
  char me[]="pushStart", err[BIFF_STRLEN];
  unsigned int tidx;

  if (_pushContextCheck(pctx)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(PUSH, err); return 1;
  }

  airSrandMT(pctx->seed);
  
  /* the ordering of things below is important: gage and fiber contexts
     have to be set up before they're copied by task setup */
  if (_pushTensorFieldSetup(pctx)
      || _pushGageSetup(pctx) 
      || _pushFiberSetup(pctx)
      || _pushTaskSetup(pctx)
      || _pushBinSetup(pctx)
      || _pushThingSetup(pctx)) {
    sprintf(err, "%s: trouble setting up context", me);
    biffAdd(PUSH, err); return 1;
  }

  /* HEY: this should be done by the user */
  pctx->process[0] = _pushForce;
  pctx->process[1] = _pushUpdate;

  pctx->finished = AIR_FALSE;
  if (pctx->numThread > 1) {
    pctx->binMutex = airThreadMutexNew();
    pctx->stageBarrierA = airThreadBarrierNew(pctx->numThread);
    pctx->stageBarrierB = airThreadBarrierNew(pctx->numThread);
  }

  /* start threads 1 and up running; they'll all hit stageBarrierA  */
  for (tidx=1; tidx<pctx->numThread; tidx++) {
    if (pctx->verbose > 1) {
      fprintf(stderr, "%s: spawning thread %d\n", me, tidx);
    }
    airThreadStart(pctx->task[tidx]->thread, _pushWorker,
                   (void *)(pctx->task[tidx]));
  }

  return 0;
}

/*
******** pushIterate
**
** (documentation)
**
** NB: this implements the body of thread 0
*/
int
pushIterate(pushContext *pctx) {
  char me[]="pushIterate", *_err, err[BIFF_STRLEN];
  unsigned int ti, numThing;

  if (!pctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(PUSH, err); return 1;
  }
  
  if (pctx->verbose) {
    fprintf(stderr, "%s: starting iteration\n", me);
  }
  /* the _pushWorker checks finished after the barriers */
  pctx->finished = AIR_FALSE;
  pctx->binIdx=0;
  pctx->stageIdx=0;
  for (ti=0; ti<pctx->numThread; ti++) {
    pctx->task[ti]->sumVel = 0;
    pctx->task[ti]->numThing = 0;
  }
  do {
    if (pctx->numThread > 1) {
      airThreadBarrierWait(pctx->stageBarrierA);
    }
    if (pctx->verbose) {
      fprintf(stderr, "%s: starting iter %d stage %d\n", me, 
              pctx->iter, pctx->stageIdx);
    }
    if (_pushStageRun(pctx->task[0], pctx->stageIdx)) {
      _err = biffGetDone(PUSH);
      fprintf(stderr, "%s: task %d trouble w/ iter %d stage %d:\n%s", me,
              pctx->task[0]->threadIdx, pctx->iter,
              pctx->task[0]->pctx->stageIdx, _err);
      return 1;
    }
    if (pctx->numThread > 1) {
      airThreadBarrierWait(pctx->stageBarrierB);
    }
    /* This is the only code to happen between barriers */
    pctx->stageIdx++;
    pctx->binIdx=0;
  } while (pctx->stageIdx < pctx->numStage);
  pctx->meanVel = 0;
  numThing = 0;
  for (ti=0; ti<pctx->numThread; ti++) {
    pctx->meanVel += pctx->task[ti]->sumVel;
    /*
    fprintf(stderr, "!%s: task %d sumVel = %g\n", me,
            ti, pctx->task[ti]->sumVel);
    */
    numThing += pctx->task[ti]->numThing;
  }
  pctx->meanVel /= numThing;
  if (pushRebin(pctx)) {
    sprintf(err, "%s: problem with new point locations", me);
    biffAdd(PUSH, err); return 1;
  }
  if (0 && 100 == pctx->iter) {
    _pushForceSample(pctx, 300, 300);
  }
  
  return 0;
}

int
pushRun(pushContext *pctx) {
  char me[]="pushRun", err[BIFF_STRLEN],
    poutS[AIR_STRLEN_MED], toutS[AIR_STRLEN_MED], soutS[AIR_STRLEN_MED];
  Nrrd *npos, *nten, *nstn;
  double vel[2], meanVel=0;
  
  pctx->iter = 0;
  pctx->time0 = airTime();
  vel[0] = AIR_NAN;
  vel[1] = AIR_NAN;
  do {
    if (400 == pctx->iter) {
      unsigned int bi, ti;
      pushBin *bin;
      for (bi=0; bi<pctx->numBin; bi++) {
        bin = pctx->bin + bi;
        for (ti=0; ti<bin->numThing; ti++) {
          ELL_3V_SET(bin->thing[ti]->point.vel, 0, 0, 0);
        }
      }
    }
    if (pushIterate(pctx)) {
      sprintf(err, "%s: trouble on iter %d", me, pctx->iter);
      biffAdd(PUSH, err); return 1;
    }
    if (pctx->snap && !(pctx->iter % pctx->snap)) {
      nten = nrrdNew();
      npos = nrrdNew();
      nstn = nrrdNew();
      sprintf(poutS, "snap.%06d.pos.nrrd", pctx->iter);
      sprintf(toutS, "snap.%06d.ten.nrrd", pctx->iter);
      sprintf(soutS, "snap.%06d.stn.nrrd", pctx->iter);
      if (pushOutputGet(npos, nten, nstn, pctx)) {
        sprintf(err, "%s: couldn't get snapshot for iter %d", me, pctx->iter);
        biffAdd(PUSH, err); return 1;
      }
      fprintf(stderr, "%s: %s, meanVel=%g, %g iter/sec\n", me,
              poutS, meanVel, pctx->iter/(airTime()-pctx->time0));
      if (nrrdSave(poutS, npos, NULL)
          || nrrdSave(toutS, nten, NULL)
          || nrrdSave(soutS, nstn, NULL)) {
        sprintf(err, "%s: couldn't save snapshot for iter %d", me, pctx->iter);
        biffMove(PUSH, err, NRRD); return 1;
      }
      nten = nrrdNuke(nten);
      npos = nrrdNuke(npos);
      nstn = nrrdNuke(nstn);
    }
    /* this goofiness is because it seems like my stupid Euler 
       integration can lead to real motion only happening on
       every other iteration ... */
    if (0 == pctx->iter) {
      vel[0] = pctx->meanVel;
      meanVel = pctx->meanVel;
    } else if (1 == pctx->iter) {
      vel[1] = pctx->meanVel;
      meanVel = (vel[0] + vel[1])/2;
    } else {
      vel[0] = vel[1];
      vel[1] = pctx->meanVel;
      meanVel = (vel[0] + vel[1])/2;
    }
    pctx->iter++;
  } while ( (pctx->iter < pctx->minIter)
            || (meanVel > pctx->minMeanVel
                && (0 == pctx->maxIter
                    || pctx->iter < pctx->maxIter)) );
  pctx->time1 = airTime();
  pctx->time = pctx->time1 - pctx->time0;

  return 0;
}

/*
** this is called *after* pushOutputGet
**
** should nix everything created by the many _push*Setup() functions
*/
int
pushFinish(pushContext *pctx) {
  char me[]="pushFinish", err[BIFF_STRLEN];
  unsigned int ii, tidx;

  if (!pctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(PUSH, err); return 1;
  }

  if (pctx->verbose > 1) {
    fprintf(stderr, "%s: finishing workers\n", me);
  }
  pctx->finished = AIR_TRUE;
  if (pctx->numThread > 1) {
    airThreadBarrierWait(pctx->stageBarrierA);
  }
  for (tidx=pctx->numThread; tidx>0; tidx--) {
    if (tidx-1) {
      airThreadJoin(pctx->task[tidx-1]->thread,
                    &(pctx->task[tidx-1]->returnPtr));
    }
    pctx->task[tidx-1]->thread = airThreadNix(pctx->task[tidx-1]->thread);
    pctx->task[tidx-1] = _pushTaskNix(pctx->task[tidx-1]);
  }
  pctx->task = (pushTask **)airFree(pctx->task);

  pctx->nten = nrrdNuke(pctx->nten);
  pctx->ninv = nrrdNuke(pctx->ninv);
  pctx->nmask = nrrdNuke(pctx->nmask);
  pctx->gctx = gageContextNix(pctx->gctx);
  pctx->fctx = tenFiberContextNix(pctx->fctx);
  for (ii=0; ii<pctx->numBin; ii++) {
    pushBinDone(pctx->bin + ii);
  }
  pctx->bin = (pushBin *)airFree(pctx->bin);
  pctx->binsEdge = pctx->numBin = 0;

  if (pctx->numThread > 1) {
    pctx->binMutex = airThreadMutexNix(pctx->binMutex);
    pctx->stageBarrierA = airThreadBarrierNix(pctx->stageBarrierA);
    pctx->stageBarrierB = airThreadBarrierNix(pctx->stageBarrierB);
  }

  return 0;
}
