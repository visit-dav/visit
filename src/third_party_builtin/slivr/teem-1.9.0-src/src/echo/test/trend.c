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

/* bad bad bad Gordon */
void
_dyeHSVtoRGB(float *R, float *G, float *B,
            float  H, float  S, float  V) {
  float min, fract, vsf, mid1, mid2;
  int sextant;
  
  if (0 == S) {
    *R = *G = *B = V;
    return;
  }
  /* else there is hue */
  if (1 == H)
    H = 0;
  H *= 6;
  sextant = (int) floor(H);
  fract = H - sextant;
  vsf = V*S*fract;
  min = V*(1 - S);
  mid1 = min + vsf;
  mid2 = V - vsf;
  switch (sextant) {
  case 0: { *R = V;    *G = mid1; *B = min;  break; }
  case 1: { *R = mid2; *G = V;    *B = min;  break; }
  case 2: { *R = min;  *G = V;    *B = mid1; break; }
  case 3: { *R = min;  *G = mid2; *B = V;    break; }
  case 4: { *R = mid1; *G = min;  *B = V;    break; }
  case 5: { *R = V;    *G = min;  *B = mid2; break; }
  }
}

#if 0  

void
makeSceneAntialias(limnCamera *cam, echoRTParm *parm,
                   echoObject **sceneP, airArray **lightArrP) {
  echoObject *scene, *rect;
  Nrrd *ntext;

  *sceneP = scene = echoObjectNew(echoList);
  *lightArrP = echoLightArrayNew();

  ELL_3V_SET(cam->from, 0, 0, 10);
  ELL_3V_SET(cam->at,   0, 0, 0);
  ELL_3V_SET(cam->up,   0, 1, 0);
  cam->uRange[0] = -3.7;
  cam->uRange[1] = 3.7;
  cam->vRange[0] = -3.7;
  cam->vRange[1] = 3.7;

  parm->jitterType = echoJitterGrid;
  parm->numSamples = 1;
  parm->imgResU = 300;
  parm->imgResV = 300;
  parm->aperture = 0.0;
  parm->renderLights = AIR_FALSE;
  parm->renderBoxes = AIR_FALSE;
  parm->seedRand = AIR_FALSE;
  parm->maxRecDepth = 10;
  parm->shadow = 1.0;

  nrrdLoad(ntext = nrrdNew(), "chirp.nrrd", NULL);
  rect = echoObjectNew(echoRectangle);
  echoRectangleSet(rect,
                         -3, -3, 0,
                         6, 0, 0,
                         0, 6, 0);
  echoMatterPhongSet(rect, 1, 1, 1, 1.0,
                     1.0, 0.0, 0.0, 1);
  echoMatterTextureSet(rect, ntext);
  echoObjectAdd(scene, rect);

  return;
}

void
makeSceneBVH(limnCamera *cam, echoRTParm *parm, echoObject **sceneP) {
  echoObject *sphere;
  int i, N;
  float r, g, b;
  echoObject *scene;
  double time0, time1;
  
  *sceneP = scene = echoObjectNew(echoList);

  ELL_3V_SET(cam->from, 9, 6, 0);
  ELL_3V_SET(cam->at,   0, 0, 0);
  ELL_3V_SET(cam->up,   0, 0, 1);
  cam->uRange[0] = -3;
  cam->uRange[1] = 3;
  cam->vRange[0] = -3;
  cam->vRange[1] = 3;

  parm->jitterType = echoJitterNone;
  parm->numSamples = 1;
  parm->imgResU = 500;
  parm->imgResV = 500;
  parm->aperture = 0.0;
  parm->renderLights = AIR_TRUE;
  parm->renderBoxes = AIR_FALSE;
  parm->seedRand = AIR_FALSE;
  parm->maxRecDepth = 10;
  parm->shadow = 0.0;

  N = 1000000;
  airArrayLenSet(LIST(scene)->objArr, N);
  for (i=0; i<N; i++) {
    sphere = echoObjectNew(echoSphere);
    echoSphereSet(sphere,
                  4*airDrandMT()-2, 4*airDrandMT()-2, 4*airDrandMT()-2, 0.005);
    _dyeHSVtoRGB(&r, &g, &b, AIR_AFFINE(0, i, N, 0.0, 1.0), 1.0, 1.0);
    echoMatterPhongSet(sphere, r, g, b, 1.0,
                       1.0, 0.0, 0.0, 50);
    LIST(scene)->obj[i] = sphere;
  }

  time0 = airTime();
  *sceneP = scene = echoListSplit3(scene, 8);
  time1 = airTime();
  printf("BVH build time = %g seconds\n", time1 - time0);
}

