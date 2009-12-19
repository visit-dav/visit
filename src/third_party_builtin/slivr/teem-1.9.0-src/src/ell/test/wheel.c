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

#include "../ell.h"

char *wheelInfo = ("Makes pictures of the eigenvalue wheel");

void
wheelTenToGeom(double geom[3], double a, double b, double c,
               double d, double e, double f) {
  double A, B, C, Q, QQQ, R, Th;

  A = -a - d - f;
  B = a*d + a*f + d*f - b*b - c*c - e*e;
  C = c*c*d + a*e*e + b*b*f - 2*b*c*e - a*d*f;
  Q = (A*A - 3*B)/9;
  R = (9*A*B - 27*C - 2*A*A*A)/54;
  QQQ = Q*Q*Q;
  Th = QQQ ? acos(R/sqrt(QQQ))/3 : 0;
  geom[0] = -A/3;
  geom[1] = 2*sqrt(Q);
  geom[2] = 180.0*Th/AIR_PI;
  return;
}

void
wheelABCToGeom(double *geom, double A, double B, double C) {
  double Q, R, QQQ, Th;

  Q = (A*A - 3*B)/9;
  R = (9*A*B - 27*C - 2*A*A*A)/54;
  QQQ = Q*Q*Q;
  Th = QQQ ? acos(R/sqrt(QQQ))/3 : 0;
  geom[0] = -A/3;
  geom[1] = 2*sqrt(Q);
  geom[2] = 180.0*Th/AIR_PI;
  return;
}

void
wheelGeomToRoot(double xroot[3], double yroot[3], double geom[3]) {
  double Th;

  Th = AIR_PI*geom[2]/180.0;
  xroot[0] = geom[0] + geom[1]*cos(Th);
  xroot[1] = geom[0] + geom[1]*cos(Th - 2*AIR_PI/3);
  xroot[2] = geom[0] + geom[1]*cos(Th + 2*AIR_PI/3);
  yroot[0] = geom[1]*sin(Th);
  yroot[1] = geom[1]*sin(Th - 2*AIR_PI/3);
  yroot[2] = geom[1]*sin(Th + 2*AIR_PI/3);
  return;
}

void
wheelGeomToABC(double ABC[3], double center, double radius, double angle) {
  double geom[3], x[3], yroot[3];

  ELL_3V_SET(geom, center, radius, angle);
  wheelGeomToRoot(x, yroot, geom);
  ELL_3V_SET(ABC,
             -x[0] - x[1] - x[2],
             x[0]*x[1] + x[1]*x[2] + x[0]*x[2],
             -x[0]*x[1]*x[2]);
  return;
}

void
wheelGeomToRNTH(double rnth[3], double center, double radius, double angle) {
  double mu1, mu2;

  mu1 = center;
  mu2 = radius*radius/2;
  rnth[0] = sqrt(mu2)/(sqrt(2)*mu1);
  rnth[1] = sqrt(3*(mu1*mu1 + mu2));
  rnth[2] = angle;
}

typedef struct {
  FILE *file;
  double psc;
  double bbox[4];
  
  double maxX, maxY, yscale;
} wheelPS;

#define WPS_X(x) AIR_AFFINE(wps->bbox[0], (x), wps->bbox[2], 0, wps->maxX)
#define WPS_Y(y) AIR_AFFINE(wps->bbox[1], (y), wps->bbox[3], 0, wps->maxY)
#define WPS_S(s)  AIR_DELTA(wps->bbox[1], (s), wps->bbox[3], 0, wps->maxY)

void
wheelPreamble(wheelPS *wps) {

  wps->maxX = wps->psc*(wps->bbox[2] - wps->bbox[0]);
  wps->maxY = wps->psc*(wps->bbox[3] - wps->bbox[1]);

  fprintf(wps->file, "%%!PS-Adobe-2.0 EPSF-2.0\n");
  fprintf(wps->file, "%%%%Creator: limn\n");
  fprintf(wps->file, "%%%%Pages: 1\n");
  fprintf(wps->file, "%%%%BoundingBox: 0 0 %d %d\n", 
          (int)(wps->maxX),
          (int)(wps->maxY));
  fprintf(wps->file, "%%%%EndComments\n");
  fprintf(wps->file, "%%%%EndProlog\n");
  fprintf(wps->file, "%%%%Page: 1 1\n");
  fprintf(wps->file, "gsave\n");
  fprintf(wps->file, "0 0 moveto\n");
  fprintf(wps->file, "%g 0 lineto\n", wps->maxX);
  fprintf(wps->file, "%g %g lineto\n", wps->maxX, wps->maxY);
  fprintf(wps->file, "0 %g lineto\n", wps->maxY);
  fprintf(wps->file, "closepath\n");
  fprintf(wps->file, "clip\n");
  fprintf(wps->file, "gsave newpath\n");
  fprintf(wps->file, "1 setlinejoin\n");
  fprintf(wps->file, "1 setlinecap\n");
  fprintf(wps->file, "/M {moveto} bind def\n");
  fprintf(wps->file, "/L {lineto} bind def\n");
  fprintf(wps->file, "/W {setlinewidth} bind def\n");
  fprintf(wps->file, "/F {fill} bind def\n");
  fprintf(wps->file, "/S {stroke} bind def\n");
  fprintf(wps->file, "/CP {closepath} bind def\n");
  fprintf(wps->file, "/RGB {setrgbcolor} bind def\n");
  fprintf(wps->file, "/Gr {setgray} bind def\n");
  fprintf(wps->file, "\n");
  return;
}

