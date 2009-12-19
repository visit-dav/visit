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


#include "limn.h"

int
limnObjectCubeAdd(limnObject *obj, unsigned int lookIdx) {
  unsigned int vII[4], vII0, partIdx;

  partIdx = limnObjectPartAdd(obj);
  /* HEY: we have to set this first so that 
     obj->setVertexRGBAFromLook can do the right thing */
  obj->part[partIdx]->lookIdx = lookIdx;
  /*
                                     7     6

                  z               4     5
                  |    y
                  |   /              3     2
                  |  /
                  | /             0     1
                    ------ x
  */
  vII0 = limnObjectVertexAdd(obj, partIdx, -1, -1, -1);
  limnObjectVertexAdd(obj, partIdx, 1, -1, -1);
  limnObjectVertexAdd(obj, partIdx, 1,  1, -1);
  limnObjectVertexAdd(obj, partIdx, -1,  1, -1);
  limnObjectVertexAdd(obj, partIdx, -1, -1,  1);
  limnObjectVertexAdd(obj, partIdx, 1, -1,  1);
  limnObjectVertexAdd(obj, partIdx, 1,  1,  1);
  limnObjectVertexAdd(obj, partIdx, -1,  1,  1);
  ELL_4V_SET(vII, vII0+3, vII0+2, vII0+1, vII0+0);
  limnObjectFaceAdd(obj, partIdx, lookIdx, 4, vII);
  ELL_4V_SET(vII, vII0+1, vII0+5, vII0+4, vII0+0);
  limnObjectFaceAdd(obj, partIdx, lookIdx, 4, vII);
  ELL_4V_SET(vII, vII0+2, vII0+6, vII0+5, vII0+1);
  limnObjectFaceAdd(obj, partIdx, lookIdx, 4, vII);
  ELL_4V_SET(vII, vII0+3, vII0+7, vII0+6, vII0+2);
  limnObjectFaceAdd(obj, partIdx, lookIdx, 4, vII);
  ELL_4V_SET(vII, vII0+0, vII0+4, vII0+7, vII0+3);
  limnObjectFaceAdd(obj, partIdx, lookIdx, 4, vII);
  ELL_4V_SET(vII, vII0+5, vII0+6, vII0+7, vII0+4);
  limnObjectFaceAdd(obj, partIdx, lookIdx, 4, vII);

  return partIdx;
}

int
limnObjectSquareAdd(limnObject *obj, unsigned int lookIdx) {
  unsigned int vII0, vII[4], partIdx;

  partIdx = limnObjectPartAdd(obj);
  /* HEY: we have to set this first so that 
     obj->setVertexRGBAFromLook can do the right thing */
  obj->part[partIdx]->lookIdx = lookIdx;
  vII0 = limnObjectVertexAdd(obj, partIdx, 0, 0, 0);
  limnObjectVertexAdd(obj, partIdx, 1, 0, 0);
  limnObjectVertexAdd(obj, partIdx, 1, 1, 0);
  limnObjectVertexAdd(obj, partIdx, 0, 1, 0);
  ELL_4V_SET(vII, vII0+0, vII0+1, vII0+2, vII0+3); 
  limnObjectFaceAdd(obj, partIdx, lookIdx, 4, vII);
  return partIdx;
}

