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

#include "unrrdu.h"
#include "privateUnrrdu.h"

#define INFO "Filtering and {up,down}sampling with a seperable kernel"
char *_unrrdu_resampleInfoL =
(INFO
 ". Provides simplified access to nrrdSpatialResample() "
 "by assuming (among other things) that the same kernel "
 "is used for resampling "
 "every axis (every axis which is being resampled), and "
 "by assuming that the whole axis is being resampled "
 "(no cropping or padding).  Chances are, you should "
 "use defaults for \"-b\" and \"-v\" and worry only "
 "about the \"-s\" and \"-k\" options.  This resampling "
 "respects the difference between cell- and "
 "node-centered data.");

int
unrrdu_resampleMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  int type, bb, pret, norenorm, older, E, defaultCenter, verbose;
  unsigned int scaleLen, ai, samplesOut;
  airArray *mop;
  float *scale;
  double padVal;
  NrrdResampleInfo *info;
  NrrdResampleContext *rsmc;
  NrrdKernelSpec *unuk;

  mop = airMopNew();
  info = nrrdResampleInfoNew();
  airMopAdd(mop, info, (airMopper)nrrdResampleInfoNix, airMopAlways);
  hparm->elideSingleOtherDefault = AIR_FALSE;
  hestOptAdd(&opt, "old", NULL, airTypeInt, 0, 0, &older, NULL,
             "instead of using the new nrrdResampleContext implementation, "
             "use the old nrrdSpatialResample implementation");
  hestOptAdd(&opt, "s,size", "sz0", airTypeOther, 1, -1, &scale, NULL,
             "For each axis, information about how many samples in output:\n "
             "\b\bo \"=\": leave this axis completely untouched: no "
             "resampling whatsoever\n "
             "\b\bo \"x<float>\": multiply the number of input samples by "
             "<float>, and round to the nearest integer, to get the number "
             "of output samples.  Use \"x1\" to resample the axis but leave "
             "the number of samples unchanged\n "
             "\b\bo \"<int>\": exact number of output samples",
             &scaleLen, NULL, &unrrduHestScaleCB);
  hestOptAdd(&opt, "k,kernel", "kern", airTypeOther, 1, 1, &unuk,
             "cubic:0,0.5",
             "The kernel to use for resampling.  Possibilities include:\n "
             "\b\bo \"box\": nearest neighbor interpolation\n "
             "\b\bo \"tent\": linear interpolation\n "
             "\b\bo \"cubic:B,C\": Mitchell/Netravali BC-family of "
             "cubics:\n "
             "\t\t\"cubic:1,0\": B-spline; maximal blurring\n "
             "\t\t\"cubic:0,0.5\": Catmull-Rom; good interpolating kernel\n "
             "\b\bo \"quartic:A\": 1-parameter family of "
             "interpolating quartics (\"quartic:0.0834\" is most accurate)\n "
             "\b\bo \"hann:R\": Hann (cosine bell) windowed sinc, radius R\n "
             "\b\bo \"black:R\": Blackman windowed sinc, radius R\n "
             "\b\bo \"gauss:S,C\": Gaussian blurring, with standard deviation "
             "S and cut-off at C standard deviations",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&opt, "nrn", NULL, airTypeInt, 0, 0, &norenorm, NULL,
             "don't do per-pass kernel weight renormalization. "
             "Doing the renormalization is not a big performance hit, and "
             "is sometimes needed to avoid \"grating\" on non-integral "
             "down-sampling.  Disabling the renormalization is needed for "
             "correct results with artificially narrow kernels. ");
  hestOptAdd(&opt, "b,boundary", "behavior", airTypeEnum, 1, 1, &bb, "bleed",
             "How to handle samples beyond the input bounds:\n "
             "\b\bo \"pad\": use some specified value\n "
             "\b\bo \"bleed\": extend border values outward\n "
             "\b\bo \"wrap\": wrap-around to other side", 
             NULL, nrrdBoundary);
  hestOptAdd(&opt, "v,value", "value", airTypeDouble, 1, 1, &padVal, "0.0",
             "for \"pad\" boundary behavior, pad with this value");
  hestOptAdd(&opt, "t,type", "type", airTypeOther, 1, 1, &type, "default",
             "type to save OUTPUT as. By default (not using this option), "
             "the output type is the same as the input type",
             NULL, NULL, &unrrduHestMaybeTypeCB);
  hestOptAdd(&opt, "cheap", NULL, airTypeInt, 0, 0, &(info->cheap), NULL,
             "(only with \"-old\") "
             "when downsampling (reducing number of samples), don't "
             "try to do correct filtering by scaling kernel to match "
             "new (stretched) index space; keep it in old index space. "
             "When used in conjunction with \"-k box\", this can implement "
             "subsampling which chooses every Nth value. ");
  hestOptAdd(&opt, "c,center", "center", airTypeEnum, 1, 1, &defaultCenter,
             (nrrdCenterCell == nrrdDefaultCenter
              ? "cell"
              : "node"),
             "(not available with \"-old\") "
             "default centering of axes when input nrrd "
             "axes don't have a known centering: \"cell\" or \"node\" ",
             NULL, nrrdCenter);
  hestOptAdd(&opt, "verbose", NULL, airTypeInt, 0, 0, &verbose, NULL,
             "(not available with \"-old\") "
             "turn on verbosity ");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);
  
  USAGE(_unrrdu_resampleInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (scaleLen != nin->dim) {
    fprintf(stderr, "%s: # sampling sizes (%d) != input nrrd dimension (%d)\n",
            me, scaleLen, nin->dim);
    airMopError(mop);
    return 1;
  }
  if (!older) {
    rsmc = nrrdResampleContextNew();
    rsmc->verbose = verbose;
    airMopAdd(mop, rsmc, (airMopper)nrrdResampleContextNix, airMopAlways);
    E = AIR_FALSE;
    if (!E) E |= nrrdResampleDefaultCenterSet(rsmc, defaultCenter);
    if (!E) E |= nrrdResampleNrrdSet(rsmc, nin);
    for (ai=0; ai<nin->dim; ai++) {
      switch((int)scale[0 + 2*ai]) {
      case 0:
        /* no resampling */
        if (!E) E |= nrrdResampleKernelSet(rsmc, ai, NULL, NULL);
        break;
      case 1:
        /* scaling of input # samples */
        if (!E) E |= nrrdResampleKernelSet(rsmc, ai, unuk->kernel, unuk->parm);
        samplesOut = AIR_ROUNDUP(scale[1 + 2*ai]*nin->axis[ai].size);
        if (!E) E |= nrrdResampleSamplesSet(rsmc, ai, samplesOut);
        break;
      case 2:
        /* explicit # of samples */
        if (!E) E |= nrrdResampleKernelSet(rsmc, ai, unuk->kernel, unuk->parm);
        samplesOut = (size_t)scale[1 + 2*ai];
        if (!E) E |= nrrdResampleSamplesSet(rsmc, ai, samplesOut);
        break;
      }
      if (!E) E |= nrrdResampleRangeFullSet(rsmc, ai);
    }
    if (!E) E |= nrrdResampleBoundarySet(rsmc, bb);
    if (!E) E |= nrrdResampleTypeOutSet(rsmc, type);
    if (!E) E |= nrrdResamplePadValueSet(rsmc, padVal);
    if (!E) E |= nrrdResampleRenormalizeSet(rsmc, !norenorm);
    if (!E) E |= nrrdResampleExecute(rsmc, nout);
    if (E) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error resampling nrrd:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
  } else {
    for (ai=0; ai<nin->dim; ai++) {
      /* this may be over-written below */
      info->kernel[ai] = unuk->kernel;
      switch((int)scale[0 + 2*ai]) {
      case 0:
        /* no resampling */
        info->kernel[ai] = NULL;
        break;
      case 1:
        /* scaling of input # samples */
        info->samples[ai] = AIR_ROUNDUP(scale[1 + 2*ai]*nin->axis[ai].size);
        break;
      case 2:
        /* explicit # of samples */
        info->samples[ai] = (size_t)scale[1 + 2*ai];
        break;
      }
      memcpy(info->parm[ai], unuk->parm, NRRD_KERNEL_PARMS_NUM*sizeof(double));
      if (info->kernel[ai] &&
          (!( AIR_EXISTS(nin->axis[ai].min) 
              && AIR_EXISTS(nin->axis[ai].max))) ) {
        nrrdAxisInfoMinMaxSet(nin, ai, 
                              (nin->axis[ai].center
                               ? nin->axis[ai].center 
                               : nrrdDefaultCenter));
      }
      info->min[ai] = nin->axis[ai].min;
      info->max[ai] = nin->axis[ai].max;
    }
    info->boundary = bb;
    info->type = type;
    info->padValue = padVal;
    info->renormalize = !norenorm;
    if (nrrdSpatialResample(nout, nin, info)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error resampling nrrd:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
  }
  

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(resample, INFO);
