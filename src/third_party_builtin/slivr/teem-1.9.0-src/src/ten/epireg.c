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

#include "ten.h"
#include "privateTen.h"

int
_tenEpiRegSave(char *fname, Nrrd *nsingle, Nrrd **nmulti,
               int len, char *desc) {
  char me[]="_tenEpiRegSave", err[BIFF_STRLEN];
  Nrrd *nout;
  airArray *mop;

  mop = airMopNew();
  if (nsingle) {
    nout = nsingle;
  } else {
    airMopAdd(mop, nout=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    if (nrrdJoin(nout, (const Nrrd**)nmulti, len, 0, AIR_TRUE)) {
      sprintf(err, "%s: couldn't join %s for output", me, desc);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
  }
  if (nrrdSave(fname, nout, NULL)) {
    sprintf(err, "%s: trouble saving %s to \"%s\"", me, desc, fname);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  fprintf(stderr, "%s: saved %s to \"%s\"\n", me, desc, fname);
  airMopOkay(mop); 
  return 0;
}


int
_tenEpiRegCheck(Nrrd **nout, Nrrd **ndwi, unsigned int dwiLen, Nrrd *ngrad,
                int reference,
                double bwX, double bwY, double DWthr,
                const NrrdKernel *kern, double *kparm) {
  char me[]="_tenEpiRegCheck", err[BIFF_STRLEN];
  unsigned int ni;

  AIR_UNUSED(DWthr);
  if (!( nout && ndwi && ngrad && kern && kparm )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenGradientCheck(ngrad, nrrdTypeDefault, 6 /* <-- HEY: not sure */)) {
    sprintf(err, "%s: problem with given gradient list", me);
    biffAdd(TEN, err); return 1;
  }
  if (dwiLen != ngrad->axis[1].size) {
    sprintf(err, "%s: got %u DWIs, but " _AIR_SIZE_T_CNV 
            " gradient directions", me,
            dwiLen, ngrad->axis[1].size);
    biffAdd(TEN, err); return 1;
  }
  for (ni=0; ni<dwiLen; ni++) {
    if (!nout[ni]) {
      sprintf(err, "%s: nout[%d] is NULL", me, ni);
      biffAdd(TEN, err); return 1;
    }
    if (nrrdCheck(ndwi[ni])) {
      sprintf(err, "%s: basic nrrd validity failed on ndwi[%d]", me, ni);
      biffMove(TEN, err, NRRD); return 1;
    }
    if (!nrrdSameSize(ndwi[0], ndwi[ni], AIR_TRUE)) {
      sprintf(err, "%s: ndwi[%d] is different from ndwi[0]", me, ni);
      biffMove(TEN, err, NRRD); return 1;
    }
  }
  if (!( 3 == ndwi[0]->dim )) {
    sprintf(err, "%s: didn't get a set of 3-D arrays (got %d-D)", me,
            ndwi[0]->dim);
    biffAdd(TEN, err); return 1;
  }
  if (!( AIR_IN_CL(-1, reference, (int)dwiLen-1) )) {
    sprintf(err, "%s: reference index %d not in valid range [-1,%d]", 
            me, reference, dwiLen-1);
    biffAdd(TEN, err); return 1;
  }
  if (!( AIR_EXISTS(bwX) && AIR_EXISTS(bwY) )) {
    sprintf(err, "%s: bwX, bwY don't both exist", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( bwX >= 0 && bwY >= 0 )) {
    sprintf(err, "%s: bwX (%g) and bwY (%g) are not both non-negative",
            me, bwX, bwY);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

/*
** this assumes that all nblur[i] are valid nrrds, and does nothing
** to manage them
*/
int
_tenEpiRegBlur(Nrrd **nblur, Nrrd **ndwi, unsigned int dwiLen,
               double bwX, double bwY, int verb) {
  char me[]="_tenEpiRegBlur", err[BIFF_STRLEN];
  NrrdResampleInfo *rinfo;
  airArray *mop;
  size_t ni, sx, sy, sz;
  double savemin[2], savemax[2];

  if (!( bwX || bwY )) {
    if (verb) {
      fprintf(stderr, "%s:\n            ", me); fflush(stderr);
    }
    for (ni=0; ni<dwiLen; ni++) {
      if (verb) {
        fprintf(stderr, "%2u ", (unsigned int)ni); fflush(stderr);
      }
      if (nrrdCopy(nblur[ni], ndwi[ni])) {
        sprintf(err, "%s: trouble copying ndwi[%u]", me, (unsigned int)ni);
        biffMove(TEN, err, NRRD); return 1;
      }
    }
    if (verb) {
      fprintf(stderr, "done\n");
    }
    return 0;
  }
  /* else we need to blur */
  sx = ndwi[0]->axis[0].size;
  sy = ndwi[0]->axis[1].size;
  sz = ndwi[0]->axis[2].size;
  mop = airMopNew();
  rinfo = nrrdResampleInfoNew();
  airMopAdd(mop, rinfo, (airMopper)nrrdResampleInfoNix, airMopAlways);
  if (bwX) {
    rinfo->kernel[0] = nrrdKernelGaussian;
    rinfo->parm[0][0] = bwX;
    rinfo->parm[0][1] = 3.0; /* how many stnd devs do we cut-off at */
  } else {
    rinfo->kernel[0] = NULL;
  }
  if (bwY) {
    rinfo->kernel[1] = nrrdKernelGaussian;
    rinfo->parm[1][0] = bwY;
    rinfo->parm[1][1] = 3.0; /* how many stnd devs do we cut-off at */
  } else {
    rinfo->kernel[1] = NULL;
  }
  rinfo->kernel[2] = NULL;
  ELL_3V_SET(rinfo->samples, sx, sy, sz);
  ELL_3V_SET(rinfo->min, 0, 0, 0);
  ELL_3V_SET(rinfo->max, sx-1, sy-1, sz-1);
  rinfo->boundary = nrrdBoundaryBleed;
  rinfo->type = nrrdTypeDefault;
  rinfo->renormalize = AIR_TRUE;
  rinfo->clamp = AIR_TRUE;
  if (verb) {
    fprintf(stderr, "%s:\n            ", me); fflush(stderr);
  }
  for (ni=0; ni<dwiLen; ni++) {
    if (verb) {
      fprintf(stderr, "%2u ", (unsigned int)ni); fflush(stderr);
    }
    savemin[0] = ndwi[ni]->axis[0].min; savemax[0] = ndwi[ni]->axis[0].max; 
    savemin[1] = ndwi[ni]->axis[1].min; savemax[1] = ndwi[ni]->axis[1].max;
    ndwi[ni]->axis[0].min = 0; ndwi[ni]->axis[0].max = sx-1;
    ndwi[ni]->axis[1].min = 0; ndwi[ni]->axis[1].max = sy-1;
    if (nrrdSpatialResample(nblur[ni], ndwi[ni], rinfo)) {
      sprintf(err, "%s: trouble blurring ndwi[%u]", me, (unsigned int)ni);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
    ndwi[ni]->axis[0].min = savemin[0]; ndwi[ni]->axis[0].max = savemax[0]; 
    ndwi[ni]->axis[1].min = savemin[1]; ndwi[ni]->axis[1].max = savemax[1];
  }
  if (verb) {
    fprintf(stderr, "done\n");
  }
  airMopOkay(mop);
  return 0;
}

int
_tenEpiRegFindThresh(double *DWthrP, Nrrd **nin, int ninLen, int save) {
  char me[]="_tenEpiRegFindThresh", err[BIFF_STRLEN];
  Nrrd *nhist, *ntmp;
  airArray *mop;
  int ni, bins, E;
  double min=0, max=0;
  NrrdRange *range;

  mop = airMopNew();
  airMopAdd(mop, nhist=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntmp=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);

  for (ni=0; ni<ninLen; ni++) {
    range = nrrdRangeNewSet(nin[ni], nrrdBlind8BitRangeFalse);
    if (!ni) {
      min = range->min;
      max = range->max;
    } else {
      min = AIR_MIN(min, range->min);
      max = AIR_MAX(max, range->max);
    }
    range = nrrdRangeNix(range);
  }
  bins = AIR_MIN(1024, (int)(max - min + 1));
  ntmp->axis[0].min = min;
  ntmp->axis[0].max = max;
  for (ni=0; ni<ninLen; ni++) {
    if (nrrdHisto(ntmp, nin[ni], NULL, NULL, bins, nrrdTypeFloat)) {
      sprintf(err, "%s: problem forming histogram of DWI %d", me, ni);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
    if (!ni) {
      E = nrrdCopy(nhist, ntmp);
    } else {
      E = nrrdArithBinaryOp(nhist, nrrdBinaryOpAdd, nhist, ntmp);
    }
    if (E) {
      sprintf(err, "%s: problem updating histogram sum on DWI %d", me, ni);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
  }
  if (save) {
    nrrdSave("regtmp-dwihist.nrrd", nhist, NULL);
  }
  /*
  if (_tenFindValley(DWthrP, nhist, 0.65, save)) {
    sprintf(err, "%s: problem finding DWI histogram valley", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  */
  if (nrrdHistoThresholdOtsu(DWthrP, nhist)) {
    sprintf(err, "%s: problem finding DWI threshold", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}

int
_tenEpiRegThreshold(Nrrd **nthresh, Nrrd **nblur, unsigned int ninLen,
                    double DWthr, int verb, int progress) {
  char me[]="_tenEpiRegThreshold", err[BIFF_STRLEN];
  airArray *mop;
  size_t I, sx, sy, sz, ni;
  float val;
  unsigned char *thr;

  if (!( AIR_EXISTS(DWthr) )) {
    if (_tenEpiRegFindThresh(&DWthr, nblur, ninLen, progress)) {
      sprintf(err, "%s: trouble with automatic threshold determination", me);
      biffAdd(TEN, err); return 1;
    }
    fprintf(stderr, "%s: using %g for DWI threshold\n", me, DWthr);
  }
  
  mop = airMopNew();
  if (verb) {
    fprintf(stderr, "%s:\n            ", me); fflush(stderr);
  }
  sx = nblur[0]->axis[0].size;
  sy = nblur[0]->axis[1].size;
  sz = nblur[0]->axis[2].size;
  for (ni=0; ni<ninLen; ni++) {
    if (verb) {
      fprintf(stderr, "%2u ", (unsigned int)ni); fflush(stderr);
    }
    if (nrrdMaybeAlloc_va(nthresh[ni], nrrdTypeUChar, 3,
                          sx, sy, sz)) {
      sprintf(err, "%s: trouble allocating threshold %u",
              me, (unsigned int)ni);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
    thr = (unsigned char *)(nthresh[ni]->data);
    for (I=0; I<sx*sy*sz; I++) {
      val = nrrdFLookup[nblur[ni]->type](nblur[ni]->data, I);
      val -= AIR_CAST(float, DWthr);
      thr[I] = (val >= 0 ? 1 : 0);
    }
  }
  if (verb) {
    fprintf(stderr, "done\n");
  }
  
  airMopOkay(mop); 
  return 0;
}

/*
** _tenEpiRegBB: find the biggest bright CC
*/
int
_tenEpiRegBB(Nrrd *nval, Nrrd *nsize) {
  unsigned char *val;
  int *size, big;
  unsigned int ci;

  val = (unsigned char *)(nval->data);
  size = (int *)(nsize->data);
  big = 0;
  for (ci=0; ci<nsize->axis[0].size; ci++) {
    big = val[ci] ? AIR_MAX(big, size[ci]) : big;
  }
  return big;
}

int
_tenEpiRegCC(Nrrd **nthr, int ninLen, int conny, int verb) {
  char me[]="_tenEpiRegCC", err[BIFF_STRLEN];
  Nrrd *nslc, *ncc, *nval, *nsize;
  airArray *mop;
  int ni, z, sz, big;

  mop = airMopNew();
  airMopAdd(mop, nslc=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nval=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ncc=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nsize=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  sz = nthr[0]->axis[2].size;
  if (verb) {
    fprintf(stderr, "%s:\n            ", me); fflush(stderr);
  }
  for (ni=0; ni<ninLen; ni++) {
    if (verb) {
      fprintf(stderr, "%2d ", ni); fflush(stderr);
    }
    /* for each volume, we find the biggest bright 3-D CC, and merge
       down (to dark) all smaller bright pieces.  Then, within each
       slice, we do 2-D CCs, find the biggest bright CC (size == big),
       and merge up (to bright) all small dark pieces, where
       (currently) small is big/2 */
    big = -1;
    if (nrrdCCFind(ncc, &nval, nthr[ni], nrrdTypeDefault, conny)
        || nrrdCCSize(nsize, ncc)
        || !(big = _tenEpiRegBB(nval, nsize)) 
        || nrrdCCMerge(ncc, ncc, nval, -1, big-1, 0, conny)
        || nrrdCCRevalue(nthr[ni], ncc, nval)) {
      if (big) {
        sprintf(err, "%s: trouble with 3-D processing nthr[%d]", me, ni);
        biffMove(TEN, err, NRRD); return 1;
      } else {
        sprintf(err, "%s: got size 0 for biggest bright CC of nthr[%d]",
                me, ni);
        biffAdd(TEN, err); return 1;
      }
    }
    for (z=0; z<sz; z++) {
      big = -1;
      if ( nrrdSlice(nslc, nthr[ni], 2, z)
           || nrrdCCFind(ncc, &nval, nslc, nrrdTypeDefault, conny)
           || nrrdCCSize(nsize, ncc)
           || !(big = _tenEpiRegBB(nval, nsize))
           || nrrdCCMerge(ncc, ncc, nval, 1, big/2, 0, conny)
           || nrrdCCRevalue(nslc, ncc, nval)
           || nrrdSplice(nthr[ni], nthr[ni], nslc, 2, z) ) {
        if (big) {
          sprintf(err, "%s: trouble processing slice %d of nthr[%d]",
                  me, z, ni);
          biffMove(TEN, err, NRRD); return 1;
        } else {
          /* biggest bright CC on this slice had size 0
             <==> there was no bright CC on this slice, move on */
        }
      }
    }
  }
  if (verb) {
    fprintf(stderr, "done\n");
  }

  airMopOkay(mop);
  return 0;
}

#define MEAN_X 0 
#define MEAN_Y 1
#define M_02   2
#define M_11   3
#define M_20   4

/*
** _tenEpiRegMoments()
**
** the moments are stored in (of course) a nrrd, one scanline per slice,
** with each scanline containing:
**
**       0       1       2       3       4
**   mean(x)  mean(y)  M_02    M_11    M_20
*/
int
_tenEpiRegMoments(Nrrd **nmom, Nrrd **nthresh, unsigned int ninLen,
                  int verb) {
  char me[]="_tenEpiRegMoments", err[BIFF_STRLEN];
  size_t sx, sy, sz, xi, yi, zi, ni;
  double N, mx, my, cx, cy, x, y, M02, M11, M20, *mom;
  float val;
  unsigned char *thr;

  sx = nthresh[0]->axis[0].size;
  sy = nthresh[0]->axis[1].size;
  sz = nthresh[0]->axis[2].size;
  if (verb) {
    fprintf(stderr, "%s:\n            ", me); fflush(stderr);
  }
  for (ni=0; ni<ninLen; ni++) {
    if (verb) {
      fprintf(stderr, "%2u ", (unsigned int)ni); fflush(stderr);
    }
    if (nrrdMaybeAlloc_va(nmom[ni], nrrdTypeDouble, 2,
                          AIR_CAST(size_t, 5), sz)) {
      sprintf(err, "%s: couldn't allocate nmom[%u]", me, (unsigned int)ni);
      biffMove(TEN, err, NRRD); return 1;
    }
    nrrdAxisInfoSet_va(nmom[ni], nrrdAxisInfoLabel, "mx,my,h,s,t", "z");
    thr = (unsigned char *)(nthresh[ni]->data);
    mom = (double *)(nmom[ni]->data);
    for (zi=0; zi<sz; zi++) {
      /* ------ find mx, my */
      N = 0;
      mx = my = 0.0;
      for (yi=0; yi<sy; yi++) {
        for (xi=0; xi<sx; xi++) {
          val = thr[xi + sx*yi];
          N += val;
          mx += xi*val;
          my += yi*val;
        }
      }
      if (N == sx*sy) {
        sprintf(err, "%s: saw only non-zero pixels in nthresh[%u]; "
                "DWI hreshold too low?", me, (unsigned int)ni);
        biffAdd(TEN, err); return 1;
      }
      if (N) {
        /* there were non-zero pixels */
        mx /= N;
        my /= N;
        cx = sx/2.0;
        cy = sy/2.0;
        /* ------ find M02, M11, M20 */
        M02 = M11 = M20 = 0.0;
        for (yi=0; yi<sy; yi++) {
          for (xi=0; xi<sx; xi++) {
            val = thr[xi + sx*yi];
            x = xi - cx;
            y = yi - cy;
            M02 += y*y*val;
            M11 += x*y*val;
            M20 += x*x*val;
          }
        }
        M02 /= N;
        M11 /= N;
        M20 /= N;
        /* ------ set output */
        mom[MEAN_X] = mx;
        mom[MEAN_Y] = my;
        mom[M_02] = M02;
        mom[M_11] = M11;
        mom[M_20] = M20;
      } else {
        /* there were no non-zero pixels */
        mom[MEAN_X] = 0;
        mom[MEAN_Y] = 0;
        mom[M_02] = 0;
        mom[M_11] = 0;
        mom[M_20] = 0;
      }
      thr += sx*sy;
      mom += 5;
    }
  }
  if (verb) {
    fprintf(stderr, "done\n");
  }

  return 0;
}

/*
** _tenEpiRegPairXforms
**
** uses moment information to compute all pair-wise transforms, which are
** stored in the 3 x ninLen x ninLen x sizeZ output.  If xfr = npxfr->data,
** xfr[0 + 3*(zi + sz*(A + ninLen*B))] is shear,
** xfr[1 +              "            ] is scale, and 
** xfr[2 +              "            ] is translate in the transform
** that maps slice zi from volume A to volume B.
*/
int
_tenEpiRegPairXforms(Nrrd *npxfr, Nrrd **nmom, int ninLen) {
  char me[]="_tenEpiRegPairXforms", err[BIFF_STRLEN];
  double *xfr, *A, *B, hh, ss, tt;
  int ai, bi, zi, sz;
  
  sz = nmom[0]->axis[1].size;
  if (nrrdMaybeAlloc_va(npxfr, nrrdTypeDouble, 4,
                        AIR_CAST(size_t, 5),
                        AIR_CAST(size_t, sz),
                        AIR_CAST(size_t, ninLen),
                        AIR_CAST(size_t, ninLen))) {
    sprintf(err, "%s: couldn't allocate transform nrrd", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  nrrdAxisInfoSet_va(npxfr, nrrdAxisInfoLabel,
                     "mx,my,h,s,t", "zi", "orig", "target");
  xfr = (double *)(npxfr->data);
  for (bi=0; bi<ninLen; bi++) {
    for (ai=0; ai<ninLen; ai++) {
      for (zi=0; zi<sz; zi++) {
        A = (double*)(nmom[ai]->data) + 5*zi;
        B = (double*)(nmom[bi]->data) + 5*zi;
        ss = sqrt((A[M_20]*B[M_02] - B[M_11]*B[M_11]) /
                  (A[M_20]*A[M_02] - A[M_11]*A[M_11]));
        hh = (B[M_11] - ss*A[M_11])/A[M_20];
        tt = B[MEAN_Y] - A[MEAN_Y];
        ELL_5V_SET(xfr + 5*(zi + sz*(ai + ninLen*bi)),
                   A[MEAN_X], A[MEAN_Y], hh, ss, tt);
      }
    }
  }
  return 0;
}

#define SHEAR  2
#define SCALE  3
#define TRAN   4

int
_tenEpiRegEstimHST(Nrrd *nhst, Nrrd *npxfr, int ninLen, Nrrd *ngrad) {
  char me[]="_tenEpiRegEstimHST", err[BIFF_STRLEN];
  double *hst, *grad, *mat, *vec, *ans, *pxfr, *gA, *gB;
  int z, sz, A, B, npairs, ri;
  Nrrd **nmat, *nvec, **ninv, *nans;
  airArray *mop;
  int order;

  order = 1;

  sz = npxfr->axis[1].size;
  npairs = ninLen*(ninLen-1);

  mop = airMopNew();
  nmat = (Nrrd**)calloc(sz, sizeof(Nrrd*));
  ninv = (Nrrd**)calloc(sz, sizeof(Nrrd*));
  airMopAdd(mop, nmat, airFree, airMopAlways);
  airMopAdd(mop, ninv, airFree, airMopAlways);
  for (z=0; z<sz; z++) {
    airMopAdd(mop, nmat[z]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    if (nrrdMaybeAlloc_va(nmat[z], nrrdTypeDouble, 2,
                          AIR_CAST(size_t, (1 == order ? 3 : 9)),
                          AIR_CAST(size_t, npairs))) {
      sprintf(err, "%s: couldn't allocate fitting matrices", me);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
    airMopAdd(mop, ninv[z]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  }
  airMopAdd(mop, nvec=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nans=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdMaybeAlloc_va(nhst, nrrdTypeDouble, 2,
                        AIR_CAST(size_t, (1 == order ? 9 : 27)),
                        AIR_CAST(size_t, sz))
      || nrrdMaybeAlloc_va(nvec, nrrdTypeDouble, 2,
                           AIR_CAST(size_t, 1),
                           AIR_CAST(size_t, npairs))) {
    sprintf(err, "%s: couldn't allocate HST nrrd", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  nrrdAxisInfoSet_va(nhst, nrrdAxisInfoLabel,
                     (1 == order 
                      ? "Hx,Hy,Hz,Sx,Sy,Sz,Tx,Ty,Tz"
                      : "HST parms"), "z");

  /* ------ per-slice: compute model fitting matrix and its pseudo-inverse */
  grad = (double *)(ngrad->data);
  for (z=0; z<sz; z++) {
    hst = (double *)(nhst->data) + (1 == order ? 9 : 27)*z;
    mat = (double *)(nmat[z]->data);
    ri = 0;
    for (A=0; A<ninLen; A++) {
      for (B=0; B<ninLen; B++) {
        if (A == B) continue;
        pxfr = (double *)(npxfr->data) + 0 + 5*(z + sz*(A + ninLen*B));
        gA = grad + 0 + 3*A;
        gB = grad + 0 + 3*B;
        if (1 == order) {
          ELL_3V_SET(mat + 3*ri,
                     gB[0] - pxfr[SCALE]*gA[0],
                     gB[1] - pxfr[SCALE]*gA[1],
                     gB[2] - pxfr[SCALE]*gA[2]);
        } else {
          ELL_9V_SET(mat + 9*ri,
                     gB[0] - pxfr[SCALE]*gA[0],
                     gB[1] - pxfr[SCALE]*gA[1],
                     gB[2] - pxfr[SCALE]*gA[2],
                     gB[0]*gB[0]*gB[0] - pxfr[SCALE]*gA[0]*gA[0]*gA[0],
                     gB[1]*gB[1]*gB[1] - pxfr[SCALE]*gA[1]*gA[1]*gA[1],
                     gB[2]*gB[2]*gB[2] - pxfr[SCALE]*gA[2]*gA[2]*gA[2],
                     gB[0]*gB[1]*gB[2] - pxfr[SCALE]*gA[0]*gA[1]*gA[2],
                     1, 1);
          /*
                     gB[0]*gB[1] - pxfr[SCALE]*gA[0]*gA[1],
                     gB[0]*gB[2] - pxfr[SCALE]*gA[0]*gA[2],
                     gB[1]*gB[2] - pxfr[SCALE]*gA[1]*gA[2]);
                     */
        }
        ri += 1;
      }
    }
    if (nrrdHasNonExist(nmat[z])) {
      /* as happens if there were zero slices in the segmentation output */
      if (1 == order) {
        ELL_3V_SET(hst + 0*3, 0, 0, 0);
        ELL_3V_SET(hst + 1*3, 0, 0, 0);
        ELL_3V_SET(hst + 2*3, 0, 0, 0);
      } else {
        ELL_9V_SET(hst + 0*9, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        ELL_9V_SET(hst + 1*9, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        ELL_9V_SET(hst + 2*9, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      }
    } else {
      if (ell_Nm_pseudo_inv(ninv[z], nmat[z])) {
        sprintf(err, "%s: trouble estimating model (slice %d)", me, z);
        biffMove(TEN, err, ELL); airMopError(mop); return 1;
      }
    }
  }
  vec = (double *)(nvec->data);

  /* ------ find Sx, Sy, Sz per slice */
  for (z=0; z<sz; z++) {
    if (nrrdHasNonExist(nmat[z])) {
      /* we've already zero-ed out this row in the HST nrrd */
      continue;
    }
    hst = (double *)(nhst->data) + (1 == order ? 9 : 27)*z;
    ri = 0;
    for (A=0; A<ninLen; A++) {
      for (B=0; B<ninLen; B++) {
        if (A == B) continue;
        pxfr = (double *)(npxfr->data) + 0 + 5*(z + sz*(A + ninLen*B));
        vec[ri] = pxfr[SCALE] - 1;
        ri += 1;
      }
    }
    if (ell_Nm_mul(nans, ninv[z], nvec)) {
      sprintf(err, "%s: trouble estimating model (slice %d): Sx, Sy, Sz",
              me, z);
      biffMove(TEN, err, ELL); airMopError(mop); return 1;
    }
    ans = (double *)(nans->data);
    if (1 == order) {
      ELL_3V_COPY(hst + 1*3, ans);
    } else {
      ELL_9V_COPY(hst + 1*9, ans);
    }
  }

  /* ------ find Hx, Hy, Hz per slice */
  for (z=0; z<sz; z++) {
    if (nrrdHasNonExist(nmat[z])) {
      /* we've already zero-ed out this row in the HST nrrd */
      continue;
    }
    hst = (double *)(nhst->data) + (1 == order ? 9 : 27)*z;
    ri = 0;
    for (A=0; A<ninLen; A++) {
      for (B=0; B<ninLen; B++) {
        if (A == B) continue;
        pxfr = (double *)(npxfr->data) + 0 + 5*(z + sz*(A + ninLen*B));
        vec[ri] = pxfr[SHEAR];
        ri += 1;
      }
    }
    if (ell_Nm_mul(nans, ninv[z], nvec)) {
      sprintf(err, "%s: trouble estimating model (slice %d): Hx, Hy, Hz",
              me, z);
      biffMove(TEN, err, ELL); airMopError(mop); return 1;
    }
    ans = (double *)(nans->data);
    if (1 == order) {
      ELL_3V_COPY(hst + 0*3, ans);
    } else {
      ELL_9V_COPY(hst + 0*9, ans);
    }
  }

  /* ------ find Tx, Ty, Tz per slice */
  for (z=0; z<sz; z++) {
    if (nrrdHasNonExist(nmat[z])) {
      /* we've already zero-ed out this row in the HST nrrd */
      continue;
    }
    hst = (double *)(nhst->data) + (1 == order ? 9 : 27)*z;
    ri = 0;
    for (A=0; A<ninLen; A++) {
      for (B=0; B<ninLen; B++) {
        if (A == B) continue;
        pxfr = (double *)(npxfr->data) + 0 + 5*(z + sz*(A + ninLen*B));
        vec[ri] = pxfr[TRAN];
        ri += 1;
      }
    }
    if (ell_Nm_mul(nans, ninv[z], nvec)) {
      sprintf(err, "%s: trouble estimating model (slice %d): Tx, Ty, Tz",
              me, z);
      biffMove(TEN, err, ELL); airMopError(mop); return 1;
    }
    ans = (double *)(nans->data);
    if (1 == order) {
      ELL_3V_COPY(hst + 2*3, ans);
    } else {
      ELL_9V_COPY(hst + 2*9, ans);
    }
  }

  airMopOkay(mop);
  return 0;
}

int
_tenEpiRegFitHST(Nrrd *nhst, Nrrd **_ncc, int ninLen,
                 double goodFrac, int prog, int verb) {
  char me[]="_tenEpiRegFitHST", err[BIFF_STRLEN];
  airArray *mop;
  Nrrd *ncc, *ntA, *ntB, *nsd, *nl2;
  unsigned int cc, sz, zi, sh, hi;
  float *mess, *two, tmp;
  double *hst, x, y, xx, xy, mm, bb;

  mop = airMopNew();
  airMopAdd(mop, ncc=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntA=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntB=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nsd=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nl2=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  /* do SD and L2 projections of the CCs along the DWI axis,
     integrate these over the X and Y axes of the slices,
     and define per-slice "messiness" as the quotient of the
     SD integral with the L2 integral */
  if (verb) {
    fprintf(stderr, "%s: measuring segmentation uncertainty ... ", me);
    fflush(stderr);
  }
  if (nrrdJoin(ncc, (const Nrrd**)_ncc, ninLen, 0, AIR_TRUE)
      || nrrdProject(ntA, ncc, 0, nrrdMeasureSD, nrrdTypeFloat)
      || nrrdProject(ntB, ntA, 0, nrrdMeasureSum, nrrdTypeFloat)
      || nrrdProject(nsd, ntB, 0, nrrdMeasureSum, nrrdTypeFloat)
      || nrrdProject(ntA, ncc, 0, nrrdMeasureL2, nrrdTypeFloat)
      || nrrdProject(ntB, ntA, 0, nrrdMeasureSum, nrrdTypeFloat)
      || nrrdProject(nl2, ntB, 0, nrrdMeasureSum, nrrdTypeFloat)
      || nrrdArithBinaryOp(ntA, nrrdBinaryOpDivide, nsd, nl2)) {
    sprintf(err, "%s: trouble doing CC projections", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  if (verb) {
    fprintf(stderr, "done\n");
  }
  if (prog && _tenEpiRegSave("regtmp-messy.txt", ntA,
                             NULL, 0, "segmentation uncertainty")) {
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  /* now ntA stores the per-slice messiness */
  mess = AIR_CAST(float*, ntA->data);

  /* allocate an array of 2 floats per slice */
  sz = ntA->axis[0].size;
  two = AIR_CAST(float*, calloc(2*sz, sizeof(float)));
  if (!two) {
    sprintf(err, "%s: couldn't allocate tmp buffer", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, two, airFree, airMopAlways);
  /* initial ordering: messiness, slice index */
  for (zi=0; zi<sz; zi++) {
    two[0 + 2*zi] = (AIR_EXISTS(mess[zi])
                     ? mess[zi]
                     : 666);  /* don't use empty slices */
    two[1 + 2*zi] = AIR_CAST(float, zi);
  }
  /* sort into ascending messiness */
  qsort(two, zi, 2*sizeof(float), nrrdValCompare[nrrdTypeFloat]);
  /* flip ordering while thresholding messiness into usability */
  for (zi=0; zi<sz; zi++) {
    tmp = two[1 + 2*zi];
    two[1 + 2*zi] = AIR_AFFINE(0, zi, sz-1, 0, 1) <= goodFrac ? 1.0f : 0.0f;
    two[0 + 2*zi] = tmp;
  }  
  /* sort again, now into ascending slice order */
  qsort(two, zi, 2*sizeof(float), nrrdValCompare[nrrdTypeFloat]);
  if (verb) {
    fprintf(stderr, "%s: using slices", me);
    for (zi=0; zi<sz; zi++) {
      if (two[1 + 2*zi]) {
        fprintf(stderr, " %u", zi);
      }
    }
    fprintf(stderr, " for fitting\n");
  }

  /* perform fitting for each column in hst (regardless of 
     whether we're using a 1st or 2nd order model */
  hst = (double*)(nhst->data);
  sh = nhst->axis[0].size;
  for (hi=0; hi<sh; hi++) {
    x = y = xy = xx = 0;
    cc = 0;
    for (zi=0; zi<sz; zi++) {
      if (!two[1 + 2*zi])
        continue;
      cc += 1;
      x += zi;
      xx += zi*zi;
      y += hst[hi + sh*zi];
      xy += zi*hst[hi + sh*zi];
    }
    x /= cc; xx /= cc; y /= cc; xy /= cc;
    mm = (xy - x*y)/(xx - x*x);
    bb = y - mm*x;
    for (zi=0; zi<sz; zi++) {
      hst[hi + sh*zi] = mm*zi + bb;
    }
  }

  airMopOkay(mop);
  return 0;
}

int
_tenEpiRegGetHST(double *hhP, double *ssP, double *ttP,
                 int reference, int ni, int zi,
                 Nrrd *npxfr, Nrrd *nhst, Nrrd *ngrad) {
  double *xfr, *hst, *_g, grad[9]; /* big enough for 2nd order */
  int sz, ninLen;

  int order;
  
  order = 1;

  /* these could also have been passed to us, but we can also discover them */
  sz = npxfr->axis[1].size;
  ninLen = npxfr->axis[2].size;

  if (-1 == reference) {
    /* we use the estimated H,S,T vectors to determine distortion
       as a function of gradient direction, and then invert this */
    _g = (double*)(ngrad->data) + 0 + 3*ni;
    if (1 == order) {
      hst = (double*)(nhst->data) + 0 + 9*zi;
      *hhP = ELL_3V_DOT(_g, hst + 0*3);
      *ssP = 1 + ELL_3V_DOT(_g, hst + 1*3);
      *ttP = ELL_3V_DOT(_g, hst + 2*3);
    } else {
      hst = (double*)(nhst->data) + 0 + 27*zi;
      ELL_9V_SET(grad, _g[0], _g[1], _g[2],
                 _g[0]*_g[0]*_g[0], _g[1]*_g[1]*_g[1], _g[2]*_g[2]*_g[2],
                 _g[0]*_g[1]*_g[2],
                 0, 0);
                 /*
                 _g[0]*_g[1], _g[0]*_g[2], _g[1]*_g[2]);
                 */
      *hhP = ELL_9V_DOT(grad, hst + 0*9);
      *ssP = 1 + ELL_9V_DOT(grad, hst + 1*9);
      *ttP = ELL_9V_DOT(grad, hst + 2*9);
    }
  } else {
    /* we register against a specific DWI */
    xfr = (double*)(npxfr->data) + 0 + 5*(zi + sz*(reference + ninLen*ni));
    *hhP = xfr[2];
    *ssP = xfr[3];
    *ttP = xfr[4];
  }
  return 0;
}

/*
** _tenEpiRegSliceWarp
**
** Apply [hh,ss,tt] transform to nin, putting results in nout, but with
** some trickiness:
** - nwght and nidx are already allocated to the the weights (type float)
**   and indices for resampling nin with "kern" and "kparm"
** - nout is already allocated to the correct size and type
** - nin is type float, but output must be type nout->type
** - nin is been transposed to have the resampled axis fastest in memory,
**   but nout output will not be transposed
*/
int
_tenEpiRegSliceWarp(Nrrd *nout, Nrrd *nin, Nrrd *nwght, Nrrd *nidx, 
                    const NrrdKernel *kern, double *kparm,
                    double hh, double ss, double tt, double cx, double cy) {
  float *wght, *in, pp, pf, tmp;
  int *idx;
  unsigned int supp;
  size_t sx, sy, xi, yi, pb, pi;
  double (*ins)(void *, size_t, double), (*clamp)(double);
  
  sy = nin->axis[0].size;
  sx = nin->axis[1].size;
  supp = AIR_CAST(unsigned int, kern->support(kparm));
  ins = nrrdDInsert[nout->type];
  clamp = nrrdDClamp[nout->type];

  in = AIR_CAST(float*, nin->data);
  for (xi=0; xi<sx; xi++) {
    idx = AIR_CAST(int*, nidx->data);
    wght = AIR_CAST(float*, nwght->data);
    for (yi=0; yi<sy; yi++) {
      pp = AIR_CAST(float, hh*(xi - cx) + ss*(yi - cy) + tt + cy);
      pb = AIR_CAST(size_t, floor(pp));
      pf = pp - pb;
      for (pi=0; pi<2*supp; pi++) {
        idx[pi] = AIR_MIN(pb + pi - (supp-1), sy-1);
        wght[pi] = pi - (supp-1) - pf;
      }
      idx += 2*supp;
      wght += 2*supp;
    }
    idx = AIR_CAST(int*, nidx->data);
    wght = AIR_CAST(float*, nwght->data);
    kern->evalN_f(wght, wght, 2*supp*sy, kparm);
    for (yi=0; yi<sy; yi++) {
      tmp = 0;
      for (pi=0; pi<2*supp; pi++) {
        tmp += in[idx[pi]]*wght[pi];
      }
      ins(nout->data, xi + sx*yi, clamp(ss*tmp));
      idx += 2*supp;
      wght += 2*supp;
    }
    in += sy;
  }

  return 0;
}

/*
** _tenEpiRegWarp()
**
*/
int
_tenEpiRegWarp(Nrrd **ndone, Nrrd *npxfr, Nrrd *nhst, Nrrd *ngrad,
               Nrrd **nin, int ninLen,
               int reference, const NrrdKernel *kern, double *kparm,
               int verb) {
  char me[]="_tenEpiRegWarp", err[BIFF_STRLEN];
  Nrrd *ntmp, *nfin, *nslcA, *nslcB, *nwght, *nidx;
  airArray *mop;
  int sx, sy, sz, ni, zi, supp;
  double hh, ss, tt, cx, cy;

  mop = airMopNew();
  airMopAdd(mop, ntmp=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nfin=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nslcA=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nslcB=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nwght=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nidx=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);

  if (verb) {
    fprintf(stderr, "%s:\n            ", me); fflush(stderr);
  }
  sx = nin[0]->axis[0].size;
  sy = nin[0]->axis[1].size;
  sz = nin[0]->axis[2].size;
  cx = sx/2.0;
  cy = sy/2.0;
  supp = (int)kern->support(kparm);
  if (nrrdMaybeAlloc_va(nwght, nrrdTypeFloat, 2,
                        AIR_CAST(size_t, 2*supp),
                        AIR_CAST(size_t, sy))
      || nrrdMaybeAlloc_va(nidx, nrrdTypeInt, 2,
                           AIR_CAST(size_t, 2*supp),
                           AIR_CAST(size_t, sy))) {
    sprintf(err, "%s: trouble allocating buffers", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  for (ni=0; ni<ninLen; ni++) {
    if (verb) {
      fprintf(stderr, "%2d ", ni); fflush(stderr);
    }
    if (nrrdCopy(ndone[ni], nin[ni])
        || ((!ni) && nrrdSlice(nslcB, ndone[ni], 2, 0)) /* only when 0==ni */
        || nrrdAxesSwap(ntmp, nin[ni], 0, 1)
        || nrrdConvert(nfin, ntmp, nrrdTypeFloat)) {
      sprintf(err, "%s: trouble prepping at ni=%d", me, ni);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
    for (zi=0; zi<sz; zi++) {
      if (_tenEpiRegGetHST(&hh, &ss, &tt, reference,
                           ni, zi, npxfr, nhst, ngrad)
          || nrrdSlice(nslcA, nfin, 2, zi)
          || _tenEpiRegSliceWarp(nslcB, nslcA, nwght, nidx, kern, kparm,
                                 hh, ss, tt, cx, cy)
          || nrrdSplice(ndone[ni], ndone[ni], nslcB, 2, zi)) {
        sprintf(err, "%s: trouble on slice %d if ni=%d", me, zi, ni);
        /* because the _tenEpiReg calls above don't use biff */
        biffMove(TEN, err, NRRD); airMopError(mop); return 1;
      }
    }
  }
  if (verb) {
    fprintf(stderr, "done\n");
  }

  airMopOkay(mop);
  return 0;
}

int
tenEpiRegister3D(Nrrd **nout, Nrrd **nin, unsigned int ninLen, Nrrd *_ngrad,
                 int reference,
                 double bwX, double bwY, double fitFrac,
                 double DWthr, int doCC, 
                 const NrrdKernel *kern, double *kparm,
                 int progress, int verbose) {
  char me[]="tenEpiRegister3D", err[BIFF_STRLEN];
  airArray *mop;
  Nrrd **nbuffA, **nbuffB, *npxfr, *nhst, *ngrad;
  int hack1, hack2;
  unsigned int i;

  hack1 = nrrdStateAlwaysSetContent;
  hack2 = nrrdStateDisableContent;
  nrrdStateAlwaysSetContent = AIR_FALSE;
  nrrdStateDisableContent = AIR_TRUE;

  mop = airMopNew();
  if (_tenEpiRegCheck(nout, nin, ninLen, _ngrad, reference,
                      bwX, bwY, DWthr,
                      kern, kparm)) {
    sprintf(err, "%s: trouble with input", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }

  nbuffA = (Nrrd **)calloc(ninLen, sizeof(Nrrd*));
  nbuffB = (Nrrd **)calloc(ninLen, sizeof(Nrrd*));
  if (!( nbuffA && nbuffB )) {
    sprintf(err, "%s: couldn't allocate tmp nrrd pointer arrays", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, nbuffA, airFree, airMopAlways);
  airMopAdd(mop, nbuffB, airFree, airMopAlways);
  for (i=0; i<ninLen; i++) {
    airMopAdd(mop, nbuffA[i] = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    airMopAdd(mop, nbuffB[i] = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    nrrdAxisInfoCopy(nout[i], nin[i], NULL, NRRD_AXIS_INFO_NONE);
  }
  airMopAdd(mop, npxfr = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nhst = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ngrad = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(ngrad, _ngrad, nrrdTypeDouble)) {
    sprintf(err, "%s: trouble converting gradients to doubles", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }

  /* ------ blur */
  if (_tenEpiRegBlur(nbuffA, nin, ninLen, bwX, bwY, verbose)) {
    sprintf(err, "%s: trouble %s", me, (bwX || bwY) ? "blurring" : "copying");
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  if (progress && _tenEpiRegSave("regtmp-blur.nrrd", NULL,
                                 nbuffA, ninLen, "blurred DWIs")) {
    airMopError(mop); return 1;
  }

  /* ------ threshold */
  if (_tenEpiRegThreshold(nbuffB, nbuffA, ninLen, 
                          DWthr, verbose, progress)) {
    sprintf(err, "%s: trouble thresholding", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  if (progress && _tenEpiRegSave("regtmp-thresh.nrrd", NULL,
                                 nbuffB, ninLen, "thresholded DWIs")) {
    airMopError(mop); return 1;
  }

  /* ------ connected components */
  if (doCC) {
    if (_tenEpiRegCC(nbuffB, ninLen, 1, verbose)) {
      sprintf(err, "%s: trouble doing connected components", me);
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
    if (progress && _tenEpiRegSave("regtmp-ccs.nrrd", NULL,
                                   nbuffB, ninLen, "connected components")) {
      airMopError(mop); return 1;
    }
  }

  /* ------ moments */
  if (_tenEpiRegMoments(nbuffA, nbuffB, ninLen, verbose)) {
    sprintf(err, "%s: trouble finding moments", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  if (progress && _tenEpiRegSave("regtmp-mom.nrrd", NULL,
                                 nbuffA, ninLen, "moments")) {
    airMopError(mop); return 1;
  }

  /* ------ transforms */
  if (_tenEpiRegPairXforms(npxfr, nbuffA, ninLen)) {
    sprintf(err, "%s: trouble calculating transforms", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  if (progress && _tenEpiRegSave("regtmp-pxfr.nrrd", npxfr,
                                 NULL, 0, "pair-wise xforms")) {
    airMopError(mop); return 1;
  }

  if (-1 == reference) {
    /* ------ HST estimation */
    if (_tenEpiRegEstimHST(nhst, npxfr, ninLen, ngrad)) {
      sprintf(err, "%s: trouble estimating HST", me);
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
    if (progress && _tenEpiRegSave("regtmp-hst.txt", nhst,
                                   NULL, 0, "HST estimates")) {
      airMopError(mop); return 1;
    }

    if (fitFrac) {
      /* ------ HST parameter fitting */
      if (_tenEpiRegFitHST(nhst, nbuffB, ninLen, fitFrac, progress, verbose)) {
        sprintf(err, "%s: trouble fitting HST", me);
        biffAdd(TEN, err); airMopError(mop); return 1;
      }
      if (progress && _tenEpiRegSave("regtmp-fit-hst.txt", nhst,
                                     NULL, 0, "fitted HST")) {
        airMopError(mop); return 1;
      }
    }
  }

  /* ------ doit */
  if (_tenEpiRegWarp(nout, npxfr, nhst, ngrad, nin, ninLen,
                     reference, kern, kparm, verbose)) {
    sprintf(err, "%s: trouble performing final registration", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  
  airMopOkay(mop);
  nrrdStateAlwaysSetContent = hack1;
  nrrdStateDisableContent = hack2;
  return 0;
}

int
tenEpiRegister4D(Nrrd *_nout, Nrrd *_nin, Nrrd *_ngrad,
                 int reference,
                 double bwX, double bwY, double fitFrac,
                 double DWthr, int doCC, 
                 const NrrdKernel *kern, double *kparm,
                 int progress, int verbose) {
  char me[]="tenEpiRegister4D", err[BIFF_STRLEN];
  unsigned int ninIdx, ninLen,
    dwiAx, rangeAxisNum, rangeAxisIdx[NRRD_DIM_MAX];
  int dwiIdx;
  Nrrd **nout, **nin, **ndwi, **ndwiOut, *ngrad, *ndwigrad;
  airArray *mop;
  double *grad, *dwigrad, glen;

  if (!(_nout && _nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (4 != _nin->dim) {
    sprintf(err, "%s: need a 4-D input array, not %d-D", me, _nin->dim);
    biffAdd(TEN, err); return 1;
  }
  if (tenGradientCheck(_ngrad, nrrdTypeDefault, 6 /* <-- HEY: not sure */)) {
    sprintf(err, "%s: problem with given gradient list", me);
    biffAdd(TEN, err); return 1;
  }

  rangeAxisNum = nrrdRangeAxesGet(_nin, rangeAxisIdx);
  if (0 == rangeAxisNum) {
    /* we fall back on old behavior */
    dwiAx = 0;
  } else if (1 == rangeAxisNum) {
    /* thankfully there's exactly one range axis */
    dwiAx = rangeAxisIdx[0];
  } else {
    sprintf(err, "%s: have %u range axes instead of 1, don't know which "
            "is DWI axis", me, rangeAxisNum);
    biffAdd(TEN, err); return 1;
  }

  ninLen = _nin->axis[dwiAx].size;
  if (!( AIR_IN_CL(6, ninLen, 120) )) {
    sprintf(err, "%s: %u (size of axis %u, and # DWIs) is unreasonable", 
            me, ninLen, dwiAx);
    biffAdd(TEN, err); return 1;
  }
  if (ninLen != _ngrad->axis[1].size) {
    sprintf(err, "%s: ninLen %u != # grads " _AIR_SIZE_T_CNV, 
            me, ninLen, _ngrad->axis[1].size);
    biffAdd(TEN, err); return 1;
  }

  mop = airMopNew();
  ngrad = nrrdNew();
  ndwigrad = nrrdNew();
  airMopAdd(mop, ngrad, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ndwigrad, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(ngrad, _ngrad, nrrdTypeDouble)
      || nrrdConvert(ndwigrad, _ngrad, nrrdTypeDouble)) { /* HACK applies */
    sprintf(err, "%s: trouble converting gradients to doubles", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }

  nin = (Nrrd **)calloc(ninLen, sizeof(Nrrd*));
  ndwi = (Nrrd **)calloc(ninLen, sizeof(Nrrd*));
  nout = (Nrrd **)calloc(ninLen, sizeof(Nrrd*));
  ndwiOut = (Nrrd **)calloc(ninLen, sizeof(Nrrd*));
  if (!(nin && ndwi && nout && ndwiOut)) {
    sprintf(err, "%s: trouble allocating local arrays", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, nin, airFree, airMopAlways);
  airMopAdd(mop, ndwi, airFree, airMopAlways);
  airMopAdd(mop, nout, airFree, airMopAlways);
  airMopAdd(mop, ndwiOut, airFree, airMopAlways);
  dwiIdx = -1;
  grad = AIR_CAST(double *, ngrad->data);
  dwigrad = AIR_CAST(double *, ndwigrad->data);
  for (ninIdx=0; ninIdx<ninLen; ninIdx++) {
    glen = ELL_3V_LEN(grad + 3*ninIdx);
    if (-1 != reference || glen > 0.0) {
      /* we're not allowed to use only those images that are truly DWIs,
         or,
         (we are so allowed and) this is a true DWI */
      dwiIdx++;
      ndwi[dwiIdx] = nrrdNew();
      ndwiOut[dwiIdx] = nrrdNew();
      airMopAdd(mop, ndwi[dwiIdx], (airMopper)nrrdNuke, airMopAlways);
      airMopAdd(mop, ndwiOut[dwiIdx], (airMopper)nrrdNuke, airMopAlways);
      if (nrrdSlice(ndwi[dwiIdx], _nin, dwiAx,
                    AIR_CAST(unsigned int, ninIdx))) {
        sprintf(err, "%s: trouble slicing at %d on axis %u",
                me, ninIdx, dwiAx);
        biffMove(TEN, err, NRRD); airMopError(mop); return 1;
      }
      /* NOTE: this works because dwiIdx <= ninIdx */
      ELL_3V_COPY(dwigrad + 3*dwiIdx, grad + 3*ninIdx);
    } else {
      /* we are only looking at true DWIs, and this isn't one of them */
      continue;
    }
  }
  if (-1 == dwiIdx) {
    sprintf(err, "%s: somehow got no DWIs", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  /* HEY: HACK! */
  ndwigrad->axis[1].size = 1 + AIR_CAST(unsigned int, dwiIdx);
  if (tenEpiRegister3D(ndwiOut, ndwi, ndwigrad->axis[1].size, ndwigrad,
                       reference,
                       bwX, bwY, fitFrac, DWthr,
                       doCC,
                       kern, kparm,
                       progress, verbose)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  dwiIdx = -1;
  for (ninIdx=0; ninIdx<ninLen; ninIdx++) {
    glen = ELL_3V_LEN(grad + 3*ninIdx);
    if (-1 != reference || glen > 0.0) {
      /* we're not allowed to use only those images that are truly DWIs,
         or,
         (we are so allowed and) this is a true DWI */
      dwiIdx++;
      nout[ninIdx] = ndwiOut[dwiIdx];
      /*
      fprintf(stderr, "!%s: (A) nout[%u] = %p\n", me, ninIdx, nout[ninIdx]);
      */
    } else {
      /* we are only looking at true DWIs, and this isn't one of them */
      nout[ninIdx] = nrrdNew();
      airMopAdd(mop, nout[ninIdx], (airMopper)nrrdNuke, airMopAlways);
      if (nrrdSlice(nout[ninIdx], _nin, dwiAx, ninIdx)) {
        sprintf(err, "%s: trouble slicing at %d on axis %u",
                me, dwiIdx, dwiAx);
        biffMove(TEN, err, NRRD); airMopError(mop); return 1;
      }
      /*
      fprintf(stderr, "!%s: (B) nout[%u] = %p\n", me, ninIdx, nout[ninIdx]);
      */
    }
  }
  if (nrrdJoin(_nout, (const Nrrd**)nout, ninLen, dwiAx, AIR_TRUE)) {
    sprintf(err, "%s: trouble joining output", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  nrrdAxisInfoCopy(_nout, _nin, NULL, NRRD_AXIS_INFO_NONE);
  if (nrrdBasicInfoCopy(_nout, _nin,
                        NRRD_BASIC_INFO_DATA_BIT
                        | NRRD_BASIC_INFO_TYPE_BIT
                        | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                        | NRRD_BASIC_INFO_DIMENSION_BIT
                        | NRRD_BASIC_INFO_CONTENT_BIT
                        | NRRD_BASIC_INFO_COMMENTS_BIT
                        | (nrrdStateKeyValuePairsPropagate
                           ? 0
                           : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    sprintf(err, "%s:", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  
  airMopOkay(mop);
  return 0;
}
