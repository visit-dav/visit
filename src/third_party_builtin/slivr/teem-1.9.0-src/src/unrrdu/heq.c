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

#define INFO "Perform histogram equalization"
char *_unrrdu_heqInfoL =
(INFO
 ". If this seems to be doing nothing, try increasing the "
 "number of histograms bins by an order of magnitude or "
 "two (or more).  Or, use \"unu gamma\" to warp the values "
 "in the direction you know they need to go.  Either of "
 "these might work because extremely tall and narrow peaks "
 "in the equalization histogram will produce poor results.");

int
unrrdu_heqMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err, *mapS;
  Nrrd *nin, *nout, *nmap;
  int smart, pret;
  unsigned int bins;
  airArray *mop;
  float amount;

  /* we want to facilitate saving out the mapping as a text file,
     but with the domain included */
  /* this is commented out with the 8 Aug 2003 advent of nrrdDefGetenv
  nrrdDefWriteBareTable = AIR_FALSE;
  */

  hestOptAdd(&opt, "b,bin", "bins", airTypeInt, 1, 1, &bins, NULL,
             "# bins to use in histogram that is created in order to "
             "calculate the mapping that achieves the equalization.");
  hestOptAdd(&opt, "s,smart", "bins", airTypeInt, 0, 1, &smart, "0",
             "# bins in value histogram to ignore in calculating the mapping. "
             "Bins are ignored when they get more hits than other bins, and "
             "when the values that fall in them are constant.  This is an "
             "effective way to prevent large regions of background value "
             "from distorting the equalization mapping.");
  hestOptAdd(&opt, "a,amount", "amount", airTypeFloat, 1, 1, &amount, "1.0",
             "extent to which the histogram equalizing mapping should be "
             "applied; 0.0: no change, 1.0: full equalization");
  hestOptAdd(&opt, "m,map", "filename", airTypeString, 1, 1, &mapS, "",
             "The value mapping used to achieve histogram equalization is "
             "represented by a univariate regular map.  By giving a filename "
             "here, that map can be saved out and applied to other nrrds "
             "with \"unu rmap\"");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_heqInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  
  if (nrrdHistoEq(nout, nin, airStrlen(mapS) ? &nmap : NULL,
                  bins, smart, amount)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble histogram equalizing:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  if (airStrlen(mapS)) {
    SAVE(mapS, nmap, NULL);
  }
  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(heq, INFO);
