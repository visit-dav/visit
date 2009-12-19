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

#include <teem/air.h>
#include <teem/hest.h>
#include <teem/biff.h>
#include <teem/nrrd.h>
#include <teem/gage.h>

char info[]="Gantry tilt be gone!  This program is actually of limited "
"utility: it can only change the tilt by shearing with the "
"X and Z axis fixed, by some angle \"around\" the X axis, assuming "
"that (X,Y,Z) is a right-handed frame. ";

int
main(int argc, char *argv[]) {
  hestParm *hparm;
  hestOpt *hopt = NULL;
  gageContext *ctx;
  gagePerVolume *pvl;
  Nrrd *nin, *nout;
  char *me, *outS;
  float angle;
  double xs, ys, zs, y, z, padval;
  const gage_t *val;
  int sx, sy, sz, E, xi, yi, zi, clamp;
  NrrdKernelSpec *gantric;
  void *out;
  double (*insert)(void *v, size_t I, double d);
  
  me = argv[0];
  hparm = hestParmNew();
  hparm->elideSingleOtherType = AIR_TRUE;

  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input volume, in nrrd format",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "a", "angle", airTypeFloat, 1, 1, &angle, NULL,
             "angle, in degrees, of the gantry tilt around the X axis. "
             "This is opposite of the amount of tweak we apply.");
  hestOptAdd(&hopt, "k", "kern", airTypeOther, 1, 1, &gantric,
             "tent",
             "The kernel to use for resampling.  Chances are, there "
             "is no justification for anything more than \"tent\".  "
             "Possibilities include:\n "
             "\b\bo \"box\": nearest neighbor interpolation\n "
             "\b\bo \"tent\": linear interpolation\n "
             "\b\bo \"cubic:B,C\": Mitchell/Netravali BC-family of "
             "cubics:\n "
             "\t\t\"cubic:1,0\": B-spline; maximal blurring\n "
             "\t\t\"cubic:0,0.5\": Catmull-Rom; good interpolating kernel\n "
             "\b\bo \"quartic:A\": 1-parameter family of "
             "interpolating quartics (\"quartic:0.0834\" is most accurate)\n "
             "\b\bo \"gauss:S,C\": Gaussian blurring, with standard deviation "
             "S and cut-off at C standard deviations",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "clamp", NULL, airTypeInt, 0, 0, &clamp, NULL,
             "clamp sampling positions to inside original volume, "
             "effectively does a bleed of the boundary values");
  hestOptAdd(&hopt, "p", "pad value", airTypeDouble, 1, 1, &padval, "0.0",
             "when NOT doing clampging (no \"-clamp\"), what value to the "
             "boundary of the volume with");
  hestOptAdd(&hopt, "o", "output", airTypeString, 1, 1, &outS, NULL,
             "output volume in nrrd format");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);

  sx = nin->axis[0].size;
  sy = nin->axis[1].size;
  sz = nin->axis[2].size;
  xs = nin->axis[0].spacing;
  ys = nin->axis[1].spacing;
  zs = nin->axis[2].spacing;
  if (!(AIR_EXISTS(xs) && AIR_EXISTS(ys) && AIR_EXISTS(zs))) {
    fprintf(stderr, "%s: all axis spacings must exist in input nrrd\n", me);
    exit(1);
  }
  fprintf(stderr, "%s: input and output have dimensions %d %d %d\n",
          me, sx, sy, sz);
  
  /* start by just copying the nrrd; then we'll meddle with the values */
  if (nrrdCopy(nout = nrrdNew(), nin)) {
    fprintf(stderr, "%s: trouble:\n%s\n", me, biffGet(NRRD));
    exit(1);
  }
  out = nout->data;
  insert = nrrdDInsert[nout->type];

  ctx = gageContextNew();
  gageParmSet(ctx, gageParmVerbose, 1);
  gageParmSet(ctx, gageParmRenormalize, AIR_TRUE);
  E = 0;
  if (!E) E |= !(pvl = gagePerVolumeNew(ctx, nin, gageKindScl));
  if (!E) E |= gagePerVolumeAttach(ctx, pvl);
  if (!E) E |= gageKernelSet(ctx, gageKernel00,
                             gantric->kernel, gantric->parm);
  if (!E) E |= gageQueryItemOn(ctx, pvl, gageSclValue);
  if (!E) E |= gageUpdate(ctx);
  if (E) {
    fprintf(stderr, "%s: trouble:\n%s\n", me, biffGet(GAGE));
    exit(1);
  }
  gageParmSet(ctx, gageParmVerbose, 0);
  val = gageAnswerPointer(ctx, pvl, gageSclValue);
  
  for (zi=0; zi<sz; zi++) {
    for (yi=0; yi<sy; yi++) {
      for (xi=0; xi<sx; xi++) {
        
        /* convert to world space, use angle to determine new
           world space position, convert back to index space,
           clamp z to find inside old volume */
        
        y = (yi - sy/2.0)*ys;
        z = (zi*zs + y*sin(-angle*3.141592653/180.0))/zs;
        if (clamp || AIR_IN_OP(0, z, sz-1)) {
          z = AIR_CLAMP(0, z, sz-1);
          gageProbe(ctx,
                    AIR_CAST(gage_t, xi),
                    AIR_CAST(gage_t, yi),
                    AIR_CAST(gage_t, z));
          insert(out, xi + sx*(yi + sy*zi), *val);
        } else {
          insert(out, xi + sx*(yi + sy*zi), padval);
        }
      }
    }
  }

  if (nrrdSave(outS, nout, NULL)) {
    fprintf(stderr, "%s: trouble:\n%s\n", me, biffGet(NRRD));
    exit(1);
  }
  ctx = gageContextNix(ctx);
  hparm = hestParmFree(hparm);
  hopt = hestOptFree(hopt);
  nrrdNuke(nout);
  nrrdNuke(nin);
  
  exit(0);
}
