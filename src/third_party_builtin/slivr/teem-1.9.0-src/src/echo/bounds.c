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

#include "echo.h"
#include "privateEcho.h"

typedef void (*_echoBoundsGet_t)(echoPos_t lo[3], echoPos_t hi[3],
                                 echoObject *obj);

extern _echoBoundsGet_t _echoBoundsGet[ECHO_TYPE_NUM];

#define BNDS_TMPL(TYPE, BODY)                                             \
void                                                                      \
_echo##TYPE##_bounds(echoPos_t lo[3], echoPos_t hi[3], echo##TYPE *obj) { \
  int dummy=0;                                                            \
                                                                          \
  do { BODY dummy=dummy;} while (0);                                      \
  lo[0] -= ECHO_EPSILON;                                                  \
  lo[1] -= ECHO_EPSILON;                                                  \
  lo[2] -= ECHO_EPSILON;                                                  \
  hi[0] += ECHO_EPSILON;                                                  \
  hi[1] += ECHO_EPSILON;                                                  \
  hi[2] += ECHO_EPSILON;                                                  \
  return;                                                                 \
}

BNDS_TMPL(Sphere,
          lo[0] = obj->pos[0] - obj->rad;
          lo[1] = obj->pos[1] - obj->rad;
          lo[2] = obj->pos[2] - obj->rad;
          hi[0] = obj->pos[0] + obj->rad;
          hi[1] = obj->pos[1] + obj->rad;
          hi[2] = obj->pos[2] + obj->rad;
          )

BNDS_TMPL(Cylinder,
          AIR_UNUSED(obj);
          ELL_3V_SET(lo, -1, -1, -1);
          ELL_3V_SET(hi,  1,  1,  1);
          )

BNDS_TMPL(Superquad,
          AIR_UNUSED(obj);
          ELL_3V_SET(lo, -1, -1, -1);
          ELL_3V_SET(hi,  1,  1,  1);
          )

BNDS_TMPL(Cube,
          AIR_UNUSED(obj);
          ELL_3V_SET(lo, -1, -1, -1);
          ELL_3V_SET(hi,  1,  1,  1);
          )

BNDS_TMPL(Triangle,
          ELL_3V_COPY(lo, obj->vert[0]);
          ELL_3V_MIN(lo, lo, obj->vert[1]);
          ELL_3V_MIN(lo, lo, obj->vert[2]);
          ELL_3V_COPY(hi, obj->vert[0]);
          ELL_3V_MAX(hi, hi, obj->vert[1]);
          ELL_3V_MAX(hi, hi, obj->vert[2]);
          )

BNDS_TMPL(Rectangle,
          echoPos_t v[3][3];

          ELL_3V_COPY(lo, obj->origin);
          ELL_3V_ADD2(v[0], lo, obj->edge0);
          ELL_3V_ADD2(v[1], lo, obj->edge1);
          ELL_3V_ADD2(v[2], v[0], obj->edge1);
          ELL_3V_MIN(lo, lo, v[0]);
          ELL_3V_MIN(lo, lo, v[1]);
          ELL_3V_MIN(lo, lo, v[2]);
          ELL_3V_COPY(hi, obj->origin);
          ELL_3V_MAX(hi, hi, v[0]);
          ELL_3V_MAX(hi, hi, v[1]);
          ELL_3V_MAX(hi, hi, v[2]);
          )

BNDS_TMPL(TriMesh,
          ELL_3V_COPY(lo, obj->min);
          ELL_3V_COPY(hi, obj->max);
          )
     
BNDS_TMPL(Isosurface,
          AIR_UNUSED(obj);
          fprintf(stderr, "_echoIsosurface_bounds: unimplemented!\n");
          )
     
BNDS_TMPL(AABBox,
          ELL_3V_COPY(lo, obj->min);
          ELL_3V_COPY(hi, obj->max);
          )

BNDS_TMPL(List,
          unsigned int i;
          echoPos_t l[3];
          echoPos_t h[3];
          echoObject *o;

          ELL_3V_SET(lo, ECHO_POS_MAX, ECHO_POS_MAX, ECHO_POS_MAX);
          ELL_3V_SET(hi, ECHO_POS_MIN, ECHO_POS_MIN, ECHO_POS_MIN);
          for (i=0; i<obj->objArr->len; i++) {
            o = obj->obj[i];
            _echoBoundsGet[o->type](l, h, o);
            ELL_3V_MIN(lo, lo, l);
            ELL_3V_MAX(hi, hi, h);
          }
          )

