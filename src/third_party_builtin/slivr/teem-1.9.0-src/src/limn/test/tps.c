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

char *info = ("Render something in postscript.");

int
main(int argc, char *argv[]) {
  char *me, *err, *outS;
  limnCamera *cam;
  float matA[16], matB[16], winscale, edgeWidth[5];
  hestOpt *hopt=NULL;
  airArray *mop;
  limnObject *obj;
  limnLook *look; int lookIdx;
  limnWindow *win;
  int partIdx, wire, concave;
  Nrrd *nmap;

  mop = airMopNew();
  cam = limnCameraNew();
  airMopAdd(mop, cam, (airMopper)limnCameraNix, airMopAlways);
  
  me = argv[0];
  hestOptAdd(&hopt, "fr", "from point", airTypeDouble, 3, 3, cam->from,"4 4 4",
             "position of camera, used to determine view vector");
  hestOptAdd(&hopt, "at", "at point", airTypeDouble, 3, 3, cam->at, "0 0 0",
             "camera look-at point, used to determine view vector");
  hestOptAdd(&hopt, "up", "up vector", airTypeDouble, 3, 3, cam->up, "0 0 1",
             "camera pseudo-up vector, used to determine view coordinates");
  hestOptAdd(&hopt, "rh", NULL, airTypeInt, 0, 0, &(cam->rightHanded), NULL,
             "use a right-handed UVN frame (V points down)");
  hestOptAdd(&hopt, "or", NULL, airTypeInt, 0, 0, &(cam->orthographic), NULL,
             "use orthogonal projection");
  hestOptAdd(&hopt, "ur", "uMin uMax", airTypeDouble, 2, 2, cam->uRange,
             "-1 1", "range in U direction of image plane");
  hestOptAdd(&hopt, "vr", "vMin vMax", airTypeDouble, 2, 2, cam->vRange,
             "-1 1", "range in V direction of image plane");
  hestOptAdd(&hopt, "e", "envmap", airTypeOther, 1, 1, &nmap, NULL,
             "16checker-based environment map",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "ws", "winscale", airTypeFloat, 1, 1, &winscale,
             "200", "world to points (PostScript) scaling");
  hestOptAdd(&hopt, "wire", NULL, airTypeInt, 0, 0, &wire, NULL,
             "just do wire-frame rendering");
  hestOptAdd(&hopt, "concave", NULL, airTypeInt, 0, 0, &concave, NULL,
             "use slightly buggy rendering method suitable for "
             "concave or self-occluding objects");
  hestOptAdd(&hopt, "wd", "5 widths", airTypeFloat, 5, 5, edgeWidth,
             "0.0 0.0 3.0 2.0 0.0",
             "width of edges drawn for five kinds of "
             "edges: back non-crease, back crease, "
             "silohuette, front crease, front non-crease");
  hestOptAdd(&hopt, "o", "output PS", airTypeString, 1, 1, &outS, "out.ps",
             "output file to render postscript into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  cam->neer = -0.000000001;
  cam->dist = 0;
  cam->faar = 0.0000000001;
  cam->atRelative = AIR_TRUE;

  if (limnCameraUpdate(cam)) {
    fprintf(stderr, "%s: trouble:\n%s\n", me, err = biffGet(LIMN));
    free(err);
    return 1;
  }
  obj = limnObjectNew(10, AIR_TRUE);
  airMopAdd(mop, obj, (airMopper)limnObjectNix, airMopAlways);

  /* create limnLooks for diffuse (#0) and flat (#1) shading */
  lookIdx = airArrayLenIncr(obj->lookArr, 2);
  look = obj->look + lookIdx + 0;
  ELL_4V_SET(look->rgba, 1, 1, 1, 1);
  ELL_3V_SET(look->kads, 0, 1, 0);
  look->spow = 0;
  look = obj->look + lookIdx + 1;
  ELL_4V_SET(look->rgba, 1, 1, 1, 1);
  ELL_3V_SET(look->kads, 1, 0, 0);
  look->spow = 0;

  /* X axis: rod */
  partIdx = limnObjectCylinderAdd(obj, 0, 0, 16);
  ELL_4M_IDENTITY_SET(matA);
  ELL_4M_SCALE_SET(matB, 1, 0.2, 0.2); ell_4m_post_mul_f(matA, matB);
  ELL_4M_TRANSLATE_SET(matB, 1.3, 0.0, 0.0); ell_4m_post_mul_f(matA, matB);
  limnObjectPartTransform(obj, partIdx, matA);

  /* Y axis: rod + ball */
  partIdx = limnObjectCylinderAdd(obj, 0, 1, 16);
  ELL_4M_IDENTITY_SET(matA);
  ELL_4M_SCALE_SET(matB, 0.2, 1, 0.2); ell_4m_post_mul_f(matA, matB);
  ELL_4M_TRANSLATE_SET(matB, 0.0, 1.3, 0.0); ell_4m_post_mul_f(matA, matB);
  limnObjectPartTransform(obj, partIdx, matA);

  partIdx = limnObjectPolarSphereAdd(obj, 0, 0, 32, 16);
  ELL_4M_IDENTITY_SET(matA);
  ELL_4M_SCALE_SET(matB, 0.28, 0.28, 0.28); ell_4m_post_mul_f(matA, matB);
  ELL_4M_TRANSLATE_SET(matB, 0.0, 2.6, 0.0); ell_4m_post_mul_f(matA, matB);
  limnObjectPartTransform(obj, partIdx, matA);

  /* Z axis: rod + ball + ball */
  partIdx = limnObjectCylinderAdd(obj, 0, 2, 16);
  ELL_4M_IDENTITY_SET(matA);
  ELL_4M_SCALE_SET(matB, 0.2, 0.2, 1); ell_4m_post_mul_f(matA, matB);
  ELL_4M_TRANSLATE_SET(matB, 0.0, 0.0, 1.3); ell_4m_post_mul_f(matA, matB);
  limnObjectPartTransform(obj, partIdx, matA);

  partIdx = limnObjectPolarSphereAdd(obj, 0, 1, 32, 16);
  ELL_4M_IDENTITY_SET(matA);
  ELL_4M_SCALE_SET(matB, 0.28, 0.28, 0.28); ell_4m_post_mul_f(matA, matB);
  ELL_4M_TRANSLATE_SET(matB, 0.0, 0.0, 2.6); ell_4m_post_mul_f(matA, matB);
  limnObjectPartTransform(obj, partIdx, matA);

  partIdx = limnObjectPolarSphereAdd(obj, 0, 2, 32, 16);
  ELL_4M_IDENTITY_SET(matA);
  ELL_4M_SCALE_SET(matB, 0.28, 0.28, 0.28); ell_4m_post_mul_f(matA, matB);
  ELL_4M_TRANSLATE_SET(matB, 0.0, 0.0, 3.2); ell_4m_post_mul_f(matA, matB);
  limnObjectPartTransform(obj, partIdx, matA);

  win = limnWindowNew(limnDevicePS);
  win->scale = winscale;
  win->ps.wireFrame = wire;
  win->ps.lineWidth[limnEdgeTypeBackFacet] = edgeWidth[0];
  win->ps.lineWidth[limnEdgeTypeBackCrease] = edgeWidth[1];
  win->ps.lineWidth[limnEdgeTypeContour] = edgeWidth[2];
  win->ps.lineWidth[limnEdgeTypeFrontCrease] = edgeWidth[3];
  win->ps.lineWidth[limnEdgeTypeFrontFacet] = edgeWidth[4];

  win->file = fopen(outS, "w");
  airMopAdd(mop, win, (airMopper)limnWindowNix, airMopAlways);

  if (limnObjectRender(obj, cam, win)
      || (concave
          ? limnObjectPSDrawConcave(obj, cam, nmap, win)
          : limnObjectPSDraw(obj, cam, nmap, win))) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  fclose(win->file);
  
  airMopOkay(mop);
  return 0;
}

