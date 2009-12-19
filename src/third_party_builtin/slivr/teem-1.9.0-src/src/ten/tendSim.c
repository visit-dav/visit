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

#define INFO "Simulate DW images from a tensor field"
char *_tend_simInfoL =
  (INFO
   ".  The output will be in the same form as the input to \"tend estim\", "
   ", and the B matrix used is the output from \"tend bmat\". "
   "The other required inputs are (\"-r\") the reference "
   "(non-diffusion-weighted) anatomical scalar volume, and (\"-i\") "
   "the corresponding volume of diffusion "
   "tensors. ");

int
tend_simMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  tenEstimateContext *tec;
  airArray *mop;

  int E, oldstuff, seed;
  Nrrd *nin, *nT2, *nbmat, *nout;
  char *outS;
  float b, sigma;

  hestOptAdd(&hopt, "old", NULL, airTypeInt, 0, 0, &oldstuff, NULL,
             "don't use the new tenEstimateContext functionality");
  hestOptAdd(&hopt, "sigma", "sigma", airTypeFloat, 1, 1, &sigma, "0.0",
             "Rician noise parameter");
  hestOptAdd(&hopt, "seed", "seed", airTypeInt, 1, 1, &seed, "42",
             "seed value for RNG which creates noise");
  hestOptAdd(&hopt, "B", "B matrix", airTypeOther, 1, 1, &nbmat, NULL,
             "B matrix, one row per diffusion-weighted image", 
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "r", "reference field", airTypeOther, 1, 1, &nT2, "-",
             "reference anatomical scan, with no diffusion weighting",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "i", "tensor field", airTypeOther, 1, 1, &nin, "-",
             "input diffusion tensor field", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "b", "b", airTypeFloat, 1, 1, &b, "1",
             "b value for simulated scan");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output image (floating point)");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_simInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  
  if (!oldstuff) {
    airSrandMT(seed);
    tec = tenEstimateContextNew();
    airMopAdd(mop, tec, (airMopper)tenEstimateContextNix, airMopAlways);
    E = 0;
    if (!E) E |= tenEstimateMethodSet(tec, tenEstimateMethodLLS);
    if (!E) E |= tenEstimateValueMinSet(tec, 0.0001);
    if (!E) E |= tenEstimateBMatricesSet(tec, nbmat, b, AIR_TRUE);
    if (!E) E |= tenEstimateThresholdSet(tec, 0, 0);
    if (!E) E |= tenEstimateUpdate(tec);
    if (!E) E |= tenEstimate1TensorSimulateVolume(tec, 
                                                  nout, sigma, b, 
                                                  nT2, nin,
                                                  nrrdTypeFloat);
    if (E) {
      airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble making DWI volume (new):\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  } else {
    if (tenSimulate(nout, nT2, nin, nbmat, b)) {
      airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble making DWI volume:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  }
  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
/* TEND_CMD(sim, INFO); */
unrrduCmd tend_simCmd = { "sim", INFO, tend_simMain };
