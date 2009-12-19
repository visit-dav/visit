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

int
main(int argc, char **argv) {
  char *me, *err;
  hestParm *hparm;
  hestOpt *hopt = NULL;
  airArray *mop;

  char *outS;
  int E, ptsNum, ptsIdx;
  Nrrd *nin, *nprobe, *nout;
  float *idata, *odata, scale[3], power, x, y, z, r, g, b, cl;
  gage_t *evec, *aniso, *tensor;

  double kparm[NRRD_KERNEL_PARMS_NUM];
  gageContext *ctx;
  gagePerVolume *pvl;

  mop = airMopNew();
  me = argv[0];
  hparm = hestParmNew();
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input diffusion tensor volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "p", "nprobe", airTypeOther, 1, 1, &nprobe, NULL,
             "input list of points to probe at, as 3xN float nrrd",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "sc", "scaling", airTypeFloat, 3, 3, scale, "1.0 1.0 1.0",
             "scaling that took index space positions (in nin) to "
             "vertex positions (in nprobe); hopefully just the \"spacings\" "
             "on the volume that was isosurfaced.");
  hestOptAdd(&hopt, "pow", "power", airTypeFloat, 1, 1, &power, "0.4",
             "power to raise cl_2 to to determine saturation of color");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, NULL,
             "output image (floating point)");
  
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, "secret testing area", AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_TRUE)) {
    airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: didn't get value tensor volume:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }

  if (!( nrrdTypeFloat == nprobe->type &&
         2 == nprobe->dim &&
         3 == nprobe->axis[0].size )) {
    fprintf(stderr, "%s: didn't get valid probe point list\n", me);
    airMopError(mop);
    return 1;
  }
  ptsNum = nprobe->axis[1].size;
  
  ctx = gageContextNew();
  airMopAdd(mop, ctx, (airMopper)gageContextNix, airMopAlways);
  gageSet(ctx, gageParmCheckIntegrals, AIR_TRUE);
  kparm[0] = 1.0;
  E = 0;
  if (!E) E |= !(pvl = gagePerVolumeNew(nin, tenGageKind));
  if (!E) E |= gagePerVolumeAttach(ctx, pvl);
  if (!E) E |= gageKernelSet(ctx, gageKernel00, nrrdKernelTent, kparm);
  if (!E) E |= gageQuerySet(pvl, ((1 << tenGageEvec) | 
                                  (1 << tenGageAniso) |
                                  (1 << tenGageTensor)) );
  if (!E) E |= gageUpdate(ctx);
  if (E) {
    airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }
  evec = gageAnswerPointer(pvl, tenGageEvec);
  aniso = gageAnswerPointer(pvl, tenGageAniso);
  tensor = gageAnswerPointer(pvl, tenGageTensor);

  if (nrrdAlloc(nout=nrrdNew(), nrrdTypeFloat, 2, 3, ptsNum)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't allocate output:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }
  
  idata = nprobe->data;
  odata = nout->data;
  for (ptsIdx=0; ptsIdx<ptsNum; ptsIdx++) {
    x = idata[3*ptsIdx + 0]/scale[0];
    y = idata[3*ptsIdx + 1]/scale[1];
    z = idata[3*ptsIdx + 2]/scale[2];
    gageProbe(ctx, x, y, z);
    r = AIR_ABS(evec[0]);
    g = AIR_ABS(evec[1]);
    b = AIR_ABS(evec[2]);
    cl = aniso[tenAniso_Cl2];
    cl = tensor[0]*AIR_CLAMP(0.0, cl, 1.0);
    cl = pow(cl, power);
    odata[3*ptsIdx + 0] = AIR_AFFINE(0.0, cl, 1.0, 0.5, r);
    odata[3*ptsIdx + 1] = AIR_AFFINE(0.0, cl, 1.0, 0.5, g);
    odata[3*ptsIdx + 2] = AIR_AFFINE(0.0, cl, 1.0, 0.5, b);
  }
  
  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving output:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }

  airMopOkay(mop);
  exit(0);
}
