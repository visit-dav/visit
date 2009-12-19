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

#include "bane.h"
#include "privateBane.h"

#define SCAT_INFO "Make V-G and V-H scatterplots"
char *_baneGkms_scatInfoL =
  (SCAT_INFO
   ". These provide a quick way to inspect a histogram volume, in order to "
   "verify that the derivative inclusion ranges were appropriate, and to "
   "get an initial sense of what sorts of boundaries were present in the "
   "original volume.");
int
baneGkms_scatMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out[2], *perr, err[BIFF_STRLEN];
  Nrrd *hvol, *nvgRaw, *nvhRaw, *nvgQuant, *nvhQuant;
  NrrdRange *vgRange, *vhRange;
  airArray *mop;
  int pret, E;
  double gamma;

  hestOptAdd(&opt, "g", "gamma", airTypeDouble, 1, 1, &gamma, "1.0",
             "gamma used to brighten/darken scatterplots. "
             "gamma > 1.0 brightens; gamma < 1.0 darkens. "
             "Negative gammas invert values (like in xv). ");
  hestOptAdd(&opt, "i", "hvolIn", airTypeOther, 1, 1, &hvol, NULL,
             "input histogram volume (from \"gkms hvol\")",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "o", "vgOut vhOut", airTypeString, 2, 2, out, NULL,
             "Filenames to use for two output scatterplots, (gradient "
             "magnitude versus value, and 2nd derivative versus value); "
             "can use PGM or PNG format");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_baneGkms_scatInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nvgRaw = nrrdNew();
  nvhRaw = nrrdNew();
  nvgQuant = nrrdNew();
  nvhQuant = nrrdNew();
  airMopAdd(mop, nvgRaw, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nvhRaw, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nvgQuant, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nvhQuant, (airMopper)nrrdNuke, airMopAlways);
  if (baneRawScatterplots(nvgRaw, nvhRaw, hvol, AIR_TRUE)) {
    sprintf(err, "%s: trouble creating raw scatterplots", me);
    biffAdd(BANE, err); airMopError(mop); return 1;
  }
  vgRange = nrrdRangeNewSet(nvgRaw, nrrdBlind8BitRangeFalse);
  vhRange = nrrdRangeNewSet(nvhRaw, nrrdBlind8BitRangeFalse);
  airMopAdd(mop, vgRange, (airMopper)nrrdRangeNix, airMopAlways);
  airMopAdd(mop, vhRange, (airMopper)nrrdRangeNix, airMopAlways);
  E = 0;
  if (!E) E |= nrrdArithGamma(nvgRaw, nvgRaw, vgRange, gamma);
  if (!E) E |= nrrdArithGamma(nvhRaw, nvhRaw, vhRange, gamma);
  if (!E) E |= nrrdQuantize(nvgQuant, nvgRaw, vgRange, 8);
  if (!E) E |= nrrdQuantize(nvhQuant, nvhRaw, vhRange, 8);
  if (E) {
    sprintf(err, "%s: trouble doing gamma or quantization", me);
    biffMove(BANE, err, NRRD); airMopError(mop); return 1;
  }

  if (!E) E |= nrrdSave(out[0], nvgQuant, NULL);
  if (!E) E |= nrrdSave(out[1], nvhQuant, NULL);
  if (E) {
    sprintf(err, "%s: trouble saving scatterplot images", me);
    biffMove(BANE, err, NRRD); airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
BANE_GKMS_CMD(scat, SCAT_INFO);

