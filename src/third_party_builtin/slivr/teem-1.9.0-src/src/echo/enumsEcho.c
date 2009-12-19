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

/* ------------------------------- jitter --------------------------- */

char
_echoJitterStr[ECHO_JITTER_NUM+1][AIR_STRLEN_SMALL] = {
  "(unknown_jitter)",
  "none",
  "grid",
  "jitter",
  "random"
};

int
_echoJitterVal[ECHO_JITTER_NUM+1] = {
  echoJitterUnknown,
  echoJitterNone,
  echoJitterGrid,
  echoJitterJitter,
  echoJitterRandom
};

char
_echoJitterDesc[ECHO_JITTER_NUM+1][AIR_STRLEN_MED] = {
  "unknown jitter",
  "nothing- samples are ALWAYS at center of region",
  "no jittering- samples are at regular grid vertices",
  "normal jittering- samples are randomly located within grid cells",
  "samples are randomly located within region"
};

char
_echoJitterStrEqv[][AIR_STRLEN_SMALL] = {
  "none",
  "grid", "regular",
  "jitter",
  "random",
  ""
};

int
_echoJitterValEqv[] = {
  echoJitterNone,
  echoJitterGrid, echoJitterGrid, 
  echoJitterJitter,
  echoJitterRandom
};

airEnum
_echoJitter = {
  "jitter",
  ECHO_JITTER_NUM,
  _echoJitterStr,  _echoJitterVal,
  _echoJitterDesc,
  _echoJitterStrEqv, _echoJitterValEqv,
  AIR_FALSE
};
airEnum *
echoJitter = &_echoJitter;

/* ------------------------------- object type --------------------------- */

char
_echoTypeStr[ECHO_TYPE_NUM+1][AIR_STRLEN_SMALL] = {
  "(unknown_object)",
  "sphere",
  "cylinder",
  "superquad",
  "cube",
  "triangle",
  "rectangle",
  "mesh",
  "isosurface",
  "AABoundingBox",
  "split",
  "list",
  "instance"
};

int
_echoTypeVal[ECHO_TYPE_NUM+1] = {
  echoTypeUnknown,
  echoTypeSphere,
  echoTypeCylinder,
  echoTypeSuperquad,
  echoTypeCube,
  echoTypeTriangle,
  echoTypeRectangle,
  echoTypeTriMesh,
  echoTypeIsosurface,
  echoTypeAABBox,
  echoTypeSplit,
  echoTypeList,
  echoTypeInstance
};

char
_echoTypeDesc[ECHO_TYPE_NUM+1][AIR_STRLEN_MED] = {
  "unknown_object",
  "sphere",
  "axis-aligned cylinder",
  "superquadric (actually, superellipsoid)",
  "unit cube, centered at the origin",
  "triangle",
  "rectangle",
  "mesh of triangles",
  "isosurface of scalar volume",
  "axis-aligned bounding box",
  "split",
  "list",
  "instance"
};

char
_echoTypeStrEqv[][AIR_STRLEN_SMALL] = {
  "sphere",
  "cylinder", "cylind", "rod",
  "superquad", "squad",
  "cube", "box",
  "triangle", "tri",
  "rectangle", "rect",
  "mesh", "tri-mesh", "trimesh",
  "isosurface",
  "aabbox", "AABoundingBox",
  "split",
  "list",
  "instance",
  ""
};

int
_echoTypeValEqv[] = {
  echoTypeSphere,
  echoTypeCylinder, echoTypeCylinder, echoTypeCylinder,
  echoTypeSuperquad, echoTypeSuperquad,
  echoTypeCube, echoTypeCube,
  echoTypeTriangle, echoTypeTriangle,
  echoTypeRectangle, echoTypeRectangle,
  echoTypeTriMesh, echoTypeTriMesh, echoTypeTriMesh,
  echoTypeIsosurface,
  echoTypeAABBox, echoTypeAABBox,
  echoTypeSplit,
  echoTypeList,
  echoTypeInstance
};

airEnum
_echoType = {
  "object type",
  ECHO_TYPE_NUM,
  _echoTypeStr,  NULL,
  _echoTypeDesc,
  _echoTypeStrEqv, _echoTypeValEqv,
  AIR_FALSE
};
airEnum *
echoType = &_echoType;

/* ------------------------------ material types --------------------------- */

char
_echoMatterStr[ECHO_MATTER_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_matter)",
  "phong",
  "glass",
  "metal",
  "light"
};

char
_echoMatterDesc[ECHO_MATTER_MAX+1][AIR_STRLEN_MED] = {
  "unknown material",
  "phong shaded surface",
  "glass",
  "metal",
  "light emitter"
};

airEnum
_echoMatter = {
  "matter",
  ECHO_MATTER_MAX,
  _echoMatterStr,  NULL,
  _echoMatterDesc,
  NULL, NULL,
  AIR_FALSE
};
airEnum *
echoMatter = &_echoMatter;
