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

/* int _VV = 0; */

/*
** learned: even when using doubles, because of limited floating point
** precision, you can get different results between quantizing 
** unrescaled (value directly from nrrd, map domain set to nrrd range,
** as with early behavior of unu rmap) and rescaled (value from nrrd
** scaled to fit in existing map domain, as with unu imap -r) value,
** to the exact same index space.
*/

/*
** I won't try to support mapping individual values through a
** colormap, as with a function evaluation on a single passed value.
** That will be handled in an upcoming library...
*/

/* 
** this identifies the different kinds of 1D maps, useful for the
** functions in this file only
*/
enum {
  kindLut=0,
  kindRmap=1,
  kindImap=2
};

double
_nrrdApplyDomainMin(const Nrrd *nmap, int ramps, int mapAxis) {
  double ret;
  
  AIR_UNUSED(ramps);
  ret = nmap->axis[mapAxis].min;
  ret = AIR_EXISTS(ret) ? ret : 0;
  return ret;
}

double
_nrrdApplyDomainMax(const Nrrd *nmap, int ramps, int mapAxis) {
  double ret;
  
  ret = nmap->axis[mapAxis].max;
  if (!AIR_EXISTS(ret)) {
    ret = nmap->axis[mapAxis].size;
    ret = ramps ? ret-1 : ret;
  }
  return ret;
}

