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

char *info = ("Plot!");

typedef struct {
  FILE *file;
  double psc;
  double bbox[4];
  
  double maxX, maxY;  /* set by plotPreamble */
} plotPS;

typedef struct {
  /* these are allocated per-graph BY HEST */
  double *graphThick, *graphGray, *dotDiameter, *dotGray;

  /* also hest-allocated */
  char *axisHorzLabel, *axisVertLabel;
  double *horzTick, *vertTick;
  
  int numHorzTick, numVertTick;
  int labelHorzTick, labelVertTick;
  double axisThick, tickThick, tickLabelSize, tickLength;
  double horzTickLabelOffset, vertTickLabelOffset;
  double dotInnerDiameterFraction;
  double axisOrig[2];
  double dbox[4];
} plotParm;

#define PPS_X(x) AIR_AFFINE(pps->bbox[0], (x), pps->bbox[2], 0, pps->maxX)
#define PPS_Y(y) AIR_AFFINE(pps->bbox[1], (y), pps->bbox[3], 0, pps->maxY)
#define PPS_S(s)  AIR_DELTA(pps->bbox[1], (s), pps->bbox[3], 0, pps->maxY)

void
plotPreamble(plotPS *pps, plotParm *pparm) {

  AIR_UNUSED(pparm);
  pps->maxX = pps->psc*(pps->bbox[2] - pps->bbox[0]);
  pps->maxY = pps->psc*(pps->bbox[3] - pps->bbox[1]);

  fprintf(pps->file, "%%!PS-Adobe-2.0 EPSF-2.0\n");
  fprintf(pps->file, "%%%%Creator: plot\n");
  fprintf(pps->file, "%%%%Pages: 1\n");
  fprintf(pps->file, "%%%%BoundingBox: 0 0 %d %d\n", 
          (int)(pps->maxX),
          (int)(pps->maxY));
  fprintf(pps->file, "%%%%EndComments\n");
  fprintf(pps->file, "%%%%EndProlog\n");
  fprintf(pps->file, "%%%%Page: 1 1\n");
  fprintf(pps->file, "gsave\n");
  fprintf(pps->file, "0 0 moveto\n");
  fprintf(pps->file, "%g 0 lineto\n", pps->maxX);
  fprintf(pps->file, "%g %g lineto\n", pps->maxX, pps->maxY);
  fprintf(pps->file, "0 %g lineto\n", pps->maxY);
  fprintf(pps->file, "closepath\n");
  fprintf(pps->file, "clip\n");
  fprintf(pps->file, "gsave newpath\n");
  fprintf(pps->file, "1 setlinejoin\n");
  fprintf(pps->file, "1 setlinecap\n");
  fprintf(pps->file, "/M {moveto} bind def\n");
  fprintf(pps->file, "/L {lineto} bind def\n");
  fprintf(pps->file, "/W {setlinewidth} bind def\n");
  fprintf(pps->file, "/F {fill} bind def\n");
  fprintf(pps->file, "/S {stroke} bind def\n");
  fprintf(pps->file, "/CP {closepath} bind def\n");
  fprintf(pps->file, "/RGB {setrgbcolor} bind def\n");
  fprintf(pps->file, "/Gr {setgray} bind def\n");
  fprintf(pps->file, "\n");
  fprintf(pps->file, "0 setlinecap\n");
  return;
}

void
plotWidth(plotPS *pps, plotParm *pparm, double width) {

  AIR_UNUSED(pparm);
  fprintf(pps->file, "%g W\n", pps->psc*width);
  return;
}

void
plotGray(plotPS *pps, plotParm *pparm, double gray) {

  AIR_UNUSED(pparm);
  fprintf(pps->file, "%g Gr\n", gray);
  return;
}

void
plotLine(plotPS *pps, plotParm *pparm,
         double x0, double y0, double x1, double y1) {
  
  AIR_UNUSED(pparm);
  fprintf(pps->file, "%g %g M\n", PPS_X(x0), PPS_Y(y0));
  fprintf(pps->file, "%g %g L S\n", PPS_X(x1), PPS_Y(y1));
  return;
}

void
plotLabel(plotPS *pps, plotParm *pparm, int centered, 
          double x, double y, char *str) {
  
  fprintf(pps->file, "gsave\n");
  plotWidth(pps, pparm, 0);
  if (centered) {
    fprintf(pps->file,  
            "0 0 M (%s) false charpath pathbbox\n"
            "exch 4 1 roll sub 2 div 3 1 roll sub -2 div\n"  /* don't ask */
            "newpath %g add exch %g add M (%s) show\n",
            str, PPS_X(x), PPS_Y(y), str);
  } else {
    fprintf(pps->file,  "%g %g M (%s) show\n", PPS_X(x), PPS_Y(y), str);
  }
  fprintf(pps->file, "grestore\n");
  return;
}

