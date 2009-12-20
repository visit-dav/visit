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

#ifndef TEN_HAS_BEEN_INCLUDED
#define TEN_HAS_BEEN_INCLUDED

#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/unrrdu.h>
#include <teem/dye.h>
#include <teem/gage.h>
#include <teem/limn.h>
#include <teem/echo.h>

#include "tenMacros.h"

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(ten_EXPORTS) || defined(teem_EXPORTS)
#    define TEN_EXPORT extern __declspec(dllexport)
#  else
#    define TEN_EXPORT extern __declspec(dllimport)
#  endif
#define TEN_EXPORT2 TEN_EXPORT
#else /* TEEM_STATIC || UNIX */
# if __GNUC__ >= 4 && (defined(ten_EXPORTS) || defined(teem_EXPORTS))
#  define TEN_EXPORT __attribute__ ((visibility("default")))
#  define TEN_EXPORT2 extern __attribute__ ((visibility("default")))
# else
#  define TEN_EXPORT extern
#  define TEN_EXPORT2 extern
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TEN tenBiffKey

/*
****** tenAniso* enum
**
** the different scalar values that can describe a tensor.  Nearly
** all of these are anisotropy metrics, but with time this has become
** a convenient way to present any scalar invariant (such as trace),
** and even the individual eigenvalues
**
** keep in sync:
** aniso.c: _tenAnisoEval_X_f()
** aniso.c: _tenAnisoEval_f[]
** aniso.c: tenAnisoCalc_f()
** enumsTen.c: tenAniso  
*/
enum {
  tenAnisoUnknown, /*  0: nobody knows */
  tenAniso_Cl1,    /*  1: Westin's linear (first version) */
  tenAniso_Cp1,    /*  2: Westin's planar (first version) */
  tenAniso_Ca1,    /*  3: Westin's linear + planar (first version) */
  tenAniso_Cs1,    /*  4: Westin's spherical (first version) */
  tenAniso_Ct1,    /*  5: gk's anisotropy type (first version) */
  tenAniso_Cl2,    /*  6: Westin's linear (second version) */
  tenAniso_Cp2,    /*  7: Westin's planar (second version) */
  tenAniso_Ca2,    /*  8: Westin's linear + planar (second version) */
  tenAniso_Cs2,    /*  9: Westin's spherical (second version) */
  tenAniso_Ct2,    /* 10: gk's anisotropy type (second version) */
  tenAniso_RA,     /* 11: Bass+Pier's relative anisotropy */
  tenAniso_FA,     /* 12: (Bass+Pier's fractional anisotropy)/sqrt(2) */
  tenAniso_VF,     /* 13: volume fraction = 1-(Bass+Pier's volume ratio) */
  tenAniso_B,      /* 14: linear term in cubic characteristic polynomial */
  tenAniso_Q,      /* 15: radius of root circle is 2*sqrt(Q) */
  tenAniso_R,      /* 16: half of third moment of eigenvalues */
  tenAniso_S,      /* 17: frobenius norm, squared */
  tenAniso_Skew,   /* 18: R/sqrt(2*Q^3) */
  tenAniso_Mode,   /* 19: 3*sqrt(6)*det(dev)/norm(dev) = sqrt(2)*skew */
  tenAniso_Th,     /* 20: acos(sqrt(2)*skew)/3 */
  tenAniso_Cz,     /* 21: Zhukov's invariant-based anisotropy metric */
  tenAniso_Det,    /* 22: plain old determinant */
  tenAniso_Tr,     /* 23: plain old trace */
  tenAniso_eval0,  /* 24: largest eigenvalue */
  tenAniso_eval1,  /* 25: middle eigenvalue */
  tenAniso_eval2,  /* 26: smallest eigenvalue */
  tenAnisoLast
};
#define TEN_ANISO_MAX 26

/*
******** tenGlyphType* enum
** 
** the different types of glyphs that may be used for tensor viz
*/
enum {
  tenGlyphTypeUnknown,    /* 0: nobody knows */
  tenGlyphTypeBox,        /* 1 */
  tenGlyphTypeSphere,     /* 2 */
  tenGlyphTypeCylinder,   /* 3 */
  tenGlyphTypeSuperquad,  /* 4 */
  tenGlyphTypeLast
};
#define TEN_GLYPH_TYPE_MAX   4

/*
******** tenGlyphParm struct
**
** all input parameters to tenGlyphGen
*/
typedef struct {
  int verbose; 
  
  /* glyphs will be shown at samples that have confidence >= confThresh,
     and anisotropy anisoType >= anisoThresh, and if nmask is non-NULL,
     then the corresponding mask value must be >= maskThresh.  If 
     onlyPositive, then samples with a non-positive eigenvalue will 
     be skipped, regardless of their purported anisotropy */
  Nrrd *nmask;
  int anisoType, onlyPositive;
  float confThresh, anisoThresh, maskThresh;

  /* glyphs have shape glyphType and size glyphScale. Superquadrics
     are tuned by sqdSharp, and things that must polygonalize do so
     according to facetRes.  Postscript rendering of glyph edges is
     governed by edgeWidth[] */
  int glyphType, facetRes;
  float glyphScale, sqdSharp;
  float edgeWidth[5];  /* 0: contour, 1: front crease, 2: front non-crease */

  /* glyphs are colored by eigenvector colEvec with the standard XYZ-RGB
     colormapping, with maximal saturation colMaxSat (use 0.0 to turn off
     coloring).  Saturation is modulated by anisotropy colAnisoType, to a
     degree controlled by colAnisoModulate (if 0, saturation is not at all
     modulated by anistropy).  Post-saturation, there is a per-channel
     gamma of colGamma. */
  int colEvec, colAnisoType;
  float colMaxSat, colIsoGray, colGamma, colAnisoModulate, ADSP[4];

  /* if doSlice, a slice of anisotropy sliceAnisoType will be depicted
     in grayscale as a sheet of grayscale squares, one per sample. As
     with glyphs, these are thresholded by confThresh and maskThresh
     (but not anisoThresh).  Things can be lightened up with a
     sliceGamma > 1, after the slice's gray values have been mapped from
     [0,1] to [sliceBias,1]. The squares will be at their corresponding
     sample locations, but offset by sliceOffset */
  unsigned int sliceAxis;
  size_t slicePos;
  int doSlice, sliceAnisoType;
  float sliceOffset, sliceBias, sliceGamma;
} tenGlyphParm;

