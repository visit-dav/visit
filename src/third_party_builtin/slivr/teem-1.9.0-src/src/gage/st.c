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

#define GAGE_CACHE_LEN 1013

unsigned int
_gageHash(int x, int y, int z) {
  unsigned int h, g;
  unsigned char s[6];
  int i;

  s[0] = x & 0xff;
  s[1] = (x >> 8) & 0xff;
  s[2] = y & 0xff;
  s[3] = (y >> 8) & 0xff;
  s[4] = z & 0xff;
  s[5] = (z >> 8) & 0xff;
  
  h = 0;
  for (i=0; i<=5; i++) {
    h = (h << 4) + s[i];
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }
  return h % GAGE_CACHE_LEN; 
}

void
_gageCacheProbe(gageContext *ctx, gage_t *grad,
                int *cc, gage_t *gc, 
                int x, int y, int z) {
  int hi;

  hi = _gageHash(x, y, z);
  if ( (cc[3*hi + 0] == x) &&
       (cc[3*hi + 1] == y) &&
       (cc[3*hi + 2] == z) ) {
    /* cache hit */
    ELL_3V_COPY(grad, gc + 3*hi);
  } else {
    /* cache miss */
    cc[3*hi + 0] = x;
    cc[3*hi + 1] = y;
    cc[3*hi + 2] = z;
    gageProbe(ctx, AIR_CAST(gage_t, x), AIR_CAST(gage_t, y),
              AIR_CAST(gage_t, z));
    ELL_3V_COPY(gc + 3*hi, grad);
  }
  return ;
}