void
plotAxes(plotPS *pps, plotParm *pparm, Nrrd *ndata) {
  double axX, axY, xx, yy, toff;
  char buff[AIR_STRLEN_SMALL];
  int ti;
  
  AIR_UNUSED(ndata);
  axX = AIR_AFFINE(pparm->dbox[0], pparm->axisOrig[0], pparm->dbox[2],
                   pps->bbox[0], pps->bbox[2]);
  axY = AIR_AFFINE(pparm->dbox[1], pparm->axisOrig[1], pparm->dbox[3],
                   pps->bbox[1], pps->bbox[3]);
  plotGray(pps, pparm, 0);
  plotWidth(pps, pparm, pparm->axisThick);
  plotLine(pps, pparm,
           axX, pps->bbox[1], axX, pps->bbox[3]);
  plotLine(pps, pparm,
           pps->bbox[0], axY, pps->bbox[2], axY);

  if (strlen(pparm->axisHorzLabel) || strlen(pparm->axisVertLabel)) {
    fprintf(pps->file, "/Helvetica findfont 20 scalefont setfont\n");
    if (strlen(pparm->axisHorzLabel)) {
      plotLabel(pps, pparm, AIR_FALSE, 
                pps->bbox[2], axY, pparm->axisHorzLabel);
    }
    if (strlen(pparm->axisVertLabel)) {
      plotLabel(pps, pparm, AIR_FALSE, 
                axX, pps->bbox[3], pparm->axisVertLabel);
    }
  }

  if (pparm->numHorzTick) {
    if (pparm->tickThick > 0) {
      toff = pparm->tickLength/2;
      plotGray(pps, pparm, 0);
      plotWidth(pps, pparm, pparm->tickThick);
      for (ti=0; ti<pparm->numHorzTick; ti++) {
        xx = AIR_AFFINE(pparm->dbox[0], pparm->horzTick[ti], pparm->dbox[2],
                        pps->bbox[0], pps->bbox[2]);
        plotLine(pps, pparm, xx, axY - toff, xx, axY + toff);
      }
    }
    if (pparm->tickLabelSize) {
      fprintf(pps->file, "/Helvetica findfont %g scalefont setfont\n",
              pparm->tickLabelSize);
      for (ti=0; ti<pparm->numHorzTick; ti++) {
        xx = AIR_AFFINE(pparm->dbox[0], pparm->horzTick[ti], pparm->dbox[2],
                        pps->bbox[0], pps->bbox[2]);
        yy = axY + pparm->horzTickLabelOffset;
        sprintf(buff, "%g", pparm->horzTick[ti]);
        plotLabel(pps, pparm, AIR_TRUE, xx, yy, buff);
      }
    }
  }
  if (pparm->numVertTick) {
    if (pparm->tickThick > 0) {
      toff = pparm->tickLength/2;
      plotGray(pps, pparm, 0);
      plotWidth(pps, pparm, pparm->tickThick);
      for (ti=0; ti<pparm->numVertTick; ti++) {
        yy = AIR_AFFINE(pparm->dbox[1], pparm->vertTick[ti], pparm->dbox[3],
                        pps->bbox[1], pps->bbox[3]);
        plotLine(pps, pparm, axX - toff, yy, axX + toff, yy);
      }
    }
    if (pparm->tickLabelSize) {
      fprintf(pps->file, "/Helvetica findfont %g scalefont setfont\n",
              pparm->tickLabelSize);
      for (ti=0; ti<pparm->numVertTick; ti++) {
        yy = AIR_AFFINE(pparm->dbox[1], pparm->vertTick[ti], pparm->dbox[3],
                        pps->bbox[1], pps->bbox[3]);
        xx = axX + pparm->vertTickLabelOffset;
        sprintf(buff, "%g", pparm->vertTick[ti]);
        plotLabel(pps, pparm, AIR_TRUE, xx, yy, buff);
      }
    }
  }
}

void
plotGraph(plotPS *pps, plotParm *pparm, Nrrd **ndata, int nidx) {
  int ii, npts;
  double xx, yy, *data, val;

  if (!( pparm->graphThick[nidx] > 0 )) {
    return;
  }

  data = (double *)(ndata[nidx]->data);
  npts = ndata[nidx]->axis[1].size;
  plotGray(pps, pparm, pparm->graphGray[nidx]);
  fprintf(pps->file, "%g W\n", pps->psc*pparm->graphThick[nidx]);
  for (ii=0; ii<npts; ii++) {
    val = data[ii];
    xx = AIR_AFFINE(0, ii, npts-1,
                    ndata[nidx]->axis[1].min, ndata[nidx]->axis[1].max);
    xx = AIR_AFFINE(pparm->dbox[0], xx, pparm->dbox[2],
                    pps->bbox[0], pps->bbox[2]);
    yy = AIR_AFFINE(pparm->dbox[1], val, pparm->dbox[3],
                    pps->bbox[1], pps->bbox[3]);
    fprintf(pps->file, "%g %g %s\n", PPS_X(xx), PPS_Y(yy),
            ii ? "L" : "M");
  }
  fprintf(pps->file, "S\n");
}