void
makeSceneGlass(limnCamera *cam, echoRTParm *parm, echoObject **sceneP) {
  echoObject *cube, *rect;
  echoObject *scene;
  Nrrd *ntext;
  
  *sceneP = scene = echoObjectNew(echoList);

  ELL_3V_SET(cam->from, 2, -3, 8);
  ELL_3V_SET(cam->at,   0, 0, 0);
  ELL_3V_SET(cam->up,   0, 0, 1);
  cam->uRange[0] = -1.0;
  cam->uRange[1] = 1.0;
  cam->vRange[0] = -1.0;
  cam->vRange[1] = 1.0;

  parm->jitterType = echoJitterNone;
  parm->numSamples = 1;
  parm->imgResU = 300;
  parm->imgResV = 300;
  parm->aperture = 0.0;
  parm->renderLights = AIR_FALSE;
  parm->shadow = 0.0;
  parm->seedRand = AIR_FALSE;
  parm->maxRecDepth = 10;
  parm->mrR = 1.0;
  parm->mrG = 0.0;
  parm->mrB = 1.0;

  cube = echoObjectNew(echoCube);
  printf("cube = %p\n", cube);
  echoMatterGlassSet(cube,
                     1.0, 1.0, 1.0,
                     1.5, 0.0, 0.0);
  echoObjectAdd(scene, cube);

  nrrdLoad(ntext=nrrdNew(), "psq.nrrd", NULL);
  
  rect = echoObjectNew(echoRectangle);
  printf("rect = %p\n", rect);
  echoRectangleSet(rect,
                         -1, -1, -0.51,
                         2, 0, 0,
                         0, 2, 0);
  echoMatterPhongSet(rect, 1.0, 1.0, 1.0, 1.0,
                     0.1, 0.6, 0.3, 40);
  echoMatterTextureSet(rect, ntext);
  echoObjectAdd(scene, rect);

  /*
  light = echoLightNew(echoLightDirectional);
  echoLightDirectionalSet(light, 1, 1, 1, 0, 0, 1);
  echoLightArrayAdd(lightArr, light);
  */
}

void
makeSceneGlass2(limnCamera *cam, echoRTParm *parm, echoObject **sceneP) {
  echoObject *cube, *rect;
  echoObject *scene;
  Nrrd *ntext;
  echoPos_t matx[16];
  
  *sceneP = scene = echoObjectNew(echoList);

  ELL_3V_SET(cam->from, 0, 0, 100);
  ELL_3V_SET(cam->at,   0, 0, 0);
  ELL_3V_SET(cam->up,   0, 1, 0);
  cam->uRange[0] = -1.0;
  cam->uRange[1] = 1.0;
  cam->vRange[0] = -1.0;
  cam->vRange[1] = 1.0;

  parm->jitterType = echoJitterNone;
  parm->numSamples = 1;
  parm->imgResU = 300;
  parm->imgResV = 300;
  parm->aperture = 0.0;
  parm->renderLights = AIR_FALSE;
  parm->shadow = 0.0;
  parm->seedRand = AIR_FALSE;
  parm->maxRecDepth = 10;
  parm->mrR = 1.0;
  parm->mrG = 0.0;
  parm->mrB = 1.0;

  ELL_4M_SET_SCALE(matx, 0.5, 0.5, 0.5);
  cube = echoRoughSphere(80, 40, matx);
  /*
  cube = echoObjectNew(echoSphere);
  echoSphereSet(cube, 0, 0, 0, 0.5);
  */
  echoMatterGlassSet(cube,
                     1.0, 1.0, 1.0,
                     1.33333, 0.0, 0.0);
  echoObjectAdd(scene, cube);

  nrrdLoad(ntext=nrrdNew(), "check.nrrd", NULL);
  
  rect = echoObjectNew(echoRectangle);
  printf("rect = %p\n", rect);
  echoRectangleSet(rect,
                         -1, -1, -0.51,
                         2, 0, 0,
                         0, 2, 0);
  echoMatterPhongSet(rect, 1.0, 1.0, 1.0, 1.0,
                     0.0, 1.0, 0.0, 40);
  echoMatterTextureSet(rect, ntext);
  echoObjectAdd(scene, rect);
  
  /*
  light = echoLightNew(echoLightDirectional);
  echoLightDirectionalSet(light, 1, 1, 1, 0, 0, 1);
  echoLightArrayAdd(lightArr, light);
  */
}

