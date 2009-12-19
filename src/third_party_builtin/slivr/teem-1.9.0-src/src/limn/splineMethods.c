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


#include "limn.h"

limnSplineTypeSpec *
limnSplineTypeSpecNew(int type, ...) {
  char me[]="limnSplineTypeSpecNew", err[BIFF_STRLEN];
  limnSplineTypeSpec *spec;
  va_list ap;

  if (airEnumValCheck(limnSplineType, type)) {
    sprintf(err, "%s: given type %d not a valid limnSplineType", me, type);
    biffAdd(LIMN, err); return NULL;
  }
  spec = (limnSplineTypeSpec *)calloc(1, sizeof(limnSplineTypeSpec));
  spec->type = type;
  va_start(ap, type);
  if (limnSplineTypeBC == type) {
    spec->B = va_arg(ap, double);
    spec->C = va_arg(ap, double);
  }
  va_end(ap);
  return spec;
}

limnSplineTypeSpec *
limnSplineTypeSpecNix(limnSplineTypeSpec *spec) {

  airFree(spec);
  return NULL;
}

/*
** _limnSplineTimeWarpSet
**
** implements GK's patented time warping technology
*/
int
_limnSplineTimeWarpSet(limnSpline *spline) {
  char me[]="_limnSplineTimeWarpSet", err[BIFF_STRLEN];
  double *cpt, *time, ss;
  int ii, N;

  cpt = (double*)(spline->ncpt->data);
  N = spline->ncpt->axis[2].size;
  time = spline->time;

  for (ii=0; ii<N; ii++) {
    if (!AIR_EXISTS(time[ii])) {
      sprintf(err, "%s: time[%d] doesn't exist", me, ii);
      biffAdd(LIMN, err); return 1;
    }
    if (ii && !(time[ii-1] < time[ii])) {
      sprintf(err, "%s: time[%d] = %g not < time[%d] = %g", me,
              ii-1, time[ii-1], ii, time[ii]);
      biffAdd(LIMN, err); return 1;
    }
    /* this will be used below */
    /* HEY: is there any way or reason to do any other kind of time warp? */
    cpt[1 + 3*ii] = ii;
  }

  for (ii=1; ii<N-1; ii++) {
    ss = (cpt[1+3*(ii+1)] - cpt[1+3*(ii-1)])/(time[ii+1] - time[ii-1]);
    cpt[0 + 3*ii] = ss*(time[ii] - time[ii-1]);
    cpt[2 + 3*ii] = ss*(time[ii+1] - time[ii]);
  }
  if (spline->loop) {
    ss = ((cpt[1+3*1] - cpt[1+3*0] + cpt[1+3*(N-1)] - cpt[1+3*(N-2)])
          / (time[1] - time[0] + time[N-1] - time[N-2]));
    cpt[2 + 3*0] = ss*(time[1] - time[0]);
    cpt[0 + 3*(N-1)] = ss*(time[N-1] - time[N-2]);
  } else {
    cpt[2 + 3*0] = ((cpt[1+3*1] - cpt[1+3*0])
                    * (time[1] - time[0]));
    cpt[0 + 3*(N-1)] = ((cpt[1+3*(N-1)] - cpt[1+3*(N-2)])
                        * (time[N-1] - time[N-2]));
  }
  /*
  fprintf(stderr, "s[0]=%g, post = %g; s[1]=%g pre = %g\n", 
          cpt[1 + 3*0], cpt[2 + 3*0], cpt[1 + 3*1], cpt[0 + 3*1]);
  */
  return 0;
}

