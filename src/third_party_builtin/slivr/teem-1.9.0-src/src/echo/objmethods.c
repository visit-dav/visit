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

#define NEW_TMPL(TYPE, BODY)                         \
echo##TYPE *                                         \
_echo##TYPE##_new(void) {                            \
  echo##TYPE *obj;                                   \
                                                     \
  obj = (echo##TYPE *)calloc(1, sizeof(echo##TYPE)); \
  obj->type = echoType##TYPE;                        \
  do { BODY } while (0);                             \
  return obj;                                        \
}

#define NIX_TMPL(TYPE, BODY)                         \
echo##TYPE *                                         \
_echo##TYPE##_nix(echo##TYPE *obj) {                 \
  int dummy=0;                                       \
                                                     \
  if (obj) {                                         \
    do { BODY dummy=dummy;} while (0);               \
    airFree(obj);                                    \
  }                                                  \
  return NULL;                                       \
}

void
_echoMatterInit(echoObject *obj) {

  obj->matter = echoMatterUnknown;
  ELL_4V_SET(obj->rgba, 0, 0, 0, 0);
  memset(obj->mat, 0,ECHO_MATTER_PARM_NUM*sizeof(echoCol_t));
  obj->ntext = NULL;
}

NEW_TMPL(Sphere,
         _echoMatterInit(OBJECT(obj));
         ELL_3V_SET(obj->pos, 0, 0, 0);
         obj->rad = 1.0;
         )

NEW_TMPL(Cylinder,
         _echoMatterInit(OBJECT(obj));
         obj->axis = 2;
         )

NEW_TMPL(Superquad,
         _echoMatterInit(OBJECT(obj));
         obj->axis = 2;
         obj->A = obj->B = 1;
         )

NEW_TMPL(Cube,
         _echoMatterInit(OBJECT(obj));
         )

NEW_TMPL(Triangle,
         _echoMatterInit(OBJECT(obj));
         ELL_3V_SET(obj->vert[0], 0, 0, 0);
         ELL_3V_SET(obj->vert[1], 0, 0, 0);
         ELL_3V_SET(obj->vert[2], 0, 0, 0);
         )

NEW_TMPL(Rectangle,
         _echoMatterInit(OBJECT(obj));
         ELL_3V_SET(obj->origin, 0, 0, 0);
         ELL_3V_SET(obj->edge0, 0, 0, 0);
         ELL_3V_SET(obj->edge1, 0, 0, 0);
         )

NEW_TMPL(TriMesh,
         _echoMatterInit(OBJECT(obj));
         ELL_3V_SET(obj->meanvert, 0, 0, 0);
         ELL_3V_SET(obj->min, ECHO_POS_MAX, ECHO_POS_MAX, ECHO_POS_MAX);
         ELL_3V_SET(obj->max, ECHO_POS_MIN, ECHO_POS_MIN, ECHO_POS_MIN);
         obj->numV = obj->numF = 0;
         obj->pos = NULL;
         obj->vert = NULL;
         )
NIX_TMPL(TriMesh,
         obj->pos = (echoPos_t *)airFree(obj->pos);
         obj->vert = (int *)airFree(obj->vert);
         )

NEW_TMPL(Isosurface,
         _echoMatterInit(OBJECT(obj));
         obj->volume = NULL;
         obj->value = 0.0;
         /* ??? */
         )

NEW_TMPL(AABBox,
         obj->obj = NULL;
         ELL_3V_SET(obj->min, ECHO_POS_MAX, ECHO_POS_MAX, ECHO_POS_MAX);
         ELL_3V_SET(obj->max, ECHO_POS_MIN, ECHO_POS_MIN, ECHO_POS_MIN);
         )

NEW_TMPL(Split,
         obj->axis = -1;
         ELL_3V_SET(obj->min0, ECHO_POS_MAX, ECHO_POS_MAX, ECHO_POS_MAX);
         ELL_3V_SET(obj->max0, ECHO_POS_MIN, ECHO_POS_MIN, ECHO_POS_MIN);
         ELL_3V_SET(obj->min1, ECHO_POS_MAX, ECHO_POS_MAX, ECHO_POS_MAX);
         ELL_3V_SET(obj->max1, ECHO_POS_MIN, ECHO_POS_MIN, ECHO_POS_MIN);
         obj->obj0 = obj->obj1 = NULL;
         )

NEW_TMPL(List,
         obj->obj = NULL;
         obj->objArr = airArrayNew((void**)&(obj->obj), NULL,
                                   sizeof(echoObject *),
                                   ECHO_LIST_OBJECT_INCR);
         )
NIX_TMPL(List,
         airArrayNuke(obj->objArr);
         )

NEW_TMPL(Instance,
         ELL_4M_IDENTITY_SET(obj->M);
         ELL_4M_IDENTITY_SET(obj->Mi);
         obj->obj = NULL;
         )

echoObject *(*
_echoObjectNew[ECHO_TYPE_NUM])(void) = {
  (echoObject *(*)(void))_echoSphere_new,
  (echoObject *(*)(void))_echoCylinder_new,
  (echoObject *(*)(void))_echoSuperquad_new,
  (echoObject *(*)(void))_echoCube_new,
  (echoObject *(*)(void))_echoTriangle_new,
  (echoObject *(*)(void))_echoRectangle_new,
  (echoObject *(*)(void))_echoTriMesh_new,
  (echoObject *(*)(void))_echoIsosurface_new,
  (echoObject *(*)(void))_echoAABBox_new,
  (echoObject *(*)(void))_echoSplit_new,
  (echoObject *(*)(void))_echoList_new,
  (echoObject *(*)(void))_echoInstance_new
};

echoObject *
echoObjectNew(echoScene *scene, signed char type) {
  echoObject *ret=NULL;
  int idx;

  if (scene && AIR_IN_OP(echoTypeUnknown, type, echoTypeLast)) {
    ret = _echoObjectNew[type]();
    idx = airArrayLenIncr(scene->catArr, 1);
    scene->cat[idx] = ret;
  }
  return ret;
}

int
echoObjectAdd(echoScene *scene, echoObject *obj) {
  int idx;

  if (scene && obj) {
    idx = airArrayLenIncr(scene->rendArr, 1);
    scene->rend[idx] = obj;
  }
  return 0;
}

echoObject *(*
_echoObjectNix[ECHO_TYPE_NUM])(echoObject *) = {
  (echoObject *(*)(echoObject *))airFree,          /* echoTypeSphere */
  (echoObject *(*)(echoObject *))airFree,          /* echoTypeCylinder */
  (echoObject *(*)(echoObject *))airFree,          /* echoTypeSuperquad */
  (echoObject *(*)(echoObject *))airFree,          /* echoTypeCube */
  (echoObject *(*)(echoObject *))airFree,          /* echoTypeTriangle */
  (echoObject *(*)(echoObject *))airFree,          /* echoTypeRectangle */
  (echoObject *(*)(echoObject *))_echoTriMesh_nix, /* echoTypeTriMesh */
  (echoObject *(*)(echoObject *))airFree,          /* echoTypeIsosurface */
  (echoObject *(*)(echoObject *))airFree,          /* echoTypeAABBox */
  (echoObject *(*)(echoObject *))airFree,          /* echoTypeSplit */
  (echoObject *(*)(echoObject *))_echoList_nix,    /* echoTypeList */
  (echoObject *(*)(echoObject *))airFree           /* echoTypeInstance */
};

echoObject *
echoObjectNix(echoObject *obj) {

  if (obj) {
    _echoObjectNix[obj->type](obj);
  }
  return NULL;
}
