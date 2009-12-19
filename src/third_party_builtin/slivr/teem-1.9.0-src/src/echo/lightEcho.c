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

/*
******* echoLightPosition()
**
** sets "pos" to xyz position for current sample of given light
*/
void
echoLightPosition(echoPos_t pos[3], echoObject *light,
                  echoThreadState *tstate) {
  char me[]="echoLightPos";
  echoPos_t x, y;
  echoRectangle *rectLight;
  
  x = tstate->jitt[0 + 2*echoJittableLight] + 0.5;
  y = tstate->jitt[1 + 2*echoJittableLight] + 0.5;
  switch(light->type) {
  case echoTypeRectangle:
    rectLight = RECTANGLE(light);
    ELL_3V_SCALE_ADD3(pos, 1, rectLight->origin,
                     x, rectLight->edge0,
                     y, rectLight->edge1);
    break;
  default:
    fprintf(stderr, "%s: currently only support echoTypeRectangle lights", me);
    break;
  }
  return;
}

/*
******* echoLightColor()
**
** sets "col" to RGB color for current sample of given light, which
** is at distance Ldist.  Knowing distance allows computation of the 
** inverse square fall-off of light intensity
*/
void
echoLightColor(echoCol_t rgb[3], echoPos_t Ldist,
               echoObject *light, echoRTParm *parm, echoThreadState *tstate) {
  echoCol_t rgba[4], falloff;
  echoPos_t x, y;
  
  x = tstate->jitt[0 + 2*echoJittableLight] + 0.5;
  y = tstate->jitt[1 + 2*echoJittableLight] + 0.5;
  if (light->ntext) {
    echoTextureLookup(rgba, light->ntext, x, y, parm);
    ELL_3V_COPY(rgb, rgba);
  } else {
    ELL_3V_COPY(rgb, light->rgba);
  }
  ELL_3V_SCALE(rgb, light->mat[echoMatterLightPower], rgb);
  if (light->mat[echoMatterLightUnit]) {
    falloff = AIR_CAST(echoCol_t, light->mat[echoMatterLightUnit]/Ldist);
    falloff *= falloff;
    ELL_3V_SCALE(rgb, falloff, rgb);
  }
  
  return;
}

void
echoEnvmapLookup(echoCol_t rgb[3], echoPos_t norm[3], Nrrd *envmap) {
  int qn;
  float *data;

#if ECHO_POS_FLOAT
  qn = limnVtoQN_f[limnQN16octa](norm);
#else
  qn = limnVtoQN_d[limnQN16octa](norm);
#endif
  data = (float*)(envmap->data) + 3*qn;
  ELL_3V_COPY(rgb, data);
}

