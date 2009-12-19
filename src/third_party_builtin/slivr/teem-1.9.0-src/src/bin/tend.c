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

#include <teem/ten.h>

#define TEND "tend"

int
main(int argc, char **argv) {
  int i, ret;
  char *me, *argv0 = NULL, *err;
  hestParm *hparm;
  airArray *mop;

  me = argv[0];

  /* parse environment variables first, in case they break nrrdDefault*
     or nrrdState* variables in a way that nrrdSanity() should see */
  nrrdDefaultGetenv();
  nrrdStateGetenv();

  /* no harm done in making sure we're sane */
  if (!nrrdSanity()) {
    fprintf(stderr, "******************************************\n");
    fprintf(stderr, "******************************************\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  %s: nrrd sanity check FAILED.\n", me);
    fprintf(stderr, "\n");
    fprintf(stderr, "  This means that either nrrd can't work on this "
            "platform, or (more likely)\n");
    fprintf(stderr, "  there was an error in the compilation options "
            "and variable definitions\n");
    fprintf(stderr, "  for how Teem was built here.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  %s\n", err = biffGetDone(NRRD));
    fprintf(stderr, "\n");
    fprintf(stderr, "******************************************\n");
    fprintf(stderr, "******************************************\n");
    free(err);
    return 1;
  }

  mop = airMopNew();
  hparm = hestParmNew();
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hparm->elideSingleEnumType = AIR_TRUE;
  hparm->elideSingleOtherType = AIR_TRUE;
  hparm->elideSingleOtherDefault = AIR_FALSE;
  hparm->elideSingleNonExistFloatDefault = AIR_TRUE;
  hparm->elideMultipleNonExistFloatDefault = AIR_TRUE;
  hparm->elideSingleEmptyStringDefault = AIR_TRUE;
  hparm->elideMultipleEmptyStringDefault = AIR_TRUE;
  hparm->cleverPluralizeOtherY = AIR_TRUE;
  hparm->columns = 78;

  /* if there are no arguments, then we give general usage information */
  if (1 >= argc) {
    tendUsage(TEND, hparm);
    airMopError(mop);
    exit(1);
  }
  /* else, we should see if they're asking for a command we know about */  
  for (i=0; tendCmdList[i]; i++) {
    if (!strcmp(argv[1], tendCmdList[i]->name))
      break;
  }
  if (tendCmdList[i]) {
    /* yes, we have that command */
    /* initialize variables used by the various commands */
    argv0 = (char *)calloc(strlen(TEND) + strlen(argv[1]) + 2, sizeof(char));
    airMopMem(mop, &argv0, airMopAlways);
    sprintf(argv0, "%s %s", TEND, argv[1]);

    /* run the individual unu program, saving its exit status */
    ret = tendCmdList[i]->main(argc-2, argv+2, argv0, hparm);
  } else {
    fprintf(stderr, "%s: unrecognized command: \"%s\"; type \"%s\" for "
            "complete list\n", me, argv[1], me);
    ret = 1;
  }

  airMopDone(mop, ret);
  return ret;
}
