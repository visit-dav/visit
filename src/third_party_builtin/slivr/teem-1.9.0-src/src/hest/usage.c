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

/*
** don't ask
*/
void
_hestSetBuff(char *B, hestOpt *O, hestParm *P, int showshort, int showlong) {
  char copy[AIR_STRLEN_HUGE], *sep;
  int max, len;

  max = _hestMax(O->max);
  if (O->flag) {
    strcpy(copy, O->flag);
    if ((sep = strchr(copy, P->multiFlagSep))) {
      *sep = 0;
      if (showshort) {
        strcat(B, "-");
        strcat(B, copy);
      }
      if (showlong) {
        if (showshort) {
          len = strlen(B);
          B[len] = P->multiFlagSep;
          B[len+1] = '\0';
        }
        strcat(B, "--");
        strcat(B, sep+1);
      }
    } else {
      strcat(B, "-");
      strcat(B, O->flag);
    }
    if (O->min || max) {
      strcat(B, "\t");
    }
  }
  if (!O->min && max) {
    strcat(B, "[");
  }
  if (O->min || max) {
    strcat(B, "<");
    strcat(B, O->name);
    if ((int)(O->min) < max && max > 1) { /* HEY scrutinize casts */
      strcat(B, "\t...");
    }
    strcat(B, ">");
  }
  if (!O->min && max) {
    strcat(B, "]");
  }
}

/* early version of _hestSetBuff() function */
#define SETBUFF(B, O) \
  strcat(B, O.flag ? "-" : ""), \
  strcat(B, O.flag ? O.flag : ""), \
  strcat(B, O.flag && (O.min || _hestMax(O.max)) ? "\t" : ""), \
  strcat(B, !O.min && _hestMax(O.max) ? "[" : ""), \
  strcat(B, O.min || _hestMax(O.max) ? "<" : ""), \
  strcat(B, O.min || _hestMax(O.max) ? O.name : ""), \
  strcat(B, (O.min < _hestMax(O.max) && (_hestMax(O.max) > 1)) ? " ...": ""), \
  strcat(B, O.min || _hestMax(O.max) ? ">" : ""), \
  strcat(B, !O.min && _hestMax(O.max) ? "]" : "");

/*
** _hestPrintStr()
**
** not a useful function.  Do not use.
*/
void
_hestPrintStr(FILE *f, int indent, int already, int width, const char *_str,
              int bslash) {
  char *str, *ws, *last;
  int nwrd, wrd, pos, s, newed=AIR_FALSE;

  str = airStrdup(_str);
  nwrd = airStrntok(str, " ");
  pos = already;
  for (wrd=0; wrd<nwrd; wrd++) {
    /* we used airStrtok() to delimit words on spaces ... */
    ws = airStrtok(!wrd ? str : NULL, " ", &last);
    /* ... but then convert tabs to spaces */
    airStrtrans(ws, '\t', ' ');
    if ((int)(pos + 1 + strlen(ws)) <= width - !!bslash) {
      /* if this word would still fit on the current line */
      if (wrd && !newed) fprintf(f, " ");
      fprintf(f, "%s", ws);
      pos += 1 + strlen(ws);
      newed = AIR_FALSE;
    } else {
      /* else we start a new line and print the indent */
      if (bslash) {
        fprintf(f, " \\");
      }
      fprintf(f, "\n");
      for (s=0; s<indent; s++) {
        fprintf(f, " ");
      }
      fprintf(f, "%s", ws); 
      pos = indent + strlen(ws);
    }
    /* if the last character of the word was a newline, then indent */
    if ('\n' == ws[strlen(ws)-1]) {
      for (s=0; s<indent; s++) {
        fprintf(f, " ");
      }
      pos = indent;
      newed = AIR_TRUE;
    } else {
      newed = AIR_FALSE;
    }
  }
  fprintf(f, "\n");
  free(str);
}

