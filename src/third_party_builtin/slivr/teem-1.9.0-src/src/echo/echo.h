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

#ifndef ECHO_HAS_BEEN_INCLUDED
#define ECHO_HAS_BEEN_INCLUDED

/* NOTE: this library has not undergone the changes as other Teem
   libraries in order to make sure that array lengths and indices
   are stored in unsigned types */

#include <stdio.h>
#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/limn.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(echo_EXPORTS) || defined(teem_EXPORTS)
#    define ECHO_EXPORT extern __declspec(dllexport)
#  else
#    define ECHO_EXPORT extern __declspec(dllimport)
#  endif
#define ECHO_EXPORT2 ECHO_EXPORT
#else /* TEEM_STATIC || UNIX */
# if __GNUC__ >= 4 && (defined(echo_EXPORTS) || defined(teem_EXPORTS))
#  define ECHO_EXPORT __attribute__ ((visibility("default")))
#  define ECHO_EXPORT2 extern __attribute__ ((visibility("default")))
# else
#  define ECHO_EXPORT extern
#  define ECHO_EXPORT2 extern
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ECHO echoBiffKey

/* all position and transform information is kept as ...
** 1: float
** 0: double
*/
#if 0
typedef float echoPos_t;
#define ECHO_POS_FLOAT 1
#else 
typedef double echoPos_t;
#define ECHO_POS_FLOAT 0
#endif


/* all color information is kept as 
** 1: float
** 0: double
*/
#if 1
typedef float echoCol_t;
#define echoCol_nt nrrdTypeFloat
#else
typedef double echoCol_t;
#define echoCol_nt nrrdTypeDouble
#endif

#define ECHO_LIST_OBJECT_INCR 32
#define ECHO_IMG_CHANNELS 5
#define ECHO_EPSILON 0.00005      /* used for adjusting ray positions */
#define ECHO_NEAR0 0.004          /* used for comparing transparency to zero */
#define ECHO_LEN_SMALL_ENOUGH 5   /* to control splitting for split objects */

#define ECHO_THREAD_MAX 512       /* max number of threads */

typedef struct {
  int jitterType,      /* from echoJitter* enum below */
    reuseJitter,       /* don't recompute jitter offsets per pixel */
    permuteJitter,     /* properly permute the various jitter arrays */
    textureNN,         /* use nearest-neighbor for texture lookups 
                          (rather than bilinear interpolation) */
    numSamples,        /* rays per pixel */
    imgResU, imgResV,  /* horz. and vert. image resolution */
    maxRecDepth,       /* max recursion depth */
    renderLights,      /* render the area lights */
    renderBoxes,       /* faintly render bounding boxes */
    seedRand,          /* call airSrandMT() (don't if repeatability wanted) */
    sqNRI,             /* how many iterations of newton-raphson we allow for
                          finding superquadric root (within tolorance sqTol) */
    numThreads;        /* number of threads to spawn per rendering */
  echoPos_t
    sqTol;             /* how close newtwon-raphson must get to zero */
  echoCol_t
    shadow,            /* the extent to which shadows are seen:
                          0: no shadow rays cast
                          >0: shadow rays cast, results weighed by shadow
                          1: full shadowing */
    glassC;            /* should really be an additional material parameter:
                          Beer's law attenuation in glass */
  float aperture,      /* shallowness of field */
    timeGamma,         /* gamma for values in time image */
    boxOpac;           /* opacity of bounding boxes with renderBoxes */
  echoCol_t
    maxRecCol[3];      /* color of max recursion depth being hit */
} echoRTParm;

struct echoScene_t;

typedef struct {
  int verbose;
  double time;         /* time it took to render image */
  Nrrd *nraw;          /* copies of arguments to echoRTRender */
  limnCamera *cam;
  struct echoScene_t *scene;
  echoRTParm *parm;
  int workIdx;         /* next work assignment (such as a scanline) */
  airThreadMutex *workMutex; /* mutex around work assignment */
} echoGlobalState;

