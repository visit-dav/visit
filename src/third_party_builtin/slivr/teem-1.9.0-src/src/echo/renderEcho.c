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

#include "echo.h"
#include "privateEcho.h"

int
echoThreadStateInit(int threadIdx, echoThreadState *tstate,
                    echoRTParm *parm, echoGlobalState *gstate) {
  char me[]="echoThreadStateInit", err[BIFF_STRLEN];

  if (!(tstate && parm && gstate)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(ECHO, err); return 1;
  }
  /* tstate->thread set by echoThreadStateNew() */
  tstate->gstate = gstate;
  /* this will probably be over-written */
  tstate->verbose = gstate->verbose;
  tstate->threadIdx = threadIdx;
  if (nrrdMaybeAlloc_va(tstate->nperm, nrrdTypeInt, 2,
                        AIR_CAST(size_t, ECHO_JITTABLE_NUM),
                        AIR_CAST(size_t, parm->numSamples))) {
    sprintf(err, "%s: couldn't allocate jitter permutation array", me);
    biffMove(ECHO, err, NRRD); return 1;
  }
  nrrdAxisInfoSet_va(tstate->nperm, nrrdAxisInfoLabel,
                     "jittable", "sample");

  if (nrrdMaybeAlloc_va(tstate->njitt, echoPos_nt, 3,
                        AIR_CAST(size_t, 2),
                        AIR_CAST(size_t, ECHO_JITTABLE_NUM),
                        AIR_CAST(size_t, parm->numSamples))) {
    sprintf(err, "%s: couldn't allocate jitter array", me);
    biffMove(ECHO, err, NRRD); return 1;
  }
  nrrdAxisInfoSet_va(tstate->njitt, nrrdAxisInfoLabel,
                     "x,y", "jittable", "sample");

  tstate->permBuff = AIR_CAST(unsigned int *, airFree(tstate->permBuff));
  if (!(tstate->permBuff = AIR_CAST(unsigned int *,
                                    calloc(parm->numSamples, sizeof(int))))) {
    sprintf(err, "%s: couldn't allocate permutation buffer", me);
    biffAdd(ECHO, err); return 1;
  }
  tstate->chanBuff = (echoCol_t *)airFree(tstate->chanBuff);
  if (!( tstate->chanBuff =
         (echoCol_t*)calloc(ECHO_IMG_CHANNELS * parm->numSamples,
                            sizeof(echoCol_t)) )) {
    sprintf(err, "%s: couldn't allocate img channel sample buffer", me);
    biffAdd(ECHO, err); return 1;
  }

  airSrandMT_r(tstate->rst, AIR_CAST(unsigned int, (parm->seedRand
                                                    ? airTime()
                                                    : threadIdx)));
  tstate->returnPtr = NULL;
  
  return 0;
}

/*
******** echoJitterCompute()
**
**
*/
void
echoJitterCompute(echoRTParm *parm, echoThreadState *tstate) {
  echoPos_t *jitt, w;
  int s, i, j, xi, yi, n, N, *perm;

  N = parm->numSamples;
  n = (int)sqrt(N);
  w = 1.0/n;
  /* each row in perm[] is for one sample, for going through all jittables;
     each column is a different permutation of [0..parm->numSamples-1] */
  perm = (int *)tstate->nperm->data;
  for (j=0; j<ECHO_JITTABLE_NUM; j++) {
    airShuffle_r(tstate->rst, tstate->permBuff,
                 parm->numSamples, parm->permuteJitter);
    for (s=0; s<N; s++) {
      perm[j + ECHO_JITTABLE_NUM*s] = tstate->permBuff[s];
    }
  }
  jitt = (echoPos_t *)tstate->njitt->data;
  for (s=0; s<N; s++) {
    for (j=0; j<ECHO_JITTABLE_NUM; j++) {
      i = perm[j + ECHO_JITTABLE_NUM*s];
      xi = i % n;
      yi = i / n;
      switch(parm->jitterType) {
      case echoJitterNone:
        jitt[0 + 2*j] = 0.0;
        jitt[1 + 2*j] = 0.0;
        break;
      case echoJitterGrid:
        jitt[0 + 2*j] = NRRD_POS(nrrdCenterCell, -0.5, 0.5, n, xi);
        jitt[1 + 2*j] = NRRD_POS(nrrdCenterCell, -0.5, 0.5, n, yi);
        break;
      case echoJitterJitter:
        jitt[0 + 2*j] = (NRRD_POS(nrrdCenterCell, -0.5, 0.5, n, xi)
                         + w*(airDrandMT_r(tstate->rst) - 0.5));
        jitt[1 + 2*j] = (NRRD_POS(nrrdCenterCell, -0.5, 0.5, n, yi)
                         + w*(airDrandMT_r(tstate->rst) - 0.5));
        break;
      case echoJitterRandom:
        jitt[0 + 2*j] = airDrandMT_r(tstate->rst) - 0.5;
        jitt[1 + 2*j] = airDrandMT_r(tstate->rst) - 0.5;
        break;
      }
    }
    jitt += 2*ECHO_JITTABLE_NUM;
  }

  return;
}

