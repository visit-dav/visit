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

#include <teem/bane.h>

#define GKMS "gkms"

int
main(int argc, char **argv) {
  int i, ret;
  char *me, *argv0 = NULL, *err;
  hestParm *hparm;
  airArray *mop;

  me = argv[0];
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
    fprintf(stderr, "  for Teem.\n");
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
    baneGkmsUsage(GKMS, hparm);
    airMopError(mop);
    exit(1);
  }
  /* else, we should see if they're asking for a command we know about */  
  /* baneGkmsCmdList[] is NULL-terminated */
  for (i=0; baneGkmsCmdList[i]; i++) {
    if (!strcmp(argv[1], baneGkmsCmdList[i]->name))
      break;
  }
  if (baneGkmsCmdList[i]) {
    /* yes, we have that command */
    /* initialize variables used by the various commands */
    argv0 = AIR_CAST(char*, malloc(strlen(GKMS) + strlen(argv[1]) + 2));
    airMopMem(mop, &argv0, airMopAlways);
    sprintf(argv0, "%s %s", GKMS, argv[1]);

    /* run the individual unu program, saving its exit status */
    ret = baneGkmsCmdList[i]->main(argc-2, argv+2, argv0, hparm);
    if (1 == ret) {
      airMopAdd(mop, err=biffGetDone(BANE), airFree, airMopAlways);
      fprintf(stderr, "%s: error:\n%s", argv0, err);
    } else if (2 == ret) {
      /* gkms command has already handled printing error messages */
      ret = 1;
    }
  } else {
    fprintf(stderr, "%s: unrecognized command: \"%s\"; type \"%s\" for "
            "complete list\n", me, argv[1], me);
    ret = 1;
  }

  airMopDone(mop, ret);
  return ret;
}
