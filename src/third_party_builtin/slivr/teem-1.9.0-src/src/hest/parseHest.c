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

/*
learned: well duh: when you send arguments to printf(), they will
be evaluated before printf() sees them, so you can't use _hestIdent()
twice with differen values
*/

#include "hest.h"
#include "privateHest.h"

#define ME ((parm && parm->verbosity) ? me : "")

/*
** _hestArgsInResponseFiles()
**
** returns the number of args that will be parsed from the response files.
** The role of this function is solely to simplify the task of avoiding
** memory leaks.  By knowing exactly how many args we'll get in the response
** file, then hestParse() can allocate its local argv[] for exactly as
** long as it needs to be, and we can avoid using an airArray.  The drawback
** is that we open and read through the response files twice.  Alas.
*/
int
_hestArgsInResponseFiles(int *argcP, int *nrfP,
                         char **argv, char *err, hestParm *parm) {
  FILE *file;
  char me[]="_hestArgsInResponseFiles: ", line[AIR_STRLEN_HUGE], *pound;
  int ai, len;

  *argcP = 0;
  *nrfP = 0;
  if (!parm->respFileEnable) {
    /* don't do response files; we're done */
    return 0;
  }

  ai = 0;
  while (argv[ai]) {
    if (parm->respFileFlag == argv[ai][0]) {
      if (!(file = fopen(argv[ai]+1, "rb"))) {
        /* can't open the indicated response file for reading */
        sprintf(err, "%scouldn't open \"%s\" for reading as response file",
                ME, argv[ai]+1);
        *argcP = 0;
        *nrfP = 0;
        return 1;
      }
      len = airOneLine(file, line, AIR_STRLEN_HUGE);
      while (len > 0) {
        if ( (pound = strchr(line, parm->respFileComment)) )
          *pound = '\0';
        airOneLinify(line);
        *argcP += airStrntok(line, AIR_WHITESPACE);
        len = airOneLine(file, line, AIR_STRLEN_HUGE);
      }
      fclose(file);
      (*nrfP)++;
    }
    ai++;
  }
  return 0;
}

/*
** _hestResponseFiles()
**
** This function is badly named.  Even if there are no response files,
** even if response files are disabled, this is the function that
** copies from the user's argc,argv to our local copy.
*/
int
_hestResponseFiles(char **newArgv, char **oldArgv,
                   hestParm *parm, airArray *pmop) {
  char line[AIR_STRLEN_HUGE], *pound;
  int len, newArgc, oldArgc, incr, ai;
  FILE *file;
  
  newArgc = oldArgc = 0;
  while(oldArgv[oldArgc]) {
    if (parm->verbosity) {
      printf("!%s:________ newArgc = %d, oldArgc = %d\n", 
             "dammit", newArgc, oldArgc);
      _hestPrintArgv(newArgc, newArgv);
    }
    if (!parm->respFileEnable
        || parm->respFileFlag != oldArgv[oldArgc][0]) {
      /* nothing to do with a response file, just copy the arg over.
         We are not allocating new memory in this case. */
      newArgv[newArgc] = oldArgv[oldArgc];
      newArgc += 1;
    }
    else {
      /* It is a response file.  Error checking on open-ability
         should have been done by _hestArgsInResponseFiles() */
      file = fopen(oldArgv[oldArgc]+1, "rb");
      len = airOneLine(file, line, AIR_STRLEN_HUGE);
      while (len > 0) {
        if (parm->verbosity)
          printf("_hestResponseFiles: line: |%s|\n", line);
        if ( (pound = strchr(line, parm->respFileComment)) )
          *pound = '\0';
        if (parm->verbosity)
          printf("_hestResponseFiles: -0-> line: |%s|\n", line);
        airOneLinify(line);
        incr = airStrntok(line, AIR_WHITESPACE);
        if (parm->verbosity)
          printf("_hestResponseFiles: -1-> line: |%s|, incr=%d\n",
                 line, incr);
        airParseStrS(newArgv + newArgc, line, AIR_WHITESPACE, incr, AIR_FALSE);
        for (ai=0; ai<incr; ai++) {
          /* This time, we did allocate memory.  We can use airFree and
             not airFreeP because these will not be reset before mopping */
          airMopAdd(pmop, newArgv[newArgc+ai], airFree, airMopAlways);
        }
        len = airOneLine(file, line, AIR_STRLEN_HUGE);
        newArgc += incr;
      }
      fclose(file);
    }
    oldArgc++;
    if (parm->verbosity) {
      _hestPrintArgv(newArgc, newArgv);
      printf("!%s: ^^^^^^^ newArgc = %d, oldArgc = %d\n", 
             "dammit", newArgc, oldArgc);
    }
  }
  newArgv[newArgc] = NULL;

  return 0;
}