#endif

void
makeSceneInstance(limnCamera *cam, echoRTParm *parm, echoScene *scene) {
  echoObject *trim, *rect, *inst;
  echoPos_t matx[16], A[16], B[16];
  
  ELL_3V_SET(cam->from, 9*1.3, 9*1.3, 11*1.3);
  ELL_3V_SET(cam->at,   0, 0, 0);
  ELL_3V_SET(cam->up,   0, 0, 1);
  cam->uRange[0] = -5;
  cam->uRange[1] = 5;
  cam->vRange[0] = -5;
  cam->vRange[1] = 5;

  parm->jitterType = echoJitterNone;
  parm->numSamples = 1;
  parm->imgResU = 300;
  parm->imgResV = 300;
  parm->aperture = 0.0;
  parm->renderLights = AIR_TRUE;
  parm->renderBoxes = AIR_FALSE;
  parm->seedRand = AIR_FALSE;
  parm->maxRecDepth = 10;
  parm->shadow = 1.0;
  
  ELL_4M_IDENTITY_SET(matx);
  ELL_4M_SCALE_SET(B, 2.5, 1.5, 0.8);
  ELL_4M_MUL(A, B, matx); ELL_4M_COPY(matx, A);
  ELL_4M_ROTATE_X_SET(B, 0.2);
  ELL_4M_MUL(A, B, matx); ELL_4M_COPY(matx, A);
  ELL_4M_ROTATE_Y_SET(B, 0.2);
  ELL_4M_MUL(A, B, matx); ELL_4M_COPY(matx, A);
  ELL_4M_ROTATE_Y_SET(B, 0.2);
  ELL_4M_MUL(A, B, matx); ELL_4M_COPY(matx, A);
  ELL_4M_TRANSLATE_SET(B, 0, 0, 1);
  ELL_4M_MUL(A, B, matx); ELL_4M_COPY(matx, A);


  /* trim = echoRoughSphere(50, 25, matx); */
  /*
  trim = echoRoughSphere(8, 4, matx);
  echoMatterGlassSet(trim, 0.8, 0.8, 0.8,
                     1.3, 0.0, 0.0);
  echoMatterPhongSet(trim, 1, 1, 1, 1.0,
                     0.1, 0.5, 0.9, 50);
  echoObjectAdd(scene, trim);
  */

  trim = echoObjectNew(scene, echoTypeSphere);
  echoSphereSet(trim, 0, 0, 0, 1);
  echoColorSet(trim, 0.8, 0.8, 0.8, 1.0);
  echoMatterGlassSet(scene, trim, 1.3, 0.0, 0.0, 0.0);
  echoMatterPhongSet(scene, trim, 0.1, 0.5, 0.9, 50);
  inst = echoObjectNew(scene, echoTypeInstance);
  echoInstanceSet(inst, matx, trim);
  echoObjectAdd(scene, inst);

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                         -3.5, -3.5, -3.5,
                         7, 0, 0,
                         0, 7, 0);
  echoColorSet(trim, 1.0, 1.0, 1.0, 1.0);
  echoMatterPhongSet(scene, rect, 0.1, 0.5, 0.9, 50);
  echoObjectAdd(scene, rect);
  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                         -3.5, -3.5, -3.5,
                         0, 7, 0,
                         0, 0, 7);
  echoColorSet(rect, 1.0, 1.0, 1.0, 1.0);
  echoMatterPhongSet(scene, rect, 0.1, 0.5, 0.9, 50);
  echoObjectAdd(scene, rect);
  /*
  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                         -3.5, -3.5, -3.5,
                         0, 0, 7,
                         7, 0, 0);
  */
  rect = echoObjectNew(scene, echoTypeSphere);
  echoSphereSet(rect, 0, 0, 0, 1);
  echoColorSet(rect, 1.0, 1.0, 1.0, 1.0);
  echoMatterPhongSet(scene, rect, 0.1, 0.5, 0.9, 50);
  inst = echoObjectNew(scene, echoTypeInstance);
  ELL_4M_SCALE_SET(A, 20, 20, 20);
  ELL_4M_TRANSLATE_SET(B, 0, -(20+3.5), 0);
  ELL_4M_MUL(matx, B, A);
  echoInstanceSet(inst, matx, rect);
  echoObjectAdd(scene, inst);
  

  /*
  light = echoLightNew(echoLightDirectional);
  echoLightDirectionalSet(light, 1, 0, 0, 1, 0.001, 0.001);
  echoLightArrayAdd(lightArr, light);
  light = echoLightNew(echoLightDirectional);
  echoLightDirectionalSet(light, 0, 1, 0, 0.001, 1, 0.001);
  echoLightArrayAdd(lightArr, light);
  light = echoLightNew(echoLightDirectional);
  echoLightDirectionalSet(light, 0, 0, 1, 0.001, 0.001, 1);
  echoLightArrayAdd(lightArr, light);
  */

  return;
}

