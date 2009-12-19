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

int
echoObjectHasMatter[ECHO_TYPE_NUM] = {
  1, /* echoTypeSphere */
  1, /* echoTypeCylinder */
  1, /* echoTypeSuperquad */
  1, /* echoTypeCube */
  1, /* echoTypeTriangle */
  1, /* echoTypeRectangle */
  1, /* echoTypeTriMesh */
  1, /* echoTypeIsosurface */
  0, /* echoTypeAABBox */
  0, /* echoTypeSplit */
  0, /* echoTypeList */
  0, /* echoTypeInstance */
};

void
echoColorSet(echoObject *obj,
             echoCol_t R, echoCol_t G, echoCol_t B, echoCol_t A) {

  if (obj && echoObjectHasMatter[obj->type]) {
    ELL_4V_SET(obj->rgba, R, G, B, A);
  }
}

void
echoMatterPhongSet(echoScene *scene, echoObject *obj,
                   echoCol_t ka, echoCol_t kd, echoCol_t ks, echoCol_t sp) {
  
  if (scene && obj && echoObjectHasMatter[obj->type]) {
    obj->matter = echoMatterPhong;
    obj->mat[echoMatterPhongKa] = ka;
    obj->mat[echoMatterPhongKd] = kd;
    obj->mat[echoMatterPhongKs] = ks;
    obj->mat[echoMatterPhongSp] = sp;
  }
}
                   
void
echoMatterGlassSet(echoScene *scene, echoObject *obj,
                   echoCol_t index, echoCol_t ka,
                   echoCol_t kd, echoCol_t fuzzy) {

  if (scene && obj && echoObjectHasMatter[obj->type]) {
    obj->matter = echoMatterGlass;
    obj->mat[echoMatterGlassIndex] = index;
    obj->mat[echoMatterGlassKa] = ka;
    obj->mat[echoMatterGlassKd] = kd;
    obj->mat[echoMatterGlassFuzzy] = fuzzy;
  }
}

void
echoMatterMetalSet(echoScene *scene, echoObject *obj,
                   echoCol_t R0, echoCol_t ka, 
                   echoCol_t kd, echoCol_t fuzzy) {

  if (scene && obj && echoObjectHasMatter[obj->type]) {
    obj->matter = echoMatterMetal;
    obj->mat[echoMatterMetalR0] = R0;
    obj->mat[echoMatterMetalKa] = ka;
    obj->mat[echoMatterMetalKd] = kd;
    obj->mat[echoMatterMetalFuzzy] = fuzzy;
  }
}

void
echoMatterLightSet(echoScene *scene, echoObject *obj,
                   echoCol_t power, echoCol_t unit) {
  
  if (scene && obj && echoObjectHasMatter[obj->type]) {
    obj->matter = echoMatterLight;
    obj->mat[echoMatterLightPower] = power;
    obj->mat[echoMatterLightUnit] = unit;
    /* HEY: god forbid we should change the material of the light after this */
    _echoSceneLightAdd(scene, obj);
  }
}

void
echoMatterTextureSet(echoScene *scene, echoObject *obj, Nrrd *ntext) {
  
  if (scene && obj && ntext && echoObjectHasMatter[obj->type] &&
      3 == ntext->dim && 
      nrrdTypeUChar == ntext->type &&
      4 == ntext->axis[0].size) {
    obj->ntext = ntext;
    _echoSceneNrrdAdd(scene, ntext);
  }
}
