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

#define INFO "Generate postscript renderings of 2D glyphs"
char *_tend_ellipseInfoL =
  (INFO
   ".  Not much to look at here.");

int
tend_ellipseDoit(FILE *file, Nrrd *nten, Nrrd *npos, Nrrd *nstn,
                 float min[2], float max[2],
                 float gscale, float dotRad, float lineWidth, float cthresh,
                 int invert) {
  size_t sx=0, sy=0, ti, nt;
  int x, y, vi, *sdata;
  double aspect, minX, minY, maxX, maxY, conf, Dxx, Dxy, Dyy, px, py, spx, spy;
  float *tdata, *pdata;
  
  if (npos) {
    nt = npos->axis[1].size;
    aspect = (max[0] - min[0])/(max[1] - min[1]);
  } else {
    spx = (AIR_EXISTS(nten->axis[1].spacing)
           ? nten->axis[1].spacing
           : 1);
    spy = (AIR_EXISTS(nten->axis[2].spacing)
           ? nten->axis[2].spacing
           : 1);
    sx = nten->axis[1].size;
    sy = nten->axis[2].size;
    nt = sx*sy;
    aspect = sx*spx/(sy*spy);
  }

  if (aspect > 7.5/10) {
    /* image has a wider aspect ratio than safely printable page area */
    minX = 0.5;
    maxX = 8.0;
    minY = 5.50 - 7.5/2/aspect;
    maxY = 5.50 + 7.5/2/aspect;
  } else {
    /* image is taller ... */
    minX = 4.25 - 10.0/2*aspect;
    maxX = 4.25 + 10.0/2*aspect;
    minY = 0.5;
    maxY = 10.5;
  }
  minX *= 72; minY *= 72;
  maxX *= 72; maxY *= 72;
  if (npos) {
    gscale *= AIR_CAST(float, (maxX - minX)/(max[0] - min[0]));
    dotRad *= AIR_CAST(float, (maxX - minX)/(max[0] - min[0]));
    lineWidth *= AIR_CAST(float, (maxX - minX)/(max[0] - min[0]));
  }

  fprintf(file, "%%!PS-Adobe-3.0 EPSF-3.0\n");
  fprintf(file, "%%%%Creator: tend ellipse\n");
  fprintf(file, "%%%%Title: blah blah blah\n");
  fprintf(file, "%%%%Pages: 1\n");
  fprintf(file, "%%%%BoundingBox: %d %d %d %d\n",
          AIR_CAST(int, floor(minX)), AIR_CAST(int, floor(minY)),
          AIR_CAST(int, ceil(maxX)), AIR_CAST(int, ceil(maxY)));
  fprintf(file, "%%%%HiResBoundingBox: %g %g %g %g\n", 
          minX, minY, maxX, maxY);
  fprintf(file, "%%%%EndComments\n");
  fprintf(file, "%%%%BeginProlog\n");
  fprintf(file, "%%%%EndProlog\n");
  fprintf(file, "%%%%Page: 1 1\n");

  fprintf(file, "gsave\n");

  if (invert) {
    fprintf(file, "0 setgray\n");
    fprintf(file, "%g %g moveto\n", minX, minY);
    fprintf(file, "%g %g lineto\n", maxX, minY);
    fprintf(file, "%g %g lineto\n", maxX, maxY);
    fprintf(file, "%g %g lineto\n", minX, maxY);
    fprintf(file, "closepath fill\n");
  }

  fprintf(file, "gsave\n");
  fprintf(file, "0.5 setgray\n");
  tdata = (float*)nten->data;
  pdata = npos ? (float*)npos->data : NULL;
  for (ti=0; ti<nt; ti++) {
    if (npos) {
      px = AIR_AFFINE(min[0], pdata[0], max[0], minX, maxX);
      py = AIR_AFFINE(min[1], pdata[1], max[1], maxY, minY);
      pdata += 2;
    } else {
      x = ti % sx;
      y = ti / sx;
      px = NRRD_CELL_POS(minX, maxX, sx, x);
      py = NRRD_CELL_POS(minY, maxY, sy, sy-1-y);
    }
    conf = tdata[0];
    if (conf > cthresh) {
      Dxx = tdata[1];
      Dxy = tdata[2];
      Dyy = tdata[3];
      fprintf(file, "gsave\n");
      fprintf(file, "matrix currentmatrix\n");
      fprintf(file, "[%g %g %g %g %g %g] concat\n",
              Dxx, -Dxy, -Dxy, Dyy, px, py);
      fprintf(file, "0 0 %g 0 360 arc closepath\n", gscale);
      fprintf(file, "setmatrix\n");
      fprintf(file, "fill\n");
      fprintf(file, "grestore\n");
    }
    tdata += 4;
  }
  fprintf(file, "grestore\n");

  if (dotRad && !nstn) {
    fprintf(file, "gsave\n");
    tdata = (float*)nten->data;
    pdata = npos ? (float*)npos->data : NULL;
    fprintf(file, "%g setgray\n", invert ? 1.0 : 0.0);
    for (ti=0; ti<nt; ti++) {
      if (npos) {
        px = AIR_AFFINE(min[0], pdata[0], max[0], minX, maxX);
        py = AIR_AFFINE(min[1], pdata[1], max[1], maxY, minY);
        pdata += 2;
      } else {
        x = ti % sx;
        y = ti / sx;
        px = NRRD_CELL_POS(minX, maxX, sx, x);
        py = NRRD_CELL_POS(minY, maxY, sy, sy-1-y);
      }
      conf = tdata[0];
      if (conf > cthresh) {
        fprintf(file, "%g %g %g 0 360 arc closepath fill\n", px, py, dotRad);
      }
      tdata += 4;
    }
    fprintf(file, "grestore\n");
  }

  if ((dotRad || lineWidth) && npos && nstn) {
    fprintf(file, "gsave\n");
    tdata = (float*)nten->data;
    pdata = npos ? (float*)npos->data : NULL;
    sdata = nstn ? (int*)nstn->data : NULL;
    fprintf(file, "%g setlinewidth\n", lineWidth);
    fprintf(file, "%g setgray\n", invert ? 1.0 : 0.0);
    fprintf(file, "1 setlinecap\n");
    fprintf(file, "1 setlinejoin\n");
    for (ti=0; ti<nstn->axis[1].size; ti++) {
      if (1 == sdata[1 + 3*ti]) {
        vi = sdata[0 + 3*ti];
        px = AIR_AFFINE(min[0], pdata[0 + 2*vi], max[0], minX, maxX);
        py = AIR_AFFINE(min[1], pdata[1 + 2*vi], max[1], maxY, minY);
        if (tdata[0 + 4*vi] > cthresh) {
          fprintf(file, "%g %g %g 0 360 arc closepath fill\n", px, py, dotRad);
        }
      } else {
        fprintf(file, "newpath\n");
        for (vi = sdata[0 + 3*ti];
             vi < sdata[0 + 3*ti] + sdata[1 + 3*ti];
             vi++) {
          px = AIR_AFFINE(min[0], pdata[0 + 2*vi], max[0], minX, maxX);
          py = AIR_AFFINE(min[1], pdata[1 + 2*vi], max[1], maxY, minY);
          fprintf(file, "%g %g %s\n", px, py,
                  vi == sdata[0 + 3*ti] ? "moveto" : "lineto");
        }
        fprintf(file, "stroke\n");
        vi = sdata[0 + 3*ti] + sdata[2 + 3*ti];
        px = AIR_AFFINE(min[0], pdata[0 + 2*vi], max[0], minX, maxX);
        py = AIR_AFFINE(min[1], pdata[1 + 2*vi], max[1], maxY, minY);
        fprintf(file, "%g %g %g 0 360 arc closepath fill\n",
                px, py, dotRad + lineWidth);
      }
    }
    fprintf(file, "grestore\n");
  }

  fprintf(file, "grestore\n");
  
  return 0;
}

