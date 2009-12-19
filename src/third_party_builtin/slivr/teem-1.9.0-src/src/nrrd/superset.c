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
******** nrrdSplice()
**
** (opposite of nrrdSlice): replaces one slice of a nrrd with
** another nrrd.  Will allocate memory for output only if nout != nin.
*/
int
nrrdSplice(Nrrd *nout, const Nrrd *nin, const Nrrd *nslice,
           unsigned int axis, size_t pos) {
  char me[]="nrrdSplice", func[]="splice", err[BIFF_STRLEN];
  size_t 
    I, 
    rowLen,                  /* length of segment */
    colStep,                 /* distance between start of each segment */
    colLen;                  /* number of periods */
  unsigned int ai;
  char *src, *dest, *sliceCont;

  if (!(nin && nout && nslice)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nslice) {
    sprintf(err, "%s: nout==nslice disallowed", me);
    biffAdd(NRRD, err); return 1;
  }

  /* check that desired slice location is legit */
  if (!( axis < nin->dim )) {
    sprintf(err, "%s: slice axis %d out of bounds (0 to %d)", 
            me, axis, nin->dim-1);
    biffAdd(NRRD, err); return 1;
  }
  if (!( pos < nin->axis[axis].size )) {
    sprintf(err, "%s: position " _AIR_SIZE_T_CNV 
            " out of bounds (0 to " _AIR_SIZE_T_CNV ")", 
            me, pos, nin->axis[axis].size-1);
    biffAdd(NRRD, err); return 1;
  }

  /* check that slice will fit in nin */
  if (nrrdCheck(nslice) || nrrdCheck(nin)) {
    sprintf(err, "%s: input or slice not valid nrrd", me);
    biffAdd(NRRD, err); return 1;
  }
  if (!( nin->dim-1 == nslice->dim )) {
    sprintf(err, "%s: dim of slice (%d) not one less than dim of input (%d)",
            me, nslice->dim, nin->dim);
    biffAdd(NRRD, err); return 1;
  }
  if (!( nin->type == nslice->type )) {
    sprintf(err, "%s: type of slice (%s) != type of input (%s)",
            me, airEnumStr(nrrdType, nslice->type),
            airEnumStr(nrrdType, nin->type));
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    if (!( nin->blockSize == nslice->blockSize )) {
      sprintf(err, "%s: input's blockSize (" _AIR_SIZE_T_CNV 
              ") != subvolume's blockSize (" _AIR_SIZE_T_CNV ")",
              me, nin->blockSize, nslice->blockSize);
      biffAdd(NRRD, err); return 1;
    }
  }
  for (ai=0; ai<nslice->dim; ai++) {
    if (!( nin->axis[ai + (ai >= axis)].size == nslice->axis[ai].size )) {
      sprintf(err, "%s: input's axis %d size (" _AIR_SIZE_T_CNV
              ") != slices axis %d size (" _AIR_SIZE_T_CNV ")",
              me, ai + (ai >= axis),
              nin->axis[ai + (ai >= axis)].size, ai, 
              nslice->axis[ai].size);
      biffAdd(NRRD, err); return 1;
    }
  }

  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s:", me);
      biffAdd(NRRD, err); return 1;
    }
  } 
  /* else we're going to splice in place */

  /* the following was copied from nrrdSlice() */
  /* set up control variables */
  rowLen = colLen = 1;
  for (ai=0; ai<nin->dim; ai++) {
    if (ai < axis) {
      rowLen *= nin->axis[ai].size;
    } else if (ai > axis) {
      colLen *= nin->axis[ai].size;
    }
  }
  rowLen *= nrrdElementSize(nin);
  colStep = rowLen*nin->axis[axis].size;

  /* the skinny */
  src = (char *)nout->data;    /* switched src,dest from nrrdSlice() */
  dest = (char *)nslice->data;
  src += rowLen*pos;
  for (I=0; I<colLen; I++) {
    /* HEY: replace with AIR_MEMCPY() or similar, when applicable */
    memcpy(src, dest, rowLen);  /* switched src,dest from nrrdSlice() */
    src += colStep;
    dest += rowLen;
  }
  
  sliceCont = _nrrdContentGet(nslice);
  if (nrrdContentSet_va(nout, func, nin, "%s,%d," _AIR_SIZE_T_CNV, 
                        sliceCont, axis, pos)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); free(sliceCont); return 1;
  }
  free(sliceCont);
  /* basic info copied by nrrdCopy above */

  return 0;
}