/*
******** limnObjectCylinderAdd
**
** adds a cylinder that fills up the bi-unit cube [-1,1]^3,
** with axis "axis" (0:X, 1:Y, 2:Z), with discretization "res"
*/
int
limnObjectCylinderAdd(limnObject *obj, unsigned int lookIdx,
                      unsigned int axis, unsigned int res) {
  unsigned int partIdx, ii, jj, tmp, vII0=0, *vII;
  double theta;
  
  partIdx = limnObjectPartAdd(obj);
  /* HEY: we have to set this first so that 
     obj->setVertexRGBAFromLook can do the right thing */
  obj->part[partIdx]->lookIdx = lookIdx;
  vII = (unsigned int *)calloc(res, sizeof(unsigned int));

  for (ii=0; ii<=res-1; ii++) {
    theta = AIR_AFFINE(0, ii, res, 0, 2*AIR_PI);
    switch(axis) {
    case 0:
      tmp = limnObjectVertexAdd(obj, partIdx,
                                1,
                                AIR_CAST(float, -sin(theta)),
                                AIR_CAST(float, cos(theta)));
      limnObjectVertexAdd(obj, partIdx,
                          -1,
                          AIR_CAST(float, -sin(theta)),
                          AIR_CAST(float, cos(theta)));
      break;
    case 1:
      tmp = limnObjectVertexAdd(obj, partIdx,
                                AIR_CAST(float, sin(theta)),
                                1,
                                AIR_CAST(float, cos(theta)));
      limnObjectVertexAdd(obj, partIdx,
                          AIR_CAST(float, sin(theta)),
                          -1,
                          AIR_CAST(float, cos(theta)));
      break;
    case 2: default:
      tmp = limnObjectVertexAdd(obj, partIdx,
                                AIR_CAST(float, cos(theta)),
                                AIR_CAST(float, sin(theta)),
                                1);
      limnObjectVertexAdd(obj, partIdx,
                          AIR_CAST(float, cos(theta)),
                          AIR_CAST(float, sin(theta)),
                          -1);
      break;
    }
    if (!ii) {
      vII0 = tmp;
    }
  }
  /* add all side faces */
  for (ii=0; ii<=res-1; ii++) {
    jj = (ii+1) % res;
    ELL_4V_SET(vII, vII0 + 2*ii, vII0 + 2*ii + 1,
               vII0 + 2*jj + 1, vII0 + 2*jj);
    limnObjectFaceAdd(obj, partIdx, lookIdx, 4, vII);
  }
  /* add top */
  for (ii=0; ii<=res-1; ii++) {
    vII[ii] = vII0 + 2*ii;
  }
  limnObjectFaceAdd(obj, partIdx, lookIdx, res, vII);
  /* add bottom */
  for (ii=0; ii<=res-1; ii++) {
    vII[ii] = vII0 + 2*(res-1-ii) + 1;
  }
  limnObjectFaceAdd(obj, partIdx, lookIdx, res, vII);
  
  free(vII);
  return partIdx;
}

int
limnObjectConeAdd(limnObject *obj, unsigned int lookIdx,
                  unsigned int axis, unsigned int res) {
  double th;
  unsigned int partIdx, tmp, vII0=0, ii, jj, *vII;

  vII = (unsigned int *)calloc(res, sizeof(unsigned int));

  partIdx = limnObjectPartAdd(obj);
  /* HEY: we have to set this first so that 
     obj->setVertexRGBAFromLook can do the right thing */
  obj->part[partIdx]->lookIdx = lookIdx;
  for (ii=0; ii<=res-1; ii++) {
    th = AIR_AFFINE(0, ii, res, 0, 2*AIR_PI);
    switch(axis) {
    case 0:
      tmp = limnObjectVertexAdd(obj, partIdx,
                                0,
                                AIR_CAST(float, -sin(th)),
                                AIR_CAST(float, cos(th)));
      break;
    case 1:
      tmp = limnObjectVertexAdd(obj, partIdx,
                                AIR_CAST(float, sin(th)),
                                0,
                                AIR_CAST(float, cos(th)));
      break;
    case 2: default:
      tmp = limnObjectVertexAdd(obj, partIdx,
                                AIR_CAST(float, cos(th)),
                                AIR_CAST(float, sin(th)),
                                0);
      break;
    }
    if (!ii) {
      vII0 = tmp;
    }
  }
  switch(axis) {
  case 0:
    limnObjectVertexAdd(obj, partIdx, 1, 0, 0);
    break;
  case 1:
    limnObjectVertexAdd(obj, partIdx, 0, 1, 0);
    break;
  case 2: default:
    limnObjectVertexAdd(obj, partIdx, 0, 0, 1);
    break;
  }
  for (ii=0; ii<=res-1; ii++) {
    jj = (ii+1) % res;
    ELL_3V_SET(vII, vII0+ii, vII0+jj, vII0+res);
    limnObjectFaceAdd(obj, partIdx, lookIdx, 3, vII);
  }
  for (ii=0; ii<=res-1; ii++) {
    vII[ii] = vII0+res-1-ii;
  }
  limnObjectFaceAdd(obj, partIdx, lookIdx, res, vII);
  
  free(vII);
  return partIdx;
}