void
plotDots(plotPS *pps, plotParm *pparm, Nrrd **ndata, int nidx) {
  int ii, npts;
  double xx, yy, orad, irad, *data, val;

  if (!( pparm->dotDiameter[nidx] > 0 )) {
    return;
  }

  fprintf(pps->file, "gsave\n");
  fprintf(pps->file, "newpath\n");
  plotWidth(pps, pparm, 0);
  data = (double *)(ndata[nidx]->data);
  npts = ndata[nidx]->axis[1].size;
  orad = pparm->dotDiameter[nidx]/2;
  irad = pparm->dotInnerDiameterFraction*orad;
  for (ii=0; ii<npts; ii++) {
    val = data[ii];
    xx = AIR_AFFINE(0, ii, npts-1,
                    ndata[nidx]->axis[1].min, ndata[nidx]->axis[1].max);
    xx = AIR_AFFINE(pparm->dbox[0], xx, pparm->dbox[2],
                    pps->bbox[0], pps->bbox[2]);
    yy = AIR_AFFINE(pparm->dbox[1], val, pparm->dbox[3],
                    pps->bbox[1], pps->bbox[3]);
    plotGray(pps, pparm, pparm->dotGray[nidx]);
    fprintf(pps->file, "%g %g %g 0 360 arc closepath fill\n", 
            PPS_X(xx), PPS_Y(yy), PPS_S(orad));
    if (irad) {
      plotGray(pps, pparm, 1.0);
      fprintf(pps->file, "%g %g %g 0 360 arc closepath fill\n", 
              PPS_X(xx), PPS_Y(yy), PPS_S(irad));
    }
  }
  fprintf(pps->file, "grestore\n");
}

void
plotEpilog(plotPS *pps, plotParm *pparm) {
  
  AIR_UNUSED(pparm);
  fprintf(pps->file, "grestore\n");
  fprintf(pps->file, "grestore\n");
  fprintf(pps->file, "%%%%Trailer\n");
  return;
}

