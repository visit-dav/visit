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
limnObjectDescribe(FILE *file, limnObject *obj) {
  limnFace *face; unsigned int si, fii;
  limnEdge *edge; unsigned int eii;
  limnVertex *vert; unsigned int vii;
  limnPart *part; unsigned int partIdx;
  limnLook *look;
  
  fprintf(file, "parts: %d\n", obj->partNum);
  for (partIdx=0; partIdx<obj->partNum; partIdx++) {
    part = obj->part[partIdx];
    fprintf(file, "part %d | verts: %d ========\n", partIdx, part->vertIdxNum);
    for (vii=0; vii<part->vertIdxNum; vii++) {
      vert = obj->vert + part->vertIdx[vii];
      fprintf(file, "part %d | %d(%d): w=(%g,%g,%g)\n", 
              partIdx, vii, part->vertIdx[vii], 
              vert->world[0], vert->world[1], vert->world[2]);
      /* vert->view[0], vert->view[1], vert->view[2]); */
      /* vert->screen[0], vert->screen[1], vert->screen[2]); */
    }
    fprintf(file, "part %d | edges: %d ========\n", partIdx, part->edgeIdxNum);
    for (eii=0; eii<part->edgeIdxNum; eii++) {
      edge = obj->edge + part->edgeIdx[eii];
      fprintf(file, "part %d==%d | %d(%d): "
              "vert(%d,%d), face(%d,%d)\n", 
              partIdx, edge->partIdx, eii, part->edgeIdx[eii],
              edge->vertIdx[0], edge->vertIdx[1],
              edge->faceIdx[0], edge->faceIdx[1]);
    }
    fprintf(file, "part %d | faces: %d ========\n", partIdx, part->faceIdxNum);
    for (fii=0; fii<part->faceIdxNum; fii++) {
      face = obj->face + part->faceIdx[fii];
      fprintf(file, "part %d==%d | %d(%d): [", 
              partIdx, face->partIdx, fii, part->faceIdx[fii]);
      for (si=0; si<face->sideNum; si++) {
        fprintf(file, "%d", face->vertIdx[si]);
        if (si < face->sideNum-1) {
          fprintf(file, ",");
        }
      }
      fprintf(file, "]; wn = (%g,%g,%g)", face->worldNormal[0],
              face->worldNormal[1], face->worldNormal[2]);
      look = obj->look + face->lookIdx;
      fprintf(file, "; RGB=(%g,%g,%g)", 
              look->rgba[0], look->rgba[1], look->rgba[2]);
      fprintf(file, "\n");
    }
  }

  return 0;
}

