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

#define INFO "Calculate balanced gradient directions for DWI acquisition"
char *_tend_gradsInfoL =
  (INFO
   ", based on a simulation of anti-podal point pairs repelling each other "
   "on the unit sphere surface. This can either distribute more uniformly "
   "a given set of gradients, or it can make a new distribution from scratch. "
   "A more clever implementation could decrease drag with time, as the "
   "solution converges, to get closer to the minimum energy configuration "
   "faster.  In the mean time, you can run a second pass on the output of "
   "the first pass, using lower drag. A second phase of the algorithm "
   "tries sign changes in gradient directions in trying to find an optimally "
   "balanced set of directions.  This uses a randomized search, so if it "
   "doesn't seem to be finishing in a reasonable amount of time, try "
   "restarting.");

int
tend_gradsMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  int num, E, nosrand;
  Nrrd *nin, *nout;
  char *outS;
  tenGradientParm *tgparm;

  mop = airMopNew();
  tgparm = tenGradientParmNew();
  airMopAdd(mop, tgparm, (airMopper)tenGradientParmNix, airMopAlways);

  hestOptAdd(&hopt, "n", "# dir", airTypeInt, 1, 1, &num, "6",
             "desired number of diffusion gradient directions");
  hestOptAdd(&hopt, "i", "grads", airTypeOther, 1, 1, &nin, "",
             "initial gradient directions to start with, instead "
             "of default random initial directions (overrides \"-n\")",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "nosrand", NULL, airTypeInt, 0, 0, &nosrand, NULL,
             "do NOT call srand() to initialize random number generator");
  hestOptAdd(&hopt, "dt", "dt", airTypeDouble, 1, 1, &(tgparm->dt), "0.05",
             "time increment in solver");
  hestOptAdd(&hopt, "drag", "drag", airTypeDouble, 1, 1, &(tgparm->drag),
             "0.0005", "viscous drag, to keep things stable");
  hestOptAdd(&hopt, "charge", "charge", airTypeDouble, 1, 1,
             &(tgparm->charge), "0.2",
             "amount of charge on each particle");
  hestOptAdd(&hopt, "single", NULL, airTypeInt, 0, 0, &(tgparm->single), NULL,
             "instead of the default behavior of tracking a pair of "
             "antipodal points (appropriate for determining DWI gradients), "
             "use only single points (appropriate for who knows what).");
  hestOptAdd(&hopt, "snap", "interval", airTypeInt, 1, 1, &(tgparm->snap), "0",
             "specifies an interval between which snapshots of the point "
             "positions should be saved out.  By default (not using this "
             "option), there is no such snapshot behavior");
  hestOptAdd(&hopt, "jitter", "jitter", airTypeDouble, 1, 1,
             &(tgparm->jitter), "0.05",
             "amount by which to perturb points when given an input nrrd");
  hestOptAdd(&hopt, "minvelo", "vel", airTypeDouble, 1, 1, 
             &(tgparm->minVelocity), "0.00001",
             "low threshold on mean velocity of repelling points, "
             "at which point repulsion phase of algorithm terminates. ");
  hestOptAdd(&hopt, "maxiter", "# iters", airTypeInt, 1, 1,
             &(tgparm->maxIteration), "1000000",
             "max number of iterations for which to run the simulation");
  hestOptAdd(&hopt, "minimprov", "delta", airTypeDouble, 1, 1, 
             &(tgparm->minMeanImprovement), "0.00005",
             "in the second phase of the algorithm, "
             "when stochastically balancing the sign of the gradients, "
             "the (small) improvement in length of mean gradient "
             "which triggers termination (as further improvements "
             "are unlikely. ");
  hestOptAdd(&hopt, "minmean", "len", airTypeDouble, 1, 1,
             &(tgparm->minMean), "0.0001",
             "if length of mean gradient falls below this, finish "
             "the balancing phase");
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &outS, "-",
             "file to write output nrrd to");
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_gradsInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  tgparm->srand = !nosrand;
  E = (nin
       ? tenGradientDistribute(nout, nin, tgparm)
       : tenGradientGenerate(nout, num, tgparm));
  if (E) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble making distribution:\n%s\n", me, err);
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
unrrduCmd tend_gradsCmd = { "grads", INFO, tend_gradsMain };