#define TEN_ANISO_DESC \
  "All the Westin metrics come in two versions.  Currently supported:\n " \
  "\b\bo \"cl1\", \"cl2\": Westin's linear\n " \
  "\b\bo \"cp1\", \"cp2\": Westin's planar\n " \
  "\b\bo \"ca1\", \"ca2\": Westin's linear + planar\n " \
  "\b\bo \"cs1\", \"cs2\": Westin's spherical (1-ca)\n " \
  "\b\bo \"ct1\", \"ct2\": GK's anisotropy type (cp/ca)\n " \
  "\b\bo \"ra\": Basser/Pierpaoli relative anisotropy/sqrt(2)\n " \
  "\b\bo \"fa\": Basser/Pierpaoli fractional anisotropy\n " \
  "\b\bo \"vf\": volume fraction = 1-(Basser/Pierpaoli volume ratio)\n " \
  "\b\bo \"tr\": trace"

/*
******** tenGage* enum
** 
** all the possible queries supported in the tenGage gage kind
** various properties of the quantities below (eigenvalues = v1, v2, v3):
** eigenvalue cubic equation: v^3 + A*v^2 + B*v + C = 0
** Trace = v1 + v2 + v3 = -A
** B = v1*v2 + v1*v3 + v2*v3
** Det = v1*v2*v3 = -C
** S = v1*v1 + v2*v2 + v3*v3
** Q = (S-B)/9 = variance({v1,v2,v3})/2 = (RootRadius/2)^2
** FA = 3*sqrt(Q/S)
** R = (9*A*B - 2*A^3 - 27*C)/54 
     = (5*A*B - 2*A*S - 27*C)/54 = thirdmoment({v1,v2,v3})/2
** P = arccos(R/sqrt(Q)^3)/3 = phase angle of cubic solution
**
** NOTE: currently tenGage knows *nothing* about nrrd->measurementFrame!
** You probably want to call tenMeasurementFrameReduce() first.
**
** Hey- the problem with adding the RGB eigenvector coloring to the
** capability of tenGage is that because this is visualization, you
** can't easily control whether the measurement frame is applied, if
** known- in that sense the RGB info is uniquely different from the
** other vector and tensor items that can be queried ... so after a 
** brief appearance here the RGB evec coloring was removed.  The
** gagePerVolume->data field that it motivated has rightly remained.
**
** !!! Changes to this list need to be propogated to tenGage.c's
** !!! _tenGageTable[] and _tenGageAnswer(), 
** !!! and to enumsTen.c's tenGage airEnum.
**
*/
enum {
  tenGageUnknown = -1,  /* -1: nobody knows */

  tenGageTensor,        /*  0: "t", the reconstructed tensor: GT[7] */
  tenGageConfidence,    /*  1: "c", first of seven tensor values: GT[1] */
  tenGageTrace,         /*  2: "tr", trace of tensor: GT[1] */
  tenGageB,             /*  3: "b": GT[1] */
  tenGageDet,           /*  4: "det", determinant of tensor: GT[1] */
  tenGageS,             /*  5: "s", square of frobenius norm: GT[1] */
  tenGageQ,             /*  6: "q", (S - B)/9: GT[1] */
  tenGageFA,            /*  7: "fa", fractional anisotropy: GT[1] */
  tenGageR,             /*  8: "r", 9*A*B - 2*A^3 - 27*C: GT[1] */
  tenGageTheta,         /*  9: "th", arccos(R/sqrt(Q^3))/AIR_PI: GT[1] */

  tenGageEval,          /* 10: "eval", all eigenvalues of tensor : GT[3] */
  tenGageEval0,         /* 11: "eval0", major eigenvalue of tensor : GT[1] */
  tenGageEval1,         /* 12: "eval1", medium eigenvalue of tensor : GT[1] */
  tenGageEval2,         /* 13: "eval2", minor eigenvalue of tensor : GT[1] */
  tenGageEvec,          /* 14: "evec", major eigenvectors of tensor: GT[9] */
  tenGageEvec0,         /* 15: "evec0", major eigenvectors of tensor: GT[3] */
  tenGageEvec1,         /* 16: "evec1", medium eigenvectors of tensor: GT[3] */
  tenGageEvec2,         /* 17: "evec2", minor eigenvectors of tensor: GT[3] */

  tenGageTensorGrad,    /* 18: "tg", all tensor component gradients, starting
                               with the confidence gradient: GT[21] */
  tenGageTensorGradMag, /* 19: "tgm", actually a 3-vector of tensor 
                               gradient norms, one for each axis: GT[3] */
  tenGageTensorGradMagMag, /* 20: "tgmm", single scalar magnitude: GT[1] */
  
  tenGageTraceGradVec,  /* 21: "trgv": gradient (vector) of trace: GT[3] */
  tenGageTraceGradMag,  /* 22: "trgm": gradient magnitude of trace: GT[1] */
  tenGageTraceNormal,   /* 23: "trn": normal of trace: GT[3] */
  
  tenGageBGradVec,      /* 24: "bgv", gradient (vector) of B: GT[3] */
  tenGageBGradMag,      /* 25: "bgm", gradient magnitude of B: GT[1] */
  tenGageBNormal,       /* 26: "bn", normal of B: GT[3] */