/*
******** echoRTRenderCheck
**
** does all the error checking required of echoRTRender and
** everything that it calls
*/
int
echoRTRenderCheck(Nrrd *nraw, limnCamera *cam, echoScene *scene,
                  echoRTParm *parm, echoGlobalState *gstate) {
  char me[]="echoRTRenderCheck", err[BIFF_STRLEN];
  int tmp;

  if (!(nraw && cam && scene && parm && gstate)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(ECHO, err); return 1;
  }
  if (limnCameraUpdate(cam)) {
    sprintf(err, "%s: camera trouble", me);
    biffMove(ECHO, err, LIMN); return 1;
  }
  if (scene->envmap) {
    if (limnEnvMapCheck(scene->envmap)) {
      sprintf(err, "%s: environment map not valid", me);
      biffMove(ECHO, err, LIMN); return 1;
    }
  }
  if (airEnumValCheck(echoJitter, parm->jitterType)) {
    sprintf(err, "%s: jitter method (%d) invalid", me, parm->jitterType);
    biffAdd(ECHO, err); return 1;
  }
  if (!(parm->numSamples > 0)) {
    sprintf(err, "%s: # samples (%d) invalid", me, parm->numSamples);
    biffAdd(ECHO, err); return 1;
  }
  if (!(parm->imgResU > 0 && parm->imgResV)) {
    sprintf(err, "%s: image dimensions (%dx%d) invalid", me,
            parm->imgResU, parm->imgResV);
    biffAdd(ECHO, err); return 1;
  }
  if (!AIR_EXISTS(parm->aperture)) {
    sprintf(err, "%s: aperture doesn't exist", me);
    biffAdd(ECHO, err); return 1;
  }
  
  switch (parm->jitterType) {
  case echoJitterNone:
  case echoJitterRandom:
    break;
  case echoJitterGrid:
  case echoJitterJitter:
    tmp = (int)sqrt(parm->numSamples);
    if (tmp*tmp != parm->numSamples) {
      sprintf(err, "%s: need a square # samples for %s jitter method (not %d)",
              me, airEnumStr(echoJitter, parm->jitterType), parm->numSamples);
      biffAdd(ECHO, err); return 1;
    }
    break;
  }

  /* for the time being things are hard-coded to be r,g,b,a,time */
  if (ECHO_IMG_CHANNELS != 5) {
    sprintf(err, "%s: ECHO_IMG_CHANNELS != 5", me);
    biffAdd(ECHO, err); return 1;
  }
  
  /* all is well */
  return 0;
}

