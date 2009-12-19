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

#include "gage.h"
#include "privateGage.h"

int
_gagePvlFlagCheck(gageContext *ctx, int pvlFlag) {
  int ret;
  unsigned int pvlIdx;
  
  ret = AIR_FALSE;
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    ret |= ctx->pvl[pvlIdx]->flag[pvlFlag];
  }
  return ret;
}

void
_gagePvlFlagDown(gageContext *ctx, int pvlFlag) {
  unsigned int pvlIdx;
  
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    ctx->pvl[pvlIdx]->flag[pvlFlag] = AIR_FALSE;
  }
}

/* 
** One could go from all the pvls' queries to the context's needD in
** one shot, but doing it in two steps (as below) seems a little clearer,
** and it means that pvl->needD isn't needlessly re-computed for 
** pvl's whose query hasn't changed.
*/

/*
** for each pvl: pvl's query --> pvl's needD
*/
void
_gagePvlNeedDUpdate(gageContext *ctx) {
  char me[]="_gagePvlNeedDUpdate";
  gagePerVolume *pvl;
  int que, needD[3];
  unsigned int pvlIdx;

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    pvl = ctx->pvl[pvlIdx];
    if (pvl->flag[gagePvlFlagQuery]) {
      ELL_3V_SET(needD, 0, 0, 0);
      que = pvl->kind->itemMax+1;
      do {
        que--;
        if (GAGE_QUERY_ITEM_TEST(pvl->query, que)) {
          needD[pvl->kind->table[que].needDeriv] = 1;
        }
      } while (que);
      if (!ELL_3V_EQUAL(needD, pvl->needD)) {
        if (ctx->verbose) {
          fprintf(stderr, "%s: updating pvl[%d]'s needD to (%d,%d,%d)\n",
                  me, pvlIdx, needD[0], needD[1], needD[2]);
        }
        ELL_3V_COPY(pvl->needD, needD);
        pvl->flag[gagePvlFlagNeedD] = AIR_TRUE;
      }
    }
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);

  return;
}

/*
** all pvls' needD --> ctx's needD
*/
void
_gageNeedDUpdate(gageContext *ctx) {
  char me[]="_gageNeedDUpdate";
  gagePerVolume *pvl;
  int needD[3];
  unsigned int pvlIdx;

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  ELL_3V_SET(needD, 0, 0, 0);
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    pvl = ctx->pvl[pvlIdx];
    needD[0] |= pvl->needD[0];
    needD[1] |= pvl->needD[1];
    needD[2] |= pvl->needD[2];
  }
  if (!ELL_3V_EQUAL(needD, ctx->needD)) {
    if (ctx->verbose) {
      fprintf(stderr, "%s: updating ctx's needD to (%d,%d,%d)\n",
              me, needD[0], needD[1], needD[2]);
    }
    ELL_3V_COPY(ctx->needD, needD);
    ctx->flag[gageCtxFlagNeedD] = AIR_TRUE;
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);

  return;
}

/*
** ctx's needD & k3pack --> needK
*/
void
_gageNeedKUpdate(gageContext *ctx) {
  char me[]="_gageNeedKUpdate";
  int kernIdx, needK[GAGE_KERNEL_NUM], change;
  
  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  for (kernIdx=0; kernIdx<GAGE_KERNEL_NUM; kernIdx++) {
    needK[kernIdx] = AIR_FALSE;
  }
  if (ctx->needD[0]) {
    needK[gageKernel00] = AIR_TRUE;
  }
  if (ctx->needD[1]) {
    needK[gageKernel11] = AIR_TRUE;
    if (ctx->parm.k3pack) {
      needK[gageKernel00] = AIR_TRUE;
    } else {
      needK[gageKernel10] = AIR_TRUE;
    }  
  }
  if (ctx->needD[2]) {
    needK[gageKernel22] = AIR_TRUE;
    if (ctx->parm.k3pack) {
      needK[gageKernel00] = AIR_TRUE;
      needK[gageKernel11] = AIR_TRUE;
    } else {
      needK[gageKernel20] = AIR_TRUE;
      needK[gageKernel21] = AIR_TRUE;
    }  
  }
  change = AIR_FALSE;
  for (kernIdx=0; kernIdx<GAGE_KERNEL_NUM; kernIdx++) {
    change |= (needK[kernIdx] != ctx->needK[kernIdx]);
  }
  if (change) {
    if (ctx->verbose) {
      fprintf(stderr, "%s: changing needK to (%d,%d,%d,%d,%d,%d)\n",
              me, needK[0], needK[1], needK[2], needK[3], needK[4], needK[5]);
    }
    for (kernIdx=0; kernIdx<GAGE_KERNEL_NUM; kernIdx++) {
      ctx->needK[kernIdx] = needK[kernIdx];
    }
    ctx->flag[gageCtxFlagNeedK] = AIR_TRUE;
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);

  return;
}

