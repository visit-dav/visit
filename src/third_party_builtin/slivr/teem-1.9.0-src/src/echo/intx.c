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

/*
** TODO: do whatever possible to minimize the amount of work 
** needed for shadow rays 
*/

int _echoVerbose = 0;

/*
** ALL of the intersection functions are responsible for setting 
** 1) intx->norm to a NORMALIZED direction
** 2) intx->t
** 3) intx->obj
** Specifically, these things must be set for non-shadow rays.  Setting
** them for shadow rays is optional. Setting intx->u and intx->v texture
** coords is always optional.
**
** Setting intx->pos and intx->view (normalized) is done by echoRayIntx
*/

int
_echoRayIntx_Noop(RAYINTX_ARGS(Object)) {

  AIR_UNUSED(intx);
  AIR_UNUSED(ray);
  AIR_UNUSED(obj);
  AIR_UNUSED(parm);
  AIR_UNUSED(tstate);
  return 0;
}

int
_echoRayIntx_CubeSurf(echoPos_t *tP, int *axP, int *dirP,
                      echoPos_t xmin, echoPos_t xmax,
                      echoPos_t ymin, echoPos_t ymax,
                      echoPos_t zmin, echoPos_t zmax,
                      echoRay *ray) {
  echoPos_t txmin, tymin, tzmin, txmax, tymax, tzmax,
    dx, dy, dz, ox, oy, oz, tmin, tmax;
  int axmin, axmax, sgn[3];

  ELL_3V_GET(dx, dy, dz, ray->dir);
  ELL_3V_GET(ox, oy, oz, ray->from);
  if (dx >= 0) { txmin = (xmin - ox)/dx; txmax = (xmax - ox)/dx; sgn[0] = -1; }
  else         { txmin = (xmax - ox)/dx; txmax = (xmin - ox)/dx; sgn[0] =  1; }
  if (dy >= 0) { tymin = (ymin - oy)/dy; tymax = (ymax - oy)/dy; sgn[1] = -1; }
  else         { tymin = (ymax - oy)/dy; tymax = (ymin - oy)/dy; sgn[1] =  1; }
  if (dz >= 0) { tzmin = (zmin - oz)/dz; tzmax = (zmax - oz)/dz; sgn[2] = -1; }
  else         { tzmin = (zmax - oz)/dz; tzmax = (zmin - oz)/dz; sgn[2] =  1; }
  if (txmin > tymin) { tmin = txmin; axmin = 0; }
  else               { tmin = tymin; axmin = 1; }
  if (tzmin > tmin)  { tmin = tzmin; axmin = 2; }
  if (txmax < tymax) { tmax = txmax; axmax = 0; }
  else               { tmax = tymax; axmax = 1; }
  if (tzmax < tmax)  { tmax = tzmax; axmax = 2; }
  if (tmin >= tmax)
    return AIR_FALSE;
  *tP = tmin;
  *axP = axmin;
  *dirP = sgn[axmin];
  if (!AIR_IN_CL(ray->neer, tmin, ray->faar)) {
    *tP = tmax;
    *axP = axmax;
    *dirP = -sgn[axmax];
    if (!AIR_IN_CL(ray->neer, tmax, ray->faar)) {
      return AIR_FALSE;
    }
  }
  return AIR_TRUE;
}

int
_echoRayIntx_CubeSolid(echoPos_t *tminP, echoPos_t *tmaxP, 
                       echoPos_t xmin, echoPos_t xmax,
                       echoPos_t ymin, echoPos_t ymax,
                       echoPos_t zmin, echoPos_t zmax,
                       echoRay *ray) {
  echoPos_t txmin, tymin, tzmin, txmax, tymax, tzmax,
    dx, dy, dz, ox, oy, oz, tmin, tmax;

  ELL_3V_GET(dx, dy, dz, ray->dir);
  ELL_3V_GET(ox, oy, oz, ray->from);
  if (dx >= 0) { txmin = (xmin - ox)/dx; txmax = (xmax - ox)/dx; }
  else         { txmin = (xmax - ox)/dx; txmax = (xmin - ox)/dx; }
  if (dy >= 0) { tymin = (ymin - oy)/dy; tymax = (ymax - oy)/dy; }
  else         { tymin = (ymax - oy)/dy; tymax = (ymin - oy)/dy; }
  if (dz >= 0) { tzmin = (zmin - oz)/dz; tzmax = (zmax - oz)/dz; }
  else         { tzmin = (zmax - oz)/dz; tzmax = (zmin - oz)/dz; }
  if (txmin > tymin) tmin = txmin;
  else               tmin = tymin;
  if (tzmin > tmin)  tmin = tzmin;
  if (txmax < tymax) tmax = txmax;
  else               tmax = tymax;
  if (tzmax < tmax)  tmax = tzmax;
  if (tmin >= tmax)
    return AIR_FALSE;

  if ( ray->faar < tmin || ray->neer > tmax )
    return AIR_FALSE;

  *tminP = AIR_MAX(tmin, ray->neer);
  *tmaxP = AIR_MIN(tmax, ray->faar);
  return AIR_TRUE;
}

