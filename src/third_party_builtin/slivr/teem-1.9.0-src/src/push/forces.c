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

#include "push.h"
#include "privatePush.h"

enum {
  pushForceUnknown,
  pushForceSpring,        /* 1 */
  pushForceGauss,         /* 2 */
  pushForceCharge,        /* 3 */
  pushForceCotan,         /* 4 */
  pushForceLast
};
#define PUSH_FORCE_MAX       4

char
_pushForceStr[PUSH_FORCE_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_force)",
  "spring",
  "gauss",
  "charge",
  "cotan",
};

char
_pushForceDesc[PUSH_FORCE_MAX+1][AIR_STRLEN_MED] = {
  "unknown_force",
  "Hooke's law, with a tunable region of attraction",
  "derivative of a Gaussian energy function",
  "inverse square law, with tunable cut-off",
  "Cotangent based energy function (from Meyer et al. SMI 05)",
};

airEnum
_pushForceEnum = {
  "force",
  PUSH_FORCE_MAX,
  _pushForceStr,  NULL,
  _pushForceDesc,
  NULL, NULL,
  AIR_FALSE
};
airEnum *
pushForceEnum = &_pushForceEnum;

/* ----------------------------------------------------------------
** ------------------------------ (stubs) -------------------------
** ----------------------------------------------------------------
*/
push_t
_pushForceUnknownFunc(push_t haveDist, push_t restDist,
                      push_t scale, const push_t *parm) {
  char me[]="_pushForceUnknownFunc";

  AIR_UNUSED(haveDist);
  AIR_UNUSED(restDist);
  AIR_UNUSED(scale);
  AIR_UNUSED(parm);
  fprintf(stderr, "%s: this is not good.\n", me);
  return AIR_NAN;
}

push_t
_pushForceUnknownMaxDist(push_t maxEval, push_t scale, const push_t *parm) {
  char me[]="_pushForceUnknownMaxDist";

  AIR_UNUSED(maxEval);
  AIR_UNUSED(scale);
  AIR_UNUSED(parm);
  fprintf(stderr, "%s: this is not good.\n", me);
  return AIR_NAN;
}

/* ----------------------------------------------------------------
** ------------------------------ SPRING --------------------------
** ----------------------------------------------------------------
** 2 parms:
** 0: spring constant (formerly known as pctx->stiff)
** 1: pull distance
*/
push_t
_pushForceSpringFunc(push_t haveDist, push_t restDist,
                     push_t scale, const push_t *parm) {
  push_t diff, ret, pull;

  pull = parm[1]*scale;
  diff = haveDist - restDist;
  if (diff > pull) {
    ret = 0;
  } else if (diff > 0) {
    ret = diff*(diff*diff/(pull*pull) - 2*diff/pull + 1);
  } else {
    ret = diff;
  }
  ret *= parm[0];
  return ret;
}

push_t
_pushForceSpringMaxDist(push_t maxEval, push_t scale, const push_t *parm) {

  return 2.0f*scale*maxEval*(1.0f + parm[1]);
}

/* ----------------------------------------------------------------
** ------------------------------ GAUSS --------------------------
** ----------------------------------------------------------------
** 1 parms:
** (scale: distance to inflection point of force function)
** parm[0]: cut-off (as a multiple of standard dev)
*/
#define _DGAUSS(x, sig, cut) (                                               \
   x >= sig*cut ? 0                                                          \
   : -exp(-x*x/(2.0*sig*sig))*x)
#define SQRTTHREE 1.73205080756887729352f

push_t
_pushForceGaussFunc(push_t haveDist, push_t restDist,
                    push_t scale, const push_t *parm) {
  push_t sig, cut;

  AIR_UNUSED(scale);
  sig = restDist/SQRTTHREE;
  cut = parm[0];
  return AIR_CAST(push_t, _DGAUSS(haveDist, sig, cut));
}

push_t
_pushForceGaussMaxDist(push_t maxEval, push_t scale, const push_t *parm) {

  return (2.0f*scale*maxEval/SQRTTHREE)*parm[0];
}

/* ----------------------------------------------------------------
** ------------------------------ CHARGE --------------------------
** ----------------------------------------------------------------
** 2 parms:
** (scale: distance to "1.0" in graph of x^(-2))
** parm[0]: vertical scaling 
** parm[1]: cut-off (as multiple of "1.0")
*/
push_t
_pushForceChargeFunc(push_t haveDist, push_t restDist,
                     push_t scale, const push_t *parm) {
  push_t xx;

  AIR_UNUSED(scale);
  xx = haveDist/restDist;
  return -parm[0]*(xx > parm[1] ? 0 : 1.0f/(xx*xx));
}

push_t
_pushForceChargeMaxDist(push_t maxEval, push_t scale, const push_t *parm) {

  return (2*scale*maxEval)*parm[1];
}

/* ----------------------------------------------------------------
** ------------------------------ COTAN ---------------------------
** ----------------------------------------------------------------
** 1 parms:
** (scale: distance to "1.0")
** parm[0]: vertical scaling 
*/
push_t
_pushForceCotanFunc(push_t haveDist, push_t restDist,
                    push_t scale, const push_t *parm) {
  push_t xx, ss;

  AIR_UNUSED(scale);
  xx = haveDist/restDist;
  ss = AIR_CAST(push_t, sin(xx*AIR_PI/2.0));
  return parm[0]*(xx > 1 ? 0 : 1.0f - 1.0f/(ss*ss));
}

