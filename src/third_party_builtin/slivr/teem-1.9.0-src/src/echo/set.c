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
echoSphereSet(echoObject *sphere,
              echoPos_t x, echoPos_t y, echoPos_t z, echoPos_t rad) {

  if (sphere && echoTypeSphere == sphere->type) {
    ELL_3V_SET(SPHERE(sphere)->pos, x, y, z);
    SPHERE(sphere)->rad = rad;
  }
  return;
}

void
echoCylinderSet(echoObject *cylind,
                int axis) {
  
  if (cylind && echoTypeCylinder == cylind->type) {
    CYLINDER(cylind)->axis = axis;
  }
  return;
}

void
echoSuperquadSet(echoObject *squad,
                 int axis, echoPos_t A, echoPos_t B) {

  if (squad && echoTypeSuperquad == squad->type) {
    SUPERQUAD(squad)->axis = axis;
    SUPERQUAD(squad)->A = A;
    SUPERQUAD(squad)->B = B;
  }
  return;
}

void
echoRectangleSet(echoObject *rect,
                 echoPos_t ogx, echoPos_t ogy, echoPos_t ogz,
                 echoPos_t e0x, echoPos_t e0y, echoPos_t e0z,
                 echoPos_t e1x, echoPos_t e1y, echoPos_t e1z) {

  if (rect && echoTypeRectangle == rect->type) {
    ELL_3V_SET(RECTANGLE(rect)->origin, ogx, ogy, ogz);
    ELL_3V_SET(RECTANGLE(rect)->edge0, e0x, e0y, e0z);
    ELL_3V_SET(RECTANGLE(rect)->edge1, e1x, e1y, e1z);
  }
  return;
}
                       
void
echoTriangleSet(echoObject *tri,
                echoPos_t x0, echoPos_t y0, echoPos_t z0, 
                echoPos_t x1, echoPos_t y1, echoPos_t z1, 
                echoPos_t x2, echoPos_t y2, echoPos_t z2) {

  if (tri && echoTypeTriangle == tri->type) {
    ELL_3V_SET(TRIANGLE(tri)->vert[0], x0, y0, z0);
    ELL_3V_SET(TRIANGLE(tri)->vert[1], x1, y1, z1);
    ELL_3V_SET(TRIANGLE(tri)->vert[2], x2, y2, z2);
  }
  return;
}

/*
******** echoTriMeshSet()
**
** This has to be called any time that the locations of the points are
** changing, even if the connectivity is not changed, because of how
** the bounding box and mean vert position is calculated here.
**
** NB: the TriMesh will directly use the given pos[] and vert[] arrays,
** so don't go freeing them after they've been passed here.
*/
void
echoTriMeshSet(echoObject *trim,
               int numV, echoPos_t *pos,
               int numF, int *vert) {
  int i;

  if (trim && echoTypeTriMesh == trim->type) {
    TRIMESH(trim)->numV = numV;
    TRIMESH(trim)->numF = numF;
    TRIMESH(trim)->pos = pos;
    TRIMESH(trim)->vert = vert;
    ELL_3V_SET(TRIMESH(trim)->min, ECHO_POS_MAX, ECHO_POS_MAX, ECHO_POS_MAX);
    ELL_3V_SET(TRIMESH(trim)->max, ECHO_POS_MIN, ECHO_POS_MIN, ECHO_POS_MIN);
    ELL_3V_SET(TRIMESH(trim)->meanvert, 0.0, 0.0, 0.0);
    for (i=0; i<numV; i++) {
      ELL_3V_MIN(TRIMESH(trim)->min, TRIMESH(trim)->min, pos + 3*i);
      ELL_3V_MAX(TRIMESH(trim)->max, TRIMESH(trim)->max, pos + 3*i);
      ELL_3V_INCR(TRIMESH(trim)->meanvert, pos + 3*i);
    }
    ELL_3V_SCALE(TRIMESH(trim)->meanvert, 1.0/numV, TRIMESH(trim)->meanvert);
  }
  return;
}

void
echoInstanceSet(echoObject *inst,
                echoPos_t *M, echoObject *obj) {
  
  if (inst && echoTypeInstance == inst->type) {
    ell_4m_INV(INSTANCE(inst)->Mi, M);
    ELL_4M_COPY(INSTANCE(inst)->M, M);
    INSTANCE(inst)->obj = obj;
  }
}
