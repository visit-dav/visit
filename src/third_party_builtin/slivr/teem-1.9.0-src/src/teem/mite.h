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

#ifndef MITE_HAS_BEEN_INCLUDED
#define MITE_HAS_BEEN_INCLUDED

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/gage.h>
#include <teem/limn.h>
#include <teem/hoover.h>
#include <teem/ten.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(mite_EXPORTS) || defined(teem_EXPORTS)
#    define MITE_EXPORT extern __declspec(dllexport)
#  else
#    define MITE_EXPORT extern __declspec(dllimport)
#  endif
#define MITE_EXPORT2 MITE_EXPORT
#else /* TEEM_STATIC || UNIX */
# if __GNUC__ >= 4 && (defined(mite_EXPORTS) || defined(teem_EXPORTS))
#  define MITE_EXPORT __attribute__ ((visibility("default")))
# else
#  define MITE_EXPORT extern
# endif
#define MITE_EXPORT2 extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MITE miteBiffKey

/*
******** mite_t
**
** the type used for representing and storing transfer function *range*
** (color, opacity, etc) information is:
** 1: float
** 0: double
*/

#if 0
typedef float mite_t;
#define mite_nt nrrdTypeFloat
#define mite_at airTypeFloat
#define limnVTOQN limnVtoQN_f
#define MITE_T_DOUBLE 0
#else
typedef double mite_t;
#define mite_nt nrrdTypeDouble
#define mite_at airTypeDouble
#define limnVTOQN limnVtoQN_d
#define MITE_T_DOUBLE 1
#endif

enum {
  miteRangeUnknown = -1,
  miteRangeAlpha,        /* 0: "A", opacity */
  miteRangeRed,          /* 1: "R" */
  miteRangeGreen,        /* 2: "G" */
  miteRangeBlue,         /* 3: "B" */
  miteRangeEmissivity,   /* 4: "E" */
  miteRangeKa,           /* 5: "a", phong ambient term */
  miteRangeKd,           /* 6: "d", phong diffuse */
  miteRangeKs,           /* 7: "s", phong specular */
  miteRangeSP,           /* 8: "p", phong specular power */
  miteRangeLast
};
#define MITE_RANGE_NUM      9

/*
** There are some constraints on how the nrrd as transfer function is
** stored and used:
** 1) all transfer functions are lookup tables: there is no 
** interpolation other than nearest neighbor (actually, someday linear
** interpolation may be supported, but that's it)
** 2) regardless of the centerings of the axes of nxtf, the lookup table
** axes will be treated as though they were cell centered (linear 
** interpolation will always use node centering)
** 3) the logical dimension of the transfer function is always one less
** ntxf->dim, with axis 0 always for the range of the function, and axes
** 1 and onwards for the domain.  For instance, a univariate opacity map
** is 2D, with ntxf->axis[0].size == 1.
** 
** So, ntxf->dim-1 is the number of variables in the domain of the transfer
** function, and ntxf->axis[0].size is the number of variables in the range.
*/