typedef struct {
  airThread *thread;    /* my thread */
  echoGlobalState *gstate;
  int verbose,          /* blah blah blah */
    threadIdx,          /* my thread index */
    depth;              /* how many recursion levels are we at */
  Nrrd *nperm,          /* ECHO_JITTABLE_NUM x parm->numSamples array 
                           of ints, each column is a (different) random
                           permutation of [0 .. parm->numSamples-1], each
                           row corresponds to the different jittables for
                           a single sample */
    *njitt;             /* 2 x ECHO_JITTABLE_NUM x parm->numSamples array
                           of echoPos_t's in domain [-1/2,1/2]; like the nperm
                           array, each row is comprised of the jitter vectors
                           (for all possible jittables) to use for 1 sample */
  unsigned int *permBuff; /* temp array for creating permutations */
  echoPos_t *jitt;      /* pointer into njitt, good for current sample */
  echoCol_t *chanBuff;  /* for storing ray color and other parameters for each
                           of the parm->numSamples rays in current pixel */
  airRandMTState *rst;  /* random number state */
  void *returnPtr;      /* for airThreadJoin */
} echoThreadState;

/*
******** echoJitter* enum
** 
** the different jitter patterns that are supported.  This setting is
** global- you can't have different jitter patterns on the lights versus
** the pixels.
*/
enum {
  echoJitterUnknown=-1,
  echoJitterNone,       /* 0: N samples all at the square center */
  echoJitterGrid,       /* 1: N samples exactly on a sqrt(N) x sqrt(N) grid */
  echoJitterJitter,     /* 2: N jittered samples on a sqrt(N) x sqrt(N) grid */
  echoJitterRandom,     /* 3: N samples randomly placed in square */
  echoJitterLast
};
#define ECHO_JITTER_NUM    4

/*
******** echoJittable* enum
**
** the different quantities to which the jitter two-vector may be
** applied.  
*/
enum {
  echoJittableUnknown=-1,
  echoJittablePixel,      /* 0 */
  echoJittableLight,      /* 1 */
  echoJittableLens,       /* 2 */
  echoJittableNormalA,    /* 3 */
  echoJittableNormalB,    /* 4 */
  echoJittableMotionA,    /* 5 */
  echoJittableMotionB,    /* 6 */
  echoJittableLast
};
#define ECHO_JITTABLE_NUM    7

/*
******** echoMatter* enum
**
** the different materials that are supported.  This setting determines
** the interpretation of the vector of floats/doubles ("mat[]") that
** constitutes material information.  All objects have an rgba[] array
** seperate from material information.  The Light material is currently only
** supported on rectangles.
*/
enum {
  echoMatterUnknown=0,
  echoMatterPhong,      /* 1 */
  echoMatterGlass,      /* 2 */
  echoMatterMetal,      /* 3 */
  echoMatterLight,      /* 4 */
  echoMatterLast
};
#define ECHO_MATTER_MAX    4

enum {
  echoMatterPhongKa,    /* 0 */
  echoMatterPhongKd,    /* 1 */
  echoMatterPhongKs,    /* 2 */
  echoMatterPhongSp     /* 3 */
};
enum {
  echoMatterGlassIndex, /* 0 */
  echoMatterGlassKa,    /* 1 */
  echoMatterGlassKd,    /* 2 */
  echoMatterGlassFuzzy  /* 3 */
};
enum {
  echoMatterMetalR0,    /* 0 */
  echoMatterMetalKa,    /* 1 */
  echoMatterMetalKd,    /* 2 */
  echoMatterMetalFuzzy  /* 3 */
};
enum {
  echoMatterLightPower, /* 0 */
  echoMatterLightUnit   /* 1 : (takes over role of old parm->refDistance)
                           distance to be considered unity when calculating
                           inverse square fall-off of light intensity, or,
                           use 0.0 to mean "this is a directional light"
                           (with no fall-off at all) */
};

#define ECHO_MATTER_PARM_NUM 4

