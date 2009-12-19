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

#define INFO "Generate twisting helical tensor field"
char *_tend_helixInfoL =
  (INFO
   ". The main utility of such a field is to expose the handedness of the "
   "raster coordinate frame, and to see if it agrees with the coordinate "
   "frame in which the tensor is expressed.  With the default settings, the "
   "region of high anisotropy is a right-handed helix (same as DNA), "
   "provided that the three axis sizes given to the \"-s\" option (below) "
   "correspond to a right-handed ordered basis (\"slow equals fast cross "
   "medium\").  In addition, the tensors twist relative to the helix. "
   "If you trace paths guided by the principal eigenvector of the tensors, "
   "along the surface of the helical cylinder, you get another "
   "right-handed helix, as if the the tensor field is modeling the result "
   "if twisting a set of fibers into single-straded helical bundle.  "
   "On the other hand, "
   "if you sample in a straight light along the diameter of the cylinder, "
   "the principal axis of the tensors twist in a *left*-handed helix. ");

void
tend_helixDoit(Nrrd *nout, float bnd,
               float r, float R, float S, float angle, float ev[3]) {
  int sx, sy, sz, xi, yi, zi, idx;
  double th, t0, t1, t2, t3, v1, v2,
    wpos[3], vpos[3],
    W2H[9], H2W[9], H2C[9], C2H[9], fv[3], rv[3], uv[3], mA[9], mB[9], inside,
    tmp[3], len;
  float *out;

  out = (float*)nout->data;
  sx = nout->axis[1].size;
  sy = nout->axis[2].size;
  sz = nout->axis[3].size;
  idx = 0;
  for (zi=0; zi<sz; zi++) {
    fprintf(stderr, "zi = %d/%d\n", zi, sz);
    vpos[2] = AIR_AFFINE(0, zi, sz-1, nout->axis[3].min, nout->axis[3].max);
    for (yi=0; yi<sy; yi++) {
      vpos[1] = AIR_AFFINE(0, yi, sy-1, nout->axis[2].min, nout->axis[2].max);
      for (xi=0; xi<sx; xi++, idx++) {
        vpos[0] = AIR_AFFINE(0, xi, sx-1, nout->axis[1].min,nout->axis[1].max);

#define WPOS(pos, th) ELL_3V_SET((pos), R*cos(th), R*sin(th), S*(th)/(2*AIR_PI))
#define VAL(th) (WPOS(wpos, th), ELL_3V_DIST(wpos, vpos))
#define RR 0.61803399
#define CC (1.0-RR)
#define SHIFT3(a,b,c,d) (a)=(b); (b)=(c); (c)=(d)
#define SHIFT2(a,b,c)   (a)=(b); (b)=(c)
        
        th = atan2(vpos[1], vpos[0]);
        th += 2*AIR_PI*floor(0.5 + vpos[2]/S - th/(2*AIR_PI));
        if (S*th/(2*AIR_PI) > vpos[2]) {
          t0 = th - AIR_PI; t3 = th;
        } else {
          t0 = th; t3 = th + AIR_PI;
        }
        t1 = RR*t0 + CC*t3;
        t2 = CC*t0 + RR*t3;
        v1 = VAL(t1);
        v2 = VAL(t2);
        while ( t3-t0 > 0.0001*(AIR_ABS(t1+t2)) ) {
          if (v1 < v2) {
            SHIFT3(t3, t2, t1, CC*t0 + RR*t2);
            SHIFT2(v2, v1, VAL(t1));
          } else {
            SHIFT3(t0, t1, t2, RR*t1 + CC*t3);
            SHIFT2(v1, v2, VAL(t2));
          }
        }

        /* well-written code is self-documenting */

        WPOS(wpos, t1);
        ELL_3V_SUB(wpos, vpos, wpos);
        ELL_3V_SET(fv, -R*sin(t1), R*cos(t1), S/AIR_PI);
        ELL_3V_NORM(fv, fv, len);
        ELL_3V_COPY(rv, wpos);
        ELL_3V_NORM(rv, rv, len);
        len = ELL_3V_DOT(rv, fv);
        ELL_3V_SCALE(tmp, -len, fv);
        ELL_3V_ADD2(rv, rv, tmp);
        ELL_3V_NORM(rv, rv, len);
        ELL_3V_CROSS(uv, rv, fv);
        ELL_3V_NORM(uv, uv, len);
        ELL_3MV_ROW0_SET(W2H, uv);
        ELL_3MV_ROW1_SET(W2H, rv);
        ELL_3MV_ROW2_SET(W2H, fv);
        ELL_3M_TRANSPOSE(H2W, W2H);
        inside = 0.5 - 0.5*airErf((ELL_3V_LEN(wpos)-r)/(bnd + 0.0001));
        th = angle*ELL_3V_LEN(wpos)/r;
        ELL_3M_ROTATE_Y_SET(H2C, th);
        ELL_3M_TRANSPOSE(C2H, H2C);
        ELL_3M_SCALE_SET(mA,
                         AIR_LERP(inside, 0.5, ev[1]),
                         AIR_LERP(inside, 0.5, ev[2]),
                         AIR_LERP(inside, 0.5, ev[0]));
        ELL_3M_MUL(mB, mA, H2C);
        ELL_3M_MUL(mA, mB, W2H);
        ELL_3M_MUL(mB, C2H, mA);
        ELL_3M_MUL(mA, H2W, mB);
        
        TEN_M2T_TT(out + 7*idx, float, mA);
        (out + 7*idx)[0] = 1.0;
      }
    }
  }
  return;
}

