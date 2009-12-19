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

#include <teem/air.h>
#include <teem/hest.h>
#include <teem/biff.h>
#include <teem/nrrd.h>
#include <teem/gage.h>
#include <teem/limn.h>
#include <teem/hoover.h>
#include <teem/ten.h>

int
probeParseKind(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  char me[] = "probeParseKind";
  gageKind **kindP;
  
  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  kindP = (gageKind **)ptr;
  airToLower(str);
  if (!strcmp("scalar", str)) {
    *kindP = gageKindScl;
  } else if (!strcmp("vector", str)) {
    *kindP = gageKindVec;
  } else if (!strcmp("tensor", str)) {
    *kindP = tenGageKind;
  } else {
    sprintf(err, "%s: not \"scalar\", \"vector\", or \"tensor\"", me);
    return 1;
  }
  
  return 0;
}

hestCB probeKindHestCB = {
  sizeof(gageKind *),
  "kind",
  probeParseKind,
  NULL
}; 

#define MREND "mrender"

char *info = ("A demonstration of hoover, gage, and nrrd measures. "
              "Uses hoover to cast rays through a volume (scalar, vector, or "
              "tensor), gage to "
              "measure one of various quantities along the rays, and a "
              "specified nrrd measure to reduce all the values along a ray "
              "down to one scalar, which is saved in the output (double) "
              "image.");

/* -------------------------------------------------------------- */

/* Even though the gageContext is really thread-specific, and
   therefore doesn't really belong in mrendUser, the first context
   from which all others is copied is logically shared across threads,
   as are the input parameter it contains. There is a per-thread
   gageContext pointer in mrendThread */

typedef struct {
  Nrrd *nin;            /* input volume to render */
  gageKind *kind;       /* the kind of volume it is */
  int verbPixel[2];     /* which pixel to do verbose stuff on */
  double rayStep,       /* distance between sampling planes */
    fromNaN;            /* what to convert non-existent value to */
  int whatq,            /* what to measure along the ray */
    measr;              /* how to reduce the ray samples to a scalar */
  /* we have a seperate copy of the kernel specs so that hest can
     set these, and then we'll gageKernelSet() them in the context
     in order to do the proper error checking- hest can't do the
     error checking that we need... */
  NrrdKernelSpec *ksp[GAGE_KERNEL_NUM];
  gageContext *gctx0;   /* gage input and parent thread state */
  hooverContext *hctx;  /* hoover input and state */
  char *outS;           /* (managed by hest) output filename */
  
  airArray *mrmop;
} mrendUser;

mrendUser *
mrendUserNew() {
  mrendUser *uu;
  int i;
  
  uu = (mrendUser *)calloc(1, sizeof(mrendUser));
  uu->nin = NULL;
  uu->kind = NULL;
  uu->rayStep = 0.0;
  uu->whatq = gageSclUnknown;
  uu->measr = nrrdMeasureUnknown;
  for (i=0; i<GAGE_KERNEL_NUM; i++) {
    uu->ksp[i] = NULL;
  }
  uu->gctx0 = gageContextNew();
  uu->hctx = hooverContextNew();
  uu->outS = NULL;
  uu->mrmop = airMopNew();
  airMopAdd(uu->mrmop, uu->gctx0, (airMopper)gageContextNix, airMopAlways);
  airMopAdd(uu->mrmop, uu->hctx, (airMopper)hooverContextNix, airMopAlways);
  return uu;
}

mrendUser *
mrendUserNix(mrendUser *uu) {
  
  if (uu) {
    airMopOkay(uu->mrmop);
    airFree(uu);
  }
  return NULL;
}

int
mrendUserCheck(mrendUser *uu) {
  char me[]="mrendUserCheck", err[BIFF_STRLEN];
  
  if (3 + uu->kind->baseDim != uu->nin->dim) {
    sprintf(err, "%s: input nrrd needs %d dimensions, not %d", 
            me,  + uu->kind->baseDim, uu->nin->dim);
    biffAdd(MREND, err); return 1;
  }
  if (!( uu->nin->axis[0].center == uu->nin->axis[1].center &&
         uu->nin->axis[0].center == uu->nin->axis[2].center )) {
    sprintf(err, "%s: axes 0,1,2 centerings (%s,%s,%s) not equal", me,
            airEnumStr(nrrdCenter, uu->nin->axis[0].center),
            airEnumStr(nrrdCenter, uu->nin->axis[1].center),
            airEnumStr(nrrdCenter, uu->nin->axis[2].center));
    biffAdd(MREND, err); return 1;
  }
  if (1 != uu->kind->table[uu->whatq].answerLength) {
    sprintf(err, "%s: quantity %s (in %s volumes) isn't a scalar; "
            "can't render it",
            me, airEnumStr(uu->kind->enm, uu->whatq), uu->kind->name);
    biffAdd(MREND, err); return 1;
  }
  
  return 0;
}