/*
******** nrrdInset()
**
** (opposite of nrrdCrop()) replace some sub-volume inside a nrrd with
** another given nrrd.
**
*/
int
nrrdInset(Nrrd *nout, const Nrrd *nin, const Nrrd *nsub, const size_t *min) {
  char me[]="nrrdInset", func[] = "inset", err[BIFF_STRLEN],
    buff1[NRRD_DIM_MAX*30], buff2[AIR_STRLEN_SMALL];
  unsigned int ai;
  size_t I,
    lineSize,                /* #bytes in one scanline to be copied */
    typeSize,                /* size of data type */
    cIn[NRRD_DIM_MAX],       /* coords for line start, in input */
    cOut[NRRD_DIM_MAX],      /* coords for line start, in output */
    szIn[NRRD_DIM_MAX],
    szOut[NRRD_DIM_MAX],
    idxIn, idxOut,           /* linear indices for input and output */
    numLines;                /* number of scanlines in output nrrd */
  char *dataIn, *dataOut, *subCont;

  /* errors */
  if (!(nout && nin && nsub && min)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nsub) {
    sprintf(err, "%s: nout==nsub disallowed", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdCheck(nin)) {
    sprintf(err, "%s: input not valid nrrd", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdCheck(nsub)) {
    sprintf(err, "%s: subvolume not valid nrrd", me);
    biffAdd(NRRD, err); return 1;
  }
  if (!( nin->dim == nsub->dim )) {
    sprintf(err, "%s: input's dim (%d) != subvolume's dim (%d)",
            me, nin->dim, nsub->dim);
    biffAdd(NRRD, err); return 1;
  }
  if (!( nin->type == nsub->type )) {
    sprintf(err, "%s: input's type (%s) != subvolume's type (%s)", me,
            airEnumStr(nrrdType, nin->type),
            airEnumStr(nrrdType, nsub->type));
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    if (!( nin->blockSize == nsub->blockSize )) {
      sprintf(err, "%s: input's blockSize (" _AIR_SIZE_T_CNV
              ") != subvolume's blockSize (" _AIR_SIZE_T_CNV ")",
              me, nin->blockSize, nsub->blockSize);
      biffAdd(NRRD, err); return 1;
    }
  }
  for (ai=0; ai<nin->dim; ai++) {
    if (!( min[ai] + nsub->axis[ai].size - 1 <= nin->axis[ai].size - 1)) {
      sprintf(err, "%s: axis %d range of inset indices [" _AIR_SIZE_T_CNV 
              "," _AIR_SIZE_T_CNV  "] not within "
              "input indices [0," _AIR_SIZE_T_CNV "]", me, ai,
              min[ai], min[ai] + nsub->axis[ai].size - 1,
              nin->axis[ai].size - 1);
      biffAdd(NRRD, err); return 1;
    }
  }
  
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s:", me);
      biffAdd(NRRD, err); return 1;
    }
  } 
  /* else we're going to inset in place */

  /* WARNING: following code copied/modified from nrrdCrop(),
     so the meanings of "in"/"out", "src"/"dest" are all messed up */

  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, szIn);
  nrrdAxisInfoGet_nva(nsub, nrrdAxisInfoSize, szOut);
  numLines = 1;
  for (ai=1; ai<nin->dim; ai++) {
    numLines *= szOut[ai];
  }
  lineSize = szOut[0]*nrrdElementSize(nin);
  
  /* the skinny */
  typeSize = nrrdElementSize(nin);
  dataIn = (char *)nout->data;
  dataOut = (char *)nsub->data;
  for (ai=0; ai<NRRD_DIM_MAX; ai++) {
    cOut[ai] = 0;
  }
  for (I=0; I<numLines; I++) {
    for (ai=0; ai<nin->dim; ai++) {
      cIn[ai] = cOut[ai] + min[ai];
    }
    NRRD_INDEX_GEN(idxOut, cOut, szOut, nin->dim);
    NRRD_INDEX_GEN(idxIn, cIn, szIn, nin->dim);
    memcpy(dataIn + idxIn*typeSize, dataOut + idxOut*typeSize, lineSize);
    /* the lowest coordinate in cOut[] will stay zero, since we are 
       copying one (1-D) scanline at a time */
    NRRD_COORD_INCR(cOut, szOut, nin->dim, 1);
  }

  /* HEY: before Teem version 2.0 figure out nrrdKind stuff here */
  
  strcpy(buff1, "[");
  for (ai=0; ai<nin->dim; ai++) {
    sprintf(buff2, "%s" _AIR_SIZE_T_CNV, (ai ? "," : ""), min[ai]);
    strcat(buff1, buff2);
  }
  strcat(buff1, "]");
  subCont = _nrrdContentGet(nsub);
  if (nrrdContentSet_va(nout, func, nin, "%s,%s", subCont, buff1)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); free(subCont); return 1;
  }
  free(subCont); 
  /* basic info copied by nrrdCopy above */

  return 0;
}