/*
** _hestPanic()
**
** all error checking on the given hest array itself (not the
** command line to be parsed).  Also, sets the "kind" field of
** the opt struct
*/
int
_hestPanic(hestOpt *opt, char *err, hestParm *parm) {
  char me[]="_hestPanic: ", tbuff[AIR_STRLEN_HUGE], *sep;
  int numvar, op, numOpts;

  numOpts = _hestNumOpts(opt);
  numvar = 0;
  for (op=0; op<numOpts; op++) {
    opt[op].kind = _hestKind(opt + op);
    if (!(AIR_IN_OP(airTypeUnknown, opt[op].type, airTypeLast))) {
      if (err)
        sprintf(err, "%s!!!!!! opt[%d].type (%d) not in valid range [%d,%d]",
                ME, op, opt[op].type, airTypeUnknown+1, airTypeLast-1);
      else
        fprintf(stderr, "%s: panic 0\n", me);
      return 1;
    }
    if (!( opt[op].valueP )) {
      if (err)
        sprintf(err, "%s!!!!!! opt[%d]'s valueP is NULL!", ME, op);
      else
        fprintf(stderr, "%s: panic 0.5\n", me);
      return 1;
    }
    if (-1 == opt[op].kind) {
      if (err)
        sprintf(err, "%s!!!!!! opt[%d]'s min (%d) and max (%d) incompatible",
                ME, op, opt[op].min, opt[op].max);
      else
        fprintf(stderr, "%s: panic 1\n", me);
      return 1;
    }
    if (5 == opt[op].kind && !(opt[op].sawP)) {
      if (err)
        sprintf(err, "%s!!!!!! have multiple variable parameters, "
                "but sawP is NULL", ME);
      else
        fprintf(stderr, "%s: panic 2\n", me);
      return 1;
    }
    if (airTypeEnum == opt[op].type) {
      if (!(opt[op].enm)) {
        if (err)
          sprintf(err, "%s!!!!!! opt[%d] is type \"enum\", but no "
                  "airEnum pointer given", ME, op);
        else
          fprintf(stderr, "%s: panic 3\n", me);
        return 1;
      }
    }
    if (airTypeOther == opt[op].type) {
      if (!(opt[op].CB)) {
        if (err)
          sprintf(err, "%s!!!!!! opt[%d] is type \"other\", but no "
                  "callbacks given", ME, op);
        else
          fprintf(stderr, "%s: panic 4\n", me);
        return 1;
      }
      if (!( opt[op].CB->size > 0 )) {
        if (err)
          sprintf(err, "%s!!!!!! opt[%d]'s \"size\" (%d) invalid", 
                  ME, op, (int)(opt[op].CB->size));
        else
          fprintf(stderr, "%s: panic 5\n", me);
        return 1;
      }
      if (!( opt[op].type )) {
        if (err)
          sprintf(err, "%s!!!!!! opt[%d]'s \"type\" is NULL",
                  ME, op);
        else
          fprintf(stderr, "%s: panic 6\n", me);
        return 1;

      }
      if (!( opt[op].CB->parse )) {
        if (err)
          sprintf(err, "%s!!!!!! opt[%d]'s \"parse\" callback NULL", ME, op);
        else
          fprintf(stderr, "%s: panic 7\n", me);
        return 1;
      }
      if (opt[op].CB->destroy && (sizeof(void*) != opt[op].CB->size)) {
        if (err)
          sprintf(err, "%s!!!!!! opt[%d] has a \"destroy\", but size isn't "
                  "sizeof(void*)", ME, op);
        else
          fprintf(stderr, "%s: panic 8\n", me);
        return 1;
      }
    }
    if (opt[op].flag) {
      strcpy(tbuff, opt[op].flag);
      if (( sep = strchr(tbuff, parm->multiFlagSep) )) {
        *sep = '\0';
        if (!( strlen(tbuff) && strlen(sep+1) )) {
          if (err)
            sprintf(err, "%s!!!!!! either short (\"%s\") or long (\"%s\") flag"
                    " of opt[%d] is zero length", ME, tbuff, sep+1, op);
          else
            fprintf(stderr, "%s: panic 9\n", me);
          return 1;
        }
      }
      else {
        if (!strlen(opt[op].flag)) {
          if (err)
            sprintf(err, "%s!!!!!! opt[%d].flag is zero length",
                    ME, op);
          else
            fprintf(stderr, "%s: panic 10\n", me);
          return 1;
        }
      }
      if (4 == opt[op].kind) {
        if (!opt[op].dflt) {
          if (err)
            sprintf(err, "%s!!!!!! flagged single variable parameter must "
                    "specify a default", ME);
          else 
            fprintf(stderr, "%s: panic 11\n", me);
          return 1;
        }
        if (!strlen(opt[op].dflt)) {
          if (err) 
            sprintf(err, "%s!!!!!! flagged single variable parameter default "
                    "must be non-zero length", ME);
          else
            fprintf(stderr, "%s: panic 12\n", me);
          return 1;
        }
      }
      /*
      sprintf(tbuff, "-%s", opt[op].flag);
      if (1 == sscanf(tbuff, "%f", &tmpF)) {
        if (err)
          sprintf(err, "%s!!!!!! opt[%d].flag (\"%s\") is numeric, bad news",
                  ME, op, opt[op].flag);
        return 1;
      }
      */
    }
    if (1 == opt[op].kind) {
      if (!opt[op].flag) {
        if (err)
          sprintf(err, "%s!!!!!! flags must have flags", ME);
        else
          fprintf(stderr, "%s: panic 13\n", me);
        return 1;
      }
    }
    else {
      if (!opt[op].name) {
        if (err)
          sprintf(err, "%s!!!!!! opt[%d] isn't a flag: must have \"name\"",
                  ME, op);
        else
          fprintf(stderr, "%s: panic 14\n", me);
        return 1;
      }
    }
    if (4 == opt[op].kind && !opt[op].dflt) {
      if (err)
        sprintf(err, "%s!!!!!! opt[%d] is single variable parameter, but "
                "no default set", ME, op);
      else
        fprintf(stderr, "%s: panic 15\n", me);
      return 1;
    }
    numvar += ((int)opt[op].min < _hestMax(opt[op].max) && (NULL == opt[op].flag)); /* HEY scrutinize casts */
  }
  if (numvar > 1) {
    if (err)
      sprintf(err, "%s!!!!!! can't have %d unflagged min<max opts, only one", 
              ME, numvar);
    else
      fprintf(stderr, "%s: panic 16\n", me);
    return 1;
  }
  return 0;
}

int
_hestErrStrlen(hestOpt *opt, int argc, char **argv) {
  int a, numOpts, ret, other;

  ret = 0;
  numOpts = _hestNumOpts(opt);
  other = AIR_FALSE;
  if (argv) {
    for (a=0; a<argc; a++) {
      ret = AIR_MAX(ret, (int)airStrlen(argv[a]));
    }
  }
  for (a=0; a<numOpts; a++) {
    ret = AIR_MAX(ret, (int)airStrlen(opt[a].flag));
    ret = AIR_MAX(ret, (int)airStrlen(opt[a].name));
    other |= opt[a].type == airTypeOther;
  }
  for (a=airTypeUnknown+1; a<airTypeLast; a++) {
    ret = AIR_MAX(ret, (int)airStrlen(airTypeStr[a]));
  }
  if (other) {
    /* the callback's error() function may sprintf an error message
       into a buffer which is size AIR_STRLEN_HUGE */
    ret += AIR_STRLEN_HUGE;
  }
  ret += 4 * 12;  /* as many as 4 ints per error message */
  ret += 257;     /* function name and text of hest's error message */

  return ret;
}

