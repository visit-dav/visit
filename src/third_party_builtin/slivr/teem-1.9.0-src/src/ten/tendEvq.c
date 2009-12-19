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

#define INFO "Quantize directions of diffusion"
char *_tend_evqInfoL =
  (INFO
   ". Because VTK doesn't do multi-dimensional colormaps, we have to "
   "quantize directions of diffusion (usually just the principal eigenvector) "
   "in order to create the usual XYZ<->RGB coloring.  Because "
   "eigenvector directions are poorly defined in regions of low "
   "anisotropy, the length of the vector (pre-quantization) is modulated "
   "by anisotropy, requiring the selection of some anisotropy metric.");

int
tend_evqMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  int which, aniso, dontScaleByAniso;
  Nrrd *nin, *nout;
  char *outS;

  hestOptAdd(&hopt, "c", "evec index", airTypeInt, 1, 1, &which, "0",
             "Which eigenvector should be quantized: \"0\" for the "
             "direction of fastest diffusion (eigenvector associated "
             "with largest eigenvalue), \"1\" or \"2\" for other two "
             "eigenvectors (associated with middle and smallest eigenvalue)");
  hestOptAdd(&hopt, "a", "aniso", airTypeEnum, 1, 1, &aniso, NULL,
             "Which anisotropy metric to scale the eigenvector "
             "with.  " TEN_ANISO_DESC,
             NULL, tenAniso);
  hestOptAdd(&hopt, "ns", NULL, airTypeInt, 0, 0, &dontScaleByAniso, NULL,
             "Don't attenuate the color by anisotropy.  By default (not "
             "using this option), regions with low or no anisotropy are "
             "very dark colors or black");
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, "-",
             "input diffusion tensor volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output image (floating point)");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_evqInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (tenEvqVolume(nout, nin, which, aniso, !dontScaleByAniso)) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble quantizing eigenvectors:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
TEND_CMD(evq, INFO);
