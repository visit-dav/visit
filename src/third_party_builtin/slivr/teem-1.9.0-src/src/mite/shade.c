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

#include "mite.h"
#include "privateMite.h"

miteShadeSpec *
miteShadeSpecNew(void) {
  miteShadeSpec *shpec;

  shpec = (miteShadeSpec *)calloc(1, sizeof(miteShadeSpec));
  if (shpec) {
    shpec->method = miteShadeMethodUnknown;
    shpec->vec0 = gageItemSpecNew();
    shpec->vec1 = gageItemSpecNew();
    shpec->scl0 = gageItemSpecNew();
    shpec->scl1 = gageItemSpecNew();
    if (!( shpec->vec0 && shpec->vec1 && 
           shpec->scl0 && shpec->scl1 )) {
      return NULL;
    }
  }
  return shpec;
}

miteShadeSpec *
miteShadeSpecNix(miteShadeSpec *shpec) {

  if (shpec) {
    shpec->vec0 = gageItemSpecNix(shpec->vec0);
    shpec->vec1 = gageItemSpecNix(shpec->vec1);
    shpec->scl0 = gageItemSpecNix(shpec->scl0);
    shpec->scl1 = gageItemSpecNix(shpec->scl1);
    airFree(shpec);
  }
  return NULL;
}

/*
******** miteShadeSpecParse
**
** set up a miteShadeSpec based on a string.  Valid forms are:
**
**   none
**   phong:<vector>
**   litten:<vector>,<vector>,<scalar>,<scalar>
**
** where <vector> and <scalar> are specifications of 3-vector and scalar
** parsable by miteVariableParse
*/
int
miteShadeSpecParse(miteShadeSpec *shpec, char *shadeStr) {
  char me[]="miteShadeSpecParse", err[BIFF_STRLEN],
    *buff, *qstr, *tok, *state;
  airArray *mop;
  int ansLength;

  mop = airMopNew();
  if (!( shpec && airStrlen(shadeStr) )) {
    sprintf(err, "%s: got NULL pointer and/or empty string", me);
    biffAdd(MITE, err); airMopError(mop); return 1;
  }
  buff = airToLower(airStrdup(shadeStr));
  if (!buff) {
    sprintf(err, "%s: couldn't strdup shading spec", me);
    biffAdd(MITE, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, buff, airFree, airMopAlways);
  shpec->method = miteShadeMethodUnknown;
  if (!strcmp("none", buff)) {
    shpec->method = miteShadeMethodNone;
  } else if (buff == strstr(buff, "phong:")) {
    shpec->method = miteShadeMethodPhong;
    qstr = buff + strlen("phong:");
    if (miteVariableParse(shpec->vec0, qstr)) {
      sprintf(err, "%s: couldn't parse \"%s\" as shading vector", me, qstr);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    ansLength = shpec->vec0->kind->table[shpec->vec0->item].answerLength;
    if (3 != ansLength) {
      sprintf(err, "%s: \"%s\" isn't a vector (answer length is %d, not 3)",
              me, qstr, ansLength);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    shpec->method = miteShadeMethodPhong;
  } else if (buff == strstr(buff, "litten:")) {
    qstr = buff + strlen("litten:");
    /* ---- first vector */
    tok = airStrtok(qstr, ",", &state);
    if (miteVariableParse(shpec->vec0, tok)) {
      sprintf(err, "%s: couldn't parse \"%s\" as first lit-tensor vector",
              me, tok);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    ansLength = shpec->vec0->kind->table[shpec->vec0->item].answerLength;
    if (3 != ansLength) {
      sprintf(err, "%s: \"%s\" isn't a vector (answer length is %d, not 3)",
              me, qstr, ansLength);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    /* ---- second vector */
    tok = airStrtok(qstr, ",", &state);
    if (miteVariableParse(shpec->vec1, tok)) {
      sprintf(err, "%s: couldn't parse \"%s\" as second lit-tensor vector",
              me, tok);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    ansLength = shpec->vec1->kind->table[shpec->vec1->item].answerLength;
    if (3 != ansLength) {
      sprintf(err, "%s: \"%s\" isn't a vector (answer length is %d, not 3)",
              me, qstr, ansLength);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    /* ---- first scalar */
    tok = airStrtok(qstr, ",", &state);
    if (miteVariableParse(shpec->scl0, tok)) {
      sprintf(err, "%s: couldn't parse \"%s\" as first lit-tensor scalar",
              me, tok);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    ansLength = shpec->scl0->kind->table[shpec->scl0->item].answerLength;
    if (1 != ansLength) {
      sprintf(err, "%s: \"%s\" isn't a scalar (answer length is %d, not 1)",
              me, qstr, ansLength);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    /* ---- second scalar */
    tok = airStrtok(qstr, ",", &state);
    if (miteVariableParse(shpec->scl1, tok)) {
      sprintf(err, "%s: couldn't parse \"%s\" as second lit-tensor scalar",
              me, tok);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    ansLength = shpec->scl1->kind->table[shpec->scl1->item].answerLength;
    if (1 != ansLength) {
      sprintf(err, "%s: \"%s\" isn't a scalar (answer length is %d, not 1)",
              me, qstr, ansLength);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    shpec->method = miteShadeMethodLitTen;
  } else {
    sprintf(err, "%s: shading specification \"%s\" not understood",
            me, shadeStr);
    biffAdd(MITE, err); airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

void
miteShadeSpecPrint(char *buff, const miteShadeSpec *shpec) {
  char me[]="miteShadeSpecPrint", var[4][AIR_STRLEN_MED];

  if (buff && shpec) {
    switch(shpec->method) {
    case miteShadeMethodNone:
      sprintf(buff, "none");
      break;
    case miteShadeMethodPhong:
      miteVariablePrint(var[0], shpec->vec0);
      sprintf(buff, "phong:%s", var[0]);
      break;
    case miteShadeMethodLitTen:
      miteVariablePrint(var[0], shpec->vec0);
      miteVariablePrint(var[1], shpec->vec1);
      miteVariablePrint(var[2], shpec->scl0);
      miteVariablePrint(var[3], shpec->scl1);
      sprintf(buff, "litten:%s,%s,%s,%s", var[0], var[1], var[2], var[3]);
      break;
    default:
      sprintf(buff, "%s: unknown shade method!", me);
      break;
    }
  }
  return;
}

void
miteShadeSpecQueryAdd(gageQuery queryScl, gageQuery queryVec, 
                      gageQuery queryTen, gageQuery queryMite,
                      miteShadeSpec *shpec) {
  if (shpec) {
    switch(shpec->method) {
    case miteShadeMethodNone:
      /* no queries to add */
      break;
    case miteShadeMethodPhong:
      miteQueryAdd(queryScl, queryVec, queryTen, queryMite, shpec->vec0);
      break;
    case miteShadeMethodLitTen:
      miteQueryAdd(queryScl, queryVec, queryTen, queryMite, shpec->vec0);
      miteQueryAdd(queryScl, queryVec, queryTen, queryMite, shpec->vec1);
      miteQueryAdd(queryScl, queryVec, queryTen, queryMite, shpec->scl0);
      miteQueryAdd(queryScl, queryVec, queryTen, queryMite, shpec->scl1);
      break;
    default:
      break;
    }
  }
  return;
}

