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

#ifndef LIMN_HAS_BEEN_INCLUDED
#define LIMN_HAS_BEEN_INCLUDED

#include <stdlib.h>

#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/hest.h>
#include <teem/ell.h>
#include <teem/nrrd.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(limn_EXPORTS) || defined(teem_EXPORTS)
#    define LIMN_EXPORT extern __declspec(dllexport)
#  else
#    define LIMN_EXPORT extern __declspec(dllimport)
#  endif
#define LIMN_EXPORT2 LIMN_EXPORT
#else /* TEEM_STATIC || UNIX */
# if __GNUC__ >= 4 && (defined(limn_EXPORTS) || defined(teem_EXPORTS))
#  define LIMN_EXPORT __attribute__ ((visibility("default")))
#  define LIMN_EXPORT2 extern __attribute__ ((visibility("default")))
# else
#  define LIMN_EXPORT extern
#  define LIMN_EXPORT2 extern
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LIMN limnBiffKey
#define LIMN_LIGHT_NUM 8

/*
******** #define LIMN_SPLINE_Q_AVG_EPS
**
** The convergence tolerance used for Buss/Fillmore quaternion
** averaging
*/
#define LIMN_SPLINE_Q_AVG_EPS 0.00001

/*
****** limnCamera struct
**
** for all standard graphics camera parameters.  Image plane is
** spanned by U and V; N always points away from the viewer, U
** always points to the right, V can point up or down, if the
** camera is left- or right-handed, respectively.
**
** Has no dynamically allocated information or pointers.
*/
typedef struct limnCamera_t {
  double from[3],     /* location of eyepoint */
    at[3],            /* what eye is looking at */
    up[3],            /* what is up direction for eye (this is not updated
                         to the "true" up) */
    uRange[2],        /* range of U values to put on horiz. image axis */
    vRange[2],        /* range of V values to put on vert. image axis */
    fov,              /* if non-NaN, and aspect is non-NaN, then {u,v}Range
                         will be set accordingly by limnCameraUpdate().
                         "fov" is the angle, in degrees, vertically subtended
                         by the view window */
    aspect,           /* the ratio of horizontal to vertical size of the 
                         view window */
    neer, faar,       /* near and far clipping plane distances
                         (misspelled for the sake of a McRosopht compiler) */
    dist;             /* distance to image plane */
  int atRelative,     /* if non-zero: given neer, faar, and dist
                         quantities indicate distance relative to the
                         _at_ point, instead of the usual (in computer
                         graphics) sense if being relative to the
                         eye point */
    orthographic,     /* no perspective projection: just orthographic */
    rightHanded;      /* if rightHanded, V = NxU (V points "downwards"),
                         otherwise, V = UxN (V points "upwards") */
  /* --------------------------------------------------------------------
     End of user-set parameters.  Things below are set by limnCameraUpdate
     -------------------------------------------------------------------- */
  double W2V[16],     /* World to view transform. The _rows_ of this
                         matrix (its 3x3 submatrix) are the U, V, N
                         vectors which form the view-space coordinate frame.
                         The column-major ordering of elements in the
                         matrix is from ell:
                         0   4   8  12
                         1   5   9  13
                         2   6  10  14
                         3   7  11  15 */
    V2W[16],          /* View to world transform */
    U[4], V[4], N[4], /* View space basis vectors (in world coords)
                         last element always zero */
    vspNeer, vspFaar, /* not usually user-set: near, far, and image plane
                         distances, in view space */
    vspDist;
} limnCamera;

/*
******** struct limnLight
**
** information for directional lighting and the ambient light.  All
** the vectors are length 4 (instead of 3) for the sake of passing them
** directly to OpenGL. For colors, the last coefficient (alpha) is
** always 1.0, and for directions it is 0.0 (w, homog coord).
** 
**
** Has no dynamically allocated information or pointers
*/
typedef struct {
  float amb[4],              /* RGBA ambient light color */
    _dir[LIMN_LIGHT_NUM][4], /* direction of light[i] (view or world space).
                                This is what the user sets via limnLightSet */
    dir[LIMN_LIGHT_NUM][4],  /* direction of light[i] (ONLY world space) 
                                Not user-set: calculated/copied from _dir[] */
    col[LIMN_LIGHT_NUM][4];  /* RGBA color of light[i] */
  int on[LIMN_LIGHT_NUM],    /* light[i] is on */
    vsp[LIMN_LIGHT_NUM];     /* light[i] lives in view space */
} limnLight;

