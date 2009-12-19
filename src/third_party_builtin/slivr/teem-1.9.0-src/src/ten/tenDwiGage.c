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

#include "ten.h"
#include "privateTen.h"

/* --------------------------------------------------------------------- */

char
_tenDwiGageStr[][AIR_STRLEN_SMALL] = {
  "(unknown tenDwiGage)",
  "all",
  "b0",
  "mdwi",
  "tlls",
  "tllserr",
  "tllserrlog",
  "twls",
  "twlserr",
  "twlserrlog",
  "tnls",
  "tnlserr",
  "tnlserrlog",
  "tmle",
  "tmleerr",
  "tmleerrlog",
  "t",
  "terr",
  "terrlog",
  "c"
};

int
_tenDwiGageVal[] = {
  tenDwiGageUnknown,
  tenDwiGageB0,
  tenDwiGageMeanDwiValue,
  tenDwiGageTensorLLS,
  tenDwiGageTensorLLSError,
  tenDwiGageTensorLLSErrorLog,
  tenDwiGageTensorWLS,
  tenDwiGageTensorWLSError,
  tenDwiGageTensorWLSErrorLog,
  tenDwiGageTensorNLS,
  tenDwiGageTensorNLSError,
  tenDwiGageTensorNLSErrorLog,
  tenDwiGageTensor,
  tenDwiGageTensorError,
  tenDwiGageTensorErrorLog,
  tenDwiGageConfidence
};

airEnum
_tenDwiGage = {
  "tenDwiGage",
  TEN_DWI_GAGE_ITEM_MAX+1,
  _tenDwiGageStr, _tenDwiGageVal,
  NULL,
  NULL, NULL,
  AIR_FALSE
};
airEnum *
tenDwiGage = &_tenDwiGage;

/* --------------------------------------------------------------------- */

typedef struct {
  unsigned int num;     /* number of total values (both baseline and DWI) */
  tenEstimateContext *tec;  /* HEY: NOT THREAD SAFE! */
} tenDwiGageKindData;

gageItemEntry
_tenDwiGageTable[TEN_DWI_GAGE_ITEM_MAX+1] = {
  /* enum value                      len,deriv,  prereqs,                                                                    parent item, parent index, needData*/
  /* the number of values is learned at run time */
  {tenDwiGageAll,        6660 /* NOT! */,  0,  {-1, -1, -1, -1, -1},                                                                  -1,        -1,    AIR_TRUE},
  {tenDwiGageB0,                       1,  0,  {tenDwiGageAll, /* MAYBE NOT... */ -1, -1, -1, -1},                         tenDwiGageAll,         0,    AIR_TRUE},
  {tenDwiGageMeanDwiValue,             1,  0,  {tenDwiGageAll, -1, -1, -1, -1},                                                       -1,        -1,    AIR_TRUE},

  {tenDwiGageTensorLLS,                7,  0,  {tenDwiGageAll, tenDwiGageMeanDwiValue, -1, -1, -1},                                   -1,        -1,    AIR_TRUE},
  {tenDwiGageTensorLLSError,           1,  0,  {tenDwiGageTensorLLS, -1, -1, -1, -1},                                                 -1,        -1,    AIR_TRUE},
  {tenDwiGageTensorLLSErrorLog,        1,  0,  {tenDwiGageTensorLLS, -1, -1, -1, -1},                                                 -1,        -1,    AIR_TRUE},

  {tenDwiGageTensorWLS,                7,  0,  {tenDwiGageAll, tenDwiGageMeanDwiValue, -1, -1, -1},                                   -1,        -1,    AIR_TRUE},
  {tenDwiGageTensorWLSError,           1,  0,  {tenDwiGageTensorWLS, -1, -1, -1, -1},                                                 -1,        -1,    AIR_TRUE},
  {tenDwiGageTensorWLSErrorLog,        1,  0,  {tenDwiGageTensorWLS, -1, -1, -1, -1},                                                 -1,        -1,    AIR_TRUE},

  {tenDwiGageTensorNLS,                7,  0,  {tenDwiGageAll, tenDwiGageMeanDwiValue, -1, -1, -1},                                   -1,        -1,    AIR_TRUE},
  {tenDwiGageTensorNLSError,           1,  0,  {tenDwiGageTensorNLS, -1, -1, -1, -1},                                                 -1,        -1,    AIR_TRUE},
  {tenDwiGageTensorNLSErrorLog,        1,  0,  {tenDwiGageTensorNLS, -1, -1, -1, -1},                                                 -1,        -1,    AIR_TRUE},

  /* these are NOT sub-items: they are copies, as controlled by the kind->data */
  {tenDwiGageTensor,                   7,  0,  {-1 /* NOT! */, -1, -1, -1, -1},                                                       -1,        -1,    AIR_TRUE},
  {tenDwiGageTensorError,              1,  0,  {-1 /* NOT! */, -1, -1, -1, -1},                                                       -1,        -1,    AIR_TRUE},
  {tenDwiGageTensorErrorLog,           1,  0,  {-1 /* NOT! */, -1, -1, -1, -1},                                                       -1,        -1,    AIR_TRUE},

  {tenDwiGageConfidence,               1,  0,  {tenDwiGageTensor, -1, -1, -1, -1},                                      tenDwiGageTensor,         0,    AIR_TRUE}
};