void
wheelWidth(wheelPS *wps, double width) {

  fprintf(wps->file, "%g W\n", width);
  return;
}

void
wheelGray(wheelPS *wps, double gray) {

  fprintf(wps->file, "%g Gr\n", gray);
  return;
}

void
wheelLabel(wheelPS *wps, double x, double y, char *str) {
  
  fprintf(wps->file, "%g %g M (%s) show\n", WPS_X(x), WPS_Y(y), str);
  return;
}

void
wheelGraph(wheelPS *wps, double a, double d, double f) {
  double A, B, C;
  int xi;
  double x, y;
  
  A = -a - d - f;
  B = a*d + a*f + d*f;
  C = -a*d*f;
  for (xi=0; xi<=99; xi++) {
    x = AIR_AFFINE(0, xi, 99, wps->bbox[0], wps->bbox[2]);
    y = (((x + A)*x + B)*x + C)/2;
    fprintf(wps->file, "%g %g %s\n", WPS_X(x), WPS_Y(wps->yscale*y),
            xi ? "L" : "M");
  }
  fprintf(wps->file, "S\n");
  return;
}

void
wheelLine(wheelPS *wps, double x0, double y0, double x1, double y1) {
  
  fprintf(wps->file, "%g %g M\n", WPS_X(x0), WPS_Y(y0));
  fprintf(wps->file, "%g %g L S\n", WPS_X(x1), WPS_Y(y1));
  return;
}

void
wheelCircle(wheelPS *wps, double xc, double yc, double rad) {

  fprintf(wps->file, "%g %g %g 0 360 arc closepath S\n", 
          WPS_X(xc), WPS_Y(yc), WPS_S(rad));
  return;
}

void
wheelArc(wheelPS *wps, double xc, double yc, double rad,
         double angle1, double angle2) {
  
  fprintf(wps->file, "newpath %g %g %g %g %g arc S\n", 
          WPS_X(xc), WPS_Y(yc), WPS_S(rad), angle1, angle2);
}

void
wheelDot(wheelPS *wps, double x, double y, double rad) {

  fprintf(wps->file, "%g %g %g 0 360 arc closepath F S\n", 
          WPS_X(x), WPS_Y(y), WPS_S(rad));
  return;
}

void
wheelEpilog(wheelPS *wps) {

  fprintf(wps->file, "grestore\n");
  fprintf(wps->file, "grestore\n");
  fprintf(wps->file, "%%%%Trailer\n");
  return;
}