enum {
  limnDeviceUnknown,
  limnDevicePS,
  limnDeviceGL,
  limnDeviceLast
};

enum {
  limnEdgeTypeUnknown,     /* 0 */
  limnEdgeTypeBackFacet,   /* 1: back-facing non-crease */
  limnEdgeTypeBackCrease,  /* 2: back-facing crease */
  limnEdgeTypeContour,     /* 3: silhoette edge */
  limnEdgeTypeFrontCrease, /* 4: front-facing crease */
  limnEdgeTypeFrontFacet,  /* 5: front-facing non-crease */
  limnEdgeTypeBorder,      /* 6: attached to only one face */
  limnEdgeTypeLone,        /* 7: attached to no faces */
  limnEdgeTypeLast
};
#define LIMN_EDGE_TYPE_MAX    7

typedef struct {
  float lineWidth[LIMN_EDGE_TYPE_MAX+1],
    creaseAngle,      /* difference between crease and facet, in *degrees* */
    bg[3];            /* background color */
  int showpage,       /* finish with "showpage" */
    wireFrame,        /* just render wire-frame */
    noBackground;     /* refrain from initially filling with bg[] color */
} limnOptsPS;

typedef struct {
  limnOptsPS ps;
  int device;
  float scale, 
    bbox[4];          /* minX minY maxX maxY */
  int yFlip;
  FILE *file;
} limnWindow;

enum {
  limnSpaceUnknown,  /* 0 */ 
  limnSpaceWorld,    /* 1 */
  limnSpaceView,     /* 2 */
  limnSpaceScreen,   /* 3 */
  limnSpaceDevice,   /* 4 */
  limnSpaceLast
};
#define LIMN_SPACE_MAX  4

enum {
  limnPrimitiveUnknown,      /* 0 */
  limnPrimitiveTriangles,    /* 1: triangle soup */
  limnPrimitiveTriangleStrip,/* 2: triangle strip */
  limnPrimitiveTriangleFan,  /* 3: triangle fan */
  limnPrimitiveQuads,        /* 4: quad soup */
  limnPrimitiveLineStrip,    /* 5: line strip */
  limnPrimitiveLast
};
#define LIMN_PRIMITIVE_MAX      5

/*
******** struct limnLook
**
** surface properties: pretty much anything having to do with 
** appearance, for points, edges, faces, etc.
*/
typedef struct {
  float rgba[4];
  float kads[3],              /* phong: ka, kd, ks */
    spow;                     /* specular power */
} limnLook;

/*
******** struct limnVertex
**
** all the information you might want for a point.
**
** This used to have seperate coordinate arrays for view, screen, and 
** device space, but these have been collapsed (in the interest of space)
** into coord, with obj->vertSpace indicating which space these are in.
** This also used to have a lookIdx (now just rgba[4]), and a partIdx
** (which was never actually used).
**
** Has no dynamically allocated information or pointers
*/
typedef struct {
  float world[4],             /* world coordinates (homogeneous) */
    rgba[4],                  /* RGBA color */
    coord[4],                 /* coordinates in some space */
    worldNormal[3];           /* vertex normal (world coords only) */
} limnVertex;

/*
******** struct limnVrt
**
** a very abbreviated limnVertex
*/
typedef struct {
  float xyzw[4],              /* homogeneous coordinates */
    norm[3];                  /* normal */
  unsigned char rgba[4];      /* RGBA color */
} limnVrt;

/*
******** struct limnEdge
**
** all the information about an edge
**
** Has no dynamically allocated information or pointers
*/
typedef struct limnEdge_t {
  unsigned int vertIdx[2], /* indices into object's master vert array */
    lookIdx,               /* index into parent's look array */
    partIdx;               /* which part do we belong to */
  int type,                /* from the limnEdgeType enum */
    faceIdx[2],            /* indices into object's master face array */
    once;                  /* flag used for certain kinds of rendering */
} limnEdge;

/*
******** struct limnFace
**
** all the information about a face
*/
typedef struct limnFace_t {
  float worldNormal[3],
    screenNormal[3];
  unsigned int *vertIdx,      /* regular array (*not* airArray) of vertex
                                 indices in object's master vert array */
    *edgeIdx,                 /* likewise for edges */
    sideNum,                  /* number of sides (allocated length of
                                 {vert,edge}Idx arrays */
    lookIdx,
    partIdx;
  int visible;                /* non-zero if face currently visible */
  float depth;
} limnFace;