push_t
_pushForceCotanMaxDist(push_t maxEval, push_t scale, const push_t *parm) {

  AIR_UNUSED(parm);
  return 2*scale*maxEval;
}

/* ----------------------------------------------------------------
** ------------------------------ arrays ... ----------------------
** ----------------------------------------------------------------
*/

int
_pushForceParmNum[PUSH_FORCE_MAX+1] = {
  0, /* pushForceUnknown */
  2, /* pushForceSpring */
  1, /* pushForceGauss */
  2, /* pushForceCharge */
  1  /* pushForceCotan */
};

push_t
(*_pushForceFunc[PUSH_FORCE_MAX+1])(push_t haveDist,
                                    push_t restDist,
                                    push_t scale,
                                    const push_t *parm) = {
                                      _pushForceUnknownFunc,
                                      _pushForceSpringFunc,
                                      _pushForceGaussFunc,
                                      _pushForceChargeFunc,
                                      _pushForceCotanFunc,
};

push_t
(*_pushForceMaxDist[PUSH_FORCE_MAX+1])(push_t maxEval,
                                       push_t scale,
                                       const push_t *parm) = {
                                         _pushForceUnknownMaxDist,
                                         _pushForceSpringMaxDist,
                                         _pushForceGaussMaxDist,
                                         _pushForceChargeMaxDist,
                                         _pushForceCotanMaxDist,
};

pushForce *
_pushForceNew() {
  pushForce *force;
  int pi;

  force = (pushForce *)calloc(1, sizeof(pushForce));
  if (force) {
    force->func = NULL;
    force->maxDist = NULL;
    for (pi=0; pi<PUSH_FORCE_PARM_MAXNUM; pi++) {
      force->parm[pi] = AIR_NAN;
    }
  }
  return force;
}

pushForce *
pushForceNix(pushForce *force) {

  airFree(force);
  return NULL;
}

pushForce *
pushForceParse(const char *_str) {
  char me[]="pushForceParse", err[BIFF_STRLEN];
  char *str, *col, *_pstr, *pstr;
  pushForce *force;
  int fri, needParm, haveParm;
  airArray *mop;
  double pval;

  if (!_str) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(PUSH, err); return NULL;
  }

  mop = airMopNew();
  str = airStrdup(_str);
  airMopAdd(mop, str, (airMopper)airFree, airMopAlways);
  force = _pushForceNew();
  airMopAdd(mop, force, (airMopper)pushForceNix, airMopOnError);

  col = strchr(str, ':');
  if (!col) {
    sprintf(err, "%s: didn't see colon separator in \"%s\"", me, str);
    biffAdd(PUSH, err); airMopError(mop); return NULL;
  }
  *col = '\0';
  fri = airEnumVal(pushForceEnum, str);
  if (pushForceUnknown == fri) {
    sprintf(err, "%s: didn't recognize \"%s\" as a force", me, str);
  }
  needParm = _pushForceParmNum[fri];
  _pstr = pstr = col+1;
  /* code lifted from teem/src/nrrd/kernel.c, should probably refactor... */
  for (haveParm=0; haveParm<needParm; haveParm++) {
    if (!pstr) {
      break;
    }
    if (1 != sscanf(pstr, "%lg", &pval)) {
      sprintf(err, "%s: trouble parsing \"%s\" as double (in \"%s\")",
              me, _pstr, _str);
      biffAdd(PUSH, err); airMopError(mop); return NULL;
    }
    force->parm[haveParm] = AIR_CAST(push_t, pval);
    if ((pstr = strchr(pstr, ','))) {
      pstr++;
      if (!*pstr) {
        sprintf(err, "%s: nothing after last comma in \"%s\" (in \"%s\")",
                me, _pstr, _str);
        biffAdd(PUSH, err); airMopError(mop); return NULL;
      }
    }
  }
  /* haveParm is now the number of parameters that were parsed. */
  if (haveParm < needParm) {
    sprintf(err, "%s: parsed only %d of %d required parameters (for %s force)"
            "from \"%s\" (in \"%s\")",
            me, haveParm, needParm,
            airEnumStr(pushForceEnum, fri), _pstr, _str);
      biffAdd(PUSH, err); airMopError(mop); return NULL;
  } else {
    if (pstr) {
      sprintf(err, "%s: \"%s\" (in \"%s\") has more than %d doubles",
              me, _pstr, _str, needParm);
      biffAdd(PUSH, err); airMopError(mop); return NULL;
    }
  }
  
  /* parameters have been set, now set the rest of the force info */
  force->func = _pushForceFunc[fri];
  force->maxDist = _pushForceMaxDist[fri];

  airMopOkay(mop);
  return force;
}

int
_pushHestForceParse(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  pushForce **fcP;
  char me[]="_pushHestForceParse", *perr;

  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  fcP = (pushForce **)ptr;
  *fcP = pushForceParse(str);
  if (!(*fcP)) {
    perr = biffGetDone(PUSH);
    strncpy(err, perr, AIR_STRLEN_HUGE-1);
    free(perr);
    return 1;
  }
  return 0;
}

hestCB
_pushHestForce = {
  sizeof(pushForce*),
  "force specification",
  _pushHestForceParse,
  (airMopper)pushForceNix
};

hestCB *
pushHestForce = &_pushHestForce;
