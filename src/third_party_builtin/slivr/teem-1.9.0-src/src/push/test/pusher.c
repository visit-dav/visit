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


#include "../push.h"

char *info = ("Test program for push library.");

int
main(int argc, char *argv[]) {
  char *me, *err;
  hestOpt *hopt=NULL;
  airArray *mop;
  
  char *outS[2];
  int seed, numThread, numThing, snap, minIter, maxIter, singleBin,
    noDriftCorrect, tln, frenet, incr;
  pushContext *pctx;
  Nrrd *nin, *nPosIn, *nStnIn, *nPosOut, *nTenOut, *nStnOut;
  double step, drag, preDrag, mass, scale, nudge, margin,
    wall, minMeanVel, tlf[3];
  NrrdKernelSpec *ksp00, *ksp11;
  pushForce *force;
  
  mop = airMopNew();
  me = argv[0];
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input volume to filter", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "pi", "npos", airTypeOther, 1, 1, &nPosIn, "",
             "positions to start at (overrides \"-np\")",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "si", "nstn", airTypeOther, 1, 1, &nStnIn, "",
             "connectivity/tractlet information for \"-pi\" nrrd",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "np", "# points", airTypeInt, 1, 1, &numThing, "100",
             "number of points to use in simulation");
  hestOptAdd(&hopt, "seed", "seed", airTypeInt, 1, 1, &seed, "42",
             "seed value for RNG which determines initial point locations");
  hestOptAdd(&hopt, "maxi", "# iters", airTypeInt, 1, 1, &maxIter, "0",
             "if non-zero, max number of iterations to do processing for");
  hestOptAdd(&hopt, "mini", "# iters", airTypeInt, 1, 1, &minIter, "0",
             "if non-zero, min number of iterations to do processing for");
  hestOptAdd(&hopt, "step", "step", airTypeDouble, 1, 1, &step, "0.01",
             "time step in integration");
  hestOptAdd(&hopt, "tlf", "thr soft step", airTypeDouble, 3, 3, tlf,
             "0.2 0.2 0.01",
             "three parameters determining tractlet formation: the "
             "nominal Cl treshold below which there is no tractlet, "
             "the region of smoothing around the threshold, and the base "
             "size of each step");
  hestOptAdd(&hopt, "tln", "# steps", airTypeInt, 1, 1, &tln, "10",
             "max number of steps in each tractlet half");
  hestOptAdd(&hopt, "fren", NULL, airTypeInt, 0, 0, &frenet, NULL,
             "use tractlet Frenet frame for force averaging");
  hestOptAdd(&hopt, "scl", "scale", airTypeDouble, 1, 1, &scale, "0.25",
             "scaling from tensor size to glyph size");
  hestOptAdd(&hopt, "drag", "drag", airTypeDouble, 1, 1, &drag, "0.01",
             "amount of drag");
  hestOptAdd(&hopt, "preDrag", "preDrag", airTypeDouble, 1, 1, &preDrag,
             "0.2",
             "amount of drag at beginning of minimum iteration period");
  hestOptAdd(&hopt, "mass", "mass", airTypeDouble, 1, 1, &mass, "1",
             "mass of each particle");
  hestOptAdd(&hopt, "force", "spec", airTypeOther, 1, 1, &force, NULL,
             "specification of force function to use",
             NULL, NULL, pushHestForce);
  hestOptAdd(&hopt, "nudge", "nudge", airTypeDouble, 1, 1, &nudge, "0.0",
             "scaling of how distance from origin generates a nudging "
             "force back towards the origin (as if by sitting in "
             "parabola y = (1/2)*nudge*x^2)");
  hestOptAdd(&hopt, "wall", "wall", airTypeDouble, 1, 1, &wall, "0.0",
             "spring constant of containing walls");
  hestOptAdd(&hopt, "marg", "margin", airTypeDouble, 1, 1, &margin, "0.2",
             "margin around [-1,1]^3 within which to allow points to "
             "overflow, outside of which they're disallowed");
  hestOptAdd(&hopt, "mmv", "mean vel", airTypeDouble, 1, 1, &minMeanVel,
             "0.1", "minimum mean velocity that signifies convergence");
  hestOptAdd(&hopt, "snap", "iters", airTypeInt, 1, 1, &snap, "0",
             "if non-zero, number of iterations between which a snapshot "
             "is saved");
  hestOptAdd(&hopt, "nt", "# threads", airTypeInt, 1, 1, &numThread, "1",
             "number of threads to run");
  hestOptAdd(&hopt, "k00", "kernel", airTypeOther, 1, 1, &ksp00,
             "tent", "kernel for tensor field sampling",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k11", "kernel", airTypeOther, 1, 1, &ksp11,
             "fordif", "kernel for finding containment gradient from mask",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "nobin", NULL, airTypeBool, 0, 0, &singleBin, NULL,
             "turn off spatial binning (which prevents multi-threading "
             "from being useful), for debugging or speed-up measurement");
  hestOptAdd(&hopt, "incr", "N", airTypeInt, 1, 1, &incr, "60",
             "increment for per-bin airArray of things");
  hestOptAdd(&hopt, "ndc", NULL, airTypeBool, 0, 0, &noDriftCorrect, NULL,
             "turn off the correction for sliding around near changes of "
             "size in the tensor field");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 2, 2, outS, "p.nrrd t.nrrd",
             "output files to save position and tensor info into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  pctx = pushContextNew();
  airMopAdd(mop, pctx, (airMopper)pushContextNix, airMopAlways);
  nPosOut = nrrdNew();
  airMopAdd(mop, nPosOut, (airMopper)nrrdNuke, airMopAlways);
  nTenOut = nrrdNew();
  airMopAdd(mop, nTenOut, (airMopper)nrrdNuke, airMopAlways);
  nStnOut = nrrdNew();
  airMopAdd(mop, nStnOut, (airMopper)nrrdNuke, airMopAlways);
  
  pctx->binIncr = incr;
  pctx->nin = nin;
  pctx->npos = nPosIn;
  pctx->nstn = nStnIn;
  pctx->numThread = numThread;
  pctx->singleBin = singleBin;
  pctx->maxIter = maxIter;
  pctx->minIter = minIter;
  pctx->seed = seed;
  pctx->drag = drag;
  pctx->preDrag = preDrag;
  pctx->step = step;
  pctx->mass = mass;
  pctx->tlThresh = tlf[0];
  pctx->tlSoft = tlf[1];
  pctx->tlStep = tlf[2];
  pctx->tlNumStep = tln;
  pctx->tlFrenet = frenet;
  pctx->force = force;
  pctx->scale = scale;
  pctx->nudge = nudge;
  pctx->wall = wall;
  pctx->margin = margin;
  pctx->minMeanVel = minMeanVel;
  pctx->snap = snap;
  pctx->numThing = numThing;
  pctx->numStage = 2;
  pctx->driftCorrect = !noDriftCorrect;
  pctx->verbose = 0;
  nrrdKernelSpecSet(pctx->ksp00, ksp00->kernel, ksp00->parm);
  nrrdKernelSpecSet(pctx->ksp11, ksp11->kernel, ksp11->parm);

  if (pushStart(pctx)
      || pushRun(pctx)
      || pushOutputGet(nPosOut, nTenOut, nStnOut, pctx)
      || pushFinish(pctx)) {
    airMopAdd(mop, err = biffGetDone(PUSH), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  fprintf(stderr, "%s: time for %d iterations= %g secs; meanVel = %g\n",
          me, pctx->iter, pctx->time, pctx->meanVel);
  if (nrrdSave(outS[0], nPosOut, NULL)
      || nrrdSave(outS[1], nTenOut, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't save output:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  airMopOkay(mop);
  return 0;
}