/*
******** limnSplineNew
**
** constructor for limnSplines.  We take all the control point information
** here, and copy it internally, in an effort to simplify the management of 
** state.  The control point nrrd is 3-D, as explained in limn.h
**
** To confuse matters, the Time type of spline doesn't need the control
** point information in the traditional sense, but it still needs to know
** "when" the control points are.  So, the _ncpt nrrd is still needed, but
** it is only a 1-D array of times.  In this case, the internal ncpt values
** are set automatically.
**
** The benefit of this approach is that if this constructor returns
** successfully, then there is no more information or state that needs to
** be set-- the returned spline can be passed to evaluate or sample.  
** LIES LIES LIES: For BC-splines, you still have to call limnSplineBCSet,
** but that's the only exception...
*/
limnSpline *
limnSplineNew(Nrrd *_ncpt, int info, limnSplineTypeSpec *spec) {
  char me[]="limnSplineNew", err[BIFF_STRLEN];
  limnSpline *spline;
  size_t N;
  unsigned int size;
  airArray *mop;
  Nrrd *nin;
  
  if (airEnumValCheck(limnSplineInfo, info)) {
    sprintf(err, "%s: info %d not a valid limnSplineInfo", me, info);
    biffAdd(LIMN, err); return NULL;
  }
  if (nrrdCheck(_ncpt)) {
    sprintf(err, "%s: given nrrd has problems", me);
    biffMove(LIMN, err, NRRD); return NULL;
  }
  if (limnSplineTypeTimeWarp == spec->type) {
    if (!(limnSplineInfoScalar == info)) {
      sprintf(err, "%s: can only time warp scalars", me);
      biffAdd(LIMN, err); return NULL;
    }
    if (!( 1 == _ncpt->dim )) {
      sprintf(err, "%s: given nrrd has dimension %d, not 1", me, _ncpt->dim);
    }
    N = _ncpt->axis[0].size;
  } else {
    if (!( 3 == _ncpt->dim )) {
      sprintf(err, "%s: given nrrd has dimension %d, not 3", me, _ncpt->dim);
      biffAdd(LIMN, err); return NULL;
    }
    size = limnSplineInfoSize[info];
    if (!( size == _ncpt->axis[0].size && 3 == _ncpt->axis[1].size )) {
      sprintf(err, "%s: expected %ux3xN nrrd, not "
              _AIR_SIZE_T_CNV "x" _AIR_SIZE_T_CNV "xN", me,
              size, _ncpt->axis[0].size, _ncpt->axis[1].size);
      biffAdd(LIMN, err); return NULL;
    }
    N = _ncpt->axis[2].size;
  }
  if (1 == N) {
    sprintf(err, "%s: need at least two control points", me);
    biffAdd(LIMN, err); return NULL;
  }
  
  mop = airMopNew();
  if (!( spline = (limnSpline*)calloc(1, sizeof(limnSpline)) )) {
    sprintf(err, "%s: couldn't allocate new spline", me);
    biffAdd(LIMN, err); airMopError(mop); return NULL;
  }
  
  airMopAdd(mop, spline, (airMopper)limnSplineNix, airMopOnError);
  spline->time = NULL;
  spline->ncpt = NULL;
  spline->type = spec->type;
  spline->info = info;
  spline->loop = AIR_FALSE;
  spline->B = spec->B;
  spline->C = spec->C;
  nin = nrrdNew();
  airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopOnError);
  if (nrrdConvert(nin, _ncpt, nrrdTypeDouble)) {
    sprintf(err, "%s: trouble allocating internal nrrd", me);
    biffMove(LIMN, err, NRRD); airMopError(mop); return NULL;
  }
  if (limnSplineTypeTimeWarp == spec->type) {
    /* set the time array to the data of the double-converted nin,
       but the nin itself is scheduled to be nixed */
    airMopAdd(mop, nin, (airMopper)nrrdNix, airMopOnOkay);
    spline->time = (double*)(nin->data);
    /* now allocate the real control point information */
    spline->ncpt = nrrdNew();
    airMopAdd(mop, spline->ncpt, (airMopper)nrrdNuke, airMopOnError);
    if (nrrdMaybeAlloc_va(spline->ncpt, nrrdTypeDouble, 3,
                          AIR_CAST(size_t, 1),
                          AIR_CAST(size_t, 3),
                          _ncpt->axis[0].size)) {
      sprintf(err, "%s: trouble allocating real control points", me);
      biffMove(LIMN, err, NRRD); airMopError(mop); return NULL;
    }
    /* and set it all to something useful */
    if (_limnSplineTimeWarpSet(spline)) {
      sprintf(err, "%s: trouble setting time warp", me);
      biffAdd(LIMN, err); airMopError(mop); return NULL;
    }
  } else {
    /* we set the control point to the double-converted nin, and we're done */
    spline->ncpt = nin;
  }
  airMopOkay(mop);
  return spline;
}

limnSpline *
limnSplineNix(limnSpline *spline) {

  if (spline) {
    spline->ncpt = (Nrrd *)nrrdNuke(spline->ncpt);
    spline->time = (double *)airFree(spline->time);
    airFree(spline);
  }
  return NULL;
}

