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

#define _COIL_IV3_FILL(radius, diam, valLen) \
  if (0 && x0) { \
    /* cycle through slices */ \
    tmp = iv3[0]; \
    for (xni=0; xni<diam-1; xni++) { \
      iv3[xni] = iv3[xni+1]; \
    } \
    iv3[diam-1] = tmp; \
    /* refill only newest one */ \
    xni = diam-1; \
    xvi = AIR_CLAMP(0, xni-(int)radius+x0, sizeX-1) - x0; \
    for (zni=0; zni<diam; zni++) { \
      zvi = AIR_CLAMP(0, zni-(int)radius+z0, sizeZ-1) - z0; \
      for (yni=0; yni<diam; yni++) { \
        yvi = AIR_CLAMP(0, yni-(int)radius+y0, sizeY-1) - y0; \
        for (vi=0; vi<valLen; vi++) { \
          iv3[xni][vi + valLen*(yni + diam*zni)] =  \
            here[vi + valLen*(0 + 2*(xvi + sizeX*(yvi + sizeY*zvi)))]; \
        } \
      } \
    } \
  } else { \
    /* have to re-fill entire thing */ \
    for (zni=0; zni<diam; zni++) { \
      zvi = AIR_CLAMP(0, zni-(int)radius+z0, sizeZ-1) - z0; \
      for (yni=0; yni<diam; yni++) { \
        yvi = AIR_CLAMP(0, yni-(int)radius+y0, sizeY-1) - y0; \
        for (xni=0; xni<diam; xni++) { \
          xvi = AIR_CLAMP(0, xni-(int)radius+x0, sizeX-1) - x0; \
          for (vi=0; vi<valLen; vi++) { \
            iv3[xni][vi + valLen*(yni + diam*zni)] =  \
              here[vi + valLen*(0 + 2*(xvi + sizeX*(yvi + sizeY*zvi)))]; \
          } \
        } \
      } \
    } \
  }

/*
** 
** iv3 is: diam x diam x diam x valLen
**
** this should be parameterized on both radius and valLen
*/
void
_coilIv3Fill_R_L(coil_t **iv3, coil_t *here, unsigned int radius, int valLen,
                 int x0, int y0, int z0, int sizeX, int sizeY, int sizeZ) {
  int diam, vi,    /* value index */
    xni, yni, zni, /* neighborhood (iv3) indices */
    xvi, yvi, zvi; /* volume indices */
  coil_t *tmp;
  
  diam = 1 + 2*radius;
  _COIL_IV3_FILL(radius, diam, valLen);
  return;
}

void
_coilIv3Fill_1_1(coil_t **iv3, coil_t *here, unsigned int radius, int valLen,
                 int x0, int y0, int z0, int sizeX, int sizeY, int sizeZ) {
  int vi,          /* value index */
    xni, yni, zni, /* neighborhood (iv3) indices */
    xvi, yvi, zvi; /* volume indices */
  coil_t *tmp;

  AIR_UNUSED(radius);
  AIR_UNUSED(valLen);

  _COIL_IV3_FILL(1, 3, 1);
  return;
}

void
_coilIv3Fill_1_7(coil_t **iv3, coil_t *here, unsigned int radius, int valLen,
             int x0, int y0, int z0, int sizeX, int sizeY, int sizeZ) {
  int vi,          /* value index */
    xni, yni, zni, /* neighborhood (iv3) indices */
    xvi, yvi, zvi; /* volume indices */
  coil_t *tmp;

  AIR_UNUSED(radius);
  AIR_UNUSED(valLen);

  _COIL_IV3_FILL(1, 3, 7);
  return;
}

int
_coilThisZGet(coilTask *task, int doFilter) {
  int thisZ, *thisFlag, *thatFlag;

  if (doFilter) {
    thisFlag = &(task->cctx->todoFilter);
    thatFlag = &(task->cctx->todoUpdate);
  } else {
    thisFlag = &(task->cctx->todoUpdate);
    thatFlag = &(task->cctx->todoFilter);
  }

  airThreadMutexLock(task->cctx->nextSliceMutex);
  if (task->cctx->nextSlice == task->cctx->size[2]
      && *thisFlag) {
    /* we're the first thread to start this phase */
    task->cctx->nextSlice = 0;
    *thisFlag = AIR_FALSE;
  }
  thisZ = task->cctx->nextSlice;
  if (task->cctx->nextSlice < task->cctx->size[2]) {
    task->cctx->nextSlice++;
    if (task->cctx->nextSlice == task->cctx->size[2]) {
      /* we just grabbed the last slice of this phase */
      *thatFlag = AIR_TRUE;
    }
  }
  airThreadMutexUnlock(task->cctx->nextSliceMutex);
  return thisZ;
}

