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

#include "push.h"
#include "privatePush.h"

/* returns NULL if position is outside simulation domain */
pushBin *
_pushBinLocate(pushContext *pctx, push_t *pos) {
  char me[]="_pushBinLocate", err[BIFF_STRLEN];
  push_t min, max;
  int be, xi, yi, zi;
  pushBin *ret;

  if (pctx->singleBin) {
    ret = pctx->bin + 0;
  } else {
    min = AIR_CAST(push_t, -1.0 - pctx->margin);
    max = AIR_CAST(push_t, 1.0 + pctx->margin);
    be = pctx->binsEdge;
    if (AIR_IN_CL(min, pos[0], max)
        && AIR_IN_CL(min, pos[1], max)
        && AIR_IN_CL(min, pos[2], max)) {
      xi = airIndex(min, pos[0], max, be);
      yi = airIndex(min, pos[1], max, be);
      if (2 == pctx->dimIn) {
        zi = 0;
      } else {
        zi = airIndex(min, pos[2], max, be);
      }
      ret = pctx->bin + xi + be*(yi + be*zi);
    } else {
      sprintf(err, "%s: one of (%g,%g,%g) coords out of bounds [%g,%g]", me,
              pos[0], pos[1], pos[2], min, max);
      ret = NULL;
    }
  }

  return ret;
}

/*
** HEY: doesn't check to make sure thing isn't already in bin
*/
void
_pushBinThingAdd(pushContext *pctx, pushBin *bin, pushThing *thing) {
  int thgI;

  AIR_UNUSED(pctx);
  thgI = airArrayLenIncr(bin->thingArr, 1);
  bin->thing[thgI] = thing;

  return;
}

void
_pushBinPointAdd(pushContext *pctx, pushBin *bin, pushPoint *point) {
  int pntI;

  AIR_UNUSED(pctx);
  pntI = airArrayLenIncr(bin->pointArr, 1);
  bin->point[pntI] = point;

  return;
}

/*
** remove the pointer to the thing from the bin, don't kill the thing
** because we don't know here if this is part of a killing, or a rebinning
*/
void
_pushBinThingRemove(pushContext *pctx, pushBin *bin, int loseIdx) {

  AIR_UNUSED(pctx);
  bin->thing[loseIdx] = bin->thing[bin->numThing-1];
  airArrayLenIncr(bin->thingArr, -1);
  
  return;
}

void
_pushBinPointRemove(pushContext *pctx, pushBin *bin, int loseIdx) {

  AIR_UNUSED(pctx);
  bin->point[loseIdx] = bin->point[bin->numPoint-1];
  airArrayLenIncr(bin->pointArr, -1);
  
  return;
}

int
_pushBinPointNullify(pushContext *pctx, pushBin *oldBin, pushPoint *point) {
  char me[]="_pushBinPointNullify", err[BIFF_STRLEN];
  pushBin *bin;
  unsigned int pointIdx;

  if (!( bin = oldBin ? oldBin : _pushBinLocate(pctx, point->pos) )) {
    sprintf(err, "%s: NULL bin for point %p (%g,%g,%g)", me,
            AIR_CAST(void*, point),
            point->pos[0], point->pos[1], point->pos[2]);
    biffAdd(PUSH, err); return 1;
  }
  for (pointIdx=0; pointIdx<bin->numPoint; pointIdx++) {
    if (point == bin->point[pointIdx]) {
      break;
    }
  }
  if (pointIdx == bin->numPoint) {
    sprintf(err, "%s: point %p (%g,%g,%g) wasn't in its bin", me,
            AIR_CAST(void*, point),
            point->pos[0], point->pos[1], point->pos[2]);
    biffAdd(PUSH, err); return 1;
  }
  bin->point[pointIdx] = NULL;
  return 0;
}

void
_pushBinNeighborSet(pushBin *bin, pushBin **nei, unsigned int num) {
  unsigned int neiI;

  bin->neighbor = (pushBin **)airFree(bin->neighbor);
  bin->neighbor = (pushBin **)calloc(1+num, sizeof(pushBin *));
  for (neiI=0; neiI<num; neiI++) {
    bin->neighbor[neiI] = nei[neiI];
  }
  bin->neighbor[neiI] = NULL;
  return;
}