void
_tenDwiGageIv3Print(FILE *file, gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_tenDwiGageIv3Print";

  AIR_UNUSED(ctx);
  AIR_UNUSED(pvl);
  fprintf(file, "%s: sorry, unimplemented\n", me);
  return;
}

void
_tenDwiGageFilter(gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_tenDwiGageFilter";
  gage_t *fw00, *fw11, *fw22, *dwi;
  int fd;
  tenDwiGageKindData *kindData;
  unsigned int J, dwiNum;

  fd = 2*ctx->radius;
  dwi = pvl->directAnswer[tenDwiGageAll];
  kindData = AIR_CAST(tenDwiGageKindData *, pvl->kind->data);
  dwiNum = kindData->num;
  if (!ctx->parm.k3pack) {
    fprintf(stderr, "!%s: sorry, 6pack filtering not implemented\n", me);
    return;
  }
  fw00 = ctx->fw + fd*3*gageKernel00;
  fw11 = ctx->fw + fd*3*gageKernel11;
  fw22 = ctx->fw + fd*3*gageKernel22;
  /* HEY: these will have to be updated if there is ever any use for
     gradients in DWIs: the second-to-last argument would change */
  switch (fd) {
  case 2:
    for (J=0; J<dwiNum; J++) {
      gageScl3PFilter2(pvl->iv3 + J*8, pvl->iv2 + J*4, pvl->iv1 + J*2,
                       fw00, fw11, fw22,
                       dwi + J, NULL, NULL,
                       pvl->needD[0], AIR_FALSE, AIR_FALSE);
    }
    break;
  case 4:
    for (J=0; J<dwiNum; J++) {
      gageScl3PFilter4(pvl->iv3 + J*64, pvl->iv2 + J*16, pvl->iv1 + J*4,
                       fw00, fw11, fw22,
                       dwi + J, NULL, NULL,
                       pvl->needD[0], AIR_FALSE, AIR_FALSE);
    }
    break;
  default:
    for (J=0; J<dwiNum; J++) {
      gageScl3PFilterN(fd, pvl->iv3 + J*fd*fd*fd,
                       pvl->iv2 + J*fd*fd, pvl->iv1 + J*fd,
                       fw00, fw11, fw22,
                       dwi + J, NULL, NULL,
                       pvl->needD[0], AIR_FALSE, AIR_FALSE);
    }
    break;
  }

  return;
}

void
_tenDwiGageAnswer(gageContext *ctx, gagePerVolume *pvl) {
  AIR_UNUSED(ctx);
  AIR_UNUSED(pvl);

#if 0
  char me[]="_tenDwiGageAnswer";
  unsigned int dwiIdx;
  tenDwiGageKindData *kindData;
  gage_t *dwiAll, dwiMean=0;

  kindData = AIR_CAST(tenDwiGageKindData *, pvl->kind->data);

  dwiAll = pvl->directAnswer[tenDwiGageAll];
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageAll)) {
    /* done if doV */
    if (ctx->verbose) {
      for (dwiIdx=0; dwiIdx<kindData->num; dwiIdx++) {
        fprintf(stderr, "%s: dwi[%u] = %g\n", me, dwiIdx, dwiAll[dwiIdx]);
      }
    }
  }
  /*
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageB0)) {
    done if doV
  }
  */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageMeanDwiValue)) {
    dwiMean = 0;
    for (dwiIdx=1; dwiIdx<kindData->num; dwiIdx++) {
      dwiMean += dwiAll[dwiIdx];
    }
    dwiMean /= 1.0f/(kindData->num - 1);
    pvl->directAnswer[tenDwiGageMeanDwiValue][0] = dwiMean;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorLLS)) {
#if GAGE_TYPE_FLOAT
    tenEstimateLinearSingle_f
#else
    tenEstimateLinearSingle_d
#endif    
      (pvl->directAnswer[tenDwiGageTensorLLS], NULL,
       dwiAll, AIR_CAST(double *, kindData->nemat->data),
       /* pvlData->vbuf */ NULL, kindData->num,
       AIR_TRUE, kindData->dwiConfThresh,
       kindData->dwiConfSoft, kindData->bval);
  }
  /*
  tenDwiGageTensorLinearFitError,
  tenDwiGageTensorNonLinearFit,
  tenDwiGageTensorNonLinearFitError,
  tenDwiGageTensor,
  tenDwiGageTensorError,
  tenDwiGageConfidence
  */
  return;
