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

#ifndef MOSS_HAS_BEEN_INCLUDED
#define MOSS_HAS_BEEN_INCLUDED

/* NOTE: this library has not undergone the changes as other Teem
   libraries in order to make sure that array lengths and indices
   are stored in unsigned types */

#include <math.h>

#include <teem/air.h>
#include <teem/hest.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(moss_EXPORTS) || defined(teem_EXPORTS)
#    define MOSS_EXPORT extern __declspec(dllexport)
#  else
#    define MOSS_EXPORT extern __declspec(dllimport)
#  endif
#define MOSS_EXPORT2 MOSS_EXPORT
#else /* TEEM_STATIC || UNIX */
# if __GNUC__ >= 4 && (defined(moss_EXPORTS) || defined(teem_EXPORTS))
#  define MOSS_EXPORT __attribute__ ((visibility("default")))
#  define MOSS_EXPORT2 extern __attribute__ ((visibility("default")))
# else
#  define MOSS_EXPORT extern
#  define MOSS_EXPORT2 extern
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MOSS mossBiffKey

#define MOSS_NCOL(img) (3 == (img)->dim ? (img)->axis[0].size : 1)
#define MOSS_AXIS0(img) (3 == (img)->dim ? 1 : 0)
#define MOSS_SX(img) (3 == (img)->dim \
                      ? (img)->axis[1].size \
                      : (img)->axis[0].size )
#define MOSS_SY(img) (3 == (img)->dim \
                      ? (img)->axis[2].size \
                      : (img)->axis[1].size )

enum {
  mossFlagUnknown=-1,  /* -1: nobody knows */
  mossFlagImage,       /*  0: image being sampled */
  mossFlagKernel,      /*  1: kernel(s) used for sampling */
  mossFlagLast
};
#define MOSS_FLAG_NUM      2

typedef struct {
  Nrrd *image;                         /* the image to sample */
  const NrrdKernel *kernel;            /* which kernel to use on both axes */
  double kparm[NRRD_KERNEL_PARMS_NUM]; /* kernel arguments */
  float *ivc;                          /* intermediate value cache */
  double *xFslw, *yFslw;               /* filter sample locations->weights */
  int fdiam, ncol;                     /* filter diameter; ivc is allocated
                                          for (fdiam+1) x (fdiam+1) x ncol
                                          doubles, with that axis ordering */
  int *xIdx, *yIdx;                    /* arrays for x and y coordinates,
                                          both allocated for fdiam */
  float *bg;                           /* background color */
  int boundary;                        /* from nrrdBoundary* enum */
  int flag[MOSS_FLAG_NUM];             /* I'm a flag-waving struct */
} mossSampler;

/* defaultsMoss.c */
MOSS_EXPORT2 const char *mossBiffKey;
MOSS_EXPORT2 int mossDefBoundary;
MOSS_EXPORT2 int mossDefCenter;
MOSS_EXPORT2 int mossVerbose;

/* methodsMoss.c */
MOSS_EXPORT mossSampler *mossSamplerNew();
MOSS_EXPORT int mossSamplerFill(mossSampler *smplr, int fdiam, int ncol);
MOSS_EXPORT void mossSamplerEmpty(mossSampler *smplr);
MOSS_EXPORT mossSampler *mossSamplerNix(mossSampler *smplr);
MOSS_EXPORT int mossImageCheck(Nrrd *image);
MOSS_EXPORT int mossImageAlloc(Nrrd *image, int type,
                               int sx, int sy, int ncol);

/* sampler.c */
MOSS_EXPORT int mossSamplerImageSet(mossSampler *smplr,
                                    Nrrd *image, float *bg);
MOSS_EXPORT int mossSamplerKernelSet(mossSampler *smplr,
                                     const NrrdKernel *kernel, double *kparm);
MOSS_EXPORT int mossSamplerUpdate(mossSampler *smplr);
MOSS_EXPORT int mossSamplerSample(float *val, mossSampler *smplr,
                                  double xPos, double yPos);

/* hestMoss.c */
MOSS_EXPORT2 hestCB *mossHestTransform;
MOSS_EXPORT2 hestCB *mossHestOrigin;

/* xform.c */
MOSS_EXPORT void mossMatPrint(FILE *f, double *mat);
MOSS_EXPORT double *mossMatRightMultiply(double *mat, double *x);
MOSS_EXPORT double *mossMatLeftMultiply (double *mat, double *x);
MOSS_EXPORT double *mossMatInvert(double *inv, double *mat);
MOSS_EXPORT double *mossMatIdentitySet(double *mat);
MOSS_EXPORT double *mossMatTranslateSet(double *mat, double tx, double ty);
MOSS_EXPORT double *mossMatRotateSet(double *mat, double angle);
MOSS_EXPORT double *mossMatFlipSet(double *mat, double angle);
MOSS_EXPORT double *mossMatShearSet(double *mat, double angleFixed,
                                    double amount);
MOSS_EXPORT double *mossMatScaleSet(double *mat, double sx, double sy);
MOSS_EXPORT void mossMatApply(double *ox, double *oy, double *mat,
                              double ix, double iy);
MOSS_EXPORT int mossLinearTransform(Nrrd *nout, Nrrd *nin, float *bg,
                                    double *mat,
                                    mossSampler *msp,
                                    double xMin, double xMax,
                                    double yMin, double yMax,
                                    int sx, int sy);


#ifdef __cplusplus
}
#endif

#endif /* MOSS_HAS_BEEN_INCLUDED */
