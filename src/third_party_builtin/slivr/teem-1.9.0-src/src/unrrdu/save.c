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

#define INFO "Write nrrd with specific format, encoding, or endianness"
char *_unrrdu_saveInfoL =
(INFO
 ". Use \"unu\tsave\t-f\tpnm\t|\txv\t-\" to view PPM- or "
 "PGM-compatible nrrds on unix.  EPS output is a EPSF-3.0 file with "
 "BoundingBox and HiResBoundingBox DSC comments, and is suitable for "
 "inclusion into other PostScript documents.  As a stand-alone file, the "
 "image is conveniently centered on an 8.5x11 inch page, with 0.5 "
 "inch margins.");

int
unrrdu_saveMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err, *outData,
    encInfo[AIR_STRLEN_HUGE], fmtInfo[AIR_STRLEN_HUGE];
  Nrrd *nin, *nout;
  airArray *mop;
  NrrdIoState *nio;
  int pret, enc[3], formatType;

  mop = airMopNew();
  nio = nrrdIoStateNew();
  airMopAdd(mop, nio, (airMopper)nrrdIoStateNix, airMopAlways);

  strcpy(fmtInfo,
         "output file format. Possibilities include:\n "
         "\b\bo \"nrrd\": standard nrrd format\n "
         "\b\bo \"pnm\": PNM image; PPM for color, PGM for grayscale\n "
         "\b\bo \"text\": plain ASCII text for 1-D and 2-D data\n "
         "\b\bo \"vtk\": VTK \"STRUCTURED_POINTS\" dataset");
  if (nrrdFormatPNG->available()) {
    strcat(fmtInfo,
           "\n \b\bo \"png\": PNG image");
  }
  strcat(fmtInfo,
         "\n \b\bo \"eps\": EPS file");
  hestOptAdd(&opt, "f,format", "form", airTypeEnum, 1, 1, &formatType, NULL,
             fmtInfo, NULL, nrrdFormatType);
  strcpy(encInfo,
         "encoding of data in file.  Not all encodings are supported in "
         "a given format. Possibilities include:"
         "\n \b\bo \"raw\": raw encoding"
         "\n \b\bo \"ascii\": print data in ascii"
         "\n \b\bo \"hex\": two hex digits per byte");
  if (nrrdEncodingGzip->available()) {
    strcat(encInfo, 
           "\n \b\bo \"gzip\", \"gz\": gzip compressed raw data");
  }
  if (nrrdEncodingBzip2->available()) {
    strcat(encInfo, 
           "\n \b\bo \"bzip2\", \"bz2\": bzip2 compressed raw data");
  }
  if (nrrdEncodingGzip->available() || nrrdEncodingBzip2->available()) {
    strcat(encInfo,
           "\n The specifiers for compressions may be followed by a colon "
           "\":\", followed by an optional digit giving compression \"level\" "
           "(for gzip) or \"block size\" (for bzip2).  For gzip, this can be "
           "followed by an optional character for a compression strategy:\n "
           "\b\bo \"d\": default, Huffman with string match\n "
           "\b\bo \"h\": Huffman alone\n "
           "\b\bo \"f\": specialized for filtered data\n "
           "For example, \"gz\", \"gz:9\", \"gz:9f\" are all valid");
  }
  hestOptAdd(&opt, "e,encoding", "enc", airTypeOther, 1, 1, enc, "raw",
             encInfo, NULL, NULL, &unrrduHestEncodingCB);
  hestOptAdd(&opt, "en,endian", "end", airTypeEnum, 1, 1, &(nio->endian),
             airEnumStr(airEndian, airMyEndian),
             "Endianness to save data out as; \"little\" for Intel and "
             "friends; \"big\" for everyone else. "
             "Defaults to endianness of this machine",
             NULL, airEndian);
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");
  hestOptAdd(&opt, "od,ouputdata", "name", airTypeString, 1, 1, &outData, "",
             "when saving to a \".nhdr\" file, "
             "this option allows you to explicitly name the data file, "
             "instead of (by default, not using this option) having it be "
             "the same filename base as the header file.");

  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_saveInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  nrrdCopy(nout, nin);
  
  nio->encoding = nrrdEncodingArray[enc[0]];
  nio->format = nrrdFormatArray[formatType];
  if (nrrdEncodingTypeGzip == enc[0]) {
    nio->zlibLevel = enc[1];
    nio->zlibStrategy = enc[2];
  } else if (nrrdEncodingTypeBzip2 == enc[0]) {
    nio->bzip2BlockSize = enc[1];
  }
  if (AIR_ENDIAN != nio->endian) {
    nrrdSwapEndian(nout);
  }
  if (airEndsWith(out, NRRD_EXT_NHDR)) {
    if (nio->format != nrrdFormatNRRD) {
      fprintf(stderr, "%s: WARNING: will use %s format\n", me,
              nrrdFormatNRRD->name);
      nio->format = nrrdFormatNRRD;
    }
    if (strlen(outData)) {
      airArrayLenSet(nio->dataFNArr, 1);
      nio->dataFN[0] = airStrdup(outData);
    }
  }

  SAVE(out, nout, nio);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(save, INFO);