#endif
}

tenDwiGageKindData*
tenDwiGageKindDataNew() {
#if 0
  tenDwiGageKindData *ret;

  ret = AIR_CAST(tenDwiGageKindData *, malloc(sizeof(tenDwiGageKindData)));
  if (ret) {
    ret->bval = AIR_NAN;
    ret->ngrad = NULL;
    ret->nbmat = NULL;
    ret->dwiConfThresh = 0;
    ret->dwiConfSoft = 0;
    ret->fitType = tenDefDwiGageFitType;
    ret->num = 6661 /* NOT: set by _tenDwiGageKindNumSet() */;
    ret->nemat = nrrdNew();
  }
  return ret;
#endif
  return NULL;
}

tenDwiGageKindData*
tenDwiGageKindDataNix(tenDwiGageKindData *kindData) {

  AIR_UNUSED(kindData);
#if 0  
  if (kindData) {
    nrrdNuke(kindData->nemat);
    airFree(kindData);
  }
#endif
  return NULL;
}

int
tenDwiGageKindCheck(const gageKind *kind) {

  AIR_UNUSED(kind);
#if 0
  char me[]="tenDwiGageKindCheck", err[BIFF_STRLEN];
  
  if (!kind) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (strcmp(kind->name, TEN_DWI_GAGE_KIND_NAME)) {
    sprintf(err, "%s: got \"%s\" kind, not \"%s\"", me,
            kind->name, TEN_DWI_GAGE_KIND_NAME);
    biffAdd(TEN, err); return 1;
  }
  if (!kind->data) {
    sprintf(err, "%s: kind->data is NULL", me);
    biffAdd(TEN, err); return 1;
  }
#endif
  return 0;
}

