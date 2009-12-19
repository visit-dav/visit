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

#define INFO "List relevant environment variables and their values"
char *_unrrdu_envInfoL = (INFO
                          ". These environment variables provide a way of "
                          "setting global variables that can affect"
                          " the way Nrrd operates.");

void
_unrrdu_envBool(FILE *file, const char *envKey, int currVal,
                const char *varName, const char *desc, int columns) {
  int val, ret;
  char *envVal;

  fprintf(file, "%s (bool): ", envKey);
  ret = nrrdGetenvBool(&val, &envVal, envKey);
  switch(ret) {
  case -1:
    fprintf(file, "not set\n");
    break;
  case AIR_TRUE:
    fprintf(file, "set to \"%s\"\n", envVal);
    break;
  case AIR_FALSE:
    fprintf(file, "set to \"%s\"? (invalid) \n", envVal);
    break;
  }
  switch(ret) {
  case -1:
  case AIR_FALSE:
    fprintf(file, "  (%s == %s; unchanged)\n",
            varName, airEnumStr(airBool, currVal));
    break;
  case AIR_TRUE:
    fprintf(file, "  ==> %s = %s   **********************\n",
            varName, airEnumStr(airBool, currVal));
    break;
  }
  _hestPrintStr(file, 0, 0, columns, desc, AIR_FALSE);
  fprintf(file, "\n");
}

void
_unrrdu_envEnum(FILE *file, const airEnum *enm, const char *envKey,
                int currVal, const char *varName,
                const char *desc, int columns) {
  int val, ret;
  char *envVal;

  /* !!! HEY: CUT + PASTE !!! */
  fprintf(file, "%s (%s enum): ", envKey, enm->name);
  ret = nrrdGetenvEnum(&val, &envVal, enm, envKey);
  switch(ret) {
  case -1:
    fprintf(file, "not set\n");
    break;
  case AIR_TRUE:
    fprintf(file, "set to \"%s\"\n", envVal);
    break;
  case AIR_FALSE:
    fprintf(file, "set to \"%s\"? (invalid) \n", envVal);
    break;
  }
  switch(ret) {
  case -1:
  case AIR_FALSE:
    fprintf(file, "  (%s == %s; unchanged)\n",
            varName, airEnumStr(enm, currVal));
    break;
  case AIR_TRUE:
    fprintf(file, "  ==> %s = %s   **********************\n",
            varName, airEnumStr(enm, currVal));
    break;
  }
  _hestPrintStr(file, 0, 0, columns, desc, AIR_FALSE);
  fprintf(file, "\n");
  /* !!! HEY: CUT + PASTE !!! */
}

void
_unrrdu_envInt(FILE *file, const char *envKey,
               int currVal, const char *varName,
               const char *desc, int columns) {
  int val, ret;
  char *envVal;

  /* !!! HEY: CUT + PASTE !!! */
  fprintf(file, "%s (int): ", envKey);
  ret = nrrdGetenvInt(&val, &envVal, envKey);
  switch(ret) {
  case -1:
    fprintf(file, "not set\n");
    break;
  case AIR_TRUE:
    fprintf(file, "set to \"%s\"\n", envVal);
    break;
  case AIR_FALSE:
    fprintf(file, "set to \"%s\"? (invalid) \n", envVal);
    break;
  }
  switch(ret) {
  case -1:
  case AIR_FALSE:
    fprintf(file, "  (%s == %d; unchanged)\n",
            varName, currVal);
    break;
  case AIR_TRUE:
    fprintf(file, "  ==> %s = %d   **********************\n",
            varName, currVal);
    break;
  }
  _hestPrintStr(file, 0, 0, columns, desc, AIR_FALSE);
  fprintf(file, "\n");
  /* !!! HEY: CUT + PASTE !!! */
}