/*
** _hestExtractFlagged()
**
** extracts the parameters associated with all flagged options from the
** given argc and argv, storing them in prms[], recording the number
** of parameters in nprm[], and whether or not the flagged option appeared
** in appr[].
**
** The "saw" information is not set here, since it is better set
** at value parsing time, which happens after defaults are enstated.
*/
int
_hestExtractFlagged(char **prms, unsigned int *nprm, int *appr,
                     int *argcP, char **argv, 
                     hestOpt *opt,
                     char *err, hestParm *parm, airArray *pmop) {
  char me[]="_hestExtractFlagged: ", ident1[AIR_STRLEN_HUGE],
    ident2[AIR_STRLEN_HUGE];
  int a, np, flag, endflag, numOpts, op;

  a = 0;
  if (parm->verbosity) 
    printf("!%s: *argcP = %d\n", me, *argcP);
  while (a<=*argcP-1) {
    if (parm->verbosity) 
      printf("!%s: a = %d -> argv[a] = %s\n", me, a, argv[a]);
    flag = _hestWhichFlag(opt, argv[a], parm);
    if (parm->verbosity) 
      printf("!%s: A: a = %d -> flag = %d\n", me, a, flag);
    if (!(0 <= flag)) {
      /* not a flag, move on */
      a++;
      continue;
    }
    /* see if we can associate some parameters with the flag */
    np = 0;
    endflag = 0;
    while (np < _hestMax(opt[flag].max) &&
           a+np+1 <= *argcP-1 &&
           -1 == (endflag = _hestWhichFlag(opt, argv[a+np+1], parm))) {
      np++;
      if (parm->verbosity)
        printf("!%s: np --> %d with endflag = %d\n", me, np, endflag);
    }
    /* we stopped because we got the max number of parameters, or
       because we hit the end of the command line, or
       because _hestWhichFlag() returned something other than -1,
       which means it returned -2, or a valid option index.  If
       we stopped because of _hestWhichFlag()'s return value, 
       endflag has been set to that return value */
    if (parm->verbosity)
      printf("!%s: B: np = %d; endflag = %d\n", me, np, endflag); 
    if (np < (int)opt[flag].min) { /* HEY scrutinize casts */
      /* didn't get minimum number of parameters */
      if (!( a+np+1 <= *argcP-1 )) {
        sprintf(err, "%shit end of line before getting %d parameter%s "
                "for %s (got %d)",
                ME, opt[flag].min, opt[flag].min > 1 ? "s" : "",
                _hestIdent(ident1, opt+flag, parm, AIR_TRUE), np);
      }
      else {
        sprintf(err, "%shit %s before getting %d parameter%s for %s (got %d)",
                ME, _hestIdent(ident1, opt+endflag, parm, AIR_FALSE),
                opt[flag].min, opt[flag].min > 1 ? "s" : "",
                _hestIdent(ident2, opt+flag, parm, AIR_FALSE), np);
      }
      return 1;
    }
    nprm[flag] = np;
    if (parm->verbosity) {
      printf("!%s:________ a=%d, *argcP = %d -> flag = %d\n", 
             me, a, *argcP, flag);
      _hestPrintArgv(*argcP, argv);
    }
    /* lose the flag argument */
    free(_hestExtract(argcP, argv, a, 1));
    /* extract the args after the flag */
    if (appr[flag]) {
      airMopSub(pmop, prms[flag], airFree);
      prms[flag] = (char *)airFree(prms[flag]);
    }
    prms[flag] = _hestExtract(argcP, argv, a, nprm[flag]);
    airMopAdd(pmop, prms[flag], airFree, airMopAlways);
    appr[flag] = AIR_TRUE;
    if (-2 == endflag) {
      /* we should lose the end-of-variable-parameter marker */
      free(_hestExtract(argcP, argv, a, 1));
    }
    if (parm->verbosity) {
      _hestPrintArgv(*argcP, argv);
      printf("!%s:^^^^^^^^ *argcP = %d\n", me, *argcP);
      printf("!%s: prms[%d] = %s\n", me, flag,
             prms[flag] ? prms[flag] : "(null)");
    }
  }

  /* make sure that flagged options without default were given */
  numOpts = _hestNumOpts(opt);
  for (op=0; op<numOpts; op++) {
    if (1 != opt[op].kind && opt[op].flag && !opt[op].dflt && !appr[op]) {
      sprintf(err, "%sdidn't get required %s",
              ME, _hestIdent(ident1, opt+op, parm, AIR_FALSE));
      return 1;
    }
  }

  return 0;
}

int
_hestNextUnflagged(int op, hestOpt *opt, int numOpts) {

  for(; op<=numOpts-1; op++) {
    if (!opt[op].flag)
      break;
  }
  return op;
}