/* -------------------------------------------------------------- */

typedef struct mrendRender_t {
  double time0, time1;  /* render start and end times */
  Nrrd *nout;           /* output image: always 2D array of doubles */
  double *imgData;       /* output image data */
  int sx, sy,           /* image dimensions */
    totalSamples;       /* total number of samples used for all rays */
  struct mrendThread_t *tinfo[HOOVER_THREAD_MAX];
} mrendRender;

typedef struct mrendThread_t {
  double *val,          /* array of ray samples */
    rayLen,             /* length of ray segment between near and far */
    rayStep;            /* ray step needed FOR THIS RAY, to acheive sampling on
                           planes (same scaling of uu->rayStep) */
  int thrid,            /* thread ID */
    valLen,             /* allocated size of val */
    valNum,             /* number of values set in val (index of next value) */
    ui, vi,             /* image coords */
    numSamples,         /* total number of samples this thread has done */
    verbose;            /* blah blah blah blah */
  gageContext *gctx;    /* thread-specific gage context (or copy of uu->gctx0
                           for the first thread) */
  const gage_t *answer; /* pointer to the SINGLE answer we care about */
} mrendThread;

int
mrendRenderBegin(mrendRender **rrP, mrendUser *uu) {
  char me[]="mrendRenderBegin", err[BIFF_STRLEN];
  gagePerVolume *pvl;
  int E, thr;
  
  /* this assumes that mrendUserCheck(uu) has passed */
  
  *rrP = (mrendRender *)calloc(1, sizeof(mrendRender));
  airMopAdd(uu->mrmop, *rrP, airFree, airMopAlways);
  /* pvl managed via parent context */
  
  (*rrP)->time0 = airTime();
  
  E = 0;
  if (!E) E |= !(pvl = gagePerVolumeNew(uu->gctx0, uu->nin, uu->kind));
  if (!E) E |= gagePerVolumeAttach(uu->gctx0, pvl);
  if (!E) E |= gageKernelSet(uu->gctx0, gageKernel00,
                             uu->ksp[gageKernel00]->kernel,
                             uu->ksp[gageKernel00]->parm);
  if (!E) E |= gageKernelSet(uu->gctx0, gageKernel11,
                             uu->ksp[gageKernel11]->kernel,
                             uu->ksp[gageKernel11]->parm);
  if (!E) E |= gageKernelSet(uu->gctx0, gageKernel22,
                             uu->ksp[gageKernel22]->kernel,
                             uu->ksp[gageKernel22]->parm);
  if (!E) E |= gageQueryItemOn(uu->gctx0, pvl, uu->whatq);
  if (!E) E |= gageUpdate(uu->gctx0);
  if (E) {
    sprintf(err, "%s: gage trouble", me);
    biffMove(MREND, err, GAGE);
    return 1;
  }
  fprintf(stderr, "%s: kernel support = %d^3 samples\n", me,
          2*uu->gctx0->radius);
  
  if (nrrdMaybeAlloc_va((*rrP)->nout=nrrdNew(), nrrdTypeDouble, 2,
                        AIR_CAST(size_t, uu->hctx->imgSize[0]),
                        AIR_CAST(size_t, uu->hctx->imgSize[1]))) {
    sprintf(err, "%s: nrrd trouble", me);
    biffMove(MREND, err, NRRD);
    return 1;
  }
  (*rrP)->nout->axis[0].min = uu->hctx->cam->uRange[0];
  (*rrP)->nout->axis[0].max = uu->hctx->cam->uRange[1];
  (*rrP)->nout->axis[1].min = uu->hctx->cam->vRange[0];
  (*rrP)->nout->axis[1].max = uu->hctx->cam->vRange[1];
  airMopAdd(uu->mrmop, (*rrP)->nout, (airMopper)nrrdNuke, airMopAlways);
  (*rrP)->imgData = AIR_CAST(double*, (*rrP)->nout->data);
  (*rrP)->sx = uu->hctx->imgSize[0];
  (*rrP)->sy = uu->hctx->imgSize[1];
  
  for (thr=0; thr<uu->hctx->numThreads; thr++) {
    (*rrP)->tinfo[thr] = (mrendThread *)calloc(1, sizeof(mrendThread));
    airMopAdd(uu->mrmop, (*rrP)->tinfo[thr], airFree, airMopAlways);
  }
  
  return 0;
}