int
limnObjectOFFWrite(FILE *file, limnObject *obj) {
  char me[]="limnObjectOFFWrite", err[BIFF_STRLEN];
  unsigned int si;
  limnVertex *vert; unsigned int vii;
  limnFace *face; unsigned int fii;
  limnPart *part; unsigned int partIdx;
  
  if (!( obj && file )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  fprintf(file, "OFF # created by Teem/limn\n");
  fprintf(file, "%d %d %d\n", obj->vertNum, obj->faceNum, obj->edgeNum);

  /* write vertices */
  for (partIdx=0; partIdx<obj->partNum; partIdx++) {
    fprintf(file, "### LIMN BEGIN PART %d\n", partIdx);
    part = obj->part[partIdx];
    for (vii=0; vii<part->vertIdxNum; vii++) {
      vert = obj->vert + part->vertIdx[vii];
      fprintf(file, "%g %g %g",
              vert->world[0]/vert->world[3],
              vert->world[1]/vert->world[3],
              vert->world[2]/vert->world[3]);
      /* verts no longer have a lookIdx
      if (vert->lookIdx) {
        fprintf(file, " %g %g %g",
                obj->look[vert->lookIdx].rgba[0],
                obj->look[vert->lookIdx].rgba[1],
                obj->look[vert->lookIdx].rgba[2]);
      }
      */
      fprintf(file, "\n");
    }
  }

  /* write faces */
  for (partIdx=0; partIdx<obj->partNum; partIdx++) {
    fprintf(file, "### LIMN BEGIN PART %d\n", partIdx);
    part = obj->part[partIdx];
    for (fii=0; fii<part->faceIdxNum; fii++) {
      face = obj->face + part->faceIdx[fii];
      fprintf(file, "%d", face->sideNum);
      for (si=0; si<face->sideNum; si++) {
        fprintf(file, " %d", face->vertIdx[si]);
      }
      if (face->lookIdx) {
        fprintf(file, " %g %g %g",
                obj->look[face->lookIdx].rgba[0],
                obj->look[face->lookIdx].rgba[1],
                obj->look[face->lookIdx].rgba[2]);
      }
      fprintf(file, "\n");
    }
  }

  return 0;
}

typedef union {
  int **i;
  void **v;
} _ippu;

int
limnObjectOFFRead(limnObject *obj, FILE *file) {
  char me[]="limnObjectOFFRead", err[BIFF_STRLEN];
  double vert[6];
  char line[AIR_STRLEN_LARGE];  /* HEY: bad Gordon */
  int lineCount, lookIdx, partIdx, idxTmp, faceNum, faceGot, got;
  unsigned int vertGot,vertNum;
  unsigned int ibuff[1024]; /* HEY: bad Gordon */
  float fbuff[1024];  /* HEY: bad bad Gordon */
  float lastRGB[3]={-1,-1,-1}; int lastLook;
  unsigned int lret;
  
  int *vertBase;
  airArray *vertBaseArr, *mop;
  _ippu u;

  if (!( obj && file )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  vertBase = NULL;
  u.i = &vertBase;
  vertBaseArr = airArrayNew(u.v, NULL, sizeof(int), 128);
  mop = airMopNew();
  airMopAdd(mop, vertBaseArr, (airMopper)airArrayNuke, airMopAlways);
  got = 0;
  lineCount = 0;
  do {
    if (!airOneLine(file, line, AIR_STRLEN_LARGE)) {
      sprintf(err, "%s: hit EOF before getting #vert #face #edge line", me);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
    lineCount++;
    got = airParseStrUI(ibuff, line, AIR_WHITESPACE, 3);
  } while (3 != got);
  vertNum = ibuff[0];
  faceNum = ibuff[1];
  
  /* read all vertex information */
  lastLook = -1;
  partIdx = limnObjectPartAdd(obj);
  vertGot = 0;
  airArrayLenIncr(vertBaseArr, 1);
  vertBase[partIdx] = vertGot;
  while (vertGot < vertNum) {
    do {
      lret = airOneLine(file, line, AIR_STRLEN_LARGE);
      lineCount++;
    } while (1 == lret);
    if (!lret) {
      sprintf(err, "%s: (near line %d) hit EOF trying to read vert %d (of %d)",
              me, lineCount, vertGot, vertNum);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
    if (1 == sscanf(line, "### LIMN BEGIN PART %d", &idxTmp)) {
      if (idxTmp != 0) {
        partIdx = limnObjectPartAdd(obj);
        if (idxTmp != partIdx) {
          sprintf(err, "%s: got signal to start part %d, not %d", 
                  me, idxTmp, partIdx);
          biffAdd(LIMN, err); airMopError(mop); return 1;
        }
        airArrayLenIncr(vertBaseArr, 1);
        vertBase[partIdx] = vertGot;
      }
      continue;
    }
    if (3 != airParseStrD(vert, line, AIR_WHITESPACE, 3)) {
      sprintf(err, "%s: couldn't parse 3 doubles from \"%s\" "
              "for vert %d (of %d)",
              me, line, vertGot, vertNum);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
    if (6 == airParseStrD(vert, line, AIR_WHITESPACE, 6)) {
      /* we could also parse an RGB color */
      if (-1 == lastLook || !ELL_3V_EQUAL(lastRGB, vert+3)) {
        lookIdx = limnObjectLookAdd(obj);
        ELL_4V_SET(obj->look[lookIdx].rgba,
                   AIR_CAST(float, vert[3]),
                   AIR_CAST(float, vert[4]),
                   AIR_CAST(float, vert[5]),
                   1);
        lastLook = lookIdx;
        ELL_3V_COPY_TT(lastRGB, float, vert+3);
      } else {
        lookIdx = lastLook;
      }
    } else {
      lookIdx = 0;
    }
    /*
    fprintf(stderr, "line %d: vertGot = %d; lookIdx = %d; partIdx = %d\n",
            lineCount, vertGot, lookIdx, partIdx);
    */
    limnObjectVertexAdd(obj, partIdx,
                        AIR_CAST(float, vert[0]),
                        AIR_CAST(float, vert[1]),
                        AIR_CAST(float, vert[2]));
    vertGot++;
  }
  /* read face information */
  partIdx = 0;
  faceGot = 0;
  while (faceGot < faceNum) {
    do {
      lret = airOneLine(file, line, AIR_STRLEN_LARGE);
      lineCount++;
    } while (1 == lret);
    if (!lret) {
      sprintf(err, "%s: (near line %d) hit EOF trying to read face %d (of %d)",
              me, lineCount, faceGot, faceNum);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
    if (1 == sscanf(line, "### LIMN BEGIN PART %d", &idxTmp)) {
      if (idxTmp != 0) {
        partIdx += 1;
        if (idxTmp != partIdx) {
          sprintf(err, "%s: (near line %d) got signal to start "
                  "part %d, not %d", 
                  me, lineCount, idxTmp, partIdx);
          biffAdd(LIMN, err); airMopError(mop); return 1;
        }
      }
      continue;
    }
    if (1 != sscanf(line, "%u", &vertNum)) {
      sprintf(err, "%s: (near line %d) can't get first int "
              "(#verts) from \"%s\" for face %d (of %d)",
              me, lineCount, line, faceGot, faceNum);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
    if (vertNum+1 != airParseStrUI(ibuff, line, AIR_WHITESPACE, vertNum+1)) {
      sprintf(err, "%s: (near line %d) couldn't parse %d ints from \"%s\" "
              "for face %d (of %d)",
              me, lineCount, vertNum+1, line, faceGot, faceNum);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
    if (vertNum+1+3 == airParseStrF(fbuff, line,
                                    AIR_WHITESPACE, vertNum+1+3)) {
      /* could also parse color */
      if (-1 == lastLook || !ELL_3V_EQUAL(lastRGB, fbuff+vertNum+1)) {
        lookIdx = limnObjectLookAdd(obj);
        ELL_4V_SET(obj->look[lookIdx].rgba, fbuff[vertNum+1+0],
                   fbuff[vertNum+1+1], fbuff[vertNum+1+2], 1);
        lastLook = lookIdx;
        ELL_3V_COPY(lastRGB, fbuff+vertNum+1);
      } else {
        lookIdx = lastLook;
      }
    } else {
      lookIdx = 0;
    }
    /*
    fprintf(stderr, "line %d: faceGot = %d; lookIdx = %d; partIdx = %d\n",
            lineCount, faceGot, lookIdx, partIdx);
    */
    limnObjectFaceAdd(obj, partIdx, lookIdx, vertNum, ibuff+1);
    faceGot++;
  }

  airMopOkay(mop); 
  return 0;
}