int
_hestExtractUnflagged(char **prms, unsigned int *nprm,
                      int *argcP, char **argv, 
                      hestOpt *opt,
                      char *err, hestParm *parm, airArray *pmop) {
  char me[]="_hestExtractUnflagged: ", ident[AIR_STRLEN_HUGE];
  int nvp, np, op, unflag1st, unflagVar, numOpts;

  numOpts = _hestNumOpts(opt);
  unflag1st = _hestNextUnflagged(0, opt, numOpts);
  if (numOpts == unflag1st) {
    /* no unflagged options; we're done */
    return 0;
  }

  for (unflagVar = unflag1st; 
       unflagVar != numOpts; 
       unflagVar = _hestNextUnflagged(unflagVar+1, opt, numOpts)) {
    if ((int)opt[unflagVar].min < _hestMax(opt[unflagVar].max)) /* HEY scrutinize casts */
      break;
  }
  /* now, if there is a variable parameter unflagged opt, unflagVar is its
     index in opt[], or else unflagVar is numOpts */

  /* grab parameters for all unflagged opts before opt[t] */
  for (op = _hestNextUnflagged(0, opt, numOpts); 
       op < unflagVar; 
       op = _hestNextUnflagged(op+1, opt, numOpts)) {
    /* printf("!%s: op = %d; unflagVar = %d\n", me, op, unflagVar); */
    np = opt[op].min;  /* min == max */
    if (!(np <= *argcP)) {
      sprintf(err, "%sdon't have %d parameter%s %s%s%sfor %s", 
              ME, np, np > 1 ? "s" : "", 
              argv[0] ? "starting at \"" : "",
              argv[0] ? argv[0] : "",
              argv[0] ? "\" " : "",
              _hestIdent(ident, opt+op, parm, AIR_TRUE));
      return 1;
    }
    prms[op] = _hestExtract(argcP, argv, 0, np);
    airMopAdd(pmop, prms[op], airFree, airMopAlways);
    nprm[op] = np;
  }
  /*
  _hestPrintArgv(*argcP, argv);
  */
  /* we skip over the variable parameter unflagged option, subtract from *argcP
     the number of parameters in all the opts which follow it, in order to get
     the number of parameters in the sole variable parameter option, 
     store this in nvp */
  nvp = *argcP;
  for (op = _hestNextUnflagged(unflagVar+1, opt, numOpts); 
       op < numOpts; 
       op = _hestNextUnflagged(op+1, opt, numOpts)) {
    nvp -= opt[op].min;  /* min == max */
  }
  if (nvp < 0) {
    op = _hestNextUnflagged(unflagVar+1, opt, numOpts);
    np = opt[op].min;
    sprintf(err, "%sdon't have %d parameter%s for %s", 
            ME, np, np > 1 ? "s" : "", 
            _hestIdent(ident, opt+op, parm, AIR_FALSE));
    return 1;
  }
  /* else we had enough args for all the unflagged options following
     the sole variable parameter unflagged option, so snarf them up */
  for (op = _hestNextUnflagged(unflagVar+1, opt, numOpts); 
       op < numOpts; 
       op = _hestNextUnflagged(op+1, opt, numOpts)) {
    np = opt[op].min;
    prms[op] = _hestExtract(argcP, argv, nvp, np);
    airMopAdd(pmop, prms[op], airFree, airMopAlways);
    nprm[op] = np;
  }

  /* now we grab the parameters of the sole variable parameter unflagged opt,
     if it exists (unflagVar < numOpts) */
  if (unflagVar < numOpts) {
    /*
    printf("!%s: unflagVar=%d: min, nvp, max = %d %d %d\n", me, unflagVar,
           opt[unflagVar].min, nvp, _hestMax(opt[unflagVar].max));
    */
    /* we'll do error checking for unexpected args later */
    nvp = AIR_MIN(nvp, _hestMax(opt[unflagVar].max));
    if (nvp < (int)opt[unflagVar].min) { /* HEY scrutinize casts */
      sprintf(err, "%sdidn't get minimum of %d arg%s for %s (got %d)",
              ME, opt[unflagVar].min, 
              opt[unflagVar].min > 1 ? "s" : "",
              _hestIdent(ident, opt+unflagVar, parm, AIR_TRUE), nvp);
      return 1;
    }
    if (nvp) {
      prms[unflagVar] = _hestExtract(argcP, argv, 0, nvp);
      airMopAdd(pmop, prms[unflagVar], airFree, airMopAlways);
      nprm[unflagVar] = nvp;
    }
    else {
      prms[unflagVar] = NULL;
      nprm[unflagVar] = 0;
    }
  }
  return 0;
}

int
_hestDefaults(char **prms, int *udflt, unsigned int *nprm, int *appr, 
              hestOpt *opt,
              char *err, hestParm *parm, airArray *mop) {
  char *tmpS, me[]="_hestDefaults: ", ident[AIR_STRLEN_HUGE];
  int op, numOpts;

  numOpts = _hestNumOpts(opt);
  for (op=0; op<numOpts; op++) {
    if (parm->verbosity) 
      printf("%s op=%d/%d: \"%s\" --> kind=%d, nprm=%u, appr=%d\n",
             me, op, numOpts-1, prms[op], opt[op].kind,
             nprm[op], appr[op]);
    switch(opt[op].kind) {
    case 1:
      /* -------- (no-parameter) boolean flags -------- */
      /* default is always ignored */
      udflt[op] = 0;
      break;
    case 2:
    case 3:
      /* -------- one required parameter -------- */
      /* -------- multiple required parameters -------- */
      /* we'll used defaults if the flag didn't appear */
      udflt[op] = opt[op].flag && !appr[op];
      break;
    case 4:
      /* -------- optional single variables -------- */
      /* if the flag appeared (if there is a flag) but the paramter didn't,
         we'll "invert" the default; if the flag didn't appear (or if there
         isn't a flag) and the parameter also didn't appear, we'll use the
         default.  In either case, nprm[op] will be zero, and in both cases,
         we need to use the default information. */
      udflt[op] = (0 == nprm[op]);
      /*
      fprintf(stderr, "%s nprm[%d] = %u --> udflt[%d] = %d\n", me,
              op, nprm[op], op, udflt[op]);
      */
      break;
    case 5:
      /* -------- multiple optional parameters -------- */
      /* we'll use the default if the flag didn't appear (if there is a
         flag) Otherwise, if nprm[op] is zero, then the user is saying,
         I want zero parameters */
      udflt[op] = opt[op].flag && !appr[op];
      break;
    }
    if (!udflt[op])
      continue;
    prms[op] = airStrdup(opt[op].dflt);
    /* fprintf(stderr, "%s: prms[%d] = |%s|\n", me, op, prms[op]); */
    if (prms[op]) {
      airMopAdd(mop, prms[op], airFree, airMopAlways);
      airOneLinify(prms[op]);
      tmpS = airStrdup(prms[op]);
      nprm[op] = airStrntok(tmpS, " ");
      tmpS = (char *)airFree(tmpS);
      /* printf("!%s: nprm[%d] in default = %u\n", me, op, nprm[op]); */
      if ((int)opt[op].min < _hestMax(opt[op].max)) { /* HEY scrutinize casts */
        if (!( AIR_IN_CL((int)opt[op].min, (int)nprm[op], _hestMax(opt[op].max)) /* HEY scrutinize casts */
               || (airTypeString == opt[op].type 
                   && parm->elideMultipleEmptyStringDefault) )) {
          sprintf(err, "%s# parameters (in default) for %s is %d, "
                  "but need between %d and %d", 
                  ME, _hestIdent(ident, opt+op, parm, AIR_TRUE), nprm[op],
                  opt[op].min, _hestMax(opt[op].max));
          return 1;
        }
      }
    }
  }
  return 0;
}