/*
******** struct limnPart
**
** one connected part of an object
*/
typedef struct limnPart_t {
  /* (air)arrays of indices in object's vert array */
  unsigned int *vertIdx, vertIdxNum; 
  airArray *vertIdxArr;

  /* (air)arrays of indices in object's edge array */
  unsigned int *edgeIdx, edgeIdxNum;
  airArray *edgeIdxArr;

  /* (air)arrays of indices in object's face array */
  unsigned int *faceIdx, faceIdxNum;
  airArray *faceIdxArr;
  int lookIdx;
  float depth;
} limnPart;

/*
******** struct limnObject
**
** the beast used to represent polygonal objects
**
** Relies on many dynamically allocated arrays
**
** learned: I used to have an array of limnParts inside here, and that
** array was managed by an airArray.  Inside the limnPart, are more
** airArrays, for example the faceIdxArr which internally stores the
** *address* of faceIdx.  When the array of limnParts is resized, the
** limnPart's faceIdx pointer is still valid, and faceIdxArr is still
** valid, but the faceIdxArr's internal pointer to the faceIdx pointer
** is now bogus.  Thus: the weakness of airArrays (as long as they
** aren't giving the data pointer anew for EACH ACCESS), is that you
** must not confuse the airArrays by changing the address of its user
** data pointer.  Putting user data pointers inside of a bigger
** airArray is a fine way to create such confusion.
*/
typedef struct {
  limnVertex *vert; unsigned int vertNum;
  airArray *vertArr;

  limnEdge *edge; unsigned int edgeNum;
  airArray *edgeArr;

  limnFace *face; unsigned int faceNum;
  airArray *faceArr;
  limnFace **faceSort;    /* pointers into "face", sorted by depth */
  
  limnPart **part; unsigned int partNum;  /* double indirection, see above */
  airArray *partArr;

  limnPart **partPool; unsigned int partPoolNum;
  airArray *partPoolArr;
  
  limnLook *look; unsigned int lookNum;
  airArray *lookArr;

  int vertSpace,           /* which space limnVertex->coord is in */
    setVertexRGBAFromLook, /* when possible, copy vertex RGB values
                              from limnLook of part (not face) */
    doEdges;               /* if non-zero, build edges as faces are added */
  unsigned incr;           /* increment to use with airArrays */
} limnObject;

/*
******** limnPolyData
**
** A simpler beast for representing polygonal surfaces and other things
**
** There is no notion of "part" here; there may be multiple
** disconnected pieces inside the surface, but there is no way of
** accessing just one such piece (because this is organized in terms
** of primitives, and each piece may be made of multiple primitives).
** Having separate parts is important for PostScript rendering, but
** the limnPolyData is more OpenGL oriented.
**
** Experimenting with *not* having airArrays here...
*/
typedef struct {
  unsigned int vertNum;  /* there are vertNum limnVrts in vert[] */
  limnVrt *vert;
  
  unsigned int indxNum;  /* there are indxNum vertex indices in indx[] */
  unsigned int *indx;    /* all indices (into vert[]) for all primitives,
                            concatenated together into one array */

  unsigned int primNum;  /* there are primNum primitives (tris or tristrips) */
  signed char *type;     /* prim ii is a type[ii] (limnPrimitive* enum) */
  unsigned int *vcnt;    /* prim ii has vcnt[ii] vertices */
} limnPolyData;

typedef struct {
  /* ------- input ------- */
  const Nrrd *nvol;             /* the volume we're operating on */
  int lowerInside,              /* lower values are logically inside
                                   isosurfaces, not outside */
    findNormals;                /* find normals for isosurface vertices with
                                   forward and central differencing */
  double transform[16];         /* map vertices through this transform;
                                   defaults to identity.  This is apt to come
                                   from gageShape->ItoW, but having this as
                                   a separate field allows limn to avoid
                                   dependence on gage */
  /* ------ internal ----- */
  int reverse;                  /* reverse sense of inside/outside (based on
                                   lowerInside and determinant of transform) */
  double normalTransform[9];    /* how to transform normals */
  Nrrd *nspanHist;              /* image of span space */
  NrrdRange *range;             /* to store min and max of nvol */
  size_t sx, sy, sz,            /* dimensions */
    spanSize;                   /* resolution of span space along edge */
  int *vidx;                    /* 5 * sx * sy array of vertex index
                                   offsets, to support re-using of vertices
                                   across voxels and slices */
  double *val;                  /* 4 * (sx+2) * (sy+2) array as value cache,
                                   with Z as fastest axis, and one sample
                                   of padding on all sides */
  /* ------ output ----- */
  unsigned int
    voxNum, vertNum, faceNum;   /* number of voxels contributing to latest
                                   isosurface, and number of vertices and
                                   faces in that isosurface */
  double time;                  /* time for extraction */
} limnContour3DContext;