  tenGageDetGradVec,    /* 27: "detgv", gradient (vector) of Det: GT[3] */
  tenGageDetGradMag,    /* 28: "detgm", gradient magnitude of Det: GT[1] */
  tenGageDetNormal,     /* 29: "detn", normal of Det: GT[3] */

  tenGageSGradVec,      /* 30: "sgv", gradient (vector) of S: GT[3] */
  tenGageSGradMag,      /* 31: "sgm", gradient magnitude of S: GT[1] */
  tenGageSNormal,       /* 32: "sn", normal of S: GT[3] */

  tenGageQGradVec,      /* 33: "qgv", gradient vector of Q: GT[3] */
  tenGageQGradMag,      /* 34: "qgm", gradient magnitude of Q: GT[1] */
  tenGageQNormal,       /* 35: "qn", normalized gradient of Q: GT[3] */

  tenGageFAGradVec,     /* 36: "fagv", gradient vector of FA: GT[3] */
  tenGageFAGradMag,     /* 37: "fagm", gradient magnitude of FA: GT[1] */
  tenGageFANormal,      /* 38: "fan", normalized gradient of FA: GT[3] */

  tenGageRGradVec,      /* 39: "rgv", gradient vector of Q: GT[3] */
  tenGageRGradMag,      /* 40: "rgm", gradient magnitude of Q: GT[1] */
  tenGageRNormal,       /* 41: "rn", normalized gradient of Q: GT[3] */

  tenGageThetaGradVec,  /* 42: "thgv", gradient vector of Th: GT[3] */
  tenGageThetaGradMag,  /* 43: "thgm", gradient magnitude of Th: GT[1] */
  tenGageThetaNormal,   /* 44: "thn", normalized gradient of Th: GT[3] */

  tenGageInvarGrads,    /* 45, "igs", projections of tensor gradient onto the
                               normalized shape gradients: eigenvalue 
                               mean, variance, skew, in that order: GT[9]  */
  tenGageInvarGradMags, /* 46: "igms", vector magnitude of the spatial
                               invariant gradients (above): GT[3] */
  tenGageRotTans,       /* 47: "rts", projections of the tensor gradient onto
                               non-normalized rotation tangents: GT[9] */
  tenGageRotTanMags,    /* 48: "rtms", mags of vectors above: GT[3] */
  tenGageEvalGrads,     /* 49: "evgs", projections of tensor gradient onto
                               gradients of eigenvalues: GT[9] */
  tenGageCl1,           /* 50: same as tenAniso_Cl1, but faster */
  tenGageCp1,           /* 51: same as tenAniso_Cp1, but faster */
  tenGageCa1,           /* 52: same as tenAniso_Ca1, but faster */
  tenGageCl2,           /* 53: same as tenAniso_Cl2, but faster */
  tenGageCp2,           /* 54: same as tenAniso_Cp2, but faster */
  tenGageCa2,           /* 55: same as tenAniso_Ca2, but faster */
  tenGageAniso,         /* 56: "an", all anisotropies: GT[TEN_ANISO_MAX+1] */
  tenGageLast
};
#define TEN_GAGE_ITEM_MAX  56

/*
******** tenDwiGage* enum
**
** all things that can be measured in the diffusion weighted images that
** underly diffusion tensor imaging
*/
enum {
  tenDwiGageUnknown = -1,       /* -1: nobody knows */

  /*  0: "all", all the measured values, both baseline and diffusion
      weighted: GT[N], where N is the number of DWIs */
  tenDwiGageAll,

  /*  1: "b0", the non-Dwi image value, either by direct measurement
      or by estimation: GT[1] */
  tenDwiGageB0,

  /*  2: "mdwi", the average Dwi image value, which is thresholded to
      create the confidence mask: GT[1] */
  tenDwiGageMeanDwiValue,

  /*  3: "tlls": GT[7],
      4: "tllserr": GT[1], 
      5: "tllserrlog": GT[1], 
     linear least squares fit of tensor value to log(Dwi)s */
  tenDwiGageTensorLLS,
  tenDwiGageTensorLLSError,      /* sum-of-sqrd-diffs w/ Dwis */
  tenDwiGageTensorLLSErrorLog,   /* sum-of-sqrd-diffs w/ log(Dwi)s */

  /*  6: "twls": GT[7],
      7: "twlserr": GT[1],
      8: "twlserrlog": GT[1],
     weighted least squares fit of tensor value to log(Dwi)s */
  tenDwiGageTensorWLS,
  tenDwiGageTensorWLSError,
  tenDwiGageTensorWLSErrorLog,

  /*  9: "tnls": GT[7],
     10: "tnlserr": GT[1],
     11: "tnlserrlog": GT[1],
     non-linear least squares fit of tensor value to Dwis (not log) */
  tenDwiGageTensorNLS,
  tenDwiGageTensorNLSError,
  tenDwiGageTensorNLSErrorLog,

  /* 12: "t": GT[7], 
     13: "terr": GT[1],
     14: "terrlog": GT[1],
     one of the above tensors and its errors, depending on settings */
  tenDwiGageTensor,
  tenDwiGageTensorErrorLog,
  tenDwiGageTensorError,

  /* 15: "c", first of seven tensor values: GT[1] */
  tenDwiGageConfidence,
  
  tenDwiGageLast
};
#define TEN_DWI_GAGE_ITEM_MAX 15

/*
******** tenEstimateMethod* enum
**
** the different ways of doing model parameter estimation
*/
enum {
  tenEstimateMethodUnknown,  /* 0 */
  tenEstimateMethodLLS,      /* 1 */
  tenEstimateMethodWLS,      /* 2 */
  tenEstimateMethodNLS,      /* 3 */
  tenEstimateMethodLast
};
#define TEN_ESTIMATE_METHOD_MAX 3

