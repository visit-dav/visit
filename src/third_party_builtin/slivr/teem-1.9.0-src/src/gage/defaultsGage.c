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

#include "gage.h"
#include "privateGage.h"

const char *
gageBiffKey = "gage";

int
gageDefVerbose = 0;

gage_t
gageDefGradMagMin = 0.00001f;

gage_t
gageDefGradMagCurvMin = 0.0f;

int
gageDefRenormalize = AIR_FALSE;

int
gageDefCheckIntegrals = AIR_TRUE;

int
gageDefK3Pack = AIR_TRUE;

gage_t
gageDefDefaultSpacing = 1.0f;

int
gageDefCurvNormalSide = 1;

gage_t
gageDefKernelIntegralNearZero = 0.0001f;

int
gageDefRequireAllSpacings = AIR_TRUE;

int
gageDefRequireEqualCenters = AIR_TRUE;

int
gageDefDefaultCenter = nrrdCenterNode;