/*
******** limnQN enum
**
** the different quantized normal schemes currently supported
*/
enum {
  limnQNUnknown,     /*  0 */
  limnQN16simple,    /*  1 */
  limnQN16border1,   /*  2 */
  limnQN16checker,   /*  3 */
  limnQN16octa,      /*  4 */
  limnQN15octa,      /*  5 */
  limnQN14checker,   /*  6 */
  limnQN14octa,      /*  7 */
  limnQN13octa,      /*  8 */
  limnQN12checker,   /*  9 */
  limnQN12octa,      /* 10 */
  limnQN11octa,      /* 11 */
  limnQN10checker,   /* 12 */
  limnQN10octa,      /* 13 */
  limnQN9octa,       /* 14 */
  limnQN8checker,    /* 15 */
  limnQN8octa,       /* 16 */
  limnQNLast
};
#define LIMN_QN_MAX     16

enum {
  limnSplineTypeUnknown,     /* 0 */
  limnSplineTypeLinear,      /* 1 */
  limnSplineTypeTimeWarp,    /* 2 */
  limnSplineTypeHermite,     /* 3 */
  limnSplineTypeCubicBezier, /* 4 */
  limnSplineTypeBC,          /* 5 */
  limnSplineTypeLast
};
#define LIMN_SPLINE_TYPE_MAX    5

enum {
  limnSplineInfoUnknown,    /* 0 */
  limnSplineInfoScalar,     /* 1 */
  limnSplineInfo2Vector,    /* 2 */
  limnSplineInfo3Vector,    /* 3 */
  limnSplineInfoNormal,     /* 4 */
  limnSplineInfo4Vector,    /* 5 */
  limnSplineInfoQuaternion, /* 6 */
  limnSplineInfoLast
};
#define LIMN_SPLINE_INFO_MAX   6

enum {
  limnCameraPathTrackUnknown, /* 0 */
  limnCameraPathTrackFrom,    /* 1: 3-D spline for *from* points, quaternion
                                 spline for camera directions towards at */
  limnCameraPathTrackAt,      /* 2: 3-D spline for *at* points, quaternion 
                                 spline for directions back to camera */
  limnCameraPathTrackBoth,    /* 3: three 3-D splines: for from point, at
                                 point, and the up vector */
  limnCameraPathTrackLast
};
#define LIMN_CAMERA_PATH_TRACK_MAX 3

/*
******** limnSpline
**
** the ncpt nrrd stores control point information in a 3-D nrrd, with
** sizes C by 3 by N, where C is the number of values needed for each 
** point (3 for 3Vecs, 1 for scalars), and N is the number of control
** points.  The 3 things per control point are 0) the pre-point info 
** (either inward tangent or an internal control point), 1) the control
** point itself, 2) the post-point info (e.g., outward tangent).
**
** NOTE: for the sake of simplicity, the ncpt nrrd is always "owned"
** by the limnSpline, that is, it is COPIED from the one given in 
** limnSplineNew() (and is converted to type double along the way),
** and it will is deleted with limnSplineNix.
*/
typedef struct limnSpline_t {
  int type,          /* from limnSplineType* enum */
    info,            /* from limnSplineInfo* enum */
    loop;            /* the last (implicit) control point is the first */
  double B, C;       /* B,C values for BC-splines */
  Nrrd *ncpt;        /* the control point info, ALWAYS a 3-D nrrd */
  double *time;      /* ascending times for non-uniform control points.
                        Currently, only used for limnSplineTypeTimeWarp */
} limnSpline;

typedef struct limnSplineTypeSpec_t {
  int type;          /* from limnSplineType* enum */
  double B, C;       /* B,C values for BC-splines */
} limnSplineTypeSpec;

