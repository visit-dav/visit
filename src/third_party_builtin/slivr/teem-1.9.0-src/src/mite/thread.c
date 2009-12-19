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

#include "mite.h"
#include "privateMite.h"

miteThread *
miteThreadNew() {
  char me[]="miteThreadNew", err[BIFF_STRLEN];
  miteThread *mtt;
  int ii;
  
  mtt = (miteThread *)calloc(1, sizeof(miteThread));
  if (!mtt) {
    sprintf(err, "%s: couldn't calloc miteThread", me);
    biffAdd(MITE, err); return NULL;
  }

  mtt->gctx = NULL;
  mtt->ansScl = mtt->ansVec = mtt->ansTen = NULL;
  mtt->_normal = NULL;
  mtt->shadeVec0 = NULL;
  mtt->shadeVec1 = NULL;
  mtt->shadeScl0 = NULL;
  mtt->shadeScl1 = NULL;
  /* were miteVal a full-fledged gageKind, the following would
     be done by gagePerVolumeNew */
  mtt->ansMiteVal = 
    (gage_t *)calloc(gageKindTotalAnswerLength(miteValGageKind), 
                     sizeof(gage_t));
  mtt->directAnsMiteVal = 
    (gage_t **)calloc(miteValGageKind->itemMax+1, sizeof(gage_t*));
  if (!(mtt->ansMiteVal && mtt->directAnsMiteVal)) {
    sprintf(err, "%s: couldn't calloc miteVal answer arrays", me);
    biffAdd(MITE, err); return NULL;
  }
  for (ii=0; ii<=miteValGageKind->itemMax; ii++) {
    mtt->directAnsMiteVal[ii] = mtt->ansMiteVal 
      + gageKindAnswerOffset(miteValGageKind, ii);
  }
  mtt->verbose = 0;
  mtt->skip = 0;
  mtt->thrid = -1;
  mtt->ui = mtt->vi = -1;
  mtt->raySample = 0;
  mtt->samples = 0;
  mtt->stage = NULL;
  /* mtt->range[], rayStep, V, RR, GG, BB, TT  initialized in 
     miteRayBegin or in miteSample */
  
  return mtt;
}

miteThread *
miteThreadNix(miteThread *mtt) {

  mtt->ansMiteVal = (gage_t *)airFree(mtt->ansMiteVal);
  mtt->directAnsMiteVal = (gage_t **)airFree(mtt->directAnsMiteVal);
  airFree(mtt);
  return NULL;
}

/*
******** miteThreadBegin()
**
** this has some of the body of what would be miteThreadInit
*/
int 
miteThreadBegin(miteThread **mttP, miteRender *mrr,
                miteUser *muu, int whichThread) {
  char me[]="miteThreadBegin", err[BIFF_STRLEN];

  /* all the miteThreads have already been allocated */
  (*mttP) = mrr->tt[whichThread];

  if (!whichThread) {
    /* this is the first thread- it just points to the parent gageContext */
    (*mttP)->gctx = muu->gctx0;
  } else {
    /* we have to generate a new gageContext */
    (*mttP)->gctx = gageContextCopy(muu->gctx0);
    if (!(*mttP)->gctx) {
      sprintf(err, "%s: couldn't set up thread %d", me, whichThread);
      biffMove(MITE, err, GAGE); return 1;
    }
  }

  if (-1 != mrr->sclPvlIdx) {
    (*mttP)->ansScl = (*mttP)->gctx->pvl[mrr->sclPvlIdx]->answer;
    (*mttP)->nPerp = ((*mttP)->ansScl 
                      + gageKindAnswerOffset(gageKindScl, gageSclNPerp));
    (*mttP)->geomTens = ((*mttP)->ansScl
                         + gageKindAnswerOffset(gageKindScl, gageSclGeomTens));
  } else {
    (*mttP)->ansScl = NULL;
    (*mttP)->nPerp = NULL;
    (*mttP)->geomTens = NULL;
  }
  (*mttP)->ansVec = (-1 != mrr->vecPvlIdx
                     ? (*mttP)->gctx->pvl[mrr->vecPvlIdx]->answer
                     : NULL);
  (*mttP)->ansTen = (-1 != mrr->tenPvlIdx
                     ? (*mttP)->gctx->pvl[mrr->tenPvlIdx]->answer
                     : NULL);
  (*mttP)->thrid = whichThread;
  (*mttP)->raySample = 0;
  (*mttP)->samples = 0;
  (*mttP)->verbose = 0;
  (*mttP)->skip = 0;
  (*mttP)->_normal = _miteAnswerPointer(*mttP, mrr->normalSpec);
  
  /* set up shading answers */
  switch(mrr->shadeSpec->method) {
  case miteShadeMethodNone:
    /* nothing to do */
    break;
  case miteShadeMethodPhong:
    (*mttP)->shadeVec0 = _miteAnswerPointer(*mttP, mrr->shadeSpec->vec0);
    break;
  case miteShadeMethodLitTen:
    (*mttP)->shadeVec0 = _miteAnswerPointer(*mttP, mrr->shadeSpec->vec0);
    (*mttP)->shadeVec1 = _miteAnswerPointer(*mttP, mrr->shadeSpec->vec1);
    (*mttP)->shadeScl0 = _miteAnswerPointer(*mttP, mrr->shadeSpec->scl0);
    (*mttP)->shadeScl1 = _miteAnswerPointer(*mttP, mrr->shadeSpec->scl1);
    break;
  default:
    sprintf(err, "%s: shade method %d not implemented!",
            me, mrr->shadeSpec->method);
    biffAdd(MITE, err); return 1;
    break;
  }

  if (_miteStageSet(*mttP, mrr)) {
    sprintf(err, "%s: trouble setting up stage array", me);
    biffAdd(MITE, err); return 1;
  }
  return 0;
}

int 
miteThreadEnd(miteThread *mtt, miteRender *mrr,
              miteUser *muu) {
  
  AIR_UNUSED(mtt);
  AIR_UNUSED(mrr);
  AIR_UNUSED(muu);
  return 0;
}