void
echoChannelAverage(echoCol_t *img,
                   echoRTParm *parm, echoThreadState *tstate) {
  int s;
  echoCol_t R, G, B, A, T;
  
  R = G = B = A = T = 0;
  for (s=0; s<parm->numSamples; s++) {
    R += tstate->chanBuff[0 + ECHO_IMG_CHANNELS*s];
    G += tstate->chanBuff[1 + ECHO_IMG_CHANNELS*s];
    B += tstate->chanBuff[2 + ECHO_IMG_CHANNELS*s];
    A += tstate->chanBuff[3 + ECHO_IMG_CHANNELS*s];
    T += tstate->chanBuff[4 + ECHO_IMG_CHANNELS*s];
  }
  img[0] = R / parm->numSamples;
  img[1] = G / parm->numSamples;
  img[2] = B / parm->numSamples;
  img[3] = A / parm->numSamples;
  img[4] = T;
  
  return;
}

/*
******** echoRayColor
**
** This is called by echoRTRender and by the various color routines,
** following an intersection with non-phong non-light material (the
** things that require reflection or refraction rays).  As such, it is
** never called on shadow rays.
*/
void
echoRayColor(echoCol_t *chan, echoRay *ray,
             echoScene *scene, echoRTParm *parm, echoThreadState *tstate) {
  char me[]="echoRayColor";
  echoCol_t rgba[4];
  echoIntx intx;
  
  tstate->depth++;
  if (tstate->depth > parm->maxRecDepth) {
    /* we've exceeded the recursion depth, so no more rays for you */
    ELL_4V_SET(chan, parm->maxRecCol[0], parm->maxRecCol[1],
               parm->maxRecCol[2], 1.0);
    goto done;
  }

  intx.boxhits = 0;
  if (!echoRayIntx(&intx, ray, scene, parm, tstate)) {
    if (tstate->verbose) {
      fprintf(stderr, "%s%s: (nothing was hit)\n",_echoDot(tstate->depth), me);
    }
    /* ray hits nothing in scene */
    ELL_4V_SET_TT(chan, echoCol_t,
                  scene->bkgr[0], scene->bkgr[1], scene->bkgr[2],
                  (parm->renderBoxes
                   ? 1.0 - pow(1.0 - parm->boxOpac, intx.boxhits)
                   : 1.0));
    goto done;
  }

  if (tstate->verbose) {
    fprintf(stderr, "%s%s: hit a %d (%p) at (%g,%g,%g)\n"
            "%s    = %g along (%g,%g,%g)\n", _echoDot(tstate->depth), me,
            intx.obj->type, AIR_CAST(void*, intx.obj),
            intx.pos[0], intx.pos[1], intx.pos[2], _echoDot(tstate->depth),
            intx.t, ray->dir[0], ray->dir[1], ray->dir[2]);
  }
  echoIntxColor(rgba, &intx, scene, parm, tstate);
  ELL_4V_COPY(chan, rgba);
 done:
  tstate->depth--;
  return;
}

