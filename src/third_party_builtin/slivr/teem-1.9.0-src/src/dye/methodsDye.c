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


#include "dye.h"

const char *
dyeBiffKey = "dye";

char
dyeSpaceToStr[][AIR_STRLEN_SMALL] = {
  "(unknown)",
  "HSV",
  "HSL",
  "RGB",
  "XYZ",
  "LAB",
  "LUV"
};

int
dyeStrToSpace(char *_str) {
  int spc;
  char *c, *str;

  spc = dyeSpaceUnknown;
  if ( (str = airStrdup(_str)) ) {
    c = str;
    while (*c) {
      *c = toupper(*c);
      c++;
    }
    for (spc=0; spc<dyeSpaceLast; spc++) {
      if (!strcmp(str, dyeSpaceToStr[spc])) {
        break;
      }
    }
    if (dyeSpaceLast == spc) {
      spc = dyeSpaceUnknown;
    }
    str = (char *)airFree(str);
  }
  return spc;
}

dyeColor *
dyeColorInit(dyeColor *col) {

  if (col) {
    ELL_3V_SET(col->val[0], AIR_NAN, AIR_NAN, AIR_NAN);
    ELL_3V_SET(col->val[1], AIR_NAN, AIR_NAN, AIR_NAN);
    col->xWhite = col->yWhite = AIR_NAN;
    col->spc[0] = dyeSpaceUnknown;
    col->spc[1] = dyeSpaceUnknown;
    col->ii = 0;
  }
  return col;
}

dyeColor *
dyeColorSet(dyeColor *col, int space, float v0, float v1, float v2) {
  
  if (col && DYE_VALID_SPACE(space)) {
    col->ii = AIR_CLAMP(0, col->ii, 1);

    /* We switch to the other one if the current one seems to be used,
       but we don't switch if new and current colorspaces are the same.
       If the other one is being used too, oh well.  */
    if (dyeSpaceUnknown != col->spc[col->ii] &&
        AIR_EXISTS(col->val[col->ii][0]) &&
        col->spc[col->ii] != space) {
      col->ii = 1 - col->ii;
    }

    ELL_3V_SET(col->val[col->ii], v0, v1, v2);
    col->spc[col->ii] = space;
  }
  return col;
}

int
dyeColorGet(float *v0P, float *v1P, float *v2P, dyeColor *col) {
  int spc;
  
  spc = dyeSpaceUnknown;
  if (v0P && v1P && v2P && col) {
    col->ii = AIR_CLAMP(0, col->ii, 1);
    spc = col->spc[col->ii];
    ELL_3V_GET(*v0P, *v1P, *v2P, col->val[col->ii]);
  }
  return spc;
}

int
dyeColorGetAs(float *v0P, float *v1P, float *v2P, 
              dyeColor *colIn, int space) {
  dyeColor _col, *col;
  
  col = &_col;
  dyeColorCopy(col, colIn);
  /* hope for no error */
  dyeConvert(col, space);
  return dyeColorGet(v0P, v1P, v2P, col);
}

dyeColor *
dyeColorNew() {
  dyeColor *col;

  col = (dyeColor *)calloc(1, sizeof(dyeColor));
  col = dyeColorInit(col);
  return col;
}

dyeColor *
dyeColorCopy(dyeColor *c1, dyeColor *c0) {
  
  if (c1 && c0) {
    memcpy(c1, c0, sizeof(dyeColor));
  }
  return c1;
}

dyeColor *
dyeColorNix(dyeColor *col) {

  if (col) {
    col = (dyeColor *)airFree(col);
  }
  return NULL;
}

int
dyeColorParse(dyeColor *col, char *_str) {
  char me[]="dyeColorParse", err[128], *str;
  char *colon, *valS;
  float v0, v1, v2;
  int spc;
  
  if (!(col && _str)) {
    sprintf(err, "%s: got NULL pointer", me); biffAdd(DYE, err); return 1;
  }
  if (!(str = airStrdup(_str))) {
    sprintf(err, "%s: couldn't strdup!", me);
    biffAdd(DYE, err); return 1;
  }
  if (!(colon = strchr(str, ':'))) {
    sprintf(err, "%s: given string \"%s\" didn't contain colon", me, str);
    biffAdd(DYE, err); return 1;
  }
  *colon = '\0';
  valS = colon+1;
  if (3 != sscanf(valS, "%g,%g,%g", &v0, &v1, &v2)) {
    sprintf(err, "%s: couldn't parse three floats from \"%s\"", me, valS);
    biffAdd(DYE, err); return 1;
  }
  spc = dyeStrToSpace(str);
  if (dyeSpaceUnknown == spc) {
    sprintf(err, "%s: couldn't parse colorspace from \"%s\"", me, str);
    biffAdd(DYE, err); return 1;
  }
  str = (char *)airFree(str);

  dyeColorSet(col, spc, v0, v1, v2);
  return 0;
}

char *
dyeColorSprintf(char *str, dyeColor *col) {
  
  if (str && col) {
    col->ii = AIR_CLAMP(0, col->ii, 1);
    sprintf(str, "%s:%g,%g,%g", dyeSpaceToStr[col->spc[col->ii]],
            col->val[col->ii][0], 
            col->val[col->ii][1], 
            col->val[col->ii][2]);
  }
  return str;
}