/*
******** hestMinNumArgs
**
** The idea is that this helps quickly determine if the options given
** on the command line are insufficient, in order to produce general
** usage information instead of some specific parse error.
**
** Because hest is strictly agnostic with respect to how many command-line
** arguments actually constitute the command itself ("rmdir": one argument,
** "cvs checkout": two arguments), it only concerns itself with the 
** command-line arguments following the command.
**
** Thus, hestMinMinArgs() returns the minimum number of command-line
** arguments (following the command) that could be valid.  If your 
** command is only one argument (like "rmdir"), then you might use
** the true argc passed by the OS to main() as such:
**
**   if (argc-1 < hestMinNumArgs(opt)) {
**     ... usage ...
**   }
**
** But if your command is two arguments (like "cvs checkout"):
**
**   if (argc-2 < hestMinNumArgs(opt)) {
**     ... usage ...
**   }
**
** HOWEVER! don't forget the response files can complicate all this:
** in one argument a response file can provide information for any
** number of arguments, and the argc itself is kind of meaningless.
** The code examples above only really apply when
** hparm->respFileEnable is false.  For example, in unrrdu (private.h)
** we find:
**
**   if ( (hparm->respFileEnable && !argc) ||
**        (!hparm->respFileEnable && argc < hestMinNumArgs(opt)) ) { 
**     ... usage ...
**   }
**
*/
int
hestMinNumArgs(hestOpt *opt) {
  hestParm *parm;
  int i, count, numOpts;

  parm = hestParmNew();
  if (_hestPanic(opt, NULL, parm)) {
    parm = hestParmFree(parm);
    return _hestMax(-1);
  }
  count = 0;
  numOpts = _hestNumOpts(opt);
  for (i=0; i<numOpts; i++) {
    if (!opt[i].dflt) {
      count += opt[i].min;
      if (!(0 == opt[i].min && 0 == opt[i].max)) {
        count += !!opt[i].flag;
      }
    }
  }  
  parm = hestParmFree(parm);
  return count;
}

void
hestInfo(FILE *file, const char *argv0, const char *info, hestParm *_parm) {
  hestParm *parm;

  parm = !_parm ? hestParmNew() : _parm;
  if (info) {
    fprintf(file, "\n%s: ", argv0);
    _hestPrintStr(file, 0, strlen(argv0) + 2, parm->columns, info, AIR_FALSE);
  }
  parm = !_parm ? hestParmFree(parm) : NULL;
}

void
hestUsage(FILE *f, hestOpt *opt, const char *argv0, hestParm *_parm) {
  int i, numOpts;
  char buff[2*AIR_STRLEN_HUGE], tmpS[AIR_STRLEN_HUGE];
  hestParm *parm;

  parm = !_parm ? hestParmNew() : _parm;

  if (_hestPanic(opt, NULL, parm)) {
    /* we can't continue; the opt array is botched */
    parm = !_parm ? hestParmFree(parm) : NULL;
    return;
  }
    
  numOpts = _hestNumOpts(opt);
  fprintf(f, "\n");
  strcpy(buff, "Usage: ");
  strcat(buff, argv0 ? argv0 : "");
  if (parm && parm->respFileEnable) {
    sprintf(tmpS, " [%cfile\t...]", parm->respFileFlag);
    strcat(buff, tmpS);
  }
  for (i=0; i<numOpts; i++) {
    strcat(buff, " ");
    if (1 == opt[i].kind || (opt[i].flag && opt[i].dflt))
      strcat(buff, "[");
    _hestSetBuff(buff, opt + i, parm, AIR_TRUE, AIR_TRUE);
    if (1 == opt[i].kind || (opt[i].flag && opt[i].dflt))
      strcat(buff, "]");
  }

  _hestPrintStr(f, strlen("Usage: "), 0, parm->columns, buff, AIR_TRUE);

  parm = !_parm ? hestParmFree(parm) : NULL;
  return;
}