/*
******** tenEvecRGBParm struct
**
** dumb little bag for the parameters relating to how to do the
** eigenvector -> RGB mapping, since its needed by various things in
** various contexts.  Note that you may need two of these, one for
** doing rgb(evec0) (the linear part) and one for doing rgb(evec2)
** (the planar part).  This used to have "aniso0" and "aniso2", but
** the associated methods were clumsy and redundant.
*/
typedef struct {
  unsigned int which; /* when the eigenvector hasn't already been computed,
                         which eigenvector to map:
                         0 for linear, 2 or planar, 1 for orthotropic */
  int aniso;          /* which anisotropy metric modulates saturation */
  double confThresh,  /* confidence threshold */
    anisoGamma,       /* gamma on aniso, pre-mapping */
    gamma,            /* per RGB component gamma, post-mapping */
    bgGray,           /* gray-value for low confidence samples */
    isoGray,          /* gray-value for isotropic samples */
    maxSat;           /* maximum saturation */
  int typeOut,        /* when output type is flexible, and if this is
                         nrrdTypeUChar or nrrdTypeUShort, then output will
                         be quantized to those types (range [0,255] and
                         [0,65535] respectively); otherwise values are
                         copied directly to output */
    genAlpha;         /* when output value set is flexible, create RGBA
                         values instead of just RGB */
} tenEvecRGBParm;

/*
******** tenFiberType* enum
**
** the different kinds of fiber tractography that we do
*/
enum {
  tenFiberTypeUnknown,    /* 0: nobody knows */
  tenFiberTypeEvec1,      /* 1: standard following of principal eigenvector */
  tenFiberTypeTensorLine, /* 2: Weinstein-Kindlmann tensorlines */
  tenFiberTypePureLine,   /* 3: "pure" tensorlines- multiplication only */
  tenFiberTypeZhukov,     /* 4: Zhukov's oriented tensor reconstruction */
  tenFiberTypeLast
};
#define TEN_FIBER_TYPE_MAX   4

/*
******** tenFiberIntg* enum
**
** the different integration styles supported.  Obviously, this is more
** general purpose than fiber tracking, so this will be moved (elsewhere
** in Teem) as needed
*/
enum {
  tenFiberIntgUnknown,   /* 0: nobody knows */
  tenFiberIntgEuler,     /* 1: dumb but fast */
  tenFiberIntgRK4,       /* 2: 4rth order runge-kutta */
  tenFiberIntgLast
};
#define TEN_FIBER_INTG_MAX  2

/*
******** tenFiberStop* enum
**
** the different reasons why fibers stop going (or never start)
*/
enum {
  tenFiberStopUnknown,    /* 0: nobody knows,
                             or, for whyNowhere: no, we did get somewhere */
  tenFiberStopAniso,      /* 1: specified aniso got below specified level */
  tenFiberStopLength,     /* 2: fiber length in world space got too long */
  tenFiberStopNumSteps,   /* 3: took too many steps along fiber */
  tenFiberStopConfidence, /* 4: tensor "confidence" value went too low */
  tenFiberStopBounds,     /* 5: fiber position stepped outside volume */
  tenFiberStopLast
};
#define TEN_FIBER_STOP_MAX   5

/*
******** #define TEN_FIBER_NUM_STEPS_MAX
**
** whatever the stop criteria are for fiber tracing, no fiber half can
** have more points than this- a useful sanity check against fibers
** done amok.
*/
#define TEN_FIBER_NUM_STEPS_MAX 10240

enum {
  tenFiberParmUnknown,         /* 0: nobody knows */
  tenFiberParmStepSize,        /* 1: base step size */
  tenFiberParmUseIndexSpace,   /* 2: non-zero iff output of fiber should be 
                                  seeded in and output in index space,
                                  instead of default world */
  tenFiberParmWPunct,          /* 3: tensor-line parameter */
  tenFiberParmLast
};
#define TEN_FIBER_PARM_MAX        4

/*
******** tenFiberContext
**
** catch-all for input, state, and output of fiber tracing.  Luckily, like
** in a gageContext, NOTHING in here is set directly by the user; everything
** should be through the tenFiber* calls
*/
typedef struct {
  /* ---- input -------- */
  const Nrrd *dtvol;    /* the volume being analyzed */
  NrrdKernelSpec *ksp;  /* how to interpolate tensor values in dtvol */
  int fiberType,        /* from tenFiberType* enum */
    intg,               /* from tenFiberIntg* enum */
    anisoStopType,      /* which aniso we do a threshold on */
    anisoSpeedType,     /* base step size is function of this anisotropy */
    stop,               /* BITFLAG for different reasons to stop a fiber */
    useIndexSpace;      /* output in index space, not world space */
  double anisoThresh,   /* anisotropy threshold */
    anisoSpeedFunc[3];  /* parameters of mapping aniso to speed */
  unsigned int maxNumSteps; /* max # steps allowed on one fiber half */
  double stepSize,      /* step size in world space */
    maxHalfLen;         /* longest propagation (forward or backward) allowed
                           from midpoint */
  double confThresh;    /* confidence threshold */
  double wPunct;        /* knob for tensor lines */
  /* ---- internal ----- */
  gageQuery query;      /* query we'll send to gageQuerySet */
  int dir;              /* current direction being computed (0 or 1) */
  double wPos[3],       /* current world space location */
    wDir[3],            /* difference between this and last world space pos */
    lastDir[3],         /* previous value of wDir */
    firstEvec[3];       /* principal eigenvector first found at seed point */
  int lastDirSet;       /* lastDir[] is usefully set */
  gageContext *gtx;     /* wrapped around pvl */
  gagePerVolume *pvl;   /* wrapped around dtvol */
  const gage_t *dten,   /* gageAnswerPointer(pvl, tenGageTensor) */
    *eval,              /* gageAnswerPointer(pvl, tenGageEval) */
    *evec,              /* gageAnswerPointer(pvl, tenGageEvec) */
    *anisoStop,         /* gageAnswerPointer(pvl, tenGage<anisoStop>) */
    *anisoSpeed;        /* gageAnswerPointer(pvl, tenGage<anisoSpeed>) */
  /* ---- output ------- */
  double halfLen[2];    /* length of each fiber half in world space */
  unsigned int numSteps[2]; /* how many samples are used for each fiber half */
  int whyStop[2],       /* why backward/forward (0/1) tracing stopped
                           (from tenFiberStop* enum) */
    whyNowhere;         /* why fiber never got started (from tenFiberStop*) */
} tenFiberContext;

