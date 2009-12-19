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


#include "limn.h"

int
limnObjectWorldHomog(limnObject *obj) {
  char me[]="limnObjectWorldHomog", err[BIFF_STRLEN];
  unsigned int vertIdx;
  limnVertex *vert;
  float h;

  if (!obj) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  for (vertIdx=0; vertIdx<obj->vertNum; vertIdx++) {
    vert = obj->vert + vertIdx;
    h = AIR_CAST(float, 1.0/vert->world[3]);
    ELL_3V_SCALE(vert->world, h, vert->world);
    vert->world[3] = 1.0;
    ELL_3V_NORM_TT(vert->worldNormal, float, vert->worldNormal, h);
  }
  
  return 0;
}

int
limnObjectFaceNormals(limnObject *obj, int space) {
  char me[]="limnObjectFaceNormals", err[BIFF_STRLEN];
  unsigned int vii, faceIdx;
  limnFace *face;
  limnVertex *vert0, *vert1, *vert2;
  float vec1[3], vec2[3], cross[3], nn[3], norm;

  if (space != limnSpaceWorld && space != obj->vertSpace) {
    sprintf(err, "%s: desired (%s) != object (%s) space", me,
            airEnumStr(limnSpace, space),
            airEnumStr(limnSpace, obj->vertSpace));
    biffAdd(LIMN, err); return 1;
  }
  for (faceIdx=0; faceIdx<obj->faceNum; faceIdx++) {
    face = obj->face + faceIdx;
    /* add up cross products at all vertices */
    ELL_3V_SET(nn, 0, 0, 0);
    for (vii=0; vii<face->sideNum; vii++) {
      vert0 = obj->vert + face->vertIdx[vii];
      vert1 = (obj->vert 
               + face->vertIdx[AIR_MOD((int)vii+1, (int)face->sideNum)]);
      vert2 = (obj->vert 
               + face->vertIdx[AIR_MOD((int)vii-1, (int)face->sideNum)]);
      if (limnSpaceWorld == space) {
        ELL_3V_SUB(vec1, vert1->world, vert0->world);
        ELL_3V_SUB(vec2, vert2->world, vert0->world);
      }
      else {
        ELL_3V_SUB(vec1, vert1->coord, vert0->coord);
        ELL_3V_SUB(vec2, vert2->coord, vert0->coord);
      }
      ELL_3V_CROSS(cross, vec1, vec2);
      ELL_3V_ADD2(nn, nn, cross);
    }

    if (limnSpaceWorld == space) {
      ELL_3V_NORM_TT(face->worldNormal, float, nn, norm);
      /*
      printf("%s: worldNormal[%d] = %g %g %g\n", me, faceIdx,
             face->worldNormal[0], face->worldNormal[1],
             face->worldNormal[2]);
      */
    }
    else {
      ELL_3V_NORM_TT(face->screenNormal, float, nn, norm);
      /*
      printf("%s: sn[%d] = %g %g %g\n", me, faceIdx,
             f->sn[0], f->sn[1], f->sn[2]);
      */
    }
  }

  return 0;
}

int
limnObjectVertexNormals(limnObject *obj) {
  /* char me[]="limnObjectVertexNormals", err[BIFF_STRLEN]; */
  unsigned int vertIdx, vertIdxIdx, faceIdx;
  limnVertex *vert;
  limnFace *face;
  double norm;

  for (vertIdx=0; vertIdx<obj->vertNum; vertIdx++) {
    vert = obj->vert + vertIdx;
    ELL_3V_SET(vert->worldNormal, 0, 0, 0);
  }
  for (faceIdx=0; faceIdx<obj->faceNum; faceIdx++) {
    face = obj->face + faceIdx;
    for (vertIdxIdx=0; vertIdxIdx<face->sideNum; vertIdxIdx++) {
      vert = obj->vert + face->vertIdx[vertIdxIdx];
      ELL_3V_INCR(vert->worldNormal, face->worldNormal);
    }
  }
  for (vertIdx=0; vertIdx<obj->vertNum; vertIdx++) {
    vert = obj->vert + vertIdx;
    ELL_3V_NORM_TT(vert->worldNormal, float, vert->worldNormal, norm);
  }
  return 0;
}

