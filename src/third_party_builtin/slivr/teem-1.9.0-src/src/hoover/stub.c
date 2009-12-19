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

#include "hoover.h"

int
hooverStubRenderBegin(void **rendInfoP, void *userInfo) {

  AIR_UNUSED(userInfo);
  *rendInfoP = NULL;
  return 0;
}

int
hooverStubThreadBegin(void **threadInfoP, void *rendInfo, void *userInfo,
                      int whichThread) {

  AIR_UNUSED(rendInfo);
  AIR_UNUSED(userInfo);
  AIR_UNUSED(whichThread);
  *threadInfoP = NULL;
  return 0;
}

int 
hooverStubRayBegin(void *threadInfo, void *renderInfo, void *userInfo,
                   int uIndex,
                   int vIndex, 
                   double rayLen,
                   double rayStartWorld[3],
                   double rayStartIndex[3],
                   double rayDirWorld[3],
                   double rayDirIndex[3]) {

  AIR_UNUSED(threadInfo);
  AIR_UNUSED(renderInfo);
  AIR_UNUSED(userInfo);
  AIR_UNUSED(uIndex);
  AIR_UNUSED(vIndex);
  AIR_UNUSED(rayLen);
  AIR_UNUSED(rayStartWorld);
  AIR_UNUSED(rayStartIndex);
  AIR_UNUSED(rayDirWorld);
  AIR_UNUSED(rayDirIndex);
  /*
  char me[]="hooverStubRayBegin";
  fprintf(stderr, "%s: ray(%d,%d), len=%g\n"
          "  start=(%g,%g,%g)\n"
          "  dir=(%g,%g,%g)\n",
          me, uIndex, vIndex, rayLen,
          rayStartWorld[0], rayStartWorld[1], rayStartWorld[2],
          rayDirWorld[0], rayDirWorld[1], rayDirWorld[2]);
  */

  return 0;
}

double
hooverStubSample(void *threadInfo, void *renderInfo, void *userInfo,
                 int num, double rayT,
                 int inside,
                 double samplePosWorld[3],
                 double samplePosIndex[3]) {

  AIR_UNUSED(threadInfo);
  AIR_UNUSED(renderInfo);
  AIR_UNUSED(userInfo);
  AIR_UNUSED(num);
  AIR_UNUSED(rayT);
  AIR_UNUSED(inside);
  AIR_UNUSED(samplePosWorld);
  AIR_UNUSED(samplePosIndex);

  /*
  char me[]="hooverStubSample";
  fprintf(stderr, "%s:      sample(%g,%g,%g)\n", me,
          samplePosWorld[0], samplePosWorld[1], samplePosWorld[2]);
  */

  /* we want the stub renderer to actually finish */
  return 1.0;  
}

int
hooverStubRayEnd(void *threadInfo, void *rendInfo, void *userInfo) {

  AIR_UNUSED(threadInfo);
  AIR_UNUSED(rendInfo);
  AIR_UNUSED(userInfo);

  return 0;
}

int
hooverStubThreadEnd(void *threadInfo, void *rendInfo, void *userInfo) {

  AIR_UNUSED(threadInfo);
  AIR_UNUSED(rendInfo);
  AIR_UNUSED(userInfo);

  return 0;
}

int
hooverStubRenderEnd(void *rendInfo, void *userInfo) {

  AIR_UNUSED(rendInfo);
  AIR_UNUSED(userInfo);

  return 0;
}

