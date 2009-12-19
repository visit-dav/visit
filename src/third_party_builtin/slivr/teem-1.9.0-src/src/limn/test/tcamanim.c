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

char *info = ("Works with camanim.tcl to test camera path splines.");

#define _LIMNMAGIC "LIMN0000"

int
_limnReadCamanim(int imgSize[2], limnCamera **keycamP, double **timeP,
                 unsigned int *numKeysP, FILE *fin) {
  char me[]="_limnReadCamanim", err[AIR_STRLEN_MED];
  char line[AIR_STRLEN_HUGE];
  unsigned int ki;
  double *tmp, *dwell, di, dn, df, fr[3], at[3], up[3], va;
  airArray *mop, *camA, *dwellA;
  
  if (!( 0 < airOneLine(fin, line, AIR_STRLEN_HUGE)
         && !strcmp(_LIMNMAGIC, line) )) {
    sprintf(err, "%s: couldn't read first line or it wasn't \"%s\"",
            me, _LIMNMAGIC);
    biffAdd(LIMN, err); return 1;
  }
  if (!( 0 < airOneLine(fin, line, AIR_STRLEN_HUGE)
         && 2 == (airStrtrans(airStrtrans(line, '{', ' '), '}', ' '),
                  sscanf(line, "imgSize %d %d", imgSize+0, imgSize+1)) )) {
    sprintf(err, "%s: couldn't read second line or it wasn't "
            "\"imgSize <sizeX> <sizeY>\"", me);
    biffAdd(LIMN, err); return 1;
  }
  
  mop = airMopNew();
  camA = airArrayNew((void **)keycamP, numKeysP, sizeof(limnCamera), 1);
  dwellA = airArrayNew((void **)&dwell, NULL, sizeof(double), 1);
  airMopAdd(mop, camA, (airMopper)airArrayNix, airMopAlways);
  airMopAdd(mop, dwellA, (airMopper)airArrayNuke, airMopAlways);

  while ( 0 < airOneLine(fin, line, AIR_STRLEN_HUGE) ) {
    airStrtrans(airStrtrans(line, '{', ' '), '}', ' ');
    ki = airArrayLenIncr(camA, 1);
    airArrayLenIncr(dwellA, 1);
    if (14 != sscanf(line, "cam.di %lg cam.at %lg %lg %lg "
                     "cam.up %lg %lg %lg cam.dn %lg cam.df %lg cam.va %lg "
                     "relDwell %lg cam.fr %lg %lg %lg",
                     &di, at+0, at+1, at+2,
                     up+0, up+1, up+2, &dn, &df, &va,
                     dwell+ki, fr+0, fr+1, fr+2)) {
      sprintf(err, "%s: trouble parsing line %d: \"%s\"", me, ki, line);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
    (*keycamP)[ki].neer = dn;
    (*keycamP)[ki].faar = df;
    (*keycamP)[ki].dist = di;
    ELL_3V_COPY((*keycamP)[ki].from, fr);
    ELL_3V_COPY((*keycamP)[ki].at, at);
    ELL_3V_COPY((*keycamP)[ki].up, up);
    (*keycamP)[ki].fov = va;
    (*keycamP)[ki].aspect = (double)imgSize[0]/imgSize[1];
    (*keycamP)[ki].atRelative = AIR_FALSE;
    (*keycamP)[ki].orthographic = AIR_FALSE;
    (*keycamP)[ki].rightHanded = AIR_TRUE;
  }

  tmp = (double*)calloc(*numKeysP, sizeof(double));
  airMopAdd(mop, tmp, airFree, airMopAlways);
  *timeP = (double*)calloc(*numKeysP, sizeof(double));
  for (ki=0; ki<*numKeysP; ki++) {
    dwell[ki] = AIR_CLAMP(0, dwell[ki], 2);
    tmp[ki] = tan(AIR_AFFINE(-0.01, dwell[ki], 2.01, 0.0, AIR_PI/2));
  }
  (*timeP)[0] = 0;
  for (ki=1; ki<*numKeysP; ki++) {
    (*timeP)[ki] = (*timeP)[ki-1] + (tmp[ki-1] + tmp[ki])/2;
  } 
  for (ki=0; ki<*numKeysP; ki++) {
    (*timeP)[ki] /= (*timeP)[*numKeysP-1];
  }

  airMopOkay(mop);
  return 0;
}

int
_limnWriteCamanim(FILE *fout, int imgSize[2],
                  limnCamera *cam, int numFrames) {
  /* char me[]="_limnWriteCamanim", err[AIR_STRLEN_MED]; */
  int fi;
  
  fprintf(fout, "%s\n", _LIMNMAGIC);
  fprintf(fout, "imgSize {%d %d}\n", imgSize[0], imgSize[1]);
  for (fi=0; fi<numFrames; fi++) {
    fprintf(fout, "cam.di %g cam.at {%g %g %g } "
            "cam.up {%g %g %g } cam.dn %g cam.df %g cam.va %g "
            "relDwell 1.0 cam.fr {%g %g %g }\n",
            cam[fi].dist,
            cam[fi].at[0], cam[fi].at[1], cam[fi].at[2],
            cam[fi].up[0], cam[fi].up[1], cam[fi].up[2],
            cam[fi].neer, cam[fi].faar, cam[fi].fov,
            cam[fi].from[0], cam[fi].from[1], cam[fi].from[2]);
  }
  return 0;
}

int
main(int argc, char *argv[]) {
  char *me;
  hestOpt *hopt=NULL;
  airArray *mop;

  char *inS, *outS, *err;
  limnCamera *keycam, *cam;
  limnSplineTypeSpec *quatType, *posType, *distType, *viewType;
  double *time;
  FILE *fin, *fout;
  int N, imgSize[2], trackWhat;
  unsigned int numKeys;

  mop = airMopNew();
  
  me = argv[0];
  hestOptAdd(&hopt, "i", "input", airTypeString, 1, 1, &inS, NULL,
             "keyframe output from camanim.tcl");
  hestOptAdd(&hopt, "n", "# frames", airTypeInt, 1, 1, &N, "128",
             "number of frames in output");
  hestOptAdd(&hopt, "t", "track what", airTypeEnum, 1, 1, &trackWhat, "both",
             "what to track", NULL, limnCameraPathTrack);
  hestOptAdd(&hopt, "q", "spline", airTypeOther, 1, 1, 
             &quatType, "tent", "spline type for quaternions",
             NULL, NULL, limnHestSplineTypeSpec);
  hestOptAdd(&hopt, "p", "spline", airTypeOther, 1, 1, 
             &posType, "tent", "spline type for from/at/up",
             NULL, NULL, limnHestSplineTypeSpec);
  hestOptAdd(&hopt, "d", "spline", airTypeOther, 1, 1, 
             &distType, "tent", "spline type for image plane distances",
             NULL, NULL, limnHestSplineTypeSpec);
  hestOptAdd(&hopt, "v", "spline", airTypeOther, 1, 1, 
             &viewType, "tent", "spline type for image fov and aspect",
             NULL, NULL, limnHestSplineTypeSpec);
  hestOptAdd(&hopt, "o", "output", airTypeString, 1, 1, &outS, NULL,
             "frame info for camanim.tcl");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!( fin = airFopen(inS, stdin, "r") )) {
    fprintf(stderr, "%s: couldn't open \"%s\" for reading\n", me, inS);
    airMopError(mop); return 1;
  }
  if (!( fout = airFopen(outS, stdout, "w") )) {
    fprintf(stderr, "%s: couldn't open \"%s\" for writing\n", me, outS);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, fin, (airMopper)airFclose, airMopAlways);
  airMopAdd(mop, fout, (airMopper)airFclose, airMopAlways);

  if (_limnReadCamanim(imgSize, &keycam, &time, &numKeys, fin)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble reading keyframe file:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, keycam, airFree, airMopAlways);
  airMopAdd(mop, time, airFree, airMopAlways);
  cam = (limnCamera *)calloc(N, sizeof(limnCamera));
  airMopAdd(mop, cam, airFree, airMopAlways);
  if (limnCameraPathMake(cam, N, keycam, time, numKeys, trackWhat,
                         quatType, posType, distType, viewType)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble making camera path:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  if (_limnWriteCamanim(fout, imgSize, cam, N)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing frame file:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  
  airMopOkay(mop);
  return 0;
}