int
_echoRayIntx_Sphere(RAYINTX_ARGS(Sphere)) {
  echoPos_t t, A, B, C, r[3], dscr, pos[3], tmp;

  AIR_UNUSED(parm);
  AIR_UNUSED(tstate);
  ELL_3V_SUB(r, ray->from, obj->pos);
  A = ELL_3V_DOT(ray->dir, ray->dir);
  B = 2*ELL_3V_DOT(ray->dir, r);
  C = ELL_3V_DOT(r, r) - obj->rad*obj->rad;
  dscr = B*B - 4*A*C;
  if (dscr <= 0) {
    /* grazes or misses (most common case) */
    return AIR_FALSE;
  }
  /* else */
  dscr = sqrt(dscr);
  t = (-B - dscr)/(2*A);
  if (!AIR_IN_CL(ray->neer, t, ray->faar)) {
    t = (-B + dscr)/(2*A);
    if (!AIR_IN_CL(ray->neer, t, ray->faar)) {
      return AIR_FALSE;
    }
  }
  /* else one of the intxs is in [neer,faar] segment */
  intx->t = t;
  ELL_3V_SCALE_ADD2(pos, 1, ray->from, t, ray->dir);
  ELL_3V_SUB(intx->norm, pos, obj->pos);
  ELL_3V_NORM(intx->norm, intx->norm, tmp);
  intx->obj = OBJECT(obj);
  /* does NOT set u, v */
  return AIR_TRUE;
}

void
_echoRayIntxUV_Sphere(echoIntx *intx) {
  echoPos_t u, v;

  if (intx->norm[0] || intx->norm[1]) {
    u = atan2(intx->norm[1], intx->norm[0]);
    intx->u = AIR_AFFINE(-AIR_PI, u, AIR_PI, 0.0, 1.0);
    v = -asin(intx->norm[2]);
    intx->v = AIR_AFFINE(-AIR_PI/2, v, AIR_PI/2, 0.0, 1.0);
  }
  else {
    intx->u = 0;
    /* this is valid because if we're here, then intx->norm[2]
       is either 1.0 or -1.0 */
    intx->v = AIR_AFFINE(1.0, intx->norm[2], -1.0, 0.0, 1.0);
  }
}