int
mrendRenderEnd(mrendRender *rr, mrendUser *uu) {
  char me[]="mrendRenderEnd", err[BIFF_STRLEN];
  int thr;
  
  /* add up # samples from all threads */
  rr->totalSamples = 0;
  for (thr=0; thr<uu->hctx->numThreads; thr++) {
    rr->totalSamples += rr->tinfo[thr]->numSamples;
  }
  
  rr->time1 = airTime();
  fprintf(stderr, "\n");
  fprintf(stderr, "%s: rendering time = %g secs\n", me,
          rr->time1 - rr->time0);
  fprintf(stderr, "%s: sampling rate = %g KHz\n", me,
          rr->totalSamples/(1000.0*(rr->time1 - rr->time0)));
  if (nrrdSave(uu->outS, rr->nout, NULL)) {
    sprintf(err, "%s: trouble saving image", me);
    biffMove(MREND, err, NRRD);
    return 1;
  }
  
  return 0;
}

/* -------------------------------------------------------------- */

int
mrendThreadBegin(mrendThread **ttP,
                 mrendRender *rr, mrendUser *uu, int whichThread) {
  
  /* allocating the mrendThreads should be part of the thread body,
     but as long as there isn't a mutex around registering them with
     the airMop in the mrendRender, then all that needs to be done
     as part of mrendRenderBegin (see above) */
  (*ttP) = rr->tinfo[whichThread];
  if (!whichThread) {
    /* this is the first thread- it just points to the parent gageContext */
    (*ttP)->gctx = uu->gctx0;
  } else {
    /* we have to generate a new gageContext */
    (*ttP)->gctx = gageContextCopy(uu->gctx0);
  }
  (*ttP)->answer = gageAnswerPointer((*ttP)->gctx,
                                     (*ttP)->gctx->pvl[0], uu->whatq);
  (*ttP)->val = NULL;
  (*ttP)->valLen = 0;
  (*ttP)->valNum = 0;
  (*ttP)->rayLen = 0;
  (*ttP)->thrid = whichThread;
  (*ttP)->numSamples = 0;
  return 0;
}

int
mrendThreadEnd(mrendThread *tt, mrendRender *rr, mrendUser *uu) {

  AIR_UNUSED(rr);
  AIR_UNUSED(uu);
  if (tt->thrid) {
    tt->gctx = gageContextNix(tt->gctx);
  }
  tt->val = AIR_CAST(double*, airFree(tt->val));
  
  return 0;
}

/* -------------------------------------------------------------- */

int
mrendRayBegin(mrendThread *tt, mrendRender *rr, mrendUser *uu,
              int uIndex,
              int vIndex,
              double rayLen,
              double rayStartWorld[3],
              double rayStartIndex[3],
              double rayDirWorld[3],
              double rayDirIndex[3]) {
  int newLen;

  AIR_UNUSED(rr);
  AIR_UNUSED(rayStartWorld);
  AIR_UNUSED(rayStartIndex);
  AIR_UNUSED(rayDirWorld);
  AIR_UNUSED(rayDirIndex);
  tt->ui = uIndex;
  tt->vi = vIndex;
  if (!( -1 == uu->verbPixel[0] && -1 == uu->verbPixel[1] )) {
    if (uIndex == uu->verbPixel[0] && vIndex == uu->verbPixel[1]) {
      gageParmSet(uu->gctx0, gageParmVerbose, AIR_TRUE);
    } else {
      gageParmSet(uu->gctx0, gageParmVerbose, AIR_FALSE);
    }
  }
  tt->rayLen = rayLen;
  tt->rayStep = (uu->rayStep*tt->rayLen /
                 (uu->hctx->cam->vspFaar - uu->hctx->cam->vspNeer));
  newLen = AIR_ROUNDUP(rayLen/tt->rayStep) + 1;
  if (!tt->val || newLen > tt->valLen) {
    tt->val = AIR_CAST(double*, airFree(tt->val));
    tt->valLen = newLen;
    tt->val = AIR_CAST(double*, calloc(newLen, sizeof(double)));
  }
  tt->valNum = 0;
  if (!uIndex) {
    fprintf(stderr, "%d/%d ", vIndex, uu->hctx->imgSize[1]);
    fflush(stderr);
  }
  
  fflush(stderr);
  return 0;
}