int
tend_helixMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  int size[3];
  Nrrd *nout;
  float R, r, S, bnd, angle, ev[3];
  double min[4], max[4];
  char *outS;

  hestOptAdd(&hopt, "s", "size", airTypeInt, 3, 3, size, NULL, 
             "sizes along fast, medium, and slow axes of the sampled volume, "
             "often called \"X\", \"Y\", and \"Z\".  It is best to use "
             "slightly different sizes here, to expose errors in interpreting "
             "axis ordering (e.g. \"-s 39 40 41\")");
  hestOptAdd(&hopt, "min", "min corner", airTypeDouble, 3, 3, min+1,
             "-2 -2 -2",
             "location of low corner of sampled tensor volume");
  hestOptAdd(&hopt, "max", "max corner", airTypeDouble, 3, 3, max+1, "2 2 2",
             "location of high corner of sampled tensor volume");
  hestOptAdd(&hopt, "b", "boundary", airTypeFloat, 1, 1, &bnd, "0.05",
             "parameter governing how fuzzy the boundary between high and "
             "low anisotropy is. Use \"-b 0\" for no fuzziness");
  hestOptAdd(&hopt, "r", "little radius", airTypeFloat, 1, 1, &r, "0.5",
             "(minor) radius of cylinder tracing helix");
  hestOptAdd(&hopt, "R", "big radius", airTypeFloat, 1, 1, &R, "1.2",
             "(major) radius of helical turns");
  hestOptAdd(&hopt, "S", "spacing", airTypeFloat, 1, 1, &S, "2",
             "spacing between turns of helix (along its axis)");
  hestOptAdd(&hopt, "a", "angle", airTypeFloat, 1, 1, &angle, "1.0",
             "maximal angle of twist of tensors along path.  There is no "
             "twist at helical core of path, and twist increases linearly "
             "with radius around this path.  Positive twist angle with "
             "positive spacing resulting in a right-handed twist around a "
             "right-handed helix. ");
  hestOptAdd(&hopt, "ev", "eigenvalues", airTypeFloat, 3, 3, ev, "0.9 0.4 0.2",
             "eigenvalues of tensors (in order) along direction of coil, "
             "circumferential around coil, and radial around coil. ");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output file");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_helixInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdAlloc_va(nout, nrrdTypeFloat, 4,
                   AIR_CAST(size_t, 7),
                   AIR_CAST(size_t, size[0]),
                   AIR_CAST(size_t, size[1]),
                   AIR_CAST(size_t, size[2]))) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble allocating output:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  min[0] = max[0] = AIR_NAN;
  nrrdAxisInfoSet_nva(nout, nrrdAxisInfoMin, min);
  nrrdAxisInfoSet_nva(nout, nrrdAxisInfoMax, max);

  tend_helixDoit(nout, bnd, r, R, S, angle, ev);
  nrrdAxisInfoSpacingSet(nout, 1);
  nrrdAxisInfoSpacingSet(nout, 2);
  nrrdAxisInfoSpacingSet(nout, 3);

  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
TEND_CMD(helix, INFO);
