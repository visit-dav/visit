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

const char *
echoBiffKey = "echo";

echoRTParm *
echoRTParmNew(void) {
  echoRTParm *parm;
  
  parm = (echoRTParm *)calloc(1, sizeof(echoRTParm));
  if (parm) {
    parm->jitterType = echoJitterNone;
    parm->reuseJitter = AIR_FALSE;
    parm->permuteJitter = AIR_TRUE;
    parm->textureNN = AIR_TRUE;
    parm->numSamples = 1;
    parm->imgResU = parm->imgResV = 256;
    parm->maxRecDepth = 5;
    parm->renderLights = AIR_TRUE;
    parm->renderBoxes = AIR_FALSE;
    parm->seedRand = AIR_TRUE;
    parm->sqNRI = 15;
    parm->numThreads = 1;
    parm->sqTol = 0.0001;
    parm->aperture = 0.0;     /* pinhole camera by default */
    parm->timeGamma = 6.0;
    parm->boxOpac = 0.2f;
    parm->shadow = 1.0;
    parm->glassC = 3;
    ELL_3V_SET(parm->maxRecCol, 1.0, 0.0, 1.0);
  }
  return parm;
}

echoRTParm *
echoRTParmNix(echoRTParm *parm) {

  airFree(parm);
  return NULL;
}

echoGlobalState *
echoGlobalStateNew(void) {
  echoGlobalState *state;
  
  state = (echoGlobalState *)calloc(1, sizeof(echoGlobalState));
  if (state) {
    state->verbose = 0;
    state->time = 0;
    state->nraw = NULL;
    state->cam = NULL;
    state->scene = NULL;
    state->parm = NULL;
    state->workIdx = 0;
    state->workMutex = NULL;
  }
  return state;
}

echoGlobalState *
echoGlobalStateNix(echoGlobalState *state) {

  airFree(state);
  /* mutex freed at end of echoRTRender() */
  return NULL;
}

echoThreadState *
echoThreadStateNew(void) {
  echoThreadState *state;
  
  state = (echoThreadState *)calloc(1, sizeof(echoThreadState));
  if (state) {
    state->thread = airThreadNew();
    state->verbose = 0;
    state->threadIdx = -1;
    state->depth = -1;
    state->njitt = nrrdNew();
    state->nperm = nrrdNew();
    state->permBuff = NULL;
    state->jitt = NULL;
    state->chanBuff = NULL;
    state->rst = airRandMTStateNew(0);
    state->returnPtr = NULL;
  }
  return state;
}

echoThreadState *
echoThreadStateNix(echoThreadState *state) {

  if (state) {
    state->thread = airThreadNix(state->thread);
    nrrdNuke(state->njitt);
    nrrdNuke(state->nperm);
    state->permBuff = AIR_CAST(unsigned int *, airFree(state->permBuff));
    state->chanBuff = AIR_CAST(echoCol_t *, airFree(state->chanBuff));
    airFree(state);
  }
  return NULL;
}

echoScene *
echoSceneNew(void) {
  echoScene *ret;
  
  ret = (echoScene *)calloc(1, sizeof(echoScene));
  if (ret) {
    ret->cat = NULL;
    ret->catArr = airArrayNew((void**)&(ret->cat), NULL,
                              sizeof(echoObject *),
                              ECHO_LIST_OBJECT_INCR);
    airArrayPointerCB(ret->catArr,
                      airNull,
                      (void *(*)(void *))echoObjectNix);
    ret->rend = NULL;
    ret->rendArr = airArrayNew((void**)&(ret->rend), NULL,
                               sizeof(echoObject *),
                               ECHO_LIST_OBJECT_INCR);
    /* no callbacks set, renderable objecs are nixed from catArr */
    ret->light = NULL;
    ret->lightArr = airArrayNew((void**)&(ret->light), NULL,
                                sizeof(echoObject *),
                                ECHO_LIST_OBJECT_INCR);
    /* no callbacks set; light objects are nixed from catArr */
    ret->nrrd = NULL;
    ret->nrrdArr = airArrayNew((void**)&(ret->nrrd), NULL,
                               sizeof(Nrrd *),
                               ECHO_LIST_OBJECT_INCR);
    airArrayPointerCB(ret->nrrdArr,
                      airNull,
                      (void *(*)(void *))nrrdNuke);
    ret->envmap = NULL;
    ELL_3V_SET(ret->ambi, 1.0, 1.0, 1.0);
    ELL_3V_SET(ret->bkgr, 0.0, 0.0, 0.0);
  }
  return ret;
}

void
_echoSceneLightAdd(echoScene *scene, echoObject *obj) {
  unsigned int idx;
  
  for (idx=0; idx<scene->lightArr->len; idx++) {
    if (obj == scene->light[idx]) {
      break;
    }
  }
  if (scene->lightArr->len == idx) {
    idx = airArrayLenIncr(scene->lightArr, 1);
    scene->light[idx] = obj;
  }
}

void
_echoSceneNrrdAdd(echoScene *scene, Nrrd *nrrd) {
  unsigned int idx;
  
  for (idx=0; idx<scene->nrrdArr->len; idx++) {
    if (nrrd == scene->nrrd[idx]) {
      break;
    }
    if (scene->nrrdArr->len == idx) {
      idx = airArrayLenIncr(scene->nrrdArr, 1);
      scene->nrrd[idx] = nrrd;
    }
  }
}

echoScene *
echoSceneNix(echoScene *scene) {
  
  if (scene) {
    airArrayNuke(scene->catArr);
    airArrayNuke(scene->rendArr);
    airArrayNuke(scene->lightArr);
    airArrayNuke(scene->nrrdArr);
    /* don't touch envmap nrrd */
    airFree(scene);
  }
  return NULL;
}
