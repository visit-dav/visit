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

#ifndef COIL_HAS_BEEN_INCLUDED
#define COIL_HAS_BEEN_INCLUDED

/* NOTE: there are various types that should be unsigned instead of
** signed; it may be a priority to fix this at a later date 
*/

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/ten.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(coil_EXPORTS) || defined(teem_EXPORTS)
#    define COIL_EXPORT extern __declspec(dllexport)
#  else
#    define COIL_EXPORT extern __declspec(dllimport)
#  endif
#define COIL_EXPORT2 COIL_EXPORT
#else /* TEEM_STATIC || UNIX */
# if __GNUC__ >= 4 && (defined(coil_EXPORTS) || defined(teem_EXPORTS))
#  define COIL_EXPORT __attribute__ ((visibility("default")))
#  define COIL_EXPORT2 extern __attribute__ ((visibility("default")))
# else
#  define COIL_EXPORT extern
#  define COIL_EXPORT2 extern
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define COIL coilBiffKey

/*
******** coil_t
** 
** this is the very crude means by which you can control the type
** of values that coil works with: "float" or "double".  It is an
** unfortunate but greatly simplifying restriction that this type
** is used for all kinds of volumes, and all methods of filtering
**
** So: choose double by defining TEEM_COIL_TYPE_DOUBLE and float
** otherwise.
*/

#ifdef TEEM_COIL_TYPE_DOUBLE
typedef double coil_t;
#define coil_nrrdType nrrdTypeDouble
#define COIL_TYPE_FLOAT 0
#else
typedef float coil_t;
#define coil_nrrdType nrrdTypeFloat
#define COIL_TYPE_FLOAT 1
#endif

/*
******** #define COIL_PARMS_NUM
**
** maximum number of parameters that may be needed by any coil-driven
** filtering method
*/
#define COIL_PARMS_NUM 5

/*
******** coilMethodType* enum
**
** enumerated possibilities for different filtering methods
*/
enum {
  coilMethodTypeUnknown,            /* 0 */
  coilMethodTypeTesting,            /* 1: basically a no-op */
  coilMethodTypeHomogeneous,        /* 2 */
  coilMethodTypePeronaMalik,        /* 3 */
  coilMethodTypeModifiedCurvature,  /* 4 */
  coilMethodTypeCurvatureFlow,      /* 5 */
  coilMethodTypeSelf,               /* 6 */
  coilMethodTypeFinish,             /* 7 */
  coilMethodTypeLast
};
#define COIL_METHOD_TYPE_MAX           7

/*
******** coilMethod struct
**
** someday, there will be total orthogonality between kind and method.
** until then, this will have only a portion of the things relating to
** running one method, regardless of kind
*/
typedef struct {
  char name[AIR_STRLEN_SMALL];
  int type;                         /* from coilMethodType* enum */
  int numParm;                      /* number of parameters we need */
} coilMethod;

/*
******** coilKindType* enum
**
** enumerated possibilities for different kinds
*/
enum {
  coilKindTypeUnknown,              /* 0 */
  coilKindTypeScalar,               /* 1 */
  coilKindType3Color,               /* 2 */
  coilKindType7Tensor,              /* 3 */
  coilKindTypeLast
};
#define COIL_KIND_TYPE_MAX             3

/*
******** coilKind struct
**
** yes, there is some redunancy with the gageKind, but the the main
** reason is that its significantly easier to implement meaningful
** per-sample filtering of various quantities, than it is to do
** gage-style convolution-based measurements at arbitrary locations.
** So, there will probably be significantly more coilKinds than
** there are gageKinds.  The two kind systems may play closer together
** at some point in the future where appropriate.
*/
typedef struct {
  char name[AIR_STRLEN_SMALL];      /* short identifying string for kind */
  unsigned int valLen;              /* number of scalars per data point
                                       1 for plain scalars (baseDim=0),
                                       or something else (baseDim=1) */
                                    /* all the available methods */
  void (*filter[COIL_METHOD_TYPE_MAX+1])(coil_t *delta, coil_t **iv3,
                                         double spacing[3],
                                         double parm[COIL_PARMS_NUM]);
  void (*update)(coil_t *val, coil_t *delta); /* how to apply update */
} coilKind;

struct coilContext_t;

