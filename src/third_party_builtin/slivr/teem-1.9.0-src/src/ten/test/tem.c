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


#include "../ten.h"

char *info = ("Test EM bimodal histogram fitting.");

int
main(int argc, char *argv[]) {
  char *me, *err;
  hestOpt *hopt=NULL;
  airArray *mop;
  Nrrd *nhisto;
  tenEMBimodalParm *biparm;
  double minprob[2];
  
  mop = airMopNew();
  me = argv[0];

  biparm = tenEMBimodalParmNew();
  airMopAdd(mop, biparm, (airMopper)tenEMBimodalParmNix, airMopAlways);

  hestOptAdd(&hopt, NULL, "histogram", airTypeOther, 1, 1, &nhisto, NULL,
             "The 1-D histogram to analyize", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "ts", "two stage", airTypeInt, 0, 0,
             &(biparm->twoStage), NULL,
             "use two-stage processing");
  hestOptAdd(&hopt, "v", "verbose", airTypeInt, 1, 1, &(biparm->verbose), "1",
             "verbosity level");
  hestOptAdd(&hopt, "mp", "minprob 1,2", airTypeDouble, 2, 2, minprob, "0 0",
             "minimum significant posterior probabilies, for first and "
             "second stages");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  biparm->minProb = minprob[0];
  biparm->minProb2 = minprob[1];

  if (tenEMBimodal(biparm, nhisto)) {
    airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: error doing fitting:\n%s", me, err);
    airMopError(mop);
    return 1;
  }
  fprintf(stderr, "%s: bimodal histogram fit\n", me);
  fprintf(stderr, "material 1 (%g%%): mean = %g, stdv = %g\n",
          100*(biparm->fraction1), biparm->mean1, biparm->stdv1);
  fprintf(stderr, "material 2 (%g%%): mean = %g, stdv = %g\n",
          100*(1 - biparm->fraction1), biparm->mean2, biparm->stdv2);
  fprintf(stderr, " ---> optimal threshold = %g (confidence = %g)\n",
          biparm->threshold, biparm->confidence);

  airMopOkay(mop);
  return 0;
}