/*
******** miteUser struct
**
** all the input parameters for mite specified by the user, as well as
** a mop for cleaning up memory used during rendering.  Currently,
** unlike gage, there is no API for setting these- you go in the
** struct and set them yourself. 
**
** Mite can currently handle scalar, 3-vector, and (symmetric) tensor
** volumes, one (at most) of each.  All these volumes must have the
** same 3-D size, because we're only using one gageContext per thread,
** and the gageContext is what stores the convolution kernel weights
** evaluated per sample.
*/
typedef struct {
  Nrrd *nsin,            /* scalar volume being rendered */
    *nvin,               /* 3-vector volume being rendered */
    *ntin,               /* tensor volume being rendered */
    **ntxf,              /* array of pointers to nrrds containing transfer
                            functions, these are never altered (the ntxf
                            in miteRender, however, is created and altered
                            based on this ntxf) */
    *nout;               /* output image container, for five-channel output
                            RGBAZ.  We'll nrrdMaybeAlloc all the image data
                            and put it in here, but we won't nrrdNuke(nout),
                            just like we won't nrrdNuke nsin, nvin, ntin, or
                            any of the ntxf[i] */
  double *debug;         /* data for verbose pixel (verbUi, verbVi) debug */
  airArray *debugArr;    /* airArray around debug */
  Nrrd *ndebug;          /* nrrd re-allocation of debug data */
  int debugIdx;          /* index into debug data */
  int ntxfNum;           /* allocated and valid length of ntxf[] */
  /* the issue of regular shading, txf-based shading, and surface normals:
     phong and lit-tensor shading ("regular shading") methods need to specify
     one or more vectors that are used for shading calculations.  These will
     be parsed from muu->shadeStr into mrr->shadeSpec, which in turn will 
     determine the pointer values of mtt->shade{Vec,Scl}{0,1}.
     ENTIRELY SEPERATE FROM THIS, a "surface normal" can be specified in
     muu->normalStr, which (if non-empty), will be parsed into mrr->normalSpec,
     which in turn will determine the pointer values of mtt->normal.  This
     normal is used for the miteVal quantities involving "N".  
     Both shading and normal specifications can be given, since they are
     used in seperate computations.  If the user wants to do miteVal-based
     stuff with the same quantity specified in (say) a phong shading 
     specification, its up to them to verify that the normalStr and the
     shadeStr refer to the same vector.
  */
  char shadeStr[AIR_STRLEN_MED], /* how to do shading */
    normalStr[AIR_STRLEN_MED];   /* what is the "surface normal" */
  /* for each possible element of the txf range, what value should it
     start at prior to rendering. Mainly needed to store non-unity values
     for the quantities not covered by a transfer function */
  mite_t rangeInit[MITE_RANGE_NUM]; 
  double refStep,        /* length of "unity" for doing opacity correction */
    rayStep,             /* distance between sampling planes */
    opacMatters,         /* minimal significant opacity, currently used to
                            assign a Z depth (really "Tw") for each rendered
                            ray */
    opacNear1;           /* opacity close enough to unity for the sake of
                            doing early ray termination */
  hooverContext *hctx;   /* context and input for all hoover-related things,
                            including camera and image parameters */
  double fakeFrom[3],    /* if non-NaN, then the "V"-dependent miteVal's will
                            use this as the nominal eye point, instead of what
                            is actually being used in the hooverContext */
    vectorD[3];          /* some vector to use for something */
  /* local initial copies of kernels, later passed to gageKernelSet */
  NrrdKernelSpec *ksp[GAGE_KERNEL_NUM];
  gageContext *gctx0;    /* context and input for all gage-related things,
                            including all kernels.  This is gageContextCopy'd
                            for multi-threaded use (hence the 0) */
  limnLight *lit;        /* a struct for all lighting info, although 
                            currently only the ambient and *first* directional
                            light are used */
  int normalSide,        /* determines direction of gradient that is used
                            as normal for shading:
                            1: the standard way: normal points to lower values
                               (higher values are more "inside"); 
                            0: "two-sided": dot-products are abs()'d
                            -1: normal points to higher values
                               (lower values more "inside")
                            Setting up the values this way allows for idioms
                            like "if (muu->normalSide) ...", meaning, if the
                            lighting is one-sided */
    verbUi, verbVi;      /* pixel coordinate for which to turn on verbosity */
  airArray *umop;        /* for things allocated which are used across
                            multiple renderings */
  /* output information from last rendering */
  double rendTime,       /* rendering time, in seconds */
    sampRate;            /* rate (KHz) at which samples were rendered */
} miteUser;

struct miteThread_t;

/*
******** miteShadeMethod* enum
**
** the different ways that shading can be done
*/
enum {
  miteShadeMethodUnknown,
  miteShadeMethodNone,        /* 1: no direction shading based on anything
                                 in the miteShadeSpec: just ambient, though
                                 things are still composited with the over
                                 operator, and transfer functions are free
                                 to implement whatever shading they can */
  miteShadeMethodPhong,       /* 2: what mite has been doing all along */
  miteShadeMethodLitTen,      /* 3: (tensor-based) lit-tensors */
  miteShadeMethodLast
};