/*
******** coilTask
**
** passed to all worker threads
*/
typedef struct {
  struct coilContext_t *cctx;      /* parent's context */
  airThread *thread;               /* my thread */
  unsigned int threadIdx;          /* which thread am I */
  coil_t *_iv3,                    /* underlying value cache */
    **iv3;                         /* short array of pointers into 2-D value
                                      caches, in which the order is based on
                                      the volume order:
                                      values, then Y, then Z */
                                   /* how to fill iv3 */
  void (*iv3Fill)(coil_t **iv3, coil_t *here, unsigned int radius, int valLen,
                  int x0, int y0, int z0, int sizeX, int sizeY, int sizeZ);
  void *returnPtr;                 /* for airThreadJoin */
} coilTask;

/*
******** coilContext struct
**
** bag of stuff relating to filtering one volume
*/
typedef struct coilContext_t {
  /* ---------- input */
  const Nrrd *nin;                 /* input volume (converted to type coil_t
                                      in nvol, below) */
  const coilKind *kind;            /* what kind of volume is nin */
  const coilMethod *method;        /* what method of filtering to use */
  unsigned int radius,             /* how big a neighborhood to look at when
                                      doing filtering (use 1 for 3x3x3 size) */
    numThreads;                    /* number of threads to enlist */
  int verbose;                     /* blah blah blah */
  double parm[COIL_PARMS_NUM];     /* all the parameters used to control the 
                                      action of the filtering.  The timestep is
                                      probably the first value. */
  /* ---------- internal */
  size_t size[3],                  /* size of volume */
    nextSlice;                     /* global indicator of next slice needing
                                      to be processed, either in filter or
                                      in update stage.  Stage is done when
                                      nextSlice == size[2] */
  double spacing[3];               /* sample spacings we'll use- we perhaps 
                                      should be using a gageShape, but this is
                                      actually all we really need... */
  Nrrd *nvol;                      /* an interleaved volume of (1st) the last
                                      filtering result, and (2nd) the update
                                      values from the current iteration */
  int finished,                    /* used to signal all threads to return */
    todoFilter, todoUpdate;        /* flags to signal which is scheduled to
                                      come next, used as part of doling out
                                      slices to workers */
  airThreadMutex *nextSliceMutex;  /* mutex around nextSlice (and effectively,
                                      also the "todo" flags above) */
  coilTask **task;                 /* dynamically allocated array of tasks */
  airThreadBarrier *filterBarrier, /* so that thread 0 can see if filtering
                                      should go onward, and set "finished" */
    *updateBarrier;                /* after the update values have been
                                      applied to current values */
} coilContext;

/* defaultsCoil.c */
COIL_EXPORT2 const char *coilBiffKey;
COIL_EXPORT2 int coilDefaultRadius;

/* enumsCoil.c */
COIL_EXPORT2 airEnum *coilMethodType;
COIL_EXPORT2 airEnum *coilKindType;

/* scalarCoil.c */
COIL_EXPORT2 const coilKind *coilKindScalar;
COIL_EXPORT2 const coilKind *coilKindArray[COIL_KIND_TYPE_MAX+1];

/* tensorCoil.c */
COIL_EXPORT2 const coilKind _coilKind7Tensor; /* no privateCoil.h */
COIL_EXPORT2 const coilKind *coilKind7Tensor;

/* realmethods.c */
COIL_EXPORT2 const coilMethod *coilMethodTesting;
COIL_EXPORT2 const coilMethod *coilMethodIsotropic;
COIL_EXPORT2 const coilMethod *coilMethodArray[COIL_METHOD_TYPE_MAX+1];

/* methodsCoil.c (sorry, confusing name!) */
COIL_EXPORT coilContext *coilContextNew();
COIL_EXPORT int coilVolumeCheck(const Nrrd *nin, const coilKind *kind);
COIL_EXPORT int coilContextAllSet(coilContext *cctx, const Nrrd *nin,
                                  const coilKind *kind,
                                  const coilMethod *method,
                                  unsigned int radius, unsigned int numThreads,
                                  int verbose, double parm[COIL_PARMS_NUM]);
COIL_EXPORT int coilOutputGet(Nrrd *nout, coilContext *cctx);
COIL_EXPORT coilContext *coilContextNix(coilContext *cctx);

/* coreCoil.c */
COIL_EXPORT int coilStart(coilContext *cctx);
COIL_EXPORT int coilIterate(coilContext *cctx, int numIterations);
COIL_EXPORT int coilFinish(coilContext *cctx);

#ifdef __cplusplus
}
#endif

#endif /* COIL_HAS_BEEN_INCLUDED */