int
_hestSetValues(char **prms, int *udflt, unsigned int *nprm, int *appr,
               hestOpt *opt,
               char *err, hestParm *parm, airArray *pmop) {
  char ident[AIR_STRLEN_HUGE], me[]="_hestSetValues: ",
    cberr[AIR_STRLEN_HUGE], *tok, *last, *prmsCopy;
  double tmpD;
  int op, type, numOpts, p, ret;
  void *vP;
  char *cP;
  size_t size;

  numOpts = _hestNumOpts(opt);
  for (op=0; op<numOpts; op++) {
    _hestIdent(ident, opt+op, parm, AIR_TRUE);
    type = opt[op].type;
    size = (airTypeEnum == type
            ? (int)sizeof(int)             /* HEY scrutinize casts */
            : (airTypeOther == type
               ? (int)opt[op].CB->size     /* HEY scrutinize casts */
               : (int)airTypeSize[type])); /* HEY scrutinize casts */
    cP = (char *)(vP = opt[op].valueP);
    if (parm->verbosity) {
      printf("%s %d of %d: \"%s\": |%s| --> kind=%d, type=%d, size=%d\n", 
             me, op, numOpts-1, prms[op], ident, opt[op].kind, type,
             (int)size);
    }
    /* we may over-write these */
    opt[op].alloc = 0;
    if (opt[op].sawP) {
      *(opt[op].sawP) = 0;
    }
    switch(opt[op].kind) {
    case 1:
      /* -------- parameter-less boolean flags -------- */
      if (vP)
        *((int*)vP) = appr[op];
      break;
    case 2:
      /* -------- one required parameter -------- */
      if (prms[op] && vP) {
        switch (type) {
        case airTypeEnum:
          if (1 != airParseStrE((int *)vP, prms[op], " ", 1, opt[op].enm)) {
            sprintf(err, "%scouldn\'t parse %s\"%s\" as %s for %s",
                    ME, udflt[op] ? "(default) " : "", prms[op],
                    opt[op].enm->name, ident);
            return 1;
          }
          break;
        case airTypeOther:
          strcpy(cberr, "");
          ret = opt[op].CB->parse(vP, prms[op], cberr);
          if (ret) {
            if (strlen(cberr))
              sprintf(err, "%serror parsing \"%s\" as %s for %s:\n%s", 
                      ME, prms[op], opt[op].CB->type, ident, cberr);
            else 
              sprintf(err, "%serror parsing \"%s\" as %s for %s: returned %d", 
                      ME, prms[op], opt[op].CB->type, ident, ret);
            return ret;
          }
          if (opt[op].CB->destroy) {
            /* vP is the address of a void*, we manage the void * */
            opt[op].alloc = 1;
            airMopAdd(pmop, (void**)vP, (airMopper)airSetNull, airMopOnError);
            airMopAdd(pmop, *((void**)vP), opt[op].CB->destroy, airMopOnError);
          }
          break;
        case airTypeString:
          if (1 != airParseStrS((char **)vP, prms[op], " ", 1,
                                parm->greedySingleString)) {
            sprintf(err, "%scouldn't parse %s\"%s\" as %s for %s", 
                    ME, udflt[op] ? "(default) " : "", prms[op],
                    airTypeStr[type], ident);
            return 1;
          }
          /* vP is the address of a char* (a char **), but what we
             manage with airMop is the char * */
          opt[op].alloc = 1;
          airMopMem(pmop, vP, airMopOnError);
          break;
        default:
          /* type isn't string or enum, so no last arg to airParseStr[type] */
          if (1 != airParseStr[type](vP, prms[op], " ", 1)) {
            sprintf(err, "%scouldn't parse %s\"%s\" as %s for %s", 
                    ME, udflt[op] ? "(default) " : "", prms[op],
                    airTypeStr[type], ident);
            return 1;
          }
          break;
        }
      }
      break;
    case 3:
      /* -------- multiple required parameters -------- */
      if (prms[op] && vP) {
        switch (type) {
        case airTypeEnum:
          if (opt[op].min !=   /* min == max */
              airParseStrE((int *)vP, prms[op], " ", opt[op].min, opt[op].enm)) {
            sprintf(err, "%scouldn't parse %s\"%s\" as %d %s%s for %s",
                    ME, udflt[op] ? "(default) " : "", prms[op],
                    opt[op].min, opt[op].enm->name,
                    opt[op].min > 1 ? "s" : "", ident);
            return 1;
          }
          break;
        case airTypeOther:
          prmsCopy = airStrdup(prms[op]);
          for (p=0; p<(int)opt[op].min; p++) { /* HEY scrutinize casts */
            tok = airStrtok(!p ? prmsCopy : NULL, " ", &last);
            strcpy(cberr, "");
            ret = opt[op].CB->parse(cP + p*size, tok, cberr);
            if (ret) {
              if (strlen(cberr))
                sprintf(err, "%serror parsing \"%s\" (in \"%s\") as %s "
                        "for %s:\n%s", 
                        ME, tok, prms[op], opt[op].CB->type, ident, cberr);
              else 
                sprintf(err, "%serror parsing \"%s\" (in \"%s\") as %s "
                        "for %s: returned %d", 
                        ME, tok, prms[op], opt[op].CB->type, ident, ret);
              free(prmsCopy);
              return 1;
            }
          }
          free(prmsCopy);
          if (opt[op].CB->destroy) {
            /* vP is an array of void*s, we manage the individual void*s */
            opt[op].alloc = 2;
            for (p=0; p<(int)opt[op].min; p++) { /* HEY scrutinize casts */
              airMopAdd(pmop, ((void**)vP)+p, (airMopper)airSetNull,
                        airMopOnError);
              airMopAdd(pmop, *(((void**)vP)+p), opt[op].CB->destroy,
                        airMopOnError);
            }
          }
          break;
        case airTypeString:
          if (opt[op].min !=   /* min == max */
              airParseStr[type](vP, prms[op], " ", opt[op].min, AIR_FALSE)) {
            sprintf(err, "%scouldn't parse %s\"%s\" as %d %s%s for %s",
                    ME, udflt[op] ? "(default) " : "", prms[op],
                    opt[op].min, airTypeStr[type], 
                    opt[op].min > 1 ? "s" : "", ident);
            return 1;
          }
          /* vP is an array of char*s, (a char**), and what we manage
             with airMop are the individual vP[p]. */
          opt[op].alloc = 2;
          for (p=0; p<(int)opt[op].min; p++) { /* HEY scrutinize casts */
            airMopMem(pmop, &(((char**)vP)[p]), airMopOnError);
          }
          break;
        default:
          if (opt[op].min !=   /* min == max */
              airParseStr[type](vP, prms[op], " ", opt[op].min)) {
            sprintf(err, "%scouldn't parse %s\"%s\" as %d %s%s for %s",
                    ME, udflt[op] ? "(default) " : "", prms[op],
                    opt[op].min, airTypeStr[type], 
                    opt[op].min > 1 ? "s" : "", ident);
            return 1;
          }
          break;
        }
      }
      break;
    case 4:
      /* -------- optional single variables -------- */
      if (prms[op] && vP) {
        switch (type) {
        case airTypeEnum:
          if (1 != airParseStrE((int *)vP, prms[op], " ", 1, opt[op].enm)) {
            sprintf(err, "%scouldn't parse %s\"%s\" as %s for %s",
                    ME, udflt[op] ? "(default) " : "", prms[op],
                    opt[op].enm->name, ident);
            return 1;
          }
          break;
        case airTypeOther:
          /* we're parsing an "other".  We will not perform the special
             flagged single variable parameter games as done above, so
             whether this option is flagged or unflagged, we're going
             to treat it like an unflagged single variable parameter option:
             if the parameter didn't appear, we'll parse it from the default,
             if it did appear, we'll parse it from the command line.  Setting
             up prms[op] thusly has already been done by _hestDefaults() */
          strcpy(cberr, "");
          ret = opt[op].CB->parse(vP, prms[op], cberr);
          if (ret) {
            if (strlen(cberr))
              sprintf(err, "%serror parsing \"%s\" as %s for %s:\n%s", 
                      ME, prms[op], opt[op].CB->type, ident, cberr);
            else 
              sprintf(err, "%serror parsing \"%s\" as %s for %s: returned %d", 
                      ME, prms[op], opt[op].CB->type, ident, ret);
            return 1;
          }
          if (opt[op].CB->destroy) {
            /* vP is the address of a void*, we manage the void* */
            opt[op].alloc = 1;
            airMopAdd(pmop, vP, (airMopper)airSetNull, airMopOnError);
            airMopAdd(pmop, *((void**)vP), opt[op].CB->destroy, airMopOnError);
          }
          break;
        case airTypeString:
          if (1 != airParseStr[type](vP, prms[op], " ", 1,
                                     parm->greedySingleString)) {
            sprintf(err, "%scouldn't parse %s\"%s\" as %s for %s",
                    ME, udflt[op] ? "(default) " : "", prms[op],
                    airTypeStr[type], ident);
            return 1;
          }
          opt[op].alloc = 1;
          if (opt[op].flag && 1 == _hestCase(opt, udflt, nprm, appr, op)) {
            /* we just parsed the default, but now we want to "invert" it */
            *((char**)vP) = (char *)airFree(*((char**)vP));
            opt[op].alloc = 0;
          }
          /* vP is the address of a char* (a char**), and what we
             manage with airMop is the char * */
          airMopMem(pmop, vP, airMopOnError);
          break;
        default:
          if (1 != airParseStr[type](vP, prms[op], " ", 1)) {
            sprintf(err, "%scouldn't parse %s\"%s\" as %s for %s",
                    ME, udflt[op] ? "(default) " : "", prms[op],
                    airTypeStr[type], ident);
            return 1;
          }
          opt[op].alloc = 0;
          if (1 == _hestCase(opt, udflt, nprm, appr, op)) {
            /* we just parsed the default, but now we want to "invert" it */
            tmpD = airDLoad(vP, type);
            airIStore(vP, type, tmpD ? 0 : 1);
          }
          break;
        }
      }
      break;
    case 5:
      /* -------- multiple optional parameters -------- */
      /* hammerhead problems in this case */
      if (prms[op] && vP) {
        if (1 == _hestCase(opt, udflt, nprm, appr, op)) {
          *((void**)vP) = NULL;
          /* alloc and sawP set above */
        } else {
          if (airTypeString == type) {
            /* this is sneakiness: we allocate one more element so that
               the resulting char** is, like argv, NULL-terminated */
            *((void**)vP) = calloc(nprm[op]+1, size);
          } else {
            *((void**)vP) = calloc(nprm[op], size);
          }
          if (parm->verbosity) {
            printf("!%s: nprm[%d] = %u\n", me, op, nprm[op]);
            printf("!%s: new array is at 0x%p\n", me, *((void**)vP));
          }
          airMopMem(pmop, vP, airMopOnError);
          *(opt[op].sawP) = nprm[op];
          /* so far everything we've done is regardless of type */
          switch (type) {
          case airTypeEnum:
            opt[op].alloc = 1;
            if (nprm[op] != 
                airParseStrE((int *)(*((void**)vP)), prms[op], " ", nprm[op], 
                             opt[op].enm)) {
              sprintf(err, "%scouldn't parse %s\"%s\" as %u %s%s for %s",
                      ME, udflt[op] ? "(default) " : "", prms[op],
                      nprm[op], opt[op].enm->name,
                      nprm[op] > 1 ? "s" : "", ident);
              return 1;
            }
            break;
          case airTypeOther:
            cP = (char *)(*((void**)vP));
            prmsCopy = airStrdup(prms[op]);
            opt[op].alloc = (opt[op].CB->destroy ? 3 : 1);
            for (p=0; p<(int)nprm[op]; p++) {  /* HEY scrutinize casts */
              tok = airStrtok(!p ? prmsCopy : NULL, " ", &last);
              /* hammerhead problems went away when this line
                 was replaced by the following one:
                 strcpy(cberr, "");
              */
              cberr[0] = 0;
              ret = opt[op].CB->parse(cP + p*size, tok, cberr);
              if (ret) {
                if (strlen(cberr))
                  sprintf(err,"%serror parsing \"%s\" (in \"%s\") as %s "
                          "for %s:\n%s", 
                          ME, tok, prms[op], opt[op].CB->type, ident, cberr);

                else 
                  sprintf(err, "%serror parsing \"%s\" (in \"%s\") as %s "
                          "for %s: returned %d", 
                          ME, tok, prms[op], opt[op].CB->type, ident, ret);
                free(prmsCopy);
                return 1;
              }
            }
            free(prmsCopy);
            if (opt[op].CB->destroy) {
              for (p=0; p<(int)nprm[op]; p++) { /* HEY scrutinize casts */
                /* avert your eyes.  vP is the address of an array of void*s.
                   We manage the void*s */
                airMopAdd(pmop, (*((void***)vP))+p, (airMopper)airSetNull,
                          airMopOnError);
                airMopAdd(pmop, *((*((void***)vP))+p), opt[op].CB->destroy,
                          airMopOnError);
              }
            }
            break;
          case airTypeString:
            opt[op].alloc = 3;
            if (nprm[op] != 
                airParseStrS((char **)(*((void**)vP)), prms[op], " ", nprm[op],
                             AIR_FALSE)) {
              sprintf(err, "%scouldn't parse %s\"%s\" as %d %s%s for %s",
                      ME, udflt[op] ? "(default) " : "", prms[op],
                      nprm[op], airTypeStr[type], 
                      nprm[op] > 1 ? "s" : "", ident);
              return 1;
            }
            /* vP is the address of an array of char*s (a char ***), and
               what we manage with airMop is the individual (*vP)[p],
               as well as vP itself (above). */
            for (p=0; p<(int)nprm[op]; p++) { /* HEY scrutinize casts */
              airMopAdd(pmop, (*((char***)vP))[p], airFree, airMopOnError);
            }
            /* do the NULL-termination described above */
            (*((char***)vP))[nprm[op]] = NULL;
            break;
          default:
            opt[op].alloc = 1;
            if (nprm[op] != 
                airParseStr[type](*((void**)vP), prms[op], " ", nprm[op])) {
              sprintf(err, "%scouldn't parse %s\"%s\" as %d %s%s for %s",
                      ME, udflt[op] ? "(default) " : "", prms[op],
                      nprm[op], airTypeStr[type], 
                      nprm[op] > 1 ? "s" : "", ident);
              return 1;
            }
            break;
          }
        }
      }
      break;
    }
  }
  return 0;
}

