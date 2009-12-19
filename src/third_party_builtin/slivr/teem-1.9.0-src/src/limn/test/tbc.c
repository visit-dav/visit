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


/*
test/tbc -s 0.95 -i s.txt -loop -n 13 -m 800 -t 55 170 > ! out.ps
*/

/* s.txt:
-10 -20
-20 -10
-10  0
-20  10
-10  20
 0   10
 10  20
 20  10
 10  0
 20 -10
 10 -20
 0  -10
 10  0
 0   10
-10  0
 0  -10
*/


#include "../limn.h"

char *info = ("Visualize the space of BC cubics with a spline.");

int
main(int argc, char *argv[]) {
  char *me, *err;
  limnSpline *spline;
  hestOpt *hopt=NULL;
  airArray *mop;
  int bi, ci, i, N, M, loop;
  Nrrd *ncptA, *ncptB, *nout;
  double *out, minT, maxT, scale, tran[2], B, C;
  limnSplineTypeSpec *spec;

  mop = airMopNew();
  
  me = argv[0];
  hestOptAdd(&hopt, "i", "spline data", airTypeOther, 1, 1, &ncptA, NULL,
             "data points for the spline, must be 2-vectors",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "loop", NULL, airTypeInt, 0, 0, &loop, NULL,
             "the last control point is in fact the first");
  hestOptAdd(&hopt, "n", "N", airTypeInt, 1, 1, &N, "10",
             "how many samples along each edge of BC space");
  hestOptAdd(&hopt, "m", "M", airTypeInt, 1, 1, &M, "512",
             "the number of sample points at which to evalute the spline");
  hestOptAdd(&hopt, "t", "tx ty", airTypeDouble, 2, 2, tran, "0.0 0.0",
             "translation for drawing");
  hestOptAdd(&hopt, "s", "scale", airTypeDouble, 1, 1, &scale, "1.0",
             "scaling for drawing");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  if (!( 2 == ncptA->dim && 2 == ncptA->axis[0].size )) {
    fprintf(stderr, "%s: didn't get a 2-D 2xN nrrd)\n", me);
    airMopError(mop);
    return 1;
  }
  spec = limnSplineTypeSpecNew(limnSplineTypeBC, 0, 0);
  airMopAdd(mop, ncptB=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (limnSplineNrrdCleverFix(ncptB, ncptA, limnSplineInfo2Vector,
                              limnSplineTypeBC)
      || !(spline = limnSplineNew(ncptB, limnSplineInfo2Vector, spec))) {
    airMopAdd(mop, err=biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }
  spline->loop = loop;
  airMopAdd(mop, spline, (airMopper)limnSplineNix, airMopAlways);
  airMopAdd(mop, nout=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  minT = limnSplineMinT(spline);
  maxT = limnSplineMaxT(spline);
  
  /* try one for error checking */
  if (limnSplineSample(nout, spline, minT, M, maxT)) {
    airMopAdd(mop, err=biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }

  printf("%%!\n");
  printf("1 setlinewidth\n");
  printf("%g %g translate\n", tran[0], tran[1]);

  for (ci=0; ci<N; ci++) {
    C = AIR_AFFINE(0, ci, N-1, 0.0, 1.0);
    for (bi=0; bi<N; bi++) {
      B = AIR_AFFINE(0, bi, N-1, 0.0, 1.0);
      
      limnSplineBCSet(spline, B, C);
      limnSplineSample(nout, spline, minT, M, maxT);
      out = (double*)(nout->data);   /* shouldn't actually change */
      
      printf("gsave\n");
      printf("%g %g translate\n", bi*500.0/(N-1), ci*500.0/(N-1));
      printf("%g %g scale\n", scale, scale);
      printf("%g %g moveto\n", out[0 + 2*0], out[1 + 2*0]);
      for (i=1; i<M; i++) {
        printf("%g %g lineto\n", out[0 + 2*i], out[1 + 2*i]);
      }
      if (spline->loop) {
        printf("closepath\n");
      }
      printf("stroke\n");
      
      printf("grestore\n");
    }
  }

  printf("showpage\n");
  
  airMopOkay(mop);
  return 0;
}

