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

#include "hest.h"
#include "privateHest.h"
#include <limits.h>

hestParm *
hestParmNew() {
  hestParm *parm;
  
  parm = (hestParm *)calloc(1, sizeof(hestParm));
  if (parm) {
    parm->verbosity = hestVerbosity;
    parm->respFileEnable = hestRespFileEnable;
    parm->elideSingleEnumType = hestElideSingleEnumType;
    parm->elideSingleOtherType = hestElideSingleOtherType;
    parm->elideSingleOtherDefault = hestElideSingleOtherDefault;
    parm->greedySingleString = hestGreedySingleString;
    parm->elideSingleNonExistFloatDefault =
      hestElideSingleNonExistFloatDefault;
    parm->elideMultipleNonExistFloatDefault =
      hestElideMultipleNonExistFloatDefault;
    parm->elideSingleEmptyStringDefault =
      hestElideSingleEmptyStringDefault;
    parm->elideMultipleEmptyStringDefault =
      hestElideMultipleEmptyStringDefault;
    parm->cleverPluralizeOtherY = hestCleverPluralizeOtherY;
    parm->columns = hestColumns;
    parm->respFileFlag = hestRespFileFlag;
    parm->respFileComment = hestRespFileComment;
    parm->varParamStopFlag = hestVarParamStopFlag;
    parm->multiFlagSep = hestMultiFlagSep;
  }
  return parm;
}

hestParm *
hestParmFree(hestParm *parm) {

  airFree(parm);
  return NULL;
}

void
_hestOptInit(hestOpt *opt) {

  opt->flag = opt->name = NULL;
  opt->type = opt->min = opt->max = 0;
  opt->valueP = NULL;
  opt->dflt = opt->info = NULL;
  opt->sawP = NULL;
  opt->enm = NULL;
  opt->CB = NULL;
  opt->sawP = NULL;
  opt->kind = opt->alloc = 0;
}

/*
hestOpt *
hestOptNew(void) {
  hestOpt *opt;
  
  opt = calloc(1, sizeof(hestOpt));
  if (opt) {
    _hestOptInit(opt);
    opt->min = 1;
  }
  return opt;
}
*/

void
hestOptAdd(hestOpt **optP, 
           char *flag, char *name,
           int type, int min, int max,
           void *valueP, const char *dflt, const char *info, ...) {
  hestOpt *ret = NULL;
  int num;
  va_list ap;
  void *dummy;

  if (!optP)
    return;

  num = *optP ? _hestNumOpts(*optP) : 0;
  if (!(ret = (hestOpt *)calloc(num+2, sizeof(hestOpt)))) {
    return;
  }
  if (num)
    memcpy(ret, *optP, num*sizeof(hestOpt));
  ret[num].flag = airStrdup(flag);
  ret[num].name = airStrdup(name);
  ret[num].type = type;
  ret[num].min = min;
  ret[num].max = max;
  ret[num].valueP = valueP;
  ret[num].dflt = airStrdup(dflt);
  ret[num].info = airStrdup(info);
  /* initialize the things that may be set below */
  ret[num].sawP = NULL;
  ret[num].enm = NULL;
  ret[num].CB = NULL;
  /* deal with var args */
  if (5 == _hestKind(&(ret[num]))) {
    va_start(ap, info);
    ret[num].sawP = va_arg(ap, unsigned int*);
    va_end(ap);
  }
  if (airTypeEnum == type) {
    va_start(ap, info);
    dummy = (void *)(va_arg(ap, unsigned int*));  /* skip sawP */
    ret[num].enm = va_arg(ap, airEnum*);
    va_end(ap);
  }
  if (airTypeOther == type) {
    va_start(ap, info);
    dummy = (void *)(va_arg(ap, unsigned int*));      /* skip sawP */
    dummy = (void *)(va_arg(ap, airEnum*));  /* skip enm */
    ret[num].CB = va_arg(ap, hestCB*);
    va_end(ap);
  }
  _hestOptInit(&(ret[num+1]));
  ret[num+1].min = 1;
  if (*optP)
    free(*optP);
  *optP = ret;
  dummy = dummy;
  return;
}

