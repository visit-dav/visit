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

#include "../echo.h"
#include "../privateEcho.h"
#include <hest.h>
#include <ten.h>

typedef struct {
  int aniso;
  float anthr, gscale, lrad, ldref, llev, show, lpos[2];
} EchoGlyphParm;

void
rgbGen(echoCol_t rgb[3], float evec[3], float an) {
  
  ELL_3V_ABS(rgb, evec);
  rgb[0] = AIR_AFFINE(0.0, an, 1.0, 0.5, rgb[0]);
  rgb[1] = AIR_AFFINE(0.0, an, 1.0, 0.5, rgb[1]);
  rgb[2] = AIR_AFFINE(0.0, an, 1.0, 0.5, rgb[2]);
}

void
makeGlyphScene(limnCam *cam, EchoParm *eparm,
               Nrrd *nten, EchoGlyphParm *gparm,
               EchoObject **sceneP, airArray **lightArrP) {
  char me[]="makeGlyphScene";
  int xi, yi, zi, sx, sy, sz, ng;
  echoPos_t x, y, z, dmat[9], MA[16], MB[16], MC[16], ident[9];
  echoCol_t rgb[3];
  EchoObject *glf, *inst;
  float *tdata, xs, ys, zs, eval[3], evec[9], c[TEN_ANISO_MAX+1],
    imin[3], imax[3], omin[3], omax[3];
  EchoObject *scene, *rect;
  EchoLight *light;
  
  scene = echoObjectNew(echoObjectList);
  *lightArrP = echoLightArrayNew();

  eparm->bgR = 0.5;
  eparm->bgG = 0.5;
  eparm->bgB = 0.5;
  eparm->renderLights = AIR_FALSE;
  eparm->renderBoxes = AIR_FALSE;

  sx = nten->axis[1].size;
  sy = nten->axis[2].size;
  sz = nten->axis[3].size;
  xs = nten->axis[1].spacing;
  ys = nten->axis[2].spacing;
  zs = nten->axis[3].spacing;
  tdata = nten->data;         /* we specifically requested float below */

  ELL_3V_SET(imin, -1, -1, -1);
  ELL_3V_SET(imax, 1, 1, 1);
  ELL_3V_SET(omin, 0, 0, 0);
  ELL_3V_SET(omax, xs*(sx-1), ys*(sy-1), zs*(sz-1));
  ELL_3V_AFFINE(cam->from, imin, cam->from, imax, omin, omax);
  ELL_3V_AFFINE(cam->at, imin, cam->at, imax, omin, omax);

  ng = 0;
  for (zi=0; zi<sz; zi++) {
    printf("%s: zi = %3d/%d ... \n", me, zi, sz-1);
    z = zs * zi;
    for (yi=0; yi<sy; yi++) {
      y = ys * yi;
      for (xi=0; xi<sx; xi++) {
        x = xs * xi;

        /* don't render tensor if confidence threshold below 0.5 */
        if (!( tdata[0] > 0.5 ))
          goto onward;

        /* do eigensystem solve; don't render if aniso threshold not met */
        tenEigensolve(eval, evec, tdata);
        tenAnisoCalc(c, eval);
        if (!( c[gparm->aniso] > gparm->anthr ))
          goto onward;

        rgbGen(rgb, evec, c[gparm->aniso]);
        if (1) {
          glf = echoObjectNew(echoObjectSphere);
          echoObjectSphereSet(glf, 0, 0, 0, 0.5);
        }
        else {
          glf = echoObjectNew(echoObjectCube);
        }
        echoMatterMetalSet(glf, rgb[0], rgb[1], rgb[2],
                           0.8, 1.0, 0.0);
        echoMatterPhongSet(glf, rgb[0], rgb[1], rgb[2], 1.0,
                           0.1, 1.0, 0.3, 1);

        TEN_LIST2MAT(dmat, tdata);
        ELL_3M_SET_IDENTITY(ident);
        ELL_3M_SCALEADD(dmat, gparm->gscale*(1-gparm->show), ident,
                        gparm->gscale*gparm->show, dmat);
        ELL_43M_INSET(MA, dmat);
        ELL_4M_SET_TRANSLATE(MB, x, y, z);
        ELL_4M_MUL(MC, MB, MA);

        inst = echoObjectNew(echoObjectInstance);
        echoObjectInstanceSet(inst, MC, glf, AIR_TRUE);

        echoObjectListAdd(scene, inst);

        ng++;

      onward:
        tdata += 7;
      }
    }
  }

  /* something to cast a shadow on 
  glf = echoObjectNew(echoObjectSphere);
  echoObjectSphereSet(glf, 35+1000, 40, 35, 1000);
  echoMatterPhongSet(glf, 1, 1, 1, 1.0,
                     0.1, 1.0, 0.3, 1);
  inst = echoObjectNew(echoObjectInstance);
  echoObjectInstanceSet(inst, MC, glf, AIR_TRUE);
  echoObjectListAdd(scene, inst);
  */
  
  printf("%s: generated %d glyphs\n", me, ng);

  if (gparm->lrad) {
    rect = echoObjectNew(echoObjectRectangle);
    echoObjectRectangleSet(rect,
                           AIR_AFFINE(0, gparm->lpos[0], 1, 0, sx*xs)
                           - gparm->lrad,
                           AIR_AFFINE(0, gparm->lpos[1], 1, 0, sy*ys)
                           - gparm->lrad,
                           0,
                           2*gparm->lrad, 0, 0,
                           0, 2*gparm->lrad, 0);
    eparm->refDistance = sz*(gparm->ldref);
    printf("llev = %g\n", gparm->llev);
    echoMatterLightSet(rect, gparm->llev, gparm->llev, gparm->llev);
    echoObjectListAdd(scene, rect);
    light = echoLightNew(echoLightArea);
    echoLightAreaSet(light, rect);
    echoLightArrayAdd(*lightArrP, light);
  }
  else {
    light = echoLightNew(echoLightDirectional);
    echoLightDirectionalSet(light, 1, 1, 1, 
                            AIR_AFFINE(0, gparm->lpos[0], 1, -sx*xs, sx*xs),
                            AIR_AFFINE(0, gparm->lpos[1], 1, -sy*ys, sy*ys),
                            AIR_AFFINE(0, 0, 1, -sz*zs, sz*zs));
    echoLightArrayAdd(*lightArrP, light);
  }

  if (1) {
    printf("%s: making BVH ... ", me); fflush(stdout);
    *sceneP = echoObjectListSplit3(scene, 8);
    printf("done\n");
  }
  else {
    *sceneP = scene;
  }
  
  return;
}


