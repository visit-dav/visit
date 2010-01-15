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

#ifndef HOOVER_HAS_BEEN_INCLUDED
#define HOOVER_HAS_BEEN_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/limn.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(hoover_EXPORTS) || defined(teem_EXPORTS)
#    define HOOVER_EXPORT extern __declspec(dllexport)
#  else
#    define HOOVER_EXPORT extern __declspec(dllimport)
#  endif
#define HOOVER_EXPORT2 HOOVER_EXPORT
#else /* TEEM_STATIC || UNIX */
# if __GNUC__ >= 4 && (defined(hoover_EXPORTS) || defined(teem_EXPORTS))
#  define HOOVER_EXPORT __attribute__ ((visibility("default")))
#  define HOOVER_EXPORT2 extern __attribute__ ((visibility("default")))
# else
#  define HOOVER_EXPORT extern
#  define HOOVER_EXPORT2 extern
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define HOOVER hooverBiffKey

#define HOOVER_THREAD_MAX 512

/* 
******** the mess of typedefs for callbacks used below
*/
typedef int (hooverRenderBegin_t)(void **renderP,
                                  void *user);
typedef int (hooverThreadBegin_t)(void **threadP,
                                  void *render,
                                  void *user,
                                  int whichThread);
typedef int (hooverRayBegin_t)(void *thread,
                               void *render,
                               void *user,
                               int uIndex,    /* img coords of current ray */
                               int vIndex, 
                               double rayLen, /* length of ray segment between
                                                 near and far planes,  */
                               double rayStartWorld[3],
                               double rayStartIndex[3],
                               double rayDirWorld[3],
                               double rayDirIndex[3]);
typedef double (hooverSample_t)(void *thread,
                                void *render,
                                void *user,
                                int num,    /* which sample this is, 0-based */
                                double rayT,/* position along ray */
                                int inside, /* sample is inside the volume */
                                double samplePosWorld[3],
                                double samplePosIndex[3]);
typedef int (hooverRayEnd_t)(void *thread,
                             void *render,
                             void *user);
typedef int (hooverThreadEnd_t)(void *thread,
                                void *render,
                                void *user);
typedef int (hooverRenderEnd_t)(void *rend, void *user);

