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

#define INFO "Calculate structure tensors from a scalar field"
char *_tend_stenInfoL =
  (INFO
   ".  Not a diffusion tensor, but it is symmetric and positive-definate.");

int
tend_stenMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  int iScale, dScale, dsmp;
  Nrrd *nin, *nout;
  char *outS;

  hestOptAdd(&hopt, "ds", "diff. scale", airTypeInt, 1, 1, &dScale, "1",
             "differentiation scale, in pixels: the radius of the "
             "kernel used for differentation to compute gradient vectors");
  hestOptAdd(&hopt, "is", "int. scale", airTypeInt, 1, 1, &iScale, "2",
             "integration scale, in pixels: the radius of the "
             "kernel used for blurring outer products of gradients "
             "in order compute structure tensors");
  hestOptAdd(&hopt, "df", "downsample factor", airTypeInt, 1, 1, &dsmp, "1",
             "the factor by which to downsample when creating volume of "
             "structure tensors");
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, "-",
             "input scalar volume",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output filename");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_stenInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (gageStructureTensor(nout, nin, dScale, iScale, dsmp)) {
    airMopAdd(mop, err=biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble calculating structure tensors:\n%s\n",
            me, err);
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
/* TEND_CMD(sten, INFO); */
unrrduCmd tend_stenCmd = { "sten", INFO, tend_stenMain };