/*
******** hestParse()
**
** documentation?
*/
int
hestParse(hestOpt *opt, int _argc, char **_argv,
          char **_errP, hestParm *_parm) {
  char me[]="hestParse: ";
  char **argv, **prms, *err;
  int a, argc, argr, *appr, *udflt, nrf, numOpts, big, ret;
  unsigned int *nprm;
  airArray *mop;
  hestParm *parm;

  numOpts = _hestNumOpts(opt);

  /* -------- initialize the mop! */
  mop = airMopNew();

  /* -------- either copy given _parm, or allocate one */
  if (_parm) {
    parm = _parm;
  }
  else {
    parm = hestParmNew();
    airMopAdd(mop, parm, (airMopper)hestParmFree, airMopAlways);
  }

  /* -------- allocate the err string.  To determine its size with total
     ridiculous safety we have to find the biggest things which can appear
     in the string. */
  big = _hestErrStrlen(opt, _argc, _argv);
  if (!(err = (char *)calloc(big, sizeof(char)))) {
    fprintf(stderr, "%s PANIC: couldn't allocate error message "
            "buffer (size %d)\n", me, big);
    exit(1);
  }
  if (_errP) {
    /* if they care about the error string, than it is mopped only
       when there _wasn't_ an error */
    *_errP = err;
    airMopAdd(mop, _errP, (airMopper)airSetNull, airMopOnOkay);
    airMopAdd(mop, err, airFree, airMopOnOkay);
  }
  else {
    /* otherwise, we're making the error string just for our own
       convenience, and we'll always clean it up on exit */
    airMopAdd(mop, err, airFree, airMopAlways);
  }

  /* -------- check on validity of the hestOpt array */
  if (_hestPanic(opt, err, parm)) {
    airMopError(mop); return 1;
  }

  /* -------- Create all the local arrays used to save state during
     the processing of all the different options */
  nprm = (unsigned int *)calloc(numOpts, sizeof(unsigned int));
  airMopMem(mop, &nprm, airMopAlways);
  appr = (int *)calloc(numOpts, sizeof(int));
  airMopMem(mop, &appr, airMopAlways);
  udflt = (int *)calloc(numOpts, sizeof(int));
  airMopMem(mop, &udflt, airMopAlways);
  prms = (char **)calloc(numOpts, sizeof(char*));
  airMopMem(mop, &prms, airMopAlways);
  for (a=0; a<numOpts; a++) {
    prms[a] = NULL;
  }

  /* -------- find out how big the argv array needs to be, first
     by seeing how many args are in the response files, and then adding
     on the args from the actual argv (getting this right the first time
     greatly simplifies the problem of eliminating memory leaks) */
  if (_hestArgsInResponseFiles(&argr, &nrf, _argv, err, parm)) {
    airMopError(mop); return 1;
  }
  argc = argr + _argc - nrf;

  if (parm->verbosity) {
    printf("!%s: nrf = %d; argr = %d; _argc = %d --> argc = %d\n", 
           me, nrf, argr, _argc, argc);
  }
  argv = (char **)calloc(argc+1, sizeof(char*));
  airMopMem(mop, &argv, airMopAlways);

  /* -------- process response files (if any) and set the remaining
     elements of argv */
  if (parm->verbosity) printf("%s: #### calling hestResponseFiles\n", me);
  if (_hestResponseFiles(argv, _argv, parm, mop)) {
    airMopError(mop); return 1;
  }
  if (parm->verbosity) printf("%s: #### hestResponseFiles done!\n", me);
  /*
  _hestPrintArgv(argc, argv);
  */

  /* -------- extract flags and their associated parameters from argv */
  if (parm->verbosity) printf("%s: #### calling hestExtractFlagged\n", me);
  if (_hestExtractFlagged(prms, nprm, appr, 
                           &argc, argv, 
                           opt,
                           err, parm, mop)) {
    airMopError(mop); return 1;
  }
  if (parm->verbosity) printf("%s: #### hestExtractFlagged done!\n", me);
  /*
  _hestPrintArgv(argc, argv);
  */

  /* -------- extract args for unflagged options */
  if (parm->verbosity) printf("%s: #### calling hestExtractUnflagged\n", me);
  if (_hestExtractUnflagged(prms, nprm,
                            &argc, argv,
                            opt,
                            err, parm, mop)) {
    airMopError(mop); return 1;
  }
  if (parm->verbosity) printf("%s: #### hestExtractUnflagged done!\n", me);

  /* currently, any left over arguments indicate error */
  if (argc) {
    sprintf(err, "%sunexpected arg%s: \"%s\"", ME,
            ('-' == argv[0][0]
             ? " (or unrecognized flag)"
             : ""), argv[0]);
    airMopError(mop); return 1;
  }

  /* -------- learn defaults */
  if (parm->verbosity) printf("%s: #### calling hestDefaults\n", me);
  if (_hestDefaults(prms, udflt, nprm, appr,
                    opt,
                    err, parm, mop)) {
    airMopError(mop); return 1;
  }
  if (parm->verbosity) printf("%s: #### hestDefaults done!\n", me);
  
  /* -------- now, the actual parsing of values */
  /* hammerhead problems in _hestSetValues */
  if (parm->verbosity) printf("%s: #### calling hestSetValues\n", me);
  ret = _hestSetValues(prms, udflt, nprm, appr,
                       opt,
                       err, parm, mop);
  if (ret) {
    airMopError(mop); return ret;
  }

  if (parm->verbosity) printf("%s: #### hestSetValues done!\n", me);

  airMopOkay(mop);
  return 0;
}