/* defaultsLimn.c */
LIMN_EXPORT2 const char *limnBiffKey;
LIMN_EXPORT2 int limnDefCameraAtRelative;
LIMN_EXPORT2 int limnDefCameraOrthographic;
LIMN_EXPORT2 int limnDefCameraRightHanded;

/* enumsLimn.c */
LIMN_EXPORT2 airEnum *limnSpace;
LIMN_EXPORT2 airEnum *limnCameraPathTrack;

/* qn.c */
LIMN_EXPORT2 int limnQNBins[LIMN_QN_MAX+1];
LIMN_EXPORT2 void (*limnQNtoV_f[LIMN_QN_MAX+1])(float *vec, int qn);
LIMN_EXPORT2 void (*limnQNtoV_d[LIMN_QN_MAX+1])(double *vec, int qn);
LIMN_EXPORT2 int (*limnVtoQN_f[LIMN_QN_MAX+1])(float *vec);
LIMN_EXPORT2 int (*limnVtoQN_d[LIMN_QN_MAX+1])(double *vec);

/* light.c */
LIMN_EXPORT void limnLightSet(limnLight *lit, int which, int vsp,
                              float r, float g, float b,
                              float x, float y, float z);
LIMN_EXPORT void limnLightAmbientSet(limnLight *lit,
                                     float r, float g, float b);
LIMN_EXPORT void limnLightSwitch(limnLight *lit, int which, int on);
LIMN_EXPORT void limnLightReset(limnLight *lit);
LIMN_EXPORT int limnLightUpdate(limnLight *lit, limnCamera *cam);

/* envmap.c */
typedef void (*limnEnvMapCB)(float rgb[3], float vec[3], void *data);
LIMN_EXPORT int limnEnvMapFill(Nrrd *envMap, limnEnvMapCB cb, 
                               int qnMethod, void *data);
LIMN_EXPORT void limnLightDiffuseCB(float rgb[3], float vec[3], void *_lit);
LIMN_EXPORT int limnEnvMapCheck(Nrrd *envMap);

/* methodsLimn.c */
LIMN_EXPORT limnLight *limnLightNew(void);
LIMN_EXPORT void limnCameraInit(limnCamera *cam);
LIMN_EXPORT limnLight *limnLightNix(limnLight *);
LIMN_EXPORT limnCamera *limnCameraNew(void);
LIMN_EXPORT limnCamera *limnCameraNix(limnCamera *cam);
LIMN_EXPORT limnWindow *limnWindowNew(int device);
LIMN_EXPORT limnWindow *limnWindowNix(limnWindow *win);

/* hestLimn.c */
LIMN_EXPORT void limnHestCameraOptAdd(hestOpt **hoptP, limnCamera *cam,
                                      char *frDef, char *atDef, char *upDef,
                                      char *dnDef, char *diDef, char *dfDef,
                                      char *urDef, char *vrDef, char *fvDef);

/* cam.c */
LIMN_EXPORT int limnCameraAspectSet(limnCamera *cam,
                                    int horz, int vert, int centering);
LIMN_EXPORT int limnCameraUpdate(limnCamera *cam);
LIMN_EXPORT int limnCameraPathMake(limnCamera *cam, int numFrames,
                                   limnCamera *keycam, double *time,
                                   int numKeys, int trackFrom, 
                                   limnSplineTypeSpec *quatType,
                                   limnSplineTypeSpec *posType,
                                   limnSplineTypeSpec *distType,
                                   limnSplineTypeSpec *viewType);

/* obj.c */
LIMN_EXPORT int limnObjectLookAdd(limnObject *obj);
LIMN_EXPORT limnObject *limnObjectNew(int incr, int doEdges);
LIMN_EXPORT limnObject *limnObjectNix(limnObject *obj);
LIMN_EXPORT void limnObjectEmpty(limnObject *obj);
LIMN_EXPORT int limnObjectPreSet(limnObject *obj,
                                 unsigned int partNum,
                                 unsigned int lookNum,
                                 unsigned int vertPerPart,
                                 unsigned int edgePerPart,
                                 unsigned int facePerPart);
LIMN_EXPORT int limnObjectPartAdd(limnObject *obj);
LIMN_EXPORT int limnObjectVertexNumPreSet(limnObject *obj,
                                          unsigned int partIdx, 
                                          unsigned int vertNum);
LIMN_EXPORT int limnObjectVertexAdd(limnObject *obj,
                                    unsigned int partIdx,
                                    float x, float y, float z);
