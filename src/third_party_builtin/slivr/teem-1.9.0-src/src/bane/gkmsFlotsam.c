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

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/*
** baneGkmsParseIncStrategy
**
** inc[0]: member of baneInc* enum
** inc[1], inc[2] ... : incParm[0], incParm[1] ... 
*/
int
baneGkmsParseIncStrategy(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  char me[]="baneGkmsParseIncStrategy";
  double *inc, *incParm;
  int i, bins;
  
  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  inc = (double *)ptr;
  incParm = inc + 1;
  for (i=0; i<BANE_PARM_NUM; i++) {
    incParm[i] = AIR_NAN;
  }
  if (1 == sscanf(str, "f:%lg", incParm+0) 
      || 2 == sscanf(str, "f:%lg,%lg", incParm+0, incParm+1)) {
    inc[0] = baneIncRangeRatio;
    return 0;
  }
  if (1 == sscanf(str, "p:%lg", incParm+1)
      || 2 == sscanf(str, "p:%lg,%lg", incParm+1, incParm+2)) {
    inc[0] = baneIncPercentile;
    incParm[0] = baneDefPercHistBins;
    return 0;
  }
  if (3 == sscanf(str, "p:%d,%lg,%lg", &bins, incParm+1, incParm+2)) {
    inc[0] = baneIncPercentile;
    incParm[0] = bins;
    return 0;
  }
  if (2 == sscanf(str, "a:%lg,%lg", incParm+0, incParm+1)) {
    inc[0] = baneIncAbsolute;
    return 0;
  }
  if (1 == sscanf(str, "s:%lg", incParm+0)
      || 2 == sscanf(str, "s:%lg,%lg", incParm+1, incParm+2)) {
    inc[0] = baneIncStdv;
    return 0;
  }

  /* else its no good */

  sprintf(err, "%s: \"%s\" not recognized", me, str);
  return 1;
}

hestCB _baneGkmsHestIncStrategy = {
  (1+BANE_PARM_NUM)*sizeof(double),
  "inclusion strategy",
  baneGkmsParseIncStrategy,
  NULL
};

hestCB *
baneGkmsHestIncStrategy = &_baneGkmsHestIncStrategy;

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

int
baneGkmsParseBEF(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  char me[]="baneGkmsParseBEF", mesg[AIR_STRLEN_MED], *nerr;
  float cent, width, shape, alpha, off, *bef;
  Nrrd **nrrdP;
  airArray *mop;
  
  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  mop = airMopNew();
  nrrdP = (Nrrd **)ptr;
  airMopAdd(mop, *nrrdP=nrrdNew(), (airMopper)nrrdNuke, airMopOnError);
  if (4 == sscanf(str, "%g,%g,%g,%g", &shape, &width, &cent, &alpha)) {
    /* its a valid BEF specification, we make the nrrd ourselves */
    if (nrrdMaybeAlloc_va(*nrrdP, nrrdTypeFloat, 2,
                          AIR_CAST(size_t, 2), AIR_CAST(size_t, 6))) {
      airMopAdd(mop, nerr = biffGetDone(NRRD), airFree, airMopOnError);
      strncpy(err, nerr, AIR_STRLEN_HUGE-1);
      airMopError(mop);
      return 1;
    }
    bef = (float *)((*nrrdP)->data);
    off = AIR_CAST(float, AIR_AFFINE(0.0, shape, 1.0, 0.0, width/2));
    /* positions */
    bef[0 + 2*0] = cent - 2*width;
    bef[0 + 2*1] = cent - width/2 - off;
    bef[0 + 2*2] = cent - width/2 + off;
    bef[0 + 2*3] = cent + width/2 - off;
    bef[0 + 2*4] = cent + width/2 + off;
    bef[0 + 2*5] = cent + 2*width;
    if (bef[0 + 2*1] == bef[0 + 2*2]) bef[0 + 2*1] -= 0.001f;
    if (bef[0 + 2*2] == bef[0 + 2*3]) bef[0 + 2*2] -= 0.001f;
    if (bef[0 + 2*3] == bef[0 + 2*4]) bef[0 + 2*4] += 0.001f;
    /* opacities */
    bef[1 + 2*0] = 0.0;
    bef[1 + 2*1] = 0.0;
    bef[1 + 2*2] = alpha;
    bef[1 + 2*3] = alpha;
    bef[1 + 2*4] = 0.0;
    bef[1 + 2*5] = 0.0;
    /* to tell gkms opac that this came from four floats */
    (*nrrdP)->ptr = *nrrdP;
  } else {
    if (nrrdLoad(*nrrdP, str, NULL)) {
      airMopAdd(mop, nerr = biffGetDone(NRRD), airFree, airMopOnError);
      sprintf(mesg, "%s: couldn't parse \"%s\" as four-parameter BEF or "
              "as a nrrd filename\n", me, str);
      strcpy(err, mesg);
      strncat(err, nerr, AIR_STRLEN_HUGE-1-strlen(mesg)-1);
      airMopError(mop);
      return 1;
    }
    (*nrrdP)->ptr = NULL;
  }

  airMopOkay(mop);
  return 0;
}