/*
******** hestParseFree()
**
** free()s whatever was allocated by hestParse()
** 
** returns NULL only to facilitate use with the airMop functions.
** You should probably just ignore this quirk.
*/
void *
hestParseFree(hestOpt *opt) {
  int op, i, numOpts;
  unsigned int ui;
  void **vP;
  void ***vAP;
  char **str;
  char ***strP;

  numOpts = _hestNumOpts(opt);
  for (op=0; op<numOpts; op++) {
    /*
    printf("!hestParseFree: op = %d/%d -> kind = %d; type = %d; alloc = %d\n", 
           op, numOpts-1, opt[op].kind, opt[op].type, opt[op].alloc);
    */
    vP = (void **)opt[op].valueP;
    vAP = (void ***)opt[op].valueP;
    str = (char **)opt[op].valueP;
    strP = (char ***)opt[op].valueP;
    switch (opt[op].alloc) {
    case 0:
      /* nothing was allocated */
      break;
    case 1:
      if (airTypeOther != opt[op].type) {
        *vP = airFree(*vP);
      }
      else {
        /* alloc is one either because we parsed one thing, and we have a
           destroy callback, or, because we parsed a dynamically-created array
           of things, and we don't have a destroy callback */
        if (opt[op].CB->destroy) {
          *vP = opt[op].CB->destroy(*vP);
        }
        else {
          *vP = airFree(*vP);
        }
      }
      break;
    case 2:
      if (airTypeString == opt[op].type) {
        for (i=0; i<(int)opt[op].min; i++) { /* HEY scrutinize casts */
          str[i] = (char *)airFree(str[i]);
        }
      }
      else {
        for (i=0; i<(int)opt[op].min; i++) { /* HEY scrutinize casts */
          vP[i] = opt[op].CB->destroy(vP[i]);
        }
      }
      break;
    case 3:
      if (airTypeString == opt[op].type) {
        for (ui=0; ui<*(opt[op].sawP); ui++) {
          (*strP)[ui] = (char *)airFree((*strP)[ui]);
        }
        *strP = (char **)airFree(*strP);
      }
      else {
        for (ui=0; ui<*(opt[op].sawP); ui++) {
          (*vAP)[ui] = opt[op].CB->destroy((*vAP)[ui]);
        }
        *vAP = (void **)airFree(*vAP);
      }
      break;
    }
  }
  return NULL;
}

