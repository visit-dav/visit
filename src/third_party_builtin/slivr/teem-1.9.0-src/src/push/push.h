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

#ifndef PUSH_HAS_BEEN_INCLUDED
#define PUSH_HAS_BEEN_INCLUDED

#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/gage.h>
#include <teem/ten.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(push_EXPORTS) || defined(teem_EXPORTS)
#    define PUSH_EXPORT extern __declspec(dllexport)
#  else
#    define PUSH_EXPORT extern __declspec(dllimport)
#  endif
#define PUSH_EXPORT2 PUSH_EXPORT
#else /* TEEM_STATIC || UNIX */
# if __GNUC__ >= 4 && (defined(push_EXPORTS) || defined(teem_EXPORTS))
#  define PUSH_EXPORT __attribute__ ((visibility("default")))
# else
#  define PUSH_EXPORT extern
# endif
#define PUSH_EXPORT2 extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
** 0: push_t == float
** 1: push_t == double
*/
#if 0
typedef double push_t;
#define push_nrrdType nrrdTypeDouble
#define PUSH_TYPE_FLOAT 0
#define tenEIGENSOLVE tenEigensolve_d
#else
typedef float push_t;
#define push_nrrdType nrrdTypeFloat
#define PUSH_TYPE_FLOAT 1
#define tenEIGENSOLVE tenEigensolve_f
#endif

#define PUSH pushBiffKey
#define PUSH_STAGE_MAXNUM 4
#define PUSH_STAGE_PARM_MAXNUM 1
#define PUSH_FORCE_PARM_MAXNUM 3
#define PUSH_THREAD_MAXNUM 512

/*
******** pointPoint
**
** information about a point in the simulation.  There are really two
** kinds of information here: "pos", "vel", and "frc" pertain to the simulation
** of point dynamics, while "ten", "aniso", "inv", and "cnt" are properties of
** the field sampled at the point.  "tan" and "cur" are only meaningful in
** tractlets.
*/
typedef struct pushPoint_t {
  struct pushThing_t *thing;   /* what thing do I belong to */
  push_t charge,               /* "charge" of the point */
    pos[3],                    /* position in world space */
    vel[3],                    /* velocity */
    frc[3],                    /* force accumulator for current iteration */
    ten[7],                    /* tensor here */
    aniso,                     /* value of Cl1 (Westin ISMRM '97) */
    inv[7],                    /* inverse of tensor */
    cnt[3],                    /* mask's containment gradient */
    tan[3],                    /* tangent: unit direction through me */
    nor[3];                    /* change in tangent, normalized */
} pushPoint;

/*
******** pushThing struct
**
** Represents both single points, and tractlets, as follows:
**
** for single points: "point" tells the whole story of the point,
** but point.{tan,nor} is meaningless.  For the sake of easily computing all
** pair-wise point interactions between things, "numVert" is 1, and
** "vert" points to "point".  "len" is 0.
**
** for tractlets: the "pos", "vel", "frc" fields of "point" summarize the
** the dynamics of the entire tractlet, while the field attributes
** ("ten", "inv", "cnt") pertain exactly to the seed point.  For example,
** a particular tensor anisotropy in "point.ten" may have resulted in this
** thing turning from a point into a tractlet, or vice versa.  "numVert" is
** the number of tractlet vertices; "vert" is the array of them.  The
** only field of the vertex points that is not meaningful is "vel": the
** tractlet velocity is "point.vel"
*/
typedef struct pushThing_t {
  int ttaagg;
  pushPoint point;             /* information about single point, or a
                                  seed point, hard to say exactly */
  unsigned int numVert;        /* 1 for single point, else length of vert[] */
  pushPoint *vert;             /* dyn. alloc. array of tractlet vertices
                                  (*not* pointers to pushPoints), or, just
                                  the address of "point" for single point */
  unsigned int seedIdx;        /* which of the vertices is the seed point */
  push_t len;                  /* 0 for point, else (world-space) length of
                                  tractlet */
} pushThing;

