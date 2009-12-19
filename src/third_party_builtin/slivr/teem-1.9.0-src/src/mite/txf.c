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

/* learned: don't confuse allocate an array of structs with an array
   of pointers to structs.  Don't be surprised when you bus error
   because of the difference 
*/

#include "mite.h"
#include "privateMite.h"

char
miteRangeChar[MITE_RANGE_NUM+1] = "ARGBEadsp";

char
_miteStageOpStr[][AIR_STRLEN_SMALL] = {
  "(unknown miteStageOp)",
  "min",
  "max",
  "add",
  "multiply"
};

int
_miteStageOpVal[] = {
  miteStageOpUnknown,
  miteStageOpMin,
  miteStageOpMax,
  miteStageOpAdd,
  miteStageOpMultiply
};

char
_miteStageOpStrEqv[][AIR_STRLEN_SMALL] = {
  "min",
  "max",
  "add", "+",
  "multiply", "*", "x"
};

int
_miteStageOpValEqv[] = {
  miteStageOpUnknown,
  miteStageOpMin,
  miteStageOpMax,
  miteStageOpAdd, miteStageOpAdd,
  miteStageOpMultiply, miteStageOpMultiply, miteStageOpMultiply
};

airEnum
_miteStageOp = {
  "miteStageOp",
  MITE_STAGE_OP_MAX+1,
  _miteStageOpStr, _miteStageOpVal,
  NULL,
  _miteStageOpStrEqv, _miteStageOpValEqv,
  AIR_FALSE
};
airEnum *
miteStageOp = &_miteStageOp;