/*
******** nrrdPad_va()
**
** strictly for padding
*/
int
nrrdPad_va(Nrrd *nout, const Nrrd *nin,
           const ptrdiff_t *min, const ptrdiff_t *max, int boundary, ...) {
  char me[]="nrrdPad_va", func[]="pad", err[BIFF_STRLEN],
    buff1[NRRD_DIM_MAX*30], buff2[AIR_STRLEN_MED];
  double padValue=AIR_NAN;
  int outside;
  unsigned int ai;
  ptrdiff_t
    cIn[NRRD_DIM_MAX];       /* coords for line start, in input */
  size_t
    typeSize,
    idxIn, idxOut,           /* linear indices for input and output */
    numOut,                  /* number of elements in output nrrd */
    szIn[NRRD_DIM_MAX],
    szOut[NRRD_DIM_MAX],
    cOut[NRRD_DIM_MAX];      /* coords for line start, in output */
  va_list ap;
  char *dataIn, *dataOut;
  
  if (!(nout && nin && min && max)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nin) {
    sprintf(err, "%s: nout==nin disallowed", me);
    biffAdd(NRRD, err); return 1;
  }
  if (!AIR_IN_OP(nrrdBoundaryUnknown, boundary, nrrdBoundaryLast)) {
    sprintf(err, "%s: boundary behavior %d invalid", me, boundary);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdBoundaryWeight == boundary) {
    sprintf(err, "%s: boundary strategy %s not applicable here", me,
            airEnumStr(nrrdBoundary, boundary));
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeBlock == nin->type && nrrdBoundaryPad == boundary) {
    sprintf(err, "%s: with nrrd type %s, boundary %s not valid", me,
            airEnumStr(nrrdType, nrrdTypeBlock),
            airEnumStr(nrrdBoundary, nrrdBoundaryPad));
    biffAdd(NRRD, err); return 1;
  }
  va_start(ap, boundary);
  if (nrrdBoundaryPad == boundary) {
    padValue = va_arg(ap, double);
  }
  va_end(ap);
  switch(boundary) {
  case nrrdBoundaryPad:
  case nrrdBoundaryBleed:
  case nrrdBoundaryWrap:
    break;
  default:
    fprintf(stderr, "%s: PANIC: boundary %d unimplemented\n", 
            me, boundary); exit(1); break;
  }
  /*
  printf("!%s: boundary = %d, padValue = %g\n", me, boundary, padValue);
  */

  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, szIn);
  for (ai=0; ai<nin->dim; ai++) {
    if (!( min[ai] <= 0 )) {
      sprintf(err, "%s: axis %d min (" _AIR_SIZE_T_CNV ") not <= 0",
              me, ai, (size_t)min[ai]);
      biffAdd(NRRD, err); return 1;
    }
    if (!( (size_t)max[ai] >= szIn[ai]-1)) {
      sprintf(err, "%s: axis %d max (" _AIR_SIZE_T_CNV
              ") not >= size-1 (" _AIR_SIZE_T_CNV ")", 
              me, ai, (size_t)max[ai], szIn[ai]-1);
      biffAdd(NRRD, err); return 1;
    }
  }
  /* this shouldn't actually be necessary .. */
  if (!nrrdElementSize(nin)) {
    sprintf(err, "%s: nrrd reports zero element size!", me);
    biffAdd(NRRD, err); return 1;
  }

  /* allocate */
  numOut = 1;
  for (ai=0; ai<nin->dim; ai++) {
    numOut *= (szOut[ai] = -min[ai] + max[ai] + 1);
  }
  nout->blockSize = nin->blockSize;
  if (nrrdMaybeAlloc_nva(nout, nin->type, nin->dim, szOut)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  
  /* the skinny */
  typeSize = nrrdElementSize(nin);
  dataIn = (char *)nin->data;
  dataOut = (char *)nout->data;
  for (ai=0; ai<NRRD_DIM_MAX; ai++) {
    cOut[ai] = 0;
  }
  for (idxOut=0; idxOut<numOut; idxOut++) {
    outside = 0;
    for (ai=0; ai<nin->dim; ai++) {
      cIn[ai] = cOut[ai] + min[ai];
      switch(boundary) {
      case nrrdBoundaryPad:
      case nrrdBoundaryBleed:
        if (!AIR_IN_CL(0, cIn[ai], (ptrdiff_t)szIn[ai]-1)) {
          cIn[ai] = AIR_CLAMP(0, cIn[ai], (ptrdiff_t)szIn[ai]-1);
          outside = 1;
        }
        break;
      case nrrdBoundaryWrap:
        if (!AIR_IN_CL(0, cIn[ai], (ptrdiff_t)szIn[ai]-1)) {
          cIn[ai] = AIR_MOD(cIn[ai], (ptrdiff_t)szIn[ai]);
          outside = 1;
        }
        break;
      }
    }
    NRRD_INDEX_GEN(idxIn, cIn, szIn, nin->dim);
    if (!outside) {
      /* the cIn coords are within the input nrrd: do memcpy() of whole
         1-D scanline, then artificially bump for-loop to the end of
         the scanline */
      memcpy(dataOut + idxOut*typeSize, dataIn + idxIn*typeSize,
             szIn[0]*typeSize);
      idxOut += nin->axis[0].size-1;
      cOut[0] += nin->axis[0].size-1;
    } else {
      /* we copy only a single value */
      if (nrrdBoundaryPad == boundary) {
        nrrdDInsert[nout->type](dataOut, idxOut, padValue);
      } else {
        memcpy(dataOut + idxOut*typeSize, dataIn + idxIn*typeSize, typeSize);
      }
    }
    NRRD_COORD_INCR(cOut, szOut, nin->dim, 0);
  }
  if (nrrdAxisInfoCopy(nout, nin, NULL, (NRRD_AXIS_INFO_SIZE_BIT |
                                         NRRD_AXIS_INFO_MIN_BIT |
                                         NRRD_AXIS_INFO_MAX_BIT ))) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  for (ai=0; ai<nin->dim; ai++) {
    nrrdAxisInfoPosRange(&(nout->axis[ai].min), &(nout->axis[ai].max),
                         nin, ai, min[ai], max[ai]);
    nout->axis[ai].kind = _nrrdKindAltered(nin->axis[ai].kind, AIR_FALSE);
  }
  strcpy(buff1, "");
  for (ai=0; ai<nin->dim; ai++) {
    sprintf(buff2, "%s[" _AIR_PTRDIFF_T_CNV "," _AIR_PTRDIFF_T_CNV "]", 
            (ai ? "x" : ""), min[ai], max[ai]);
    strcat(buff1, buff2);
  }
  if (nrrdBoundaryPad == boundary) {
    sprintf(buff2, "%s(%g)", airEnumStr(nrrdBoundary, nrrdBoundaryPad), 
            padValue);
  } else {
    strcpy(buff2, airEnumStr(nrrdBoundary, boundary));
  }
  if (nrrdContentSet_va(nout, func, nin, "%s,%s", buff1, buff2)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
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
    biffAdd(NRRD, err); return 1;
  }
  /* copy origin, then shift it along the spatial axes */
  _nrrdSpaceVecCopy(nout->spaceOrigin, nin->spaceOrigin);
  for (ai=0; ai<nin->dim; ai++) {
    if (AIR_EXISTS(nin->axis[ai].spaceDirection[0])) {
      _nrrdSpaceVecScaleAdd2(nout->spaceOrigin,
                             1.0, nout->spaceOrigin,
                             min[ai], nin->axis[ai].spaceDirection);
    }
  }

  return 0;
}

