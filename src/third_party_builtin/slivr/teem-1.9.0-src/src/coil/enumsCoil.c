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

#include "coil.h"

char
_coilMethodTypeStr[COIL_METHOD_TYPE_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_method)",
  "testing",
  "homogeneous",
  "perona-malik",
  "modified curvature",
  "curvature flow",
  "self",
  "finish"
};

char
_coilMethodTypeDesc[COIL_METHOD_TYPE_MAX+1][AIR_STRLEN_MED] = {
  "unknown_method",
  "nothing, actually, just here for testing",
  "homogenous isotropic diffusion (Gaussian blurring)",
  "Perona-Malik",
  "modified curvature diffusion",
  "curvature flow",
  "self-diffusion of diffusion tensors",
  "finish a phd already"
};

char
_coilMethodTypeStrEqv[][AIR_STRLEN_SMALL] = {
  "test", "testing",
  "iso", "homog", "homogeneous",
  "pm", "perona-malik",
  "mcde",
  "flow",
  "self",
  "finish",
  ""
};

int
_coilMethodTypeValEqv[] = {
  coilMethodTypeTesting, coilMethodTypeTesting,
  coilMethodTypeHomogeneous, coilMethodTypeHomogeneous, coilMethodTypeHomogeneous,
  coilMethodTypePeronaMalik, coilMethodTypePeronaMalik,
  coilMethodTypeModifiedCurvature,
  coilMethodTypeCurvatureFlow,
  coilMethodTypeSelf,
  coilMethodTypeFinish,
};

airEnum
_coilMethodType = {
  "method",
  COIL_METHOD_TYPE_MAX,
  _coilMethodTypeStr,  NULL,
  _coilMethodTypeDesc,
  _coilMethodTypeStrEqv, _coilMethodTypeValEqv,
  AIR_FALSE
};
airEnum *
coilMethodType = &_coilMethodType;

/* -------------------------------------------------- */

char
_coilKindTypeStr[COIL_KIND_TYPE_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_kind)",
  "scalar",
  "3color",
  "7tensor"
};

char
_coilKindTypeDesc[COIL_KIND_TYPE_MAX+1][AIR_STRLEN_MED] = {
  "unknown_kind",
  "plain old scalar quantities",
  "3-component color",
  "ten-style 7-valued tensor"
};

char
_coilKindTypeStrEqv[][AIR_STRLEN_SMALL] = {
  "scalar",
  "3color",
  "7tensor", "tensor",
  ""
};

int
_coilKindTypeValEqv[] = {
  coilKindTypeScalar,
  coilKindType3Color,
  coilKindType7Tensor, coilKindType7Tensor
};

airEnum
_coilKindType = {
  "kind",
  COIL_KIND_TYPE_MAX,
  _coilKindTypeStr,  NULL,
  _coilKindTypeDesc,
  _coilKindTypeStrEqv, _coilKindTypeValEqv,
  AIR_FALSE
};
airEnum *
coilKindType = &_coilKindType;