int
_echoRayIntx_Cylinder(RAYINTX_ARGS(Cylinder)) {
  echoPos_t A, B, C, aa, bb, cc, dd, ee, ff, dscr, cylt1, cylt2, t, tmax,
    twot[2], cylp1, cylp2, pos[3], tmp;
  int tidx, radi0, radi1, twocap[2], cap;

  AIR_UNUSED(parm);
  AIR_UNUSED(tstate);

  if (!_echoRayIntx_CubeSolid(&t, &tmax,
                              -1-ECHO_EPSILON, 1+ECHO_EPSILON,
                              -1-ECHO_EPSILON, 1+ECHO_EPSILON,
                              -1-ECHO_EPSILON, 1+ECHO_EPSILON, ray)) {
    return AIR_FALSE;
  }
  switch(obj->axis) {
  case 0:
    radi0 = 1; radi1 = 2;
    break;
  case 1:
    radi0 = 0; radi1 = 2;
    break;
  case 2: default:
    radi0 = 0; radi1 = 1;
    break;
  }
  aa = ray->dir[radi0];
  bb = ray->dir[radi1];
  ee = ray->dir[obj->axis];
  cc = ray->from[radi0];
  dd = ray->from[radi1];
  ff = ray->from[obj->axis];
  A = aa*aa + bb*bb;
  B = 2*(aa*cc + bb*dd);
  C = cc*cc + dd*dd - 1;
  dscr = B*B - 4*A*C;
  if (dscr <= 0) {
    /* infinite ray grazes or misses the infinite cylinder (not
       bounded to [-1,1] along cylinder's axis), so therefore the ray
       grazes or misses the actual cylinder */
    return AIR_FALSE;
  }
  /* else infinite ray intersects the infinite cylinder */
  dscr = sqrt(dscr);
  cylt1 = (-B - dscr)/(2*A);
  cylp1 = ff + cylt1*ee;
  cylt2 = (-B + dscr)/(2*A);
  cylp2 = ff + cylt2*ee;
  if ( (cylp1 <= -1 && cylp2 <= -1)
       || (cylp1 >= 1 && cylp2 >= 1) ) {
    /* both intersections with infinite cylinder lie on ONE side of the
       finite extent, so there can't be an intersection */
    return AIR_FALSE;
  }
  /* else infinite ray DOES intersect finite cylinder; we have to find
     if any of the intersections are in the [neer, faar] interval */
  tidx = 0;
  if (AIR_IN_CL(-1, cylp1, 1)) {
    twot[tidx] = cylt1;
    twocap[tidx] = 0;
    tidx++;
  }
  if (AIR_IN_CL(-1, cylp2, 1)) {
    twot[tidx] = cylt2;
    twocap[tidx] = 0;
    tidx++;
  }
  if (tidx < 2) {
    /* at least one of the two intersections is with the endcaps */
    t = (-ff - 1)/ee;
    ELL_3V_SCALE_ADD2(pos, 1, ray->from, t, ray->dir);
    aa = pos[radi0]; bb = pos[radi1]; cc = aa*aa + bb*bb;
    if (cc <= 1) {
      twot[tidx] = t;
      twocap[tidx] = 1;
      tidx++;
    }
    if (tidx < 2) {
      /* try other endcap */
      t = (-ff + 1)/ee;
      ELL_3V_SCALE_ADD2(pos, 1, ray->from, t, ray->dir);
      aa = pos[radi0]; bb = pos[radi1]; cc = aa*aa + bb*bb;
      if (cc <= 1) {
        twot[tidx] = t;
        twocap[tidx] = 1;
        tidx++;
      }
    }
  }
  if (!tidx) {
    return AIR_FALSE;
  }
  if (2 == tidx && twot[0] > twot[1]) {
    ELL_SWAP2(twot[0], twot[1], aa);
    ELL_SWAP2(twocap[0], twocap[1], cap);
  }
  t = twot[0];
  cap = twocap[0];
  if (!AIR_IN_CL(ray->neer, t, ray->faar)) {
    if (1 == tidx) {
      return AIR_FALSE;
    }
    t = twot[1];
    cap = twocap[1];
    if (!AIR_IN_CL(ray->neer, t, ray->faar)) {
      return AIR_FALSE;
    }
  }
  /* else one of the intxs is in [neer,faar] segment */
  intx->t = t;
  ELL_3V_SCALE_ADD2(pos, 1, ray->from, t, ray->dir);
  switch(obj->axis) {
  case 0:
    ELL_3V_SET(intx->norm,     cap*pos[0], (1-cap)*pos[1], (1-cap)*pos[2]);
    break;
  case 1:
    ELL_3V_SET(intx->norm, (1-cap)*pos[0],     cap*pos[1], (1-cap)*pos[2]);
    break;
  case 2: default:
    ELL_3V_SET(intx->norm, (1-cap)*pos[0], (1-cap)*pos[1],     cap*pos[2]);
    break;
  }
  ELL_3V_NORM(intx->norm, intx->norm, tmp);
  intx->obj = OBJECT(obj);
  /* does NOT set u, v */
  return AIR_TRUE;
}

int
_echoRayIntx_Cube(RAYINTX_ARGS(Cube)) {
  echoPos_t t;
  int ax, dir;

  AIR_UNUSED(parm);
  if (!_echoRayIntx_CubeSurf(&t, &ax, &dir,
                             -1, 1,
                             -1, 1,
                             -1, 1, ray)) 
    return AIR_FALSE;
  intx->obj = (echoObject *)obj;
  intx->t = t;
  switch(ax) {
  case 0: ELL_3V_SET(intx->norm, dir, 0, 0); break;
  case 1: ELL_3V_SET(intx->norm, 0, dir, 0); break;
  case 2: ELL_3V_SET(intx->norm, 0, 0, dir); break;
  }
  intx->face = ax + 3*(dir + 1)/2;
  if (tstate->verbose) {
    fprintf(stderr, "%s%s: ax = %d --> norm = (%g,%g,%g)\n",
            _echoDot(tstate->depth), "_echoRayIntx_Cube", ax,
            intx->norm[0], intx->norm[1], intx->norm[2]);
  }
  /* does NOT set u, v */
  return AIR_TRUE;
}