void
_hestOptFree(hestOpt *opt) {
  
  opt->flag = (char *)airFree(opt->flag);
  opt->name = (char *)airFree(opt->name);
  opt->dflt = (char *)airFree(opt->dflt);
  opt->info = (char *)airFree(opt->info);
  return;
}

hestOpt *
hestOptFree(hestOpt *opt) {
  int op, num;

  if (!opt)
    return NULL;

  num = _hestNumOpts(opt);
  if (opt[num].min) {
    /* we only try to free this if it looks like something we allocated */
    for (op=0; op<num; op++) {
      _hestOptFree(opt+op);
    }
    free(opt);
  }
  return NULL;
}

int
hestOptCheck(hestOpt *opt, char **errP) {
  char *err, me[]="hestOptCheck";
  hestParm *parm;
  int big;

  big = _hestErrStrlen(opt, 0, NULL);
  if (!(err = (char *)calloc(big, sizeof(char)))) {
    fprintf(stderr, "%s PANIC: couldn't allocate error message "
            "buffer (size %d)\n", me, big);
    exit(1);
  }
  parm = hestParmNew();
  if (_hestPanic(opt, err, parm)) {
    /* problems */
    if (errP) {
      /* they did give a pointer address; they'll free it */
      *errP = err;
    }
    else {
      /* they didn't give a pointer address; their loss */
      free(err);
    }
    hestParmFree(parm);
    return 1;
  }
  /* else, no problems */
  if (errP)
    *errP = NULL;
  free(err);
  hestParmFree(parm);
  return 0;
}


/*
** _hestIdent()
**
** how to identify an option in error and usage messages
*/
char *
_hestIdent(char *ident, hestOpt *opt, hestParm *parm, int brief) {
  char copy[AIR_STRLEN_HUGE], *sep;

  if (opt->flag && (sep = strchr(opt->flag, parm->multiFlagSep))) {
    strcpy(copy, opt->flag);
    sep = strchr(copy, parm->multiFlagSep);
    *sep = '\0';
    if (brief) 
      sprintf(ident, "-%s%c--%s option", copy, parm->multiFlagSep, sep+1);
    else 
      sprintf(ident, "-%s option", copy);
  }
  else {
    sprintf(ident, "%s%s%s option", 
            opt->flag ? "\"-"      : "<",
            opt->flag ? opt->flag : opt->name,
            opt->flag ? "\""       : ">");
  }
  return ident;
}

int
_hestMax(int max) {
  
  if (-1 == max) {
    max = INT_MAX;
  }
  return max;
}

int
_hestKind(hestOpt *opt) {
  int max;
  
  max = _hestMax(opt->max);
  if (!( (int)opt->min <= max )) {    /* HEY scrutinize casts */
    /* invalid */
    return -1;
  }

  if (0 == opt->min && 0 == max) {
    /* flag */
    return 1;
  }

  if (1 == opt->min && 1 == max) {
    /* single fixed parameter */
    return 2;
  }

  if (2 <= opt->min && 2 <= max && (int)opt->min == max) {  /* HEY scrutinize casts */
    /* multiple fixed parameters */
    return 3;
  }
  
  if (0 == opt->min && 1 == max) {
    /* single optional parameter */
    return 4;
  }

  /* else multiple variable parameters */
  return 5;
}

void
_hestPrintArgv(int argc, char **argv) {
  int a;

  printf("argc=%d : ", argc);
  for (a=0; a<argc; a++) {
    printf("%s ", argv[a]);
  }
  printf("\n");
}

