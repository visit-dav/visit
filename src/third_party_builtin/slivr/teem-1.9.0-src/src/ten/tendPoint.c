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

#define INFO "Describe everything about one sample in a DT volume"
char *_tend_pointInfoL =
  (INFO
   ". ");

int
tend_pointMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  int loc[3], idx, sx, sy, sz, i;
  Nrrd *nin;
  float *tdata, eval[3], evec[9], c[TEN_ANISO_MAX+1],
    angle, axis[3], mat[9];

  hestOptAdd(&hopt, "p", "x y z", airTypeInt, 3, 3, loc, NULL,
             "coordinates of sample to be described");
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, "-",
             "input diffusion tensor volume", NULL, NULL, nrrdHestNrrd);

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_pointInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: didn't get a valid DT volume:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  sx = nin->axis[1].size;
  sy = nin->axis[2].size;
  sz = nin->axis[3].size;
  if (!( AIR_IN_CL(0, loc[0], sx-1) &&
         AIR_IN_CL(0, loc[1], sy-1) &&
         AIR_IN_CL(0, loc[2], sz-1) )) {
    fprintf(stderr, "%s: location (%d,%d,%d) not inside volume "
            "[0..%d]x[0..%d]x[0..%d]\n",
            me, loc[0], loc[1], loc[2],
            sx-1, sy-1, sz-1);
    airMopError(mop); return 1;
  }

  idx = loc[0] + sx*(loc[1] + sy*loc[2]);
  tdata = (float*)(nin->data) + 7*idx;
  fprintf(stderr, "location = (%d,%d,%d) = %d\n", loc[0], loc[1], loc[2], idx);
  fprintf(stderr, "confidence = %g\n", tdata[0]);
  fprintf(stderr, "tensor =\n");
  fprintf(stderr, "{%.7f,%.7f,%.7f,%.7f,%.7f,%.7f} = \n",
          tdata[1], tdata[2], tdata[3], tdata[4], tdata[5], tdata[6]);
  fprintf(stderr, "% 15.7f % 15.7f % 15.7f\n", tdata[1], tdata[2], tdata[3]);
  fprintf(stderr, "% 15.7f % 15.7f % 15.7f\n", tdata[2], tdata[4], tdata[5]);
  fprintf(stderr, "% 15.7f % 15.7f % 15.7f\n", tdata[3], tdata[5], tdata[6]);
  tenEigensolve_f(eval, evec, tdata);
  fprintf(stderr, "eigensystem = (<eigenvalue> : <eigenvector>):\n");
  fprintf(stderr, "% 15.7f : % 15.7f % 15.7f % 15.7f\n",
          eval[0], evec[0], evec[1], evec[2]);
  fprintf(stderr, "% 15.7f : % 15.7f % 15.7f % 15.7f\n",
          eval[1], evec[3], evec[4], evec[5]);
  fprintf(stderr, "% 15.7f : % 15.7f % 15.7f % 15.7f\n",
          eval[2], evec[6], evec[7], evec[8]);
  angle = ell_3m_to_aa_f(axis, evec);
  fprintf(stderr, "eigenvector rotation: %g around {%g,%g,%g}\n",
          angle, axis[0], axis[1], axis[2]);
  ell_aa_to_3m_f(mat, angle, axis);
  fprintf(stderr, "% 15.7f % 15.7f % 15.7f\n",
          mat[0], mat[1], mat[2]);
  fprintf(stderr, "% 15.7f % 15.7f % 15.7f\n",
          mat[3], mat[4], mat[5]);
  fprintf(stderr, "% 15.7f % 15.7f % 15.7f\n",
          mat[6], mat[7], mat[8]);
  tenAnisoCalc_f(c, eval);
  fprintf(stderr, "anisotropies = \n");
  for (i=1; i<=TEN_ANISO_MAX; i++) {
    fprintf(stderr, "%s: % 15.7f\n",
            airEnumStr(tenAniso, i), c[i]);
  }

  airMopOkay(mop);
  return 0;
}
TEND_CMD(point, INFO);