/*
******** miteShadeSpec struct
**
** describes how to do shading.  With more and more generality in the
** expressions that are evaluated for transfer function application,
** there is less need for this "shading" per se (phong shading can be
** expressed with multiplicative and additive transfer functions).
** But its here for the time being...
*/
typedef struct {
  int method;                 /* from miteShadeMethod* enum */
  gageItemSpec *vec0, *vec1,
    *scl0, *scl1;             /* things to use for shading.  How these are
                                 interpreted is determined by shadeMethod:
                                 phong: vec0 is used as normal
                                 litten: lit-tensors based on vec0 and vec1,
                                 as weighted by scl0, scl1 */
} miteShadeSpec;

/*
******** miteRender
**
** rendering-parameter-set-specific (but non-thread-specific) state,
** used internally by mite.  The primary role here is to store information
** derived from the miteUser, in a form which is more immediately useful
** for rendering.  
*/
typedef struct {
  Nrrd **ntxf;                /* array of transfer function nrrds.  The 
                                 difference from those in miteUser is that 
                                 opacity correction (based on rayStep and 
                                 refStep) has been applied to these, and
                                 these have been converted/unquantized to
                                 type mite_t */
  int ntxfNum;                /* allocated and valid length of ntxf[] */
  int sclPvlIdx, vecPvlIdx,
    tenPvlIdx;                /* indices of the different gageKinds of 
                                 volumes in the gageContext's array of
                                 gagePerVolumes.  Probably a hack */
  miteShadeSpec *shadeSpec;   /* information based on muu->shadeStr */
  gageItemSpec *normalSpec;   /* information based on muu->normalStr */
  double time0;               /* rendering start time */
  gageQuery queryMite;        /* record of the miteVal quantities which 
                                 we'll need to compute per-sample */
  int queryMiteNonzero;       /* shortcut miteVal computation if possible */

  /* as long as there's no mutex around how the miteThreads are
     airMopAdded to the miteUser's mop, these have to be _allocated_ in
     mrendRenderBegin, instead of mrendThreadBegin, which still has the
     role of initializing them */
  struct miteThread_t *tt[HOOVER_THREAD_MAX];  
  airArray *rmop;             /* for things allocated which are rendering
                                 (or rendering parameter) specific */
} miteRender;

/*
******** miteStageOp* enum
**
** the kinds of things we can do per txf to modify the range
** variables.  previously mite only supported seperable transfer
** functions (i.e., multiplication only).  It is tempting to use all
** the operations available as nrrdBinaryOps, but that would lead to
** goofy ones like Mod and GreaterThan, which either require or create
** integral values which aren't of much use in transfer functions.
** More generality in how opacities and colors are assigned will
** likely be supported by some simple programmability, supported by
** the likes of the funk library, which will be entirely seperate from
** the miteStageOp mechanism.
*/
enum {
  miteStageOpUnknown,   /* 0 */
  miteStageOpMin,       /* 1 */
  miteStageOpMax,       /* 2 */
  miteStageOpAdd,       /* 3 */
  miteStageOpMultiply,  /* 4 */
  miteStageOpLast
};
#define MITE_STAGE_OP_MAX  4

typedef struct {
  gage_t *val;                  /* the txf axis variable, computed either by
                                   gage or by mite.  This points into the 
                                   answer vector in one of the thread's
                                   pervolumes, or into ansMiteVal in the
                                   miteThread.  It can be either a scalar
                                   or a vector */
  int size,                     /* number of entries along this txf axis */
    op,                         /* from miteStageOp* enum.  Note that this
                                   operation applies to ALL the range variables
                                   adjusted by this txf (can't add color while
                                   multiplying opacity) */
    (*qn)(gage_t *);            /* if non-NULL: callback for doing vector
                                   quantization of vector-valued txf domain
                                   variables */
  double min, max;              /* quantization range of values which is 
                                   covered by this axis of the txf; copied
                                   from corresponding axis of the nrrd */
  mite_t *data;                 /* pointer to txf data.  If non-NULL, the
                                   following fields are meaningful */
  int rangeIdx[MITE_RANGE_NUM], /* indices into miteThread's range, so that
                                   we know which quantities to update */
    rangeNum;                   /* number of range variables set by the txf
                                   == number of pointers in range[] to use */
  char *label;                  /* pointer into axis label identifying txf
                                   domain variable, NOT COPIED */
} miteStage;