/*
******** limnSplineNrrdCleverFix
**
** given that the ncpt nrrd for limnSplineNew() has to be a particular
** dimension and shape, and given that convenient ways of creating nrrds
** don't always lead to such configurations, we supply some minimal 
** cleverness to bridge the gap.  As the name implies, you should be
** wary of this function.
**
** The job of this function is NOT to check the validity of a nrrd for
** a given spline.  That is up to limnSplineNew().
**
** Currently, this function is used by limnHestSpline, but it probably
** won't be used anywhere else within limn.
**
** If requested, we also take a stab at guessing limnSplineInfo.
*/
int
limnSplineNrrdCleverFix(Nrrd *nout, Nrrd *nin, int info, int type) {
  char me[]="limnSplineNrrdCleverFix", err[BIFF_STRLEN];
  ptrdiff_t min[3], max[3];
  size_t N;
  unsigned int wantSize;
  Nrrd *ntmpA, *ntmpB;
  airArray *mop;

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me); 
    biffAdd(LIMN, err); return 1;
  }
  if (airEnumValCheck(limnSplineInfo, info)
      || airEnumValCheck(limnSplineType, type)) {
    sprintf(err, "%s: invalid spline info (%d) or type (%d)", me, info, type);
    biffAdd(LIMN, err); return 1;
  }
  if (nrrdCheck(nin)) {
    sprintf(err, "%s: nrrd has problems", me);
    biffMove(LIMN, err, NRRD); return 1;
  }

  mop = airMopNew();
  airMopAdd(mop, ntmpA=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntmpB=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  wantSize = limnSplineInfoSize[info];
  switch(nin->dim) {
  case 3:
    /* we assume that things are okay */
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: trouble setting output", me);
      biffMove(LIMN, err, NRRD); airMopError(mop); return 1;
    }
    break;
  case 2:
    N = nin->axis[1].size;
    if (wantSize != nin->axis[0].size) {
      sprintf(err, "%s: expected axis[0].size %d for info %s, but got "
              _AIR_SIZE_T_CNV, 
              me, wantSize, airEnumStr(limnSplineInfo, info),
              nin->axis[0].size);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
    if (limnSplineTypeTimeWarp == type) {
      /* time-warp handled differently */
      if (nrrdAxesDelete(nout, nin, 0)) {
        sprintf(err, "%s: couldn't make data 1-D", me);
        biffMove(LIMN, err, NRRD); airMopError(mop); return 1;
      }
    } else {
      if (limnSplineTypeHasImplicitTangents[type]) {
        ELL_3V_SET(min, 0, -1, 0);
        ELL_3V_SET(max, wantSize-1, 1, N-1); 
        if (nrrdAxesInsert(ntmpA, nin, 1)
            || nrrdPad_va(nout, ntmpA, min, max, nrrdBoundaryPad, 0.0)) {
          sprintf(err, "%s: trouble with axinsert/pad", me);
          biffMove(LIMN, err, NRRD); airMopError(mop); return 1;
        }
      } else {
        /* the post- and pre- point information may be interlaced with the 
           main control point values */
        if (1 != AIR_MOD((int)N, 3)) {
          sprintf(err, "%s: axis[1].size must be 1+(multiple of 3) when using "
                  "interlaced tangent information, not " _AIR_SIZE_T_CNV,
                  me, N);
          biffAdd(LIMN, err); airMopError(mop); return 1;
        }
        ELL_2V_SET(min, 0, -1);
        ELL_2V_SET(max, wantSize-1, N);
        if (nrrdPad_va(ntmpA, nin, min, max, nrrdBoundaryPad, 0.0)
            || nrrdAxesSplit(nout, ntmpA, 1, 3, (N+2)/3)) {
          sprintf(err, "%s: trouble with pad/axsplit", me);
          biffMove(LIMN, err, NRRD); airMopError(mop); return 1;
        }
      }
    }
    break;
  case 1:
    N = nin->axis[0].size;
    if (limnSplineInfoScalar != info) {
      sprintf(err, "%s: can't have %s spline with 1-D nrrd", me,
              airEnumStr(limnSplineInfo, info));
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
    if (limnSplineTypeTimeWarp == type) {
      /* nothing fancey needed for time-warp */
      if (nrrdCopy(nout, nin)) {
        sprintf(err, "%s: trouble setting output", me);
        biffMove(LIMN, err, NRRD); airMopError(mop); return 1;
      }
    } else {
      if (limnSplineTypeHasImplicitTangents[type]) {
        ELL_3V_SET(min, 0, -1, 0);
        ELL_3V_SET(max, 0, 1, N-1); 
        if (nrrdAxesInsert(ntmpA, nin, 0)
            || nrrdAxesInsert(ntmpB, ntmpA, 0)
            || nrrdPad_va(nout, ntmpB, min, max, nrrdBoundaryPad, 0.0)) {
          sprintf(err, "%s: trouble with axinsert/axinsert/pad", me);
          biffMove(LIMN, err, NRRD); airMopError(mop); return 1;
        }
      } else {
        /* the post- and pre- point information may be interlaced with the 
           main control point values */
        if (1 != AIR_MOD((int)N, 3)) {
          sprintf(err, "%s: axis[1].size must be 1+(multiple of 3) when using "
                  "interlaced tangent information, not " _AIR_SIZE_T_CNV, 
                  me, N);
          biffAdd(LIMN, err); airMopError(mop); return 1;
        }
        ELL_2V_SET(min, 0, -1);
        ELL_2V_SET(max, 0, N+1);
        if (nrrdAxesInsert(ntmpA, nin, 0)
            || nrrdPad_va(ntmpB, ntmpA, min, max, nrrdBoundaryPad, 0.0)
            || nrrdAxesSplit(nout, ntmpB, 1, 3, (N+2)/3)) {
          sprintf(err, "%s: trouble with axinsert/pad/axsplit", me);
          biffMove(LIMN, err, NRRD); airMopError(mop); return 1;
        }
      }
    }
    break;
  default:
    sprintf(err, "%s: input nrrd dim %d baffling", me, nin->dim);
    biffAdd(LIMN, err); return 1;
  }
  if (nrrdCheck(nout)) {
    sprintf(err, "%s: oops: didn't create valid output", me);
    biffMove(LIMN, err, NRRD); airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

limnSpline *
limnSplineCleverNew(Nrrd *ncpt, int info, limnSplineTypeSpec *spec) {
  char me[]="limnSplineCleverNew", err[BIFF_STRLEN];
  limnSpline *spline;
  Nrrd *ntmp;
  airArray *mop;

  if (!( ncpt && spec )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return NULL;
  }
  mop = airMopNew();
  airMopAdd(mop, ntmp = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (limnSplineNrrdCleverFix(ntmp, ncpt, info, spec->type)) {
    sprintf(err, "%s: couldn't fix up given control point nrrd", me);
    biffAdd(LIMN, err); airMopError(mop); return NULL;
  }
  if (!( spline = limnSplineNew(ntmp, info, spec) )) {
    sprintf(err, "%s: couldn't create spline", me);
    biffAdd(LIMN, err); airMopError(mop); return NULL;
  }

  airMopOkay(mop);
  return spline;
}

int
limnSplineUpdate(limnSpline *spline, Nrrd *_ncpt) {
  char me[]="limnSplineUpdate", err[BIFF_STRLEN];
  Nrrd *ntmp;

  if (!(spline && _ncpt)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  if (nrrdCheck(_ncpt)) {
    sprintf(err, "%s: given nrrd has problems", me);
    biffMove(LIMN, err, NRRD); return 1;
  }
  if (limnSplineTypeTimeWarp == spline->type) {
    if (!( 1 == _ncpt->dim )) {
      sprintf(err, "%s: given nrrd has dimension %d, not 1", me, _ncpt->dim);
      biffAdd(LIMN, err); return 1;
    }
    if (!( spline->ncpt->axis[2].size == _ncpt->axis[0].size )) {
      sprintf(err, "%s: have " _AIR_SIZE_T_CNV " time points, but got "
              _AIR_SIZE_T_CNV, me,
              spline->ncpt->axis[2].size, _ncpt->axis[0].size);
      biffAdd(LIMN, err); return 1;
    }
  } else {
    if (!( nrrdSameSize(spline->ncpt, _ncpt, AIR_TRUE) )) {
      sprintf(err, "%s: given ncpt doesn't match original one", me);
      biffMove(LIMN, err, NRRD); return 1;
    }
  }

  if (limnSplineTypeTimeWarp == spline->type) {
    ntmp = nrrdNew();
    if (nrrdWrap_va(ntmp, spline->time, nrrdTypeDouble, 1,
                    _ncpt->axis[0].size)
        || nrrdConvert(ntmp, _ncpt, nrrdTypeDouble)) {
      sprintf(err, "%s: trouble copying info", me);
      biffMove(LIMN, err, NRRD); nrrdNix(ntmp); return 1;
    }
    if (_limnSplineTimeWarpSet(spline)) {
      sprintf(err, "%s: trouble setting time warp", me);
      biffAdd(LIMN, err); nrrdNix(ntmp); return 1;
    }
    nrrdNix(ntmp); 
  } else {
    if (nrrdConvert(spline->ncpt, _ncpt, nrrdTypeDouble)) {
      sprintf(err, "%s: trouble converting to internal nrrd", me);
      biffMove(LIMN, err, NRRD); return 1;
    }
  }  

  return 0;
}