/*
******** pushBin
**
** the data structure for doing spatial binning.  This really serves two
** related purposes- to bin *all* the points in the simulation-- both 
** single points as well as vertices of tractlets-- and also to bin the
** all "things".  The binning of points is used for the force calculation
** stage, and the binngin of things is used for the update stage.
**
** The bins are the *only* way to access the things in the simulation, so
** bins own the things they contain.  However, because things point to
** points, bins do not own the points they contain.
*/
typedef struct pushBin_t {
  unsigned int numPoint;       /* # of points in this bin */
  pushPoint **point;           /* dyn. alloc. array of point pointers */
  airArray *pointArr;          /* airArray around pointPtr and numThing */
  unsigned int numThing;       /* # of things in this bin */
  pushThing **thing;           /* dyn. alloc. array of thing pointers */
  airArray *thingArr;          /* airArray around thingPtr and numThing */
  struct pushBin_t **neighbor; /* pre-computed NULL-terminated list of all
                                  neighboring bins, including myself */
} pushBin;

/*
******** pushTask
**
** The information specific for a thread.  
*/
typedef struct pushTask_t {
  struct pushContext_t *pctx;  /* parent's context */
  gageContext *gctx;           /* result of gageContextCopy(pctx->gctx) */
  const gage_t *tenAns, *invAns,
    *cntAns;                   /* results of gage probing */
  tenFiberContext *fctx;       /* result of tenFiberContextCopy(pctx->fctx) */
  airThread *thread;           /* my thread */
  unsigned int threadIdx,      /* which thread am I */
    numThing;                  /* # things I let live this iteration */
  double sumVel,               /* sum of velocities of things in my bins */
    *vertBuff;                 /* buffer for tractlet vertices */
  void *returnPtr;             /* for airThreadJoin */
} pushTask;

/*
******** pushForce
**
** the functions and information which determine inter-point forces
*/
typedef struct {
  push_t (*func)(push_t haveDist, push_t restDist, push_t scale,
                 const push_t parm[PUSH_FORCE_PARM_MAXNUM]);
  push_t (*maxDist)(push_t maxEval, push_t scale,
                    const push_t parm[PUSH_FORCE_PARM_MAXNUM]);
  push_t parm[PUSH_FORCE_PARM_MAXNUM];
} pushForce;

/*
******** pushProcess
**
** the sort of function that is called by the worker threads
*/
typedef int (*pushProcess)(pushTask *task, int bin,
                           const push_t parm[PUSH_STAGE_PARM_MAXNUM]);