int
main(int argc, char *argv[]) {
  char *me, *outS;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;
  
  double tval[6], ABC[3], geom[3], rnth[3], RA, norm, mu2, tmpr=0, tmpa=0,
    xroot[3], yroot[3], bbox[4], htick, htth, psc;
  wheelPS wps;
  int correct, labels, drawRA;

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hparm->elideMultipleNonExistFloatDefault = AIR_TRUE;
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "t", "a b c d e f", airTypeDouble, 6, 6, tval,
             "nan nan nan nan nan nan nan",
             "six values of symmetric tensors");
  hestOptAdd(&hopt, "ABC", "A B C", airTypeDouble, 3, 3, ABC, "nan nan nan",
             "directly give coefficients of cubic polynomial "
             "(and override info from \"-t\")");
  hestOptAdd(&hopt, "g", "c rad th", airTypeDouble, 3, 3, geom, "nan nan nan",
             "directly give center, radius, and angle (in degrees) of wheel "
             "(and override info from \"-t\" and \"-ABC\"");
  hestOptAdd(&hopt, "p", "RA norm th", airTypeDouble, 3, 3, rnth, 
             "nan nan nan",
             "directly give RA, norm, and angle (in degrees) of tensor "
             "(and override info from \"-t\", \"-ABC\", and \"-geom\"");
  hestOptAdd(&hopt, "correct", NULL, airTypeInt, 0, 0, &correct, NULL,
             "when using \"-g\", be honest about what the estimated "
             "acos(sqrt(2)*skew)/3 is going to be");
  hestOptAdd(&hopt, "labels", NULL, airTypeInt, 0, 0, &labels, NULL,
             "put little labels on things; fix with psfrag in LaTeX");
  hestOptAdd(&hopt, "RA", NULL, airTypeInt, 0, 0, &drawRA, NULL,
             "draw extra geometry associated with RA");
  hestOptAdd(&hopt, "htick", "pos", airTypeDouble, 1, 1, &htick, "nan",
             "location of single tick mark on horizontal axis");
  hestOptAdd(&hopt, "htth", "thick", airTypeDouble, 1, 1, &htth, "3",
             "thickness of horizontal tick");
  hestOptAdd(&hopt, "bb", "bbox", airTypeDouble, 4, 4, bbox, 
             "nan nan nan nan", "bounding box, in world space around the "
             "region of the graph that should be drawn to EPS");
  hestOptAdd(&hopt, "ysc", "scale", airTypeDouble, 1, 1, &(wps.yscale), "0.5",
             "scaling on Y axis for drawing graph of characteristic "
             "polynomial, or \"0\" to turn this off.");
  hestOptAdd(&hopt, "psc", "scale", airTypeDouble, 1, 1, &psc, "100",
             "scaling from world space to PostScript points");
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &outS, "-",
             "file to write EPS output to");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, wheelInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!(wps.file = airFopen(outS, stdout, "wb"))) {
    fprintf(stderr, "%s: couldn't open output file\n", me);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, wps.file, (airMopper)airFclose, airMopAlways);

  if (AIR_EXISTS(rnth[0])) {
    RA = rnth[0];
    norm = rnth[1];
    mu2 = (norm*norm/3)*(2*RA*RA/(1 + 2*RA*RA));
    geom[0] = sqrt(mu2)/(sqrt(2)*RA);
    geom[1] = sqrt(2*mu2);
    geom[2] = rnth[2];
    wheelGeomToRoot(xroot, yroot, geom);
    wheelGeomToABC(ABC, geom[0], geom[1], geom[2]);
  } else if (AIR_EXISTS(geom[0])) {
    wheelGeomToRoot(xroot, yroot, geom);
    if (correct) {
      tval[0] = xroot[0];
      tval[1] = tval[2] = 0;
      tval[3] = xroot[1];
      tval[4] = 0;
      tval[5] = xroot[2];
      wheelTenToGeom(geom,
                     tval[0], tval[1], tval[2], tval[3], tval[4], tval[5]);
      wheelGeomToRoot(xroot, yroot, geom);
    }
    wheelGeomToABC(ABC, geom[0], geom[1], geom[2]);
    wheelGeomToRNTH(rnth, geom[0], geom[1], geom[2]);
  } else if (AIR_EXISTS(ABC[0])) {
    wheelABCToGeom(geom, ABC[0], ABC[1], ABC[2]);
    wheelGeomToRNTH(rnth, geom[0], geom[1], geom[2]);
    wheelGeomToRoot(xroot, yroot, geom);
  } else {
    wheelTenToGeom(geom, tval[0], tval[1], tval[2], tval[3], tval[4], tval[5]);
    wheelGeomToRoot(xroot, yroot, geom);
    wheelGeomToRNTH(rnth, geom[0], geom[1], geom[2]);
    wheelGeomToABC(ABC, geom[0], geom[1], geom[2]);
  }
  fprintf(stderr, "%s: RNTH: %g %g %g\n", me, rnth[0], rnth[1], rnth[2]);
  fprintf(stderr, "%s: ABC: %g %g %g\n", me, ABC[0], ABC[1], ABC[2]);
  fprintf(stderr, "%s: xroot: %g %g %g\n",
          me, xroot[0], xroot[1], xroot[2]);
  fprintf(stderr, "%s: geom: %g %g %g\n", me, geom[0], geom[1], geom[2]);

  if (!AIR_EXISTS(bbox[0])) {
    bbox[0] = geom[0] - 1.2*geom[1];
    bbox[1] = - 1.2*geom[1];
    bbox[2] = geom[0] + 1.2*geom[1];
    bbox[3] = + 1.2*geom[1];
    fprintf(stderr, "%s: bbox %g %g %g %g\n", me,
            bbox[0], bbox[1], bbox[2], bbox[3]);
  }
  wps.psc = psc;
  ELL_4V_COPY(wps.bbox, bbox);
  wheelPreamble(&wps);

  /* graph */
  if (wps.yscale) {
    wheelWidth(&wps, 4);
    wheelGray(&wps, 0.5);
    wheelGraph(&wps, xroot[0], xroot[1], xroot[2]);
  }

  /* axis */
  wheelWidth(&wps, 2);
  wheelGray(&wps, 0.0);
  wheelLine(&wps, bbox[0], 0, bbox[2], 0);

  /* circle */
  wheelWidth(&wps, 3);
  wheelCircle(&wps, geom[0], 0, geom[1]);

  /* spokes */
  wheelWidth(&wps, 4);
  wheelLine(&wps, geom[0], 0, xroot[0], yroot[0]);
  wheelLine(&wps, geom[0], 0, xroot[1], yroot[1]);
  wheelLine(&wps, geom[0], 0, xroot[2], yroot[2]);
  
  /* dots at spoke ends */
  wheelDot(&wps, xroot[0], yroot[0], 0.025*geom[1]);
  wheelDot(&wps, xroot[1], yroot[1], 0.025*geom[1]);
  wheelDot(&wps, xroot[2], yroot[2], 0.025*geom[1]);

  /* lines from dots to roots */
  wheelWidth(&wps, 2);
  fprintf(wps.file, "gsave\n");
  fprintf(wps.file, "[2 4] 0 setdash\n");
  wheelLine(&wps, xroot[0], 0, xroot[0], yroot[0]);
  wheelLine(&wps, xroot[1], 0, xroot[1], yroot[1]);
  wheelLine(&wps, xroot[2], 0, xroot[2], yroot[2]);
  fprintf(wps.file, "grestore\n");

  /* tickmarks */
  wheelWidth(&wps, 6);
  wheelLine(&wps, xroot[0], -0.02*geom[1], xroot[0], 0.02*geom[1]);
  wheelLine(&wps, xroot[1], -0.02*geom[1], xroot[1], 0.02*geom[1]);
  wheelLine(&wps, xroot[2], -0.02*geom[1], xroot[2], 0.02*geom[1]);
  if (AIR_EXISTS(htick)) {
    wheelWidth(&wps, htth);
    wheelLine(&wps, htick, -0.04, htick, 0.04);
  }

  /* RA angle */
  if (drawRA) {
    wheelWidth(&wps, 3);
    wheelLine(&wps, 0.0, 0.0, geom[0], geom[1]);
    wheelWidth(&wps, 2);
    fprintf(wps.file, "gsave\n");
    fprintf(wps.file, "[2 4] 0 setdash\n");
    wheelLine(&wps, geom[0], geom[1], geom[0], 0);
    fprintf(wps.file, "grestore\n");
  }

  /* labels, if wanted */
  if (labels) {
    fprintf(wps.file, "/Helvetica findfont 20 scalefont setfont\n");
    wheelLabel(&wps, geom[0], 0, "center");
    wheelLine(&wps, geom[0], -0.02*geom[1], geom[0], 0.02*geom[1]);
    wheelLabel(&wps, (geom[0] + xroot[0])/1.8, yroot[0]/1.8, "radius");
    wheelWidth(&wps, 2);
    wheelArc(&wps, geom[0], 0, geom[1]/2, 0, geom[2]);
    wheelLabel(&wps, geom[0] + geom[1]*cos(AIR_PI*geom[2]/180/2)/2.5, 
               geom[1]*sin(AIR_PI*geom[2]/180/2)/2.5, "theta");
    if (drawRA) {
      tmpr = sqrt(geom[0]*geom[0] + geom[1]*geom[1]);
      tmpa = atan(2.0*rnth[0]);
      wheelWidth(&wps, 2);
      wheelArc(&wps, 0, 0, 0.2*tmpr, 0, 180*tmpa/AIR_PI);
      wheelLabel(&wps, 0.2*tmpr*cos(tmpa/2), 0.2*tmpr*sin(tmpa/2), "phi");
    }
    wheelLabel(&wps, xroot[0], yroot[0], "spoke0");
    wheelLabel(&wps, xroot[1], yroot[1], "spoke-");
    wheelLabel(&wps, xroot[2], yroot[2], "spoke+");
    wheelLabel(&wps, xroot[0], 0, "root0");
    wheelLabel(&wps, xroot[1], 0, "root-");
    wheelLabel(&wps, xroot[2], 0, "root+");
  }

  wheelEpilog(&wps);

  airMopOkay(mop);
  exit(0);
}