int
_tenDwiGageKindReadyCheck(const gageKind *kind) {

  AIR_UNUSED(kind);
#if 0
  char me[]="_tenDwiGageKindReadyCheck", err[BIFF_STRLEN];
  tenDwiGageKindData *kindData;

  if (tenDwiGageKindCheck(kind)) {
    sprintf(err, "%s: didn't get valid kind", me);
    biffAdd(TEN, err); return 1;
  }
  kindData = AIR_CAST(tenDwiGageKindData *, kind->data);
  if (!AIR_EXISTS(kindData->bval)) {
    sprintf(err, "%s: bval doesn't exist", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( !!(kindData->ngrad) ^ !!(kindData->nbmat) )) {
    sprintf(err, "%s: not one of ngrad or nbmat set", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( AIR_EXISTS(kindData->dwiConfThresh) 
         && AIR_EXISTS(kindData->dwiConfSoft) )) {
    sprintf(err, "%s: thresh and soft not set", me);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenDwiGageFitType, kindData->fitType)) {
    sprintf(err, "%s: fitType %d not valid", me, kindData->fitType);
    biffAdd(TEN, err); return 1;
  }
#endif
  return 0;
}

void *
_tenDwiGagePvlDataNew(const gageKind *kind) {

  AIR_UNUSED(kind);
#if 0
  char me[]="_tenDwiGagePvlDataNew", err[BIFF_STRLEN];
  tenDwiGagePvlData *pvlData;
  tenDwiGageKindData *kindData;

  if (_tenDwiGageKindReadyCheck(kind)) {
    sprintf(err, "%s: kindData not ready for use", me);
    biffMove(GAGE, err, TEN); return NULL;
  }

  pvlData = AIR_CAST(tenDwiGagePvlData *,
                     malloc(sizeof(tenDwiGagePvlData)));
  if (pvlData) {
    kindData = AIR_CAST(tenDwiGageKindData *, kind->data);
    pvlData->vbuf = AIR_CAST(double *,
                             calloc(kindData->num, sizeof(double)));
  }
  return AIR_CAST(void*, pvlData);
#endif
  return NULL;
}

void *
_tenDwiGagePvlDataCopy(const gageKind *kind, const void *pvlData) {

  AIR_UNUSED(pvlData);
  return _tenDwiGagePvlDataNew(kind);
}

void *
_tenDwiGagePvlDataNix(const gageKind *kind, void *_pvlData) {

  AIR_UNUSED(kind);
  AIR_UNUSED(_pvlData);
#if 0
  tenDwiGagePvlData *pvlData;

  AIR_UNUSED(kind);
  if (_pvlData) {
    pvlData = AIR_CAST(tenDwiGagePvlData *, _pvlData);
    airFree(pvlData->vbuf);
    airFree(pvlData);
  }
#endif
  return NULL;
}

gageKind
_tenDwiGageKind = {
  TEN_DWI_GAGE_KIND_NAME,
  &_tenDwiGage,
  1,
  6662 /* NOT: set by _tenDwiGageKindNumSet() */,
  TEN_DWI_GAGE_ITEM_MAX,
  NULL /* NOT: modified copy of _tenDwiGageTable,
          allocated by tenDwiGageKindNew(), and
          set by _tenDwiGageKindNumSet() */,
  _tenDwiGageIv3Print,
  _tenDwiGageFilter,
  _tenDwiGageAnswer,
  _tenDwiGagePvlDataNew,
  _tenDwiGagePvlDataCopy,
  _tenDwiGagePvlDataNix,
  NULL /* NOT: set by tenDwiGageKindNew() */
};

gageKind *
tenDwiGageKindNew() {
  gageKind *kind;

  kind = AIR_CAST(gageKind *, malloc(sizeof(gageKind)));
  memcpy(kind, &_tenDwiGageKind, sizeof(gageKind));

  kind->table = AIR_CAST(gageItemEntry *,
                         malloc(sizeof(_tenDwiGageTable)));
  memcpy(kind->table, _tenDwiGageTable, sizeof(_tenDwiGageTable));

  kind->data = AIR_CAST(void *, tenDwiGageKindDataNew());

  return kind;
}

gageKind *
tenDwiGageKindNix(gageKind *kind) {

  if (kind) {
    airFree(kind->table);
    tenDwiGageKindDataNix(AIR_CAST(tenDwiGageKindData *, kind->data));
    airFree(kind);
  }
  return NULL;
}

void
_tenDwiGageKindNumSet(gageKind *kind, unsigned int num) {
  char me[]="_tenDwiGageKindNumSet";

  kind->valLen = num;
  fprintf(stderr, "%s: table[%u].answerLength = %u\n", me, 
          tenDwiGageAll, num);
  kind->table[tenDwiGageAll].answerLength = num;
  AIR_CAST(tenDwiGageKindData *, kind->data)->num = num;
  return;
}

int
tenDwiGageKindFitType(gageKind *kind, int fitType) {

  AIR_UNUSED(kind);
  AIR_UNUSED(fitType);
#if 0
  char me[]="tenDwiGageKindFitType", err[BIFF_STRLEN];

  if (tenDwiGageKindCheck(kind)) {
    sprintf(err, "%s: trouble with given kind", me);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenDwiGageFitType, fitType)) {
    sprintf(err, "%s: %d not a valid %s", me, fitType, 
            tenDwiGageFitType->name);
    biffAdd(TEN, err); return 1;
  }

  switch(fitType) {
  case tenDwiGageFitTypeLinear:
    kind->table[tenDwiGageTensor].prereq[0] = 
      tenDwiGageTensorLinearFit;
    kind->table[tenDwiGageTensorError].prereq[0] = 
      tenDwiGageTensorLinearFitError;
    break;
  case tenDwiGageFitTypeNonLinear:
    kind->table[tenDwiGageTensor].prereq[0] = 
      tenDwiGageTensorNonLinearFit;
    kind->table[tenDwiGageTensorError].prereq[0] = 
      tenDwiGageTensorNonLinearFitError;
    break;
  default:
    sprintf(err, "%s: fitType %d not implemented!", me, fitType);
    biffAdd(TEN, err); return 1;
    break;
  }

#endif
  return 0;
}