/*
******** echoType* enum
**
** the types of objects that echo supports
*/
enum {
  echoTypeUnknown=-1,
  echoTypeSphere,         /*  0 */
  echoTypeCylinder,       /*  1 */
  echoTypeSuperquad,      /*  2 */
  echoTypeCube,           /*  3 */
  echoTypeTriangle,       /*  4 */
  echoTypeRectangle,      /*  5 */
  echoTypeTriMesh,        /*  6: only triangles in the mesh */
  echoTypeIsosurface,     /*  7 */
  echoTypeAABBox,         /*  8 */
  echoTypeSplit,          /*  9 */
  echoTypeList,           /* 10 */
  echoTypeInstance,       /* 11 */
  echoTypeLast
};

#define ECHO_TYPE_NUM        12

/*
******** echoObject (generic) and all other object structs
**
** every starts with ECHO_OBJECT_COMMON, and all the "real" objects
** have a ECHO_OBJECT_MATTER following that
*/

#define ECHO_OBJECT_COMMON              \
  signed char type

#define ECHO_OBJECT_MATTER              \
  unsigned char matter;                 \
  echoCol_t rgba[4];                    \
  echoCol_t mat[ECHO_MATTER_PARM_NUM];  \
  Nrrd *ntext

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;   /* ha! its not actually in every object, but in
                           those cases were we want to access it without
                           knowing object type, then it will be there ... */
} echoObject;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  echoPos_t pos[3], rad;
} echoSphere;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  int axis;
} echoCylinder;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  int axis;
  echoPos_t A, B;
} echoSuperquad;

/* edges are unit length, [-0.5, 0.5] on every edge */
typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
} echoCube;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  echoPos_t vert[3][3];  /* e0 = vert[1]-vert[0],
                            e1 = vert[2]-vert[0],
                            normal = e0 x e1 */
} echoTriangle;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  echoPos_t origin[3], edge0[3], edge1[3];
} echoRectangle;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  echoPos_t meanvert[3], min[3], max[3];
  int numV, numF;
  echoPos_t *pos;
  int *vert;
} echoTriMesh;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  /* this needs more stuff, perhaps a gageContext */
  Nrrd *volume;
  float value;
} echoIsosurface;

typedef struct {
  ECHO_OBJECT_COMMON;
  echoObject *obj;
  echoPos_t min[3], max[3];
} echoAABBox;

typedef struct {
  ECHO_OBJECT_COMMON;
  int axis;                    /* which axis was split: 0:X, 1:Y, 2:Z */
  echoPos_t min0[3], max0[3],
    min1[3], max1[3];          /* bboxes of two children */
  echoObject *obj0, *obj1;     /* two splits, or ??? */
} echoSplit;

typedef struct {
  ECHO_OBJECT_COMMON;
  echoObject **obj;
  airArray *objArr;
} echoList;  

typedef struct {
  ECHO_OBJECT_COMMON;
  echoPos_t Mi[16], M[16];
  echoObject *obj;
} echoInstance;

/*
******** echoScene
**
** this is the central list of all objects in a scene, and all nrrds
** used for textures and isosurface volumes.  The scene "owns" all 
** the objects it points to, so that nixing it will cause all objects
** and nrrds to be nixed and nuked, respectively.
*/
typedef struct echoScene_t {
  echoObject **cat;    /* array of ALL objects and all lights */
  airArray *catArr;
  echoObject **rend;   /* array of top-level objects to be rendered */
  airArray *rendArr;
  echoObject **light;  /* convenience pointers to lights within cat[] */
  airArray *lightArr;
  Nrrd **nrrd;         /* nrrds for textures and isosurfaces */
  airArray *nrrdArr;
  Nrrd *envmap;        /* 16checker-based diffuse environment map,
                          not touched by echoSceneNix() */
  echoCol_t ambi[3],   /* color of ambient light */
    bkgr[3];           /* color of background */
} echoScene;

/*
******** echoRay
**
** all info associated with a ray being intersected against a scene
*/
typedef struct {
  echoPos_t from[3],    /* ray comes from this point */
    dir[3],             /* ray goes in this (not normalized) direction */
    neer, faar;         /* look for intx in this interval */
  int shadow;           /* this is a shadow ray */
  echoCol_t transp;     /* for shadow rays, the transparency so far; starts
                           at 1.0, goes down to 0.0 */
} echoRay;

