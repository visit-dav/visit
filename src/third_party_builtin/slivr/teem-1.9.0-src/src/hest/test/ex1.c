/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
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


#include "../hest.h"

int
main(int argc, char **argv) {
  static int res[2], v, numIn;
  static char **in, *out;
  static int *mm, mmm;
  int n;
  hestOpt opt[] = {
    {"res",   "sx sy", airTypeInt,    2,  2,   res,  NULL, 
     "image resolution"},
    {"v",     "level", airTypeInt,    0,  1,   &v,   NULL /* "0" */,
     "verbosity level"},
    {"VV",    "level", airTypeInt,    0,  5,   &mm,  "33 22 11",
     "gonzo level", &mmm},
    {"out",   "file",  airTypeString, 1,  1,   &out, "output.ppm",
     "PPM image output"},
    {NULL,    "input",  airTypeString, 1, -1,   &in,  NULL,
     "input image file(s)", &numIn},
    {NULL, NULL, 0}
  };
  hestParm *parm;
  char *err = NULL, info[] = 
    "This program does nothing in particular, though it does attempt "
    "to pose as some sort of command-line image processing program. "
    "Any implied functionality is purely coincidental, especially since "
    "this software was written by a sleep-deprived grad student.";

  parm = hestParmNew();
  parm->respFileEnable = AIR_TRUE;

  if (1 == argc) {
    /* didn't get anything at all on command line */
    hestInfo(stderr, argv[0], info, parm);
    hestUsage(stderr, opt, argv[0], parm);
    hestGlossary(stderr, opt, parm);
    parm = hestParmFree(parm);
    exit(1);
  }

  /* else we got something, see if we can parse it */
  if (hestParse(opt, argc-1, argv+1, &err, parm)) {
    fprintf(stderr, "ERROR: %s\n", err); free(err);
    hestUsage(stderr, opt, argv[0], parm);
    hestGlossary(stderr, opt, parm);
    parm = hestParmFree(parm);
    exit(1);
  }
  
  printf("(err = %s)\n", err);
  printf("res = %d %d\n", res[0], res[1]);
  printf("  v = %d\n", v);
  printf("out = \"%s\"\n", out);
  printf(" mm = %d ints:", mmm);
  for (n=0; n<=mmm-1; n++) {
    printf(" %d", mm[n]);
  }
  printf("\n");
  printf(" in = %d files:", numIn);
  for (n=0; n<=numIn-1; n++) {
    printf(" \"%s\"", in[n]);
  }
  printf("\n");

  hestParseFree(opt);
  parm = hestParmFree(parm);
  exit(0);
}
