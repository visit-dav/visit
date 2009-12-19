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

#include "nrrd.h"
#include "privateNrrd.h"

NrrdRange *
nrrdRangeNew(double min, double max) {
  NrrdRange *range;

  range = (NrrdRange *)calloc(1, sizeof(NrrdRange));
  if (range) {
    range->min = min;
    range->max = max;
    range->hasNonExist = nrrdHasNonExistUnknown;
  }
  return range;
}

NrrdRange *
nrrdRangeCopy(const NrrdRange *rin) {
  NrrdRange *rout=NULL;

  if (rin) {
    rout = nrrdRangeNew(rin->min, rin->max);
    rout->hasNonExist = rin->hasNonExist;
  }
  return rout;
}

NrrdRange *
nrrdRangeNix(NrrdRange *range) {

  airFree(range);
  return NULL;
}

void
nrrdRangeReset(NrrdRange *range) {
  
  if (range) {
    range->min = AIR_NAN;
    range->max = AIR_NAN;
    range->hasNonExist = nrrdHasNonExistUnknown;
  }
}

void
nrrdRangeSet(NrrdRange *range, const Nrrd *nrrd, int blind8BitRange) {
  NRRD_TYPE_BIGGEST _min, _max;
  int blind;
  
  if (!range) {
    return;
  }
  if (nrrd 
      && !airEnumValCheck(nrrdType, nrrd->type)
      && nrrdTypeBlock != nrrd->type) {
    blind = (nrrdBlind8BitRangeTrue == blind8BitRange
             || (nrrdBlind8BitRangeState == blind8BitRange
                 && nrrdStateBlind8BitRange));
    if (blind && 1 == nrrdTypeSize[nrrd->type]) {
      if (nrrdTypeChar == nrrd->type) {
        range->min = SCHAR_MIN;
        range->max = SCHAR_MAX;
      } else {
        range->min = 0;
        range->max = UCHAR_MAX;
      }
      range->hasNonExist = nrrdHasNonExistFalse;
    } else {
      nrrdMinMaxExactFind[nrrd->type](&_min, &_max, &(range->hasNonExist),
                                      nrrd);
      range->min = nrrdDLoad[nrrd->type](&_min);
      range->max = nrrdDLoad[nrrd->type](&_max);
    }
  } else {
    range->min = range->max = AIR_NAN;
    range->hasNonExist = nrrdHasNonExistUnknown;
  }
  return;
}

void
nrrdRangeSafeSet(NrrdRange *range, const Nrrd *nrrd, int blind8BitRange) {
  double minIn, maxIn;

  if (!range) {
    return;
  }
  minIn = range->min;
  maxIn = range->max;
  nrrdRangeSet(range, nrrd, blind8BitRange);
  if (AIR_EXISTS(minIn)) {
    range->min = minIn;
  }
  if (AIR_EXISTS(maxIn)) {
    range->max = maxIn;
  }
  return;
}

NrrdRange *
nrrdRangeNewSet(const Nrrd *nrrd, int blind8BitRange) {
  NrrdRange *range;

  range = nrrdRangeNew(0, 0);  /* doesn't matter what values are used here */
  nrrdRangeSet(range, nrrd, blind8BitRange);
  return range;
}

/*
******** nrrdHasNonExist
**
** returns the nrrdHasNonExist* enum value appropriate for a given nrrd.
** By cleverness, this value can be used as a regular C boolean, so that
** the function will act as you expect.  
**
** (the existence of this function implies that I'll never have an airEnum
** of the same name, which would be the usual thing to do with a C enum,
** but I don't think an airEnum for this would be useful)
*/
int
nrrdHasNonExist(const Nrrd *nrrd) {
  NRRD_TYPE_BIGGEST _min, _max;
  int ret;
  
  if (nrrd
      && !airEnumValCheck(nrrdType, nrrd->type)
      && nrrdTypeBlock != nrrd->type) {
    if (nrrdTypeIsIntegral[nrrd->type]) {
      ret = nrrdHasNonExistFalse;
    } else {
      /* HEY: this could be optimized by being more specialized */
      nrrdMinMaxExactFind[nrrd->type](&_min, &_max, &ret, nrrd);
    }
  } else {
    ret = nrrdHasNonExistUnknown;
  }
  return ret;
}