int
limnObjectPolarSphereAdd(limnObject *obj, unsigned int lookIdx,
                         unsigned int axis, unsigned int thetaRes,
                         unsigned int phiRes) {
  unsigned int partIdx, vII0, nti, ti, pi, vII[4], pl;
  double t, p;
  
  thetaRes = AIR_MAX(thetaRes, 3);
  phiRes = AIR_MAX(phiRes, 2);
  
  partIdx = limnObjectPartAdd(obj);
  /* HEY: we have to set this first so that 
     obj->setVertexRGBAFromLook can do the right thing */
  obj->part[partIdx]->lookIdx = lookIdx;
  switch(axis) {
  case 0:
    vII0 = limnObjectVertexAdd(obj, partIdx, 1, 0, 0);
    break;
  case 1:
    vII0 = limnObjectVertexAdd(obj, partIdx, 0, 1, 0);
    break;
  case 2: default:
    vII0 = limnObjectVertexAdd(obj, partIdx, 0, 0, 1);
    break;
  }
  for (pi=1; pi<=phiRes-1; pi++) {
    p = AIR_AFFINE(0, pi, phiRes, 0, AIR_PI);
    for (ti=0; ti<=thetaRes-1; ti++) {
      t = AIR_AFFINE(0, ti, thetaRes, 0, 2*AIR_PI);
      switch(axis) {
      case 0:
        limnObjectVertexAdd(obj, partIdx,
                            AIR_CAST(float, cos(p)),
                            AIR_CAST(float, -sin(t)*sin(p)),
                            AIR_CAST(float, cos(t)*sin(p)));
        break;
      case 1:
        limnObjectVertexAdd(obj, partIdx,
                            AIR_CAST(float, sin(t)*sin(p)),
                            AIR_CAST(float, cos(p)),
                            AIR_CAST(float, cos(t)*sin(p)));
        break;
      case 2: default:
        limnObjectVertexAdd(obj, partIdx,
                            AIR_CAST(float, cos(t)*sin(p)),
                            AIR_CAST(float, sin(t)*sin(p)),
                            AIR_CAST(float, cos(p)));
        break;
      }
    }
  }
  switch(axis) {
  case 0:
    pl = limnObjectVertexAdd(obj, partIdx, -1, 0, 0);
    break;
  case 1:
    pl = limnObjectVertexAdd(obj, partIdx, 0, -1, 0);
    break;
  case 2: default:
    pl = limnObjectVertexAdd(obj, partIdx, 0, 0, -1);
    break;
  }
  for (ti=1; ti<=thetaRes; ti++) {
    nti = ti < thetaRes ? ti+1 : 1;
    ELL_3V_SET(vII, vII0+ti, vII0+nti, vII0+0);
    limnObjectFaceAdd(obj, partIdx, lookIdx, 3, vII);
  }
  for (pi=0; pi<=phiRes-3; pi++) {
    for (ti=1; ti<=thetaRes; ti++) {
      nti = ti < thetaRes ? ti+1 : 1;
      ELL_4V_SET(vII, vII0+pi*thetaRes + ti, vII0+(pi+1)*thetaRes + ti,
                 vII0+(pi+1)*thetaRes + nti, vII0+pi*thetaRes + nti);
      limnObjectFaceAdd(obj, partIdx, lookIdx, 4, vII);
    }  
  }
  for (ti=1; ti<=thetaRes; ti++) {
    nti = ti < thetaRes ? ti+1 : 1;
    ELL_3V_SET(vII, vII0+pi*thetaRes + ti, pl, vII0+pi*thetaRes + nti);
    limnObjectFaceAdd(obj, partIdx, lookIdx, 3, vII);
  }

  return partIdx;
}