/*
** _hestWhichFlag()
**
** given a string in "flag" (with the hypen prefix) finds which of
** the flags in the given array of options matches that.  Returns
** the index of the matching option, or -1 if there is no match,
** but returns -2 if the flag is the end-of-variable-parameter
** marker (according to parm->varParamStopFlag)
*/
int
_hestWhichFlag(hestOpt *opt, char *flag, hestParm *parm) {
  char buff[AIR_STRLEN_HUGE], copy[AIR_STRLEN_HUGE], *sep;
  int op, numOpts;
  
  numOpts = _hestNumOpts(opt);
  if (parm->verbosity)
    printf("_hestWhichFlag: flag = %s, numOpts = %d\n", flag, numOpts);
  for (op=0; op<numOpts; op++) {
    if (parm->verbosity)
      printf("_hestWhichFlag: op = %d\n", op);
    if (!opt[op].flag)
      continue;
    if (strchr(opt[op].flag, parm->multiFlagSep) ) {
      strcpy(copy, opt[op].flag);
      sep = strchr(copy, parm->multiFlagSep);
      *sep = '\0';
      /* first try the short version */
      sprintf(buff, "-%s", copy);
      if (!strcmp(flag, buff))
        return op;
      /* then try the long version */
      sprintf(buff, "--%s", sep+1);
      if (!strcmp(flag, buff))
        return op;
    }
    else {
      /* flag has only the short version */
      sprintf(buff, "-%s", opt[op].flag);
      if (!strcmp(flag, buff))
        return op;
    }
  }
  if (parm->verbosity)
    printf("_hestWhichFlag: numOpts = %d\n", numOpts);
  if (parm->varParamStopFlag) {
    sprintf(buff, "-%c", parm->varParamStopFlag);
    if (parm->verbosity)
      printf("_hestWhichFlag: flag = %s, buff = %s\n", flag, buff);
    if (!strcmp(flag, buff))
      return -2;
  }
  if (parm->verbosity)
    printf("_hestWhichFlag: numOpts = %d\n", numOpts);
  return -1;
}


/*
** _hestCase()
**
** helps figure out logic of interpreting parameters and defaults
** for kind 4 and kind 5 options.
*/
int
_hestCase(hestOpt *opt, int *udflt, unsigned int *nprm, int *appr, int op) {
  
  if (opt[op].flag && !appr[op]) {
    return 0;
  }
  else if ( (4 == opt[op].kind && udflt[op]) ||
            (5 == opt[op].kind && !nprm[op]) ) {
    return 1;
  }
  else {
    return 2;
  }
}

/*
** _hestExtract()
**
** takes "np" parameters, starting at "a", out of the given argv, and puts
** them into a string WHICH THIS FUNCTION ALLOCATES, and also adjusts
** the argc value given as "*argcP".
*/
char *
_hestExtract(int *argcP, char **argv, int a, int np) {
  int len, n;
  char *ret;

  if (!np)
    return NULL;

  len = 0;
  for (n=0; n<np; n++) {
    if (a+n==*argcP) {
      return NULL;
    }
    len += strlen(argv[a+n]);
    if (strstr(argv[a+n], " ")) {
      len += 2;
    }
  }
  len += np;
  ret = (char *)calloc(len, sizeof(char));
  strcpy(ret, "");
  for (n=0; n<np; n++) {
    /* if a single element of argv has spaces in it, someone went
       to the trouble of putting it in quotes, and we perpetuate
       the favor by quoting it when we concatenate all the argv
       elements together, so that airParseStrS will recover it as a 
       single string again */
    if (strstr(argv[a+n], " ")) {
      strcat(ret, "\"");
    }
    /* HEY: if there is a '\"' character in this string, quoted or
       not, its going to totally confuse later parsing */
    strcat(ret, argv[a+n]);
    if (strstr(argv[a+n], " ")) {
      strcat(ret, "\"");
    }
    if (n < np-1)
      strcat(ret, " ");
  }
  for (n=a+np; n<=*argcP; n++) {
    argv[n-np] = argv[n];
  }
  *argcP -= np;
  return ret;
}

int
_hestNumOpts(hestOpt *opt) {
  int num = 0;

  while (opt[num].flag || opt[num].name || opt[num].type) {
    num++;
  }
  return num;
}

int
_hestArgc(char **argv) {
  int num = 0;

  while (argv && argv[num]) {
    num++;
  }
  return num;
}


