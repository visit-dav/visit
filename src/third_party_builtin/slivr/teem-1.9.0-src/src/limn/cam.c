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

/*
******** limnCameraUpdate()
**
** sets in cam: W2V, V2W, U, V, N, vspNeer, vspFaar, vspDist
** and, if fov and aspect are set, this also sets uRange and vRange
**
** This does use biff to describe problems with camera settings
*/
int
limnCameraUpdate(limnCamera *cam) {
  char me[] = "limnCameraUpdate", err[BIFF_STRLEN];
  double len, bb[4], uu[4], vv[4], nn[4], TT[16], RR[16];

  if (!cam) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }

  ELL_4V_SET(uu, 0, 0, 0, 0);
  ELL_4V_SET(vv, 0, 0, 0, 0);
  ELL_4V_SET(nn, 0, 0, 0, 0);
  ELL_4V_SET(bb, 0, 0, 0, 1);
  ELL_3V_SUB(nn, cam->at, cam->from);
  len = ELL_3V_LEN(nn);
  if (!len) {
    sprintf(err, "%s: cam->at (%g,%g,%g) == cam->from", me,
            cam->at[0], cam->at[1], cam->at[2]);
    biffAdd(LIMN, err); return 1;
  }
  if (cam->atRelative) {
    /* ctx->cam->{neer,dist} are "at" relative */
    cam->vspNeer = cam->neer + len;
    cam->vspFaar = cam->faar + len;
    cam->vspDist = cam->dist + len;
  }
  else {
    /* ctx->cam->{neer,dist} are eye relative */
    cam->vspNeer = cam->neer;
    cam->vspFaar = cam->faar;
    cam->vspDist = cam->dist;
  }
  if (!(cam->vspNeer > 0 && cam->vspDist > 0 && cam->vspFaar > 0)) {
    sprintf(err, "%s: eye-relative near (%g), dist (%g), or far (%g) <= 0",
            me, cam->vspNeer, cam->vspDist, cam->vspFaar);
    biffAdd(LIMN, err); return 1;
  }
  if (!(cam->vspNeer <= cam->vspFaar)) {
    sprintf(err, "%s: eye-relative near (%g) further than far (%g)",
            me, cam->vspNeer, cam->vspFaar);
    biffAdd(LIMN, err); return 1 ;
  }
  if (AIR_EXISTS(cam->fov)) {
    if (!( AIR_IN_OP(0.0, cam->fov, 180.0) )) {
      sprintf(err, "%s: cam->fov (%g) not in valid range between 0 and 180",
              me, cam->fov);
      biffAdd(LIMN, err); return 1 ;
    }
    if (!AIR_EXISTS(cam->aspect)) {
      sprintf(err, "%s: cam->fov set, but cam->aspect isn't", me);
      biffAdd(LIMN, err); return 1;
    }
    /* "fov" is half vertical angle */
    cam->vRange[0] = -tan(cam->fov*AIR_PI/360)*(cam->vspDist); 
    cam->vRange[1] = -cam->vRange[0];
    cam->uRange[0] = cam->vRange[0]*(cam->aspect);
    cam->uRange[1] = -cam->uRange[0];
  }
  /* else cam->fov isn't set, but we're not going to complain if
     uRange and vRange aren't both set ... */

  ELL_3V_SCALE(nn, 1.0/len, nn);
  ELL_3V_CROSS(uu, nn, cam->up);
  len = ELL_3V_LEN(uu);
  if (!len) {
    sprintf(err, "%s: cam->up is co-linear with view direction", me);
    biffAdd(LIMN, err); return 1 ;
  }
  ELL_3V_SCALE(uu, 1.0/len, uu);

  if (cam->rightHanded) {
    ELL_3V_CROSS(vv, nn, uu);
  }
  else {
    ELL_3V_CROSS(vv, uu, nn);
  }

  ELL_4V_COPY(cam->U, uu);
  ELL_4V_COPY(cam->V, vv);
  ELL_4V_COPY(cam->N, nn);
  ELL_4M_TRANSLATE_SET(TT, -cam->from[0], -cam->from[1], -cam->from[2]);
  ELL_4M_ROWS_SET(RR, uu, vv, nn, bb);
  ELL_4M_MUL(cam->W2V, RR, TT);
  ell_4m_inv_d(cam->V2W, cam->W2V);

  return 0;
}

