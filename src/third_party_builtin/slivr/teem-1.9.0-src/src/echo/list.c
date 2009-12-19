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

void
echoListAdd(echoObject *list, echoObject *child) {
  int idx;
  
  if (!( list && child &&
         (echoTypeList == list->type ||
          echoTypeAABBox == list->type) ))
    return;
  
  idx = airArrayLenIncr(LIST(list)->objArr, 1);
  LIST(list)->obj[idx] = child;

  return;
}

int
_echoPosCompare(double *A, double *B) {
  
  return *A < *B ? -1 : (*A > *B ? 1 : 0);
}

/*
******** echoListSplit()
**
** returns a echoObjectSplit to point to the same things as pointed
** to by the given echoObjectList
*/
echoObject *
echoListSplit(echoScene *scene, echoObject *list, int axis) {
  echoPos_t lo[3], hi[3], loest0[3], hiest0[3],
    loest1[3], hiest1[3];
  double *mids;
  echoObject *o, *split, *list0, *list1;
  int i, splitIdx, len;

  if (!( echoTypeList == list->type ||
         echoTypeAABBox == list->type )) {
    return list;
  }

  len = LIST(list)->objArr->len;
  if (len <= ECHO_LEN_SMALL_ENOUGH) {
    /* there is nothing or only one object */
    return list;
  }

  split = echoObjectNew(scene, echoTypeSplit);
  list0 = echoObjectNew(scene, echoTypeList);
  list1 = echoObjectNew(scene, echoTypeList);
  SPLIT(split)->axis = axis;
  SPLIT(split)->obj0 = list0;
  SPLIT(split)->obj1 = list1;

  mids = (double *)malloc(2 * len * sizeof(double));
  for (i=0; i<len; i++) {
    o = LIST(list)->obj[i];
    echoBoundsGet(lo, hi, o);
    mids[0 + 2*i] = (lo[axis] + hi[axis])/2;
    *((unsigned int *)(mids + 1 + 2*i)) = i;
  }
  /* overkill, I know, I know */
  qsort(mids, len, 2*sizeof(double),
        (int (*)(const void *, const void *))_echoPosCompare);
  /*
  for (i=0; i<len; i++) {
    printf("%d -> %g\n", i, mids[0 + 2*i]);
  }
  */
  
  splitIdx = len/2;
  /* printf("splitIdx = %d\n", splitIdx); */
  ELL_3V_SET(loest0, ECHO_POS_MAX, ECHO_POS_MAX, ECHO_POS_MAX);
  ELL_3V_SET(loest1, ECHO_POS_MAX, ECHO_POS_MAX, ECHO_POS_MAX);
  ELL_3V_SET(hiest0, ECHO_POS_MIN, ECHO_POS_MIN, ECHO_POS_MIN);
  ELL_3V_SET(hiest1, ECHO_POS_MIN, ECHO_POS_MIN, ECHO_POS_MIN);
  airArrayLenSet(LIST(list0)->objArr, splitIdx);
  for (i=0; i<splitIdx; i++) {
    o = LIST(list)->obj[*((unsigned int *)(mids + 1 + 2*i))];
    LIST(list0)->obj[i] = o;
    echoBoundsGet(lo, hi, o);
    /*
    printf("000 lo = (%g,%g,%g), hi = (%g,%g,%g)\n",
           lo[0], lo[1], lo[2], hi[0], hi[1], hi[2]);
    */
    ELL_3V_MIN(loest0, loest0, lo);
    ELL_3V_MAX(hiest0, hiest0, hi);
  }
  airArrayLenSet(LIST(list1)->objArr, len-splitIdx);
  for (i=splitIdx; i<len; i++) {
    o = LIST(list)->obj[*((unsigned int *)(mids + 1 + 2*i))];
    LIST(list1)->obj[i-splitIdx] = o;
    echoBoundsGet(lo, hi, o);
    /*
    printf("111 lo = (%g,%g,%g), hi = (%g,%g,%g)\n",
           lo[0], lo[1], lo[2], hi[0], hi[1], hi[2]);
    */
    ELL_3V_MIN(loest1, loest1, lo);
    ELL_3V_MAX(hiest1, hiest1, hi);
  }
  /*
  printf("0: loest = (%g,%g,%g); hiest = (%g,%g,%g)\n",
         loest0[0], loest0[1], loest0[2], 
         hiest0[0], hiest0[1], hiest0[2]);
  printf("1: loest = (%g,%g,%g); hiest = (%g,%g,%g)\n",
         loest1[0], loest1[1], loest1[2], 
         hiest1[0], hiest1[1], hiest1[2]);
  */
  ELL_3V_COPY(SPLIT(split)->min0, loest0);
  ELL_3V_COPY(SPLIT(split)->max0, hiest0);
  ELL_3V_COPY(SPLIT(split)->min1, loest1);
  ELL_3V_COPY(SPLIT(split)->max1, hiest1);

  /* we can't delete the list object here, we just gut it so 
     that there's nothing substantial left of it */
  airArrayLenSet(LIST(list)->objArr, 0);
  mids = (double *)airFree(mids);
  return split;
}

echoObject *
echoListSplit3(echoScene *scene, echoObject *list, int depth) {
  echoObject *ret, *tmp0, *tmp1;

  if (!( echoTypeList == list->type ||
         echoTypeAABBox == list->type )) 
    return NULL;

  if (!depth)
    return list;

  ret = echoListSplit(scene, list, 0);

#define DOIT(obj, ax) ((obj) = echoListSplit(scene, (obj), (ax)))
#define MORE(obj) echoTypeSplit == (obj)->type

  if (MORE(ret)) {
    tmp0 = DOIT(SPLIT(ret)->obj0, 1);
    if (MORE(tmp0)) {
      tmp1 = DOIT(SPLIT(tmp0)->obj0, 2);
      if (MORE(tmp1)) {
        SPLIT(tmp1)->obj0 = echoListSplit3(scene, SPLIT(tmp1)->obj0, depth-1);
        SPLIT(tmp1)->obj1 = echoListSplit3(scene, SPLIT(tmp1)->obj1, depth-1);
      }
      tmp1 = DOIT(SPLIT(tmp0)->obj1, 2);
      if (MORE(tmp1)) {
        SPLIT(tmp1)->obj0 = echoListSplit3(scene, SPLIT(tmp1)->obj0, depth-1);
        SPLIT(tmp1)->obj1 = echoListSplit3(scene, SPLIT(tmp1)->obj1, depth-1);
      }
    }
    tmp0 = DOIT(SPLIT(ret)->obj1, 1);
    if (MORE(tmp0)) {
      tmp1 = DOIT(SPLIT(tmp0)->obj0, 2);
      if (MORE(tmp1)) {
        SPLIT(tmp1)->obj0 = echoListSplit3(scene, SPLIT(tmp1)->obj0, depth-1);
        SPLIT(tmp1)->obj1 = echoListSplit3(scene, SPLIT(tmp1)->obj1, depth-1);
      }
      tmp1 = DOIT(SPLIT(tmp0)->obj1, 2);
      if (MORE(tmp1)) {
        SPLIT(tmp1)->obj0 = echoListSplit3(scene, SPLIT(tmp1)->obj0, depth-1);
        SPLIT(tmp1)->obj1 = echoListSplit3(scene, SPLIT(tmp1)->obj1, depth-1);
      }
    }
  }
  return ret;
}