void
_echoRayIntxUV_Cube(echoIntx *intx) {
  echoPos_t x, y, z;

  ELL_3V_GET(x, y, z, intx->pos);
  switch(intx->face) {
  case 0:
    intx->u = AIR_AFFINE(-1, y, 1, 0.0, 1.0);
    intx->v = AIR_AFFINE(-1, -z, 1, 0.0, 1.0);
    break;
  case 1:
    intx->u = AIR_AFFINE(-1, -x, 1, 0.0, 1.0);
    intx->v = AIR_AFFINE(-1, -z, 1, 0.0, 1.0);
    break;
  case 2:
    intx->u = AIR_AFFINE(-1, -x, 1, 0.0, 1.0);
    intx->v = AIR_AFFINE(-1, y, 1, 0.0, 1.0);
    break;
  case 3:
    intx->u = AIR_AFFINE(-1, -y, 1, 0.0, 1.0);
    intx->v = AIR_AFFINE(-1, z, 1, 0.0, 1.0);
    break;
  case 4:
    intx->u = AIR_AFFINE(-1, x, 1, 0.0, 1.0);
    intx->v = AIR_AFFINE(-1, z, 1, 0.0, 1.0);
    break;
  case 5:
    intx->u = AIR_AFFINE(-1, x, 1, 0.0, 1.0);
    intx->v = AIR_AFFINE(-1, -y, 1, 0.0, 1.0);
    break;
  }
}

/*
** TRI_INTX
**
** given a triangle in terms of origin, edge0, edge1, this will
** begin the intersection calculation:
** - sets pvec, tvec, qvec (all of them if intx is not ruled out )
** - sets u, and rules out intx based on (u < 0.0 || u > 1.0)
** - sets v, and rules out intx based on COND
** - sets t, and rules out intx based on (t < neer || t > faar)
*/
#define TRI_INTX(ray, origin, edge0, edge1, pvec, qvec, tvec,                \
                 det, t, u, v, COND, NOPE)                                   \
  ELL_3V_CROSS(pvec, ray->dir, edge1);                                       \
  det = ELL_3V_DOT(pvec, edge0);                                             \
  if (det > -ECHO_EPSILON && det < ECHO_EPSILON) {                           \
    NOPE;                                                                    \
  }                                                                          \
  /* now det is the reciprocal of the determinant */                         \
  det = 1.0/det;                                                             \
  ELL_3V_SUB(tvec, ray->from, origin);                                       \
  u = det * ELL_3V_DOT(pvec, tvec);                                          \
  if (u < 0.0 || u > 1.0) {                                                  \
    NOPE;                                                                    \
  }                                                                          \
  ELL_3V_CROSS(qvec, tvec, edge0);                                           \
  v = det * ELL_3V_DOT(qvec, ray->dir);                                      \
  if (COND) {                                                                \
    NOPE;                                                                    \
  }                                                                          \
  t = det * ELL_3V_DOT(qvec, edge1);                                         \
  if (t < ray->neer || t > ray->faar) {                                      \
    NOPE;                                                                    \
  }

int
_echoRayIntx_Rectangle(RAYINTX_ARGS(Rectangle)) {
  echoPos_t pvec[3], qvec[3], tvec[3], det, t, u, v, *edge0, *edge1, tmp;
  
  AIR_UNUSED(tstate);
  if (echoMatterLight == obj->matter
      && (ray->shadow || !parm->renderLights)) {
    return AIR_FALSE;
  }
  edge0 = obj->edge0;
  edge1 = obj->edge1;
  TRI_INTX(ray, obj->origin, edge0, edge1,
           pvec, qvec, tvec, det, t, u, v,
           (v < 0.0 || v > 1.0), return AIR_FALSE);
  intx->t = t;
  intx->u = u;
  intx->v = v;
  ELL_3V_CROSS(intx->norm, edge0, edge1);
  ELL_3V_NORM(intx->norm, intx->norm, tmp);
  intx->obj = OBJECT(obj);
  /* DOES set u, v */
  return AIR_TRUE;
}