void
makeSceneGlassTest(limnCamera *cam, echoRTParm *parm, echoScene *scene) {
  echoObject *cube, *rect, *inst;
  echoCol_t r, g, b;
  Nrrd *ntext;
  int i, N;
  echoPos_t ma[16], mb[16];

  ELL_3V_SET(cam->from, 0, 0, 10);
  ELL_3V_SET(cam->at,   0, 0, 0);
  ELL_3V_SET(cam->up,   1, 0, 0);
  cam->uRange[0] = -1.1;
  cam->uRange[1] = 1.1;
  cam->vRange[0] = -1.1;
  cam->vRange[1] = 1.1;

  parm->jitterType = echoJitterNone;
  parm->numSamples = 1;
  parm->imgResU = 220;
  parm->imgResV = 220;
  parm->aperture = 0.0;
  parm->renderLights = AIR_FALSE;
  parm->seedRand = AIR_FALSE;
  ELL_3V_SET(scene->bkgr, 0.2, 0.3, 0.4);
  /* parm->shadow = 0.0; */

  /* create scene */
  N = 11;
  for (i=0; i<N; i++) {
    cube = echoObjectNew(scene, echoTypeCube);
    _dyeHSVtoRGB(&r, &g, &b, AIR_AFFINE(0, i, N, 0.0, 1.0), 1.0, 1.0);
    echoColorSet(cube, r, g, b, 1);
    echoMatterGlassSet(scene, cube, 1.1, 0.0, 0.0, 0);
    inst = echoObjectNew(scene, echoTypeInstance);
    ELL_4M_IDENTITY_SET(ma);
    ELL_4M_SCALE_SET(mb, 1.0/(N+2), 0.8, 3.0/(N+2));
    ell_4m_POST_MUL(ma, mb);
    ELL_4M_ROTATE_X_SET(mb, AIR_AFFINE(0, i, N-1, -AIR_PI/2, AIR_PI/2));
    ell_4m_POST_MUL(ma, mb);
    ELL_4M_TRANSLATE_SET(mb, AIR_AFFINE(0, i, N-1, -0.8, 0.8), 0, 1);
    ell_4m_POST_MUL(ma, mb);
    echoInstanceSet(inst, ma, cube);
    echoObjectAdd(scene, inst);
  }

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   -1, 1, 0,
                   0, -2, 0,
                   2, 0, 0);
  echoColorSet(rect, 1.0, 1.0, 1.0, 1.0);
  echoMatterPhongSet(scene, rect, 1.0, 0.0, 0.0, 40);
  nrrdLoad(ntext=nrrdNew(), "pot.png", NULL);
  echoMatterTextureSet(scene, rect, ntext);
  echoObjectAdd(scene, rect);

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   -0.25, -0.25, 10,
                   0.5, 0, 0,
                   0, 0.5, 0);
  echoColorSet(rect, 1, 1, 1, 1);
  echoMatterLightSet(scene, rect, 1, 0);

  return;
}