/*
******** hooverContext struct
**
** Everything that hooverRender() needs to do its thing, and no more.
** This is all read-only informaiton.
** 1) camera information
** 3) volume information
** 4) image information
** 5) opaque "user information" pointer
** 6) stuff about multi-threading
** 7) the callbacks
*/
typedef struct {

  /******** 1) camera information */
  limnCamera *cam;           /* camera info */

  /******** 2) volume information: size and spacing, centering */
  int volSize[3];            /* X,Y,Z resolution of volume */
  double volSpacing[3];      /* distance between samples in X,Y,Z direction */
  int volCentering;          /* either nrrdCenterNode or nrrdCenterCell */
  
  /******** 3) image information: dimensions + centering */
  int imgSize[2],            /* # samples of image along U and V axes */
    imgCentering;            /* either nrrdCenterNode or nrrdCenterCell */
  
  /******** 4) opaque "user information" pointer */
  void *user;                /* passed to all callbacks */

  /******** 5) stuff about multi-threading */
  int numThreads,            /* number of threads to spawn per rendering */
    workIdx;                 /* next work assignment (such as a scanline) */
  airThreadMutex *workMutex; /* mutex around work assignment */
  
  /*
  ******* 6) the callbacks 
  **
  ** The conceptual ordering of these callbacks is as they are listed
  ** below.  For example, rayBegin and rayEnd are called multiple
  ** times between threadBegin and threadEnd, and so on.  All of these
  ** are initialized to one of the stub functions provided by hoover.  
  **
  ** A non-zero return of any of these indicates error. Which callback
  ** failed is represented by the return value of hooverRender(), the
  ** return value from the callback is stored in *errCodeP by
  ** hooverRender(), and the problem thread number is stored in
  ** *errThreadP.
  */

  /* 
  ** renderBegin()
  **
  ** called once at beginning of whole rendering, and
  ** *renderP is passed to all following calls as "render".
  ** Any mechanisms for inter-thread communication go nicely in 
  ** the render.
  **
  ** int (*renderBegin)(void **renderP, void *user);
  */
  hooverRenderBegin_t *renderBegin;
  
  /* 
  ** threadBegin() 
  **
  ** called once per thread, and *threadP is passed to all
  ** following calls as "thread".
  **
  ** int (*threadBegin)(void **threadP, void *render, void *user,
  **                    int whichThread);
  */
  hooverThreadBegin_t *threadBegin;
  
  /*
  ** rayBegin()
  **
  ** called once at the beginning of each ray.  This function will be
  ** called regardless of whether the ray actually intersects the
  ** volume, but this will change in the future.
  **
  ** int (*rayBegin)(void *thread, void *render, void *user,
  **                 int uIndex, int vIndex, 
  **                 double rayLen,
  **                 double rayStartWorld[3], double rayStartIndex[3],
  **                 double rayDirWorld[3], double rayDirIndex[3]);
  */
  hooverRayBegin_t *rayBegin;

  /* 
  ** sample()
  **
  ** called once per sample along the ray, and the return value is
  ** used to indicate how far to increment the ray position for the
  ** next sample.  Negative values back you up.  A return of 0.0 is
  ** taken to mean a non-erroneous ray termination, a return of NaN is
  ** taken to mean an error condition.  It is the user's
  ** responsibility to store an error code or whatever they want
  ** somewhere accessible.
  **
  ** This is not a terribly flexible scheme (don't forget, this is
  ** hoover) in that it imposes some constraints on how multi-threading
  ** can work: one thread can not render multiple rays
  ** simulatenously.  If there were more args to sample() (like a
  ** ray, or an integral rayIndex), then this would be possible,
  ** but it would mean that _hooverThreadBody() would have to
  ** implement all the smarts about which samples belong on which rays,
  ** and which rays belong with which threads.
  **
  ** At some point now or in the future, an effort will be made to
  ** never call this function if the ray does not in fact intersect
  ** the volume at all.
  **
  ** double (*sample)(void *thread, void *render, void *user,
  **                  int num, double rayT, int inside,
  **                  double samplePosWorld[3],
  **                  double samplePosIndex[3]);
  */
  hooverSample_t *sample;

  /*
  ** rayEnd()
  ** 
  ** called at the end of the ray.  The end of a ray is when:
  ** 1) sample returns 0.0, or,
  ** 2) the sample location goes behind far plane
  **
  ** int (*rayEnd)(void *thread, void *render, void *user);
  */
  hooverRayEnd_t *rayEnd;

  /* 
  ** threadEnd()
  **
  ** called at end of thread
  **
  ** int (*threadEnd)(void *thread, void *render, void *user);
  */
  hooverThreadEnd_t *threadEnd;
  
  /* 
  ** renderEnd()
  ** 
  ** called once at end of whole rendering
  **
  ** int (*renderEnd)(void *rend, void *user);
  */
  hooverRenderEnd_t *renderEnd;

} hooverContext;

/*
******** hooverErr... enum
**
** possible returns from hooverRender.
** hooverErrNone: no error, all is well: 
** hooverErrInit: error detected in hoover, call biffGet(HOOVER)
** otherwise, return indicates which call-back had trouble
*/
enum {
  hooverErrNone,
  hooverErrInit,           /* call biffGet(HOOVER) */
  hooverErrRenderBegin,
  hooverErrThreadCreate,
  hooverErrThreadBegin,
  hooverErrRayBegin,
  hooverErrSample,
  hooverErrRayEnd,
  hooverErrThreadEnd,
  hooverErrThreadJoin,
  hooverErrRenderEnd,
  hooverErrLast
};

/* defaultsHoover.c */
HOOVER_EXPORT2 const char *hooverBiffKey;
HOOVER_EXPORT2 int hooverDefVolCentering;
HOOVER_EXPORT2 int hooverDefImgCentering;

/* methodsHoover.c */
HOOVER_EXPORT hooverContext *hooverContextNew();
HOOVER_EXPORT int hooverContextCheck(hooverContext *ctx);
HOOVER_EXPORT void hooverContextNix(hooverContext *ctx);

/* rays.c */
HOOVER_EXPORT int hooverRender(hooverContext *ctx,
                               int *errCodeP, int *errThreadP);

/* stub.c */
HOOVER_EXPORT2 hooverRenderBegin_t hooverStubRenderBegin;
HOOVER_EXPORT2 hooverThreadBegin_t hooverStubThreadBegin;
HOOVER_EXPORT2 hooverRayBegin_t hooverStubRayBegin;
HOOVER_EXPORT2 hooverSample_t hooverStubSample;
HOOVER_EXPORT2 hooverRayEnd_t hooverStubRayEnd;
HOOVER_EXPORT2 hooverThreadEnd_t hooverStubThreadEnd;
HOOVER_EXPORT2 hooverRenderEnd_t hooverStubRenderEnd;

#ifdef __cplusplus
}
#endif

#endif /* HOOVER_HAS_BEEN_INCLUDED */