void
_coilProcess(coilTask *task, int doFilter) {
  int xi, yi, sizeX, sizeY, thisZ, sizeZ, valLen, radius;
  coil_t *here;
  void (*filter)(coil_t *delta, coil_t **iv3, 
                 double spacing[3],
                 double parm[COIL_PARMS_NUM]);
  
  sizeX = task->cctx->size[0];
  sizeY = task->cctx->size[1];
  sizeZ = task->cctx->size[2];
  valLen = task->cctx->kind->valLen;
  radius = task->cctx->radius;
  filter = task->cctx->kind->filter[task->cctx->method->type];
  if (doFilter) {
    while (1) {
      thisZ = _coilThisZGet(task, doFilter);
      if (thisZ == sizeZ) {
        break;
      }
      here = (coil_t*)(task->cctx->nvol->data) + 2*valLen*sizeX*sizeY*thisZ;
      for (yi=0; yi<sizeY; yi++) {
        for (xi=0; xi<sizeX; xi++) {
          task->iv3Fill(task->iv3, here + 0*valLen, radius, valLen,
                        xi, yi, thisZ, sizeX, sizeY, sizeZ);
          filter(here + 1*valLen, task->iv3,
                 task->cctx->spacing, task->cctx->parm);
          here += 2*valLen;
        }
      }
    }
  } else {
    while (1) {
      thisZ = _coilThisZGet(task, doFilter);
      if (thisZ == sizeZ) {
        break;
      }
      here = (coil_t*)(task->cctx->nvol->data) + 2*valLen*sizeX*sizeY*thisZ;
      for (yi=0; yi<sizeY; yi++) {
        for (xi=0; xi<sizeX; xi++) {
          task->cctx->kind->update(here + 0*valLen, here + 1*valLen);
          here += 2*valLen;
        }
      }
    }
  }
  return;
}

coilTask *
_coilTaskNew(coilContext *cctx, int threadIdx) {
  coilTask *task;
  int len, diam, xi;

  len = cctx->kind->valLen;
  diam = 1 + 2*cctx->radius;
  task = (coilTask *)calloc(1, sizeof(coilTask));
  if (task) {
    task->cctx = cctx;
    task->thread = airThreadNew();
    task->threadIdx = threadIdx;
    task->_iv3 = (coil_t*)calloc(len*diam*diam*diam, sizeof(coil_t));
    task->iv3 = (coil_t**)calloc(diam, sizeof(coil_t*));
    for (xi=0; xi<diam; xi++) {
      task->iv3[xi] = task->_iv3 + xi*len*diam*diam;
    }
    if (1 == cctx->radius && 1 == cctx->kind->valLen) {
      task->iv3Fill = _coilIv3Fill_1_1;
    } else if (1 == cctx->radius && 7 == cctx->kind->valLen) {
      task->iv3Fill = _coilIv3Fill_1_7;
    } else {
      task->iv3Fill = _coilIv3Fill_R_L;
    }
    task->returnPtr = NULL;
  }
  return task;
}

coilTask *
_coilTaskNix(coilTask *task) {

  if (task) {
    task->thread = airThreadNix(task->thread);
    task->_iv3 = (coil_t *)airFree(task->_iv3);
    task->iv3 = (coil_t **)airFree(task->iv3);
    free(task);
  }
  return NULL;
}

void *
_coilWorker(void *_task) {
  char me[]="_coilWorker";
  coilTask *task;

  task = (coilTask *)_task;

  while (1) {
    /* wait until parent has set cctx->finished */
    if (task->cctx->verbose > 1) {
      fprintf(stderr, "%s(%d): waiting to check finished\n",
              me, task->threadIdx);
    }
    airThreadBarrierWait(task->cctx->filterBarrier);
    if (task->cctx->finished) {
      if (task->cctx->verbose > 1) {
        fprintf(stderr, "%s(%d): done!\n", me, task->threadIdx);
      }
      break;
    }
    /* else there's work to do ... */

    /* first: filter */
    if (task->cctx->verbose > 1) {
      fprintf(stderr, "%s(%d): filtering ... \n",
              me, task->threadIdx);
    }
    _coilProcess(task, AIR_TRUE);

    /* second: update */
    airThreadBarrierWait(task->cctx->updateBarrier);
    if (task->cctx->verbose > 1) {
      fprintf(stderr, "%s(%d): updating ... \n",
              me, task->threadIdx);
    }
    _coilProcess(task, AIR_FALSE);

  }

  return _task;
}