int
limnObjectPolarSuperquadAdd(limnObject *obj,
                            unsigned int lookIdx, unsigned int axis,
                            float A, float B, 
                            unsigned int thetaRes, unsigned int phiRes) {
  unsigned int partIdx, vII0, nti, ti, pi, vII[4], pl;
  double x, y, z, t, p;
  
  thetaRes = AIR_MAX(thetaRes, 3);
  phiRes = AIR_MAX(phiRes, 2);
  
  partIdx = limnObjectPartAdd(obj);
  /* HEY: we have to set this first so that 
     obj->setVertexRGBAFromLook can do the right thing */
  obj->part[partIdx]->lookIdx = lookIdx;
  switch(axis) {
  case 0:
    vII0 = limnObjectVertexAdd(obj, partIdx, 1, 0, 0);
    break;
  case 1:
    vII0 = limnObjectVertexAdd(obj, partIdx, 0, 1, 0);
    break;
  case 2: default:
    vII0 = limnObjectVertexAdd(obj, partIdx, 0, 0, 1);
    break;
  }
  for (pi=1; pi<=phiRes-1; pi++) {
    p = AIR_AFFINE(0, pi, phiRes, 0, AIR_PI);
    for (ti=0; ti<=thetaRes-1; ti++) {
      t = AIR_AFFINE(0, ti, thetaRes, 0, 2*AIR_PI);
      switch(axis) {
      case 0:
        x = airSgnPow(cos(p),B);
        y = -airSgnPow(sin(t),A) * airSgnPow(sin(p),B);
        z = airSgnPow(cos(t),A) * airSgnPow(sin(p),B);
        break;
      case 1:
        x = airSgnPow(sin(t),A) * airSgnPow(sin(p),B);
        y = airSgnPow(cos(p),B);
        z = airSgnPow(cos(t),A) * airSgnPow(sin(p),B);
        break;
      case 2: default:
        x = airSgnPow(cos(t),A) * airSgnPow(sin(p),B);
        y = airSgnPow(sin(t),A) * airSgnPow(sin(p),B);
        z = airSgnPow(cos(p),B);
        break;
      }
      limnObjectVertexAdd(obj, partIdx,
                          AIR_CAST(float, x),
                          AIR_CAST(float, y),
                          AIR_CAST(float, z));
    }
  }
  switch(axis) {
  case 0:
    pl = limnObjectVertexAdd(obj, partIdx, -1, 0, 0);
    break;
  case 1:
    pl = limnObjectVertexAdd(obj, partIdx, 0, -1, 0);
    break;
  case 2: default:
    pl = limnObjectVertexAdd(obj, partIdx, 0, 0, -1);
    break;
  }
  for (ti=1; ti<=thetaRes; ti++) {
    nti = ti < thetaRes ? ti+1 : 1;
    ELL_3V_SET(vII, vII0+ti, vII0+nti, vII0+0);
    limnObjectFaceAdd(obj, partIdx, lookIdx, 3, vII);
  }
  for (pi=0; pi<=phiRes-3; pi++) {
    for (ti=1; ti<=thetaRes; ti++) {
      nti = ti < thetaRes ? ti+1 : 1;
      ELL_4V_SET(vII, vII0+pi*thetaRes + ti, vII0+(pi+1)*thetaRes + ti,
                 vII0+(pi+1)*thetaRes + nti, vII0+pi*thetaRes + nti);
      limnObjectFaceAdd(obj, partIdx, lookIdx, 4, vII);
    }  
  }
  for (ti=1; ti<=thetaRes; ti++) {
    nti = ti < thetaRes ? ti+1 : 1;
    ELL_3V_SET(vII, vII0+pi*thetaRes + ti, pl, vII0+pi*thetaRes + nti);
    limnObjectFaceAdd(obj, partIdx, lookIdx, 3, vII);
  }

  return partIdx;
}
