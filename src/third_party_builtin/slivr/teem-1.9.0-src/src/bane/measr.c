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

#include "bane.h"
#include "privateBane.h"

double
_baneMeasr_StockAnswer(baneMeasr *measr, gage_t *san, double *parm) {

  AIR_UNUSED(parm);
  return san[measr->offset0];
}

baneMeasr *
baneMeasrNew(int type, double *parm) {
  char me[]="baneMeasrNew", err[BIFF_STRLEN];
  baneMeasr *measr;
  int item;

  AIR_UNUSED(parm);
  if (!( AIR_IN_OP(baneMeasrUnknown, type, baneMeasrLast) )) {
    sprintf(err, "%s: baneMeasr %d invalid", me, type);
    biffAdd(BANE, err); return NULL;
  }
  /* for now, parm is ignored ... */
  measr = (baneMeasr*)calloc(1, sizeof(baneMeasr));
  if (!measr) {
    sprintf(err, "%s: couldn't allocate baneMeasr!", me);
    biffAdd(BANE, err); return NULL;
  }
  measr->type = type;
  measr->range = NULL;
  GAGE_QUERY_RESET(measr->query);
  switch(type) {
    /* --------------------------------------------------------------- */
  case baneMeasrValuePositive:
    item = gageSclValue;
    sprintf(measr->name, "%s, positive", airEnumStr(gageScl, item));
    GAGE_QUERY_ITEM_ON(measr->query, item);
    measr->range = baneRangeNew(baneRangePositive);
    measr->offset0 = gageKindAnswerOffset(gageKindScl, item);
    measr->answer = _baneMeasr_StockAnswer;
    break;
    /* --------------------------------------------------------------- */
  case baneMeasrValueZeroCentered:
    item = gageSclValue;
    sprintf(measr->name, "%s, zero-centered", airEnumStr(gageScl, item));
    GAGE_QUERY_ITEM_ON(measr->query, item);
    measr->range = baneRangeNew(baneRangeZeroCentered);
    measr->offset0 = gageKindAnswerOffset(gageKindScl, item);
    measr->answer = _baneMeasr_StockAnswer;
    break;
    /* --------------------------------------------------------------- */
  case baneMeasrValueAnywhere:
    item = gageSclValue;
    sprintf(measr->name, "%s, anywhere", airEnumStr(gageScl, item));
    GAGE_QUERY_ITEM_ON(measr->query, item);
    measr->range = baneRangeNew(baneRangeAnywhere);
    measr->offset0 = gageKindAnswerOffset(gageKindScl, item);
    measr->answer = _baneMeasr_StockAnswer;
    break;
    /* --------------------------------------------------------------- */
  case baneMeasrGradMag:
    item = gageSclGradMag;
    sprintf(measr->name, "%s", airEnumStr(gageScl, item));
    GAGE_QUERY_ITEM_ON(measr->query, item);
    measr->range = baneRangeNew(baneRangePositive);
    measr->offset0 = gageKindAnswerOffset(gageKindScl, item);
    measr->answer = _baneMeasr_StockAnswer;
    break;
    /* --------------------------------------------------------------- */
  case baneMeasrLaplacian:
    item = gageSclLaplacian;
    sprintf(measr->name, "%s", airEnumStr(gageScl, item));
    GAGE_QUERY_ITEM_ON(measr->query, item);
    measr->range = baneRangeNew(baneRangeZeroCentered);
    measr->offset0 = gageKindAnswerOffset(gageKindScl, item);
    measr->answer = _baneMeasr_StockAnswer;
    break;
    /* --------------------------------------------------------------- */
  case baneMeasr2ndDD:
    item = gageScl2ndDD;
    sprintf(measr->name, "%s", airEnumStr(gageScl, item));
    GAGE_QUERY_ITEM_ON(measr->query, item);
    measr->range = baneRangeNew(baneRangeZeroCentered);
    measr->offset0 = gageKindAnswerOffset(gageKindScl, item);
    measr->answer = _baneMeasr_StockAnswer;
    break;
    /* --------------------------------------------------------------- */
  case baneMeasrTotalCurv:
    item = gageSclTotalCurv;
    sprintf(measr->name, "%s", airEnumStr(gageScl, item));
    GAGE_QUERY_ITEM_ON(measr->query, item);
    measr->range = baneRangeNew(baneRangePositive);
    measr->offset0 = gageKindAnswerOffset(gageKindScl, item);
    measr->answer = _baneMeasr_StockAnswer;
    break;
    /* --------------------------------------------------------------- */
  case baneMeasrFlowlineCurv:
    item = gageSclFlowlineCurv;
    sprintf(measr->name, "%s", airEnumStr(gageScl, item));
    GAGE_QUERY_ITEM_ON(measr->query, item);
    measr->range = baneRangeNew(baneRangePositive);
    measr->offset0 = gageKindAnswerOffset(gageKindScl, item);
    measr->answer = _baneMeasr_StockAnswer;
    break;
    /* --------------------------------------------------------------- */
  default:    
    sprintf(err, "%s: Sorry, baneMeasr %d not implemented", me, type);
    biffAdd(BANE, err); baneMeasrNix(measr); return NULL;
  }
  return measr;
}

double
baneMeasrAnswer(baneMeasr *measr, gageContext *gctx) {
  char me[]="baneMeasrAnswer";
  double ret;
  
  if (measr && gctx && 1 == gctx->pvlNum) {
    ret = measr->answer(measr, gctx->pvl[0]->answer, measr->parm);
  } else {
    fprintf(stderr, "%s: something is terribly wrong\n", me);
    ret = AIR_NAN;
  }
  return ret;
}

baneMeasr *
baneMeasrCopy(baneMeasr *measr) {
  char me[]="baneMeasrCopy", err[BIFF_STRLEN];
  baneMeasr *ret = NULL;
  
  ret = baneMeasrNew(measr->type, measr->parm);
  if (!ret) {
    sprintf(err, "%s: couldn't make new measr", me);
    biffAdd(BANE, err); return NULL;
  }
  return ret;
}

baneMeasr *
baneMeasrNix(baneMeasr *measr) {
  
  if (measr) {
    baneRangeNix(measr->range);
    airFree(measr);
  }
  return NULL;
}