/*
** _nrrdApply1DSetUp()
**
** some error checking and initializing needed for 1D LUTS, regular,
** and irregular maps.  The intent is that if this succeeds, then
** there is no need for any further error checking.
**
** The only thing this function DOES is allocate the output nrrd, and
** set meta information.  The rest is just error checking.
**
** The given NrrdRange has to be fleshed out by the caller: it can't
** be NULL, and both range->min and range->max must exist.
*/
int
_nrrdApply1DSetUp(Nrrd *nout, const Nrrd *nin, const NrrdRange *range, 
                  const Nrrd *nmap, int kind, int typeOut,
                  int rescale, int multi) {
  char me[]="_nrrdApply1DSetUp", err[BIFF_STRLEN], *mapcnt;
  char nounStr[][AIR_STRLEN_SMALL]={"lut",
                                    "regular map",
                                    "irregular map"};
  char mnounStr[][AIR_STRLEN_SMALL]={"multi lut",
                                     "multi regular map",
                                     "multi irregular map"}; 
                                      /* wishful thinking */
  char verbStr[][AIR_STRLEN_SMALL]={"lut",
                                    "rmap",
                                    "imap"};
  char mverbStr[][AIR_STRLEN_SMALL]={"mlut",
                                     "mrmap",
                                     "mimap"}; /* wishful thinking */
  int mapAxis, copyMapAxis0=AIR_FALSE, axisMap[NRRD_DIM_MAX];
  unsigned int ax, dim, entLen;
  size_t size[NRRD_DIM_MAX];
  double domMin, domMax;

  if (nout == nin) {
    sprintf(err, "%s: due to laziness, nout==nin always disallowed", me);
    biffAdd(NRRD, err); return 1;
  }
  if (airEnumValCheck(nrrdType, typeOut)) {
    sprintf(err, "%s: invalid requested output type %d", me, typeOut);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeBlock == nin->type || nrrdTypeBlock == typeOut) {
    sprintf(err, "%s: input or requested output type is %s, need scalar",
            me, airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(NRRD, err); return 1;
  }
  if (rescale && !(range
                   && AIR_EXISTS(range->min) 
                   && AIR_EXISTS(range->max))) {
    sprintf(err, "%s: want rescaling but didn't get a range, or "
            "not both range->{min,max} exist", me);
    biffAdd(NRRD, err); return 1;
  }
  if (kindLut == kind || kindRmap == kind) {
    if (!multi) {
      mapAxis = nmap->dim - 1;
      if (!(0 == mapAxis || 1 == mapAxis)) {
        sprintf(err, "%s: dimension of %s should be 1 or 2, not %d", 
                me, nounStr[kind], nmap->dim);
        biffAdd(NRRD, err); return 1;
      }
      copyMapAxis0 = (1 == mapAxis);
    } else {
      mapAxis = nmap->dim - nin->dim - 1;
      if (!(0 == mapAxis || 1 == mapAxis)) {
        sprintf(err, "%s: dimension of %s should be %d or %d, not %d", 
                me, mnounStr[kind],
                nin->dim + 1, nin->dim + 2, nmap->dim);
        biffAdd(NRRD, err); return 1;
      }
      copyMapAxis0 = (1 == mapAxis);
      /* need to make sure the relevant sizes match */
      for (ax=0; ax<nin->dim; ax++) {
        if (nin->axis[ax].size != nmap->axis[mapAxis + 1 + ax].size) {
          sprintf(err, "%s: input and mmap don't have compatible sizes: "
                  "nin->axis[%d].size (" _AIR_SIZE_T_CNV ") "
                  "!= nmap->axis[%d].size (" _AIR_SIZE_T_CNV "): ",
                  me, ax, nin->axis[ax].size, 
                  mapAxis + 1 + ax, nmap->axis[mapAxis + 1 + ax].size);
          biffAdd(NRRD, err); return 1;
        }
      }
    }
    domMin = _nrrdApplyDomainMin(nmap, AIR_FALSE, mapAxis);
    domMax = _nrrdApplyDomainMax(nmap, AIR_FALSE, mapAxis);
    if (!( domMin < domMax )) {
      sprintf(err, "%s: (axis %d) domain min (%g) not less than max (%g)", me,
              mapAxis, domMin, domMax);
      biffAdd(NRRD, err); return 1;
    }
    if (nrrdHasNonExist(nmap)) {
      sprintf(err, "%s: %s nrrd has non-existent values",
              me, multi ? mnounStr[kind] : nounStr[kind]);
      biffAdd(NRRD, err); return 1;
    }
    entLen = mapAxis ? nmap->axis[0].size : 1;
  } else {
    if (multi) {
      sprintf(err, "%s: sorry, multi irregular maps not implemented", me);
      biffAdd(NRRD, err); return 1;
    }
    /* its an irregular map */
    if (nrrd1DIrregMapCheck(nmap)) {
      sprintf(err, "%s: problem with irregular map", me);
      biffAdd(NRRD, err); return 1;
    }
    /* mapAxis has no meaning for irregular maps, but we'll pretend ... */
    mapAxis = nmap->axis[0].size == 2 ? 0 : 1;
    copyMapAxis0 = AIR_TRUE;
    entLen = nmap->axis[0].size-1;
  }
  if (mapAxis + nin->dim > NRRD_DIM_MAX) {
    sprintf(err, "%s: input nrrd dim %d through non-scalar %s exceeds "
            "NRRD_DIM_MAX %d",
            me, nin->dim,
            multi ? mnounStr[kind] : nounStr[kind], NRRD_DIM_MAX);
    biffAdd(NRRD, err); return 1;
  }
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, size+mapAxis);
  if (mapAxis) {
    size[0] = entLen;
    axisMap[0] = -1;
  }
  for (dim=0; dim<nin->dim; dim++) {
    axisMap[dim+mapAxis] = dim;
  }
  /*
  fprintf(stderr, "##%s: pre maybe alloc: nout->data = %p\n", me, nout->data);
  for (dim=0; dim<mapAxis + nin->dim; dim++) {
    fprintf(stderr, "    size[%d] = %d\n", d, (int)size[d]);
  }
  fprintf(stderr, "   nout->dim = %d; nout->type = %d = %s; sizes = %d,%d\n", 
          nout->dim, nout->type,
          airEnumStr(nrrdType, nout->type));
  fprintf(stderr, "   typeOut = %d = %s\n", typeOut,
          airEnumStr(nrrdType, typeOut));
  */
  if (nrrdMaybeAlloc_nva(nout, typeOut, mapAxis + nin->dim, size)) {
    sprintf(err, "%s: couldn't allocate output nrrd", me);
    biffAdd(NRRD, err); return 1;
  }
  /*
  fprintf(stderr, "   nout->dim = %d; nout->type = %d = %s\n",
          nout->dim, nout->type,
          airEnumStr(nrrdType, nout->type),
          nout->axis[0].size, nout->axis[1].size);
  for (d=0; d<nout->dim; d++) {
    fprintf(stderr, "    size[%d] = %d\n", d, (int)nout->axis[d].size);
  }
  fprintf(stderr, "##%s: post maybe alloc: nout->data = %p\n", me, nout->data);
  */
  if (nrrdAxisInfoCopy(nout, nin, axisMap, NRRD_AXIS_INFO_NONE)) {
    sprintf(err, "%s: trouble copying axis info", me);
    biffAdd(NRRD, err); return 1;
  }
  if (copyMapAxis0) {
    _nrrdAxisInfoCopy(nout->axis + 0, nmap->axis + 0,
                      NRRD_AXIS_INFO_SIZE_BIT);
  }

  mapcnt = _nrrdContentGet(nmap);
  if (nrrdContentSet_va(nout, multi ? mverbStr[kind] : verbStr[kind],
                        nin, "%s", mapcnt)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); free(mapcnt); return 1;
  }
  free(mapcnt); 
  if (nrrdBasicInfoCopy(nout, nin,
                        NRRD_BASIC_INFO_DATA_BIT
                        | NRRD_BASIC_INFO_TYPE_BIT
                        | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                        | NRRD_BASIC_INFO_DIMENSION_BIT
                        | NRRD_BASIC_INFO_CONTENT_BIT
                        | (nrrdStateKeyValuePairsPropagate
                           ? 0
                           : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  return 0;
}

/*
** _nrrdApply1DLutOrRegMap()
**
** the guts of nrrdApply1DLut and nrrdApply1DRegMap
**
** yikes, does NOT use biff, since we're only supposed to be called
** after copious error checking.  
**
** FOR INSTANCE, this allows nout == nin, which could be a big
** problem if mapAxis == 1.
**
** we don't need a typeOut arg because nout has already been allocated
** as some specific type; we'll look at that.
**
** NOTE: non-existant values get passed through regular maps and luts
** "unchanged".  However, if the output type is integral, the results
** are probaby undefined.  HEY: there is currently no warning message
** or error handling based on nrrdStateDisallowIntegerNonExist, but
** there really should be.
*/
int
_nrrdApply1DLutOrRegMap(Nrrd *nout, const Nrrd *nin, const NrrdRange *range, 
                        const Nrrd *nmap, int ramps, int rescale, int multi) {
  /* char me[]="_nrrdApply1DLutOrRegMap"; */
  char *inData, *outData, *mapData, *entData0, *entData1;
  size_t N, I;
  double (*inLoad)(const void *v), (*mapLup)(const void *v, size_t I),
    (*outInsert)(void *v, size_t I, double d),
    val, mapIdxFrac, domMin, domMax;
  unsigned int i, mapAxis, mapLen, mapIdx, entSize, entLen, inSize, outSize;

  if (!multi) {
    mapAxis = nmap->dim - 1;           /* axis of nmap containing entries */
  } else {
    mapAxis = nmap->dim - nin->dim - 1;
  }
  mapData = (char *)nmap->data;        /* map data, as char* */
                                       /* low end of map domain */
  domMin = _nrrdApplyDomainMin(nmap, ramps, mapAxis);
                                       /* high end of map domain */
  domMax = _nrrdApplyDomainMax(nmap, ramps, mapAxis);
  mapLen = nmap->axis[mapAxis].size;   /* number of entries in map */
  mapLup = nrrdDLookup[nmap->type];    /* how to get doubles out of map */
  inData = (char *)nin->data;          /* input data, as char* */
  inLoad = nrrdDLoad[nin->type];       /* how to get doubles out of nin */
  inSize = nrrdElementSize(nin);       /* size of one input value */
  outData = (char *)nout->data;        /* output data, as char* */
  outInsert = nrrdDInsert[nout->type]; /* putting doubles into output */
  entLen = (mapAxis                    /* number of elements in one entry */
            ? nmap->axis[0].size
            : 1);
  outSize = entLen*nrrdElementSize(nout); /* size of entry in output */
  entSize = entLen*nrrdElementSize(nmap); /* size of entry in map */

  N = nrrdElementNumber(nin);       /* the number of values to be mapped */
  if (ramps) {
    /* regular map */
    for (I=0; I<N; I++) {
      /* ...
      if (!(I % 100)) fprintf(stderr, "I = %d\n", (int)I);
      ... */
      val = inLoad(inData);
      /* ...
      fprintf(stderr, "##%s: val = \na% 31.15f --> ", me, val);
      ... */
      if (rescale) {
        val = AIR_AFFINE(range->min, val, range->max, domMin, domMax);
        /* ...
        fprintf(stderr, "\nb% 31.15f (min,max = %g,%g)--> ", val,
                range->min, range->max);
        ... */
      }
      /* ...
      fprintf(stderr, "\nc% 31.15f --> clamp(%g,%g), %d --> ",
              val, domMin, domMax, mapLen);
      ... */
      if (AIR_EXISTS(val)) {
        val = AIR_CLAMP(domMin, val, domMax);
        mapIdxFrac = AIR_AFFINE(domMin, val, domMax, 0, mapLen-1);
        /* ...
        fprintf(stderr, "mapIdxFrac = \nd% 31.15f --> ",
                mapIdxFrac);
        ... */
        mapIdx = (unsigned int)mapIdxFrac;
        mapIdx -= mapIdx == mapLen-1;
        mapIdxFrac -= mapIdx;
        /* ...
        fprintf(stderr, "%s: (%d,\ne% 31.15f) --> \n",
                me, mapIdx, mapIdxFrac); 
        ... */
        entData0 = mapData + mapIdx*entSize;
        entData1 = mapData + (mapIdx+1)*entSize;
        for (i=0; i<entLen; i++) {
          val = ((1-mapIdxFrac)*mapLup(entData0, i) + 
                 mapIdxFrac*mapLup(entData1, i));
          outInsert(outData, i, val);
          /* ...
          fprintf(stderr, "f% 31.15f\n", val); 
          ... */
        }
      } else {
        /* copy non-existant values from input to output */
        for (i=0; i<entLen; i++) {
          outInsert(outData, i, val);
        }
      }
      inData += inSize;
      outData += outSize;
      if (multi) {
        mapData += mapLen*entSize;
      }
    }    
  } else {
    /* lookup table */
    for (I=0; I<N; I++) {
      val = inLoad(inData);
      if (rescale) {
        val = AIR_AFFINE(range->min, val, range->max, domMin, domMax);
      }
      if (AIR_EXISTS(val)) {
        mapIdx = airIndexClamp(domMin, val, domMax, mapLen);
        entData0 = mapData + mapIdx*entSize;
        for (i=0; i<entLen; i++) {
          outInsert(outData, i, mapLup(entData0, i));
        }
      } else {
        /* copy non-existant values from input to output */
        for (i=0; i<entLen; i++) {
          outInsert(outData, i, val);
        }
      }
      inData += inSize;
      outData += outSize;
      if (multi) {
        mapData += mapLen*entSize;
      }
    }
  }

  return 0;
}

/*
******** nrrdApply1DLut
**
** A "lut" is a simple lookup table: the data points are evenly spaced,
** with cell-centering assumed, and there is no interpolation except
** nearest neighbor.  The axis min and max are used to determine the
** range of values that can be mapped with the lut.
**
** Of the three kinds of 1-D maps, only luts can have output type block.
**
** If the lut nrrd is 1-D, then the output is a scalar nrrd with the
** same dimension as the input.  If the lut nrrd is 2-D, then each
** value in the input is mapped to a vector of values from the lut,
** which is always a scanline along axis 0.  
**
** This allows lut length to be simply 1.
*/
int
nrrdApply1DLut(Nrrd *nout, const Nrrd *nin,
               const NrrdRange *_range, const Nrrd *nlut,
               int typeOut, int rescale) {
  char me[]="nrrdApply1DLut", err[BIFF_STRLEN];
  NrrdRange *range;
  airArray *mop;
  
  if (!(nout && nlut && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
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
  if (_nrrdApply1DSetUp(nout, nin, range, nlut, kindLut, typeOut,
                        rescale, AIR_FALSE)
      || _nrrdApply1DLutOrRegMap(nout, nin, range, nlut, AIR_FALSE,
                                 rescale, AIR_FALSE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

int
nrrdApplyMulti1DLut(Nrrd *nout, const Nrrd *nin,
                    const NrrdRange *_range, const Nrrd *nmlut,
                    int typeOut, int rescale) {
  char me[]="nrrdApplyMulti1DLut", err[BIFF_STRLEN];
  NrrdRange *range;
  airArray *mop;
  
  if (!(nout && nmlut && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
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
  if (_nrrdApply1DSetUp(nout, nin, range, nmlut, kindLut, typeOut,
                        rescale, AIR_TRUE)
      || _nrrdApply1DLutOrRegMap(nout, nin, range, nmlut, AIR_FALSE,
                                 rescale, AIR_TRUE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

/*
******** nrrdApply1DRegMap
**
** A regular map has data points evenly spaced, with node-centering and
** and linear interpolation assumed.  As with luts, the axis min and
** max determine the range of values that are mapped.  Previously, the
** intent was to allow interpolation with an arbitrary kernel, but
** that would be complicated to generalize to N-D.  Also, for the 
** foreseeable future, this function is to be used by nrrdHistoEq() and
** nothing else in Teem, so the need for higher-order filtering is not
** accute.  Besides, it probably belongs in another library anyway.
**
** On second thought, eliminating arbitrary interpolation makes the
** existance of this function a little less motivated, given that it
** is a special case of nrrdApply1DIrregMap().  It should be faster,
** though, because there is no index search.
**
** NOTE: comments in the preceding paragraphs were made in ancient times-
** this function is the basis of the very popular "unu rmap", and this
** function's existance is indeed perfectly well motivated, regardless
** of the lack of arbitrary reconstruction.
**
** If the lut nrrd is 1-D, then the output is a scalar nrrd with the
** same dimension as the input.  If the lut nrrd is 2-D, then each
** value in the input is mapped to a linear weighting of vectors 
** from the map; the vectors are the scanlines along axis 0.
**
** NB: this function makes NO provisions for non-existant input values.
** There won't be any memory errors, but the results are undefined.
**
** This allows map length to be simply 1.
*/
int
nrrdApply1DRegMap(Nrrd *nout, const Nrrd *nin,
                  const NrrdRange *_range, const Nrrd *nmap,
                  int typeOut, int rescale) {
  char me[]="nrrdApply1DRegMap", err[BIFF_STRLEN];
  NrrdRange *range;
  airArray *mop;

  if (!(nout && nmap && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
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
  if (_nrrdApply1DSetUp(nout, nin, range, nmap, kindRmap, typeOut,
                        rescale, AIR_FALSE)
      || _nrrdApply1DLutOrRegMap(nout, nin, range, nmap, AIR_TRUE,
                                 rescale, AIR_FALSE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

int
nrrdApplyMulti1DRegMap(Nrrd *nout, const Nrrd *nin,
                       const NrrdRange *_range, const Nrrd *nmmap,
                       int typeOut, int rescale) {
  char me[]="nrrdApplyMulti1DRegMap", err[BIFF_STRLEN];
  NrrdRange *range;
  airArray *mop;

  if (!(nout && nmmap && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
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
  if (_nrrdApply1DSetUp(nout, nin, range, nmmap, kindRmap, typeOut,
                        rescale, AIR_TRUE)
      || _nrrdApply1DLutOrRegMap(nout, nin, range, nmmap, AIR_TRUE,
                                 rescale, AIR_TRUE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

/*
******** nrrd1DIrregMapCheck()
**
** return zero only for the valid forms of 1D irregular map.
** imap must be 2D, both sizes >= 2, non-block-type, no non-existant
** values in range.  If the first point's position is non-existant,
** than the first three points positions must be -inf, NaN, and +inf,
** and none of the other points locations can be non-existant, and
** they must increase monotonically.  There must be at least two
** points with existant positions.
*/
int
nrrd1DIrregMapCheck(const Nrrd *nmap) {
  char me[]="nrrd1DIrregMapCheck", err[BIFF_STRLEN];
  double (*mapLup)(const void *v, size_t I);
  int i, entLen, mapLen, baseI;
  size_t min[2], max[2];
  Nrrd *nrange;

  if (!nmap) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdCheck(nmap)) {
    sprintf(err, "%s: ", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeBlock == nmap->type) {
    sprintf(err, "%s: map is %s type, need scalar", 
            me, airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(NRRD, err); return 1;
  }
  if (2 != nmap->dim) {
    sprintf(err, "%s: map needs to have dimension 2, not %d", me, nmap->dim);
    biffAdd(NRRD, err); return 1;
  }
  entLen = nmap->axis[0].size;
  mapLen = nmap->axis[1].size;
  if (!( entLen >= 2 && mapLen >= 2 )) {
    sprintf(err, "%s: both map's axes sizes should be >= 2 (not %d,%d)",
            me, entLen, mapLen);
    biffAdd(NRRD, err); return 1;
  }
  min[0] = 1; max[0] = nmap->axis[0].size-1;
  min[1] = 0; max[1] = nmap->axis[1].size-1;
  if (nrrdCrop(nrange=nrrdNew(), nmap, min, max)) {
    sprintf(err, "%s: couldn't crop to isolate range of map", me);
    biffAdd(NRRD, err); nrrdNuke(nrange); return 1;
  }
  if (nrrdHasNonExist(nrange)) {
    sprintf(err, "%s: map has non-existent values in its range", me);
    biffAdd(NRRD, err); nrrdNuke(nrange); return 1;
  }
  nrrdNuke(nrange);
  mapLup = nrrdDLookup[nmap->type];
  if (AIR_EXISTS(mapLup(nmap->data, 0))) {
    baseI = 0;
  } else {
    baseI = 3;
    if (!( mapLen >= 5 )) {
      sprintf(err, "%s: length of map w/ non-existant locations must "
              "be >= 5 (not %d)", me, mapLen);
      biffAdd(NRRD, err); return 1;
    }
    if (!( airFP_NEG_INF == airFPClass_d(mapLup(nmap->data, 0*entLen)) &&
           airFP_QNAN    == airFPClass_d(mapLup(nmap->data, 1*entLen)) &&
           airFP_POS_INF == airFPClass_d(mapLup(nmap->data, 2*entLen)) )) {
      sprintf(err, "%s: 1st entry's position non-existant, but position "
              "of 1st three entries not -inf, NaN, and +inf", me);
      biffAdd(NRRD, err); return 1;
    }
  }
  for (i=baseI; i<mapLen; i++) {
    if (!AIR_EXISTS(mapLup(nmap->data, i*entLen))) {
      sprintf(err, "%s: entry %d has non-existant position", me, i);
      biffAdd(NRRD, err); return 1;
    }
  }
  for (i=baseI; i<mapLen-1; i++) {
    if (!( mapLup(nmap->data, i*entLen) < mapLup(nmap->data, (i+1)*entLen) )) {
      sprintf(err, "%s: map entry %d pos (%g) not < entry %d pos (%g)",
              me, i, mapLup(nmap->data, i*entLen),
              i+1, mapLup(nmap->data, (i+1)*entLen));
      biffAdd(NRRD, err); return 1;
    }
  }
  return 0;
}

/*
******** nrrd1DIrregAclCheck()
**
** returns zero only on valid accelerators for 1D irregular mappings
*/
int
nrrd1DIrregAclCheck(const Nrrd *nacl) {
  char me[]="nrrd1DIrregAclCheck", err[BIFF_STRLEN];

  if (!nacl) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdCheck(nacl)) {
    sprintf(err, "%s: ", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeUShort != nacl->type) {
    sprintf(err, "%s: type should be %s, not %s", me,
            airEnumStr(nrrdType, nrrdTypeUShort),
            airEnumStr(nrrdType, nacl->type));
    biffAdd(NRRD, err); return 1;
  }
  if (2 != nacl->dim) {
    sprintf(err, "%s: dimension should be 2, not %d", me, nacl->dim);
    biffAdd(NRRD, err); return 1;
  }
  if (!( nacl->axis[0].size == 2 && nacl->axis[1].size >= 2 )) {
    sprintf(err, "%s: sizes (" _AIR_SIZE_T_CNV "," _AIR_SIZE_T_CNV ") "
            "not (2,>=2)", me, nacl->axis[0].size, nacl->axis[1].size);
    biffAdd(NRRD, err); return 1;
  }

  return 0;
}

/*
** _nrrd1DIrregMapDomain()
**
** ALLOCATES an array of doubles storing the existant control point
** locations, and sets its length in *poslenP.  If there are the three
** points with non-existant locations, these are ignored.
**
** Assumes that nrrd1DIrregMapCheck has been called on "nmap".
*/
double *
_nrrd1DIrregMapDomain(int *posLenP, int *baseIP, const Nrrd *nmap) {
  char me[]="_nrrd1DIrregMapDomain", err[BIFF_STRLEN];
  int i, entLen, baseI, posLen;
  double *pos, (*mapLup)(const void *v, size_t I);

  mapLup = nrrdDLookup[nmap->type];
  baseI = AIR_EXISTS(mapLup(nmap->data, 0)) ? 0 : 3;
  if (baseIP) {
    *baseIP = baseI;
  }
  entLen = nmap->axis[0].size;
  posLen = nmap->axis[1].size - baseI;
  if (posLenP) {
    *posLenP = posLen;
  }
  pos = (double*)malloc(posLen * sizeof(double));
  if (!pos) {
    sprintf(err, "%s: couldn't allocate %d doubles\n", me, posLen);
    biffAdd(NRRD, err); return NULL;
  }
  for (i=0; i<posLen; i++) {
    pos[i] = mapLup(nmap->data, (baseI+i)*entLen);
  }
  return pos;
}

/*
** _nrrd1DIrregFindInterval()
**
** The hard part of doing 1D irregular mapping: given an array of
** control point locations, and a value, find which interval the value
** lies in.  The lowest and highest possible indices are given in
** "loI" and "hiI".  Results are undefined if these do not in fact
** bound the location of correct interval, or if loI > hiI, or if the
** query positon "p" is not in the domain vector "pos".  Intervals are
** identified by the integral index of the LOWER of the two control
** points spanning the interval.
**
** This imposes the same structure of half-open intervals that
** is done by airIndex().  That is, a value p is in interval i
** if pos[i] <= p < pos[i+1] for all but the last interval, and
** pos[i] <= p <= pos[i+1] for the last interval (in which case
** i == hiI)
*/
int
_nrrd1DIrregFindInterval(const double *pos, double p, int loI, int hiI) {
  int midI;

  /*
  fprintf(stderr, "##%s(%g): hi: %d/%g-%g | %d/%g-%g\n",
          "_nrrd1DIrregFindInterval", p,
          loI, pos[loI], pos[loI+1], 
          hiI, pos[hiI], pos[hiI+1]);
  */
  while (loI < hiI) {
    midI = (loI + hiI)/2;
    if ( pos[midI] <= p && ((midI <  hiI && p <  pos[midI+1]) ||
                            (midI == hiI && p <= pos[midI+1])) ) {
      /* p is between (pos[midI],pos[midI+1]): we're done */
      loI = hiI = midI;
    } else if (pos[midI] > p) {
      /* p is below interval midI: midI-1 is valid upper bound */
      hiI = midI-1;
    } else {
      /* p is above interval midI: midI+1 is valid lower bound */
      loI = midI+1;
    }
    /*
    fprintf(stderr, "##%s(%g): %d/%g-%g | %d/%g-%g | %d/%g-%g\n",
            "_nrrd1DIrregFindInterval", p,
            loI, pos[loI], pos[loI+1], 
            midI, pos[midI], pos[midI+1], 
            hiI, pos[hiI], pos[hiI+1]);
    */
  }
  return loI;
}

/*
******** nrrd1DIrregAclGenerate()
**
** Generates the "acl" that is used to speed up the action of
** nrrdApply1DIrregMap().  Basically, the domain of the map
** is quantized into "acllen" bins, and for each bin, the
** lowest and highest possible map interval is stored. This
** either obviates or speeds up the task of finding which
** interval contains a given value.
**
** Assumes that nrrd1DIrregMapCheck has been called on "nmap".
*/
int
nrrd1DIrregAclGenerate(Nrrd *nacl, const Nrrd *nmap, size_t aclLen) {
  char me[]="nrrd1DIrregAclGenerate", err[BIFF_STRLEN];
  int posLen;
  unsigned int ii;
  unsigned short *acl;
  double lo, hi, min, max, *pos;

  if (!(nacl && nmap)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (!(aclLen >= 2)) {
    sprintf(err, "%s: given acl length (" _AIR_SIZE_T_CNV 
            ") is too small", me, aclLen);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdMaybeAlloc_va(nacl, nrrdTypeUShort, 2, 
                        AIR_CAST(size_t, 2), AIR_CAST(size_t, aclLen))) {
    sprintf(err, "%s: ", me);
    biffAdd(NRRD, err); return 1;
  }
  acl = (unsigned short *)nacl->data;
  pos = _nrrd1DIrregMapDomain(&posLen, NULL, nmap);
  if (!pos) {
    sprintf(err, "%s: couldn't determine domain", me); 
    biffAdd(NRRD, err); return 1;
  }
  nacl->axis[1].min = min = pos[0];
  nacl->axis[1].max = max = pos[posLen-1];
  for (ii=0; ii<=aclLen-1; ii++) {
    lo = AIR_AFFINE(0, ii, aclLen, min, max);
    hi = AIR_AFFINE(0, ii+1, aclLen, min, max);
    acl[0 + 2*ii] = _nrrd1DIrregFindInterval(pos, lo, 0, posLen-2);
    acl[1 + 2*ii] = _nrrd1DIrregFindInterval(pos, hi, 0, posLen-2);
  }
  free(pos);

  return 0;
}

/*
******** nrrdApply1DIrregMap()
**
** Linear interpolation between irregularly spaced control points.
** Obviously, the location of the control point has to be given
** explicitly.  The map nrrd must have dimension 2, and each 
** control point is represented by a scanline along axis 0.  The
** first value is the position of the control point, and the remaining
** value(s) are linearly weighted according to the position of the
** input value among the control point locations.
**
** To allow "coloring" of non-existant values -inf, NaN, and +inf, if
** the very first value of the map (the location of the first control
** point) is non-existant, then the first three control point locations
** must be -inf, NaN, and +inf, in that order, and the information
** about these points will be used for corresponding input values.
** Doing this makes everything slower, however, because airFPClass_f()
** is called on every single value.
**
** This assumes that nrrd1DIrregMapCheck has been called on "nmap",
** and that nrrd1DIrregAclCheck has been called on "nacl" (if it is
** non-NULL).
*/
int
nrrdApply1DIrregMap(Nrrd *nout, const Nrrd *nin, const NrrdRange *_range,
                    const Nrrd *nmap, const Nrrd *nacl,
                    int typeOut, int rescale) {
  char me[]="nrrdApply1DIrregMap", err[BIFF_STRLEN];
  size_t N, I;
  int i, *acl, entLen, posLen, aclLen, mapIdx, aclIdx,
    entSize, colSize, inSize, lo, hi, baseI;
  double val, *pos, domMin, domMax, mapIdxFrac,
    (*mapLup)(const void *v, size_t I),
    (*inLoad)(const void *v), (*outInsert)(void *v, size_t I, double d);
  char *inData, *outData, *entData0, *entData1;
  NrrdRange *range;
  airArray *mop;

  if (!(nout && nmap && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
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
  if (_nrrdApply1DSetUp(nout, nin, range, nmap,
                        kindImap, typeOut, rescale, AIR_FALSE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  if (nacl && nrrd1DIrregAclCheck(nacl)) {
    sprintf(err, "%s: given acl isn't valid", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  
  if (nacl) {
    acl = (int *)nacl->data;
    aclLen = nacl->axis[1].size;
  } else {
    acl = NULL;
    aclLen = 0;
  }
  pos = _nrrd1DIrregMapDomain(&posLen, &baseI, nmap);
  if (!pos) {
    sprintf(err, "%s: couldn't determine domain", me); 
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, pos, airFree, airMopAlways);
  mapLup = nrrdDLookup[nmap->type];
  
  inData = (char *)nin->data;
  inLoad = nrrdDLoad[nin->type];
  inSize = nrrdElementSize(nin);
  mapLup = nrrdDLookup[nmap->type];
  entLen = nmap->axis[0].size;    /* entLen is really 1 + entry length */
  entSize = entLen*nrrdElementSize(nmap);
  colSize = (entLen-1)*nrrdTypeSize[typeOut];
  outData = (char *)nout->data;
  outInsert = nrrdDInsert[nout->type];
  domMin = pos[0];
  domMax = pos[posLen-1];
  
  N = nrrdElementNumber(nin);
  for (I=0;
       I<N;
       I++, inData += inSize, outData += colSize) {
    val = inLoad(inData);
    /* _VV = ( (AIR_EXISTS(val) && (21 == (int)(-val))) 
       || 22400 < I ); */
    /* if (_VV)
       fprintf(stderr, "##%s: (%d) val = % 31.15f\n", me, (int)I, val); */
    if (!AIR_EXISTS(val)) {
      /* got a non-existant value */
      if (baseI) {
        /* and we know how to deal with them */
        switch (airFPClass_d(val)) {
        case airFP_NEG_INF:
          mapIdx = 0;
          break;
        case airFP_SNAN:
        case airFP_QNAN:
          mapIdx = 1;
          break;
        case airFP_POS_INF:
          mapIdx = 2;
          break;
        default:
          mapIdx = 0;
          fprintf(stderr, "%s: PANIC: non-existant value/class %g/%d "
                  "not handled\n",
                  me, val, airFPClass_d(val));
          exit(1);
        }
        entData0 = (char*)(nmap->data) + mapIdx*entSize;
        for (i=1; i<entLen; i++) {
          outInsert(outData, i-1, mapLup(entData0, i));
        }
        continue;  /* we're done! (with this value) */
      } else {
        /* we don't know how to properly deal with this non-existant value:
           we use the first entry, and then fall through to code below */
        mapIdx = 0;
        mapIdxFrac = 0.0;
      }
    } else {
      /* we have an existant value */
      if (rescale) {
        val = AIR_AFFINE(range->min, val, range->max, domMin, domMax);
        /* if (_VV) fprintf(stderr, "   rescaled --> % 31.15f\n", val); */
      }
      val = AIR_CLAMP(domMin, val, domMax);
      if (acl) {
        aclIdx = airIndex(domMin, val, domMax, aclLen);
        lo = acl[0 + 2*aclIdx];
        hi = acl[1 + 2*aclIdx];
      } else {
        lo = 0;
        hi = posLen-2;
      }
      if (lo < hi) {
        mapIdx = _nrrd1DIrregFindInterval(pos, val, lo, hi);
      } else {
        /* acl did its job ==> lo == hi */
        mapIdx = lo;
      }
    }
    mapIdxFrac = AIR_AFFINE(pos[mapIdx], val, pos[mapIdx+1], 0.0, 1.0);
    /* if (_VV) fprintf(stderr, "##%s: val=\n% 31.15f --> "
                     "mapIdx,frac = %d,\n% 31.15f\n",
                     me, val, mapIdx, mapIdxFrac); */
    entData0 = (char*)(nmap->data) + (baseI+mapIdx)*entSize;
    entData1 = (char*)(nmap->data) + (baseI+mapIdx+1)*entSize;
    /* if (_VV) fprintf(stderr, "##%s: 2; %d/\n% 31.15f --> entLen=%d "
                     "baseI=%d -->\n",
                     me, mapIdx, mapIdxFrac, entLen, baseI); */
    for (i=1; i<entLen; i++) {
      val = ((1-mapIdxFrac)*mapLup(entData0, i) +
             mapIdxFrac*mapLup(entData1, i));
      /* if (_VV) fprintf(stderr, "% 31.15f\n", val); */
      outInsert(outData, i-1, val);
    }
    /* if (_VV) fprintf(stderr, "##%s: 3\n", me); */
  }
  airMopOkay(mop);
  return 0;
}

/*
******** nrrdApply1DSubstitution
**
** A "subst" is a substitution table, i.e. a list of pairs that
** describes what values should be substituted with what (substitution
** rules).  So, nsubst must be a scalar 2xN array.  The output is a
** copy of the input with values substituted using this table.
**
** Unlike with lookup tables and maps (irregular and regular), we
** aren't at liberty to change the dimensionality of the data (can't
** substitute a grayscale with a color).  The ability to apply
** substitutions to non-scalar data will be in a different function.
** Also unlike lookup tables and maps, the output type is the SAME as
** the input type; the output does NOT inherit the type of the
** substitution
*/
int
nrrdApply1DSubstitution(Nrrd *nout, const Nrrd *nin, const Nrrd *_nsubst) {
  char me[]="nrrdApply1DSubstitution", err[BIFF_STRLEN];
  double (*lup)(const void *, size_t);
  double (*ins)(void *, size_t, double);
  Nrrd *nsubst;
  double val, *subst;
  size_t ii, num;
  int jj, asize0, asize1, changed;
  airArray *mop;

  if (!(nout && _nsubst && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeBlock == nin->type || nrrdTypeBlock == _nsubst->type) {
    sprintf(err, "%s: input or substitution type is %s, need scalar",
            me, airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(NRRD, err); return 1;
  }
  if (2 != _nsubst->dim) {
    sprintf(err, "%s: substitution table has to be 2-D, not %d-D",
            me, _nsubst->dim);
    biffAdd(NRRD, err); return 1;
  }
  nrrdAxisInfoGet_va(_nsubst, nrrdAxisInfoSize, &asize0, &asize1);
  if (2 != asize0) {
    sprintf(err, "%s: substitution table has to be 2xN, not %dxN",
            me, asize0);
    biffAdd(NRRD, err); return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: couldn't initialize by copy to output", me);
      biffAdd(NRRD, err); return 1;
    }
  }

  mop = airMopNew();
  nsubst = nrrdNew();
  airMopAdd(mop, nsubst, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(nsubst, _nsubst, nrrdTypeDouble)) {
    sprintf(err, "%s: couldn't create double copy of substitution table", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  lup = nrrdDLookup[nout->type];
  ins = nrrdDInsert[nout->type];
  subst = (double *)nsubst->data;
  num = nrrdElementNumber(nout);
  for (ii=0; ii<num; ii++) {
    val = lup(nout->data, ii);
    changed = AIR_FALSE;
    for (jj=0; jj<asize1; jj++) {
      if (val == subst[jj*2+0]) {
        val = subst[jj*2+1];
        changed = AIR_TRUE;
      }
    }
    if (changed) {
      ins(nout->data, ii, val);
    }
  }

  airMopOkay(mop);
  return 0;
}
