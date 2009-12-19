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

enum {
  kindLut=0,
  kindRmap=1
};

/*
** _nrrdApply2DSetUp()
**
** some error checking and initializing needed for 2D LUTS and regular
** maps.  The intent is that if this succeeds, then there is no need
** for any further error checking.
**
** The only thing this function DOES is allocate the output nrrd, and
** set meta information.  The rest is just error checking.
**
** The given NrrdRange has to be fleshed out by the caller: it can't
** be NULL, and both range->min and range->max must exist.
*/
int
_nrrdApply2DSetUp(Nrrd *nout, const Nrrd *nin,
                  const NrrdRange *range0, const NrrdRange *range1,
                  const Nrrd *nmap, int kind, int typeOut,
                  int rescale0, int rescale1) {
  char me[]="_nrrdApply2DSetUp", err[BIFF_STRLEN], *mapcnt;
  char nounStr[][AIR_STRLEN_SMALL]={"2D lut",
                                    "2D regular map"};
  char verbStr[][AIR_STRLEN_SMALL]={"lut2",
                                    "rmap2"};
  int mapAxis, copyMapAxis0=AIR_FALSE, axisMap[NRRD_DIM_MAX];
  unsigned int dim, entLen;
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
  if (!(2 == nin->axis[0].size)) {
    sprintf(err, "%s: input axis[0] must have size 2 (not " 
            _AIR_SIZE_T_CNV ")", me, nin->axis[0].size);
    biffAdd(NRRD, err); return 1;
  }
  if (!(nin->dim > 1)) {
    sprintf(err, "%s: input dimension must be > 1 (not %u)", me, nin->dim);
    biffAdd(NRRD, err); return 1;
  }
  if (rescale0 && !(range0
                    && AIR_EXISTS(range0->min) 
                    && AIR_EXISTS(range0->max))) {
    sprintf(err, "%s: want axis 0 rescaling but didn't get range, or "
            "not both range->{min,max} exist", me);
    biffAdd(NRRD, err); return 1;
  }
  if (rescale1 && !(range1
                    && AIR_EXISTS(range1->min) 
                    && AIR_EXISTS(range1->max))) {
    sprintf(err, "%s: want axis 1 rescaling but didn't get range, or "
            "not both range->{min,max} exist", me);
    biffAdd(NRRD, err); return 1;
  }
  mapAxis = nmap->dim - 2;
  if (!(0 == mapAxis || 1 == mapAxis)) {
    sprintf(err, "%s: dimension of %s should be 2 or 3, not %d", 
            me, nounStr[kind], nmap->dim);
    biffAdd(NRRD, err); return 1;
  }
  copyMapAxis0 = (1 == mapAxis);
  domMin = _nrrdApplyDomainMin(nmap, AIR_FALSE, mapAxis);
  domMax = _nrrdApplyDomainMax(nmap, AIR_FALSE, mapAxis);
  if (!( domMin < domMax )) {
    sprintf(err, "%s: (axis %d) domain min (%g) not less than max (%g)", me,
            mapAxis, domMin, domMax);
    biffAdd(NRRD, err); return 1;
  }
  domMin = _nrrdApplyDomainMin(nmap, AIR_FALSE, mapAxis+1);
  domMax = _nrrdApplyDomainMax(nmap, AIR_FALSE, mapAxis+1);
  if (!( domMin < domMax )) {
    sprintf(err, "%s: (axis %d) domain min (%g) not less than max (%g)", me,
            mapAxis+1, domMin, domMax);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdHasNonExist(nmap)) {
    sprintf(err, "%s: %s nrrd has non-existent values",
            me, nounStr[kind]);
    biffAdd(NRRD, err); return 1;
  }
  entLen = mapAxis ? nmap->axis[0].size : 1;
  if (mapAxis + nin->dim - 1 > NRRD_DIM_MAX) {
    sprintf(err, "%s: input nrrd dim %d through non-scalar %s exceeds "
            "NRRD_DIM_MAX %d",
            me, nin->dim, nounStr[kind], NRRD_DIM_MAX);
    biffAdd(NRRD, err); return 1;
  }
  if (mapAxis) {
    size[0] = entLen;
    axisMap[0] = -1;
  }
  for (dim=1; dim<nin->dim; dim++) {
    size[dim-1+mapAxis] = nin->axis[dim].size;
    axisMap[dim-1+mapAxis] = dim;
  }
  /*
  fprintf(stderr, "##%s: pre maybe alloc: nout->data = %p\n", me, nout->data);
  for (dim=0; dim<mapAxis + nin->dim - 1; dim++) {
    fprintf(stderr, "    size[%d] = %d\n", dim, (int)size[dim]);
  }
  fprintf(stderr, "   nout->dim = %u; nout->type = %d = %s; sizes = %u,%u\n", 
          nout->dim, nout->type,
          airEnumStr(nrrdType, nout->type),
          AIR_CAST(unsigned int, nout->axis[0].size),
          AIR_CAST(unsigned int, nout->axis[1].size));
  fprintf(stderr, "   typeOut = %d = %s\n", typeOut,
          airEnumStr(nrrdType, typeOut));
  */
  if (nrrdMaybeAlloc_nva(nout, typeOut, nin->dim - 1 + mapAxis, size)) {
    sprintf(err, "%s: couldn't allocate output nrrd", me);
    biffAdd(NRRD, err); return 1;
  }
  /*
  fprintf(stderr, "   nout->dim = %d; nout->type = %d = %s\n",
          nout->dim, nout->type,
          airEnumStr(nrrdType, nout->type));
  for (dim=0; dim<nout->dim; dim++) {
    fprintf(stderr, "    size[%d] = %d\n", dim, (int)nout->axis[dim].size);
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
  if (nrrdContentSet_va(nout, verbStr[kind], nin, "%s", mapcnt)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); free(mapcnt); return 1;
  }
  free(mapcnt); 
  nrrdBasicInfoInit(nout, (NRRD_BASIC_INFO_DATA_BIT
                           | NRRD_BASIC_INFO_TYPE_BIT
                           | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                           | NRRD_BASIC_INFO_DIMENSION_BIT
                           | NRRD_BASIC_INFO_CONTENT_BIT));
  return 0;
}

/*
** _nrrdApply2DLutOrRegMap()
**
** the guts of nrrdApply2DLut and nrrdApply2DRegMap
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
_nrrdApply2DLutOrRegMap(Nrrd *nout, const Nrrd *nin,
                        const NrrdRange *range0, const NrrdRange *range1,
                        const Nrrd *nmap, int ramps,
                        int rescale0, int rescale1) {
  char me[]="_nrrdApply2DLutOrRegMap";
  char *inData, *outData, *mapData, *entData;
  size_t N, I;
  double (*inLoad)(const void *v), (*mapLup)(const void *v, size_t I),
    (*outInsert)(void *v, size_t I, double d),
    val0, val1, domMin0, domMax0, domMin1, domMax1;
  unsigned int i, mapAxis, mapLen0, mapLen1, mapIdx0, mapIdx1,
    entSize, entLen, inSize, outSize;

  mapAxis = nmap->dim - 2;             /* axis of nmap containing entries */
  mapData = (char *)nmap->data;        /* map data, as char* */
                                       /* low end of map domain */
  domMin0 = _nrrdApplyDomainMin(nmap, ramps, mapAxis + 0);
  domMin1 = _nrrdApplyDomainMin(nmap, ramps, mapAxis + 1);
                                       /* high end of map domain */
  domMax0 = _nrrdApplyDomainMax(nmap, ramps, mapAxis + 0);
  domMax1 = _nrrdApplyDomainMax(nmap, ramps, mapAxis + 1);
  mapLen0 = nmap->axis[mapAxis+0].size;   /* number of entries in map axis 0 */
  mapLen1 = nmap->axis[mapAxis+1].size;   /* number of entries in map axis 1 */
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

  /*
  fprintf(stderr, "!%s: entLen = %u, mapLen = %u,%u\n", me,
          entLen, mapLen0, mapLen1);
  */

  N = nrrdElementNumber(nin)/2;       /* number of value pairs to be mapped */
  /* _VV = 1; */
  if (ramps) {
    fprintf(stderr, "%s: PANIC: unimplemented\n", me);
    exit(1);
  } else {
    /* lookup table */
    for (I=0; I<N; I++) {
      val0 = inLoad(inData + 0*inSize);
      val1 = inLoad(inData + 1*inSize);
      if (rescale0) {
        val0 = AIR_AFFINE(range0->min, val0, range0->max, domMin0, domMax0);
      }
      if (rescale1) {
        val1 = AIR_AFFINE(range1->min, val1, range1->max, domMin1, domMax1);
      }
      if (AIR_EXISTS(val0) && AIR_EXISTS(val1)) {
        mapIdx0 = airIndexClamp(domMin0, val0, domMax0, mapLen0);
        mapIdx1 = airIndexClamp(domMin1, val1, domMax1, mapLen1);
        entData = mapData + entSize*(mapIdx0 + mapLen0*mapIdx1);
        for (i=0; i<entLen; i++) {
          outInsert(outData, i, mapLup(entData, i));
        }
      } else {
        /* copy non-existant values from input to output */
        for (i=0; i<entLen; i++) {
          outInsert(outData, i, val0 + val1);  /* HEY this is weird */
        }
      }
      inData += 2*inSize;
      outData += outSize;
    }
  }

  return 0;
}