void
makeSceneGlassMetal(limnCamera *cam, echoRTParm *parm, echoScene *scene) {
  echoObject *sphere, *rect;
  
  ELL_3V_SET(cam->from, 4, 0, 5);
  ELL_3V_SET(cam->at,   0, 0, 0);
  ELL_3V_SET(cam->up,   -1, 0, 0);
  cam->uRange[0] = -0.7;
  cam->uRange[1] = 0.7;
  cam->vRange[0] = -0.0;
  cam->vRange[1] = 1.4;

  parm->jitterType = echoJitterJitter;
  parm->numSamples = 36;
  parm->imgResU = 220;
  parm->imgResV = 220;
  parm->aperture = 0.0;
  parm->renderLights = AIR_FALSE;
  parm->seedRand = AIR_FALSE;
  ELL_3V_SET(scene->bkgr, 0.2, 0.3, 0.4);

  /* create scene */
  sphere = echoObjectNew(scene, echoTypeSphere);
  echoSphereSet(sphere, 0.70, -0.3, -0.4, 0.1);
  echoColorSet(sphere, 1, 0, 0, 1.0);
  echoMatterPhongSet(scene, sphere, 0.1, 0.6, 0.3, 40);
  echoObjectAdd(scene, sphere);

  sphere = echoObjectNew(scene, echoTypeSphere);
  echoSphereSet(sphere, 0.66, 0.0, -0.4, 0.1);
  echoColorSet(sphere, 0, 1, 0, 1.0);
  echoMatterGlassSet(scene, sphere, 1.0, 0, 1.0, 0.0);
  echoMatterPhongSet(scene, sphere, 0.1, 0.6, 0.3, 40);
  echoObjectAdd(scene, sphere);

  sphere = echoObjectNew(scene, echoTypeSphere);
  echoSphereSet(sphere, 0.62, 0.3, -0.4, 0.1);
  echoColorSet(sphere, 0, 0, 1, 1.0);
  echoMatterPhongSet(scene, sphere, 0.1, 0.6, 0.3, 40);
  echoObjectAdd(scene, sphere);

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   0.5, 0.5, 0.5,
                   0, -1, 0,
                   0, 0, -1);
  echoColorSet(rect, 1.0, 1.0, 1.0, 1.0);
  echoMatterMetalSet(scene, rect, 0.7, 0.0, 0.0, 0.2);
  echoObjectAdd(scene, rect);

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   -1.5, -1.5, -1,
                   3, 0, 0,
                   0, 3, 0);
  echoColorSet(rect, 1.0, 1.0, 1.0, 1.0);
  echoMatterPhongSet(scene, rect, 0.1, 0.6, 0.3, 40);
  echoObjectAdd(scene, rect);

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   0.5-0.2, -0.2, 40,
                   0.4, 0, 0,
                   0, 0.4, 0);
  echoColorSet(rect, 1, 1, 1, 1);
  echoMatterLightSet(scene, rect, 1, 0);

  return;
}

void
makeSceneTexture(limnCamera *cam, echoRTParm *parm, echoScene *scene) {
  echoObject /* *trim, */ *rect, /* *inst, */ *sphere;
  Nrrd *ntext;
  
  ELL_3V_SET(cam->from, 9, 9, 11);
  ELL_3V_SET(cam->at,   0, 0, 0);
  ELL_3V_SET(cam->up,   0, 0, 1);
  cam->uRange[0] = -4;
  cam->uRange[1] = 4;
  cam->vRange[0] = -4;
  cam->vRange[1] = 4;

  parm->jitterType = echoJitterNone;
  parm->numSamples = 1;
  parm->imgResU = 300;
  parm->imgResV = 300;
  parm->aperture = 0.0;
  parm->renderLights = AIR_FALSE;
  parm->renderBoxes = AIR_FALSE;
  parm->seedRand = AIR_FALSE;
  parm->maxRecDepth = 10;
  parm->shadow = 1.0;

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   -2, -2, 0,
                   4, 0, 0,
                   0, 4, 0);
  echoColorSet(rect, 1, 1, 1, 1);
  echoMatterPhongSet(scene, rect, 0.1, 1, 0.9, 50);
  echoObjectAdd(scene, rect);

  nrrdLoad(ntext=nrrdNew(), "tmp.png", NULL);
  echoMatterTextureSet(scene, rect, ntext);

  sphere = echoObjectNew(scene, echoTypeSphere);
  echoSphereSet(sphere, 0, 0, 0, 3);
  echoColorSet(sphere, 1, 1, 1, 1);
  echoMatterPhongSet(scene, sphere, 0.1, 0.5, 0.9, 50);
  echoMatterTextureSet(scene, sphere, ntext);
  echoObjectAdd(scene, sphere);

  /*
  ELL_4M_SET_SCALE(matx, 3, 3, 3);
  trim = echoRoughSphere(80, 40, matx);
  echoMatterPhongSet(trim, 1, 1, 1, 1.0,
                     0.1, 0.5, 0.9, 50);
  echoMatterTextureSet(trim, ntext);
  echoObjectAdd(scene, trim);
  */

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   0, 0, 60,
                   0, 2, 0,
                   0, 0, 2);
  echoColorSet(rect, 1, 1, 1, 1);
  echoMatterLightSet(scene, rect, 1, 1);

  return;
}