LIMN_EXPORT int limnObjectEdgeAdd(limnObject *obj,
                                  unsigned int partIdx,
                                  unsigned int lookIdx,
                                  unsigned int faceIdx,
                                  unsigned int vertIdx0,
                                  unsigned int vertIdx1);
LIMN_EXPORT int limnObjectFaceNumPreSet(limnObject *obj,
                                        unsigned int partIdx,
                                        unsigned int faceNum);
LIMN_EXPORT int limnObjectFaceAdd(limnObject *obj,
                                  unsigned int partIdx,
                                  unsigned int lookIdx,
                                  unsigned int sideNum, 
                                  unsigned int *vertIdx);

/* polydata.c */
LIMN_EXPORT limnPolyData *limnPolyDataNew(void);
LIMN_EXPORT limnPolyData *limnPolyDataNix(limnPolyData *pld);
LIMN_EXPORT int limnPolyDataAlloc(limnPolyData *pld,
                                  unsigned int vertNum,
                                  unsigned int indxNum,
                                  unsigned int primNum);
LIMN_EXPORT size_t limnPolyDataSize(limnPolyData *pld);
LIMN_EXPORT int limnPolyDataCopy(limnPolyData *pldB, const limnPolyData *pldA);
LIMN_EXPORT int limnPolyDataCopyN(limnPolyData *pldB, const limnPolyData *pldA,
                                  unsigned int num);
LIMN_EXPORT int limnPolyDataCube(limnPolyData *pld, int sharpEdge);
LIMN_EXPORT int limnPolyDataCylinder(limnPolyData *pld,
                                     unsigned int res, int sharpEdge);
LIMN_EXPORT int limnPolyDataSuperquadric(limnPolyData *pld,
                                         float A, float B,
                                         unsigned int thetaRes,
                                         unsigned int phiRes);
LIMN_EXPORT int limnPolyDataSpiralSuperquadric(limnPolyData *pld,
                                               float A, float B,
                                               unsigned int thetaRes,
                                               unsigned int phiRes);
LIMN_EXPORT int limnPolyDataPolarSphere(limnPolyData *pld,
                                        unsigned int thetaRes,
                                        unsigned int phiRes);
LIMN_EXPORT int limnPolyDataSpiralSphere(limnPolyData *pld,
                                         unsigned int thetaRes,
                                         unsigned int phiRes);
LIMN_EXPORT int limnPolyDataPlane(limnPolyData *pld,
                                  unsigned int uRes, unsigned int vRes);
LIMN_EXPORT void limnPolyDataTransform_f(limnPolyData *pld,
                                         const float homat[16]);
LIMN_EXPORT void limnPolyDataTransform_d(limnPolyData *pld,
                                         const double homat[16]);
LIMN_EXPORT unsigned int limnPolyDataPolygonNumber(limnPolyData *pld);

/* io.c */
LIMN_EXPORT int limnObjectDescribe(FILE *file, limnObject *obj);
LIMN_EXPORT int limnObjectOFFRead(limnObject *obj, FILE *file);
LIMN_EXPORT int limnObjectOFFWrite(FILE *file, limnObject *obj);

/* shapes.c */
LIMN_EXPORT int limnObjectCubeAdd(limnObject *obj, unsigned int lookIdx);
LIMN_EXPORT int limnObjectSquareAdd(limnObject *obj, unsigned int lookIdx);
LIMN_EXPORT int limnObjectLoneEdgeAdd(limnObject *obj, unsigned int lookIdx);
LIMN_EXPORT int limnObjectCylinderAdd(limnObject *obj, unsigned int lookIdx,
                                      unsigned int axis, unsigned int res);
LIMN_EXPORT int limnObjectPolarSphereAdd(limnObject *obj, unsigned int lookIdx,
                                         unsigned int axis,
                                         unsigned int thetaRes,
                                         unsigned int phiRes);
LIMN_EXPORT int limnObjectConeAdd(limnObject *obj, unsigned int lookIdx,
                                  unsigned int axis, unsigned int res);
LIMN_EXPORT int limnObjectPolarSuperquadAdd(limnObject *obj,
                                            unsigned int lookIdx,
                                            unsigned int axis,
                                            float A, float B,
                                            unsigned int thetaRes,
                                            unsigned int phiRes);