/*
******** nrrdPad_nva()
**
** unlike other {X_va,X_nva} pairs, nrrdPad_nva() is a wrapper around
** nrrdPad_va() instead of the other way around.
*/
int
nrrdPad_nva(Nrrd *nout, const Nrrd *nin,
            const ptrdiff_t *min, const ptrdiff_t *max,
            int boundary, double padValue) {
  char me[]="nrrdPad_nva", err[BIFF_STRLEN];
  int E;

  if (!AIR_IN_OP(nrrdBoundaryUnknown, boundary, nrrdBoundaryLast)) {
    sprintf(err, "%s: boundary behavior %d invalid", me, boundary);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdBoundaryPad == boundary) {
    E = nrrdPad_va(nout, nin, min, max, boundary, padValue);
  } else {
    E = nrrdPad_va(nout, nin, min, max, boundary);
  }
  if (E) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  
  return 0;
}

/*
******** nrrdSimplePad_va()
**
** pads by a given amount on top and bottom of EVERY axis
*/
int
nrrdSimplePad_va(Nrrd *nout, const Nrrd *nin, unsigned int pad,
                 int boundary, ...) {
  char me[]="nrrdSimplePad_va", err[BIFF_STRLEN];
  unsigned ai;
  int ret;
  ptrdiff_t min[NRRD_DIM_MAX], max[NRRD_DIM_MAX];
  double padValue;
  va_list ap;

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  for (ai=0; ai<nin->dim; ai++) {
    min[ai] = -AIR_CAST(ptrdiff_t, pad);
    max[ai] = nin->axis[ai].size-1 + pad;
  }
  va_start(ap, boundary);
  if (nrrdBoundaryPad == boundary) {
    padValue = va_arg(ap, double);
    ret = nrrdPad_va(nout, nin, min, max, boundary, padValue);
  } else {
    ret = nrrdPad_va(nout, nin, min, max, boundary);
  }
  va_end(ap);
  if (ret) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  return 0;
}

/*
******** nrrdSimplePad_nva()
**
** unlike other {X_va,X_nva} pairs, nrrdSimplePad_nva() is a wrapper
** around nrrdSimplePad_va() instead of the other way around.
*/
int
nrrdSimplePad_nva(Nrrd *nout, const Nrrd *nin, unsigned int pad,
                  int boundary, double padValue) {
  char me[]="nrrdSimplePad_nva", err[BIFF_STRLEN];
  int E;

  if (!AIR_IN_OP(nrrdBoundaryUnknown, boundary, nrrdBoundaryLast)) {
    sprintf(err, "%s: boundary behavior %d invalid", me, boundary);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdBoundaryPad == boundary) {
    E = nrrdSimplePad_va(nout, nin, pad, boundary, padValue);
  } else {
    E = nrrdSimplePad_va(nout, nin, pad, boundary);
  }
  if (E) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }

  return 0;
}