void
makeSceneDOF(limnCamera *cam, echoRTParm *parm, echoScene *scene) {
  echoObject *rect;
  Nrrd *ntext;
  
  ELL_3V_SET(cam->from, 6, 6, 20);
  ELL_3V_SET(cam->at,   0, 0, 0);
  ELL_3V_SET(cam->up,   0, 1, 0);
  cam->uRange[0] = -3.3;
  cam->uRange[1] = 3.3;
  cam->vRange[0] = -3.3;
  cam->vRange[1] = 3.3;

  parm->jitterType = echoJitterJitter;
  parm->numSamples = 4;
  parm->imgResU = 300;
  parm->imgResV = 300;
  parm->aperture = 0.5;
  parm->renderLights = AIR_FALSE;
  parm->renderBoxes = AIR_FALSE;
  parm->seedRand = AIR_FALSE;
  parm->maxRecDepth = 10;
  parm->shadow = 1.0;

  nrrdLoad(ntext = nrrdNew(), "tmp.png", NULL);

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   -0.5, 1.5, -3,
                   2, 0, 0,
                   0, -2, 0);
  echoColorSet(rect, 1, 0.5, 0.5, 1);
  echoMatterPhongSet(scene, rect, 1.0, 0.0, 0.0, 1);
  echoMatterTextureSet(scene, rect, ntext);
  echoObjectAdd(scene, rect);

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   -1, 1, 0,
                   2, 0, 0,
                   0, -2, 0);
  echoColorSet(rect, 0.5, 1, 0.5, 1);
  echoMatterPhongSet(scene, rect, 1.0, 0.0, 0.0, 1);
  echoMatterTextureSet(scene, rect, ntext);
  echoObjectAdd(scene, rect);

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   -1.5, 0.5, 3,
                   2, 0, 0,
                   0, -2, 0);
  echoColorSet(rect, 0.5, 0.5, 1, 1);
  echoMatterPhongSet(scene, rect, 1.0, 0.0, 0.0, 1);
  echoMatterTextureSet(scene, rect, ntext);
  echoObjectAdd(scene, rect);

  return;
}

void
makeSceneShadow(limnCamera *cam, echoRTParm *parm, echoScene *scene) {
  echoObject *sphere, *rect, *tri;

  ELL_3V_SET(cam->from, 2, 0, 20);
  ELL_3V_SET(cam->at,   0, 0, 0);
  ELL_3V_SET(cam->up,   -1, 0, 0);
  cam->uRange[0] = -1.8;
  cam->uRange[1] = 1.8;
  cam->vRange[0] = -1.8;
  cam->vRange[1] = 1.8;

  parm->jitterType = echoJitterGrid;
  parm->numSamples = 9;
  parm->imgResU = 200;
  parm->imgResV = 200;
  parm->aperture = 0.0;
  parm->renderLights = AIR_FALSE;
  parm->shadow = 0.5;

  /* create scene */
  sphere = echoObjectNew(scene, echoTypeSphere);
  echoSphereSet(sphere, 0, -1, -1, 0.2);
  echoColorSet(sphere, 0.5, 0.5, 1, 1.0);
  echoMatterPhongSet(scene, sphere, 0.1, 0.6, 0.3, 40);
  echoObjectAdd(scene, sphere);

  sphere = echoObjectNew(scene, echoTypeSphere);
  echoSphereSet(sphere, 0, 1, -1, 0.2);
  echoColorSet(sphere, 1, 0.5, 0.5, 1.0);
  echoMatterPhongSet(scene, sphere, 0.1, 0.6, 0.3, 40);
  echoObjectAdd(scene, sphere);

  sphere = echoObjectNew(scene, echoTypeSphere);
  echoSphereSet(sphere, 0, 0, 1, 0.2);
  echoColorSet(sphere, 0.5, 1, 0.5, 1.0);
  echoMatterPhongSet(scene, sphere, 0.1, 0.6, 0.3, 40);
  echoObjectAdd(scene, sphere);

  tri = echoObjectNew(scene, echoTypeTriangle);
  echoTriangleSet(tri,
                  0, -1, -1,
                  0, 1, -1,
                  0, 0, 1);
  echoColorSet(tri, 1, 1, 0, 1.0);
  echoMatterPhongSet(scene, tri, 0.1, 0.6, 0.3, 40);
  echoObjectAdd(scene, tri);
  
  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   1.7, 1.7, -2,
                   -3.4, 0, 0,
                   0, -3.4, 0);
  echoColorSet(rect, 1.0, 0.8, 1.0, 1.0);
  echoMatterPhongSet(scene, rect, 0.1, 0.3, 0.7, 3000);
  echoObjectAdd(scene, rect);

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   1.0, 0.2, 4,
                   0.2, 0, 0,
                   0, 0.2, 0);
  echoColorSet(rect, 1, 1, 1, 1);
  echoMatterLightSet(scene, rect, 1, 0);
  echoObjectAdd(scene, rect);
}