int
_limnObjectViewTransform(limnObject *obj, limnCamera *cam) {
  unsigned int vertIdx;
  limnVertex *vert;
  float d;

  for (vertIdx=0; vertIdx<obj->vertNum; vertIdx++) {
    vert = obj->vert + vertIdx;
    ELL_4MV_MUL_TT(vert->coord, float, cam->W2V, vert->world);
    d = AIR_CAST(float, 1.0/vert->world[3]);
    ELL_4V_SCALE(vert->coord, d, vert->coord);
    /*
    printf("%s: w[%d] = %g %g %g %g --> v = %g %g %g\n", 
           "_limnObjectVTransform",
           pi, p->w[0], p->w[1], p->w[2], p->w[3], p->v[0], p->v[1], p->v[2]);
    */
  }
  obj->vertSpace = limnSpaceView;
  return 0;
}

int
_limnObjectScreenTransform(limnObject *obj, limnCamera *cam) {
  char me[]="_limnObjectScreenTransform", err[BIFF_STRLEN];
  unsigned int vertIdx;
  limnVertex *vert;
  float d;

  if (limnSpaceView != obj->vertSpace) {
    sprintf(err, "%s: object's verts in %s (not %s) space", me,
            airEnumStr(limnSpace, obj->vertSpace),
            airEnumStr(limnSpace, limnSpaceView));
    biffAdd(LIMN, err); return 1;
  }
  for (vertIdx=0; vertIdx<obj->vertNum; vertIdx++) {
    vert = obj->vert + vertIdx;
    d = (cam->orthographic 
         ? 1.0f
         : AIR_CAST(float, cam->vspDist/vert->coord[2]));
    vert->coord[0] *= d;
    vert->coord[1] *= d;
    /* coord[2] unchanged */
    /*
    printf("%s: v[%d] = %g %g %g --> s = %g %g %g\n", "_limnObjectSTransform",
           pi, p->v[0], p->v[1], p->v[2], p->s[0], p->s[1], p->s[2]);
    */
  }
  obj->vertSpace = limnSpaceScreen;
  return 0;
}

int
_limnObjectDeviceTransform(limnObject *obj, limnCamera *cam,
                           limnWindow *win) {
  char me[]="_limnObjectDeviceTransform", err[BIFF_STRLEN];
  unsigned int vertIdx;
  limnVertex *vert;
  float wy0, wy1, wx0, wx1, t;
  
  if (limnSpaceScreen != obj->vertSpace) {
    sprintf(err, "%s: object's verts in %s (not %s) space", me,
            airEnumStr(limnSpace, obj->vertSpace),
            airEnumStr(limnSpace, limnSpaceScreen));
    biffAdd(LIMN, err); return 1;
  }
  wx0 = 0;
  wx1 = AIR_CAST(float, (cam->uRange[1] - cam->uRange[0])*win->scale);
  wy0 = 0;
  wy1 = AIR_CAST(float, (cam->vRange[1] - cam->vRange[0])*win->scale);
  ELL_4V_SET(win->bbox, wx0, wy0, wx1, wy1);
  if (win->yFlip) {
    ELL_SWAP2(wy0, wy1, t);
  }
  for (vertIdx=0; vertIdx<obj->vertNum; vertIdx++) {
    vert = obj->vert + vertIdx;
    vert->coord[0] = AIR_CAST(float, AIR_AFFINE(cam->uRange[0], vert->coord[0],
                                                cam->uRange[1], wx0, wx1));
    vert->coord[1] = AIR_CAST(float, AIR_AFFINE(cam->vRange[0], vert->coord[1],
                                                cam->vRange[1], wy0, wy1));
    /* coord[2] unchanged */
    /*
    printf("%s: s[%d] = %g %g --> s = %g %g\n", "_limnObjectDTransform",
           pi, p->s[0], p->s[1], p->d[0], p->d[1]);
    */
  }
  obj->vertSpace = limnSpaceDevice;
  return 0;
}

int
limnObjectSpaceTransform(limnObject *obj, limnCamera *cam,
                      limnWindow *win, int space) {
  char me[]="limnObjectSpaceTransform", err[BIFF_STRLEN];
  int E=0;

  /* HEY: deal with cam->orthographic */
  switch(space) {
  case limnSpaceView:
    E = _limnObjectViewTransform(obj, cam);
    break;
  case limnSpaceScreen:
    E = _limnObjectScreenTransform(obj, cam);
    break;
  case limnSpaceDevice:
    E = _limnObjectDeviceTransform(obj, cam, win);
    break;
  default:
    sprintf(err, "%s: space %d unknown or unimplemented\n", me, space);
    biffAdd(LIMN, err); return 1;
    break;
  }
  if (E) {
    sprintf(err, "%s: trouble", me);
    biffAdd(LIMN, err); return 1;
  }

  return 0;
}

