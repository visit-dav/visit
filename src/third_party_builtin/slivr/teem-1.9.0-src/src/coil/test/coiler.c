/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
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


#include "../coil.h"

char *info = ("Test program for coil library.");

int
main(int argc, char *argv[]) {
  char *me, *err, *outS;
  hestOpt *hopt=NULL;
  airArray *mop;
  
  int numIters, numThreads, methodType, kindType, _parmLen, pi, radius;
  Nrrd *nin, *nout;
  coilContext *cctx;
  double *_parm, parm[COIL_PARMS_NUM];
  
  mop = airMopNew();
  me = argv[0];
  hestOptAdd(&hopt, "iter", "# iters", airTypeInt, 1, 1, &numIters, "5",
             "number of iterations to do processing for");
  hestOptAdd(&hopt, "nt", "# threads", airTypeInt, 1, 1, &numThreads, "5",
             "number of threads to run");
  hestOptAdd(&hopt, "k", "kind", airTypeEnum, 1, 1, &kindType, NULL,
             "what kind of volume is input", NULL, coilKindType);
  hestOptAdd(&hopt, "m", "method", airTypeEnum, 1, 1, &methodType, "test",
             "what kind of filtering to perform", NULL, coilMethodType);
  hestOptAdd(&hopt, "p", "parms", airTypeDouble, 1, -1, &_parm, NULL,
             "all the parameters required for filtering method", &_parmLen);
  hestOptAdd(&hopt, "r", "radius", airTypeInt, 1, 1, &radius, "1",
             "radius of filtering neighborhood");
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &(nin), "",
             "input volume to filter", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output file to save filtering result into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  cctx = coilContextNew();
  airMopAdd(mop, cctx, (airMopper)coilContextNix, airMopAlways);
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  
  if (_parmLen != coilMethodArray[methodType]->numParm) {
    fprintf(stderr, "%s: %s method wants %d parms, but got %d\n", me, 
            coilMethodArray[methodType]->name,
            coilMethodArray[methodType]->numParm, _parmLen);
    airMopError(mop); 
    return 1;
  }
  for (pi=0; pi<_parmLen; pi++) {
    parm[pi] = _parm[pi];
  }
  if (coilContextAllSet(cctx, nin,
                        coilKindArray[kindType], coilMethodArray[methodType],
                        radius, numThreads, 1 /* verbose */,
                        parm)
      || coilStart(cctx)
      || coilIterate(cctx, numIters)
      || coilFinish(cctx)
      || coilOutputGet(nout, cctx)) {
    airMopAdd(mop, err = biffGetDone(COIL), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble with coil:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't save output:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  airMopOkay(mop);
  return 0;
}