int
_echoRayIntx_Triangle(RAYINTX_ARGS(Triangle)) {
  echoPos_t pvec[3], qvec[3], tvec[3], det, t, u, v, edge0[3], edge1[3], tmp;
  
  AIR_UNUSED(parm);
  AIR_UNUSED(tstate);
  ELL_3V_SUB(edge0, obj->vert[1], obj->vert[0]);
  ELL_3V_SUB(edge1, obj->vert[2], obj->vert[0]);
  TRI_INTX(ray, obj->vert[0], edge0, edge1,
           pvec, qvec, tvec, det, t, u, v,
            (v < 0.0 || u + v > 1.0), return AIR_FALSE);
  intx->t = t;
  intx->u = u;
  intx->v = v;
  ELL_3V_CROSS(intx->norm, edge0, edge1);
  ELL_3V_NORM(intx->norm, intx->norm, tmp);
  intx->obj = (echoObject *)obj;
  /* DOES set u, v */
  return AIR_TRUE;
}

int
_echoRayIntx_TriMesh(RAYINTX_ARGS(TriMesh)) {
  echoPos_t *pos, vert0[3], edge0[3], edge1[3], pvec[3], qvec[3], tvec[3],
    det, t, tmax, u, v, tmp;
  echoTriMesh *trim;
  int i, ret;

  AIR_UNUSED(parm);
  trim = TRIMESH(obj);
  if (!_echoRayIntx_CubeSolid(&t, &tmax,
                              trim->min[0], trim->max[0],
                              trim->min[1], trim->max[1],
                              trim->min[2], trim->max[2], ray)) {
    if (tstate->verbose) {
      fprintf(stderr, "%s%s: trimesh bbox (%g,%g,%g) --> (%g,%g,%g) not hit\n",
              _echoDot(tstate->depth), "_echoRayIntx_TriMesh",
              trim->min[0], trim->min[1], trim->min[2],
              trim->max[0], trim->max[1], trim->max[2]);
    }
    return AIR_FALSE;
  }
  /* stupid linear search for now */
  ret = AIR_FALSE;
  for (i=0; i<trim->numF; i++) {
    pos = trim->pos + 3*trim->vert[0 + 3*i];
    ELL_3V_COPY(vert0, pos);
    pos = trim->pos + 3*trim->vert[1 + 3*i];
    ELL_3V_SUB(edge0, pos, vert0);
    pos = trim->pos + 3*trim->vert[2 + 3*i];
    ELL_3V_SUB(edge1, pos, vert0);
    TRI_INTX(ray, vert0, edge0, edge1,
             pvec, qvec, tvec, det, t, u, v,
             (v < 0.0 || u + v > 1.0), continue);
    if (ray->shadow) {
      return AIR_TRUE;
    }
    intx->t = ray->faar = t;
    ELL_3V_CROSS(intx->norm, edge0, edge1);
    ELL_3V_NORM(intx->norm, intx->norm, tmp);
    intx->obj = (echoObject *)obj;
    intx->face = i;
    ret = AIR_TRUE;
  }
  /* does NOT set u, v */
  return ret;
}

void
_echoRayIntxUV_TriMesh(echoIntx *intx) {
  echoPos_t u, v, norm[3], len;
  echoTriMesh *trim;

  trim = TRIMESH(intx->obj);
  ELL_3V_SUB(norm, intx->pos, trim->meanvert);
  ELL_3V_NORM(norm, norm, len);
  if (norm[0] || norm[1]) {
    u = atan2(norm[1], norm[0]);
    intx->u = AIR_AFFINE(-AIR_PI, u, AIR_PI, 0.0, 1.0);
    v = -asin(norm[2]);
    intx->v = AIR_AFFINE(-AIR_PI/2, v, AIR_PI/2, 0.0, 1.0);
  }
  else {
    intx->u = 0;
    intx->v = AIR_AFFINE(1.0, norm[2], -1.0, 0.0, 1.0);
  }
}

int
_echoRayIntx_AABBox(RAYINTX_ARGS(AABBox)) {
  int ret;
  echoAABBox *box;
  echoPos_t t, tmax;

  box = AABBOX(obj);
  if (_echoRayIntx_CubeSolid(&t, &tmax,
                             box->min[0], box->max[0],
                             box->min[1], box->max[1],
                             box->min[2], box->max[2], ray)) {
    intx->boxhits++;
    ret = _echoRayIntx[box->obj->type](intx, ray, box->obj, parm, tstate);
  } else {
    ret = AIR_FALSE;
  }
  return ret;
}

