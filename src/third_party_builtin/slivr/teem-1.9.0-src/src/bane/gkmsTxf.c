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

#define TXF_INFO "Create Levoy-style triangular 2D opacity functions"
char *_baneGkms_txfInfoL =
  (TXF_INFO
   ". The triangles are in the 2D space of data value and gradient "
   "magnitude.  They can be tilted sideways and clipped at the bottom. "
   "This doesn't strictly speaking belong in \"gkms\" but there's no "
   "other good place in Teem.");
int
baneGkms_txfMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *perr, err[BIFF_STRLEN];
  Nrrd *nout;
  airArray *mop;
  int pret, E, res[2], vi, gi, step;
  float min[2], max[2], top[2], v0, g0, *data, v, g,
    gwidth, width, mwidth, 
    tvl, tvr, vl, vr, tmp, maxa;

  hestOptAdd(&opt, "r", "Vres Gres", airTypeInt, 2, 2, res, "256 256",
             "resolution of the transfer function in value and gradient "
             "magnitude");
  hestOptAdd(&opt, "min", "Vmin Gmin", airTypeFloat, 2, 2, min, "0.0 0.0",
             "minimum value and grad mag in txf");
  hestOptAdd(&opt, "max", "Vmax Gmax", airTypeFloat, 2, 2, max, NULL,
             "maximum value and grad mag in txf");
  hestOptAdd(&opt, "v", "base value", airTypeFloat, 1, 1, &v0, NULL,
             "data value at which to position bottom of triangle");
  hestOptAdd(&opt, "g", "gthresh", airTypeFloat, 1, 1, &g0, "0.0",
             "lowest grad mag to receive opacity");
  hestOptAdd(&opt, "gw", "gwidth", airTypeFloat, 1, 1, &gwidth, "0.0",
             "range of grad mag values over which to apply threshold "
             "at low gradient magnitudes");
  hestOptAdd(&opt, "top", "Vtop Gtop", airTypeFloat, 2, 2, top, NULL,
             "data value and grad mag at center of top of triangle");
  hestOptAdd(&opt, "w", "value width", airTypeFloat, 1, 1, &width, NULL,
             "range of values to be spanned at top of triangle");
  hestOptAdd(&opt, "mw", "value width", airTypeFloat, 1, 1, &mwidth, "0",
             "range of values to be spanned at BOTTOM of triangle");
  hestOptAdd(&opt, "step", NULL, airTypeInt, 0, 0, &step, NULL,
             "instead of assigning opacity inside a triangular region, "
             "make it more like a step function, in which opacity never "
             "decreases in increasing data value");
  hestOptAdd(&opt, "a", "max opac", airTypeFloat, 1, 1, &maxa, "1.0",
             "highest opacity to assign");
  hestOptAdd(&opt, "o", "opacOut", airTypeString, 1, 1, &out, NULL,
             "output opacity function filename");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_baneGkms_txfInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  E = 0;
  if (!E) E |= nrrdMaybeAlloc_va(nout, nrrdTypeFloat, 3,
                                 AIR_CAST(size_t, 1),
                                 AIR_CAST(size_t, res[0]),
                                 AIR_CAST(size_t, res[1]));
  if (!E) E |= !(nout->axis[0].label = airStrdup("A"));
  if (!E) E |= !(nout->axis[1].label = airStrdup("gage(scalar:v)"));
  if (!E) nrrdAxisInfoSet_va(nout, nrrdAxisInfoMin,
                             AIR_NAN, (double)min[0], (double)min[1]);
  if (!E) nrrdAxisInfoSet_va(nout, nrrdAxisInfoMax,
                             AIR_NAN, (double)max[0], (double)max[1]);
  if (!E) E |= !(nout->axis[2].label = airStrdup("gage(scalar:gm)"));
  if (E) {
    sprintf(err, "%s: trouble creating opacity function nrrd", me);
    biffMove(BANE, err, NRRD); airMopError(mop); return 1;
  }
  data = (float *)nout->data;
  tvl = top[0] - width/2; 
  tvr = top[0] + width/2;
  mwidth /= 2;
  for (gi=0; gi<res[1]; gi++) {
    g = AIR_CAST(float, NRRD_CELL_POS(min[1], max[1], res[1], gi));
    for (vi=0; vi<res[0]; vi++) {
      v = AIR_CAST(float, NRRD_CELL_POS(min[0], max[0], res[0], vi));
      vl = AIR_CAST(float, AIR_AFFINE(0, g, top[1], v0-mwidth, tvl));
      vr = AIR_CAST(float, AIR_AFFINE(0, g, top[1], v0+mwidth, tvr));
      if (g > top[1]) {
        data[vi + res[0]*gi] = 0;
        continue;
      }
      tmp = AIR_CAST(float, (v - vl)/(0.00001 + vr - vl));
      tmp = 1 - AIR_ABS(2*tmp - 1);
      if (step && v > (vr + vl)/2) {
        tmp = 1;
      }
      tmp = AIR_MAX(0, tmp);
      data[vi + res[0]*gi] = tmp*maxa;
      tmp = AIR_CAST(float, AIR_AFFINE(g0 - gwidth/2, g, g0 + gwidth/2,
                                       0.0, 1.0));
      tmp = AIR_CLAMP(0, tmp, 1);
      data[vi + res[0]*gi] *= tmp;
    }
  }
  if (nrrdSave(out, nout, NULL)) {
    sprintf(err, "%s: trouble saving opacity function", me);
    biffMove(BANE, err, NRRD); airMopError(mop); return 1;
  }
  
  airMopOkay(mop);
  return 0;
}
BANE_GKMS_CMD(txf, TXF_INFO);

