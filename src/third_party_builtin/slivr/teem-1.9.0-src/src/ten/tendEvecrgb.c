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

#define INFO "Make an RGB volume from an eigenvector and an anisotropy"
char *_tend_evecrgbInfoL =
  (INFO
   ". ");

int
tend_evecrgbMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  tenEvecRGBParm *rgbp;
  Nrrd *nin, *nout;
  char *outS;

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);

  rgbp = tenEvecRGBParmNew();
  airMopAdd(mop, rgbp, AIR_CAST(airMopper, tenEvecRGBParmNix), airMopAlways);
  
  hestOptAdd(&hopt, "c", "evec index", airTypeUInt, 1, 1, &(rgbp->which), NULL,
             "which eigenvector will be colored. \"0\" for the "
             "principal, \"1\" for the middle, \"2\" for the minor");
  hestOptAdd(&hopt, "a", "aniso", airTypeEnum, 1, 1, &(rgbp->aniso), NULL,
             "Which anisotropy to use for modulating the saturation "
             "of the colors.  " TEN_ANISO_DESC,
             NULL, tenAniso);
  hestOptAdd(&hopt, "t", "thresh", airTypeDouble, 1, 1, &(rgbp->confThresh),
             "0.5", "confidence threshold");
  hestOptAdd(&hopt, "bg", "background", airTypeDouble, 1, 1, &(rgbp->bgGray),
             "0", "gray level to use for voxels who's confidence is zero ");
  hestOptAdd(&hopt, "gr", "gray", airTypeDouble, 1, 1, &(rgbp->isoGray), "0",
             "the gray level to desaturate towards as anisotropy "
             "decreases (while confidence remains 1.0)");
  hestOptAdd(&hopt, "gam", "gamma", airTypeDouble, 1, 1, &(rgbp->gamma), "1",
             "gamma to use on color components");
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, "-",
             "input diffusion tensor volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output image (floating point)");

  USAGE(_tend_evecrgbInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (tenEvecRGB(nout, nin, rgbp)) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble doing colormapping:\n%s\n", me, err);
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
TEND_CMD(evecrgb, INFO);