/*
******** pushContext
**
** everything for doing one simulation computation
*/
typedef struct pushContext_t {
  /* INPUT ----------------------------- */
  Nrrd *nin,                       /* image of 2D or 3D masked tensors */
    *npos,                         /* positions to start with
                                      (overrides numPoint) */
    *nstn;                         /* start/nums for tractlets in npos */
  double drag,                     /* to slow fast things down */
    preDrag,                       /* different drag pre-min-iter */
    step,                          /* time step in integration */
    mass,                          /* mass of particles */
    scale,                         /* scaling from tensor to glyph size */
    nudge,                         /* scaling of nudging towards center */
    wall,                          /* spring constant of walls */
    margin,                        /* space allowed around [-1,1]^3 for pnts */
    tlThresh, tlSoft, tlStep,      /* tractlet formation parameters */
    minMeanVel;                    /* stop if mean velocity drops below this */
  int tlFrenet,                    /* use Frenet frames for tractlet forces */
    singleBin,                     /* disable binning (for debugging) */
    driftCorrect,                  /* prevent sliding near anisotropy edges */
    verbose;                       /* blah blah blah */
  unsigned int seed,               /* seed value for airSrand48 */
    tlNumStep,                     /* max # points on each tractlet half */
    binIncr,                       /* increment for per-bin thing airArray */
    numThing,                      /* number things to start simulation w/ */
    numThread,                     /* number of threads to use */
    numStage,                      /* number of stages */
    minIter,                       /* if non-zero, min number of iterations */
    maxIter,                       /* if non-zero, max number of iterations */
    snap;                          /* if non-zero, interval between iterations
                                      at which output snapshots are saved */
  pushForce *force;                /* force function to use */
  NrrdKernelSpec *ksp00,           /* for sampling tensor field */
    *ksp11;                        /* for gradient of mask */
  push_t stageParm[PUSH_STAGE_MAXNUM][PUSH_STAGE_PARM_MAXNUM];
  pushProcess process[PUSH_STAGE_MAXNUM]; /* the function for each stage */
  /* INTERNAL -------------------------- */
  Nrrd *nten,                      /* 3D image of 3D masked tensors */
    *ninv,                         /* pre-computed inverse of nten */
    *nmask;                        /* mask image from nten */
  gageContext *gctx;               /* gage context around nten and nmask */
  gagePerVolume *tpvl, *ipvl;      /* gage pervolumes around nten and ninv */
  tenFiberContext *fctx;           /* tenFiber context around nten */
  int finished;                    /* used to signal all threads to return */
  unsigned int dimIn,              /* dimension (2 or 3) of input */
    binsEdge,                      /* # bins along edge of grid */
    numBin,                        /* total # bins in grid */
    stageIdx,                      /* stage currently undergoing processing */
    binIdx;                        /* *next* bin of points needing to be
                                      processed.  Stage is done when
                                      binIdx == numBin */
  pushBin *bin;                    /* volume of bins (see binsEdge, numBin) */
  double maxDist,                  /* max distance btween interacting points */
    maxEval, meanEval,             /* max and mean principal eval in field */
    minPos[3],                     /* lower corner of world position */
    maxPos[3],                     /* upper corner of world position */
    meanVel,                       /* latest mean velocity of particles */
    time0, time1;                  /* time at start and stop of run */
  pushTask **task;                 /* dynamically allocated array of tasks */
  airThreadMutex *binMutex;        /* mutex around bin */
  airThreadBarrier *stageBarrierA, /* barriers between stages */
    *stageBarrierB;
  /* OUTPUT ---------------------------- */
  double time;                     /* how long it took to run */
  unsigned int iter;               /* how many iterations were needed */
  Nrrd *noutPos,                   /* list of 2D or 3D positions */
    *noutTen;                      /* list of 2D or 3D masked tensors */
} pushContext;

/* defaultsPush.c */
PUSH_EXPORT2 const char *pushBiffKey;

/* methodsPush.c */
PUSH_EXPORT pushThing *pushThingNew(unsigned int numVert);
PUSH_EXPORT pushThing *pushThingNix(pushThing *thg);
PUSH_EXPORT void pushBinInit(pushBin *bin, unsigned int incr);
PUSH_EXPORT void pushBinDone(pushBin *bin);
PUSH_EXPORT pushContext *pushContextNew(void);
PUSH_EXPORT pushContext *pushContextNix(pushContext *pctx);

/* forces.c */
PUSH_EXPORT pushForce *pushForceParse(const char *str);
PUSH_EXPORT pushForce *pushForceNix(pushForce *force);
PUSH_EXPORT2 hestCB *pushHestForce;

/* corePush.c */
PUSH_EXPORT int pushStart(pushContext *pctx);
PUSH_EXPORT int pushIterate(pushContext *pctx);
PUSH_EXPORT int pushRun(pushContext *pctx);
PUSH_EXPORT int pushFinish(pushContext *pctx);

/* binning.c */
PUSH_EXPORT int pushBinThingAdd(pushContext *pctx, pushThing *thing);
PUSH_EXPORT int pushBinPointAdd(pushContext *pctx, pushPoint *point);
PUSH_EXPORT void pushBinAllNeighborSet(pushContext *pctx);
PUSH_EXPORT int pushRebin(pushContext *pctx);

/* action.c */
PUSH_EXPORT int pushOutputGet(Nrrd *nPosOut, Nrrd *nTenOut, Nrrd *nStnOut, 
                              pushContext *pctx);

#ifdef __cplusplus
}
#endif

#endif /* PUSH_HAS_BEEN_INCLUDED */