hestCB _baneGkmsHestBEF = {
  sizeof(Nrrd *),
  "boundary emphasis function",
  baneGkmsParseBEF,
  (airMopper)nrrdNuke
};

hestCB *
baneGkmsHestBEF = &_baneGkmsHestBEF;

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/*
** gthr[0] = 1: some scaling of max grad "x<float>"
**           0: absolute                 "<float>"
** gthr[1] = the scaling, or the absolute
*/
int
baneGkmsParseGthresh(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  char me[]="baneGkmsParseGthresh";
  float *gthr;
  
  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  gthr = (float *)ptr;

  if ('x' == str[0]) {
    if (1 != sscanf(str+1, "%f", gthr+1)) {
      sprintf(err, "%s: can't parse \"%s\" as x<float>", me, str);
      return 1;
    }
    gthr[0] = 1;
  } else {
    if (1 != sscanf(str, "%f", gthr+1)) {
      sprintf(err, "%s: can't parse \"%s\" as float", me, str);
      return 1;
    }
    gthr[0] = 0;
  }
  return 0;
}

hestCB _baneGkmsHestGthresh = {
  2*sizeof(float),
  "gthresh specification",
  baneGkmsParseGthresh,
  NULL
};

hestCB *
baneGkmsHestGthresh = &_baneGkmsHestGthresh;

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/*
******** baneGkmsCmdList[]
**
** NULL-terminated array of unrrduCmd pointers, as ordered by
** BANE_GKMS_MAP macro
*/
unrrduCmd *
baneGkmsCmdList[] = {
  BANE_GKMS_MAP(BANE_GKMS_LIST)
  NULL
};

/*
******** baneGkmsUsage
**
** prints out a little banner, and a listing of all available commands
** with their one-line descriptions
*/
void
baneGkmsUsage(char *me, hestParm *hparm) {
  int i, maxlen, len, c;
  char buff[AIR_STRLEN_LARGE], fmt[AIR_STRLEN_LARGE];

  maxlen = 0;
  for (i=0; baneGkmsCmdList[i]; i++) {
    maxlen = AIR_MAX(maxlen, (int)strlen(baneGkmsCmdList[i]->name));
  }

  sprintf(buff, "--- Semi-Automatic Generation of Transfer Functions ---");
  sprintf(fmt, "%%%ds\n",
          (int)((hparm->columns-strlen(buff))/2 + strlen(buff) - 1));
  fprintf(stderr, fmt, buff);
  
  for (i=0; baneGkmsCmdList[i]; i++) {
    len = strlen(baneGkmsCmdList[i]->name);
    strcpy(buff, "");
    for (c=len; c<maxlen; c++)
      strcat(buff, " ");
    strcat(buff, me);
    strcat(buff, " ");
    strcat(buff, baneGkmsCmdList[i]->name);
    strcat(buff, " ... ");
    len = strlen(buff);
    fprintf(stderr, "%s", buff);
    _hestPrintStr(stderr, len, len, hparm->columns,
                  baneGkmsCmdList[i]->info, AIR_FALSE);
  }
}

char
_baneGkmsMeasrStr[][AIR_STRLEN_SMALL] = {
  "(unknown measr)",
  "min",
  "max",
  "mean",
  "median",
  "mode"
};

int
_baneGkmsMeasrVal[] = {
  nrrdMeasureUnknown,
  nrrdMeasureHistoMin,
  nrrdMeasureHistoMax,
  nrrdMeasureHistoMean,
  nrrdMeasureHistoMedian,
  nrrdMeasureHistoMode
};

airEnum
_baneGkmsMeasr = {
  "measurement",
  5,
  _baneGkmsMeasrStr, _baneGkmsMeasrVal,
  NULL,
  NULL, NULL,
  AIR_FALSE
};

airEnum *
baneGkmsMeasr = &_baneGkmsMeasr;