int
mrendRayEnd(mrendThread *tt, mrendRender *rr, mrendUser *uu) {
  double answer;

  if (tt->valNum) {
    nrrdMeasureLine[uu->measr](&answer,
                               nrrdTypeDouble,
                               tt->val, nrrdTypeDouble,
                               tt->valNum,
                               0, tt->rayLen);
    answer = AIR_EXISTS(answer) ? answer : uu->fromNaN;
    rr->imgData[(tt->ui) + (rr->sx)*(tt->vi)] = answer;
  } else {
    rr->imgData[(tt->ui) + (rr->sx)*(tt->vi)] = 0.0;
  }
  
  return 0;
}

/* -------------------------------------------------------------- */

double
mrendSample(mrendThread *tt, mrendRender *rr, mrendUser *uu,
            int num, double rayT,
            int inside,
            double samplePosWorld[3],
            double samplePosIndex[3]) {
  char me[]="mrendSample", err[BIFF_STRLEN];

  AIR_UNUSED(rr);
  AIR_UNUSED(uu);
  AIR_UNUSED(num);
  AIR_UNUSED(rayT);
  AIR_UNUSED(samplePosWorld);

  if (inside) {
    if (gageProbe(tt->gctx,
                  AIR_CAST(gage_t, samplePosIndex[0]),
                  AIR_CAST(gage_t, samplePosIndex[1]),
                  AIR_CAST(gage_t, samplePosIndex[2]))) {
      sprintf(err, "%s: gage trouble: %s (%d)", me,
              tt->gctx->errStr, tt->gctx->errNum);
      biffAdd(MREND, err);
      return AIR_NAN;
    }
    tt->val[tt->valNum++] = *(tt->answer);
    tt->numSamples++;
  }
  
  return tt->rayStep;
}

/* -------------------------------------------------------------- */

#if 0

this was nixed once mrender learned to handle volume of general
kind, instead of being restricted to scalars


/*
** learned: if you're playing games with strings with two passes, where
** you first generate the set of strings in order to calculate their
** cumulative length, and then (2nd pass) concatenate the strings
** together, be very sure that the generation of the strings on the
** two passes is identical.  Had a very troublesome memory error because
** I was using short version of the description string to determine
** allocation, and then the long version in the second pass...
*/
char *
mrendGage(char *prefix) {
  char *line, *ret;
  int i, len;
  
  /* 1st pass through- determine needed buffer size */
  len = 0;
  for (i=airEnumUnknown(gageScl)+1; !airEnumValCheck(gageScl, i); i++) {
    if (1 == gageKindScl->table[i].answerLength) {
      line = airEnumFmtDesc(gageScl, i, AIR_FALSE, "\n \b\bo \"%s\": %s");
      len += strlen(line);
      free(line);
    }
  }
  ret = (char*)calloc(strlen(prefix) + len + 1, sizeof(char));
  if (ret) {
    strcpy(ret, prefix);
    /* 2nd pass through: create output */
    for (i=airEnumUnknown(gageScl)+1; !airEnumValCheck(gageScl, i); i++) {
      if (1 == gageKindScl->table[i].answerLength) {
        line = airEnumFmtDesc(gageScl, i, AIR_FALSE, "\n \b\bo \"%s\": %s");
        strcat(ret, line);
        free(line);
      }
    }
  }
  return ret;
}

#endif