int
_echoRayIntx_Split(RAYINTX_ARGS(Split)) {
  char me[]="_echoRayIntx_Split";
  echoObject *a, *b;
  echoPos_t *mina, *minb, *maxa, *maxb, t, tmax;
  int ret;

  if (ray->dir[obj->axis] > 0) {
    a = obj->obj0;
    mina = obj->min0;
    maxa = obj->max0;
    b = obj->obj1;
    minb = obj->min1;
    maxb = obj->max1;
  }
  else {
    a = obj->obj1;
    mina = obj->min1;
    maxa = obj->max1;
    b = obj->obj0;
    minb = obj->min0;
    maxb = obj->max0;
  }
  
  if (tstate->verbose) {
    fprintf(stderr, "%s%s: (shadow = %d):\n",
            _echoDot(tstate->depth), me, ray->shadow);
    fprintf(stderr, "%s%s: 1st: (%g,%g,%g) -- (%g,%g,%g) (obj %d)\n", 
            _echoDot(tstate->depth), me,
            mina[0], mina[1], mina[2],
            maxa[0], maxa[1], maxa[2], a->type);
    fprintf(stderr, "%s%s: 2nd: (%g,%g,%g) -- (%g,%g,%g) (obj %d)\n",
            _echoDot(tstate->depth), me,
            minb[0], minb[1], minb[2],
            maxb[0], maxb[1], maxb[2], b->type);
  }

  ret = AIR_FALSE;
  if (_echoRayIntx_CubeSolid(&t, &tmax,
                             mina[0], maxa[0],
                             mina[1], maxa[1],
                             mina[2], maxa[2], ray)) {
    intx->boxhits++;
    if (_echoRayIntx[a->type](intx, ray, a, parm, tstate)) {
      if (ray->shadow) {
        return AIR_TRUE;
      }
      ray->faar = intx->t;
      ret = AIR_TRUE;
    }
  }
  if (_echoRayIntx_CubeSolid(&t, &tmax,
                             minb[0], maxb[0],
                             minb[1], maxb[1],
                             minb[2], maxb[2], ray)) {
    intx->boxhits++;
    if (_echoRayIntx[b->type](intx, ray, b, parm, tstate)) {
      ray->faar = intx->t;
      ret = AIR_TRUE;
    }
  }
  return ret;
}

int
_echoRayIntx_List(RAYINTX_ARGS(List)) {
  unsigned int i;
  int ret;
  echoObject *kid;

  ret = AIR_FALSE;
  for (i=0; i<obj->objArr->len; i++) {
    kid = obj->obj[i];
    if (_echoRayIntx[kid->type](intx, ray, kid, parm, tstate)) {
      ray->faar = intx->t;
      ret = AIR_TRUE;
      if (ray->shadow) {
        /* no point in testing any further */
        return ret;
      }
    }
  }
  
  return ret;
}

int
_echoRayIntx_Instance(RAYINTX_ARGS(Instance)) {
  echoPos_t a[4], b[4], tmp;
  echoRay iray;

  /*
  ELL_3V_COPY(iray.from, ray->from);
  ELL_3V_COPY(iray.dir, ray->dir);
  */
  ELL_4V_SET(a, ray->from[0], ray->from[1], ray->from[2], 1);
  ELL_4MV_MUL(b, obj->Mi, a);
  ELL_34V_HOMOG(iray.from, b);
  ELL_4V_SET(a, ray->dir[0], ray->dir[1], ray->dir[2], 0);
  ELL_4MV_MUL(b, obj->Mi, a); 
  ELL_3V_COPY(iray.dir, b);
  if (tstate->verbose) {
    fprintf(stderr, "%s%s: dir (%g,%g,%g)\n%s   -- Mi --> "
            "(%g,%g,%g,%g)\n%s   --> (%g,%g,%g)\n",
            _echoDot(tstate->depth), "_echoRayIntx_Instance",
            a[0], a[1], a[2], _echoDot(tstate->depth),
            b[0], b[1], b[2], b[3], _echoDot(tstate->depth), 
            iray.dir[0], iray.dir[1], iray.dir[2]);
  }
  
  iray.neer = ray->neer;
  iray.faar = ray->faar;
  iray.shadow = ray->shadow;

  if (_echoRayIntx[obj->obj->type](intx, &iray, obj->obj, parm, tstate)) {
    ELL_4V_SET(a, intx->norm[0], intx->norm[1], intx->norm[2], 0);
    ELL_4MV_TMUL(b, obj->Mi, a);
    ELL_3V_COPY(intx->norm, b);
    ELL_3V_NORM(intx->norm, intx->norm, tmp);
    if (tstate->verbose) {
      fprintf(stderr, "%s%s: hit a %d (at t=%g) with M == \n", 
              _echoDot(tstate->depth), "_echoRayIntx_Instance",
              obj->obj->type, intx->t);
      ell_4m_PRINT(stderr, obj->M);
      fprintf(stderr, "%s   ... (det = %f), and Mi == \n",
              _echoDot(tstate->depth), ell_4m_DET(obj->M));
      ell_4m_PRINT(stderr, obj->Mi);
    }
    return AIR_TRUE;
  }
  /* else */
  return AIR_FALSE;
}