int
echoParseTenNrrd(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  char me[] = "echoParseTenNrrd", *nerr;
  Nrrd **nrrdP;
  airArray *mop;
  
  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  nrrdP = ptr;
  mop = airMopNew();
  *nrrdP = nrrdNew();
  airMopAdd(mop, *nrrdP, (airMopper)nrrdNuke, airMopOnError);
  if (nrrdLoad(*nrrdP, str)) {
    airMopAdd(mop, nerr = biffGetDone(NRRD), airFree, airMopOnError);
    if (strlen(nerr) > AIR_STRLEN_HUGE - 1)
      nerr[AIR_STRLEN_HUGE - 1] = '\0';
    strcpy(err, nerr);
    airMopError(mop);
    return 1;
  }
  if (!tenValidTensor(*nrrdP, nrrdTypeFloat, AIR_TRUE)) {
    /* why not use the given err[] as a temp buffer */
    sprintf(err, "%s: \"%s\" isn't a valid tensor volume", me, str);
    biffAdd(TEN, err);
    airMopAdd(mop, nerr = biffGetDone(TEN), airFree, airMopOnError);
    if (strlen(nerr) > AIR_STRLEN_HUGE - 1)
      nerr[AIR_STRLEN_HUGE - 1] = '\0';
    strcpy(err, nerr);
    airMopError(mop);
    return 1;
  }
  if (!( AIR_EXISTS((*nrrdP)->axis[1].spacing) &&
         AIR_EXISTS((*nrrdP)->axis[2].spacing) &&
         AIR_EXISTS((*nrrdP)->axis[3].spacing) )) {
    sprintf(err, "%s: need existant spacings on x,y,z axes", me);
    airMopError(mop);
    return 1;
  }
  airMopOkay(mop);
  return 0;
}