int
tend_ellipseMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr;
  airArray *mop;

  Nrrd *nten, *npos, *nstn;
  char *outS;
  float gscale, dotRad, lineWidth, cthresh, min[2], max[2];
  FILE *fout;
  int invert;

  mop = airMopNew();

  hestOptAdd(&hopt, "ctr", "conf thresh", airTypeFloat, 1, 1, &cthresh, "0.5",
             "Glyphs will be drawn only for tensors with confidence "
             "values greater than this threshold");
  hestOptAdd(&hopt, "gsc", "scale", airTypeFloat, 1, 1, &gscale, "1",
             "over-all glyph size");
  hestOptAdd(&hopt, "dot", "radius", airTypeFloat, 1, 1, &dotRad, "0.0",
             "radius of little dot to put in middle of ellipse, or \"0\" "
             "for no such dot");
  hestOptAdd(&hopt, "wid", "width", airTypeFloat, 1, 1, &lineWidth, "0.0",
             "with of lines for tractlets");
  hestOptAdd(&hopt, "inv", NULL, airTypeInt, 0, 0, &invert, NULL,
             "use white ellipses on black background, instead of reverse");
  hestOptAdd(&hopt, "min", "minX minY", airTypeFloat, 2, 2, min, "-1 -1",
             "when using \"-p\", minimum corner");
  hestOptAdd(&hopt, "max", "maxX maxY", airTypeFloat, 2, 2, max, "1 1",
             "when using \"-p\", maximum corner");

  /* input/output */
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nten, "-",
             "image of 2D tensors", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "p", "pos array", airTypeOther, 1, 1, &npos, "",
             "Instead of being on a grid, tensors are at arbitrary locations, "
             "as defined by this 2-by-N array of floats", NULL, NULL,
             nrrdHestNrrd);
  hestOptAdd(&hopt, "s", "stn array", airTypeOther, 1, 1, &nstn, "",
             "Locations given by \"-p\" have this connectivity", NULL, NULL,
             nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output PostScript file");

  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_ellipseInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (npos) {
    if (!( 2 == nten->dim && 4 == nten->axis[0].size
           && 2 == npos->dim && 2 == npos->axis[0].size
           && nten->axis[1].size == npos->axis[1].size )) {
      fprintf(stderr, "%s: didn't get matching lists of tensors and pos's\n",
              me);
      airMopError(mop); return 1;
    }
    if (!( nrrdTypeFloat == npos->type )) {
      fprintf(stderr, "%s: didn't get float type positions\n", me);
      airMopError(mop); return 1;
    }
  } else {
    if (!(3 == nten->dim && 4 == nten->axis[0].size)) {
      fprintf(stderr, "%s: didn't get a 3-D 4-by-X-by-Y 2D tensor array\n",
              me);
      airMopError(mop); return 1;
    }
  }
  if (!( nrrdTypeFloat == nten->type )) {
    fprintf(stderr, "%s: didn't get float type tensors\n", me);
    airMopError(mop); return 1;
  }
  if (nstn) {
    if (!( nrrdTypeUInt == nstn->type 
           && 2 == nstn->dim
           && 3 == nstn->axis[0].size )) {
      fprintf(stderr, "%s: connectivity isn't 2-D 3-by-N array of %ss\n",
              me, airEnumStr(nrrdType, nrrdTypeInt));
      airMopError(mop); return 1;
    }
  }
  if (!(fout = airFopen(outS, stdout, "wb"))) {
    fprintf(stderr, "%s: couldn't open \"%s\" for writing\n", me, outS);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, fout, (airMopper)airFclose, airMopAlways);

  tend_ellipseDoit(fout, nten, npos, nstn, min, max, 
                   gscale, dotRad, lineWidth, cthresh, invert);

  airMopOkay(mop);
  return 0;
}
/* TEND_CMD(glyph, INFO); */
unrrduCmd tend_ellipseCmd = { "ellipse", INFO, tend_ellipseMain };