/*
******** limnCameraAspectSet
**
** simply sets the "aspect" field of the cam.  Note that calling this
** does *not* automatically mean that the uRange and vRange in the camera
** will be set according to the "fov"- the "fov" has to actually be set
** (be non-NaN) for that to happen.  This allows dumber functions to
** call this whenever they have the information required to do so, even
** if the "aspect" is not going to be needed for a given camera use
*/
int
limnCameraAspectSet(limnCamera *cam, int horz, int vert, int centering) {
  char me[] = "limnCameraAspectSet", err[BIFF_STRLEN];

  if (!cam) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  if (!( horz > 0 && vert > 0 )) {
    sprintf(err, "%s: bad image dimensions %dx%d", me, horz, vert);
    biffAdd(LIMN, err); return 1;
  }
  if (airEnumValCheck(nrrdCenter, centering)) {
    sprintf(err, "%s: centering %d not valid", me, centering);
    biffAdd(LIMN, err); return 1;
  }

  if (nrrdCenterCell == centering) {
    cam->aspect = ((double)horz)/vert;
  } else {
    cam->aspect = ((double)(horz-1))/(vert-1);
  }
  
  return 0;
}

/*
******** limnCameraPathMake
**
** uses limnSplines to do camera paths based on key-frames 
**
** output: cameras at all "numFrames" frames are set in the 
** PRE-ALLOCATED array of output cameras, "cam".
**
** input: 
** keycam: array of keyframe cameras
** time: times associated with the key frames 
** ---> both of these arrays are length "numKeys" <---
** trackWhat: takes values from the limnCameraPathTrack* enum
** quatType: spline to control camera orientations. This is needed for 
**          tracking at or from, but not needed for limnCameraPathTrackBoth.
**          This is the only limnSplineTypeSpec* argument that can be NULL.
** posType: spline to control whichever of from, at, and up are needed for
**          the given style of tracking.
** distType: spline to control neer, faar, dist: positions of near clipping,
**          far clipping, and image plane, as well as the 
**          distance between from and at (which is used if not doing
**          limnCameraPathTrackBoth)
** viewType: spline to control fov (and aspect, if you're crazy)
**
** NOTE: The "atRelative", "orthographic", and "rightHanded" fields
** are copied from keycam[0] into all output cam[i], but you still need
** to correctly set them for all keycam[i] for limnCameraUpdate to work
** as expected.  Also, for the sake of simplicity, this function only works
** with fov and aspect, instead of {u,v}Range, and hence both "fov" and
** "aspect" need to set in *all* the keycams, even if neither of them
** ever changes!
*/
int
limnCameraPathMake(limnCamera *cam, int numFrames,
                   limnCamera *keycam, double *time, int numKeys,
                   int trackWhat, 
                   limnSplineTypeSpec *quatType,
                   limnSplineTypeSpec *posType,
                   limnSplineTypeSpec *distType,
                   limnSplineTypeSpec *viewType) {
  char me[]="limnCameraPathMake", err[BIFF_STRLEN];
  char which[AIR_STRLEN_MED];
  airArray *mop;
  Nrrd *nquat, *nfrom, *natpt, *nupvc, *ndist, *nfova, *ntime, *nsample;
  double fratVec[3], *quat, *from, *atpt, *upvc, *dist, *fova,
    W2V[9], N[3], fratDist;
  limnSpline *timeSpline, *quatSpline, *fromSpline, *atptSpline, *upvcSpline,
    *distSpline, *fovaSpline;
  limnSplineTypeSpec *timeType;
  int ii, E;
  
  if (!( cam && keycam && time && posType && distType && viewType )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  if (!( AIR_IN_OP(limnCameraPathTrackUnknown, trackWhat,
                   limnCameraPathTrackLast) )) {
    sprintf(err, "%s: trackWhat %d not in valid range [%d,%d]", me,
            trackWhat, limnCameraPathTrackUnknown+1,
            limnCameraPathTrackLast-1);
    biffAdd(LIMN, err); return 1;
  }
  if (limnCameraPathTrackBoth != trackWhat && !quatType) {
    sprintf(err, "%s: need the quaternion limnSplineTypeSpec if not "
            "doing trackBoth", me);
    biffAdd(LIMN, err); return 1;
  }

  /* create and allocate nrrds.  For the time being, we're allocating
     more different nrrds, and filling their contents, than we need
     to-- nquat is not needed if we're doing limnCameraPathTrackBoth,
     for example.  However, we do make an effort to only do the spline
     evaluation on the things we actually need to know. */
  mop = airMopNew();
  airMopAdd(mop, nquat = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nfrom = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, natpt = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nupvc = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ndist = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nfova = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntime = nrrdNew(), (airMopper)nrrdNix, airMopAlways);
  if (nrrdWrap_va(ntime, time, nrrdTypeDouble, 1,
                  AIR_CAST(size_t, numKeys))) {
    sprintf(err, "%s: trouble wrapping time values", me);
    biffMove(LIMN, err, NRRD); airMopError(mop); return 1;
  }
  airMopAdd(mop, nsample = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  timeType = limnSplineTypeSpecNew(limnSplineTypeTimeWarp);
  airMopAdd(mop, timeType, (airMopper)limnSplineTypeSpecNix, airMopAlways);
  if (nrrdMaybeAlloc_va(nquat, nrrdTypeDouble, 2,
                        AIR_CAST(size_t, 4), AIR_CAST(size_t, numKeys))
      || nrrdMaybeAlloc_va(nfrom, nrrdTypeDouble, 2,
                           AIR_CAST(size_t, 3), AIR_CAST(size_t, numKeys))
      || nrrdMaybeAlloc_va(natpt, nrrdTypeDouble, 2,
                           AIR_CAST(size_t, 3), AIR_CAST(size_t, numKeys))
      || nrrdMaybeAlloc_va(nupvc, nrrdTypeDouble, 2,
                           AIR_CAST(size_t, 3), AIR_CAST(size_t, numKeys))
      || nrrdMaybeAlloc_va(ndist, nrrdTypeDouble, 2,
                           AIR_CAST(size_t, 4), AIR_CAST(size_t, numKeys))
      || nrrdMaybeAlloc_va(nfova, nrrdTypeDouble, 2,
                           AIR_CAST(size_t, 2), AIR_CAST(size_t, numKeys))) {
    sprintf(err, "%s: couldn't allocate buffer nrrds", me);
    biffMove(LIMN, err, NRRD); airMopError(mop); return 1;
  }
  quat = (double*)(nquat->data);
  from = (double*)(nfrom->data);
  atpt = (double*)(natpt->data);
  upvc = (double*)(nupvc->data);
  dist = (double*)(ndist->data);
  fova = (double*)(nfova->data);
  
  /* check cameras, and put camera information into nrrds */
  for (ii=0; ii<numKeys; ii++) {
    if (limnCameraUpdate(keycam + ii)) {
      sprintf(err, "%s: trouble with camera at keyframe %d\n", me, ii);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
    if (!( AIR_EXISTS(keycam[ii].fov) && AIR_EXISTS(keycam[ii].aspect) )) {
      sprintf(err, "%s: fov, aspect not both defined on keyframe %d", me, ii);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
    ell_4m_to_q_d(quat + 4*ii, keycam[ii].W2V);
    if (ii) {
      if (0 > ELL_4V_DOT(quat + 4*ii, quat + 4*(ii-1))) {
        ELL_4V_SCALE(quat + 4*ii, -1, quat + 4*ii);
      }
    }
    ELL_3V_COPY(from + 3*ii, keycam[ii].from);
    ELL_3V_COPY(atpt + 3*ii, keycam[ii].at);
    ELL_3V_COPY(upvc + 3*ii, keycam[ii].up);
    ELL_3V_SUB(fratVec, keycam[ii].from, keycam[ii].at);
    fratDist = ELL_3V_LEN(fratVec);
    ELL_4V_SET(dist + 4*ii, fratDist, 
               keycam[ii].neer, keycam[ii].dist, keycam[ii].faar);
    ELL_2V_SET(fova + 2*ii, keycam[ii].fov, keycam[ii].aspect);
  }

  /* create splines from nrrds */
  if (!( (strcpy(which, "quaternion"), quatSpline = 
          limnSplineCleverNew(nquat, limnSplineInfoQuaternion, quatType))
         && (strcpy(which, "from point"), fromSpline = 
             limnSplineCleverNew(nfrom, limnSplineInfo3Vector, posType))
         && (strcpy(which, "at point"), atptSpline = 
             limnSplineCleverNew(natpt, limnSplineInfo3Vector, posType))
         && (strcpy(which, "up vector"), upvcSpline = 
             limnSplineCleverNew(nupvc, limnSplineInfo3Vector, posType))
         && (strcpy(which, "plane distances"), distSpline = 
             limnSplineCleverNew(ndist, limnSplineInfo4Vector, distType))
         && (strcpy(which, "field-of-view"), fovaSpline =
             limnSplineCleverNew(nfova, limnSplineInfo2Vector, viewType))
         && (strcpy(which, "time warp"), timeSpline = 
             limnSplineCleverNew(ntime, limnSplineInfoScalar, timeType)) )) {
    sprintf(err, "%s: trouble creating %s spline", me, which);
    biffAdd(LIMN, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, quatSpline, (airMopper)limnSplineNix, airMopAlways);
  airMopAdd(mop, fromSpline, (airMopper)limnSplineNix, airMopAlways);
  airMopAdd(mop, atptSpline, (airMopper)limnSplineNix, airMopAlways);
  airMopAdd(mop, upvcSpline, (airMopper)limnSplineNix, airMopAlways);
  airMopAdd(mop, distSpline, (airMopper)limnSplineNix, airMopAlways);
  airMopAdd(mop, fovaSpline, (airMopper)limnSplineNix, airMopAlways);
  airMopAdd(mop, timeSpline, (airMopper)limnSplineNix, airMopAlways);

  /* evaluate splines */
  E = AIR_FALSE;
  if (!E) E |= limnSplineSample(nsample, timeSpline,
                                limnSplineMinT(timeSpline), numFrames,
                                limnSplineMaxT(timeSpline));
  quat = NULL;
  from = NULL;
  atpt = NULL;
  upvc = NULL;
  switch(trackWhat) {
  case limnCameraPathTrackAt:
    if (!E) E |= limnSplineNrrdEvaluate(natpt, atptSpline, nsample);
    if (!E) atpt = (double*)(natpt->data);
    if (!E) E |= limnSplineNrrdEvaluate(nquat, quatSpline, nsample);
    if (!E) quat = (double*)(nquat->data);    
    break;
  case limnCameraPathTrackFrom:
    if (!E) E |= limnSplineNrrdEvaluate(nfrom, fromSpline, nsample);
    if (!E) from = (double*)(nfrom->data);
    if (!E) E |= limnSplineNrrdEvaluate(nquat, quatSpline, nsample);
    if (!E) quat = (double*)(nquat->data);    
    break;
  case limnCameraPathTrackBoth:
    if (!E) E |= limnSplineNrrdEvaluate(nfrom, fromSpline, nsample);
    if (!E) from = (double*)(nfrom->data);
    if (!E) E |= limnSplineNrrdEvaluate(natpt, atptSpline, nsample);
    if (!E) atpt = (double*)(natpt->data);
    if (!E) E |= limnSplineNrrdEvaluate(nupvc, upvcSpline, nsample);
    if (!E) upvc = (double*)(nupvc->data);
    break;
  }
  dist = NULL;
  if (!E) E |= limnSplineNrrdEvaluate(ndist, distSpline, nsample);
  if (!E) dist = (double*)(ndist->data);
  fova = NULL;
  if (!E) E |= limnSplineNrrdEvaluate(nfova, fovaSpline, nsample);
  if (!E) fova = (double*)(nfova->data);
  if (E) {
    sprintf(err, "%s: trouble evaluating splines", me);
    biffAdd(LIMN, err); airMopError(mop); return 1;
  }

  /* copy information from nrrds back into cameras */
  for (ii=0; ii<numFrames; ii++) {
    cam[ii].atRelative = keycam[0].atRelative;
    cam[ii].orthographic = keycam[0].orthographic;
    cam[ii].rightHanded = keycam[0].rightHanded;
    if (limnCameraPathTrackBoth == trackWhat) {
      ELL_3V_COPY(cam[ii].from, from + 3*ii);
      ELL_3V_COPY(cam[ii].at, atpt + 3*ii);
      ELL_3V_COPY(cam[ii].up, upvc + 3*ii);
    } else {
      fratDist = (dist + 4*ii)[0];
      ell_q_to_3m_d(W2V, quat + 4*ii);
      ELL_3MV_ROW1_GET(cam[ii].up, W2V);
      if (cam[ii].rightHanded) {
        ELL_3V_SCALE(cam[ii].up, -1, cam[ii].up);
      }
      ELL_3MV_ROW2_GET(N, W2V);
      if (limnCameraPathTrackFrom == trackWhat) {
        ELL_3V_COPY(cam[ii].from, from + 3*ii);
        ELL_3V_SCALE_ADD2(cam[ii].at, 1.0, cam[ii].from, fratDist, N);
      } else {
        ELL_3V_COPY(cam[ii].at, atpt + 3*ii);
        ELL_3V_SCALE_ADD2(cam[ii].from, 1.0, cam[ii].at, -fratDist, N);
      }
    }
    cam[ii].neer = (dist + 4*ii)[1];
    cam[ii].dist = (dist + 4*ii)[2];
    cam[ii].faar = (dist + 4*ii)[3];
    cam[ii].fov = (fova + 2*ii)[0];
    cam[ii].aspect = (fova + 2*ii)[1];
    if (limnCameraUpdate(cam + ii)) {
      sprintf(err, "%s: trouble with output camera %d\n", me, ii);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
  }
  
  airMopOkay(mop);
  return 0;
}