int
main(int argc, char *argv[]) {
  char *me, *err, *outS;
  hestOpt *hopt=NULL;
  airArray *mop;

  int numGrth, numDtdi, numGrgr, numDtgr, numNrrd, ni;
  plotPS pps;
  plotParm pparm;
  Nrrd **_ndata, **ndata;

  mop = airMopNew();
  
  me = argv[0];

  hestOptAdd(&hopt, "i", "data", airTypeOther, 1, -1, &_ndata, NULL,
             "input nrrd containing data to plot",
             &numNrrd, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "dbox", "minX minY maxX maxY", airTypeDouble,
             4, 4, pparm.dbox, NULL,
             "bounding box, in data space");
  
  hestOptAdd(&hopt, "bbox", "minX minY maxX maxY", airTypeDouble,
             4, 4, pps.bbox, NULL,
             "bounding box, in graph space");
  hestOptAdd(&hopt, "psc", "PS scale", airTypeDouble, 1, 1, &(pps.psc), "300",
             "scaling from graph space to PostScript points");
  
  hestOptAdd(&hopt, "grth", "graph thickness", airTypeDouble,
             1, -1, &(pparm.graphThick), "0.01",
             "thickness of line for graph, or \"0\" for no graph line", 
             &numGrth);
  hestOptAdd(&hopt, "grgr", "graph gray", airTypeDouble,
             1, -1, &(pparm.graphGray), "0",
             "grayscale to use for graph", &numGrgr);
  hestOptAdd(&hopt, "dtdi", "dot diameter", airTypeDouble,
             1, -1, &(pparm.dotDiameter), "0.1",
             "radius of dot drawn at data points, or \"0\" for no dots",
             &numDtdi);
  hestOptAdd(&hopt, "dtgr", "dot gray", airTypeDouble,
             1, -1, &(pparm.dotGray), "0",
             "grayscale to use for dots", &numDtgr);
  hestOptAdd(&hopt, "dtid", "dot inner diam frac", airTypeDouble,
             1, 1, &(pparm.dotInnerDiameterFraction), "0.0",
             "fractional radius of white dot drawn within dot");

  hestOptAdd(&hopt, "tihz", "pos", airTypeDouble,
             0, -1, &(pparm.horzTick), "",
             "locations for tickmarks on horizontal axis",
             &(pparm.numHorzTick));
  hestOptAdd(&hopt, "tivt", "pos", airTypeDouble,
             0, -1, &(pparm.vertTick), "",
             "locations for tickmarks on vertical axis",
             &(pparm.numVertTick));
  hestOptAdd(&hopt, "tiho", "offset", airTypeDouble,
             1, 1, &(pparm.horzTickLabelOffset), "0",
             "horizontal tick label offset");
  hestOptAdd(&hopt, "tivo", "offset", airTypeDouble,
             1, 1, &(pparm.vertTickLabelOffset), "0",
             "vertical tick label offset");
  hestOptAdd(&hopt, "tils", "size", airTypeDouble,
             1, 1, &(pparm.tickLabelSize), "0",
             "font size for labels on tick marks, or \"0\" for no labels");
  hestOptAdd(&hopt, "tith", "tick thickness", airTypeDouble,
             1, 1, &(pparm.tickThick), "0.01",
             "thickness of lines for tick marks");
  hestOptAdd(&hopt, "tiln", "tick length", airTypeDouble,
             1, 1, &(pparm.tickLength), "0.08",
             "length of lines for tick marks");

  hestOptAdd(&hopt, "axth", "axis thickness", airTypeDouble,
             1, 1, &(pparm.axisThick), "0.01",
             "thickness of lines for axes");
  hestOptAdd(&hopt, "axor", "axis origin", airTypeDouble,
             2, 2, &(pparm.axisOrig), "0 0",
             "origin of lines for axes, in data space");
  hestOptAdd(&hopt, "axhl", "horiz axis label", airTypeString,
             1, 1, &(pparm.axisHorzLabel), "",
             "label on horizontal axis");
  hestOptAdd(&hopt, "axvl", "vert axis label", airTypeString,
             1, 1, &(pparm.axisVertLabel), "",
             "label on vertical axis");

  hestOptAdd(&hopt, "o", "output PS", airTypeString,
             1, 1, &outS, "out.ps",
             "output file to render postscript into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!( numGrth == numDtdi 
         && numDtdi == numGrgr
         && numGrgr == numDtgr )) {
    fprintf(stderr, "%s: number of arguments given to grth (%d), dtdi (%d), "
            "grgr (%d), dtgr (%d) not all equal\n", me,
            numGrth, numDtdi, numGrgr, numDtgr);
    airMopError(mop); return 1;
  }
  if (!( numNrrd == numGrth )) {
    fprintf(stderr, "%s: number of nrrds (%d) != number graph options (%d)\n",
            me, numNrrd, numGrth);
    airMopError(mop); return 1;
  }

  /* check nrrds */
  for (ni=0; ni<numNrrd; ni++) {
    if (!( (1 == _ndata[ni]->dim || 2 == _ndata[ni]->dim) 
           && nrrdTypeBlock != _ndata[ni]->type )) {
      fprintf(stderr, "%s: input nrrd must be 1-D or 2-D array of scalars",
              me);
      airMopError(mop); return 1;
    }
  }
  ndata = (Nrrd**)calloc(numNrrd, sizeof(Nrrd *));
  airMopAdd(mop, ndata, airFree, airMopAlways);
  for (ni=0; ni<numNrrd; ni++) {
    ndata[ni] = nrrdNew();
    airMopAdd(mop, ndata[ni], (airMopper)nrrdNuke, airMopAlways);
    if (nrrdConvert(ndata[ni], _ndata[ni], nrrdTypeDouble)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: couldn't convert input %d to %s:\n%s\n",
              me, ni, airEnumStr(nrrdType, nrrdTypeDouble), err);
      airMopError(mop); return 1;
    }
    if (1 == ndata[ni]->dim) {
      if (nrrdAxesInsert(ndata[ni], ndata[ni], 0)) {
        airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
        fprintf(stderr, "%s: couldn't insert axis 0 on nrrd %d:\n%s\n",
                me, ni, err);
        airMopError(mop); return 1;
      }
    }
    /* currently assuming node centering */
    if (!AIR_EXISTS(ndata[ni]->axis[1].min)) {
      ndata[ni]->axis[1].min = 0;
    }
    if (!AIR_EXISTS(ndata[ni]->axis[1].max)) {
      ndata[ni]->axis[1].max = ndata[ni]->axis[1].size-1;
    }
  }

  if (!(pps.file = airFopen(outS, stdout, "wb"))) {
    fprintf(stderr, "%s: couldn't open output file\n", me);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, pps.file, (airMopper)airFclose, airMopAlways);
  
  plotPreamble(&pps, &pparm);
  plotAxes(&pps, &pparm, ndata[0]);
  for (ni=0; ni<numNrrd; ni++) {
    plotGraph(&pps, &pparm, ndata, ni);
    plotDots(&pps, &pparm, ndata, ni);
  }
  plotEpilog(&pps, &pparm);

  airMopOkay(mop);
  return 0;
}
