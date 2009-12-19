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

/*
** RETIRED: nrrdMinMaxSet()
**
** Sets nrrd->min and nrrd->max to the extremal (existant) values in
** the given nrrd, by calling the appropriate member of nrrdMinMaxExactFind[]
**
** calling this function will result in nrrd->hasNonExist being set
** (because of the nrrdFindMinMax[] functions)
**
** decided NOT to use biff, so that this is a more distinct alternative to 
** nrrdMinMaxCleverSet().

void
nrrdMinMaxSet(Nrrd *nrrd) {
  NRRD_TYPE_BIGGEST _min, _max;

  if (nrrd) {
    if (!airEnumValCheck(nrrdType, nrrd->type)
        && nrrdTypeBlock != nrrd->type) {
      nrrdFindMinMax[nrrd->type](&_min, &_max, nrrd);
      nrrd->min = nrrdDLoad[nrrd->type](&_min);
      nrrd->max = nrrdDLoad[nrrd->type](&_max);
    } else {
      nrrd->min = nrrd->max = AIR_NAN;
    }
  }
  return;
}
*/

/*
** RETIRED: nrrdMinMaxCleverSet()
**
** basically a wrapper around nrrdMinMaxSet(), with bells + whistles:
** 1) will call nrrdMinMaxSet only when one of nrrd->min and nrrd->max
**    are non-existent, with the end result that only the non-existent
**    values are over-written
** 2) obeys the nrrdStateClever8BitMinMax global state to short-cut
**    finding min and max for 8-bit data.  Values for nrrd->min or 
**    nrrd->max which were existant to start with are untouched.
** 3) reports error if there are no existent values in nrrd (AIR_EXISTS()
**    fails on every value)
**
** Like nrrdMinMaxSet(), this will always set nrrd->hasNonExist.
**
** Uses biff.

int
nrrdMinMaxCleverSet(Nrrd *nrrd) {
  char me[]="nrrdMinMaxCleverSet", err[BIFF_STRLEN];
  double min, max;

  if (!nrrd) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (airEnumValCheck(nrrdType, nrrd->type)) {
    sprintf(err, "%s: input nrrd has invalid type (%d)", me, nrrd->type);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeBlock == nrrd->type) {
    sprintf(err, "%s: can't find min/max of type %s", me,
            airEnumStr(nrrdType, nrrdTypeBlock));
  }
  if (AIR_EXISTS(nrrd->min) && AIR_EXISTS(nrrd->max)) {
    / * both of min and max already set, so we won't look for those, but
       we have to comply with stated behavior of always setting hasNonExist * /
    nrrdHasNonExistSet(nrrd);
    return 0;
  }
  if (nrrdStateClever8BitMinMax
      && (nrrdTypeChar == nrrd->type || nrrdTypeUChar == nrrd->type)) {
    if (nrrdTypeChar == nrrd->type) {
      if (!AIR_EXISTS(nrrd->min))
        nrrd->min = SCHAR_MIN;
      if (!AIR_EXISTS(nrrd->max))
        nrrd->max = SCHAR_MAX;
    } else {
      if (!AIR_EXISTS(nrrd->min))
        nrrd->min = 0;
      if (!AIR_EXISTS(nrrd->max))
        nrrd->max = UCHAR_MAX;
    }
    nrrdHasNonExistSet(nrrd);
    return 0;
  }

  / * at this point we need to find either min and/or max (at least
     one of them was non-existent on the way in) * /

  / * save incoming values in case they exist * /
  min = nrrd->min;
  max = nrrd->max;
  / * this will set nrrd->min, nrrd->max, and hasNonExist * /
  nrrdMinMaxSet(nrrd);
  if (!( AIR_EXISTS(nrrd->min) && AIR_EXISTS(nrrd->max) )) {
    sprintf(err, "%s: no existent values!", me);
    biffAdd(NRRD, err); return 1;
  }
  / * re-enstate the existent incoming min and/or max values * /
  if (AIR_EXISTS(min))
    nrrd->min = min;
  if (AIR_EXISTS(max))
    nrrd->max = max;

  return 0;
}
*/