BNDS_TMPL(Split,
          AIR_UNUSED(obj);
          fprintf(stderr, "_echoSplit_bounds: unimplemented!\n");
          )

BNDS_TMPL(Instance,
          echoPos_t a[8][4];
          echoPos_t b[8][4];
          echoPos_t l[3];
          echoPos_t h[3];

          _echoBoundsGet[obj->obj->type](l, h, obj->obj);
          ELL_4V_SET(a[0], l[0], l[1], l[2], 1);
          ELL_4V_SET(a[1], h[0], l[1], l[2], 1);
          ELL_4V_SET(a[2], l[0], h[1], l[2], 1);
          ELL_4V_SET(a[3], h[0], h[1], l[2], 1);
          ELL_4V_SET(a[4], l[0], l[1], h[2], 1);
          ELL_4V_SET(a[5], h[0], l[1], h[2], 1);
          ELL_4V_SET(a[6], l[0], h[1], h[2], 1);
          ELL_4V_SET(a[7], h[0], h[1], h[2], 1);
          ELL_4MV_MUL(b[0], obj->M, a[0]); ELL_4V_HOMOG(b[0], b[0]);
          ELL_4MV_MUL(b[1], obj->M, a[1]); ELL_4V_HOMOG(b[1], b[1]);
          ELL_4MV_MUL(b[2], obj->M, a[2]); ELL_4V_HOMOG(b[2], b[2]);
          ELL_4MV_MUL(b[3], obj->M, a[3]); ELL_4V_HOMOG(b[3], b[3]);
          ELL_4MV_MUL(b[4], obj->M, a[4]); ELL_4V_HOMOG(b[4], b[4]);
          ELL_4MV_MUL(b[5], obj->M, a[5]); ELL_4V_HOMOG(b[5], b[5]);
          ELL_4MV_MUL(b[6], obj->M, a[6]); ELL_4V_HOMOG(b[6], b[6]);
          ELL_4MV_MUL(b[7], obj->M, a[7]); ELL_4V_HOMOG(b[7], b[7]);
          ELL_3V_MIN(lo, b[0], b[1]);
          ELL_3V_MIN(lo, lo, b[2]);
          ELL_3V_MIN(lo, lo, b[3]);
          ELL_3V_MIN(lo, lo, b[4]);
          ELL_3V_MIN(lo, lo, b[5]);
          ELL_3V_MIN(lo, lo, b[6]);
          ELL_3V_MIN(lo, lo, b[7]);
          ELL_3V_MAX(hi, b[0], b[1]);
          ELL_3V_MAX(hi, hi, b[2]);
          ELL_3V_MAX(hi, hi, b[3]);
          ELL_3V_MAX(hi, hi, b[4]);
          ELL_3V_MAX(hi, hi, b[5]);
          ELL_3V_MAX(hi, hi, b[6]);
          ELL_3V_MAX(hi, hi, b[7]);
          )
          
_echoBoundsGet_t
_echoBoundsGet[ECHO_TYPE_NUM] = {
  (_echoBoundsGet_t)_echoSphere_bounds,
  (_echoBoundsGet_t)_echoCylinder_bounds,
  (_echoBoundsGet_t)_echoSuperquad_bounds,
  (_echoBoundsGet_t)_echoCube_bounds,
  (_echoBoundsGet_t)_echoTriangle_bounds,
  (_echoBoundsGet_t)_echoRectangle_bounds,
  (_echoBoundsGet_t)_echoTriMesh_bounds,
  (_echoBoundsGet_t)_echoIsosurface_bounds,
  (_echoBoundsGet_t)_echoAABBox_bounds,
  (_echoBoundsGet_t)_echoSplit_bounds,
  (_echoBoundsGet_t)_echoList_bounds,
  (_echoBoundsGet_t)_echoInstance_bounds,
};

void
echoBoundsGet(echoPos_t *lo, echoPos_t *hi, echoObject *obj) {
  _echoBoundsGet[obj->type](lo, hi, obj);
}
