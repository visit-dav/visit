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


#include "../limn.h"

char *info = ("Save a triangular piece of an image to an EPS file. "
              "You might want to ilk -t 1,-0.5,0,0,0.866,0 -k tent "
              "-0 u:0,1 -b pad -bg 0 before you use this. ");

int
main(int argc, char *argv[]) {
  char *me;
  hestOpt *hopt=NULL;
  airArray *mop;

  NrrdIoState *nio;
  FILE *file;
  char *outS;  
  Nrrd *nin;
  float width, scale, hack, minX, maxX, minY, maxY;
  int gray, sx, sy, labels;

  mop = airMopNew();
  me = argv[0];
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input image.  Must be SQUARE 8-bit RGB or gray",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "w", "width", airTypeFloat, 1, 1, &width, "0.0",
             "border width to put around triangle, in pixels, "
             "or \"0\" to not have any border");
  hestOptAdd(&hopt, "labels", NULL, airTypeInt, 0, 0, &labels, NULL,
             "put little labels on things; fix with psfrag in LaTeX");
  hestOptAdd(&hopt, "o", "output EPS", airTypeString, 1, 1, &outS, NULL,
             "output file to render postscript into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  if (!( 2 == nin->dim || 3 == nin->dim )) {
    fprintf(stderr, "%s: input nrrd must be 2-D or 3-D (not %d-D)\n", me,
            nin->dim);
    airMopError(mop); return 1;
  }
  if (!( nrrdTypeUChar == nin->type )) {
    fprintf(stderr, "%s: input nrrd must be type %s (not %s)\n", me,
            airEnumStr(nrrdType, nrrdTypeUChar),
            airEnumStr(nrrdType, nin->type));
    airMopError(mop); return 1;
  }
  sx = (2 == nin->dim ? nin->axis[0].size : nin->axis[1].size);
  sy = (2 == nin->dim ? nin->axis[1].size : nin->axis[2].size);
  gray = 2 == nin->dim || 1 == nin->axis[0].size;
  if (!( sx == sy )) {
    fprintf(stderr, "%s: image must be square (not %d x %d)\n", me, sx, sy);
    airMopError(mop); return 1;
  }

  if (!( file = airFopen(outS, stdout, "wb") )) {
    fprintf(stderr, "%s: couldn't open \"%s\" for writing", me, outS);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, file, (airMopper)airFclose, airMopAlways);
  nio = nrrdIoStateNew();
  airMopAdd(mop, nio, (airMopper)nrrdIoStateNix, airMopAlways);

  hack = sqrt(3)/2;
  /* sorry, copied from nrrd/formatEPS.c */
  minX = 0.5;
  maxX = 8.0;
  minY = 5.50 - 7.5*hack*sy/sx/2;
  maxY = 5.50 + 7.5*hack*sy/sx/2;
  scale = 7.5/sx;
  minX *= 72; minY *= 72;
  maxX *= 72; maxY *= 72;
  scale *= 72;
  fprintf(file, "%%!PS-Adobe-3.0 EPSF-3.0\n");
  fprintf(file, "%%%%Creator: hairy pathetic monster\n");
  fprintf(file, "%%%%Title: raving lunatic\n");
  fprintf(file, "%%%%Pages: 1\n");
  fprintf(file, "%%%%BoundingBox: %d %d %d %d\n",
          (int)floor(minX), (int)floor(minY),
          (int)ceil(maxX), (int)ceil(maxY));
  fprintf(file, "%%%%HiResBoundingBox: %g %g %g %g\n", 
          minX, minY, maxX, maxY);
  fprintf(file, "%%%%EndComments\n");
  fprintf(file, "%%%%BeginProlog\n");
  fprintf(file, "%% linestr creates an empty string to hold "
          "one scanline\n");
  fprintf(file, "/linestr %d string def\n", sx*(gray ? 1 : 3));
  fprintf(file, "%%%%EndProlog\n");
  fprintf(file, "%%%%Page: 1 1\n");
  fprintf(file, "gsave\n");

  fprintf(file, "%g %g moveto\n", minX, minY);
  fprintf(file, "%g %g lineto\n", maxX, minY);
  fprintf(file, "%g %g lineto\n",
          (minX + maxX)/2, minY + hack*(maxX - minX));
  fprintf(file, "closepath\n");
  fprintf(file, "clip\n");
  fprintf(file, "gsave newpath\n");

  fprintf(file, "%g %g translate\n", minX, minY);
  fprintf(file, "%g %g scale\n", sx*scale, sy*scale);
  fprintf(file, "%d %d 8\n", sx, sy);
  fprintf(file, "[%d 0 0 -%d 0 %d]\n", sx, sy, sy);
  fprintf(file, "{currentfile linestr readhexstring pop} %s\n",
          gray ? "image" : "false 3 colorimage");
  nrrdEncodingHex->write(file, nin->data, nrrdElementNumber(nin),
                         nin, nio);

  nio->dataFile = NULL;
  fprintf(file, "\n");

  fprintf(file, "grestore\n");
  if (width) {
    fprintf(file, "%g %g moveto\n", minX, minY);
    fprintf(file, "%g %g lineto\n", maxX, minY);
    fprintf(file, "%g %g lineto\n",
            (minX + maxX)/2, minY + hack*(maxX - minX));
    fprintf(file, "closepath\n");
    fprintf(file, "%g setlinewidth 0 setgray stroke\n",
            2*width*scale);
  }
  if (labels) {
    /* happily, psfrag doesn't respect the clipping path */
    fprintf(file, "/Helvetica findfont 20 scalefont setfont\n");
    fprintf(file, "%g %g moveto (A) show\n", maxX, minY);
    fprintf(file, "%g %g moveto (B) show\n", (minX + maxX)/2,
            minY + hack*(maxX - minX));
    fprintf(file, "%g %g moveto (C) show\n", minX, minY);
  }

  fprintf(file, "grestore\n");
  
  airMopOkay(mop);
  return 0;
}