/*
******** struct tenEmBimodalParm
**
** input and output parameters for tenEMBimodal (for fitting two
** gaussians to a histogram).  Currently, all fields are directly
** set/read; no API help here.
**
** "fraction" means prior probability
**
** In the output, material #1 is the one with the lower mean
*/
typedef struct {
  /* ----- input -------- */
  double minProb,        /* threshold for negligible posterior prob. values */
    minProb2,            /* minProb for 2nd stage fitting */
    minDelta,            /* convergence test for maximization */
    minFraction,         /* smallest fraction (in 0.0 to 1.0) that material
                            1 or 2 can legitimately have */
    minConfidence,       /* smallest confidence value that the model fitting
                            is allowed to have */
    twoStage,            /* wacky two-stage fitting */
    verbose;             /* output messages and/or progress images */
  unsigned int maxIteration; /* cap on # of non-convergent iters allowed */
  /* ----- internal ----- */
  double *histo,         /* double version of histogram */
    *pp1, *pp2,          /* pre-computed posterior probabilities for the
                            current iteration */
    vmin, vmax,          /* value range represented by histogram. This is
                            saved from given histogram, and used to inform
                            final output values, but it is not used for 
                            any intermediate histogram calculations, all of
                            which are done entirely in index space */
    delta;               /* some measure of model change between iters */
  int N,                 /* number of bins in histogram */
    stage;               /* current stage (1 or 2) */
  unsigned int iteration;  /* current iteration */
  /* ----- output ------- */
  double mean1, stdv1,   /* material 1 mean and  standard dev */
    mean2, stdv2,        /* same for material 2 */
    fraction1,           /* fraction of material 1 (== 1 - fraction2) */
    confidence,          /* (mean2 - mean1)/(stdv1 + stdv2) */
    threshold;           /* minimum-error threshold */
} tenEMBimodalParm;

/*
******** struct tenGradientParm
**
** all parameters for repulsion-based generation of gradient directions
*/
typedef struct {
  double mass, 
    charge,
    drag, 
    dt,
    jitter,
    minVelocity,
    minMean,
    minMeanImprovement;
  int srand, snap, single;
  unsigned minIteration, maxIteration;
} tenGradientParm;

/*
******** struct tenEstimateContext
**
** for handling estimation of diffusion models
*/
typedef struct {
  /* input ----------- */
  double bValue,           /* scalar b value */
    valueMin,              /* smallest sensible for input Dwi value, 
                              must be > 0.0 (for taking log) */
    sigma,                 /* noise parameter */
    dwiConfThresh,         /* mean Dwi threshold for confidence mask */
    dwiConfSoft;           /* softness in confidence mask */
                           /* NOTE: for both _ngrad and _nbmat,
                              only one can be non-NULL, and axis[1].size
                              is the total # values, both Dwi and non-Dwi */
  const Nrrd *_ngrad,      /* caller's 3-by-allNum gradient list */
    *_nbmat;               /* caller's 6-by-allNum B-matrix list,
                              off-diagonals are *NOT* pre-multiplied by 2 */
  const float *all_f;      /* caller's list of all values (length allNum) */
  const double *all_d;     /* caller's list of all values (length allNum) */
  int simulate,            /* if non-zero, we're being used for simulation,
                              not estimation: be tolerant of unset parms */
    estimateMethod,        /* what kind of estimation to do */
    estimateB0,            /* if non-zero, B0 should be estimated along with
                              rest of model. Otherwise, B0 is found by simply
                              taking average of non-Dwi images */
    recordTime,            /* if non-zero, record estimation time */
    recordErrorDwi,
    recordErrorLogDwi,
    verbose,               /* blah blah blah */
    progress;              /* progress indication for volume processing */
  unsigned int WLSIterNum; /* number of iterations for WLS */
  /* internal -------- */
  /* a "dwi" in here is basically any value (diffusion-weighted or not)
     that varies as a function of the model parameters being estimated */
  int flag[128];           /* flags for state management */
  unsigned int allNum,     /* total number of images (Dwi and non-Dwi) */
    dwiNum;                /* number of Dwis */
  Nrrd *nbmat,             /* B-matrices (dwiNum of them) for the Dwis, with
                              off-diagonals (*YES*) pre-multiplied by 2,
                              and with a 7th column of -1.0 if estimateB0 */
    *nwght,                /* dwiNum x dwiNum matrix of weights */
    *nemat;                /* pseudo-inverse of nbmat */
  double knownB0,          /* B0 known from DWI, only if !estimateB0 */
    *all,                  /* (copy of) vector of input values,
                              allocated for allNum */
    *bnorm,                /* frob norm of B-matrix, allocated for allNum */
    *allTmp, *dwiTmp,      /* for storing intermediate values,
                              allocated for allNum and dwiNum respectively */
    *dwi,                  /* the Dwi values, allocated for dwiNum */
    time0;                 /* start time */
  /* output ---------- */
  double estimatedB0,      /* estimated non-Dwi value, only if estimateB0 */
    ten[7],                /* the estimated single tensor */
    conf,                  /* the "confidence" mask value (i.e. ten[0]) */
    mdwi,                  /* mean Dwi value (used for conf mask calc) */
    time,                  /* time required for estimation */
    errorDwi,              /* error in Dwi of estimate */
    errorLogDwi;           /* error in log(Dwi) of estimate */
} tenEstimateContext;

