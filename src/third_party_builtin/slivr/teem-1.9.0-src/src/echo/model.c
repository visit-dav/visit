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

#include "echo.h"
#include "privateEcho.h"

void
_echoPosSet(echoPos_t *p3, echoPos_t *matx, echoPos_t *p4) {
  echoPos_t a[4], b[4];
  
  if (matx) {
    ELL_4V_SET(a, p4[0], p4[1], p4[2], 1);
    ELL_4MV_MUL(b, matx, a);
    ELL_34V_HOMOG(p3, b);
  }
  else {
    ELL_3V_COPY(p3, p4);
  }
}

echoObject *
echoRoughSphereNew(echoScene *scene, int theRes, int phiRes, echoPos_t *matx) {
  echoObject *trim;
  echoPos_t *_pos, *pos, tmp[3];
  int *_vert, *vert, thidx, phidx, n;
  echoPos_t th, ph;

  trim = echoObjectNew(scene, echoTypeTriMesh);
  TRIMESH(trim)->numV = 2 + (phiRes-1)*theRes;
  TRIMESH(trim)->numF = (2 + 2*(phiRes-2))*theRes;

  _pos = pos = (echoPos_t *)calloc(3*TRIMESH(trim)->numV, sizeof(echoPos_t));
  _vert = vert = (int *)calloc(3*TRIMESH(trim)->numF, sizeof(int));

  ELL_3V_SET(tmp, 0, 0, 1); _echoPosSet(pos, matx, tmp); pos += 3;
  for (phidx=1; phidx<phiRes; phidx++) {
    ph = AIR_AFFINE(0, phidx, phiRes, 0.0, AIR_PI);
    for (thidx=0; thidx<theRes; thidx++) {
      th = AIR_AFFINE(0, thidx, theRes, 0.0, 2*AIR_PI);
      ELL_3V_SET(tmp, cos(th)*sin(ph), sin(th)*sin(ph), cos(ph));
      _echoPosSet(pos, matx, tmp); pos += 3;
    }
  }
  ELL_3V_SET(tmp, 0, 0, -1); _echoPosSet(pos, matx, tmp);

  for (thidx=0; thidx<theRes; thidx++) {
    n = AIR_MOD(thidx+1, theRes);
    ELL_3V_SET(vert, 0, 1+thidx, 1+n); vert += 3;
  }
  for (phidx=0; phidx<phiRes-2; phidx++) {
    for (thidx=0; thidx<theRes; thidx++) {
      n = AIR_MOD(thidx+1, theRes);
      ELL_3V_SET(vert, 1+phidx*theRes+thidx, 1+(1+phidx)*theRes+thidx,
                 1+phidx*theRes+n); vert += 3;
      ELL_3V_SET(vert, 1+(1+phidx)*theRes+thidx, 1+(1+phidx)*theRes+n, 
                 1+phidx*theRes+n); vert += 3;
    }
  }
  for (thidx=0; thidx<theRes; thidx++) {
    n = AIR_MOD(thidx+1, theRes);
    ELL_3V_SET(vert, 1+(phiRes-2)*theRes+thidx, TRIMESH(trim)->numV-1,
               1+(phiRes-2)*theRes+n); 
    vert += 3;
  }

  echoTriMeshSet(trim, TRIMESH(trim)->numV, _pos, TRIMESH(trim)->numF, _vert);
  return(trim);
}
