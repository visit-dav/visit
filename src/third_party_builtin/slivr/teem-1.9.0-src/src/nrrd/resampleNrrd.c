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

#include "nrrd.h"
#include "privateNrrd.h"

/*
  (this was written before airMopSub ... )
learned: if you start using airMop stuff, and you register a free, but
then you free the memory yourself, YOU HAVE GOT TO register a NULL in
place of the original free.  The next malloc may end up at the same
address as what you just freed, and if you want this memory to NOT be
mopped up, then you'll be confused with the original registered free
goes into effect and mops it up for you, even though YOU NEVER
REGISTERED a free for the second malloc.  If you want simple stupid
tools, you have to treat them accordingly (be extremely careful with
fire).  

learned: well, duh.  The reason to use:

    for (I=0; I<numOut; I++) {

instead of

    for (I=0; I<=numOut-1; I++) {

is that if numOut is of an unsigned type and has value 0, then these
two will have very different results!

*/

int
nrrdSimpleResample(Nrrd *nout, Nrrd *nin,
                   const NrrdKernel *kernel, const double *parm,
                   const size_t *samples, const double *scalings) {
  char me[]="nrrdSimpleResample", err[BIFF_STRLEN];
  NrrdResampleInfo *info;
  int p, np, center;
  unsigned ai;

  if (!(nout && nin && kernel && (samples || scalings))) {
    sprintf(err, "%s: not NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (!(info = nrrdResampleInfoNew())) {
    sprintf(err, "%s: can't allocate resample info struct", me);
    biffAdd(NRRD, err); return 1;
  }

  np = kernel->numParm;
  for (ai=0; ai<nin->dim; ai++) {
    info->kernel[ai] = kernel;
    if (samples) {
      info->samples[ai] = samples[ai];
    } else {
      center = _nrrdCenter(nin->axis[ai].center);
      if (nrrdCenterCell == center) {
        info->samples[ai] = (size_t)(nin->axis[ai].size*scalings[ai]);
      } else {
        info->samples[ai] = (size_t)((nin->axis[ai].size - 1)
                                     *scalings[ai]) + 1;
      }
    }
    for (p=0; p<np; p++)
      info->parm[ai][p] = parm[p];
    /* set the min/max for this axis if not already set to something */
    if (!( AIR_EXISTS(nin->axis[ai].min) && AIR_EXISTS(nin->axis[ai].max) ))
      nrrdAxisInfoMinMaxSet(nin, ai, nrrdDefaultCenter);
    info->min[ai] = nin->axis[ai].min;
    info->max[ai] = nin->axis[ai].max;
  }
  /* we go with the defaults (enstated by _nrrdResampleInfoInit())
     for all the remaining fields */

  if (nrrdSpatialResample(nout, nin, info)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }

  info = nrrdResampleInfoNix(info);
  return 0;
}

/*
** _nrrdResampleCheckInfo()
**
** checks validity of given NrrdResampleInfo *info: 
** - all required parameters exist
** - both min[d] and max[d] for all axes d
*/
int
_nrrdResampleCheckInfo(const Nrrd *nin, const NrrdResampleInfo *info) {
  char me[] = "_nrrdResampleCheckInfo", err[BIFF_STRLEN];
  const NrrdKernel *k;
  int center, p, np;
  unsigned int ai, minsmp;

  if (nrrdTypeBlock == nin->type || nrrdTypeBlock == info->type) {
    sprintf(err, "%s: can't resample to or from type %s", me,
            airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdBoundaryUnknown == info->boundary) {
    sprintf(err, "%s: didn't set boundary behavior\n", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdBoundaryPad == info->boundary && !AIR_EXISTS(info->padValue)) {
    sprintf(err, "%s: asked for boundary padding, but no pad value set\n", me);
    biffAdd(NRRD, err); return 1;
  }
  for (ai=0; ai<nin->dim; ai++) {
    k = info->kernel[ai];
    /* we only care about the axes being resampled */
    if (!k)
      continue;
    if (!(info->samples[ai] > 0)) {
      sprintf(err, "%s: axis %d # samples (" _AIR_SIZE_T_CNV ") invalid", 
              me, ai, info->samples[ai]);
      biffAdd(NRRD, err); return 1;
    }
    if (!( AIR_EXISTS(nin->axis[ai].min) && AIR_EXISTS(nin->axis[ai].max) )) {
      sprintf(err, "%s: input nrrd's axis %d min,max have not both been set",
              me, ai);
      biffAdd(NRRD, err); return 1;
    }
    if (!( AIR_EXISTS(info->min[ai]) && AIR_EXISTS(info->max[ai]) )) {
      sprintf(err, "%s: info's axis %d min,max not both set", me, ai);
      biffAdd(NRRD, err); return 1;
    }
    np = k->numParm;
    for (p=0; p<np; p++) {
      if (!AIR_EXISTS(info->parm[ai][p])) {
        sprintf(err, "%s: didn't set parameter %d (of %d) for axis %d\n",
                me, p, np, ai);
        biffAdd(NRRD, err); return 1;
      }
    }
    center = _nrrdCenter(nin->axis[ai].center);
    minsmp = nrrdCenterCell == center ? 1 : 2;
    if (!( nin->axis[ai].size >= minsmp && info->samples[ai] >= minsmp )) {
      sprintf(err, "%s: axis %d # input samples (" _AIR_SIZE_T_CNV 
              ") or output samples (" _AIR_SIZE_T_CNV ") "
              " invalid for %s centering",
              me, ai, nin->axis[ai].size, info->samples[ai],
              airEnumStr(nrrdCenter, center));
      biffAdd(NRRD, err); return 1;
    }
  }
  return 0;
}

/*
** _nrrdResampleComputePermute()
**
** figures out information related to how the axes in a nrrd are
** permuted during resampling: permute, topRax, botRax, passes, ax[][], sz[][]
*/
void
_nrrdResampleComputePermute(unsigned int permute[], 
                            unsigned int ax[NRRD_DIM_MAX][NRRD_DIM_MAX], 
                            size_t sz[NRRD_DIM_MAX][NRRD_DIM_MAX], 
                            int *topRax,
                            int *botRax,
                            unsigned int *passes,
                            const Nrrd *nin,
                            const NrrdResampleInfo *info) {
  /* char me[]="_nrrdResampleComputePermute"; */
  unsigned int bi, ai, pi;
  
  /* what are the first (top) and last (bottom) axes being resampled? */
  *topRax = *botRax = -1;
  for (ai=0; ai<nin->dim; ai++) {
    if (info->kernel[ai]) {
      if (*topRax < 0) {
        *topRax = ai;
      }
      *botRax = ai;
    }
  }

  /* figure out total number of passes needed, and construct the
     permute[] array.  permute[i] = j means that the axis in position
     i of the old array will be in position j of the new one
     (permute[] answers "where do I put this", not "what do I put here").
  */
  *passes = bi = 0;
  for (ai=0; ai<nin->dim; ai++) {
    if (info->kernel[ai]) {
      do {
        bi = AIR_MOD((int)bi+1, (int)nin->dim); /* HEY scrutinize casts */
      } while (!info->kernel[bi]);
      permute[bi] = ai;
      *passes += 1;
    } else {
      permute[ai] = ai;
      bi += bi == ai;
    }
  }
  permute[nin->dim] = nin->dim;
  if (!*passes) {
    /* none of the kernels was non-NULL */
    return;
  }
  
  /*
  fprintf(stderr, "%s: permute:\n", me);
  for (d=0; d<nin->dim; d++) {
    fprintf(stderr, "   permute[%d] = %d\n", d, permute[ai]);
  }
  */

  /* create array of how the axes will be arranged in each pass ("ax"), 
     and create array of how big each axes is in each pass ("sz").
     The input to pass i will have axis layout described in ax[i] and
     axis sizes described in sz[i] */
  for (ai=0; ai<nin->dim; ai++) {
    ax[0][ai] = ai;
    sz[0][ai] = nin->axis[ai].size;
  }
  for (pi=0; pi<*passes; pi++) {
    for (ai=0; ai<nin->dim; ai++) {
      ax[pi+1][permute[ai]] = ax[pi][ai];
      if (ai == (unsigned int)*topRax) {  /* HEY scrutinize casts */
        /* this is the axis which is getting resampled, 
           so the number of samples is potentially changing */
        sz[pi+1][permute[ai]] = (info->kernel[ax[pi][ai]]
                                 ? info->samples[ax[pi][ai]]
                                 : sz[pi][ai]);
      } else {
        /* this axis is just a shuffled version of the
           previous axis; no resampling this pass.
           Note: this case also includes axes which aren't 
           getting resampled whatsoever */
        sz[pi+1][permute[ai]] = sz[pi][ai];
      }
    }
  }

  return;
}

/*
** _nrrdResampleMakeWeightIndex()
**
** _allocate_ and fill the arrays of indices and weights that are
** needed to process all the scanlines along a given axis; also
** be so kind as to set the sampling ratio (<1: downsampling,
** new sample spacing larger, >1: upsampling, new sample spacing smaller)
**
** returns "dotLen", the number of input samples which are required
** for resampling this axis, or 0 if there was an error.  Uses biff.
*/
int
_nrrdResampleMakeWeightIndex(nrrdResample_t **weightP,
                             int **indexP, double *ratioP,
                             const Nrrd *nin, const NrrdResampleInfo *info,
                             unsigned int ai) {
  char me[]="_nrrdResampleMakeWeightIndex", err[BIFF_STRLEN];
  int sizeIn, sizeOut, center, dotLen, halfLen, *index, base, idx;
  nrrdResample_t minIn, maxIn, minOut, maxOut, spcIn, spcOut,
    ratio, support, integral, pos, idxD, wght;
  nrrdResample_t *weight;
  double parm[NRRD_KERNEL_PARMS_NUM];

  int e, i;

  if (!(info->kernel[ai])) {
    sprintf(err, "%s: don't see a kernel for dimension %d", me, ai);
    biffAdd(NRRD, err); *weightP = NULL; *indexP = NULL; return 0;
  }

  center = _nrrdCenter(nin->axis[ai].center);
  sizeIn = nin->axis[ai].size;
  sizeOut = info->samples[ai];
  minIn = AIR_CAST(nrrdResample_t, nin->axis[ai].min);
  maxIn = AIR_CAST(nrrdResample_t, nin->axis[ai].max);
  minOut = AIR_CAST(nrrdResample_t, info->min[ai]);
  maxOut = AIR_CAST(nrrdResample_t, info->max[ai]);
  spcIn = NRRD_SPACING(center, minIn, maxIn, sizeIn);
  spcOut = NRRD_SPACING(center, minOut, maxOut, sizeOut);
  *ratioP = ratio = spcIn/spcOut;
  support = AIR_CAST(nrrdResample_t,
                     info->kernel[ai]->support(info->parm[ai]));
  integral = AIR_CAST(nrrdResample_t,
                      info->kernel[ai]->integral(info->parm[ai]));
  /*
  fprintf(stderr, 
          "!%s(%d): size{In,Out} = %d, %d, support = %f; ratio = %f\n", 
          me, d, sizeIn, sizeOut, support, ratio);
  */
  if (ratio > 1) {
    /* if upsampling, we need only as many samples as needed for
       interpolation with the given kernel */
    dotLen = (int)(2*ceil(support));
  } else {
    /* if downsampling, we need to use all the samples covered by
       the stretched out version of the kernel */
    if (info->cheap) {
      dotLen = (int)(2*ceil(support));
    } else {
      dotLen = (int)(2*ceil(support/ratio));
    }
  }
  /*
  fprintf(stderr, "!%s(%d): dotLen = %d\n", me, d, dotLen);
  */

  weight = (nrrdResample_t*)calloc(sizeOut*dotLen, sizeof(nrrdResample_t));
  index = (int*)calloc(sizeOut*dotLen, sizeof(int));
  if (!(weight && index)) {
    sprintf(err, "%s: can't allocate weight and index arrays", me);
    biffAdd(NRRD, err); *weightP = NULL; *indexP = NULL; return 0;
  }

  /* calculate sample locations and do first pass on indices */
  halfLen = dotLen/2;
  for (i=0; i<sizeOut; i++) {
    pos = AIR_CAST(nrrdResample_t,
                   NRRD_POS(center, minOut, maxOut, sizeOut, i));
    idxD = AIR_CAST(nrrdResample_t,
                    NRRD_IDX(center, minIn, maxIn, sizeIn, pos));
    base = (int)floor(idxD) - halfLen + 1;
    for (e=0; e<dotLen; e++) {
      index[e + dotLen*i] = base + e;
      weight[e + dotLen*i] = idxD - index[e + dotLen*i];
    }
    /* ********
    if (!i) {
      fprintf(stderr, "%s: sample locations:\n", me);
    }
    fprintf(stderr, "%s: %d (sample locations)\n        ", me, i);
    for (e=0; e<dotLen; e++) {
      fprintf(stderr, "%d/%g ", index[e + dotLen*i], weight[e + dotLen*i]);
    }
    fprintf(stderr, "\n");
    ******** */
  }

  /*
  nrrdBoundaryPad,      1: fill with some user-specified value
  nrrdBoundaryBleed,    2: copy the last/first value out as needed
  nrrdBoundaryWrap,     3: wrap-around
  nrrdBoundaryWeight,   4: normalize the weighting on the existing samples;
                        ONLY sensible for a strictly positive kernel
                        which integrates to unity (as in blurring)
  */

  /* figure out what to do with the out-of-range indices */
  for (i=0; i<dotLen*sizeOut; i++) {
    idx = index[i];
    if (!AIR_IN_CL(0, idx, sizeIn-1)) {
      switch(info->boundary) {
      case nrrdBoundaryPad:
      case nrrdBoundaryWeight:  /* this will be further handled later */
        idx = sizeIn;
        break;
      case nrrdBoundaryBleed:
        idx = AIR_CLAMP(0, idx, sizeIn-1);
        break;
      case nrrdBoundaryWrap:
        idx = AIR_MOD(idx, sizeIn);
        break;
      default:
        sprintf(err, "%s: boundary behavior %d unknown/unimplemented", 
                me, info->boundary);
        biffAdd(NRRD, err); *weightP = NULL; *indexP = NULL; return 0;
      }
      index[i] = idx;
    }
  }

  /* run the sample locations through the chosen kernel.  We play a 
     sneaky trick on the kernel parameter 0 in case of downsampling
     to create the blurring of the old index space, but only if !cheap */
  memcpy(parm, info->parm[ai], NRRD_KERNEL_PARMS_NUM*sizeof(double));
  if (ratio < 1 && !(info->cheap)) {
    parm[0] /= ratio;
  }
  info->kernel[ai]->EVALN(weight, weight, dotLen*sizeOut, parm);

  /* ********
  for (i=0; i<sizeOut; i++) {
    fprintf(stderr, "%s: %d (sample weights)\n        ", me, i);
    for (e=0; e<dotLen; e++) {
      fprintf(stderr, "%d/%g ", index[e + dotLen*i], weight[e + dotLen*i]);
    }
    fprintf(stderr, "\n");
  }
  ******** */

  if (nrrdBoundaryWeight == info->boundary) {
    if (integral) {
      /* above, we set to sizeIn all the indices that were out of 
         range.  We now use that to determine the sum of the weights
         for the indices that were in-range */
      for (i=0; i<sizeOut; i++) {
        wght = 0;
        for (e=0; e<dotLen; e++) {
          if (sizeIn != index[e + dotLen*i]) {
            wght += weight[e + dotLen*i];
          }
        }
        for (e=0; e<dotLen; e++) {
          idx = index[e + dotLen*i];
          if (sizeIn != idx) {
            weight[e + dotLen*i] *= integral/wght;
          } else {
            weight[e + dotLen*i] = 0;
          }
        }
      }
    }
  } else {
    /* try to remove ripple/grating on downsampling */
    /* if (ratio < 1 && info->renormalize && integral) { */
    if (info->renormalize && integral) {
      for (i=0; i<sizeOut; i++) {
        wght = 0;
        for (e=0; e<dotLen; e++) {
          wght += weight[e + dotLen*i];
        }
        if (wght) {
          for (e=0; e<dotLen; e++) {
            /* this used to normalize the weights so that they summed
               to integral ("*= integral/wght"), which meant that if
               you use a very truncated Gaussian, then your over-all
               image brightness goes down.  This seems very contrary
               to the whole point of renormalization. */
            weight[e + dotLen*i] *= AIR_CAST(nrrdResample_t, 1.0/wght);
          }
        }
      }
    }
  }
  /* ********
  fprintf(stderr, "%s: sample weights:\n", me);
  for (i=0; i<sizeOut; i++) {
    fprintf(stderr, "%s: %d\n        ", me, i);
    wght = 0;
    for (e=0; e<dotLen; e++) {
      fprintf(stderr, "%d/%g ", index[e + dotLen*i], weight[e + dotLen*i]);
      wght += weight[e + dotLen*i];
    }
    fprintf(stderr, " (sum = %g)\n", wght);
  }
  ******** */

  *weightP = weight;
  *indexP = index;
  /*
  fprintf(stderr, "!%s: dotLen = %d\n", me, dotLen);
  */
  return dotLen;
}

/*
******** nrrdSpatialResample()
**
** general-purpose array-resampler: resamples a nrrd of any type
** (except block) and any dimension along any or all of its axes, with
** any combination of up- or down-sampling along the axes, with any
** kernel (specified by callback), with potentially a different kernel
** for each axis.  Whether or not to resample along axis d is
** controlled by the non-NULL-ity of info->kernel[ai].  Where to sample
** on the axis is controlled by info->min[ai] and info->max[ai]; these
** specify a range of "positions" aka "world space" positions, as 
** determined by the per-axis min and max of the input nrrd, which must
** be set for every resampled axis.
** 
** we cyclically permute those axes being resampled, and never touch
** the position (in axis ordering) of axes along which we are not
** resampling.  This strategy is certainly not the most intelligent
** one possible, but it does mean that the axis along which we're
** currently resampling-- the one along which we'll have to look at
** multiple adjecent samples-- is that resampling axis which is
** currently most contiguous in memory.  It may make sense to precede
** the resampling with an axis permutation which bubbles all the
** resampled axes to the front (most contiguous) end of the axis list,
** and then puts them back in place afterwards, depending on the cost
** of such axis permutation overhead.
*/
int
nrrdSpatialResample(Nrrd *nout, const Nrrd *nin,
                    const NrrdResampleInfo *info) {
  char me[]="nrrdSpatialResample", func[]="resample", err[BIFF_STRLEN];
  nrrdResample_t
    *array[NRRD_DIM_MAX],      /* intermediate copies of the input data
                                  undergoing resampling; we don't need a full-
                                  fledged nrrd for these.  Only about two of
                                  these arrays will be allocated at a time;
                                  intermediate results will be free()d when not
                                  needed */
    *_inVec,                   /* current input vector being resampled;
                                  not necessarily contiguous in memory
                                  (if strideIn != 1) */
    *inVec,                    /* buffer for input vector; contiguous */
    *_outVec,                  /* output vector in context of volume;
                                  never contiguous */
    tmpF;
  double ratio,                /* factor by which or up or downsampled */
    ratios[NRRD_DIM_MAX];      /* record of "ratio" for all resampled axes,
                                  used to compute new spacing in output */

  Nrrd *floatNin;              /* if the input nrrd type is not nrrdResample_t,
                                  then we convert it and keep it here */
  unsigned int ai,
    pi,                        /* current pass */
    topLax,
    permute[NRRD_DIM_MAX],     /* how to permute axes of last pass to get
                                  axes for current pass */
    ax[NRRD_DIM_MAX+1][NRRD_DIM_MAX],  /* axis ordering on each pass */
    passes;                    /* # of passes needed to resample all axes */
  int i, s, e,
    topRax,                    /* the lowest index of an axis which is
                                  resampled.  If all axes are being resampled,
                                  then this is 0.  If for some reason the
                                  "x" axis (fastest stride) is not being
                                  resampled, but "y" is, then topRax is 1 */
    botRax,                    /* index of highest axis being resampled */
    typeIn, typeOut;           /* types of input and output of resampling */
  size_t sz[NRRD_DIM_MAX+1][NRRD_DIM_MAX];
                               /* how many samples along each
                                  axis, changing on each pass */

  /* all these variables have to do with the spacing of elements in
     memory for the current pass of resampling, and they (except
     strideIn) are re-set at the beginning of each pass */
  nrrdResample_t
    *weight;                  /* sample weights */
  unsigned int ci[NRRD_DIM_MAX+1],
    co[NRRD_DIM_MAX+1];
  int 
    sizeIn, sizeOut,          /* lengths of input and output vectors */
    dotLen,                   /* # input samples to dot with weights to get
                                 one output sample */
    doRound,                  /* actually do rounding on output: we DO NOT
                                 round when info->round but the output 
                                 type is not integral */
    *index;                   /* dotLen*sizeOut 2D array of input indices */
  size_t 
    I,                        /* swiss-army int */
    strideIn,                 /* the stride between samples in the input
                                 "scanline" being resampled */
    strideOut,                /* stride between samples in output 
                                 "scanline" from resampling */
    L, LI, LO, numLines,      /* top secret */
    numOut;                   /* # of _samples_, total, in output volume;
                                 this is for allocating the output */
  airArray *mop;              /* for cleaning up */
  
  if (!(nout && nin && info)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdBoundaryUnknown == info->boundary) {
    sprintf(err, "%s: need to specify a boundary behavior", me);
    biffAdd(NRRD, err); return 1;
  }

  typeIn = nin->type;
  typeOut = nrrdTypeDefault == info->type ? typeIn : info->type;

  if (_nrrdResampleCheckInfo(nin, info)) {
    sprintf(err, "%s: problem with arguments", me);
    biffAdd(NRRD, err); return 1;
  }
  
  _nrrdResampleComputePermute(permute, ax, sz,
                              &topRax, &botRax, &passes,
                              nin, info);
  topLax = topRax ? 0 : 1;

  /* not sure where else to put this:
     (want to put it before 0 == passes branch)
     We have to assume some centering when doing resampling, and it would
     be stupid to not record it in the outgoing nrrd, since the value of
     nrrdDefaultCenter could always change. */
  for (ai=0; ai<nin->dim; ai++) {
    if (info->kernel[ai]) {
      nout->axis[ai].center = _nrrdCenter(nin->axis[ai].center);
    }
  }

  if (0 == passes) {
    /* actually, no resampling was desired.  Copy input to output,
       but with the clamping that we normally do at the end of resampling */
    nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, sz[0]);
    if (nrrdMaybeAlloc_nva(nout, typeOut, nin->dim, sz[0])) {
      sprintf(err, "%s: couldn't allocate output", me);
      biffAdd(NRRD, err); return 1;
    }
    numOut = nrrdElementNumber(nout);
    for (I=0; I<numOut; I++) {
      tmpF = nrrdFLookup[nin->type](nin->data, I);
      tmpF = nrrdFClamp[typeOut](tmpF);
      nrrdFInsert[typeOut](nout->data, I, tmpF);
    }
    nrrdAxisInfoCopy(nout, nin, NULL, NRRD_AXIS_INFO_NONE);
    /* HEY: need to create textual representation of resampling parameters */
    if (nrrdContentSet_va(nout, func, nin, "")) {
      sprintf(err, "%s:", me);
      biffAdd(NRRD, err); return 1;
    }
    if (nrrdBasicInfoCopy(nout, nin,
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
      biffAdd(NRRD, err); return 1;
    }
    return 0;
  }

  mop = airMopNew();
  /* convert input nrrd to nrrdResample_t if necessary */
  if (nrrdResample_nrrdType != typeIn) {
    if (nrrdConvert(floatNin = nrrdNew(), nin, nrrdResample_nrrdType)) {
      sprintf(err, "%s: couldn't create float copy of input", me);
      biffAdd(NRRD, err); airMopError(mop); return 1;
    }
    array[0] = (nrrdResample_t*)floatNin->data;
    airMopAdd(mop, floatNin, (airMopper)nrrdNuke, airMopAlways);
  } else {
    floatNin = NULL;
    array[0] = (nrrdResample_t*)nin->data;
  }
  
  /* compute strideIn; this is actually the same for every pass
     because (strictly speaking) in every pass we are resampling
     the same axis, and axes with lower indices are constant length */
  strideIn = 1;
  for (ai=0; ai<(unsigned int)topRax; ai++) { /* HEY scrutinize casts */
    strideIn *= nin->axis[ai].size;
  }
  /*
  printf("%s: strideIn = " _AIR_SIZE_T_CNV "\n", me, strideIn);
  */

  /* go! */
  for (pi=0; pi<passes; pi++) {
    /*
    printf("%s: --- pass %d --- \n", me, pi);
    */
    numLines = strideOut = 1;
    for (ai=0; ai<nin->dim; ai++) {
      if (ai < (unsigned int)botRax) {   /* HEY scrutinize cast */
        strideOut *= sz[pi+1][ai];
      }
      if (ai != (unsigned int)topRax) {  /* HEY scrutinize cast */
        numLines *= sz[pi][ai];
      }
    }
    sizeIn = sz[pi][topRax];
    sizeOut = sz[pi+1][botRax];
    numOut = numLines*sizeOut;
    /* for the rest of the loop body, d is the original "dimension"
       for the axis being resampled */
    ai = ax[pi][topRax];
    /*
    printf("%s(%d): numOut = " _AIR_SIZE_T_CNV "\n", me, pi, numOut);
    printf("%s(%d): numLines = " _AIR_SIZE_T_CNV "\n", me, pi, numLines);
    printf("%s(%d): stride: In=%d, Out=%d\n", me, pi, 
           (int)strideIn, (int)strideOut);
    printf("%s(%d): sizeIn = %d\n", me, pi, sizeIn);
    printf("%s(%d): sizeOut = %d\n", me, pi, sizeOut);
    */

    /* we can free the input to the previous pass 
       (if its not the given data) */
    if (pi > 0) {
      if (pi == 1) {
        if (array[0] != nin->data) {
          airMopSub(mop, floatNin, (airMopper)nrrdNuke);
          floatNin = nrrdNuke(floatNin);
          array[0] = NULL;
          /*
          printf("%s: pi %d: freeing array[0]\n", me, pi);
          */
        }
      } else {
        airMopSub(mop, array[pi-1], airFree);
        array[pi-1] = (nrrdResample_t*)airFree(array[pi-1]);
        /*
        printf("%s: pi %d: freeing array[%d]\n", me, pi, pi-1);
        */
      }
    }

    /* allocate output volume */
    array[pi+1] = (nrrdResample_t*)calloc(numOut, sizeof(nrrdResample_t));
    if (!array[pi+1]) {
      sprintf(err, "%s: couldn't create array of " _AIR_SIZE_T_CNV 
              " nrrdResample_t's for output of pass %d",
              me, numOut, pi);
      biffAdd(NRRD, err); airMopError(mop); return 1;
    }
    airMopAdd(mop, array[pi+1], airFree, airMopAlways);
    /*
    printf("%s: allocated array[%d]\n", me, pi+1);
    */

    /* allocate contiguous input scanline buffer, we alloc one more
       than needed to provide a place for the pad value.  That is, in
       fact, the over-riding reason to copy a scanline to a local
       array: so that there is a simple consistent (non-branchy) way
       to incorporate the pad values */
    inVec = (nrrdResample_t *)calloc(sizeIn+1, sizeof(nrrdResample_t));
    airMopAdd(mop, inVec, airFree, airMopAlways);
    inVec[sizeIn] = AIR_CAST(nrrdResample_t, info->padValue);

    dotLen = _nrrdResampleMakeWeightIndex(&weight, &index, &ratio,
                                          nin, info, ai);
    if (!dotLen) {
      sprintf(err, "%s: trouble creating weight and index vector arrays", me);
      biffAdd(NRRD, err); airMopError(mop); return 1;
    }
    ratios[ai] = ratio;
    airMopAdd(mop, weight, airFree, airMopAlways);
    airMopAdd(mop, index, airFree, airMopAlways);

    /* the skinny: resample all the scanlines */
    _inVec = array[pi];
    _outVec = array[pi+1];
    memset(ci, 0, (NRRD_DIM_MAX+1)*sizeof(int));
    memset(co, 0, (NRRD_DIM_MAX+1)*sizeof(int));
    for (L=0; L<numLines; L++) {
      /* calculate the index to get to input and output scanlines,
         according the coordinates of the start of the scanline */
      NRRD_INDEX_GEN(LI, ci, sz[pi], nin->dim);
      NRRD_INDEX_GEN(LO, co, sz[pi+1], nin->dim);
      _inVec = array[pi] + LI;
      _outVec = array[pi+1] + LO;
      
      /* read input scanline into contiguous array */
      for (i=0; i<sizeIn; i++) {
        inVec[i] = _inVec[i*strideIn];
      }

      /* do the weighting */
      for (i=0; i<sizeOut; i++) {
        tmpF = 0.0;
        /*
        fprintf(stderr, "%s: i = %d (tmpF=0)\n", me, (int)i);
        */
        for (s=0; s<dotLen; s++) {
          tmpF += inVec[index[s + dotLen*i]]*weight[s + dotLen*i];
          /*
          fprintf(stderr, "  tmpF += %g*%g == %g\n",
                  inVec[index[s + dotLen*i]], weight[s + dotLen*i], tmpF);
          */
        }
        _outVec[i*strideOut] = tmpF;
        /*
        fprintf(stderr, "--> out[%d] = %g\n",
                i*strideOut, _outVec[i*strideOut]);
        */
      }
 
      /* update the coordinates for the scanline starts.  We don't
         use the usual NRRD_COORD macros because we're subject to
         the unusual constraint that ci[topRax] and co[permute[topRax]]
         must stay exactly zero */
      e = topLax;
      ci[e]++; 
      co[permute[e]]++;
      while (L < numLines-1 && ci[e] == sz[pi][e]) {
        ci[e] = co[permute[e]] = 0;
        e++;
        e += e == topRax;
        ci[e]++; 
        co[permute[e]]++;
      }
    }

    /* pass-specific clean up */
    airMopSub(mop, weight, airFree);
    airMopSub(mop, index, airFree);
    airMopSub(mop, inVec, airFree);
    weight = (nrrdResample_t*)airFree(weight);
    index = (int*)airFree(index);
    inVec = (nrrdResample_t*)airFree(inVec);
  }

  /* clean up second-to-last array and scanline buffers */
  if (passes > 1) {
    airMopSub(mop, array[passes-1], airFree);
    array[passes-1] = (nrrdResample_t*)airFree(array[passes-1]);
    /*
    printf("%s: now freeing array[%d]\n", me, passes-1);
    */
  } else if (array[passes-1] != nin->data) {
    airMopSub(mop, floatNin, (airMopper)nrrdNuke);
    floatNin = nrrdNuke(floatNin);
  }
  array[passes-1] = NULL;
  
  /* create output nrrd and set axis info */
  if (nrrdMaybeAlloc_nva(nout, typeOut, nin->dim, sz[passes])) {
    sprintf(err, "%s: couldn't allocate final output nrrd", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopOnError);
  nrrdAxisInfoCopy(nout, nin, NULL, 
                   (NRRD_AXIS_INFO_SIZE_BIT
                    | NRRD_AXIS_INFO_MIN_BIT
                    | NRRD_AXIS_INFO_MAX_BIT
                    | NRRD_AXIS_INFO_SPACING_BIT
                    | NRRD_AXIS_INFO_SPACEDIRECTION_BIT  /* see below */
                    | NRRD_AXIS_INFO_THICKNESS_BIT
                    | NRRD_AXIS_INFO_KIND_BIT));
  for (ai=0; ai<nin->dim; ai++) {
    if (info->kernel[ai]) {
      /* we do resample this axis */
      nout->axis[ai].spacing = nin->axis[ai].spacing/ratios[ai];
      /* no way to usefully update thickness: we could be doing blurring
         but maintaining the number of samples: thickness increases, or
         we could be downsampling, in which the relationship between the
         sampled and the skipped regions of space becomes complicated:
         no single scalar can represent it, or we could be upsampling,
         in which the notion of "skip" could be rendered meaningless */
      nout->axis[ai].thickness = AIR_NAN;
      nout->axis[ai].min = info->min[ai];
      nout->axis[ai].max = info->max[ai];
      /*
        HEY: this is currently a bug: all this code was written long
        before there were space directions, so min/max are always 
        set, regardless of whethere there are incoming space directions
        which then disallows output space directions on the same axes
      _nrrdSpaceVecScale(nout->axis[ai].spaceDirection,
                         1.0/ratios[ai], nin->axis[ai].spaceDirection);
      */
      nout->axis[ai].kind = _nrrdKindAltered(nin->axis[ai].kind, AIR_TRUE);
    } else {
      /* this axis remains untouched */
      nout->axis[ai].min = nin->axis[ai].min;
      nout->axis[ai].max = nin->axis[ai].max;
      nout->axis[ai].spacing = nin->axis[ai].spacing;
      nout->axis[ai].thickness = nin->axis[ai].thickness;
      nout->axis[ai].kind = nin->axis[ai].kind;
    }
  }
  /* HEY: need to create textual representation of resampling parameters */
  if (nrrdContentSet_va(nout, func, nin, "")) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }

  /* copy the resampling final result into the output nrrd, maybe
     rounding as we go to make sure that 254.9999 is saved as 255
     in uchar output, and maybe clamping as we go to insure that
     integral results don't have unexpected wrap-around. */
  if (info->round) {
    if (nrrdTypeInt == typeOut ||
        nrrdTypeUInt == typeOut ||
        nrrdTypeLLong == typeOut ||
        nrrdTypeULLong == typeOut) {
      fprintf(stderr, "%s: WARNING: possible erroneous output with "
              "rounding of %s output type due to int-based implementation "
              "of rounding\n", me, airEnumStr(nrrdType, typeOut));
    }
    doRound = nrrdTypeIsIntegral[typeOut];
  } else {
    doRound = AIR_FALSE;
  }
  numOut = nrrdElementNumber(nout);
  for (I=0; I<numOut; I++) {
    tmpF = array[passes][I];
    if (doRound) {
      tmpF = AIR_CAST(nrrdResample_t, AIR_ROUNDUP(tmpF));
    }
    if (info->clamp) {
      tmpF = nrrdFClamp[typeOut](tmpF);
    }
    nrrdFInsert[typeOut](nout->data, I, tmpF);
  }

  if (nrrdBasicInfoCopy(nout, nin,
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
    biffAdd(NRRD, err); return 1;
  }

  /* enough already */
  airMopOkay(mop);
  return 0;
}