void
hestGlossary(FILE *f, hestOpt *opt, hestParm *_parm) {
  int i, j, len, maxlen, numOpts;
  char buff[2*AIR_STRLEN_HUGE], tmpS[AIR_STRLEN_HUGE];
  hestParm *parm;

  parm = !_parm ? hestParmNew() : _parm;

  if (_hestPanic(opt, NULL, parm)) {
    /* we can't continue; the opt array is botched */
    parm = !_parm ? hestParmFree(parm) : NULL;
    return;
  }
    
  numOpts = _hestNumOpts(opt);

  maxlen = 0;
  if (numOpts) {
    fprintf(f, "\n");
  }
  for (i=0; i<numOpts; i++) {
    strcpy(buff, "");
    _hestSetBuff(buff, opt + i, parm, AIR_TRUE, AIR_FALSE);
    maxlen = AIR_MAX((int)strlen(buff), maxlen);
  }
  if (parm && parm->respFileEnable) {
    sprintf(buff, "%cfile ...", parm->respFileFlag);
    len = strlen(buff);
    for (j=len; j<maxlen; j++) {
      fprintf(f, " ");
    }
    fprintf(f, "%s = ", buff);
    strcpy(buff, "response file(s) containing command-line arguments");
    _hestPrintStr(f, maxlen + 3, maxlen + 3, parm->columns, buff, AIR_FALSE);
  }
  for (i=0; i<numOpts; i++) {
    strcpy(buff, "");
    _hestSetBuff(buff, opt + i, parm, AIR_TRUE, AIR_FALSE);
    airOneLinify(buff);
    len = strlen(buff);
    for (j=len; j<maxlen; j++) {
      fprintf(f, " ");
    }
    fprintf(f, "%s", buff);
    strcpy(buff, "");
#if 1
    if (opt[i].flag && strchr(opt[i].flag, parm->multiFlagSep)) {
      /* there is a long-form flag as well as short */
      _hestSetBuff(buff, opt + i, parm, AIR_FALSE, AIR_TRUE);
      strcat(buff, " = ");
      fprintf(f, " , ");
    } else {
      /* there is only a short-form flag */
      fprintf(f, " = ");
    }
#else
    fprintf(f, " = ");
#endif
    if (opt[i].info) {
      strcat(buff, opt[i].info);
    }
    if ((opt[i].min || _hestMax(opt[i].max))
        && (!( 2 == opt[i].kind
               && airTypeEnum == opt[i].type 
               && parm->elideSingleEnumType )) 
        && (!( 2 == opt[i].kind
               && airTypeOther == opt[i].type 
               && parm->elideSingleOtherType )) 
        ) {
      /* if there are newlines in the info, then we want to clarify the
         type by printing it on its own line */
      if (opt[i].info && strchr(opt[i].info, '\n')) {
        strcat(buff, "\n ");
      }
      else {
        strcat(buff, " ");
      }
      strcat(buff, "(");
      if (opt[i].min == 0 && _hestMax(opt[i].max) == 1) {
        strcat(buff, "optional\t");
      }
      else {
        if ((int)opt[i].min == _hestMax(opt[i].max) && _hestMax(opt[i].max) > 1) { /* HEY scrutinize casts */
          sprintf(tmpS, "%d\t", _hestMax(opt[i].max));
          strcat(buff, tmpS);
        }
        else if ((int)opt[i].min < _hestMax(opt[i].max)) { /* HEY scrutinize casts */
          if (-1 == opt[i].max) {
            sprintf(tmpS, "%d\tor\tmore\t", opt[i].min);
          }
          else {
            sprintf(tmpS, "%d..%d\t", opt[i].min, _hestMax(opt[i].max));
          }
          strcat(buff, tmpS);
        }
      }
      sprintf(tmpS, "%s%s", 
              (airTypeEnum == opt[i].type
               ? opt[i].enm->name
               : (airTypeOther == opt[i].type
                  ? opt[i].CB->type
                  : airTypeStr[opt[i].type])),
              (_hestMax(opt[i].max) > 1 
               ? (airTypeOther == opt[i].type
                  && 'y' == opt[i].CB->type[airStrlen(opt[i].CB->type)-1]
                  && parm->cleverPluralizeOtherY
                  ? "\bies" 
                  : "s")
               : ""));
      strcat(buff, tmpS);
      strcat(buff, ")");
    }
    /*
    fprintf(stderr, "!%s: parm->elideSingleOtherDefault = %d\n",
            "hestGlossary", parm->elideSingleOtherDefault);
    */
    if (opt[i].dflt 
        && (opt[i].min || _hestMax(opt[i].max))
        && (!( 2 == opt[i].kind
               && (airTypeFloat == opt[i].type || airTypeDouble == opt[i].type)
               && !AIR_EXISTS(airAtod(opt[i].dflt)) 
               && parm->elideSingleNonExistFloatDefault ))
        && (!( (3 == opt[i].kind || 5 == opt[i].kind) 
               && (airTypeFloat == opt[i].type || airTypeDouble == opt[i].type)
               && !AIR_EXISTS(airAtod(opt[i].dflt)) 
               && parm->elideMultipleNonExistFloatDefault ))
        && (!( 2 == opt[i].kind
               && airTypeOther == opt[i].type
               && parm->elideSingleOtherDefault ))
        && (!( 2 == opt[i].kind
               && airTypeString == opt[i].type
               && parm->elideSingleEmptyStringDefault 
               && 0 == airStrlen(opt[i].dflt) ))
        && (!( (3 == opt[i].kind || 5 == opt[i].kind) 
               && airTypeString == opt[i].type
               && parm->elideMultipleEmptyStringDefault 
               && 0 == airStrlen(opt[i].dflt) ))
        ) {
      /* if there are newlines in the info, then we want to clarify the
         default by printing it on its own line */
      if (opt[i].info && strchr(opt[i].info, '\n')) {
        strcat(buff, "\n ");
      }
      else {
        strcat(buff, "; ");
      }
      strcat(buff, "default:\t");
      strcpy(tmpS, opt[i].dflt);
      airStrtrans(tmpS, ' ', '\t');
      strcat(buff, "\"");
      strcat(buff, tmpS);
      strcat(buff, "\"");
    }
    _hestPrintStr(f, maxlen + 3, maxlen + 3, parm->columns, buff, AIR_FALSE);
  }
  parm = !_parm ? hestParmFree(parm) : NULL;

  return;
}