/* defaultsTen.c */
TEN_EXPORT2 const char *tenBiffKey;
TEN_EXPORT2 const char tenDefFiberKernel[];
TEN_EXPORT2 double tenDefFiberStepSize;
TEN_EXPORT2 int tenDefFiberUseIndexSpace;
TEN_EXPORT2 int tenDefFiberMaxNumSteps;
TEN_EXPORT2 double tenDefFiberMaxHalfLen;
TEN_EXPORT2 int tenDefFiberAnisoStopType;
TEN_EXPORT2 double tenDefFiberAnisoThresh;
TEN_EXPORT2 int tenDefFiberIntg;
TEN_EXPORT2 double tenDefFiberWPunct;

/* grads.c */
TEN_EXPORT tenGradientParm *tenGradientParmNew(void);
TEN_EXPORT tenGradientParm *tenGradientParmNix(tenGradientParm *tgparm);
TEN_EXPORT int tenGradientCheck(const Nrrd *ngrad, int type,
                                unsigned int minnum);
TEN_EXPORT int tenGradientRandom(Nrrd *ngrad, unsigned int num, int srand);
TEN_EXPORT int tenGradientJitter(Nrrd *nout, const Nrrd *nin, double dist);
TEN_EXPORT int tenGradientMeanMinimize(Nrrd *nout, const Nrrd *nin,
                                       tenGradientParm *tgparm);
TEN_EXPORT int tenGradientDistribute(Nrrd *nout, const Nrrd *nin,
                                     tenGradientParm *tgparm);
TEN_EXPORT int tenGradientGenerate(Nrrd *nout, unsigned int num,
                                   tenGradientParm *tgparm);

/* enumsTen.c */
TEN_EXPORT2 airEnum *tenAniso;
TEN_EXPORT2 airEnum _tenGage;
TEN_EXPORT2 airEnum *tenGage;
TEN_EXPORT2 airEnum *tenFiberType;
TEN_EXPORT2 airEnum *tenFiberStop;
TEN_EXPORT2 airEnum *tenGlyphType;

/* glyph.c */
TEN_EXPORT tenGlyphParm *tenGlyphParmNew();
TEN_EXPORT tenGlyphParm *tenGlyphParmNix(tenGlyphParm *parm);
TEN_EXPORT int tenGlyphParmCheck(tenGlyphParm *parm,
                                 const Nrrd *nten, const Nrrd *npos,
                                 const Nrrd *nslc);
TEN_EXPORT int tenGlyphGen(limnObject *glyphs, echoScene *scene,
                           tenGlyphParm *parm,
                           const Nrrd *nten, const Nrrd *npos,
                           const Nrrd *nslc);

/* tensor.c */
TEN_EXPORT2 int tenVerbose;
TEN_EXPORT int tenTensorCheck(const Nrrd *nin,
                              int wantType, int want4D, int useBiff);
TEN_EXPORT int tenMeasurementFrameReduce(Nrrd *nout, const Nrrd *nin);
TEN_EXPORT int tenExpand(Nrrd *tnine, const Nrrd *tseven,
                         double scale, double thresh);
TEN_EXPORT int tenShrink(Nrrd *tseven, const Nrrd *nconf, const Nrrd *tnine);
TEN_EXPORT int tenEigensolve_f(float eval[3], float evec[9],
                               const float ten[7]);
TEN_EXPORT int tenEigensolve_d(double eval[3], double evec[9],
                               const double ten[7]);
TEN_EXPORT void tenMakeOne_f(float ten[7],
                             float conf, float eval[3], float evec[9]);
TEN_EXPORT int tenMake(Nrrd *nout, const Nrrd *nconf,
                       const Nrrd *neval, const Nrrd *nevec);
TEN_EXPORT int tenSlice(Nrrd *nout, const Nrrd *nten,
                        unsigned int axis, size_t pos, unsigned int dim);
TEN_EXPORT void tenInvariantGradients_d(double mu1[7],
                                        double mu2[7], double *mu2Norm,
                                        double skw[7], double *skwNorm,
                                        double ten[7]);
TEN_EXPORT void tenRotationTangents_d(double phi1[7],
                                      double phi2[7],
                                      double phi3[7],
                                      double evec[9]);
TEN_EXPORT void tenInv_f(float inv[7], float ten[7]);
TEN_EXPORT void tenInv_d(double inv[7], double ten[7]);

/* chan.c */
/* old tenCalc* functions replaced by tenEstimate* */
TEN_EXPORT int tenDWMRIKeyValueParse(Nrrd **ngradP, Nrrd **nbmatP,
                                     double *bP, const Nrrd *ndwi);
TEN_EXPORT int tenBMatrixCalc(Nrrd *nbmat, const Nrrd *ngrad);
TEN_EXPORT int tenEMatrixCalc(Nrrd *nemat, const Nrrd *nbmat, int knownB0);
TEN_EXPORT void tenEstimateLinearSingle_f(float *ten, float *B0P,
                                          const float *dwi, const double *emat,
                                          double *vbuf, unsigned int DD,
                                          int knownB0, float thresh,
                                          float soft, float b);
TEN_EXPORT void tenEstimateLinearSingle_d(double *ten, double *B0P,
                                          const double *dwi, const double*emat,
                                          double *vbuf, unsigned int DD,
                                          int knownB0, double thresh,
                                          double soft, double b);
TEN_EXPORT int tenEstimateLinear3D(Nrrd *nten, Nrrd **nterrP, Nrrd **nB0P,
                                   const Nrrd *const *ndwi,
                                   unsigned int dwiLen, 
                                   const Nrrd *nbmat, int knownB0, 
                                   double thresh, double soft, double b);
TEN_EXPORT int tenEstimateLinear4D(Nrrd *nten, Nrrd **nterrP, Nrrd **nB0P,
                                   const Nrrd *ndwi, const Nrrd *_nbmat,
                                   int knownB0,
                                   double thresh, double soft, double b);