void
makeSceneSimple(limnCamera *cam, echoRTParm *parm, echoScene *scene) {
  echoObject *tri, *rect, *sphere;
  Nrrd *ntext;

  ELL_3V_SET(cam->from, 5, -5, 9);
  ELL_3V_SET(cam->at,   0, 0, 0);
  ELL_3V_SET(cam->up,   0, 0, 1);
  cam->uRange[0] = -3.6;
  cam->uRange[1] = 3.6;
  cam->vRange[0] = -3.6;
  cam->vRange[1] = 3.6;

  parm->jitterType = echoJitterJitter;
  parm->numSamples = 9;
  parm->imgResU = 300;
  parm->imgResV = 300;
  parm->aperture = 0.0;
  parm->textureNN = AIR_FALSE;
  parm->renderLights = AIR_TRUE;
  parm->renderBoxes = AIR_FALSE;
  parm->seedRand = AIR_TRUE;
  parm->maxRecDepth = 10;
  ELL_3V_SET(parm->maxRecCol, 0, 0, 0);
  parm->shadow = 1.0;

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   -5, -5, -1.4,
                   10, 0, 0,
                   0, 10, 0);
  echoColorSet(rect, 1, 1, 1, 1.0);
  echoMatterPhongSet(scene, rect, 0.1, 0.5, 0.6, 50);
  if (nrrdLoad(ntext=nrrdNew(), "pot.png", NULL)) {
    /* oops, no pot */
    airFree(biffGetDone(NRRD));
    nrrdNuke(ntext);
  } else {
    echoMatterTextureSet(scene, rect, ntext);
  }
  echoObjectAdd(scene, rect);

  sphere = echoObjectNew(scene, echoTypeSphere);
  echoSphereSet(sphere, 0, 0, 0, 1.85);
  echoColorSet(sphere, 1, 1, 1, 1.0);
  echoMatterGlassSet(scene, sphere, 1.5, 0.0, 0.0, 0.0);
  echoMatterMetalSet(scene, sphere, 0.8, 0.0, 1.0, 0.15);
  echoObjectAdd(scene, sphere);

  tri = echoObjectNew(scene, echoTypeTriangle);
  echoTriangleSet(tri,
                  0.1, 0.1, 2,
                  2, 2, 2,
                  0, 2, 2);
  echoColorSet(tri, 1, 0.4, 0.4, 1.0);
  echoMatterPhongSet(scene, tri, 0.4, 0.6, 0.0, 90);
  echoObjectAdd(scene, tri);

  tri = echoObjectNew(scene, echoTypeTriangle);
  echoTriangleSet(tri,
                  -0.1, 0.1, 2,
                  -2, 2, 2,
                  -2, 0, 2);
  echoColorSet(tri, 0.4, 1.0, 0.4, 1.0);
  echoMatterPhongSet(scene, tri, 0.4, 0.6, 0.0, 90);
  echoObjectAdd(scene, tri);

  tri = echoObjectNew(scene, echoTypeTriangle);
  echoTriangleSet(tri,
                  -0.1, -0.1, 2,
                  -2, -2, 2,
                  0, -2, 2);
  echoColorSet(tri, 0.4, 0.4, 1.0, 1.0);
  echoMatterPhongSet(scene, tri, 0.4, 0.6, 0.0, 90);
  echoObjectAdd(scene, tri);

  rect = echoObjectNew(scene, echoTypeRectangle);
  echoRectangleSet(rect,
                   -0.5, -0.5, 10,
                   1.0, 0.0, 0,
                   0.0, 1.0, 0);
  echoColorSet(rect, 1, 1, 1, 1);
  echoMatterLightSet(scene, rect, 1, 0);
  echoObjectAdd(scene, rect);

  return;
}