/*
******** miteVal* enum
** 
** the quantities not measured by gage (but often reliant on gage-based
** measurements) which can appear in the transfer function domain.
** In many respects, these behave like gage queries, and these are
** associated with a gageKind (miteValGageKind), but it is hardly a 
** real, bona fide, gageKind. The answers for these are stored in
** the miteThread, in lieu of a gagePerVolume
*/
enum {
  miteValUnknown=-1,    /* -1: nobody knows */
  miteValXw,            /*  0: "Xw", X position, world space (gage_t[1]) */
  miteValXi,            /*  1: "Xi", X     "   , index   "   (gage_t[1]) */
  miteValYw,            /*  2: "Yw", Y     "   , world   "   (gage_t[1]) */
  miteValYi,            /*  3: "Yi", Y     "   , index   "   (gage_t[1]) */
  miteValZw,            /*  4: "Zw", Z     "   , world   "   (gage_t[1]) */
  miteValZi,            /*  5: "Zi", Z     "   , index   "   (gage_t[1]) */
  miteValRw,            /*  6: "Rw", dist to origin, world " (gage_t[1]) */
  miteValRi,            /*  7: "Ri", dist to origin, index " (gage_t[1]) */
  miteValTw,            /*  8: "Tw", ray position (gage_t[1]) */
  miteValTi,            /*  9: "Ti", ray index (ray sample #) (gage_t[1]) */
  miteValView,          /* 10: "V", the view vector (gage_t[3]) */
  miteValNormal,        /* 11: "N", the nominal surface normal, as measured
                               by a scalar, vector, or tensor kind, and then
                               determined by the semantics of
                               muu->normalSide */
  miteValNdotV,         /* 12: "NdotV", surface normal dotted w/ view vector
                               (towards eye) (gage_t[1]) */
  miteValNdotL,         /* 13: "NdotL", surface normal dotted w/ light vector
                               (towards the light source) (gage_t[1]) */
  miteValVrefN,         /* 14: "VrefN", view vector reflected across normal
                               (gage_t[3]) */
  miteValGTdotV,        /* 15: "GTdotV", normal curvature in view direction,
                               the contraction of the geometry tensor along
                               the view vector (gage_t[1]) */
  miteValVdefT,         /* 16: "defT", view direction, deflected by tensor,
                               then normalized (gage_t[3]) */
  miteValVdefTdotV,     /* 17: "VdefTdotV", VdefT dotted back with V, not the
                               same as the tensor contraction along V,
                               (gage_t[1]) */
  miteValWdotD,         /* 18: "WdotD", world space position dotted with
                               muu->vectorD */
  miteValLast
};
#define MITE_VAL_ITEM_MAX  18

/*
******** miteThread
**
** thread-specific state for mite's internal use
*/
typedef struct miteThread_t {
  gageContext *gctx;            /* per-thread context */
  gage_t *ansScl,               /* pointer to gageKindScl answer vector */
    *nPerp, *geomTens,          /* convenience pointers into ansScl */
    *ansVec,                    /* pointer to gageKindVec answer vector */
    *ansTen,                    /* pointer to tenGageKind answer vector */
    *ansMiteVal,                /* room for all the miteVal answers, which
                                   unlike ans{Scl,Vec,Ten} is allocated by
                                   mite instead of by gage */
    **directAnsMiteVal,         /* pointers into ansMiteVal */
    *_normal,                   /* pointer into ans{Scl,Vec,Ten} array above,
                                   (NOT ansMiteVal); this will determine the
                                   value of miteValNormal according to the
                                   semantics of muu->normalSide */
    *shadeVec0, *shadeVec1, 
    *shadeScl0, *shadeScl1;     /* pointers into the ans* arrays above,
                                   used for shading */
  int verbose,                  /* non-zero if 
                                   (ui,vi) = (muu->verbUi,muu->verbVi) 
                                   Because of the actions associated with 
                                   recording values in muu->ndebug, it is
                                   currently reasonable for verbose to be
                                   non-zero for at most *ONE* pixel: it has 
                                   to be one pixel because only one thread
                                   should be creating and storing this per-
                                   pixel information in the miteUser */
    skip,                       /* non-zero if we shouldn't actually do any
                                   work for this pixel */
    thrid,                      /* thread ID */
    ui, vi,                     /* image coords of current ray */
    raySample,                  /* number of samples finished in this ray */
    samples;                    /* number of samples handled so far by
                                   this thread */
  miteStage *stage;             /* array of stages for txf computation */
  int stageNum;                 /* number of stages == length of stage[] */
  mite_t range[MITE_RANGE_NUM], /* rendering variables, which are either
                                   copied from miteUser's rangeInit[], or
                                   over-written by txf evaluation */
    rayStep,                    /* per-ray step (may need to be different for
                                   each ray to enable sampling on planes) */
    V[3],                       /* per-ray view direction */
    RR, GG, BB, TT,             /* per-ray composited values */
    ZZ;                         /* for storing ray-depth when opacity passed
                                   muu->opacMatters */
} miteThread;