TEN_EXPORT void tenSimulateOne_f(float *dwi, float B0, const float *ten,
                                 const double *bmat, unsigned int DD, float b);
TEN_EXPORT int tenSimulate(Nrrd *ndwi, const Nrrd *nT2, const Nrrd *nten,
                           const Nrrd *nbmat, double b);

/* estimate.c */
TEN_EXPORT2 airEnum *tenEstimateMethod;
TEN_EXPORT tenEstimateContext *tenEstimateContextNew();
TEN_EXPORT void tenEstimateVerboseSet(tenEstimateContext *tec,
                                      int verbose);
TEN_EXPORT int tenEstimateMethodSet(tenEstimateContext *tec,
                                    int estMethod);
TEN_EXPORT int tenEstimateSigmaSet(tenEstimateContext *tec,
                                   double sigma);
TEN_EXPORT int tenEstimateValueMinSet(tenEstimateContext *tec,
                                      double valueMin);
TEN_EXPORT int tenEstimateGradientsSet(tenEstimateContext *tec,
                                       const Nrrd *ngrad, 
                                       double bValue, int estimateB0);
TEN_EXPORT int tenEstimateBMatricesSet(tenEstimateContext *tec,
                                       const Nrrd *nbmat,
                                       double bValue, int estimateB0);
TEN_EXPORT int tenEstimateThresholdSet(tenEstimateContext *tec,
                                       double thresh, double soft);
TEN_EXPORT int tenEstimateUpdate(tenEstimateContext *tec);
TEN_EXPORT int tenEstimate1TensorSimulateSingle_f(tenEstimateContext *tec,
                                                  float *simval,
                                                  float sigma,
                                                  float bValue, float B0,
                                                  const float _ten[7]);
TEN_EXPORT int tenEstimate1TensorSimulateSingle_d(tenEstimateContext *tec,
                                                  double *simval,
                                                  double sigma,
                                                  double bValue, double B0,
                                                  const double ten[7]);
TEN_EXPORT int tenEstimate1TensorSimulateVolume(tenEstimateContext *tec,
                                                Nrrd *ndwi, 
                                                double sigma, double bValue,
                                                const Nrrd *nB0,
                                                const Nrrd *nten,
                                                int outType);
TEN_EXPORT int tenEstimate1TensorSingle_f(tenEstimateContext *tec,
                                          float ten[7], const float *all);
TEN_EXPORT int tenEstimate1TensorSingle_d(tenEstimateContext *tec,
                                          double ten[7], const double *all);
TEN_EXPORT int tenEstimate1TensorVolume4D(tenEstimateContext *tec,
                                          Nrrd *nten,
                                          Nrrd **nB0P, Nrrd **nterrP,
                                          const Nrrd *ndwi, int outType);
TEN_EXPORT tenEstimateContext *tenEstimateContextNix(tenEstimateContext *tec);

/* aniso.c */
TEN_EXPORT2 float (*_tenAnisoEval_f[TEN_ANISO_MAX+1])(const float eval[3]);
TEN_EXPORT float tenAnisoEval_f(const float eval[3], int aniso);
TEN_EXPORT2 double (*_tenAnisoEval_d[TEN_ANISO_MAX+1])(const double eval[3]);
TEN_EXPORT double tenAnisoEval_d(const double eval[3], int aniso);
TEN_EXPORT void tenAnisoCalc_f(float c[TEN_ANISO_MAX+1], const float eval[3]);
TEN_EXPORT int tenAnisoPlot(Nrrd *nout, int aniso, unsigned int res,
                            int whole, int nanout);
TEN_EXPORT int tenAnisoVolume(Nrrd *nout, const Nrrd *nin,
                              int aniso, double confThresh);
TEN_EXPORT int tenAnisoHistogram(Nrrd *nout, const Nrrd *nin,
                                 const Nrrd *nwght, int right,
                                 int version, unsigned int resolution);
TEN_EXPORT tenEvecRGBParm *tenEvecRGBParmNew(void);
TEN_EXPORT tenEvecRGBParm *tenEvecRGBParmNix(tenEvecRGBParm *rgbp);
TEN_EXPORT int tenEvecRGBParmCheck(const tenEvecRGBParm *rgbp);
TEN_EXPORT void tenEvecRGBSingle_f(float RGB[3], float conf,
                                   const float eval[3], const float evec[3],
                                   const tenEvecRGBParm *rgbp);
TEN_EXPORT void tenEvecRGBSingle_d(double RGB[3], double conf,
                                   const double eval[3], const double evec[3],
                                   const tenEvecRGBParm *rgbp);

/* miscTen.c */
TEN_EXPORT int tenEvecRGB(Nrrd *nout, const Nrrd *nin,
                          const tenEvecRGBParm *rgbp);
TEN_EXPORT short tenEvqOne_f(float vec[3], float scl);
TEN_EXPORT int tenEvqVolume(Nrrd *nout, const Nrrd *nin, int which,
                            int aniso, int scaleByAniso);
TEN_EXPORT int tenBMatrixCheck(const Nrrd *nbmat,
                               int type, unsigned int minnum);
TEN_EXPORT int _tenFindValley(double *valP, const Nrrd *nhist,
                              double tweak, int save);

/* fiberMethods.c */
TEN_EXPORT tenFiberContext *tenFiberContextNew(const Nrrd *dtvol);
TEN_EXPORT int tenFiberTypeSet(tenFiberContext *tfx, int type);
TEN_EXPORT int tenFiberKernelSet(tenFiberContext *tfx,
                                 const NrrdKernel *kern,
                                 const double parm[NRRD_KERNEL_PARMS_NUM]);