void
_unrrdu_envUInt(FILE *file, const char *envKey,
                unsigned int currVal, const char *varName,
                const char *desc, int columns) {
  int ret;
  unsigned int val;
  char *envVal;

  /* !!! HEY: CUT + PASTE !!! */
  fprintf(file, "%s (unsigned int): ", envKey);
  ret = nrrdGetenvUInt(&val, &envVal, envKey);
  switch(ret) {
  case -1:
    fprintf(file, "not set\n");
    break;
  case AIR_TRUE:
    fprintf(file, "set to \"%s\"\n", envVal);
    break;
  case AIR_FALSE:
    fprintf(file, "set to \"%s\"? (invalid) \n", envVal);
    break;
  }
  switch(ret) {
  case -1:
  case AIR_FALSE:
    fprintf(file, "  (%s == %d; unchanged)\n",
            varName, currVal);
    break;
  case AIR_TRUE:
    fprintf(file, "  ==> %s = %u   **********************\n",
            varName, currVal);
    break;
  }
  _hestPrintStr(file, 0, 0, columns, desc, AIR_FALSE);
  fprintf(file, "\n");
  /* !!! HEY: CUT + PASTE !!! */
}

int
unrrdu_envMain(int argc, char **argv, char *me, hestParm *hparm) {

  AIR_UNUSED(argc);
  AIR_UNUSED(argv);
  AIR_UNUSED(me);

  hestInfo(stderr, me, _unrrdu_envInfoL, hparm);
  fprintf(stderr, "\n");

  _hestPrintStr(stderr, 0, 0, hparm->columns, 
                ("Each variable in the listing below starts with the name of "
                 "the environment variable (\"NRRD_...\"), what type of value "
                 "it represents (e.g. \"int\", \"bool\"), what the "
                 "environment variable is currently set to, what the "
                 "corresponding Nrrd global variable is set to, and a "
                 "description of the variable."),
                AIR_FALSE);
  fprintf(stderr, "\n");

  _hestPrintStr(stderr, 0, 0, hparm->columns, 
                ("Bool variables may be set to true simply by setting the "
                 "environment variable; setting the value to \"true\" or "
                 "\"false\" sets the bool accordingly.  Enum variables may "
                 "be set by setting the environment variable to any string "
                 "that parses as one of the enum values.  Int and unsigned "
                 "int variables are set via a string parse-able as a numeric "
                 "value."),
                AIR_FALSE);
  fprintf(stderr, "\n");

  _unrrdu_envBool(stderr, 
                  nrrdEnvVarStateKeyValuePairsPropagate,
                  nrrdStateKeyValuePairsPropagate,
                  "nrrdStateKeyValuePairsPropagate",
                  "When true, key/value pairs are copied from input "
                  "nrrd to output nrrd just like other basic info that hasn't "
                  "just been modified (e.g. type, dimension, block size).",
                  hparm->columns);
  _unrrdu_envEnum(stderr,
                  nrrdCenter, nrrdEnvVarDefaultCenter,
                  nrrdDefaultCenter,
                  "nrrdDefaultCenter",
                  "The type of sample centering to use when none has been "
                  "set but one has to be chosen for some operation "
                  "(e.g. resampling).",
                  hparm->columns);
  _unrrdu_envEnum(stderr,
                  nrrdEncodingType, nrrdEnvVarDefaultWriteEncodingType,
                  nrrdDefaultWriteEncodingType,
                  "nrrdDefaultWriteEncodingType",
                  "When writing nrrds, what encoding to use. Only "
                  "\"unu save\" affords explicit control of output encoding.",
                  hparm->columns);
  _unrrdu_envBool(stderr,
                  nrrdEnvVarStateKindNoop,
                  nrrdStateKindNoop,
                  "nrrdStateKindNoop",
                  "When true, Nrrd makes not even the slightest effort to be "
                  "smart about setting the \"kind\" field of an axis after "
                  "some operation that modified its samples.",
                  hparm->columns);
  _unrrdu_envInt(stderr,
                 nrrdEnvVarStateVerboseIO,
                 nrrdStateVerboseIO,
                 "nrrdStateVerboseIO",
                 "The verbosity level of Nrrd input/output operations.",
                  hparm->columns);
  _unrrdu_envBool(stderr,
                  nrrdEnvVarStateBlind8BitRange,
                  nrrdStateBlind8BitRange,
                  "nrrdStateBlind8BitRange",
                  "When true, the determined range of 8-bit data will always "
                  "be [0,255] (for uchar) or [-128,127] (for signed char), "
                  "instead of actually looking into the data to find its "
                  "range.",
                  hparm->columns);
  _unrrdu_envBool(stderr,
                  nrrdEnvVarDefaultWriteBareText,
                  nrrdDefaultWriteBareText,
                  "nrrdDefaultWriteBareText",
                  "When false, text files used for saving nrrds start with "
                  "comment (\"# ...\") lines containing nrrd fields.",
                  hparm->columns);
  _unrrdu_envEnum(stderr,
                  nrrdType, nrrdEnvVarStateMeasureType,
                  nrrdStateMeasureType,
                  "nrrdStateMeasureType",
                  "For measurements (\"unu project\") like sum and product, "
                  "the type of the output result, when one hasn't been "
                  "explicitly requested.",
                  hparm->columns);
  _unrrdu_envInt(stderr,
                 nrrdEnvVarStateMeasureModeBins,
                 nrrdStateMeasureModeBins,
                 "nrrdStateMeasureModeBins",
                 "When measuring mode but without a given histogram, how many "
                 "bins to use in the temporary internal histogram.",
                  hparm->columns);
  _unrrdu_envEnum(stderr,
                  nrrdType, nrrdEnvVarStateMeasureHistoType,
                  nrrdStateMeasureHistoType,
                  "nrrdStateMeasureHistoType",
                  "Output type for most measurements of histograms, when one "
                  "hasn't been explicitly requested",
                  hparm->columns);
  _unrrdu_envBool(stderr,
                  nrrdEnvVarStateAlwaysSetContent,
                  nrrdStateAlwaysSetContent,
                  "nrrdStateAlwaysSetContent",
                  "If true, the output content string is set even when the "
                  "input content string is not set.",
                  hparm->columns);
  _unrrdu_envBool(stderr,
                  nrrdEnvVarStateDisableContent,
                  nrrdStateDisableContent,
                  "nrrdStateDisableContent",
                  "If true, output content is never set.",
                  hparm->columns);
  _unrrdu_envUInt(stderr,
                  nrrdEnvVarDefaultWriteCharsPerLine,
                  nrrdDefaultWriteCharsPerLine,
                  "nrrdDefaultWriteCharsPerLine",
                  "When using text encoding, maximum # characters allowed "
                  "per line.",
                  hparm->columns);
  _unrrdu_envUInt(stderr,
                  nrrdEnvVarDefaultWriteValsPerLine,
                  nrrdDefaultWriteValsPerLine,
                  "nrrdDefaultWriteValsPerLine",
                  "When using text encoding, maximum # values allowed "
                  "per line",
                  hparm->columns);
  _unrrdu_envBool(stderr,
                  nrrdEnvVarStateGrayscaleImage3D,
                  nrrdStateGrayscaleImage3D,
                  "nrrdStateGrayscaleImage3D",
                  "If true, reading a 2-D grayscale image results in a "
                  "3-D image with a single sample (size=1) on the first "
                  "(fastest) axis.",
                  hparm->columns);

#if 0
  /* GLK is ambivalent about the continued existence of these ... */
  nrrdGetenvDouble(/**/ &nrrdDefaultKernelParm0,
                   nrrdEnvVarDefaultKernelParm0);
  nrrdGetenvDouble(/**/ &nrrdDefaultSpacing,
                   nrrdEnvVarDefaultSpacing);
#endif  

  return 0;
}

UNRRDU_CMD(env, INFO);