/*
******** miteVariableParse()
**
** takes a string (usually the label from a nrrd axis) and parses it
** to determine the gageItemSpec from it (which means finding the
** kind and item).  The valid formats are:
**
**   ""                  : NULL kind, -1 item
**   <item>              : miteValGageKind (DEPRECATED)
**   mite(<item>)        : miteValGageKind
**   gage(<item>)        : gageKindScl (DEPRECATED)
**   gage(scalar:<item>) : gageKindScl
**   gage(vector:<item>) : gageKindVec
**   gage(tensor:<item>) : tenGageKind
**
** Notice that "scalar", "vector", and "tensor" to NOT refer to the type 
** of the quantity being measured, but rather to the type of volume in 
** which quantity is measured (i.e., the gageKind used)
*/
int
miteVariableParse(gageItemSpec *isp, const char *label) {
  char me[]="miteVariableParse", err[BIFF_STRLEN], *buff, *endparen,
    *kqstr, *col, *kstr, *qstr;
  airArray *mop;
  
  if (!( isp && label )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(MITE, err); return 1;
  }
  if (0 == strlen(label)) {
    /* nothing was specified; we try to indicate that by mimicking 
       the return of gageItemSpecNew() */
    isp->item = -1;
    isp->kind = NULL;
    return 0;
  }
  /* else given string was non-empty */
  mop = airMopNew();
  buff = airStrdup(label);
  if (!buff) {
    sprintf(err, "%s: couldn't strdup label!", me);
    biffAdd(MITE, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, buff, airFree, airMopAlways);
  if (strstr(buff, "gage(") == buff) {
    /* txf domain variable is to be measured directly by gage */
    if (!(endparen = strstr(buff, ")"))) {
      sprintf(err, "%s: didn't see close paren after \"gage(\"", me);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    *endparen = 0;
    kqstr = buff + strlen("gage(");
    /* first see if its a (deprecated) gageKindScl specification */
    isp->item = airEnumVal(gageScl, kqstr);
    if (-1 != isp->item) {
      isp->kind = gageKindScl;
      fprintf(stderr, "\n%s: WARNING: deprecated use of txf domain "
              "\"gage(%s)\" without explicit gage kind specification; "
              "should use \"gage(%s:%s)\" instead\n\n",
              me, kqstr, gageKindScl->name, kqstr);
    } else {
      /* should be of form "<kind>:<item>" */
      col = strstr(kqstr, ":");
      if (!col) {
        sprintf(err, "%s: didn't see \":\" seperator between gage "
                "kind and item", me);
        biffAdd(MITE, err); airMopError(mop); return 1;
      }
      *col = 0;
      kstr = kqstr;
      qstr = col+1;
      if (!strcmp(gageKindScl->name, kstr)) {
        isp->kind = gageKindScl;
      } else if (!strcmp(gageKindVec->name, kstr)) {
        isp->kind = gageKindVec;
      } else if (!strcmp(tenGageKind->name, kstr)) {
        isp->kind = tenGageKind;
      } else {
        sprintf(err, "%s: don't recognized \"%s\" gage kind", me, kstr);
        biffAdd(MITE, err); airMopError(mop); return 1;
      }
      isp->item = airEnumVal(isp->kind->enm, qstr);
      if (-1 == isp->item) {
        sprintf(err, "%s: couldn't parse \"%s\" as a %s variable",
                me, qstr, isp->kind->name);
        biffAdd(MITE, err); airMopError(mop); return 1;
      }
    }
  } else if (strstr(buff, "mite(") == buff) {
    /* txf domain variable is *not* directly measured by gage */
    if (!(endparen = strstr(buff, ")"))) {
      sprintf(err, "%s: didn't see close paren after \"mite(\"", me);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    *endparen = 0;
    qstr = buff + strlen("mite(");
    isp->item = airEnumVal(miteVal, qstr);
    if (-1 == isp->item) {
      sprintf(err, "%s: couldn't parse \"%s\" as a miteVal variable",
              me, qstr);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    isp->kind = miteValGageKind;
  } else {
    /* didn't start with "gage(" or "mite(" */
    isp->item = airEnumVal(miteVal, label);
    if (-1 != isp->item) {
      /* its measured by mite */
      isp->kind = miteValGageKind;
      fprintf(stderr, "\n%s: WARNING: deprecated use of txf domain "
              "\"%s\"; should use \"mite(%s)\" instead\n\n",
              me, label, label);
    } else {
      sprintf(err, "%s: \"%s\" not a recognized variable", me, label);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
  }
  airMopOkay(mop);
  return 0;
}

void
miteVariablePrint(char *buff, const gageItemSpec *isp) {
  char me[]="miteVariablePrint";

  if (!(isp->kind)) {
    strcpy(buff, "");
  } else if (gageKindScl == isp->kind
             || gageKindVec == isp->kind
             || tenGageKind == isp->kind) {
    sprintf(buff, "gage(%s:%s)", isp->kind->name, 
            airEnumStr(isp->kind->enm, isp->item));
  } else if (miteValGageKind == isp->kind) {
    sprintf(buff, "%s(%s)", isp->kind->name, 
            airEnumStr(isp->kind->enm, isp->item));
  } else {
    sprintf(buff, "(%s: unknown gageKind!)", me);
  }
  return;
}

int
miteNtxfCheck(const Nrrd *ntxf) {
  char me[]="miteNtxfCheck", err[BIFF_STRLEN], *rangeStr, *domStr;
  gageItemSpec isp;
  int log2;
  unsigned int rii, axi;

  if (nrrdCheck(ntxf)) {
    sprintf(err, "%s: basic nrrd validity check failed", me);
    biffMove(MITE, err, NRRD); return 1;
  }
  if (!( nrrdTypeFloat == ntxf->type || 
         nrrdTypeDouble == ntxf->type || 
         nrrdTypeUChar == ntxf->type )) {
    sprintf(err, "%s: need a type %s, %s or %s nrrd (not %s)", me,
            airEnumStr(nrrdType, nrrdTypeFloat),
            airEnumStr(nrrdType, nrrdTypeDouble),
            airEnumStr(nrrdType, nrrdTypeUChar),
            airEnumStr(nrrdType, ntxf->type));
    biffAdd(MITE, err); return 1;
  }
  if (!( 2 <= ntxf->dim )) {
    sprintf(err, "%s: nrrd dim (%d) isn't at least 2 (for a 1-D txf)",
            me, ntxf->dim);
    biffAdd(MITE, err); return 1;
  }
  rangeStr = ntxf->axis[0].label;
  if (0 == airStrlen(rangeStr)) {
    sprintf(err, "%s: axis[0]'s label doesn't specify txf range", me);
    biffAdd(MITE, err); return 1;
  }
  if (airStrlen(rangeStr) != ntxf->axis[0].size) {
    sprintf(err, "%s: axis[0]'s size is " _AIR_SIZE_T_CNV 
            ", but label specifies " _AIR_SIZE_T_CNV " values",
            me, ntxf->axis[0].size, airStrlen(rangeStr));
    biffAdd(MITE, err); return 1;
  }
  for (rii=0; rii<airStrlen(rangeStr); rii++) {
    if (!strchr(miteRangeChar, rangeStr[rii])) {
      sprintf(err, "%s: char %d of axis[0]'s label (\"%c\") isn't a valid "
              "transfer function range specifier (not in \"%s\")",
              me, rii, rangeStr[rii], miteRangeChar);
      biffAdd(MITE, err); return 1;
    }
  }
  for (axi=1; axi<ntxf->dim; axi++) {
    if (1 == ntxf->axis[axi].size) {
      sprintf(err, "%s: # samples on axis %d must be > 1", me, axi);
      biffAdd(MITE, err); return 1;
    }
    domStr = ntxf->axis[axi].label;
    if (0 == airStrlen(domStr)) {
      sprintf(err, "%s: axis[%d] of txf didn't specify a domain variable",
              me, axi);
      biffAdd(MITE, err); return 1;
    }
    if (miteVariableParse(&isp, domStr)) {
      sprintf(err, "%s: couldn't parse txf domain \"%s\" for axis %d\n", 
              me, domStr, axi);
      biffAdd(MITE, err); return 1;
    }
    if (!( 1 == isp.kind->table[isp.item].answerLength ||
           3 == isp.kind->table[isp.item].answerLength )) {
      sprintf(err, "%s: %s not a scalar or vector (answerLength = %d): "
              "can't be a txf domain variable", me, domStr,
              isp.kind->table[isp.item].answerLength);
      biffAdd(MITE, err); return 1;
    }
    if (3 == isp.kind->table[isp.item].answerLength) {
      /* has to be right length for one of the quantization schemes */
      log2 = airLog2(ntxf->axis[axi].size);
      if (-1 == log2) {
        sprintf(err, "%s: txf axis size for %s must be power of 2 (not "
                _AIR_SIZE_T_CNV ")",
                me, domStr, ntxf->axis[axi].size);
        biffAdd(MITE, err); return 1;
      } else {
        if (!( AIR_IN_CL(8, log2, 16) )) {
          sprintf(err, "%s: log_2 of txf axis size for %s should be in "
                  "range [8,16] (not %d)", me, domStr, log2);
          biffAdd(MITE, err); return 1;
        }
      }
    } else {
      if (!( AIR_EXISTS(ntxf->axis[axi].min) && 
             AIR_EXISTS(ntxf->axis[axi].max) )) {
        sprintf(err, "%s: min and max of axis %d aren't both set", me, axi);
        biffAdd(MITE, err); return 1;
      }
      if (!( ntxf->axis[axi].min < ntxf->axis[axi].max )) {
        sprintf(err, "%s: min (%g) not less than max (%g) on axis %d", 
                me, ntxf->axis[axi].min, ntxf->axis[axi].max, axi);
        biffAdd(MITE, err); return 1;
      }
    }
  }
  
  return 0;
}

/*
******** miteQueryAdd()
**
** This looks a given gageItemSpec and sets the bits in the
** gageKindScl and tenGageKind queries that are required to calculate
** the quantity
**
** NOTE: This does NOT initialize the query{Scl,Vec,Ten}: it
** just adds on new items to the existing queries
**
** HEY: this is really unfortunate: each new gage type use for
** volume rendering in mite will have to explicitly added as
** arguments here, which is part of the reason that mite may end
** up explicitly depending on the libraries supplying those gageKinds
** (like how mite now must depend on ten)
**
** The queryMite argument is a little odd- its not a real gage kind,
** but we use it to organize which of the miteVal quantities we take
** the time to compute in miteSample().
*/
void
miteQueryAdd(gageQuery queryScl, gageQuery queryVec, 
             gageQuery queryTen, gageQuery queryMite,
             gageItemSpec *isp) {
  char me[]="miteQueryAdd";
  
  if (NULL == isp->kind) {
    /* nothing to add */
  } else if (gageKindScl == isp->kind) {
    GAGE_QUERY_ITEM_ON(queryScl, isp->item);
  } else if (gageKindVec == isp->kind) {
    GAGE_QUERY_ITEM_ON(queryVec, isp->item);
  } else if (tenGageKind == isp->kind) {
    GAGE_QUERY_ITEM_ON(queryTen, isp->item);
  } else if (miteValGageKind == isp->kind) {
    /* regardless of whether the mite query requires scl, vec, or ten
       queries, we add it to the quantites that have to be computed
       per-sample */
    GAGE_QUERY_ITEM_ON(queryMite, isp->item);
    /* HEY: some these have useful analogs for tensor data, but I
       won't be able to express them.  This means that while Phong
       shading of *scalar* volumes can be implemented with transfer
       functions, it is currently not possible in *tensor* volumes
       (for instance, using the gradient of fractional anisotropy) */
    switch(isp->item) {
    case miteValVrefN:
    case miteValNdotV: 
    case miteValNdotL:
      /* the "N" can be a normalized vector from any of the
         available kinds (except miteValGageKind!), and its
         associated query will be handled elsewhere, so there's
         nothing to add here */
      break;
    case miteValGTdotV:
      GAGE_QUERY_ITEM_ON(queryScl, gageSclGeomTens);
      break;
    case miteValVdefT:
      GAGE_QUERY_ITEM_ON(queryTen, tenGageTensor);
    case miteValVdefTdotV:
      GAGE_QUERY_ITEM_ON(queryTen, tenGageTensor);
      break;
    }
  } else {
    fprintf(stderr, "%s: PANIC: unrecognized non-NULL gageKind\n", me);
    exit(1);
  }
  return;
}

int
_miteNtxfCopy(miteRender *mrr, miteUser *muu) {
  char me[]="_miteNtxfCopy", err[BIFF_STRLEN];
  int ni, E;
  
  mrr->ntxf = (Nrrd **)calloc(muu->ntxfNum, sizeof(Nrrd *));
  if (!mrr->ntxf) {
    sprintf(err, "%s: couldn't calloc %d ntxf pointers", me, muu->ntxfNum);
    biffAdd(MITE, err); return 1;
  }
  mrr->ntxfNum = muu->ntxfNum;
  airMopAdd(mrr->rmop, mrr->ntxf, airFree, airMopAlways);
  E = 0;
  for (ni=0; ni<mrr->ntxfNum; ni++) {
    mrr->ntxf[ni] = nrrdNew();
    if (!E) airMopAdd(mrr->rmop, mrr->ntxf[ni],
                      (airMopper)nrrdNuke, airMopAlways);
    if (!( nrrdTypeUChar == muu->ntxf[ni]->type 
           || nrrdTypeFloat == muu->ntxf[ni]->type 
           || nrrdTypeDouble == muu->ntxf[ni]->type )) {
      sprintf(err, "%s: sorry, can't handle txf of type %s (only %s, %s, %s)",
              me, airEnumStr(nrrdType, muu->ntxf[ni]->type),
              airEnumStr(nrrdType, nrrdTypeUChar),
              airEnumStr(nrrdType, nrrdTypeFloat),
              airEnumStr(nrrdType, nrrdTypeDouble));
      biffAdd(MITE, err); return 1;
    }
    /* note that key/values need to be copied for the sake of
       identifying a non-default miteStageOp */
    switch(muu->ntxf[ni]->type) {
    case nrrdTypeUChar:
      if (!E) E |= nrrdUnquantize(mrr->ntxf[ni], muu->ntxf[ni], nrrdTypeUChar);
      if (!E) E |= nrrdKeyValueCopy(mrr->ntxf[ni], muu->ntxf[ni]);
      break;
    case mite_nt:
      if (!E) E |= nrrdCopy(mrr->ntxf[ni], muu->ntxf[ni]);
      break;
    default:  /* will be either float or double (whatever mite_nt isn't) */
      if (!E) E |= nrrdConvert(mrr->ntxf[ni], muu->ntxf[ni], mite_nt);
      if (!E) E |= nrrdKeyValueCopy(mrr->ntxf[ni], muu->ntxf[ni]);
      break;
    }
  }
  if (E) {
    sprintf(err, "%s: troubling copying/converting all ntxfs", me);
    biffMove(MITE, err, NRRD); return 1;
  }
  return 0;
}

int
_miteNtxfAlphaAdjust(miteRender *mrr, miteUser *muu) {
  char me[]="_miteNtxfAlphaAdjust", err[BIFF_STRLEN];
  int ni, ei, ri, nnum, rnum;
  Nrrd *ntxf;
  mite_t *data, alpha, frac;
  
  if (_miteNtxfCopy(mrr, muu)) {
    sprintf(err, "%s: trouble copying/converting transfer functions", me);
    biffAdd(MITE, err); return 1;
  }
  frac = muu->rayStep/muu->refStep;
  for (ni=0; ni<mrr->ntxfNum; ni++) {
    ntxf = mrr->ntxf[ni];
    if (!strchr(ntxf->axis[0].label, miteRangeChar[miteRangeAlpha])) {
      continue;
    }
    /* else this txf sets opacity */
    data = (mite_t *)ntxf->data;
    rnum = ntxf->axis[0].size;
    nnum = nrrdElementNumber(ntxf)/rnum;
    for (ei=0; ei<nnum; ei++) {
      for (ri=0; ri<rnum; ri++) {
        if (ntxf->axis[0].label[ri] == miteRangeChar[miteRangeAlpha]) {
          alpha = data[ri + rnum*ei];
          data[ri + rnum*ei] = 1 - pow(AIR_MAX(0, 1-alpha), frac);
        }
      }
    }
  }
  return 0;
}

int
_miteStageNum(miteRender *mrr) {
  int num, ni;

  num = 0;
  for (ni=0; ni<mrr->ntxfNum; ni++) {
    num += mrr->ntxf[ni]->dim - 1;
  }
  return num;
}

void
_miteStageInit(miteStage *stage) {
  int rii;

  stage->val = NULL;
  stage->size = -1;
  stage->op = miteStageOpUnknown;
  stage->qn = NULL;
  stage->min = stage->max = AIR_NAN;
  stage->data = NULL;
  for (rii=0; rii<=MITE_RANGE_NUM-1; rii++) {
    stage->rangeIdx[rii] = -1;
  }
  stage->rangeNum = -1;
  stage->label = NULL;
  return;
}

gage_t *
_miteAnswerPointer(miteThread *mtt, gageItemSpec *isp) {
  char me[]="_miteAnswerPointer";
  gage_t *ret;

  if (!isp->kind) {
    /* we got a NULL kind (as happens with output of
       gageItemSpecNew(), or miteVariableParse of an
       empty string); only NULL return is sensible */
    return NULL;
  }

  if (gageKindScl == isp->kind) {
    ret = mtt->ansScl;
  } else if (gageKindVec == isp->kind) {
    ret = mtt->ansVec;
  } else if (tenGageKind == isp->kind) {
    ret = mtt->ansTen;
  } else if (miteValGageKind == isp->kind) {
    ret = mtt->ansMiteVal;
  } else {
    fprintf(stderr, "\nPANIC: %s: unknown gageKind!\n", me);
    exit(1);
  }
  ret += gageKindAnswerOffset(isp->kind, isp->item);
  return ret;
}

/*
** _miteStageSet
**
** ALLOCATES and initializes stage array in a miteThread
*/
int
_miteStageSet(miteThread *mtt, miteRender *mrr) {
  char me[]="_miteStageSet", err[BIFF_STRLEN], *value;
  int ni, di, stageIdx, rii, stageNum, log2;
  Nrrd *ntxf;
  miteStage *stage;
  gageItemSpec isp;
  char rc;
  
  stageNum = _miteStageNum(mrr);
  /* fprintf(stderr, "!%s: stageNum = %d\n", me, stageNum); */
  mtt->stage = (miteStage *)calloc(stageNum, sizeof(miteStage));
  if (!mtt->stage) {
    sprintf(err, "%s: couldn't alloc array of %d stages", me, stageNum);
    biffAdd(MITE, err); return 1;
  }
  airMopAdd(mrr->rmop, mtt->stage, airFree, airMopAlways);
  mtt->stageNum = stageNum;
  stageIdx = 0;
  for (ni=0; ni<mrr->ntxfNum; ni++) {
    ntxf = mrr->ntxf[ni];
    for (di=ntxf->dim-1; di>=1; di--) {
      stage = mtt->stage + stageIdx;
      _miteStageInit(stage);
      miteVariableParse(&isp, ntxf->axis[di].label);
      stage->val = _miteAnswerPointer(mtt, &isp);
      stage->label = ntxf->axis[di].label;
      /*
      fprintf(stderr, "!%s: ans=%p + offset[%d]=%d == %p\n", me,
              mtt->ans, dom, kind->ansOffset[dom], stage->val);
      */
      stage->size = ntxf->axis[di].size;
      stage->min =  ntxf->axis[di].min;
      stage->max =  ntxf->axis[di].max;
      if (di > 1) {
        stage->data = NULL;
      } else {
        stage->data = (mite_t *)ntxf->data;
        value = nrrdKeyValueGet(ntxf, "miteStageOp");
        if (value) {
          stage->op = airEnumVal(miteStageOp, value);
          if (miteStageOpUnknown == stage->op) {
            stage->op = miteStageOpMultiply;
          }
        } else {
          stage->op = miteStageOpMultiply;
        }
        if (1 == isp.kind->table[isp.item].answerLength) {
          stage->qn = NULL;
        } else if (3 == isp.kind->table[isp.item].answerLength) {
          log2 = airLog2(ntxf->axis[di].size);
          switch(log2) {
          case 8:  stage->qn = limnVtoQN_GT[ limnQN8octa]; break;
          case 9:  stage->qn = limnVtoQN_GT[ limnQN9octa]; break;
          case 10: stage->qn = limnVtoQN_GT[limnQN10octa]; break;
          case 11: stage->qn = limnVtoQN_GT[limnQN11octa]; break;
          case 12: stage->qn = limnVtoQN_GT[limnQN12octa]; break;
          case 13: stage->qn = limnVtoQN_GT[limnQN13octa]; break;
          case 14: stage->qn = limnVtoQN_GT[limnQN14octa]; break;
          case 15: stage->qn = limnVtoQN_GT[limnQN15octa]; break;
          case 16: stage->qn = limnVtoQN_GT[limnQN16octa]; break;
          default:
            sprintf(err, "%s: txf axis %d size " _AIR_SIZE_T_CNV 
                    " not usable for vector txf domain variable %s", me,
                    di, ntxf->axis[di].size, ntxf->axis[di].label);
            biffAdd(MITE, err); return 1;
            break;
          }
        } else {
          sprintf(err, "%s: %s not scalar or vector (len = %d): can't be "
                  "a txf domain variable", me,
                  ntxf->axis[di].label,
                  isp.kind->table[isp.item].answerLength);
          biffAdd(MITE, err); return 1;
        }
        stage->rangeNum = ntxf->axis[0].size;
        for (rii=0; rii<stage->rangeNum; rii++) {
          rc = ntxf->axis[0].label[rii];
          stage->rangeIdx[rii] = strchr(miteRangeChar, rc) - miteRangeChar;
          /*
          fprintf(stderr, "!%s: range: %c -> %d\n", "_miteStageSet",
                  ntxf->axis[0].label[rii], stage->rangeIdx[rii]);
          */
        }
      }
      stageIdx++;
    }
  }
  return 0;
}

void
_miteStageRun(miteThread *mtt, miteUser *muu) {
  int stageIdx, ri, rii, txfIdx, finalIdx;
  miteStage *stage;
  mite_t *rangeData;
  double *dbg=NULL;

  finalIdx = 0;
  if (mtt->verbose) {
    dbg = muu->debug + muu->debugIdx;
  }
  for (stageIdx=0; stageIdx<mtt->stageNum; stageIdx++) {
    stage = &(mtt->stage[stageIdx]);
    if (stage->qn) {
      /* its a vector-valued txf domain variable */
      txfIdx = stage->qn(stage->val);
      /* right now, we can't store vector-valued txf domain variables */
    } else {
      /* its a scalar txf domain variable */
      txfIdx = airIndexClamp(stage->min, *(stage->val),
                             stage->max, stage->size);
      if (mtt->verbose) {
        dbg[0 + 2*stageIdx] = *(stage->val);
      }
    }
    finalIdx = stage->size*finalIdx + txfIdx;
    if (mtt->verbose) {
      dbg[1 + 2*stageIdx] = txfIdx;
    }
    if (stage->data) {
      rangeData = stage->data + stage->rangeNum*finalIdx;
      for (rii=0; rii<stage->rangeNum; rii++) {
        ri = stage->rangeIdx[rii];
        switch(stage->op) {
        case miteStageOpMin:
          mtt->range[ri] = AIR_MIN(mtt->range[ri], rangeData[rii]);
          break;
        case miteStageOpMax:
          mtt->range[ri] = AIR_MAX(mtt->range[ri], rangeData[rii]);
          break;
        case miteStageOpAdd:
          mtt->range[ri] += rangeData[rii];
          break;
        case miteStageOpMultiply:
        default:
          mtt->range[ri] *= rangeData[rii];
          break;
        }
      }
      finalIdx = 0;
    }
  }
  return;
}