/*
******** echoIntx
**
** all info about nature and location of an intersection 
*/
typedef struct {
  echoObject *obj;      /* computed with every intersection */
  echoPos_t t,          /* computed with every intersection */
    u, v;               /* sometimes needed for texturing */
  echoPos_t norm[3],    /* computed with every intersection */
    view[3],            /* always used with coloring */
    refl[3],            /* reflection of view across line spanned by normal */
    pos[3];             /* always used with coloring (and perhaps texturing) */
  int face,             /* in intx with cube, which face was hit 
                           (used for textures) */
    boxhits;            /* how many bounding boxes we hit */
} echoIntx;

/* enumsEcho.c ------------------------------------------ */
ECHO_EXPORT2 airEnum *echoJitter;
ECHO_EXPORT2 airEnum *echoType;
ECHO_EXPORT2 airEnum *echoMatter;

/* methodsEcho.c --------------------------------------- */
ECHO_EXPORT2 const char *echoBiffKey;
ECHO_EXPORT echoRTParm *echoRTParmNew();
ECHO_EXPORT echoRTParm *echoRTParmNix(echoRTParm *parm);
ECHO_EXPORT echoGlobalState *echoGlobalStateNew();
ECHO_EXPORT echoGlobalState *echoGlobalStateNix(echoGlobalState *state);
ECHO_EXPORT echoThreadState *echoThreadStateNew();
ECHO_EXPORT echoThreadState *echoThreadStateNix(echoThreadState *state);
ECHO_EXPORT echoScene *echoSceneNew();
ECHO_EXPORT echoScene *echoSceneNix(echoScene *scene);

/* objmethods.c --------------------------------------- */
ECHO_EXPORT echoObject *echoObjectNew(echoScene *scene, signed char type);
ECHO_EXPORT int echoObjectAdd(echoScene *scene, echoObject *obj);
ECHO_EXPORT echoObject *echoObjectNix(echoObject *obj);

/* model.c ---------------------------------------- */
ECHO_EXPORT echoObject *echoRoughSphereNew(echoScene *scene,
                                           int theRes, int phiRes,
                                           echoPos_t *matx);

/* bounds.c --------------------------------------- */
ECHO_EXPORT void echoBoundsGet(echoPos_t *lo, echoPos_t *hi, echoObject *obj);

/* list.c --------------------------------------- */
ECHO_EXPORT void echoListAdd(echoObject *parent, echoObject *child);
ECHO_EXPORT echoObject *echoListSplit(echoScene *scene,
                                      echoObject *list, int axis);
ECHO_EXPORT echoObject *echoListSplit3(echoScene *scene,
                                       echoObject *list, int depth);

/* set.c --------------------------------------- */
ECHO_EXPORT void echoSphereSet(echoObject *sphere,
                               echoPos_t x, echoPos_t y,
                               echoPos_t z, echoPos_t rad);
ECHO_EXPORT void echoCylinderSet(echoObject *cylind,
                                 int axis);
ECHO_EXPORT void echoSuperquadSet(echoObject *squad,
                                  int axis, echoPos_t A, echoPos_t B);
ECHO_EXPORT void echoRectangleSet(echoObject *rect,
                                  echoPos_t ogx, echoPos_t ogy, echoPos_t ogz,
                                  echoPos_t x0, echoPos_t y0, echoPos_t z0,
                                  echoPos_t x1, echoPos_t y1, echoPos_t z1);
ECHO_EXPORT void echoTriangleSet(echoObject *tri,
                                 echoPos_t x0, echoPos_t y0, echoPos_t z0, 
                                 echoPos_t x1, echoPos_t y1, echoPos_t z1, 
                                 echoPos_t x2, echoPos_t y2, echoPos_t z2);
ECHO_EXPORT void echoTriMeshSet(echoObject *trim,
                                int numV, echoPos_t *pos,
                                int numF, int *vert);
ECHO_EXPORT void echoInstanceSet(echoObject *inst,
                                 echoPos_t *M, echoObject *obj);

