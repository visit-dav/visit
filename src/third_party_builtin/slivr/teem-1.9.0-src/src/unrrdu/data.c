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

#define INFO "Print data segment of a nrrd file"
char *_unrrdu_dataInfoL = 
(INFO  ".  The value of this is to pass the data segment in isolation to a "
 "stand-alone decoder, in case this Teem build lacks an optional "
 "data encoding required for a given nrrd file.  Caveats: "
 "Will start copying "
 "characters from the datafile until EOF is hit, so this won't work "
 "correctly if the datafile has extraneous content at the end.  Will "
 "skip lines (as per \"line skip:\" header field) if needed, but can only "
 "skip bytes (as per \"byte skip:\") if the encoding is NOT a compression. "
 "\n \n "
 "To make vol.raw contain the uncompressed data from vol.nrrd "
 "which uses \"gz\" encoding: \"unu data vol.nrrd | gunzip > vol.raw\"");

int
unrrdu_dataMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *err, *inS=NULL;
  Nrrd *nin;
  NrrdIoState *nio;
  airArray *mop;
  int car, pret;

  mop = airMopNew();
  hestOptAdd(&opt, NULL, "nin", airTypeString, 1, 1, &inS, NULL,
             "input nrrd");
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_dataInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nio = nrrdIoStateNew();
  airMopAdd(mop, nio, (airMopper)nrrdIoStateNix, airMopAlways);
  nio->skipData = AIR_TRUE;
  nio->keepNrrdDataFileOpen = AIR_TRUE;
  nin = nrrdNew();
  airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdLoad(nin, inS, nio)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error reading header:\n%s", me, err);
    airMopError(mop);
    return 1;
  }
  if (_nrrdDataFNNumber(nio) > 1) {
    fprintf(stderr, "%s: sorry, currently can't operate with multiple "
            "detached datafiles\n", me);
    airMopError(mop);
    return 1;
  }
  if (!( nrrdFormatNRRD == nio->format )) {
    fprintf(stderr, "%s: can only print data of NRRD format files\n", me);
    airMopError(mop); return 1;
  }
  car = fgetc(nio->dataFile);
#ifdef _MSC_VER
  /* needed because otherwise printing a carraige return will 
     automatically also produce a newline */
  _setmode(_fileno(stdout), _O_BINARY);
#endif
  while (EOF != car) {
    fputc(car, stdout);
    car = fgetc(nio->dataFile);
  }
  airFclose(nio->dataFile);
  
  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(data, INFO);