int
coilStart(coilContext *cctx) {
  char me[]="coilStart", err[BIFF_STRLEN];
  int valIdx, valLen;
  coil_t (*lup)(const void*, size_t), *val;
  unsigned tidx, elIdx;

  if (!cctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(COIL, err); return 1;
  }
  cctx->task = (coilTask **)calloc(cctx->numThreads, sizeof(coilTask *));
  if (!(cctx->task)) {
    sprintf(err, "%s: couldn't allocate array of tasks", me);
    biffAdd(COIL, err); return 1;
  }
  
  /* we create tasks for ALL threads, including me, thread 0 */
  cctx->task[0] = NULL;
  for (tidx=0; tidx<cctx->numThreads; tidx++) {
    cctx->task[tidx] = _coilTaskNew(cctx, tidx);
    if (!(cctx->task[tidx])) {
      sprintf(err, "%s: couldn't allocate task %d", me, tidx);
      biffAdd(COIL, err); return 1;
    }
  }
  
  cctx->finished = AIR_FALSE;
  if (cctx->numThreads > 1) {
    cctx->nextSliceMutex = airThreadMutexNew();
    cctx->filterBarrier = airThreadBarrierNew(cctx->numThreads);
    cctx->updateBarrier = airThreadBarrierNew(cctx->numThreads);
  }

  /* initialize the values in cctx->nvol */
  val = (coil_t*)(cctx->nvol->data);
  valLen = cctx->kind->valLen;
#if COIL_TYPE_FLOAT
  lup = nrrdFLookup[cctx->nin->type];
#else
  lup = nrrdDLookup[cctx->nin->type];
#endif
  for (elIdx=0; elIdx<cctx->size[0]*cctx->size[1]*cctx->size[2]; elIdx++) {
    for (valIdx=0; valIdx<valLen; valIdx++) {
      val[valIdx + 0*valLen] = lup(cctx->nin->data, valIdx + valLen*elIdx);
      val[valIdx + 1*valLen] = 0;
    }
    val += 2*valLen;
  }
  
  /* start threads 1 and up running; they'll all hit filterBarrier  */
  for (tidx=1; tidx<cctx->numThreads; tidx++) {
    if (cctx->verbose > 1) {
      fprintf(stderr, "%s: spawning thread %d\n", me, tidx);
    }
    airThreadStart(cctx->task[tidx]->thread, _coilWorker,
                   (void *)(cctx->task[tidx]));
  }

  /* set things as though we've just finished an update phase */
  cctx->nextSlice = cctx->size[2];
  cctx->todoFilter = AIR_TRUE;
  cctx->todoUpdate = AIR_FALSE;
  
  return 0;
}

/*
******** coilIterate
**
** (documentation)
**
** NB: this implements the body of thread 0
*/
int
coilIterate(coilContext *cctx, int numIterations) {
  char me[]="coilIterate", err[BIFF_STRLEN];
  int iter;
  double time0, time1;

  if (!cctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(COIL, err); return 1;
  }
  
  time0 = airTime();
  for (iter=0; iter<numIterations; iter++) {
    if (cctx->verbose) {
      fprintf(stderr, "%s: starting iter %d (of %d)\n", me, iter, 
              numIterations);
    }
    cctx->finished = AIR_FALSE;
    if (cctx->numThreads > 1) {
      airThreadBarrierWait(cctx->filterBarrier);
    }
    
    /* first: filter */
    if (cctx->verbose > 1) {
      fprintf(stderr, "%s: filtering ... \n", me);
    }
    _coilProcess(cctx->task[0], AIR_TRUE);

    /* second: update */
    if (cctx->verbose > 1) {
      fprintf(stderr, "%s: updating ... \n", me);
    }
    if (cctx->numThreads > 1) {
      airThreadBarrierWait(cctx->updateBarrier);
    }
    _coilProcess(cctx->task[0], AIR_FALSE);

  }
  time1 = airTime();
  if (cctx->verbose) {
    fprintf(stderr, "%s: elapsed time = %g (%g/iter)\n", me,
            time1 - time0, (time1 - time0)/numIterations);
  }
  return 0;
}

int
coilFinish(coilContext *cctx) {
  char me[]="coilFinish", err[BIFF_STRLEN];
  unsigned int tidx;

  if (!cctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(COIL, err); return 1;
  }

  if (cctx->verbose > 1) {
    fprintf(stderr, "%s: finishing workers\n", me);
  }
  cctx->finished = AIR_TRUE;
  if (cctx->numThreads > 1) {
    airThreadBarrierWait(cctx->filterBarrier);
  }
  for (tidx=1; tidx<cctx->numThreads; tidx++) {
    airThreadJoin(cctx->task[tidx]->thread, &(cctx->task[tidx]->returnPtr));
    cctx->task[tidx]->thread = airThreadNix(cctx->task[tidx]->thread);
    cctx->task[tidx] = _coilTaskNix(cctx->task[tidx]);
  }
  cctx->task[0]->thread = airThreadNix(cctx->task[0]->thread);
  cctx->task[0] = _coilTaskNix(cctx->task[0]);
  cctx->task = (coilTask **)airFree(cctx->task);

  if (cctx->numThreads > 1) {
    cctx->nextSliceMutex = airThreadMutexNix(cctx->nextSliceMutex);
    cctx->filterBarrier = airThreadBarrierNix(cctx->filterBarrier);
    cctx->updateBarrier = airThreadBarrierNix(cctx->updateBarrier);
  }

  return 0;
}