/* defaultsMite.c */
MITE_EXPORT2 const char *miteBiffKey;
MITE_EXPORT2 double miteDefRefStep;
MITE_EXPORT2 int miteDefRenorm;
MITE_EXPORT2 int miteDefNormalSide;
MITE_EXPORT2 double miteDefOpacNear1;
MITE_EXPORT2 double miteDefOpacMatters;

/* kindnot.c */
MITE_EXPORT2 airEnum *miteVal;
MITE_EXPORT2 gageKind *miteValGageKind;

/* txf.c */
MITE_EXPORT2 airEnum *miteStageOp;
MITE_EXPORT2 char miteRangeChar[MITE_RANGE_NUM+1];
MITE_EXPORT int miteVariableParse(gageItemSpec *isp, const char *label);
MITE_EXPORT void miteVariablePrint(char *buff, const gageItemSpec *isp);
MITE_EXPORT int miteNtxfCheck(const Nrrd *ntxf);
MITE_EXPORT void miteQueryAdd(gageQuery queryScl, gageQuery queryVec, 
                              gageQuery queryTen, gageQuery queryMite,
                              gageItemSpec *isp);

/* user.c */
MITE_EXPORT miteUser *miteUserNew();
MITE_EXPORT miteUser *miteUserNix(miteUser *muu);

/* shade.c */
MITE_EXPORT miteShadeSpec *miteShadeSpecNew();
MITE_EXPORT miteShadeSpec *miteShadeSpecNix(miteShadeSpec *);
MITE_EXPORT int miteShadeSpecParse(miteShadeSpec *shpec, char *shadeStr);
MITE_EXPORT void miteShadeSpecPrint(char *buff, const miteShadeSpec *shpec);
MITE_EXPORT void miteShadeSpecQueryAdd(gageQuery queryScl, gageQuery queryVec, 
                                       gageQuery queryTen, gageQuery queryMite,
                                       miteShadeSpec *shpec);

/* renderMite.c */
MITE_EXPORT int miteRenderBegin(miteRender **mrrP, miteUser *muu);
MITE_EXPORT int miteRenderEnd(miteRender *mrr, miteUser *muu);

/* thread.c */
MITE_EXPORT miteThread *miteThreadNew();
MITE_EXPORT miteThread *miteThreadNix(miteThread *mtt);
MITE_EXPORT int miteThreadBegin(miteThread **mttP, miteRender *mrr,
                                miteUser *muu, int whichThread);
MITE_EXPORT int miteThreadEnd(miteThread *mtt, miteRender *mrr, miteUser *muu);

/* ray.c */
MITE_EXPORT int miteRayBegin(miteThread *mtt, miteRender *mrr, miteUser *muu,
                             int uIndex, int vIndex, 
                             double rayLen,
                             double rayStartWorld[3], double rayStartIndex[3],
                             double rayDirWorld[3], double rayDirIndex[3]);
MITE_EXPORT double miteSample(miteThread *mtt, miteRender *mrr, miteUser *muu,
                              int num, double rayT, int inside,
                              double samplePosWorld[3],
                              double samplePosIndex[3]);
MITE_EXPORT int miteRayEnd(miteThread *mtt, miteRender *mrr,
                           miteUser *muu);

#ifdef __cplusplus
}
#endif

#endif /* MITE_HAS_BEEN_INCLUDED */