void *
_echoRTRenderThreadBody(void *_arg) {
  char done[20];
  int imgUi, imgVi,         /* integral pixel indices */
    samp;                   /* which sample are we doing */
  echoPos_t tmp0, tmp1,
    pixUsz, pixVsz,         /* U and V dimensions of a pixel */
    U[4], V[4], N[4],       /* view space basis (only first 3 elements used) */
    imgU, imgV,             /* floating point pixel center locations */
    eye[3],                 /* eye center before jittering */
    at[3],                  /* ray destination (pixel center post-jittering) */
    imgOrig[3];             /* image origin */
  double time0;
  echoRay ray;              /* (not a pointer) */
  echoThreadState *arg;
  echoCol_t *img, *chan;    /* current scanline of channel buffer array */
  Nrrd *nraw;               /* copies of arguments to echoRTRender ... */
  limnCamera *cam;
  echoScene *scene;
  echoRTParm *parm;

  arg = (echoThreadState *)_arg;
  nraw = arg->gstate->nraw;
  cam = arg->gstate->cam;
  scene = arg->gstate->scene;
  parm = arg->gstate->parm;

  echoJitterCompute(arg->gstate->parm, arg);
  if (arg->gstate->verbose > 2) {
    nrrdSave("jitt.nrrd", arg->njitt, NULL);
  }
  
  /* set eye, U, V, N, imgOrig */
  ELL_3V_COPY(eye, arg->gstate->cam->from);
  ELL_4MV_ROW0_GET(U, cam->W2V);
  ELL_4MV_ROW1_GET(V, cam->W2V);
  ELL_4MV_ROW2_GET(N, cam->W2V);
  ELL_3V_SCALE_ADD2(imgOrig, 1.0, eye, cam->vspDist, N);
  
  /* determine size of a single pixel (based on cell-centering) */
  pixUsz = (cam->uRange[1] - cam->uRange[0])/(parm->imgResU);
  pixVsz = (cam->vRange[1] - cam->vRange[0])/(parm->imgResV);

  arg->depth = 0;
  ray.shadow = AIR_FALSE;
  arg->verbose = AIR_FALSE;

  while (1) {
    if (arg->gstate->workMutex) {
      airThreadMutexLock(arg->gstate->workMutex);
    }
    imgVi = arg->gstate->workIdx;
    if (arg->gstate->workIdx < parm->imgResV) {
      arg->gstate->workIdx += 1;
    }
    if (!(imgVi % 5)) {
      fprintf(stderr, "%s", airDoneStr(0, imgVi, parm->imgResV-1, done));
      fflush(stderr);
    }
    if (arg->gstate->workMutex) {
      airThreadMutexUnlock(arg->gstate->workMutex);
    }
    if (imgVi == parm->imgResV) {
      /* we're done! */
      break;
    }

    imgV = NRRD_POS(nrrdCenterCell, cam->vRange[0], cam->vRange[1],
                    parm->imgResV, imgVi);
    for (imgUi=0; imgUi<parm->imgResU; imgUi++) {
      imgU = NRRD_POS(nrrdCenterCell, cam->uRange[0], cam->uRange[1],
                      parm->imgResU, imgUi);
      img = ((echoCol_t *)nraw->data 
             + ECHO_IMG_CHANNELS*(imgUi + parm->imgResU*imgVi));
      
      /* initialize things on first "scanline" */
      arg->jitt = (echoPos_t *)arg->njitt->data;
      chan = arg->chanBuff;

      /* arg->verbose = ( (38 == imgUi && 121 == imgVi) ); */
      
      if (arg->verbose) {
        fprintf(stderr, "\n");
        fprintf(stderr, "-----------------------------------------------\n");
        fprintf(stderr, "----------------- (%3d, %3d) ------------------\n",
                imgUi, imgVi);
        fprintf(stderr, "-----------------------------------------------\n\n");
      }

      /* go through samples */
      for (samp=0; samp<parm->numSamples; samp++) {
        /* set ray.from[] */
        ELL_3V_COPY(ray.from, eye);
        if (parm->aperture) {
          tmp0 = parm->aperture*(arg->jitt[0 + 2*echoJittableLens]);
          tmp1 = parm->aperture*(arg->jitt[1 + 2*echoJittableLens]);
          ELL_3V_SCALE_ADD3(ray.from, 1, ray.from, tmp0, U, tmp1, V);
        }
        
        /* set at[] */
        tmp0 = imgU + pixUsz*(arg->jitt[0 + 2*echoJittablePixel]);
        tmp1 = imgV + pixVsz*(arg->jitt[1 + 2*echoJittablePixel]);
        ELL_3V_SCALE_ADD3(at, 1, imgOrig, tmp0, U, tmp1, V);

        /* do it! */
        ELL_3V_SUB(ray.dir, at, ray.from);
        ELL_3V_NORM(ray.dir, ray.dir, tmp0);
        ray.neer = 0.0;
        ray.faar = ECHO_POS_MAX;
        time0 = airTime();
        if (0) {
          memset(chan, 0, ECHO_IMG_CHANNELS*sizeof(echoCol_t));
        } else {
          echoRayColor(chan, &ray, scene, parm, arg);
        }
        chan[4] = AIR_CAST(echoCol_t, airTime() - time0);
        
        /* move to next "scanline" */
        arg->jitt += 2*ECHO_JITTABLE_NUM;
        chan += ECHO_IMG_CHANNELS;
      }
      echoChannelAverage(img, parm, arg);
      img += ECHO_IMG_CHANNELS;
      if (!parm->reuseJitter) {
        echoJitterCompute(parm, arg);
      }
    }
  }

  return _arg;
}