void
pushBinAllNeighborSet(pushContext *pctx) {
  pushBin *nei[3*3*3];
  unsigned int numNei, xi, yi, zi, be, xx, yy, zz, xmax, ymax, zmax;
  int xmin, ymin, zmin;

  if (pctx->singleBin) {
    numNei = 0;
    nei[numNei++] = pctx->bin + 0;
    _pushBinNeighborSet(pctx->bin + 0, nei, numNei);
  } else {
    be = pctx->binsEdge;
    for (zi=0; zi<(2 == pctx->dimIn ? 1 : be); zi++) {
      if (2 == pctx->dimIn) {
        zmin = zmax = 0;
      } else {
        zmin = AIR_MAX(0, (int)zi-1);
        zmax = AIR_MIN(zi+1, be-1);
      }
      for (yi=0; yi<be; yi++) {
        ymin = AIR_MAX(0, (int)yi-1);
        ymax = AIR_MIN(yi+1, be-1);
        for (xi=0; xi<be; xi++) {
          xmin = AIR_MAX(0, (int)xi-1);
          xmax = AIR_MIN(xi+1, be-1);
          numNei = 0;
          for (zz=zmin; zz<=zmax; zz++) {
            for (yy=ymin; yy<=ymax; yy++) {
              for (xx=xmin; xx<=xmax; xx++) {
                nei[numNei++] = pctx->bin + xx + be*(yy + be*zz);
              }
            }
          }
          _pushBinNeighborSet(pctx->bin + xi + be*(yi + be*zi), nei, numNei);
        }
      }
    }
  }
  return;
}

int
pushBinThingAdd(pushContext *pctx, pushThing *thing) {
  char me[]="pushBinThingAdd", err[BIFF_STRLEN];
  pushBin *bin;
  
  if (!( bin = _pushBinLocate(pctx, thing->point.pos) )) {
    sprintf(err, "%s: can't locate point of thing %p",
            me, AIR_CAST(void*, thing));
    biffAdd(PUSH, err); return 1;
  }
  _pushBinThingAdd(pctx, bin, thing);
  return 0;
}

int
pushBinPointAdd(pushContext *pctx, pushPoint *point) {
  char me[]="pushBinPointAdd", err[BIFF_STRLEN];
  pushBin *bin;
  
  if (!( bin = _pushBinLocate(pctx, point->pos) )) {
    sprintf(err, "%s: can't locate point %p", me, AIR_CAST(void*, point));
    biffAdd(PUSH, err); return 1;
  }
  _pushBinPointAdd(pctx, bin, point);
  return 0;
}

/*
** This function is only called by the master thread, this 
** does *not* have to be thread-safe in any way
*/
int
pushRebin(pushContext *pctx) {
  char me[]="pushRebin";
  unsigned int oldBinIdx, thingIdx, pointIdx;
  pushBin *oldBin, *newBin;
  pushThing *thing;
  pushPoint *point;

  /* even if there is a single bin, we have to toss out-of-bounds
     things, and prune nullified points */
  for (oldBinIdx=0; oldBinIdx<pctx->numBin; oldBinIdx++) {
    oldBin = pctx->bin + oldBinIdx;

    /* quietly clear out pointers to points that got nullified,
       or that went out of bounds (not an error here) */
    for (pointIdx=0; pointIdx<oldBin->numPoint; /* nope! */) {
      point = oldBin->point[pointIdx];
      if (!point) {
        /* this point got nullified */
        _pushBinPointRemove(pctx, oldBin, pointIdx);
      } else {
        if (!( newBin = _pushBinLocate(pctx, point->pos) )) {
          /* this point is out of bounds */
          _pushBinPointRemove(pctx, oldBin, pointIdx);
        } else {
          if (oldBin != newBin) {
            _pushBinPointRemove(pctx, oldBin, pointIdx);
            _pushBinPointAdd(pctx, newBin, point);
          } else {
            /* its in the right bin, move on */
            pointIdx++;
          }
        }
      }
    } /* for pointIdx */
    
    for (thingIdx=0; thingIdx<oldBin->numThing; /* nope! */) {
      thing = oldBin->thing[thingIdx];
      if (!( newBin = _pushBinLocate(pctx, thing->point.pos) )) {
        /* bad thing! I kill you now */
        fprintf(stderr, "%s: killing thing at (%g,%g,%g)\n", me,
                thing->point.pos[0],
                thing->point.pos[1],
                thing->point.pos[2]);
        /* any real out-of-bounds points in the bins have already
           been removed, so we don't need to nullify here */
        _pushBinThingRemove(pctx, oldBin, thingIdx);
        pushThingNix(thing);
      } else {
        if (oldBin != newBin) {
          _pushBinThingRemove(pctx, oldBin, thingIdx);
          _pushBinThingAdd(pctx, newBin, thing);
          /* don't increment thingIdx; the *next* thing index
             is now at thingIdx */
        } else {
          /* this thing is already in the right bin, move on */
          thingIdx++;
        }
      }
    } /* for thingIdx */
    
  } /* for oldBinIdx */

  return 0;
}