/*
******** nrrdApply2DLut
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
nrrdApply2DLut(Nrrd *nout, const Nrrd *nin, unsigned int domainAxis,
               const NrrdRange *_range0, const NrrdRange *_range1,
               const Nrrd *nlut,
               int typeOut, int rescale0, int rescale1) {
  char me[]="nrrdApply2DLut", err[BIFF_STRLEN];
  NrrdRange *range0, *range1;
  airArray *mop;
  Nrrd *nin0, *nin1;
  
  if (!(nout && nlut && nin)) {
    sprintf(err, "%s: got NULL pointer (%p,%p,%p)", me,
            AIR_CAST(void*,nout), AIR_CAST(void*,nlut), AIR_CAST(void*,nin));
    biffAdd(NRRD, err); return 1;
  }
  if (0 != domainAxis) {
    sprintf(err, "%s: sorry, domainAxis must currently be 0 (not %u)", me,
            domainAxis);
    biffAdd(NRRD, err); return 1;
  }
  mop = airMopNew();
  if (_range0) {
    range0 = nrrdRangeCopy(_range0);
    airMopAdd(mop, nin0 = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    if (nrrdSlice(nin0, nin, 0, 0)) {
      sprintf(err, "%s: trouble learning range 0", me);
      biffAdd(NRRD, err); airMopError(mop); return 1;
    }
    nrrdRangeSafeSet(range0, nin0, nrrdBlind8BitRangeState);
  } else {
    range0 = nrrdRangeNewSet(nin, nrrdBlind8BitRangeState);
  }
  if (_range1) {
    range1 = nrrdRangeCopy(_range1);
    airMopAdd(mop, nin1 = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    if (nrrdSlice(nin1, nin, 0, 1)) {
      sprintf(err, "%s: trouble learning range 1", me);
      biffAdd(NRRD, err); airMopError(mop); return 1;
    }
    nrrdRangeSafeSet(range1, nin1, nrrdBlind8BitRangeState);
  } else {
    range1 = nrrdRangeNewSet(nin, nrrdBlind8BitRangeState);
  }
  airMopAdd(mop, range0, (airMopper)nrrdRangeNix, airMopAlways);
  airMopAdd(mop, range1, (airMopper)nrrdRangeNix, airMopAlways);
  if (_nrrdApply2DSetUp(nout, nin, range0, range1,
                        nlut, kindLut, typeOut, rescale0, rescale1)
      || _nrrdApply2DLutOrRegMap(nout, nin, range0, range1,
                                 nlut, AIR_FALSE, rescale0, rescale1)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

/*
******** nrrdApply2DRegMap
**

no, this doesn't actually exist yet....

int
nrrdApply2DRegMap(Nrrd *nout, const Nrrd *nin,
                  const NrrdRange *_range0, const NrrdRange *_range1,
                  const Nrrd *nmap, int typeOut, int rescale) {
  char me[]="nrrdApply2DRegMap", err[BIFF_STRLEN];
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
  if (_nrrdApply2DSetUp(nout, nin, range, nmap, kindRmap, typeOut,
                        rescale, AIR_FALSE)
      || _nrrdApply2DLutOrRegMap(nout, nin, range, nmap, AIR_TRUE,
                                 rescale, AIR_FALSE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

*/