/*
******** hestParseOrDie()
**
** dumb little function which encapsulate a common usage of hest:
** first, make sure hestOpt is valid with hestOptCheck().  Then,
** if argc is 0: maybe show info, usage, and glossary, all according
**    to the given boolean flags, then exit(1)
** if parsing failed: show error message, and maybe usage and glossary,
**    again according to boolean flags, then exit(1)
** if parsing succeeded: return
*/
void
hestParseOrDie(hestOpt *opt, int argc, char **argv, hestParm *parm,
               char *me, char *info,
               int doInfo, int doUsage, int doGlossary) {
  int E;
  char *errS;

  if (opt) {
    if (hestOptCheck(opt, &errS)) {
      fprintf(stderr, "ERROR in hest usage:\n%s\n", errS); free(errS);
      exit(1);
    }
    E = 0;
    if ( (!argc) ||
         (E = hestParse(opt, argc, argv, &errS, parm)) ) {
      if (E) {
        fprintf(stderr, "ERROR: %s\n", errS); free(errS);
      } else {
        if (doInfo && info) hestInfo(stderr, me?me:"", info, parm);
      }
      if (doUsage) hestUsage(stderr, opt, me?me:"", parm);
      if (doGlossary) hestGlossary(stderr, opt, parm);
      hestParmFree(parm);
      hestOptFree(opt);
      exit(1);
    }
  }
  
  return;
}
