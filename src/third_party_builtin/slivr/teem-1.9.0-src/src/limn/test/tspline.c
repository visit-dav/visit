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
test/tspline -loop -t 300 300 -s 10 -i s.txt:2v:bc:1,0 -m 1000 > ! out.ps 
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

char *info = ("Test limnSplines by drawing postscript curves. "
              "As such, the only limnSpline allowed is 2vector. "
              "The output is written to standard out.");

int
main(int argc, char *argv[]) {
  char *me, *err;
  limnSpline *spline, *warp;
  hestOpt *hopt=NULL;
  airArray *mop;
  int i, M, ret, pause, loop;
  Nrrd *nout, *ntmp;
  double *out, minT, maxT, scale, tran[2];

  mop = airMopNew();
  
  me = argv[0];
  hestOptAdd(&hopt, "i", "spline", airTypeOther, 1, 1, &spline, NULL,
             "the spline that we want to sample", NULL, NULL, limnHestSpline);
  hestOptAdd(&hopt, "w", "timewarp", airTypeOther, 1, 1, &warp, "",
             "how to (optionally) warp the spline domain",
             NULL, NULL, limnHestSpline);
  hestOptAdd(&hopt, "loop", NULL, airTypeInt, 0, 0, &loop, NULL,
             "the last control point is in fact the first");
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

  spline->loop = loop;
  if (!( limnSplineInfo2Vector == spline->info )) {
    fprintf(stderr, "%s: sorry, can only have %s info for PostScript\n",
            me, airEnumStr(limnSplineInfo, limnSplineInfo2Vector));
    airMopError(mop);
    return 1;
  }
  if (warp) {
    warp->loop = loop;
    if (!( limnSplineTypeTimeWarp == warp->type )) {
      fprintf(stderr, "%s: %s spline isn't; its %s\n", me,
              airEnumStr(limnSplineType, limnSplineTypeTimeWarp),
              airEnumStr(limnSplineType, warp->type));
      airMopError(mop);
      return 1;
    }
    if (loop) {
      if (!( limnSplineNumPoints(warp) == 1 + limnSplineNumPoints(spline) )) {
        fprintf(stderr, "%s: # warp points (%d) needs to be 1 more than "
                "# spline points (%d) for looping\n", me, 
                limnSplineNumPoints(warp), limnSplineNumPoints(spline));
        airMopError(mop);
        return 1;
      }
    } else {
      if (!( limnSplineNumPoints(warp) ==  limnSplineNumPoints(spline) )) {
        fprintf(stderr, "%s: # warp points (%d) != # spline points (%d)\n", me,
                limnSplineNumPoints(warp), limnSplineNumPoints(spline));
                
        airMopError(mop);
        return 1;
      }
    }
  }
  
  airMopAdd(mop, nout=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntmp=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (warp) {
    minT = limnSplineMinT(warp);
    maxT = limnSplineMaxT(warp);
    ret = (limnSplineSample(ntmp, warp, minT, M, maxT)
           || limnSplineNrrdEvaluate(nout, spline, ntmp));
  } else {
    minT = limnSplineMinT(spline);
    maxT = limnSplineMaxT(spline);
    ret = limnSplineSample(nout, spline, minT, M, maxT);
  }
  if (ret) {
    airMopAdd(mop, err=biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }

  out = (double*)(nout->data);
  pause = M/150;
  printf("%%!\n");
  printf("1 setlinewidth\n");
  printf("%g %g moveto\n",
         scale*out[0 + 2*0] + tran[0], scale*out[1 + 2*0] + tran[1]);
  printf("gsave\n");
  printf("0.2 setlinewidth\n");
  printf("currentpoint newpath 3 0 360 arc stroke\n");
  printf("grestore\n");
  for (i=1; i<M; i++) {
    printf("%g %g lineto\n",
           scale*out[0 + 2*i] + tran[0], scale*out[1 + 2*i] + tran[1]);
    if (0 == AIR_MOD(i, pause)) {
      printf("gsave\n");
      printf("0.2 setlinewidth\n");
      printf("currentpoint newpath 3 0 360 arc stroke\n");
      printf("grestore\n");
    }
  }
  printf("stroke\n");
  printf("showpage\n");
  
  airMopOkay(mop);
  return 0;
}

