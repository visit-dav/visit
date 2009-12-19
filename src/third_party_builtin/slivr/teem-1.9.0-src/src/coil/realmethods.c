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

/* ------------------------------------------ */
const coilMethod
_coilMethodTesting = {
  "testing",
  coilMethodTypeTesting,
  0
};
const coilMethod*
coilMethodTesting = &_coilMethodTesting;

/* ------------------------------------------ */
const coilMethod
_coilMethodHomogeneous = {
  "homogeneous",
  coilMethodTypeHomogeneous,
  1
};
const coilMethod*
coilMethodHomogeneous = &_coilMethodHomogeneous;

/* ------------------------------------------ */
const coilMethod
_coilMethodPeronaMalik = {
  "perona-malik",
  coilMethodTypePeronaMalik,
  2
};
const coilMethod*
coilMethodPeronaMalik = &_coilMethodPeronaMalik;

/* ------------------------------------------ */
const coilMethod
_coilMethodModifiedCurvature = {
  "modified-curvature",
  coilMethodTypeModifiedCurvature,
  3
};
const coilMethod*
coilMethodModifiedCurvature = &_coilMethodModifiedCurvature;

/* ------------------------------------------ */
const coilMethod
_coilMethodSelf = {
  "self",
  coilMethodTypeSelf,
  1
};
const coilMethod*
coilMethodSelf = &_coilMethodSelf;

/* ------------------------------------------ */
const coilMethod
_coilMethodFinish = {
  "finish",
  coilMethodTypeFinish,
  4
};
const coilMethod*
coilMethodFinish = &_coilMethodFinish;

/* ------------------------------------------ */
const coilMethod*
coilMethodArray[COIL_METHOD_TYPE_MAX+1] = {
  NULL,
  &_coilMethodTesting,
  &_coilMethodHomogeneous,
  &_coilMethodPeronaMalik,
  &_coilMethodModifiedCurvature,
  NULL,
  &_coilMethodSelf,
  &_coilMethodFinish
};
