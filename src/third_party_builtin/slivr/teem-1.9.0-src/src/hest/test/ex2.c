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
  int res[2], v, numIn;
  char **in, *out;
  int n;
  hestOpt *opt = NULL;
  hestParm *parm;
  char *err = NULL, info[] = 
    "This program does nothing in particular, though it does attempt "
    "to pose as some sort of command-line image processing program. "
    "As usual, any implied functionality is purely coincidental, "
    "especially since this is the output of a unicyclist.";

  parm = hestParmNew();
  parm->respFileEnable = AIR_TRUE;

  opt = NULL;
  hestOptAdd(&opt, "res",   "sx sy", airTypeInt,    2,  2,  res,  NULL,
             "image resolution");
  hestOptAdd(&opt, "v",     "level", airTypeInt,    0,  1,  &v,   "0",
             "verbosity level");
  hestOptAdd(&opt, "out",   "file",  airTypeString, 1,  1,  &out, "output.ppm",
             "PPM image output");
  hestOptAdd(&opt, NULL,    "input", airTypeString, 1, -1,  &in,  NULL,
             "input image file(s)", &numIn);
  
  if (1 == argc) {
    /* didn't get anything at all on command line */
    /* print program information ... */
    hestInfo(stderr, argv[0], info, parm);
    /* ... and usage information ... */
    hestUsage(stderr, opt, argv[0], parm);
    hestGlossary(stderr, opt, parm);
    /* ... and avoid memory leaks */
    opt = hestOptFree(opt);
    parm = hestParmFree(parm);
    exit(1);
  }

  /* else we got something, see if we can parse it */
  if (hestParse(opt, argc-1, argv+1, &err, parm)) {
    fprintf(stderr, "ERROR: %s\n", err); free(err);
    /* print usage information ... */
    hestUsage(stderr, opt, argv[0], parm);
    hestGlossary(stderr, opt, parm);
    /* ... and then avoid memory leaks */
    opt = hestOptFree(opt);
    parm = hestParmFree(parm);
    exit(1);
  }
  
  printf("(err = %s)\n", err);
  printf("res = %d %d\n", res[0], res[1]);
  printf("  v = %d\n", v);
  printf("out = \"%s\"\n", out);
  printf(" in = %d files:", numIn);
  for (n=0; n<=numIn-1; n++) {
    printf(" \"%s\"", in[n]);
  }
  printf("\n");

  /* free the memory allocated by parsing ... */
  hestParseFree(opt);
  /* ... and the other stuff */
  opt = hestOptFree(opt);
  parm = hestParmFree(parm);
  exit(0);
}