/*
******** nrrdConvert()
**
** copies values from one type of nrrd to another, without any
** transformation, except what you get with a cast.  The point is to
** make available on Nrrds the exact same behavior as you have in C
** with casts and assignments.
*/
int
nrrdConvert(Nrrd *nout, const Nrrd *nin, int type) {
  char me[]="nrrdConvert", typeS[AIR_STRLEN_SMALL], err[BIFF_STRLEN];
  size_t num, size[NRRD_DIM_MAX];

  if (!( nin && nout 
         && !airEnumValCheck(nrrdType, nin->type)
         && !airEnumValCheck(nrrdType, type) )) {
    sprintf(err, "%s: invalid args", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nin->type == nrrdTypeBlock || type == nrrdTypeBlock) {
    sprintf(err, "%s: can't convert to or from nrrd type %s", me,
            airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nin && nrrdTypeSize[type] != nrrdTypeSize[nin->type]) {
    sprintf(err, "%s: nout==nin but input,output type sizes unequal", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdStateDisallowIntegerNonExist 
      && !nrrdTypeIsIntegral[nin->type] && nrrdTypeIsIntegral[type]) {
    /* there's a risk of non-existant values getting converted to
       non-sensical integral values */
    if (nrrdHasNonExist(nin)) {
      sprintf(err, "%s: can't convert to integral values (%s) with "
              "non-existant values in input", me,
              airEnumStr(nrrdType, type));
      biffAdd(NRRD, err); return 1;
    }
  }

  /* if we're actually converting to the same type, just do a copy */
  if (type == nin->type) {
    if (nout == nin) {
      /* nout == nin is allowed if the input and output type are
         of the same size, which will certainly be the case if the
         input and output types are identical, so there's actually
         no work to do */
    } else {
      if (nrrdCopy(nout, nin)) {
        sprintf(err, "%s: couldn't copy input to output", me);
        biffAdd(NRRD, err); return 1;
      }
    }
  } else {
    /* allocate space if necessary */
    nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, size);
    /* MUST be nrrdMaybeAlloc_nva (not nrrd Alloc_nva) because we allow
       nout==nin if type sizes match */
    if (nrrdMaybeAlloc_nva(nout, type, nin->dim, size)) {
      sprintf(err, "%s: failed to allocate output", me);
      biffAdd(NRRD, err); return 1;
    }
    
    /* call the appropriate converter */
    num = nrrdElementNumber(nin);
    _nrrdConv[nout->type][nin->type](nout->data, nin->data, num);
    nout->blockSize = 0;
    
    /* copy peripheral information */
    nrrdAxisInfoCopy(nout, nin, NULL, NRRD_AXIS_INFO_NONE);
    sprintf(typeS, "(%s)", airEnumStr(nrrdType, nout->type));
    if (nrrdContentSet_va(nout, typeS, nin, "")) {
      sprintf(err, "%s:", me);
      biffAdd(NRRD, err); return 1;
    }
    /* the min and max have probably changed if there was a conversion
       to integral values, or to a lower precision representation */
    if (nrrdBasicInfoCopy(nout, nin,
                          NRRD_BASIC_INFO_DATA_BIT
                          | NRRD_BASIC_INFO_TYPE_BIT
                          | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                          | NRRD_BASIC_INFO_DIMENSION_BIT
                          | NRRD_BASIC_INFO_CONTENT_BIT
                          | NRRD_BASIC_INFO_COMMENTS_BIT
                          | (nrrdStateKeyValuePairsPropagate
                             ? 0
                             : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
      sprintf(err, "%s:", me);
      biffAdd(NRRD, err); return 1;
    }
  }
  return 0;
}

/*
******** nrrdQuantize()
**
** convert values to 8, 16, or 32 bit unsigned quantities
** by mapping the value range delimited by the nrrd's min
** and max to the representable range 
**
** NOTE: for the time being, this uses a "double" as the intermediate
** value holder, which may mean needless loss of precision
*/
int
nrrdQuantize(Nrrd *nout, const Nrrd *nin, const NrrdRange *_range,
             unsigned int bits) {
  char me[]="nrrdQuantize", func[]="quantize", err[BIFF_STRLEN];
  double valIn, minIn, maxIn, eps;
  int type=nrrdTypeUnknown;
  size_t I, num, size[NRRD_DIM_MAX];
  unsigned char *outUC;
  unsigned short *outUS;
  unsigned int *outUI;
  airArray *mop;
  NrrdRange *range;

  if (!(nin && nout)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    sprintf(err, "%s: can't quantize type %s", me,
            airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(NRRD, err); return 1;
  }

  /* determine nrrd type from number of bits */
  switch (bits) {
  case 8:  type = nrrdTypeUChar;  break;
  case 16: type = nrrdTypeUShort; break;
  case 32: type = nrrdTypeUInt;   break;
  default:
    sprintf(err, "%s: bits has to be 8, 16, or 32 (not %d)", me, bits);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nin && nrrdTypeSize[type] != nrrdTypeSize[nin->type]) {
    sprintf(err, "%s: nout==nin but input,output type sizes unequal", me);
    biffAdd(NRRD, err); return 1;
  }
  mop = airMopNew();
  if (_range) {
    range = nrrdRangeCopy(_range);
    nrrdRangeSafeSet(range, nin, nrrdBlind8BitRangeState);
  } else {
    range = nrrdRangeNewSet(nin, nrrdBlind8BitRangeState);
  }
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  if (nrrdStateDisallowIntegerNonExist && range->hasNonExist) {
    sprintf(err, "%s: can't quantize non-existent values (NaN, +/-inf)", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  
  /* allocate space if necessary */
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, size);
  /* MUST be nrrdMaybeAlloc_nva (not nrrd Alloc_nva) because we allow
     nout==nin if type sizes match */
  if (nrrdMaybeAlloc_nva(nout, type, nin->dim, size)) {
    sprintf(err, "%s: failed to create output", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }

  /* the skinny */
  num = nrrdElementNumber(nin);
  minIn = range->min; 
  maxIn = range->max;
  eps = (minIn == maxIn ? 1.0 : 0.0);
  outUC = (unsigned char*)nout->data;
  outUS = (unsigned short*)nout->data;
  outUI = (unsigned int*)nout->data;
  switch(bits) {
  case 8:
    for (I=0; I<num; I++) {
      valIn = nrrdDLookup[nin->type](nin->data, I);
      valIn = AIR_CLAMP(minIn, valIn, maxIn);
      outUC[I] = airIndex(minIn, valIn, maxIn+eps, 1 << 8);
    }
    break;
  case 16:
    for (I=0; I<num; I++) {
      valIn = nrrdDLookup[nin->type](nin->data, I);
      valIn = AIR_CLAMP(minIn, valIn, maxIn);
      outUS[I] = airIndex(minIn, valIn, maxIn+eps, 1 << 16);
    }
    break;
  case 32:
    for (I=0; I<num; I++) {
      valIn = nrrdDLookup[nin->type](nin->data, I);
      valIn = AIR_CLAMP(minIn, valIn, maxIn);
      outUI[I] = AIR_CAST(unsigned int,
                          airIndexULL(minIn, valIn, maxIn+eps,
                                      AIR_ULLONG(1) << 32));
    }
    break;
  }
  
  /* set information in new volume */
  if (nout != nin) {
    nrrdAxisInfoCopy(nout, nin, NULL, NRRD_AXIS_INFO_NONE);
  }
  if (nrrdContentSet_va(nout, func, nin, "%d", bits)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  if (nrrdBasicInfoCopy(nout, nin,
                        NRRD_BASIC_INFO_DATA_BIT
                        | NRRD_BASIC_INFO_TYPE_BIT
                        | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                        | NRRD_BASIC_INFO_DIMENSION_BIT
                        | NRRD_BASIC_INFO_CONTENT_BIT
                        | NRRD_BASIC_INFO_OLDMIN_BIT
                        | NRRD_BASIC_INFO_OLDMAX_BIT
                        | NRRD_BASIC_INFO_COMMENTS_BIT
                        | (nrrdStateKeyValuePairsPropagate
                           ? 0
                           : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  nout->oldMin = minIn;
  nout->oldMax = maxIn;
  nout->blockSize = 0;

  airMopOkay(mop); 
  return 0;
}

/*
******** nrrdUnquantize()
**
** try to recover floating point values from a quantized nrrd,
** using the oldMin and oldMax values, if they exist.  If they
** don't exist, the output range will be 0.0 to 1.0.  However,
** because we're using NRRD_CELL_POS to recover values,
** the output values will never be exactly 0.0 to 1.0 (or oldMin
** to oldMax).  In unsigned char data, for instance, the value
** V will be mapped to:
** NRRD_CELL_POS(0.0, 1.0, 256, V) ==
** AIR_AFFINE(0, V + 0.5, 256, 0.0, 1.0) ==
** ((double)(1.0)-(0.0))*((double)(V+0.5)-(0))/((double)(256)-(0)) + (0.0) ==
** (1.0)*(V+0.5) / (256.0) + (0.0) ==
** (V+0.5)/256
** so a 0 will be mapped to 1/512 = 0.00195
*/
int
nrrdUnquantize(Nrrd *nout, const Nrrd *nin, int type) {
  char me[]="nrrdUnquantize", func[]="unquantize", err[BIFF_STRLEN];
  float *outF;
  double *outD, minIn, numValIn, minOut, maxOut, valIn;
  size_t NN, II, size[NRRD_DIM_MAX];

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (airEnumValCheck(nrrdType, type)) {
    sprintf(err, "%s: don't recognize type %d\n", me, type);
    biffAdd(NRRD, err); return 1;
  }
  if (!( type == nrrdTypeFloat || type == nrrdTypeDouble )) {
    sprintf(err, "%s: output type must be %s or %s (not %s)", me,
            airEnumStr(nrrdType, nrrdTypeFloat),
            airEnumStr(nrrdType, nrrdTypeDouble),
            airEnumStr(nrrdType, type));
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    sprintf(err, "%s: can't unquantize type %s", me,
            airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(NRRD, err); return 1;
  }
  if (!nrrdTypeIsIntegral[nin->type]) {
    sprintf(err, "%s: can only unquantize integral types, not %s", me,
            airEnumStr(nrrdType, nin->type));
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nin && nrrdTypeSize[type] != nrrdTypeSize[nin->type]) {
    sprintf(err, "%s: nout==nin but input,output type sizes unequal", me);
    biffAdd(NRRD, err); return 1;
  }

  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, size);
  if (nrrdMaybeAlloc_nva(nout, type, nin->dim, size)) {
    sprintf(err, "%s: failed to create output", me);
    biffAdd(NRRD, err); return 1;
  }
  minIn = nrrdTypeMin[nin->type];
  numValIn = nrrdTypeNumberOfValues[nin->type];
  if (AIR_EXISTS(nin->oldMin) && AIR_EXISTS(nin->oldMax)) {
    minOut = nin->oldMin;
    maxOut = nin->oldMax;
  } else {
    minOut = 0.0;
    maxOut = 1.0;
  }
  outF = (float*)nout->data;
  outD = (double*)nout->data;
  NN = nrrdElementNumber(nin);
  switch(type) {
  case nrrdTypeFloat:
    for (II=0; II<NN; II++) {
      valIn = minIn + nrrdDLookup[nin->type](nin->data, II);
      outF[II] = AIR_CAST(nrrdResample_t,
                          NRRD_CELL_POS(minOut, maxOut, numValIn, valIn));
    }
    break;
  case nrrdTypeDouble:
    for (II=0; II<NN; II++) {
      valIn = minIn + nrrdDLookup[nin->type](nin->data, II);
      outD[II] = NRRD_CELL_POS(minOut, maxOut, numValIn, valIn);
    }
    break;
  }

  /* set information in new volume */
  if (nout != nin) {
    nrrdAxisInfoCopy(nout, nin, NULL, NRRD_AXIS_INFO_NONE);
  }
  if (nrrdContentSet_va(nout, func, nin, "")) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdBasicInfoCopy(nout, nin,
                        NRRD_BASIC_INFO_DATA_BIT
                        | NRRD_BASIC_INFO_TYPE_BIT
                        | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                        | NRRD_BASIC_INFO_DIMENSION_BIT
                        | NRRD_BASIC_INFO_CONTENT_BIT
                        | NRRD_BASIC_INFO_OLDMIN_BIT
                        | NRRD_BASIC_INFO_OLDMAX_BIT
                        | NRRD_BASIC_INFO_COMMENTS_BIT
                        | (nrrdStateKeyValuePairsPropagate
                           ? 0
                           : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  nout->oldMin = nout->oldMax = AIR_NAN;
  nout->blockSize = 0;
  return 0;
}


/*
** _nrrdHistoEqCompare()
**
** used by nrrdHistoEq in smart mode to sort the "steady" array
** in _descending_ order
*/
int 
_nrrdHistoEqCompare(const void *a, const void *b) {

  return *((unsigned int*)b) - *((unsigned int*)a);
}

/*
******** nrrdHistoEq()
**
** performs histogram equalization on given nrrd, treating it as a
** big one-dimensional array.  The procedure is as follows: 
** - create a histogram of nrrd (using "bins" bins)
** - integrate the histogram, and normalize and shift this so it is 
**   a monotonically increasing function from min to max, where
**   (min,max) is the range of values in the nrrd
** - map the values in the nrrd through the adjusted histogram integral
** 
** If the histogram of the given nrrd is already as flat as can be,
** the histogram integral will increase linearly, and the adjusted
** histogram integral should be close to the identity function, so
** the values shouldn't change much.
**
** If the nhistP arg is non-NULL, then it is set to point to
** the histogram that was used for calculation. Otherwise this
** histogram is deleted on return.
**
** This is all that is done normally, when "smart" is == 0.  In
** "smart" mode (activated by setting "smart" to something greater
** than 0), the histogram is analyzed during its creation to detect if
** there are a few bins which keep getting hit with the same value
** over and over.  It may be desirable to ignore these bins in the
** histogram integral because they may not contain any useful
** information, and so they should not effect how values are
** re-mapped.  The value of "smart" is the number of bins that will be
** ignored.  For instance, use the value 1 if the problem with naive
** histogram equalization is a large amount of background (which is
** exactly one fixed value).  
*/
int
nrrdHistoEq(Nrrd *nout, const Nrrd *nin, Nrrd **nmapP,
            unsigned int bins, unsigned int smart, float amount) {
  char me[]="nrrdHistoEq", func[]="heq", err[BIFF_STRLEN];
  Nrrd *nhist, *nmap;
  float *ycoord = NULL;
  double val, min, max, *last = NULL;
  int *respect = NULL, lort;
  unsigned int *hist, *steady = NULL, idx, hirt;
  size_t I, num;
  airArray *mop;
  NrrdRange *range;
  unsigned bii;

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    sprintf(err, "%s: can't histogram equalize type %s", me,
            airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(NRRD, err); return 1;
  }
  if (!(bins > 4)) {
    sprintf(err, "%s: need # bins > 4 (not %d)", me, bins);
    biffAdd(NRRD, err); return 1;
  }
  /* we start by simply copying, because the only thing we're 
     changing is the values themselves, and all peripheral
     information is unchanged by this value remapping */
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s:", me); 
      biffAdd(NRRD, err); return 1;
    }
  }

  mop = airMopNew();
  if (nmapP) {
    airMopAdd(mop, nmapP, (airMopper)airSetNull, airMopOnError);
  }
  num = nrrdElementNumber(nin);
  if (smart <= 0) {
    nhist = nrrdNew();
    if (nrrdHisto(nhist, nin, NULL, NULL, bins, nrrdTypeUInt)) {
      sprintf(err, "%s: failed to create histogram", me);
      biffAdd(NRRD, err); airMopError(mop); return 1;
    }
    airMopAdd(mop, nhist, (airMopper)nrrdNuke, airMopAlways);
    hist = (unsigned int*)nhist->data;
    min = nhist->axis[0].min;
    max = nhist->axis[0].max;
  } else {
    /* for "smart" mode, we have to some extra work while creating the
       histogram to look for bins incessantly hit with the exact same
       value */
    if (nrrdMaybeAlloc_va(nhist=nrrdNew(), nrrdTypeUInt, 1,
                          AIR_CAST(size_t, bins))) {
      sprintf(err, "%s: failed to allocate histogram", me);
      biffAdd(NRRD, err); airMopError(mop); return 1;
    }
    airMopAdd(mop, nhist, (airMopper)nrrdNuke, airMopAlways);
    hist = (unsigned int*)nhist->data;
    nhist->axis[0].size = bins;
    /* allocate the respect, steady, and last arrays */
    respect = (int*)calloc(bins, sizeof(int));
    steady = (unsigned int*)calloc(2*bins, sizeof(unsigned int));
    last = (double*)calloc(bins, sizeof(double));
    airMopMem(mop, &respect, airMopAlways);
    airMopMem(mop, &steady, airMopAlways);
    airMopMem(mop, &last, airMopAlways);
    if (!(respect && steady && last)) {
      sprintf(err, "%s: couldn't allocate smart arrays", me);
      biffAdd(NRRD, err); airMopError(mop); return 1;
    }
    /* steady[0 + 2*bii] == how many times has bin bii seen the same value
       steady[1 + 2*bii] == bii (steady will be rearranged by qsort()) */
    for (bii=0; bii<bins; bii++) {
      last[bii] = AIR_NAN;
      respect[bii] = 1;
      steady[1 + 2*bii] = bii;
    }
    /* now create the histogram */
    range = nrrdRangeNewSet(nin, nrrdBlind8BitRangeState);
    airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
    if (range->min == range->max) {
      sprintf(err, "%s: invalid min and max in nrrd.  "
              "Min and max are equivalent (min,max = %g).", me, range->min);
      biffAdd(NRRD, err); airMopError(mop); return 1;
    }
    min = range->min;
    max = range->max;
    for (I=0; I<num; I++) {
      val = nrrdDLookup[nin->type](nin->data, I);
      if (AIR_EXISTS(val)) {
        idx = airIndex(min, val, max, bins);
        ++hist[idx];
        if (AIR_EXISTS(last[idx])) {
          steady[0 + 2*idx] = (last[idx] == val
                               ? 1 + steady[0 + 2*idx]
                               : 0);
        }
        last[idx] = val;
      }
    }
    /* now sort the steady array */
    qsort(steady, bins, 2*sizeof(unsigned int), _nrrdHistoEqCompare);
    /* we ignore some of the bins according to "smart" arg */
    for (bii=0; bii<smart; bii++) {
      respect[steady[1+2*bii]] = 0;
      /* printf("%s: disrespecting bin %d\n", me, steady[1+2*bii]); */
    }
  }
  if (nrrdMaybeAlloc_va(nmap=nrrdNew(), nrrdTypeFloat, 1,
                        AIR_CAST(size_t, bins+1))) {
    sprintf(err, "%s: failed to create map nrrd", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, nmap, (airMopper)nrrdNuke,
            nmapP ? airMopOnError : airMopAlways);
  ycoord = (float*)nmap->data;
  nmap->axis[0].min = min;
  nmap->axis[0].max = max;

  /* integrate the histogram then normalize it */
  for (bii=0; bii<=bins; bii++) {
    if (bii == 0) {
      ycoord[bii] = 0;
    } else {
      ycoord[bii] = ycoord[bii-1] + hist[bii-1]*(smart 
                                                 ? respect[bii-1] 
                                                 : 1);
    }
  }
  /* if we've done smart, the integral will have little flat spots
     where we ignored hits in the histogram.  That means the mapping
     will actually be lower at that value than it should be.  In
     truth, we should be using an irregular mapping for this, and the
     control points at the ignored bins should just be missing.  So we
     have to do this silliness to raise those control points in the
     regular map. */
  if (smart) {
    /* there are bins+1 control points, with indices 0 to bins.
       We'll fix control points 1 to bins-1.  ycoord[bii] is too low
       if hist[bii-1] was not respected (!respect[bii-1]) */
    for (bii=1; bii<=bins-1; bii++) {
      if (!respect[bii-1]) {
        /* lort and hirt will bracket the index of the bad control point
           with points corresponding either to respected bins or the
           endpoints of the histogram */
        for (lort=bii; lort>=1 && !respect[lort-1]; lort--);
        for (hirt=bii; hirt<bins && !respect[hirt-1]; hirt++);
        ycoord[bii] = AIR_CAST(nrrdResample_t,
                               AIR_AFFINE(lort, bii, hirt,
                                          ycoord[lort], ycoord[hirt]));
      }
    }
    /* the very last control point has to be handled differently */
    if (!respect[bins-1]) {
      ycoord[bins] += ycoord[bins-1] - ycoord[bins-2];
    }
  }
  /* rescale the histogram integration to span the original
     value range, and affect the influence of "amount" */
  for (bii=0; bii<=bins; bii++) {
    ycoord[bii] = AIR_CAST(nrrdResample_t,
                           AIR_AFFINE(0.0, ycoord[bii], ycoord[bins],
                                      min, max));
    ycoord[bii] = AIR_CAST(nrrdResample_t,
                           AIR_AFFINE(0.0, amount, 1.0,
                                      AIR_AFFINE(0, bii, bins, min, max),
                                      ycoord[bii]));
  }

  /* map the nrrd values through the normalized histogram integral */
  if (nrrdApply1DRegMap(nout, nin, NULL, nmap, nin->type, AIR_FALSE)) {
    sprintf(err, "%s: problem remapping", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  /*
  for (I=0; I<num; I++) {
    val = nrrdDLookup[nin->type](nin->data, I);
    if (AIR_EXISTS(val)) {
      AIR_INDEX(min, val, max, bins, idx);
      val = AIR_AFFINE(xcoord[idx], val, xcoord[idx+1], 
                       ycoord[idx], ycoord[idx+1]);
    }
    nrrdDInsert[nout->type](nout->data, I, val);
  }
  */

  /* if user is interested, set pointer to map nrrd,
     otherwise it will be nixed by airMop */
  if (nmapP) {
    *nmapP = nmap;
  }

  /* fiddling with content is the only thing we'll do */
  if (nrrdContentSet_va(nout, func, nin, "%d,%d", bins, smart)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  if (nrrdBasicInfoCopy(nout, nin,
                        NRRD_BASIC_INFO_DATA_BIT
                        | NRRD_BASIC_INFO_TYPE_BIT
                        | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                        | NRRD_BASIC_INFO_DIMENSION_BIT
                        | NRRD_BASIC_INFO_CONTENT_BIT
                        | NRRD_BASIC_INFO_COMMENTS_BIT
                        | (nrrdStateKeyValuePairsPropagate
                           ? 0
                           : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
