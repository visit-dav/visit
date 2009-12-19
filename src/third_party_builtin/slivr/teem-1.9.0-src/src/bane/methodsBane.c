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

void
_baneAxisInit(baneAxis *axis) {

  axis->res = 0;
  axis->measr = NULL;
  axis->inc = NULL;
}

void
_baneAxisEmpty(baneAxis *axis) {

  axis->measr = baneMeasrNix(axis->measr);
  axis->inc = baneIncNix(axis->inc);
}

baneHVolParm *
baneHVolParmNew() {
  baneHVolParm *hvp;
  int i, j;
  
  hvp = (baneHVolParm *)calloc(1, sizeof(baneHVolParm));
  if (hvp) {
    hvp->verbose = baneDefVerbose;
    hvp->makeMeasrVol = baneDefMakeMeasrVol;
    hvp->measrVol = NULL;
    hvp->measrVolDone = AIR_FALSE;
    _baneAxisInit(hvp->axis + 0);
    _baneAxisInit(hvp->axis + 1);
    _baneAxisInit(hvp->axis + 2);
    hvp->k3pack = AIR_TRUE;
    for(i=gageKernelUnknown+1; i<gageKernelLast; i++) {
      hvp->k[i] = NULL;
      for (j=0; j<NRRD_KERNEL_PARMS_NUM; j++)
        hvp->kparm[i][j] = AIR_NAN;
    }
    hvp->renormalize = baneDefRenormalize;
    hvp->clip = NULL;
    hvp->incLimit = baneDefIncLimit;
  }
  return hvp;
}

void
baneHVolParmAxisSet(baneHVolParm *hvp, unsigned int axisIdx,
                    unsigned int res, baneMeasr *measr, baneInc *inc) {

  if (hvp && axisIdx <= 2) {
    _baneAxisEmpty(hvp->axis + axisIdx);
    hvp->axis[axisIdx].res = res;
    hvp->axis[axisIdx].measr = baneMeasrCopy(measr);
    hvp->axis[axisIdx].inc = baneIncCopy(inc);
  }
  return;
}

void
baneHVolParmClipSet(baneHVolParm *hvp, baneClip *clip) {

  if (hvp && clip) {
    hvp->clip = baneClipNix(hvp->clip);
    hvp->clip = baneClipCopy(clip);
  }
  return;
}

baneHVolParm *
baneHVolParmNix(baneHVolParm *hvp) {
  
  if (hvp) {
    if (hvp->measrVol) {
      nrrdNuke(hvp->measrVol);
    }
    _baneAxisEmpty(hvp->axis + 0);
    _baneAxisEmpty(hvp->axis + 1);
    _baneAxisEmpty(hvp->axis + 2);
    baneClipNix(hvp->clip);
    free(hvp);
  }
  return NULL;
}

/*
******** baneHVolParmGKMSInit()
**
** The way Gordon does it.
*/
void
baneHVolParmGKMSInit(baneHVolParm *hvp) {
  baneMeasr *measr;
  baneInc *inc;
  double parm[BANE_PARM_NUM];

  if (hvp) {
    /* no parms to set */
    measr = baneMeasrNew(baneMeasrGradMag, parm);
    parm[0] = 1024;
    parm[1] = 0.15;
    inc = baneIncNew(baneIncPercentile, measr->range, parm);
    baneHVolParmAxisSet(hvp, 0, 256, measr, inc);
    measr = baneMeasrNix(measr);
    inc = baneIncNix(inc);

    /* no parms to set */
    measr = baneMeasrNew(baneMeasr2ndDD, parm);
    parm[0] = 1024;
    parm[1] = 0.25;
    inc = baneIncNew(baneIncPercentile, measr->range, parm);
    baneHVolParmAxisSet(hvp, 1, 256, measr, inc);
    measr = baneMeasrNix(measr);
    inc = baneIncNix(inc);

    /* no parms to set */
    measr = baneMeasrNew(baneMeasrValueAnywhere, parm);
    parm[0] = 1.0;
    inc = baneIncNew(baneIncRangeRatio, measr->range, parm);
    baneHVolParmAxisSet(hvp, 2, 256, measr, inc);
    measr = baneMeasrNix(measr);
    inc = baneIncNix(inc);

    nrrdKernelParse(&(hvp->k[gageKernel00]), hvp->kparm[gageKernel00],
                    "cubic:0,0.5");  /* catmull-rom */
    nrrdKernelParse(&(hvp->k[gageKernel11]), hvp->kparm[gageKernel11],
                    "cubicd:1,0");   /* b-spline */
    nrrdKernelParse(&(hvp->k[gageKernel22]), hvp->kparm[gageKernel22],
                    "cubicdd:1,0");  /* b-spline */
  }
}
