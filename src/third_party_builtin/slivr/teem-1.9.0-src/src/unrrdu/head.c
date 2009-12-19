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

#include "unrrdu.h"
#include "privateUnrrdu.h"

#define INFO "Print header of one or more nrrd files"
char *_unrrdu_headInfoL = 
(INFO  ".  The value of this is simply to print the contents of nrrd "
 "headers.  This avoids the use of \"head -N\", where N has to be "
 "determined manually, which always risks printing raw binary data "
 "(following the header) to screen, which tends to clobber terminal "
 "settings, as well as be annoying.");

int
unrrdu_headDoit(char *me, NrrdIoState *nio, char *inS, FILE *fout) {
  char err[BIFF_STRLEN];
  airArray *mop;
  unsigned int len;
  FILE *fin;

  mop = airMopNew();
  if (!( fin = airFopen(inS, stdin, "rb") )) {
    sprintf(err, "%s: couldn't fopen(\"%s\",\"rb\"): %s\n", 
            me, inS, strerror(errno));
    biffAdd(me, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, fin, (airMopper)airFclose, airMopAlways);

  if (_nrrdOneLine(&len, nio, fin)) {
    sprintf(err, "%s: error getting first line of file \"%s\"", me, inS);
    biffAdd(me, err); airMopError(mop); return 1;
  }
  if (!len) {
    sprintf(err, "%s: immediately hit EOF\n", me);
    biffAdd(me, err); airMopError(mop); return 1;
  }
  if (!( nrrdFormatNRRD->contentStartsLike(nio) )) {
    sprintf(err, "%s: first line (\"%s\") isn't a nrrd magic\n", 
            me, nio->line);
    biffAdd(me, err); airMopError(mop); return 1;
  }
  while (len > 1) {
    fprintf(fout, "%s\n", nio->line);
    _nrrdOneLine(&len, nio, fin);
  };
  
  /* experience has shown that on at least windows and darwin, the writing
     process's fwrite() to stdout will fail if we exit without consuming
     everything from stdin */
  if (stdin == fin) {
    int c = getc(fin);
    while (EOF != c) {
      c = getc(fin);
    }
  }

  airMopOkay(mop);
  return 0;
}

int
unrrdu_headMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *err, **inS;
  NrrdIoState *nio;
  airArray *mop;
  int pret;
  unsigned int ni, ninLen;

  mop = airMopNew();
  hestOptAdd(&opt, NULL, "nin1", airTypeString, 1, -1, &inS, NULL,
             "input nrrd(s)", &ninLen);
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_headInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nio = nrrdIoStateNew();
  airMopAdd(mop, nio, (airMopper)nrrdIoStateNix, airMopAlways);

  for (ni=0; ni<ninLen; ni++) {
    if (ninLen > 1) {
      fprintf(stdout, "==> %s <==\n", inS[ni]);
    }
    if (unrrdu_headDoit(me, nio, inS[ni], stdout)) {
      airMopAdd(mop, err = biffGetDone(me), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble reading from \"%s\":\n%s",
              me, inS[ni], err);
      /* continue working on the remaining files */
    }
    if (ninLen > 1 && ni < ninLen-1) {
      fprintf(stdout, "\n");
    }
  }

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(head, INFO);