hestCB echoParseTenNrrdCB = {
  sizeof(Nrrd *),
  "nrrd",
  echoParseTenNrrd,
  (airMopper)nrrdNuke
};

int
main(int argc, char *argv[]) {
  airArray *mop, *lightArr;
  hestOpt *opt = NULL;
  limnCam *cam;
  hestParm *hparm;
  Nrrd *nten, *nraw, *nimg, *nppm, *ntmp, *npgm;
  echoPos_t ur[2], vr[2];
  int E, is[2];
  char *me, *outS, *err,
    info[] = "Generates cool images of tensor glyphs";
  EchoParm *eparm;
  EchoGlobalState *state;
  EchoObject *scene;
  EchoGlyphParm gparm;

  me = argv[0];
  mop = airMopNew();

  hparm = hestParmNew();
  hparm->respFileEnable = AIR_TRUE;
  hparm->verbosity = 0;

  cam = echoLimnCamNew();
  airMopAdd(mop, cam, (airMopper)limnCamNix, airMopAlways);
  cam->neer = 0;
  cam->dist = 0;
  cam->faar = 0;
  cam->atRel = AIR_TRUE;

  state = echoGlobalStateNew();
  airMopAdd(mop, state, (airMopper)echoGlobalStateNix, airMopAlways);

  eparm = echoParmNew();
  airMopAdd(mop, eparm, (airMopper)echoParmNix, airMopAlways);

  hestOptAdd(&opt, "i", "nin", airTypeOther, 1, 1, &nten, NULL,
             "diffusion tensor data",
             NULL, NULL, &echoParseTenNrrdCB);
  hestOptAdd(&opt, "fr", "eye point", airTypeFloat, 3, 3, cam->from,
             "40 40 40", "camera eye point");
  hestOptAdd(&opt, "at", "lookat", airTypeFloat, 3, 3, cam->at, "0 0 0",
             "camera look-at point");
  hestOptAdd(&opt, "up", "up", airTypeFloat, 3, 3, cam->up, "0 0 1",
             "camera pseudo up vector");
  hestOptAdd(&opt, "ur", "U range", echoPos_airType, 2, 2, ur, "-20 20",
             "range in U direction of image plane");
  hestOptAdd(&opt, "vr", "V range", echoPos_airType, 2, 2, vr, "-20 20",
             "range in V direction of image plane");
  hestOptAdd(&opt, "is", "image size", airTypeInt, 2, 2, is, "256 256",
             "image dimensions");
  hestOptAdd(&opt, "ns", "# samples", airTypeInt, 1, 1, &(eparm->samples), "1",
             "# of samples per pixel (1 --> no jitter)");
  hestOptAdd(&opt, "o", "output", airTypeString, 1, 1, &outS, "out.ppm",
             "PPM image output");
  hestOptAdd(&opt, "ap", "aperture", airTypeFloat, 1, 1, &(eparm->aperture),
             "0.0", "camera aperture (0.0 --> no depth of field)");
  hestOptAdd(&opt, "an", "aniso", airTypeEnum, 1, 1, &(gparm.aniso), "fa",
             "which anisotropy metric", NULL, &tenAniso);
  hestOptAdd(&opt, "th", "thresh", airTypeFloat, 1, 1, &(gparm.anthr), "0.8",
             "threshold on anisotropy");
  hestOptAdd(&opt, "gs", "scale", airTypeFloat, 1, 1, &(gparm.gscale), "0.8",
             "over-all scaling on all glyphs");
  hestOptAdd(&opt, "gh", "show", airTypeFloat, 1, 1, &(gparm.show), "1.0",
             "how much to \"show\" the data:\n "
             "0.0 --> all identity; 1.0 --> all data");
  hestOptAdd(&opt, "lr", "light radius", airTypeFloat, 1, 1,
             &(gparm.lrad), "5", "\"radius\" of area light over volume");
  hestOptAdd(&opt, "ld", "ref dist", airTypeFloat, 1, 1,
             &(gparm.ldref), "0.5", "\"reference distance\" for light, "
             "expressed as a fraction of Z dimension of volume");
  hestOptAdd(&opt, "ll", "light level", airTypeFloat, 1, 1,
             &(gparm.llev), "1", "area light intensity");
  hestOptAdd(&opt, "lp", "light position", airTypeFloat, 2, 2,
             &(gparm.lpos), "0.5 0.5", "area light position");
  hestOptAdd(&opt, "sh", NULL, airTypeInt, 0, 0,
             &(eparm->shadow), NULL, "render shadows");
  if (hestOptCheck(opt, &err)) { printf("%s\n", err); exit(1); }

  if (1 == argc) {
    hestInfo(stderr, argv[0], info, hparm);
    hestUsage(stderr, opt, argv[0], hparm);
    hestGlossary(stderr, opt, hparm);
    opt = hestOptFree(opt);
    hparm = hestParmFree(hparm);
    exit(1);
  }

  /* parse command line */
  if (hestParse(opt, argc-1, argv+1, &err, hparm)) {
    fprintf(stderr, "ERROR: %s\n", err); free(err);
    hestUsage(stderr, opt, argv[0], hparm);
    hestGlossary(stderr, opt, hparm);
    opt = hestOptFree(opt);
    hparm = hestParmFree(hparm);
    exit(1);
  }

  /* finish dealing with parsed information */
  cam->uRange[0] = ur[0];
  cam->uRange[1] = ur[1];
  cam->vRange[0] = vr[0];
  cam->vRange[1] = vr[1];
  eparm->imgResU = is[0];
  eparm->imgResV = is[1];
  eparm->jitter = (1 == eparm->samples ? echoJitterNone : echoJitterJitter);

  /* do the glyph thing */
  makeGlyphScene(cam, eparm, nten, &gparm,
                 &scene, &lightArr);
  
  /* render */
  nraw = nrrdNew();
  nimg = nrrdNew();
  nppm = nrrdNew();
  ntmp = nrrdNew();
  npgm = nrrdNew();
  airMopAdd(mop, nraw, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nimg, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nppm, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, npgm, (airMopper)nrrdNuke, airMopAlways);
  E = 0;
  printf("%s: rendering (%d samples)... ", me, eparm->samples); 
  fflush(stdout);
  if (!E) E |= echoRender(nraw, cam, eparm, state, scene, lightArr);
  printf("done.\n");
  if (!E) E |= echoComposite(nimg, nraw, eparm);
  if (!E) E |= echoPPM(nppm, nimg, eparm);
  if (E) {
    airMopAdd(mop, err = biffGetDone(ECHO), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  printf("%s: render time = %g seconds (%g fps)\n",
         me, state->time, 1.0/state->time);
  if (!E) E |= nrrdSave("raw.nrrd", nraw, NULL);
  if (!E) E |= nrrdSave("out.ppm", nppm, NULL);
  if (!E) E |= nrrdSlice(ntmp, nraw, 0, 3);
  ntmp->min = 0.0; ntmp->max = 1.0;
  if (!E) E |= nrrdQuantize(npgm, ntmp, 8);
  if (!E) E |= nrrdSave("alpha.pgm", npgm, NULL);
  if (!E) E |= nrrdSlice(ntmp, nraw, 0, 4);
  if (!E) E |= nrrdHistoEq(ntmp, ntmp, NULL, 2048, 2);
  if (!E) E |= nrrdQuantize(npgm, ntmp, 8);
  if (!E) E |= nrrdSave("time.pgm", npgm, NULL);
  if (E) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);

  hestParseFree(opt);
  opt = hestOptFree(opt);
  hparm = hestParmFree(hparm);
  exit(0);
}
