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

#include <teem/air.h>
#include <teem/hest.h>
#include <teem/nrrd.h>

char *info = ("Converts from floating-point \"index\" to floating-point "
              "\"position\", given the centering of the data "
              "(cell vs. node), the range of positions, and the number "
              "of intervals into which position has been quantized. ");

int
main(int argc, char *argv[]) {
  char *me;
  hestOpt *hopt = NULL;
  int center;
  double minPos, maxPos, pos, index, size;

  me = argv[0];
  hestOptAdd(&hopt, NULL, "center", airTypeEnum, 1, 1, &center, NULL,
             "which centering applies to the quantized position.\n "
             "Possibilities are:\n "
             "\b\bo \"cell\": for histogram bins, quantized values, and "
             "pixels-as-squares\n "
             "\b\bo \"node\": for non-trivially interpolated "
             "sample points", NULL, nrrdCenter);
  hestOptAdd(&hopt, NULL, "minPos", airTypeDouble, 1, 1, &minPos, NULL,
             "smallest position associated with index 0");
  hestOptAdd(&hopt, NULL, "maxPos", airTypeDouble, 1, 1, &maxPos, NULL,
             "highest position associated with highest index");
  hestOptAdd(&hopt, NULL, "num", airTypeDouble, 1, 1, &size, NULL,
             "number of intervals into which position has been quantized");
  hestOptAdd(&hopt, NULL, "index", airTypeDouble, 1, 1, &index, NULL,
             "the input index, to be converted to position");
  hestParseOrDie(hopt, argc-1, argv+1, NULL, me, info,
                 AIR_TRUE, AIR_TRUE, AIR_TRUE);

  pos = NRRD_POS(center, minPos, maxPos, size, index);
  printf("%g\n", pos);
  
  hestParseFree(hopt);
  hestOptFree(hopt);
  exit(0);
}
