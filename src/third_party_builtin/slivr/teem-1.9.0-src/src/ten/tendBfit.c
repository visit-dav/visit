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

#define INFO "Non-linear least-squares fitting of b-value curves"
char *_tend_bfitInfoL =
  (INFO
   ". Axis 0 is replaced by three values: amp, dec, err, based on a "
   "non-linear least-squares fit of amp*exp(-b*dec) to the range of DWI "
   "values along input axis 0, as a function of changing b values.  ");

int
tend_bfitMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  Nrrd *nin, *nout;
  double *bb, *ww, *_ww, eps;
  unsigned int ii, bbLen, _wwLen;
  int iterMax;
  char *outS;

  hparm->respFileEnable = AIR_TRUE;

  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, "-",
             "Input nrrd.  List of DWIs from different b-values must "
             "be along axis 0", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "b", "b1 b2", airTypeDouble, 2, -1, &bb, NULL,
             "b values across axis 0 of input nrrd", &bbLen);
  hestOptAdd(&hopt, "w", "w1 w2", airTypeDouble, 2, -1, &_ww, "nan nan",
             "weights for samples in non-linear fitting", &_wwLen);
  hestOptAdd(&hopt, "imax", "# iter", airTypeInt, 1, 1, &iterMax, "10",
             "max number of iterations to use in non-linear fitting, or, "
             "use 0 to do only initial linear fit");
  hestOptAdd(&hopt, "eps", "epsilon", airTypeDouble, 1, 1, &eps, "1",
             "epsilon convergence threshold for non-linear fitting");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output tensor volume");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_bfitInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!( bbLen == nin->axis[0].size )) {
    fprintf(stderr, "%s: got %d b-values but axis 0 size is " 
            _AIR_SIZE_T_CNV "\n", me,
            bbLen, nin->axis[0].size);
    airMopError(mop); return 1;
  }
  if (AIR_EXISTS(_ww[0])) {
    if (!( _wwLen == nin->axis[0].size )) {
      fprintf(stderr, "%s: got %d weights but axis 0 size is " 
              _AIR_SIZE_T_CNV "\n", me,
              _wwLen, nin->axis[0].size);
      airMopError(mop); return 1;
    }
    ww = _ww;
  } else {
    /* no explicit weights specified */
    ww = (double*)calloc(nin->axis[0].size, sizeof(double));
    airMopAdd(mop, ww, airFree, airMopAlways);
    for (ii=0; ii<nin->axis[0].size; ii++) {
      ww[ii] = 1.0;
    }
  }
  
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (tenBVecNonLinearFit(nout, nin, bb, ww, iterMax, eps)) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
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
/* TEND_CMD(bfit, INFO); */
unrrduCmd tend_bfitCmd = { "bfit", INFO, tend_bfitMain };