/*
** ctx's ksp[] & needK --> radius
**
*/
int
_gageRadiusUpdate(gageContext *ctx) {
  char me[]="_gageRadiusUpdate", err[BIFF_STRLEN];
  int kernIdx, radius;
  double maxRad, rad;
  NrrdKernelSpec *ksp;

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  maxRad = 0;
  for (kernIdx=0; kernIdx<GAGE_KERNEL_NUM; kernIdx++) {
    if (ctx->needK[kernIdx]) {
      ksp = ctx->ksp[kernIdx];
      if (!ksp) {
        sprintf(err, "%s: need kernel %s but it hasn't been set", 
                me, airEnumStr(gageKernel, kernIdx));
        biffAdd(GAGE, err); return 1;
      }
      rad = ksp->kernel->support(ksp->parm);
      maxRad = AIR_MAX(maxRad, rad);
      if (ctx->verbose) {
        fprintf(stderr, "%s: k[%s]=%s -> rad = %g -> maxRad = %g\n", me,
                airEnumStr(gageKernel, kernIdx), ksp->kernel->name,
                rad, maxRad);
      }
    }
  }
  radius = AIR_ROUNDUP(maxRad);
  /* In case either kernels have tiny supports (less than 0.5), or if
     we in fact don't need any kernels, then we need to do this to 
     ensure that we generate a valid (trivial) padding */
  radius = AIR_MAX(radius, 1);
  if (radius != ctx->radius) {
    if (ctx->verbose) {
      fprintf(stderr, "%s: changing radius from %d to %d\n",
              me, ctx->radius, radius);
    }
    ctx->radius = radius;
    ctx->flag[gageCtxFlagRadius] = AIR_TRUE;
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);

  return 0;
}

int
_gageCacheSizeUpdate(gageContext *ctx) {
  char me[]="_gageCacheSizeUpdate", err[BIFF_STRLEN];
  int fd;
  gagePerVolume *pvl;
  unsigned int pvlIdx;

  if (ctx->verbose) fprintf(stderr, "%s: hello (radius = %d)\n", me,
                            ctx->radius);
  if (!( ctx->radius > 0 )) {
    sprintf(err, "%s: have bad radius %d", me, ctx->radius);
    biffAdd(GAGE, err); return 1;
  }
  fd = 2*ctx->radius;
  ctx->fsl = (gage_t *)airFree(ctx->fsl);
  ctx->fw = (gage_t *)airFree(ctx->fw);
  ctx->off = (unsigned int *)airFree(ctx->off);
  ctx->fsl = (gage_t *)calloc(fd*3, sizeof(gage_t));
  ctx->fw = (gage_t *)calloc(fd*3*GAGE_KERNEL_NUM, sizeof(gage_t));
  ctx->off = (unsigned int *)calloc(fd*fd*fd, sizeof(unsigned int));
  if (!(ctx->fsl && ctx->fw && ctx->off)) {
    sprintf(err, "%s: couldn't allocate filter caches for fd=%d", me, fd);
    biffAdd(GAGE, err); return 1;
  }
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    pvl = ctx->pvl[pvlIdx];
    pvl->iv3 = (gage_t *)airFree(pvl->iv3);
    pvl->iv2 = (gage_t *)airFree(pvl->iv2);
    pvl->iv1 = (gage_t *)airFree(pvl->iv1);
    pvl->iv3 = (gage_t *)calloc(fd*fd*fd*pvl->kind->valLen, sizeof(gage_t));
    pvl->iv2 = (gage_t *)calloc(fd*fd*pvl->kind->valLen, sizeof(gage_t));
    pvl->iv1 = (gage_t *)calloc(fd*pvl->kind->valLen, sizeof(gage_t));
    if (!(pvl->iv3 && pvl->iv2 && pvl->iv1)) {
      sprintf(err, "%s: couldn't allocate pvl[%d]'s value caches for fd=%d",
              me, pvlIdx, fd);
      biffAdd(GAGE, err); return 1;
    }
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);
  
  return 0;
}