void
_echoRayIntxUV_Noop(echoIntx *intx) {

  AIR_UNUSED(intx);
}

/*
** NB: the intersections with real objects need to normalize
** intx->norm
*/      
_echoRayIntx_t
_echoRayIntx[ECHO_TYPE_NUM] = {
  (_echoRayIntx_t)_echoRayIntx_Sphere,
  (_echoRayIntx_t)_echoRayIntx_Cylinder,
  (_echoRayIntx_t)_echoRayIntx_Superquad,
  (_echoRayIntx_t)_echoRayIntx_Cube,
  (_echoRayIntx_t)_echoRayIntx_Triangle,
  (_echoRayIntx_t)_echoRayIntx_Rectangle,
  (_echoRayIntx_t)_echoRayIntx_TriMesh,
  (_echoRayIntx_t)_echoRayIntx_Noop,
  (_echoRayIntx_t)_echoRayIntx_AABBox,
  (_echoRayIntx_t)_echoRayIntx_Split,
  (_echoRayIntx_t)_echoRayIntx_List,
  (_echoRayIntx_t)_echoRayIntx_Instance,
};

_echoRayIntxUV_t
_echoRayIntxUV[ECHO_TYPE_NUM] = {
  _echoRayIntxUV_Sphere,  /* echoTypeSphere */
  _echoRayIntxUV_Noop,    /* echoTypeCylinder */
  _echoRayIntxUV_Noop,    /* sqd.c: echoTypeSuperquad */  
  _echoRayIntxUV_Cube,    /* echoTypeCube */
  _echoRayIntxUV_Noop,    /* echoTypeTriangle */
  _echoRayIntxUV_Noop,    /* echoTypeRectangle */
  _echoRayIntxUV_TriMesh, /* echoTypeTriMesh */
  _echoRayIntxUV_Noop,    /* echoTypeIsosurface */
  _echoRayIntxUV_Noop,    /* echoTypeAABBox */
  _echoRayIntxUV_Noop,    /* echoTypeSplit */
  _echoRayIntxUV_Noop,    /* echoTypeList */
  _echoRayIntxUV_Noop     /* echoTypeInstance */
};

int
echoRayIntx(echoIntx *intx, echoRay *ray, echoScene *scene,
            echoRTParm *parm, echoThreadState *tstate) {
  unsigned int idx;
  int ret;
  echoObject *kid;
  echoPos_t tmp;

  _echoVerbose = tstate->verbose;

  ret = AIR_FALSE;
  for (idx=0; idx<scene->rendArr->len; idx++) {
    kid = scene->rend[idx];
    if (_echoRayIntx[kid->type](intx, ray, kid, parm, tstate)) {
      ray->faar = intx->t;
      ret = AIR_TRUE;
      if (ray->shadow) {
        /* no point in testing any further */
        return ret;
      }
    }
  }
  if (ret) {
    /* being here means we're not a shadow ray */
    ELL_3V_SCALE_ADD2(intx->pos, 1, ray->from, intx->t, ray->dir);
    ELL_3V_SCALE(intx->view, -1, ray->dir);
    ELL_3V_NORM(intx->view, intx->view, tmp);
    /* this is needed for phong materials; for glass and metal,
       it is either used directly, or as a reference in fuzzification */
    _ECHO_REFLECT(intx->refl, intx->norm, intx->view, tmp);
  }

  return ret;
}
