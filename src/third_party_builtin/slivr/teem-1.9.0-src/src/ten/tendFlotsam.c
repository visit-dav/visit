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

/*
******** tendCmdList[]
**
** NULL-terminated array of unrrduCmd pointers, as ordered by
** TEN_MAP macro
*/
unrrduCmd *
tendCmdList[] = {
  TEND_MAP(TEND_LIST)
  NULL
};

/*
******** tendUsage
**
** prints out a little banner, and a listing of all available commands
** with their one-line descriptions
*/
void
tendUsage(char *me, hestParm *hparm) {
  int i, maxlen, len, c;
  char buff[AIR_STRLEN_LARGE], fmt[AIR_STRLEN_LARGE];

  maxlen = 0;
  for (i=0; tendCmdList[i]; i++) {
    maxlen = AIR_MAX(maxlen, (int)strlen(tendCmdList[i]->name));
  }

  sprintf(buff, "--- Diffusion Tensor Processing and Analysis ---");
  sprintf(fmt, "%%%ds\n",
          (int)((hparm->columns-strlen(buff))/2 + strlen(buff) - 1));
  fprintf(stderr, fmt, buff);
  
  for (i=0; tendCmdList[i]; i++) {
    len = strlen(tendCmdList[i]->name);
    strcpy(buff, "");
    for (c=len; c<maxlen; c++)
      strcat(buff, " ");
    strcat(buff, me);
    strcat(buff, " ");
    strcat(buff, tendCmdList[i]->name);
    strcat(buff, " ... ");
    len = strlen(buff);
    fprintf(stderr, "%s", buff);
    _hestPrintStr(stderr, len, len, hparm->columns,
                  tendCmdList[i]->info, AIR_FALSE);
  }
}

/*
******** tendFiberStopParse
** 
** for parsing the different ways in which a fiber should be stopped
** For the sake of laziness and uniformity, the stop information is 
** stored in an array of 3 (three) doubles:
** info[0]: int value from tenFiberStop* enum
** info[1]: 1st parameter associated with stop method (always used)
** info[2]: 2nd parameter, used occasionally
*/
int
tendFiberStopParse(void *ptr, char *_str, char err[AIR_STRLEN_HUGE]) {
  char me[]="tenFiberStopParse", *str, *opt, *opt2;
  double *info;
  airArray *mop;
  int integer;

  if (!(ptr && _str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  info = (double *)ptr;

  mop = airMopNew();
  str = airStrdup(_str);
  airMopMem(mop, &str, airMopAlways);
  opt = strchr(str, ':');
  if (!opt) {
    /* couldn't parse string as nrrdEncoding, but there wasn't a colon */
    sprintf(err, "%s: didn't see a colon in \"%s\"", me, str);
    airMopError(mop); return 1;
  } 
  *opt = '\0';
  opt++;
  info[0] = airEnumVal(tenFiberStop, str);
  if (tenFiberStopUnknown == info[0]) {
    sprintf(err, "%s: didn't recognize \"%s\" as %s",
            me, str, tenFiberStop->name);
    airMopError(mop); return 1;
  }
  switch((int)info[0]) {
  case tenFiberStopAniso:
    /* <aniso>,<level> : tenAniso,double */
    opt2 = strchr(opt, ',');
    if (!opt2) {
      sprintf(err, "%s: didn't see comma between aniso and level in \"%s\"",
              me, opt);
      airMopError(mop); return 1;
    }
    *opt2 = '\0';
    opt2++;
    info[1] = airEnumVal(tenAniso, opt);
    if (tenAnisoUnknown == info[1]) {
      sprintf(err, "%s: didn't recognize \"%s\" as %s",
              me, opt, tenAniso->name);
      airMopError(mop); return 1;
    }
    if (1 != sscanf(opt2, "%lg", info+2)) {
      sprintf(err, "%s: couldn't parse aniso level \"%s\" as double",
              me, opt2);
      airMopError(mop); return 1;
    }
    /*
    fprintf(stderr, "!%s: parsed aniso:%s,%g\n", me,
            airEnumStr(tenAniso, info[1]), info[2]);
    */
    break;
  case tenFiberStopLength:
    /* <length> : double */
  case tenFiberStopConfidence:
    /* <conf> : double */
    if (1 != sscanf(opt, "%lg", info+1)) {
      sprintf(err, "%s: couldn't parse %s \"%s\" as double", me,
              (info[0] == tenFiberStopLength
               ? "length" : "confidence"), opt);
      airMopError(mop); return 1;
    }
    /*
    fprintf(stderr, "!%s: parse %s:%g\n", me, 
            (info[0] == tenFiberStopLength ? "length" : "confidence"),
            info[1]);
    */
    break;
  case tenFiberStopNumSteps:
    /* <#steps> : int */
    if (1 != sscanf(opt, "%d", &integer)) {
      sprintf(err, "%s: couldn't parse \"%s\" as int", me, opt);
      airMopError(mop); return 1;
    }
    info[1] = integer;
    /* fprintf(stderr, "!%s: parse steps:%d\n", me, integer); */
    break;
  case tenFiberStopBounds:
    /* moron */
    break;
  default:
    sprintf(err, "%s: stop method %d not suppored", me, (int)info[0]);
    airMopError(mop); return 1;
    break;
  }
  airMopOkay(mop);
  return 0;
}

hestCB
_tendFiberStopCB = {
  3*sizeof(double),
  "fiber stop",
  tendFiberStopParse,
  NULL
};

hestCB *
tendFiberStopCB = &_tendFiberStopCB;