void
makeSceneRainLights(limnCamera *cam, echoRTParm *parm, echoScene *scene) {
  echoObject *sphere, *rect;
  int i, N;
  echoPos_t w;
  float r, g, b;

  ELL_3V_SET(cam->from, 2.5, 0, 5);
  ELL_3V_SET(cam->at,   0, 0, 0);
  ELL_3V_SET(cam->up,   0, 0, 1);
  cam->uRange[0] = -1.7;
  cam->uRange[1] = 1.7;
  cam->vRange[0] = -1.7;
  cam->vRange[1] = 1.7;

  parm->jitterType = echoJitterJitter;
  parm->numSamples = 36;
  parm->imgResU = 1000;
  parm->imgResV = 1000;
  parm->numSamples = 16;
  parm->imgResU = 200;
  parm->imgResV = 200;
  parm->aperture = 0.0;
  parm->renderLights = AIR_TRUE;
  parm->shadow = 0.0;
  ELL_3V_SET(scene->bkgr, 0.1, 0.1, 0.1);

  /* create scene */
  sphere = echoObjectNew(scene, echoTypeSphere);
  echoSphereSet(sphere, 0, 0, 0, 1.0);
  echoColorSet(sphere, 1.0, 1.0, 1.0, 1.0);
  echoMatterPhongSet(scene, sphere, 0.02, 0.2, 1.0, 400);
  echoObjectAdd(scene, sphere);

  N = 8;
  w = 1.7/N;

  for (i=0; i<N; i++) {
    rect = echoObjectNew(scene, echoTypeRectangle);
    echoRectangleSet(rect,
                     w/2, AIR_AFFINE(0, i, N-1, -1-w/2, 1-w/2), 1.5,
                     0, w, 0,
                     w, 0, 0);
    _dyeHSVtoRGB(&r, &g, &b, AIR_AFFINE(0, i, N, 0.0, 1.0), 1.0, 1.0);
    echoColorSet(rect, r, g, b, 1);
    echoMatterLightSet(scene, rect, 1, 0);
    echoObjectAdd(scene, rect);
  }

}

int
main(int argc, char **argv) {
  Nrrd *nraw;
  limnCamera *cam;
  echoRTParm *parm;
  echoGlobalState *state;
  echoScene *scene;
  airArray *mop;
  char *me, *err, *env;
  int E, tmp;

  AIR_UNUSED(argc);
  me = argv[0];

  mop = airMopNew();

  cam = limnCameraNew();
  airMopAdd(mop, cam, (airMopper)limnCameraNix, airMopAlways);
  cam->neer = 0;
  cam->dist = 0;
  cam->faar = 0;
  cam->atRelative = AIR_TRUE;
  cam->dist = 0;
  cam->rightHanded = AIR_TRUE;

  parm = echoRTParmNew();
  airMopAdd(mop, parm, (airMopper)echoRTParmNix, airMopAlways);

  state = echoGlobalStateNew();
  airMopAdd(mop, state, (airMopper)echoGlobalStateNix, airMopAlways);

  scene = echoSceneNew();
  airMopAdd(mop, scene, (airMopper)echoSceneNix, airMopAlways);

  nraw = nrrdNew();
  airMopAdd(mop, nraw, (airMopper)nrrdNuke, airMopAlways);

  /* makeSceneGlass(cam, parm, scene); */
  /* makeSceneGlass2(cam, parm, scene); */
  /* makeSceneGlassMetal(cam, parm, scene); */
  /* makeSceneGlassTest(cam, parm, scene); */
  /* makeSceneBVH(cam, parm, scene); */
  /* makeSceneInstance(cam, parm, scene); */
  /* makeSceneTexture(cam, parm, scene); */
  /* makeSceneSimple(cam, parm, scene);  */
  /* makeSceneRainLights(cam, parm, scene); */
  /* makeSceneAntialias(cam, parm, scene); */
  makeSceneShadow(cam, parm, scene); 
  /* makeSceneDOF(cam, parm, scene); */

  if ((env = getenv("NT"))) {
    if (1 == sscanf(env, "%d", &tmp)) {
      parm->numThreads = tmp;
    }
  } else {
    parm->numThreads = 1;
  }

  E = 0;
  if (!E) E |= echoRTRender(nraw, cam, scene, parm, state);
  if (E) {
    airMopAdd(mop, err = biffGetDone(ECHO), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  printf("render time = %g seconds (%g fps)\n",
         state->time, 1.0/state->time);
  if (!E) E |= nrrdSave("raw.nrrd", nraw, NULL);
  if (E) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