/* matter.c ------------------------------------------ */
ECHO_EXPORT2 int echoObjectHasMatter[ECHO_TYPE_NUM];
ECHO_EXPORT void echoColorSet(echoObject *obj,
                              echoCol_t R, echoCol_t G,
                              echoCol_t B, echoCol_t A);
ECHO_EXPORT void echoMatterPhongSet(echoScene *scene, echoObject *obj,
                                    echoCol_t ka, echoCol_t kd,
                                    echoCol_t ks, echoCol_t sp);
ECHO_EXPORT void echoMatterGlassSet(echoScene *scene, echoObject *obj,
                                    echoCol_t index, echoCol_t ka,
                                    echoCol_t kd, echoCol_t fuzzy);
ECHO_EXPORT void echoMatterMetalSet(echoScene *scene, echoObject *obj,
                                    echoCol_t R0, echoCol_t ka,
                                    echoCol_t kd, echoCol_t fuzzy);
ECHO_EXPORT void echoMatterLightSet(echoScene *scene, echoObject *obj,
                                    echoCol_t power, echoCol_t unit);
ECHO_EXPORT void echoMatterTextureSet(echoScene *scene, echoObject *obj,
                                      Nrrd *ntext);

/* lightEcho.c ------------------------------------------- */
ECHO_EXPORT void echoLightPosition(echoPos_t pos[3], echoObject *light,
                                   echoThreadState *tstate);
ECHO_EXPORT void echoLightColor(echoCol_t rgb[3], echoPos_t Ldist,
                                echoObject *light, echoRTParm *parm,
                                echoThreadState *tstate);
ECHO_EXPORT void echoEnvmapLookup(echoCol_t rgb[3], echoPos_t norm[3],
                                  Nrrd *envmap);

/* color.c ------------------------------------------- */
ECHO_EXPORT void echoTextureLookup(echoCol_t rgba[4], Nrrd *ntext,
                                   echoPos_t u, echoPos_t v, echoRTParm *parm);
ECHO_EXPORT void echoIntxMaterialColor(echoCol_t rgba[4], echoIntx *intx,
                                       echoRTParm *parm);
ECHO_EXPORT void echoIntxLightColor(echoCol_t ambi[3], echoCol_t diff[3],
                                    echoCol_t spec[3], echoCol_t sp,
                                    echoIntx *intx, echoScene *scene,
                                    echoRTParm *parm, echoThreadState *tstate);
ECHO_EXPORT void echoIntxFuzzify(echoIntx *intx, echoCol_t fuzz,
                                 echoThreadState *tstate);

/* intx.c ------------------------------------------- */
ECHO_EXPORT int echoRayIntx(echoIntx *intx, echoRay *ray, echoScene *scene,
                            echoRTParm *parm, echoThreadState *tstate);
ECHO_EXPORT void echoIntxColor(echoCol_t rgba[4], echoIntx *intx,
                               echoScene *scene, echoRTParm *parm,
                               echoThreadState *tstate);

/* renderEcho.c ---------------------------------------- */
ECHO_EXPORT int echoThreadStateInit(int threadIdx, echoThreadState *tstate,
                                    echoRTParm *parm, echoGlobalState *gstate);
ECHO_EXPORT void echoJitterCompute(echoRTParm *parm, echoThreadState *state);
ECHO_EXPORT void echoRayColor(echoCol_t rgba[4], echoRay *ray,
                              echoScene *scene, echoRTParm *parm,
                              echoThreadState *tstate);
ECHO_EXPORT void echoChannelAverage(echoCol_t *img,
                                    echoRTParm *parm, echoThreadState *tstate);
ECHO_EXPORT int echoRTRenderCheck(Nrrd *nraw, limnCamera *cam,
                                  echoScene *scene, echoRTParm *parm,
                                  echoGlobalState *gstate);
ECHO_EXPORT int echoRTRender(Nrrd *nraw, limnCamera *cam, echoScene *scene,
                             echoRTParm *parm, echoGlobalState *gstate);

#ifdef __cplusplus
}
#endif

#endif /* ECHO_HAS_BEEN_INCLUDED */