int
main(int argc, char *argv[]) {
  hestOpt *hopt=NULL;
  hestParm *hparm;
  int E, Ecode, Ethread, renorm, base, offfr;
  char *me, *errS, *whatS;
  mrendUser *uu;
  airArray *mop;
  double gmc, turn, eye[3], eyedist;
  
  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hparm->respFileEnable = AIR_TRUE;
  uu = mrendUserNew();
  
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  airMopAdd(mop, uu, (airMopper)mrendUserNix, airMopAlways);
  
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &(uu->nin), NULL,
             "input nrrd to render", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "k", "kind", airTypeOther, 1, 1, &(uu->kind), NULL,
             "\"kind\" of volume (\"scalar\", \"vector\", or \"tensor\")",
             NULL, NULL, &probeKindHestCB);
  limnHestCameraOptAdd(&hopt, uu->hctx->cam,
                       NULL, "0 0 0", "0 0 1",
                       NULL, NULL, NULL,
                       "nan nan", "nan nan", "20");
  hestOptAdd(&hopt, "offfr", NULL, airTypeInt, 0, 0, &offfr, NULL,
             "the given eye point (\"-fr\") is to be interpreted "
             "as an offset from the at point.");
  hestOptAdd(&hopt, "turn", "angle", airTypeDouble, 1, 1, &turn, "0.0",
             "angle (degrees) by which to rotate the from point around "
             "true up, for making stereo pairs.  Positive means move "
             "towards positive U (the right)");
  hestOptAdd(&hopt, "is", "image size", airTypeInt, 2, 2, uu->hctx->imgSize,
             "256 256", "image dimensions");
  hestOptAdd(&hopt, "k00", "kernel", airTypeOther, 1, 1,
             &(uu->ksp[gageKernel00]), "tent",
             "value reconstruction kernel",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k11", "kernel", airTypeOther, 1, 1,
             &(uu->ksp[gageKernel11]), "cubicd:1,0",
             "first derivative kernel",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k22", "kernel", airTypeOther, 1, 1,
             &(uu->ksp[gageKernel22]), "cubicdd:1,0",
             "second derivative kernel",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "rn", NULL, airTypeBool, 0, 0, &renorm, NULL,
             "renormalize kernel weights at each new sample location. "
             "\"Accurate\" kernels don't need this; doing it always "
             "makes things go slower");
  hestOptAdd(&hopt, "q", "query", airTypeString, 1, 1, &whatS, NULL,
             "the quantity (scalar, vector, or matrix) to learn by probing");
  hestOptAdd(&hopt, "m", "measure", airTypeEnum, 1, 1, &(uu->measr), NULL,
             "how to collapse list of ray samples into one scalar. "
             NRRD_MEASURE_DESC,
             NULL, nrrdMeasure);
  hestOptAdd(&hopt, "gmc", "min gradmag", airTypeDouble, 1, 1, &gmc, "0.0",
             "For curvature-related queries, set answer to zero when "
             "gradient magnitude is below this");
  hestOptAdd(&hopt, "fn", "from nan", airTypeDouble, 1, 1, &(uu->fromNaN),
             "nan", "When histo-based measures generate NaN answers, the "
             "value that should be substituted for NaN.");
  hestOptAdd(&hopt, "step", "size", airTypeDouble, 1, 1, &(uu->rayStep),
             "0.01", "step size along ray in world space");
  hestOptAdd(&hopt, "nt", "# threads", airTypeInt, 1, 1,
             &(uu->hctx->numThreads),
             "1", "number of threads hoover should use");
  hestOptAdd(&hopt, "vp", "img coords", airTypeInt, 2, 2, &(uu->verbPixel),
             "-1 -1", "pixel coordinates for which to turn on all verbose "
             "debugging messages, or \"-1 -1\" to disable this.");
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &(uu->outS), "-",
             "file to write output nrrd to.  Defaults to stdout (\"-\").");
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  uu->whatq = airEnumVal(uu->kind->enm, whatS);
  if (-1 == uu->whatq) {
    /* -1 indeed always means "unknown" for any gageKind */
    fprintf(stderr, "%s: couldn't parse \"%s\" as measure of \"%s\" volume\n",
            me, whatS, uu->kind->name);
    hestUsage(stderr, hopt, me, hparm);
    hestGlossary(stderr, hopt, hparm);
    airMopError(mop);
    return 1;
  }
  
  if (mrendUserCheck(uu)) {
    fprintf(stderr, "%s: problem with input parameters:\n%s\n",
            me, errS = biffGetDone(MREND)); free(errS);
    airMopError(mop);
    return 1;
  }
  gageParmSet(uu->gctx0, gageParmGradMagCurvMin,
              AIR_CAST(gage_t, gmc));
  gageParmSet(uu->gctx0, gageParmRequireAllSpacings,
              AIR_CAST(gage_t, AIR_FALSE));
  gageParmSet(uu->gctx0, gageParmRenormalize,
              AIR_CAST(gage_t, renorm));
  fprintf(stderr, "%s: will render %s of %s in %s volume\n", me,
          airEnumStr(nrrdMeasure, uu->measr),
          airEnumStr(uu->kind->enm, uu->whatq), uu->kind->name);
  
  if (offfr) {
    ELL_3V_INCR(uu->hctx->cam->from, uu->hctx->cam->at);
  }
  if (limnCameraAspectSet(uu->hctx->cam, 
                          uu->hctx->imgSize[0], uu->hctx->imgSize[1],
                          nrrdCenterCell)
      || limnCameraUpdate(uu->hctx->cam)) {
    airMopAdd(mop, errS = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble setting camera:\n%s\n", me, errS);
    airMopError(mop);
    return 1;
  }
  if (turn) {
    turn *= AIR_PI/180;
    ELL_3V_SUB(eye, uu->hctx->cam->from, uu->hctx->cam->at);
    ELL_3V_NORM(eye, eye, eyedist);
    ELL_3V_SCALE_ADD2(uu->hctx->cam->from,
                      cos(turn), eye,
                      sin(turn), uu->hctx->cam->U);
    ELL_3V_SCALE(uu->hctx->cam->from, eyedist, uu->hctx->cam->from);
    if (limnCameraUpdate(uu->hctx->cam)) {
      airMopAdd(mop, errS = biffGetDone(LIMN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble setting camera (again):\n%s\n", me, errS);
      airMopError(mop);
      return 1;
    }
  }
  /*
    fprintf(stderr, "%s: camera info\n", me);
    fprintf(stderr, "    U = {%g,%g,%g}\n",
    uu->hctx->cam->U[0], uu->hctx->cam->U[1], uu->hctx->cam->U[2]);
    fprintf(stderr, "    V = {%g,%g,%g}\n",
    uu->hctx->cam->V[0], uu->hctx->cam->V[1], uu->hctx->cam->V[2]);
    fprintf(stderr, "    N = {%g,%g,%g}\n",
    uu->hctx->cam->N[0], uu->hctx->cam->N[1], uu->hctx->cam->N[2]);
  */
  
  /* set remaining fields of hoover context */
  base = uu->kind->baseDim;
  uu->hctx->volSize[0] = uu->nin->axis[base+0].size;
  uu->hctx->volSize[1] = uu->nin->axis[base+1].size;
  uu->hctx->volSize[2] = uu->nin->axis[base+2].size;
  uu->hctx->volSpacing[0] = uu->nin->axis[base+0].spacing;
  uu->hctx->volSpacing[1] = uu->nin->axis[base+1].spacing;
  uu->hctx->volSpacing[2] = uu->nin->axis[base+2].spacing;
  if (nrrdCenterUnknown != uu->nin->axis[base].center) {
    uu->hctx->volCentering = uu->nin->axis[base].center;
    fprintf(stderr, "%s: setting volCentering to %s\n", me,
            airEnumStr(nrrdCenter, uu->nin->axis[base].center));
  }
  fprintf(stderr, "!%s: uu->hctx->volCentering = %d\n",
          me, uu->hctx->volCentering);
  /* this is reasonable for now */
  uu->hctx->imgCentering = nrrdCenterCell;
  uu->hctx->user = uu;
  uu->hctx->renderBegin = (hooverRenderBegin_t *)mrendRenderBegin;
  uu->hctx->threadBegin = (hooverThreadBegin_t *)mrendThreadBegin;
  uu->hctx->rayBegin = (hooverRayBegin_t *)mrendRayBegin;
  uu->hctx->sample = (hooverSample_t *)mrendSample;
  uu->hctx->rayEnd = (hooverRayEnd_t *)mrendRayEnd;
  uu->hctx->threadEnd = (hooverThreadEnd_t *)mrendThreadEnd;
  uu->hctx->renderEnd = (hooverRenderEnd_t *)mrendRenderEnd;
  
  if (!airThreadCapable && 1 != uu->hctx->numThreads) {
    fprintf(stderr, "%s: This Teem not compiled with "
            "multi-threading support.\n", me);
    fprintf(stderr, "%s: --> can't use %d threads; only using 1\n",
            me, uu->hctx->numThreads);
    uu->hctx->numThreads = 1;
  }
  
  E = hooverRender(uu->hctx, &Ecode, &Ethread);
  if (E) {
    if (hooverErrInit == E) {
      fprintf(stderr, "%s: ERROR (code %d, thread %d):\n%s\n",
              me, Ecode, Ethread, errS = biffGetDone(HOOVER));
      free(errS);
    } else {
      fprintf(stderr, "%s: ERROR (code %d, thread %d):\n%s\n",
              me, Ecode, Ethread, errS = biffGetDone(MREND));
      free(errS);
    }
    airMopError(mop);
    return 1;
  }
  
  airMopOkay(mop);
  return 0;
}
