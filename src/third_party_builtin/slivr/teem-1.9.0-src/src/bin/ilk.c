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

#include <teem/unrrdu.h>
#include <teem/moss.h>

char *ilkInfo = ("(I)mage (L)inear Trans(X-->K)forms. "
                 "Applies linear (homogenous coordinate) transforms "
                 "to a given image, using the given kernel for "
                 "resampling. ");

int
main(int argc, char *argv[]) {
  char *me, *errS, *outS;
  hestOpt *hopt=NULL;
  hestParm *hparm;
  airArray *mop;
  Nrrd *nin, *nout;
  NrrdKernelSpec *ksp;
  mossSampler *msp;
  double mat[6], **matList, *origInfo, origMat[6], origInvMat[6], ox, oy,
    min[2], max[2];
  int d, bound, ax0, size[2];
  unsigned int matListLen, _bkgLen, i;
  float *bkg, *_bkg, scale[4];
  
  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hparm->elideSingleEnumType = AIR_TRUE;
  hparm->elideSingleOtherType = AIR_TRUE;
  hparm->elideSingleOtherDefault = AIR_FALSE;
  hparm->elideMultipleNonExistFloatDefault = AIR_TRUE;
  hparm->respFileEnable = AIR_TRUE;
  
  hestOptAdd(&hopt, "i", "image", airTypeOther, 1, 1, &nin, "-",
             "input image", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "0", "origin", airTypeOther, 1, 1, &origInfo, "p:0,0",
             "where to location (0,0) prior to applying transforms.\n "
             "\b\bo \"u:<float>,<float>\" locate origin in a unit box "
             "[0,1]x[0,1] which covers the original image\n "
             "\b\bo \"p:<float>,<float>\" locate origin at a particular "
             "pixel location, in the index space of the image",
             NULL, NULL, mossHestOrigin);
  hestOptAdd(&hopt, "t", "xform0", airTypeOther, 1, -1, &matList, NULL,
             "transform(s) to apply to image.  Transforms "
             "are applied in the order in which they appear.\n "
             "\b\bo \"identity\": no geometric transform, just resampling\n "
             "\b\bo \"translate:x,y\": shift image by vector (x,y), as "
             "measured in pixels\n "
             "\b\bo \"rotate:ang\": rotate CCW by ang degrees\n "
             "\b\bo \"scale:xs,ys\": scale by xs in X, and ys in Y\n "
             "\b\bo \"shear:fix,amnt\": shear by amnt, keeping fixed "
             "the pixels along a direction <fix> degrees from the X axis\n "
             "\b\bo \"flip:ang\": flip along axis an angle <ang> degrees from "
             "the X axis\n "
             "\b\bo \"a,b,tx,c,d,ty\": specify the transform explicitly "
             "in row-major order (opposite of PostScript) ",
             &matListLen, NULL, mossHestTransform);
  hestOptAdd(&hopt, "k", "kernel", airTypeOther, 1, 1, &ksp,
             "cubic:0,0.5", "reconstruction kernel",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "min", "xMin yMin", airTypeDouble, 2, 2, min, "nan nan",
             "lower bounding corner of output image. Default (by not "
             "using this option) is the lower corner of input image. ");
  hestOptAdd(&hopt, "max", "xMax yMax", airTypeDouble, 2, 2, max, "nan nan",
             "upper bounding corner of output image. Default (by not "
             "using this option) is the upper corner of input image. ");
  hestOptAdd(&hopt, "b", "boundary", airTypeEnum, 1, 1, &bound, "bleed",
             "what to do when sampling outside original image.\n "
             "\b\bo \"bleed\": copy values at image border outward\n "
             "\b\bo \"wrap\": do wrap-around on image locations\n "
             "\b\bo \"pad\": use a given background value (via \"-bg\")",
             NULL, nrrdBoundary);
  hestOptAdd(&hopt, "bg", "bg0 bg1", airTypeFloat, 1, -1, &_bkg, "nan",
             "background color to use with boundary behavior \"pad\". "
             "Defaults to all zeroes.",
             &_bkgLen);
  hestOptAdd(&hopt, "s", "xSize ySize", airTypeOther, 2, 2, scale, "x1 x1",
             "For each axis, information about how many samples in output:\n "
             "\b\bo \"x<float>\": number of output samples is some scaling of "
             " the number input samples; multiplied by <float>\n "
             "\b\bo \"<int>\": specify exact number of samples",
             NULL, NULL, &unrrduHestScaleCB);
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &outS, "-",
             "file to write output nrrd to");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, ilkInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  msp = mossSamplerNew();
  airMopAdd(mop, msp, (airMopper)mossSamplerNix, airMopAlways);
  msp->boundary = bound;
  if (mossSamplerKernelSet(msp, ksp->kernel, ksp->parm)) {
    fprintf(stderr, "%s: trouble with sampler:\n%s\n",
            me, errS = biffGetDone(MOSS)); free(errS);
    airMopError(mop); return 1;
  }
  if (nrrdBoundaryPad == bound) {
    if (_bkgLen != MOSS_NCOL(nin)) {
      fprintf(stderr, "%s: got %d background colors, image has "
              _AIR_SIZE_T_CNV " colors\n", 
              me, _bkgLen, MOSS_NCOL(nin));
      airMopError(mop); return 1;
    } else {
      bkg = _bkg;
    }
  } else {
    /* maybe warn user if they gave a background that won't be used? */
    /* No- because hest is stupid, and right now we always parse the
       single (default) "nan" for this argument! */
    bkg = NULL;
  }

  ax0 = MOSS_AXIS0(nin);
  if (!( AIR_EXISTS(nin->axis[ax0+0].min)
         && AIR_EXISTS(nin->axis[ax0+0].max))) {
    nrrdAxisInfoMinMaxSet(nin, ax0+0, mossDefCenter);
  }
  if (!( AIR_EXISTS(nin->axis[ax0+1].min)
         && AIR_EXISTS(nin->axis[ax0+1].max))) {
    nrrdAxisInfoMinMaxSet(nin, ax0+1, mossDefCenter);
  }
  min[0] = AIR_EXISTS(min[0]) ? min[0] : nin->axis[ax0+0].min;
  max[0] = AIR_EXISTS(max[0]) ? max[0] : nin->axis[ax0+0].max;
  min[1] = AIR_EXISTS(min[1]) ? min[1] : nin->axis[ax0+1].min;
  max[1] = AIR_EXISTS(max[1]) ? max[1] : nin->axis[ax0+1].max;
  for (d=0; d<2; d++) {
    switch((int)scale[0 + 2*d]) {
    case 0:
      /* same number of samples as input */
      size[d] = nin->axis[ax0+d].size;
      break;
    case 1:
      /* scaling of input # samples */
      size[d] = (int)(scale[1 + 2*d]*nin->axis[ax0+d].size);
      break;
    case 2:
      /* explicit # of samples */
      size[d] = (int)(scale[1 + 2*d]);
      break;
    }
  }

  /* find origin-based pre- and post- translate */
  if (0 == origInfo[0]) {
    /* absolute pixel position */
    mossMatTranslateSet(origMat, -origInfo[1], -origInfo[2]);
  } else {
    /* in unit box [0,1]x[0,1] */
    ox = AIR_AFFINE(0.0, origInfo[1], 1.0,
                    nin->axis[ax0+0].min, nin->axis[ax0+0].max);
    oy = AIR_AFFINE(0.0, origInfo[2], 1.0, 
                    nin->axis[ax0+1].min, nin->axis[ax0+1].max);
    mossMatTranslateSet(origMat, -ox, -oy);
  }
  mossMatInvert(origInvMat, origMat);

  /* form complete transform */
  mossMatIdentitySet(mat);
  mossMatLeftMultiply(mat, origMat);
  for (i=0; i<matListLen; i++) {
    mossMatLeftMultiply(mat, matList[i]);
  }
  mossMatLeftMultiply(mat, origInvMat);

  if (!AIR_EXISTS(nin->axis[ax0+0].min) || !AIR_EXISTS(nin->axis[ax0+0].max)) {
    nrrdAxisInfoMinMaxSet(nin, ax0+0, mossDefCenter);
  }
  if (!AIR_EXISTS(nin->axis[ax0+1].min) || !AIR_EXISTS(nin->axis[ax0+1].max)) {
    nrrdAxisInfoMinMaxSet(nin, ax0+1, mossDefCenter);
  }
  if (mossLinearTransform(nout, nin, bkg,
                          mat, msp,
                          min[0], max[0], min[1], max[1],
                          size[0], size[1])) {
    fprintf(stderr, "%s: problem doing transform:\n%s\n",
            me, errS = biffGetDone(MOSS)); free(errS);
    airMopError(mop); return 1;
  }

  if (nrrdSave(outS, nout, NULL)) {
    fprintf(stderr, "%s: problem saving output:\n%s\n",
            me, errS = biffGetDone(NRRD)); free(errS);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  exit(0);
}