/* transform.c */
LIMN_EXPORT int limnObjectWorldHomog(limnObject *obj);
LIMN_EXPORT int limnObjectFaceNormals(limnObject *obj, int space);
LIMN_EXPORT int limnObjectVertexNormals(limnObject *obj);
LIMN_EXPORT int limnObjectSpaceTransform(limnObject *obj, limnCamera *cam,
                                         limnWindow *win, int space);
LIMN_EXPORT int limnObjectPartTransform(limnObject *obj, unsigned int partIdx,
                                        float tx[16]);
LIMN_EXPORT int limnObjectDepthSortParts(limnObject *obj);
LIMN_EXPORT int limnObjectDepthSortFaces(limnObject *obj);
LIMN_EXPORT int limnObjectFaceReverse(limnObject *obj);

/* renderLimn.c */
LIMN_EXPORT int limnObjectRender(limnObject *obj, limnCamera *cam,
                                 limnWindow *win);
LIMN_EXPORT int limnObjectPSDraw(limnObject *obj, limnCamera *cam,
                                 Nrrd *envMap, limnWindow *win);
LIMN_EXPORT int limnObjectPSDrawConcave(limnObject *obj, limnCamera *cam,
                                        Nrrd *envMap, limnWindow *win);

/* splineMethods.c */
LIMN_EXPORT limnSplineTypeSpec *limnSplineTypeSpecNew(int type, ...);
LIMN_EXPORT limnSplineTypeSpec *
  limnSplineTypeSpecNix(limnSplineTypeSpec *spec);
LIMN_EXPORT limnSpline *limnSplineNew(Nrrd *ncpt, int info,
                                      limnSplineTypeSpec *spec);
LIMN_EXPORT limnSpline *limnSplineNix(limnSpline *spline);
LIMN_EXPORT int limnSplineNrrdCleverFix(Nrrd *nout, Nrrd *nin,
                                        int info, int type);
LIMN_EXPORT limnSpline *limnSplineCleverNew(Nrrd *ncpt, int info,
                                            limnSplineTypeSpec *spec);
LIMN_EXPORT int limnSplineUpdate(limnSpline *spline, Nrrd *ncpt);

/* splineMisc.c */
LIMN_EXPORT2 airEnum *limnSplineType;
LIMN_EXPORT2 airEnum *limnSplineInfo;
LIMN_EXPORT limnSpline *limnSplineParse(char *str);
LIMN_EXPORT limnSplineTypeSpec *limnSplineTypeSpecParse(char *str);
LIMN_EXPORT2 hestCB *limnHestSpline;
LIMN_EXPORT2 hestCB *limnHestSplineTypeSpec;
LIMN_EXPORT2 unsigned int limnSplineInfoSize[LIMN_SPLINE_INFO_MAX+1];
LIMN_EXPORT2 int limnSplineTypeHasImplicitTangents[LIMN_SPLINE_TYPE_MAX+1];
LIMN_EXPORT int limnSplineNumPoints(limnSpline *spline);
LIMN_EXPORT double limnSplineMinT(limnSpline *spline);
LIMN_EXPORT double limnSplineMaxT(limnSpline *spline);
LIMN_EXPORT void limnSplineBCSet(limnSpline *spline, double B, double C);

/* splineEval.c */
LIMN_EXPORT void limnSplineEvaluate(double *out,
                                    limnSpline *spline, double time);
LIMN_EXPORT int limnSplineNrrdEvaluate(Nrrd *nout,
                                       limnSpline *spline, Nrrd *nin);
LIMN_EXPORT int limnSplineSample(Nrrd *nout, limnSpline *spline,
                                 double minT, size_t M, double maxT);

/* contour.c */
LIMN_EXPORT limnContour3DContext *limnContour3DContextNew(void);
LIMN_EXPORT limnContour3DContext *
  limnContour3DContextNix(limnContour3DContext *);
LIMN_EXPORT int limnContour3DVolumeSet(limnContour3DContext *lctx,
                                       const Nrrd *nvol);
LIMN_EXPORT int limnContour3DLowerInsideSet(limnContour3DContext *lctx,
                                            int lowerInside);
LIMN_EXPORT int limnContour3DTransformSet(limnContour3DContext *lctx,
                                          const double mat[16]);
LIMN_EXPORT int limnContour3DExtract(limnContour3DContext *lctx,
                                     limnObject *cont, double isovalue);

#ifdef __cplusplus
}
#endif

#endif /* LIMN_HAS_BEEN_INCLUDED */
