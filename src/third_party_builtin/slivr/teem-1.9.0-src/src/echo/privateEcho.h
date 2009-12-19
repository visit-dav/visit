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

#ifndef ECHO_PRIVATE_HAS_BEEN_INCLUDED
#define ECHO_PRIVATE_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#if ECHO_POS_FLOAT
#  define echoPos_nt nrrdTypeFloat
#  define echoPos_at airTypeFloat
#  define ell_4m_INV ell_4m_inv_f
#  define ell_4m_PRINT ell_4m_print_f
#  define ell_4m_DET ell_4m_det_f
#  define ell_3v_PERP ell_3v_perp_f
#  define ell_4m_POST_MUL ell_4m_post_mul_f
#  define ECHO_POS_MIN (-FLT_MAX)
#  define ECHO_POS_MAX FLT_MAX
#  define ECHO_POS_EPS FLT_EPSILON
#else
#  define echoPos_nt nrrdTypeDouble
#  define echoPos_at airTypeDouble
#  define ell_4m_INV ell_4m_inv_d
#  define ell_4m_PRINT ell_4m_print_d
#  define ell_4m_DET ell_4m_det_d
#  define ell_3v_PERP ell_3v_perp_d
#  define ell_4m_POST_MUL ell_4m_post_mul_d
#  define ECHO_POS_MIN (-DBL_MAX)
#  define ECHO_POS_MAX DBL_MAX
#  define ECHO_POS_EPS DBL_EPSILON
#endif

#define OBJECT(obj)    ((echoObject*)obj)
#define SPLIT(obj)     ((echoSplit*)obj)
#define LIST(obj)      ((echoList*)obj)
#define SPHERE(obj)    ((echoSphere*)obj)
#define CYLINDER(obj)  ((echoCylinder*)obj)
#define SUPERQUAD(obj) ((echoSuperquad*)obj)
#define RECTANGLE(obj) ((echoRectangle*)obj)
#define AABBOX(obj)    ((echoAABBox*)obj)
#define TRIMESH(obj)   ((echoTriMesh*)obj)
#define TRIANGLE(obj)  ((echoTriangle*)obj)
#define INSTANCE(obj)  ((echoInstance*)obj)

#define _ECHO_REFLECT(refl, norm, view, tmp) \
  (tmp) = 2*ELL_3V_DOT((view), (norm)); \
  ELL_3V_SCALE_ADD2((refl), -1.0, (view), (tmp), (norm))

#define ECHO_NEW(TYPE) \
  (echoObject##TYPE *)echoNew(echoObject##Type)

/* methodsEcho.c */
extern void _echoSceneLightAdd(echoScene *scene, echoObject *obj);
extern void _echoSceneNrrdAdd(echoScene *scene, Nrrd *nrrd);

/* intx.c */
#define RAYINTX_ARGS(TYPE) echoIntx *intx, echoRay *ray, \
                           echo##TYPE *obj, echoRTParm *parm,  \
                           echoThreadState *tstate
typedef int (*_echoRayIntx_t)(RAYINTX_ARGS(Object));
extern _echoRayIntx_t _echoRayIntx[/* object type idx */];
typedef void (*_echoRayIntxUV_t)(echoIntx *intx);
extern _echoRayIntxUV_t _echoRayIntxUV[/* object type idx */];
extern int _echoRayIntx_CubeSolid(echoPos_t *tminP, echoPos_t *tmaxP, 
                                  echoPos_t xmin, echoPos_t xmax,
                                  echoPos_t ymin, echoPos_t ymax,
                                  echoPos_t zmin, echoPos_t zmax,
                                  echoRay *ray);

/* sqd.c */
extern int _echoRayIntx_Superquad(RAYINTX_ARGS(Superquad));

/* color.c */
extern char _echoBuff[];
extern char *_echoDot(int depth);
#define INTXCOLOR_ARGS echoCol_t rgba[4], echoIntx *intx,  \
                       echoScene *scene, echoRTParm *parm, \
                       echoThreadState *tstate
typedef void (*_echoIntxColor_t)(INTXCOLOR_ARGS);
extern _echoIntxColor_t _echoIntxColor[/* matter idx */];

#ifdef __cplusplus
}
#endif

#endif /*  ECHO_PRIVATE_HAS_BEEN_INCLUDED */
