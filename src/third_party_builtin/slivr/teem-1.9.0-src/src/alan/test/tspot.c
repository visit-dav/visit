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


#include "../alan.h"

int
main(int argc, char *argv[]) {
  alanContext *actx;
  char *err, *me;
  Nrrd *ninit, *nparm, *npri;

  me = argv[0];

  ninit = nrrdNew();
  if (nrrdLoad(ninit, "init.nrrd", NULL)) {
    fprintf(stderr, "%s: load(init.nrrd) failed\n", me);
    free(biffGetDone(NRRD));
    ninit = nrrdNuke(ninit);
  }
  nparm = nrrdNew();
  if (nrrdLoad(nparm, "parm.nrrd", NULL)) {
    fprintf(stderr, "%s: load(parm.nrrd) failed\n", me);
    free(biffGetDone(NRRD));
    nparm = nrrdNuke(nparm);
  }
  npri = nrrdNew();
  if (nrrdLoad(npri, "pri.nrrd", NULL)) {
    fprintf(stderr, "%s: load(pri.nrrd) failed\n", me);
    free(biffGetDone(NRRD));
    npri = nrrdNuke(npri);
  }
  actx = alanContextNew();
  if (alanDimensionSet(actx, 2)
      || alan2DSizeSet(actx, 100, 100)
      || alanParmSet(actx, alanParmMaxIteration, 100000)
      || alanParmSet(actx, alanParmVerbose, 1)
      || alanParmSet(actx, alanParmTextureType, alanTextureTypeTuring)
      || alanParmSet(actx, alanParmRandRange, 4.0)
      || alanParmSet(actx, alanParmK, 0.0125)
      || alanParmSet(actx, alanParmH, 1.2)
      || alanParmSet(actx, alanParmAlpha, 16.0+0.07)
      || alanParmSet(actx, alanParmBeta, 12.0-0.07)
      || alanParmSet(actx, alanParmSpeed, 1.38)
      || alanParmSet(actx, alanParmMinAverageChange, 0.00002)
      || alanParmSet(actx, alanParmSaveInterval, 500)
      || alanParmSet(actx, alanParmFrameInterval,500)
      || alanParmSet(actx, alanParmConstantFilename, AIR_TRUE)
      || alanParmSet(actx, alanParmWrapAround, AIR_TRUE)
      || alanParmSet(actx, alanParmNumThreads, 10)
      ) {
    err = biffGetDone(ALAN);
    fprintf(stderr, "%s: trouble: %s\n", me, err); 
    free(err); return 1;
  }

  if (alanUpdate(actx) 
      || alanInit(actx, ninit, nparm)) {
    err = biffGetDone(ALAN);
    fprintf(stderr, "%s: trouble: %s\n", me, err); 
    free(err); return 1;
  }
  fprintf(stderr, "%s: going to run (%d threads) ...\n", me, actx->numThreads);
  alanRun(actx);
  fprintf(stderr, "%s: stop = %d: %s\n", me, actx->stop,
          airEnumDesc(alanStop, actx->stop));
  /*
  nrrdSave("lev0.nrrd", actx->nlev[0], NULL);
  nrrdSave("lev1.nrrd", actx->nlev[1], NULL);
  */
  
  actx = alanContextNix(actx);
  return 0;
}