TEN_EXPORT int tenFiberIntgSet(tenFiberContext *tfx, int intg);
TEN_EXPORT int tenFiberStopSet(tenFiberContext *tfx, int stop, ...);
TEN_EXPORT void tenFiberStopOn(tenFiberContext *tfx, int stop);
TEN_EXPORT void tenFiberStopOff(tenFiberContext *tfx, int stop);
TEN_EXPORT void tenFiberStopReset(tenFiberContext *tfx);
TEN_EXPORT int tenFiberAnisoSpeedSet(tenFiberContext *tfx, int aniso,
                                     double lerp, double thresh, double soft);
TEN_EXPORT int tenFiberAnisoSpeedReset(tenFiberContext *tfx);
TEN_EXPORT int tenFiberParmSet(tenFiberContext *tfx, int parm, double val);
TEN_EXPORT int tenFiberUpdate(tenFiberContext *tfx);
TEN_EXPORT tenFiberContext *tenFiberContextCopy(tenFiberContext *tfx);
TEN_EXPORT tenFiberContext *tenFiberContextNix(tenFiberContext *tfx);

/* fiber.c */
TEN_EXPORT int tenFiberTraceSet(tenFiberContext *tfx, Nrrd *nfiber,
                                double *buff, unsigned int halfBuffLen,
                                unsigned int *startIdxP, unsigned int *endIdxP,
                                double seed[3]);
TEN_EXPORT int tenFiberTrace(tenFiberContext *tfx,
                             Nrrd *fiber, double seed[3]);

/* epireg.c */
TEN_EXPORT int _tenEpiRegFindThresh(double *DWthrP, Nrrd **nin,
                                    int ninLen, int save);
TEN_EXPORT int tenEpiRegister3D(Nrrd **nout, Nrrd **ndwi,
                                unsigned int dwiLen, Nrrd *ngrad,
                                int reference,
                                double bwX, double bwY,
                                double fitFrac, double DWthr,
                                int doCC,
                                const NrrdKernel *kern, double *kparm,
                                int progress, int verbose);
TEN_EXPORT int tenEpiRegister4D(Nrrd *nout, Nrrd *nin, Nrrd *ngrad,
                                int reference,
                                double bwX, double bwY,
                                double fitFrac, double DWthr,
                                int doCC, 
                                const NrrdKernel *kern, double *kparm,
                                int progress, int verbose);

/* mod.c */
TEN_EXPORT int tenSizeNormalize(Nrrd *nout, const Nrrd *nin, double weight[3],
                                double amount, double target);
TEN_EXPORT int tenSizeScale(Nrrd *nout, const Nrrd *nin, double amount);
TEN_EXPORT int tenAnisoScale(Nrrd *nout, const Nrrd *nin, double scale,
                             int fixDet, int makePositive);
TEN_EXPORT int tenEigenvaluePower(Nrrd *nout, const Nrrd *nin, double expo);
TEN_EXPORT int tenEigenvalueClamp(Nrrd *nout, const Nrrd *nin,
                                  double min, double max);
TEN_EXPORT int tenEigenvalueAdd(Nrrd *nout, const Nrrd *nin, double val);

/* bvec.c */
TEN_EXPORT int tenBVecNonLinearFit(Nrrd *nout, const Nrrd *nin, 
                                   double *bb, double *ww,
                                   int iterMax, double eps);

/* tenGage.c */
TEN_EXPORT2 gageKind *tenGageKind;

/* tenDwiGage.c */
#define TEN_DWI_GAGE_KIND_NAME "dwi"
TEN_EXPORT2 airEnum _tenDwiGage;
TEN_EXPORT2 airEnum *tenDwiGage;
TEN_EXPORT gageKind *tenDwiGageKindNew();
TEN_EXPORT gageKind *tenDwiGageKindNix(gageKind *dwiKind);
TEN_EXPORT int tenDwiGageKindGradients(gageKind *dwiKind,
                                       double bval, const Nrrd *ngrad);
TEN_EXPORT int tenDwiGageKindBMatrices(gageKind *dwiKind,
                                       double bval, const Nrrd *nbmat);
TEN_EXPORT int tenDwiGageKindConfThreshold(gageKind *dwiKind,
                                           double thresh, double soft);
TEN_EXPORT int tenDwiGageKindFitType(gageKind *dwiKind, int fitType);

/* bimod.c */
TEN_EXPORT tenEMBimodalParm* tenEMBimodalParmNew(void);
TEN_EXPORT tenEMBimodalParm* tenEMBimodalParmNix(tenEMBimodalParm *biparm);
TEN_EXPORT int tenEMBimodal(tenEMBimodalParm *biparm, const Nrrd *nhisto);

/* tend{Flotsam,Anplot,Anvol,Evec,Eval,...}.c */
#define TEND_DECLARE(C) TEN_EXPORT2 unrrduCmd tend_##C##Cmd;
#define TEND_LIST(C) &tend_##C##Cmd,
/* removed from below (superseded by estim): F(calc) \ */
#define TEND_MAP(F) \
F(grads) \
F(epireg) \
F(bmat) \
F(estim) \
F(sim) \
F(make) \
F(helix) \
F(sten) \
F(glyph) \
F(ellipse) \
F(anplot) \
F(anvol) \
F(anscale) \
F(anhist) \
F(point) \
F(slice) \
F(fiber) \
F(norm) \
F(eval) \
F(evalpow) \
F(evalclamp) \
F(evaladd) \
F(evec) \
F(evecrgb) \
F(evq) \
F(unmf) \
F(expand) \
F(shrink) \
F(bfit) \
F(satin)
TEND_MAP(TEND_DECLARE)
TEN_EXPORT2 unrrduCmd *tendCmdList[]; 
TEN_EXPORT void tendUsage(char *me, hestParm *hparm);
TEN_EXPORT2 hestCB *tendFiberStopCB;

#ifdef __cplusplus
}
#endif

#endif /* TEN_HAS_BEEN_INCLUDED */