int
limnObjectPartTransform(limnObject *obj, unsigned int partIdx,
                        float xform[16]) {
  unsigned int vertIdxIdx;
  limnPart *part;
  limnVertex *vert;
  float tmp[4];
  
  part= obj->part[partIdx];
  for (vertIdxIdx=0; vertIdxIdx<part->vertIdxNum; vertIdxIdx++) {
    vert = obj->vert + part->vertIdx[vertIdxIdx];
    ELL_4MV_MUL(tmp, xform, vert->world);
    ELL_4V_COPY(vert->world, tmp);
  }

  return 0;
}

int
_limnPartDepthCompare(const void *_a, const void *_b) {
  limnPart **a;
  limnPart **b;

  a = (limnPart **)_a;
  b = (limnPart **)_b;
  return AIR_COMPARE((*b)->depth, (*a)->depth);
}

int
limnObjectDepthSortParts(limnObject *obj) {
  limnPart *part;
  limnVertex *vert;
  limnFace *face;
  limnEdge *edge;
  unsigned int partIdx, ii;

  for (partIdx=0; partIdx<obj->partNum; partIdx++) {
    part = obj->part[partIdx];
    part->depth = 0;
    for (ii=0; ii<part->vertIdxNum; ii++) {
      vert = obj->vert + part->vertIdx[ii];
      part->depth += vert->coord[2];
    }
    part->depth /= part->vertIdxNum;
  }
  
  qsort(obj->part, obj->partNum, sizeof(limnPart*), _limnPartDepthCompare);
  
  /* re-assign partIdx, post-sorting */
  for (partIdx=0; partIdx<obj->partNum; partIdx++) {
    part = obj->part[partIdx];
    for (ii=0; ii<part->edgeIdxNum; ii++) {
      edge = obj->edge + part->edgeIdx[ii];
      edge->partIdx = partIdx;
    }
    for (ii=0; ii<part->faceIdxNum; ii++) {
      face = obj->face + part->faceIdx[ii];
      face->partIdx = partIdx;
    }
  }

  return 0;
}

int
_limnFaceDepthCompare(const void *_a, const void *_b) {
  limnFace **a;
  limnFace **b;

  a = (limnFace **)_a;
  b = (limnFace **)_b;
  return -AIR_COMPARE((*a)->depth, (*b)->depth);
}

int
limnObjectDepthSortFaces(limnObject *obj) {
  limnFace *face;
  limnVertex *vert;
  unsigned int faceIdx, vii;

  obj->faceSort = (limnFace **)calloc(obj->faceNum, sizeof(limnFace *));
  for (faceIdx=0; faceIdx<obj->faceNum; faceIdx++) {
    face = obj->face + faceIdx;
    face->depth = 0;
    for (vii=0; vii<face->sideNum; vii++) {
      vert = obj->vert + face->vertIdx[vii];
      face->depth += vert->coord[2];
    }
    face->depth /= face->sideNum;
    obj->faceSort[faceIdx] = face;
  }

  qsort(obj->faceSort, obj->faceNum,
        sizeof(limnFace *), _limnFaceDepthCompare);

  return 0;
}

int
limnObjectFaceReverse(limnObject *obj) {
  char me[]="limnObjectFaceReverse", err[BIFF_STRLEN];
  limnFace *face;
  unsigned int faceIdx, sii;
  int *buff;

  if (!obj) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  buff = NULL;
  for (faceIdx=0; faceIdx<obj->faceNum; faceIdx++) {
    face = obj->face + faceIdx;
    buff = (int *)calloc(face->sideNum, sizeof(int));
    if (!(buff)) {
      sprintf(err, "%s: couldn't allocate %d side buffer for face %d\n", 
              me, face->sideNum, faceIdx);
      biffAdd(LIMN, err); return 1;
    }
    memcpy(buff, face->vertIdx, face->sideNum*sizeof(int));
    for (sii=0; sii<face->sideNum; sii++) {
      face->vertIdx[sii] = buff[face->sideNum-1-sii];
    }
    memcpy(buff, face->edgeIdx, face->sideNum*sizeof(int));
    for (sii=0; sii<face->sideNum; sii++) {
      face->edgeIdx[sii] = buff[face->sideNum-1-sii];
    }
    free(buff);
  }
  return 0;
}
