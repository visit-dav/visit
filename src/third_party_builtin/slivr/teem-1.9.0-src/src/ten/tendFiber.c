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

#define INFO "Extract a single fiber tract, given a start point"
char *_tend_fiberInfoL =
  (INFO
   ".  Currently, only fibers based on 4th-order Runge-Kutta on "
   "principal eigenvector are supported via \"tend\".  The \"world space\" "
   "in which fibers are calculated has the volume inscribed in a bi-unit cube "
   "centered at the origin. The output fiber is in the form "
   "of a 3xN array of doubles, with N points along fiber.");

int
tend_fiberMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;
  char *outS;

  tenFiberContext *tfx;
  NrrdKernelSpec *ksp;
  double start[3], step, *_stop, *stop;
  int E;
  Nrrd *nin, *nout;
  unsigned int si, stopLen;
  
  hestOptAdd(&hopt, "s", "seed point", airTypeDouble, 3, 3, start, NULL,
             "seed point for fiber; it will propogate in two opposite "
             "directions starting from here");
  hestOptAdd(&hopt, "step", "step size", airTypeDouble, 1, 1, &step, "0.01",
             "stepsize along fiber, in world space");
  hestOptAdd(&hopt, "stop", "stop1", airTypeOther, 1, -1, &_stop, 
             NULL, "the conditions that should signify the end of a fiber. "
             "Multiple stopping criteria are logically OR-ed and tested at "
             "every point along the fiber.  Possibilities include:\n "
             "\b\bo \"aniso:<type>,<thresh>\": require anisotropy to be "
             "above the given threshold.  Which anisotropy type is given "
             "as with \"tend anvol\" (see its usage info)\n "
             "\b\bo \"len:<length>\": limits the length, in world space, "
             "of each fiber half\n "
             "\b\bo \"steps:<N>\": limits the number of points in each "
             "fiber half\n "
             "\b\bo \"conf:<thresh>\": requires the tensor confidence value "
             "to be above the given threshold. ",
             &stopLen, NULL, tendFiberStopCB);
  hestOptAdd(&hopt, "k", "kernel", airTypeOther, 1, 1, &ksp,
             "tent", "kernel for reconstructing tensor field",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, "-",
             "input diffusion tensor volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output fiber");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_fiberInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  tfx = tenFiberContextNew(nin);
  if (!tfx) {
    airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: failed to create the fiber context:\n%s\n", me, err);
    airMopError(mop); exit(1);
  }
  airMopAdd(mop, tfx, (airMopper)tenFiberContextNix, airMopAlways);
  E = 0;
  for (si=0, stop=_stop; si<stopLen; si++, stop+=3) {
    switch((int)stop[0]) {
    case tenFiberStopAniso:
      if (!E) E |= tenFiberStopSet(tfx, tenFiberStopAniso,
                                   (int)stop[1], stop[2]);
      break;
    case tenFiberStopLength:
      if (!E) E |= tenFiberStopSet(tfx, tenFiberStopLength, stop[1]);
      break;
    case tenFiberStopNumSteps:
      if (!E) E |= tenFiberStopSet(tfx, tenFiberStopNumSteps, (int)stop[1]);
      break;
    case tenFiberStopConfidence:
      if (!E) E |= tenFiberStopSet(tfx, tenFiberStopConfidence, stop[1]);
      break;
    case tenFiberStopBounds:
    default:
      /* nothing to do */
      break;
    }
  }
  if (!E) E |= tenFiberTypeSet(tfx, tenFiberTypeEvec1);
  if (!E) E |= tenFiberKernelSet(tfx, ksp->kernel, ksp->parm);
  if (!E) E |= tenFiberIntgSet(tfx, tenFiberIntgRK4);
  if (!E) E |= tenFiberParmSet(tfx, tenFiberParmStepSize, step);
  if (!E) E |= tenFiberParmSet(tfx, tenFiberParmUseIndexSpace, AIR_TRUE);
  if (!E) E |= tenFiberUpdate(tfx);
  if (E) {
    airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); exit(1);
  }
  if (tenFiberTrace(tfx, nout, start)) {
    airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); exit(1);
  }
  if (nout->data) {
    fprintf(stderr, "%s: whyStop[backward] = %s; whyStop[forward] = %s\n", me,
            airEnumStr(tenFiberStop, tfx->whyStop[0]),
            airEnumStr(tenFiberStop, tfx->whyStop[1]));
    if (nrrdSave(outS, nout, NULL)) {
      airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble writing:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  } else {
    fprintf(stderr, "%s: fiber failed to start: %s.\n",
            me, airEnumDesc(tenFiberStop, tfx->whyNowhere));
  }
  airMopOkay(mop);
  return 0;
}
/* TEND_CMD(fiber, INFO); */
unrrduCmd tend_fiberCmd = { "fiber", INFO, tend_fiberMain };