/*
******** echoRTRender
**
** top-level call to accomplish all (ray-tracing) rendering.  As much
** error checking as possible should be done here and not in the
** lower-level functions.
*/
int
echoRTRender(Nrrd *nraw, limnCamera *cam, echoScene *scene,
             echoRTParm *parm, echoGlobalState *gstate) {
  char me[]="echoRTRender", err[BIFF_STRLEN];
  int tid, ret;
  airArray *mop;
  echoThreadState *tstate[ECHO_THREAD_MAX];

  if (echoRTRenderCheck(nraw, cam, scene, parm, gstate)) {
    sprintf(err, "%s: problem with input", me);
    biffAdd(ECHO, err); return 1;
  }
  gstate->nraw = nraw;
  gstate->cam = cam;
  gstate->scene = scene;
  gstate->parm = parm;
  mop = airMopNew();
  if (nrrdMaybeAlloc_va(nraw, echoCol_nt, 3,
                        AIR_CAST(size_t, ECHO_IMG_CHANNELS),
                        AIR_CAST(size_t, parm->imgResU),
                        AIR_CAST(size_t, parm->imgResV))) {
    sprintf(err, "%s: couldn't allocate output image", me);
    biffMove(ECHO, err, NRRD); airMopError(mop); return 1;
  }
  airMopAdd(mop, nraw, (airMopper)nrrdNix, airMopOnError);
  nrrdAxisInfoSet_va(nraw, nrrdAxisInfoLabel,
                     "r,g,b,a,t", "x", "y");
  nrrdAxisInfoSet_va(nraw, nrrdAxisInfoMin,
                     AIR_NAN, cam->uRange[0], cam->vRange[0]);
  nrrdAxisInfoSet_va(nraw, nrrdAxisInfoMax,
                     AIR_NAN, cam->uRange[1], cam->vRange[1]);
  gstate->time = airTime();

  if (parm->numThreads > 1) {
    gstate->workMutex = airThreadMutexNew();
    airMopAdd(mop, gstate->workMutex,
              (airMopper)airThreadMutexNix, airMopAlways);
  } else {
    gstate->workMutex = NULL;
  }
  for (tid=0; tid<parm->numThreads; tid++) {
    if (!( tstate[tid] = echoThreadStateNew() )) {
      sprintf(err, "%s: failed to create thread state %d", me, tid);
      biffAdd(ECHO, err); airMopError(mop); return 1;
    }
    if (echoThreadStateInit(tid, tstate[tid], parm, gstate)) {
      sprintf(err, "%s: failed to initialized thread state %d", me, tid);
      biffAdd(ECHO, err); airMopError(mop); return 1;
    }
    airMopAdd(mop, tstate[tid], (airMopper)echoThreadStateNix, airMopAlways);
  }
  fprintf(stderr, "%s:       ", me);  /* prep for printing airDoneStr */
  gstate->workIdx = 0;
  for (tid=0; tid<parm->numThreads; tid++) {
    if (( ret = airThreadStart(tstate[tid]->thread, _echoRTRenderThreadBody,
                               (void *)(tstate[tid])) )) {
      sprintf(err, "%s: thread[%d] failed to start: %d", me, tid, ret);
      biffAdd(ECHO, err); airMopError(mop); return 1;
    }
  }
  for (tid=0; tid<parm->numThreads; tid++) {
    if (( ret = airThreadJoin(tstate[tid]->thread,
                              (void **)(&(tstate[tid]->returnPtr))) )) {
      sprintf(err, "%s: thread[%d] failed to join: %d", me, tid, ret);
      biffAdd(ECHO, err); airMopError(mop); return 1;
    }
  }

  gstate->time = airTime() - gstate->time;
  fprintf(stderr, "\n%s: time = %g\n", me, gstate->time);
  
  airMopOkay(mop);
  return 0;
}