/*
******** gageStructureTensor()
**
** Computes volume of structure tensors.  Currently, only works on a scalar
** fields (for multi-scalar fields, just add structure tensors from each
** component scalar), and only uses the B-spline kernel for differentiation
** and derivative blurring.  
**
** Note, if you want to use dsmp > 1, its your responsibility to give
** an appropriate iScale > 1, so that you don't undersample.
*/
int
gageStructureTensor(Nrrd *nout, const Nrrd *nin,
                    int dScale, int iScale, int dsmp) {
  char me[]="gageStructureTensor", err[BIFF_STRLEN];
  NrrdKernelSpec *gk0, *gk1, *ik0;
  int E, rad, diam, osx, osy, osz, oxi, oyi, ozi,
    _ixi, _iyi, _izi, ixi, iyi, izi, wi, *coordCache;
  gageContext *ctx;
  gageQuery query;
  gagePerVolume *pvl;
  airArray *mop;
  gage_t *grad, *ixw, *iyw, *izw, wght, sten[6], *gradCache, *out;
  double xs, ys, zs, ms;

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, me); return 1;
  }
  if (!( 3 == nin->dim && nrrdTypeBlock != nin->type )) {
    sprintf(err, "%s: nin isn't a 3D non-block type nrrd", me);
    biffAdd(GAGE, me); return 1;
  }
  /*
  if (!( AIR_EXISTS(nin->axis[0].spacing) 
         && AIR_EXISTS(nin->axis[1].spacing) 
         && AIR_EXISTS(nin->axis[2].spacing) )) {
    sprintf(err, "%s: nin's axis 0,1,2 spacings don't all exist", me);
    biffAdd(GAGE, me); return 1;
  }
  */
  if (!( dScale >= 1 && iScale >= 1 && dsmp >= 1 )) {
    sprintf(err, "%s: dScale (%d), iScale (%d), dsmp (%d) not all >= 1", 
            me, dScale, iScale, dsmp);
    biffAdd(GAGE, me); return 1;
  }

  mop = airMopNew();
  gk0 = nrrdKernelSpecNew();
  gk1 = nrrdKernelSpecNew();
  ik0 = nrrdKernelSpecNew();
  airMopAdd(mop, gk0, (airMopper)nrrdKernelSpecNix, airMopAlways);
  airMopAdd(mop, gk1, (airMopper)nrrdKernelSpecNix, airMopAlways);
  airMopAdd(mop, ik0, (airMopper)nrrdKernelSpecNix, airMopAlways);
  if ( nrrdKernelSpecParse(gk0, "cubic:1,0")
       || nrrdKernelSpecParse(gk1, "cubicd:1,0")
       || nrrdKernelSpecParse(ik0, "cubic:1,0")) {
    sprintf(err, "%s: couldn't set up kernels", me);
    biffMove(GAGE, me, NRRD); airMopError(mop); return 1;
  }
  /* manually set scale parameters */
  gk0->parm[0] = dScale;
  gk1->parm[0] = dScale;
  ik0->parm[0] = 1.0;       /* this is more complicated ... */
  ctx = gageContextNew();
  airMopAdd(mop, ctx, (airMopper)gageContextNix, airMopAlways);
  gageParmSet(ctx, gageParmRenormalize, AIR_TRUE);
  E = 0;
  if (!E) E |= !(pvl = gagePerVolumeNew(ctx, nin, gageKindScl));
  if (!E) E |= gagePerVolumeAttach(ctx, pvl);
  if (!E) E |= gageKernelSet(ctx, gageKernel00, gk0->kernel, gk0->parm);
  if (!E) E |= gageKernelSet(ctx, gageKernel11, gk1->kernel, gk1->parm);
  if (!E) GAGE_QUERY_RESET(query);
  if (!E) GAGE_QUERY_ITEM_ON(query, gageSclGradVec);
  if (!E) E |= gageQuerySet(ctx, pvl, query);
  if (!E) E |= gageUpdate(ctx);
  if (E) {
    sprintf(err, "%s: ", me);
    biffAdd(GAGE, err); airMopError(mop); return 1;
  }
  grad = _gageAnswerPointer(ctx, pvl, gageSclGradVec);
  
  xs = nin->axis[0].spacing;
  ys = nin->axis[1].spacing;
  zs = nin->axis[2].spacing;
  xs = AIR_EXISTS(xs) ? AIR_ABS(xs) : 1.0;
  ys = AIR_EXISTS(ys) ? AIR_ABS(ys) : 1.0;
  zs = AIR_EXISTS(zs) ? AIR_ABS(zs) : 1.0;
  ms = airCbrt(xs*ys*zs);
  /* ms is geometric mean of {xs,ys,zs} */
  xs /= ms;
  ys /= ms;
  zs /= ms;
  fprintf(stderr, "iScale = %d, xs, ys, zs = %g, %g, %g\n",
          iScale, xs, ys, xs);

  rad = 0;
  ik0->parm[0] = iScale/xs;
  rad = AIR_MAX(rad, AIR_ROUNDUP(ik0->kernel->support(ik0->parm)));
  ik0->parm[0] = iScale/ys;
  rad = AIR_MAX(rad, AIR_ROUNDUP(ik0->kernel->support(ik0->parm)));
  ik0->parm[0] = iScale/zs;
  rad = AIR_MAX(rad, AIR_ROUNDUP(ik0->kernel->support(ik0->parm)));
  diam = 2*rad + 1;
  ixw = (gage_t*)calloc(diam, sizeof(gage_t));
  iyw = (gage_t*)calloc(diam, sizeof(gage_t));
  izw = (gage_t*)calloc(diam, sizeof(gage_t));
  if (!(ixw && iyw && izw)) {
    sprintf(err, "%s: couldn't allocate grad vector or weight buffers", me);
    biffAdd(GAGE, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, ixw, airFree, airMopAlways);
  airMopAdd(mop, iyw, airFree, airMopAlways);
  airMopAdd(mop, izw, airFree, airMopAlways);

  /* the only reason that it is thread-safe to cache gageProbe results,
     without having the cache hang directly off the gageContext, is that
     we're doing all the probing for one context in one shot- producing
     an entirely volume of structure tensors with one function call */
  gradCache = (gage_t*)calloc(3*GAGE_CACHE_LEN, sizeof(gage_t));
  coordCache = (int*)calloc(3*GAGE_CACHE_LEN, sizeof(int));
  if (!(gradCache && coordCache)) {
    sprintf(err, "%s: couldn't allocate caches", me);
    biffAdd(GAGE, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, gradCache, airFree, airMopAlways);
  airMopAdd(mop, coordCache, airFree, airMopAlways);
  for (ixi=0; ixi<GAGE_CACHE_LEN; ixi++) {
    coordCache[3*ixi + 0] = -1;
    coordCache[3*ixi + 1] = -1;
    coordCache[3*ixi + 2] = -1;
  }

  for (wi=-rad; wi<=rad; wi++) {
    ik0->parm[0] = iScale/xs;
    ixw[wi+rad] = AIR_CAST(gage_t, ik0->kernel->eval1_d(wi, ik0->parm));
    ik0->parm[0] = iScale/ys;
    iyw[wi+rad] = AIR_CAST(gage_t, ik0->kernel->eval1_d(wi, ik0->parm));
    ik0->parm[0] = iScale/zs;
    izw[wi+rad] = AIR_CAST(gage_t, ik0->kernel->eval1_d(wi, ik0->parm));
    fprintf(stderr, "%d --> (%g,%g,%g) -> (%g,%g,%g)\n",
            wi, wi/xs, wi/ys, wi/zs, ixw[wi+rad], iyw[wi+rad], izw[wi+rad]);
  }

  osx = (nin->axis[0].size)/dsmp;
  osy = (nin->axis[1].size)/dsmp;
  osz = (nin->axis[2].size)/dsmp;
  if (nrrdMaybeAlloc_va(nout, gage_nrrdType, 4,
                        AIR_CAST(size_t, 7),
                        AIR_CAST(size_t, osx),
                        AIR_CAST(size_t, osy),
                        AIR_CAST(size_t, osz))) {
    sprintf(err, "%s: couldn't allocate output", me);
    biffMove(GAGE, err, NRRD); airMopError(mop); return 1;
  }
  airMopAdd(mop, nout, (airMopper)nrrdEmpty, airMopOnError);

  out = (gage_t *)nout->data;
  for (ozi=0; ozi<osz; ozi++) {
    fprintf(stderr, "%s: z = %d/%d\n", me, ozi+1, osz);
    for (oyi=0; oyi<osy; oyi++) {
      for (oxi=0; oxi<osx; oxi++) {

        sten[0] = sten[1] = sten[2] = sten[3] = sten[4] = sten[5] = 0;
        for (_izi=0; _izi<diam; _izi++) {
          izi = AIR_CLAMP(0, _izi - rad + ozi*dsmp,
                          (int)nin->axis[2].size-1);
          if (!izw[_izi]) continue;
          for (_iyi=0; _iyi<diam; _iyi++) {
            iyi = AIR_CLAMP(0, _iyi - rad + oyi*dsmp,
                            (int)nin->axis[1].size-1);
            if (!iyw[_iyi]) continue;
            for (_ixi=0; _ixi<diam; _ixi++) {
              ixi = AIR_CLAMP(0, _ixi - rad + oxi*dsmp,
                              (int)nin->axis[0].size-1);
              if (!ixw[_ixi]) continue;
              wght = ixw[_ixi]*iyw[_iyi]*izw[_izi];
              _gageCacheProbe(ctx, grad, coordCache, gradCache, ixi, iyi, izi);
              sten[0] += wght*grad[0]*grad[0];
              sten[1] += wght*grad[0]*grad[1];
              sten[2] += wght*grad[0]*grad[2];
              sten[3] += wght*grad[1]*grad[1];
              sten[4] += wght*grad[1]*grad[2];
              sten[5] += wght*grad[2]*grad[2];
            }
          }
        }
        out[0] = 1.0;
        out[1] = sten[0];
        out[2] = sten[1];
        out[3] = sten[2];
        out[4] = sten[3];
        out[5] = sten[4];
        out[6] = sten[5]; 
        out += 7;
        
      }
    }
  }

  airMopOkay(mop);
  return 0;
}