void
_gageOffValueUpdate(gageContext *ctx) {
  char me[]="_gageOffValueUpdate";
  int fd, i, j, k;
  unsigned int sx, sy;

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);

  sx = ctx->shape->size[0];
  sy = ctx->shape->size[1];
  fd = 2*ctx->radius;
  /* HEY: look into special casing this for small fd */
  for (k=0; k<fd; k++) {
    for (j=0; j<fd; j++) {
      for (i=0; i<fd; i++) {
        ctx->off[i + fd*(j + fd*k)] = i + sx*(j + sy*k);
      }
    }
  }
  /* no flags to set for further action */
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);
  
  return;
}

/*
******** gageUpdate()
**
** call just before probing begins.
*/
int
gageUpdate(gageContext *ctx) {
  char me[]="gageUpdate", err[BIFF_STRLEN];
  int i;

  if (!( ctx )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (0 == ctx->pvlNum) {
    sprintf(err, "%s: context has no attached pervolumes", me);
    biffAdd(GAGE, err); return 1;
  }

  /* start traversing the whole update graph ... */
  if (ctx->verbose) {
    fprintf(stderr, "%s: hello ____________________ \n", me);
    fprintf(stderr, "    context flags:");
    for (i=gageCtxFlagUnknown+1; i<gageCtxFlagLast; i++) {
      fprintf(stderr, " %d=%d", i, ctx->flag[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "    pvl flags:");
    for (i=gagePvlFlagUnknown+1; i<gagePvlFlagLast; i++) {
      fprintf(stderr, " %d=%d", i, _gagePvlFlagCheck(ctx, i));
    }
    fprintf(stderr, "\n");
  }
  if (_gagePvlFlagCheck(ctx, gagePvlFlagQuery)) {
    _gagePvlNeedDUpdate(ctx);
    _gagePvlFlagDown(ctx, gagePvlFlagQuery);
  }
  if (_gagePvlFlagCheck(ctx, gagePvlFlagNeedD)) {
    _gageNeedDUpdate(ctx);
    _gagePvlFlagDown(ctx, gagePvlFlagNeedD);
  }
  if (ctx->flag[gageCtxFlagNeedD] || ctx->flag[gageCtxFlagK3Pack]) {
    _gageNeedKUpdate(ctx);
    ctx->flag[gageCtxFlagNeedD] = AIR_FALSE;
    ctx->flag[gageCtxFlagK3Pack] = AIR_FALSE;
  }
  if (ctx->flag[gageCtxFlagKernel] || ctx->flag[gageCtxFlagNeedK]) {
    if (_gageRadiusUpdate(ctx)) {
      sprintf(err, "%s: trouble", me); biffAdd(GAGE, err); return 1;
    }
    ctx->flag[gageCtxFlagKernel] = AIR_FALSE;
    ctx->flag[gageCtxFlagNeedK] = AIR_FALSE;
  }
  if (ctx->flag[gageCtxFlagRadius]
      /* HEY HEY HEY: this is a total hack: right now its possible for a 
         new pvl to have unallocated iv3,iv2,iv1, if it was attached to a
         context which had already been probing, as was the case with
         _tenRegisterDoit.  So, with this hack we reallocate ALL caches
         just because a new pervolume was attached ... */
      || _gagePvlFlagCheck(ctx, gagePvlFlagVolume)) {
    if (_gageCacheSizeUpdate(ctx)) {
      sprintf(err, "%s: trouble", me); biffAdd(GAGE, err); return 1;
    }
  }
  if (ctx->flag[gageCtxFlagRadius]
      || ctx->flag[gageCtxFlagShape]
      /* see above; following flags that triggered _gageCacheSizeUpdate(ctx) */
      || _gagePvlFlagCheck(ctx, gagePvlFlagVolume)) {
    _gageOffValueUpdate(ctx);
    ctx->flag[gageCtxFlagShape] = AIR_FALSE;
  }
  ctx->flag[gageCtxFlagRadius] = AIR_FALSE;
    
  /* chances are, something above has invalidated the state maintained
     during successive calls to gageProbe() */
  gagePointReset(&ctx->point);
  if (ctx->verbose) fprintf(stderr, "%s: bye ^^^^^^^^^^^^^^^^^^^ \n", me);

  return 0;
}
