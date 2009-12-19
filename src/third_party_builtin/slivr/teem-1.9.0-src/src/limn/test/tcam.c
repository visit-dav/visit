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


#include "../limn.h"

char *info = ("dumb little demonstration of calculating the "
              "world-to-view and view-to-world transforms given "
              "from, at, and up locations.");

int
main(int argc, char *argv[]) {
  char *me, *err;
  limnCamera *cam;
  float mat[16];
  hestOpt *hopt=NULL;
  airArray *mop;

  mop = airMopNew();
  cam = limnCameraNew();
  airMopAdd(mop, cam, (airMopper)limnCameraNix, airMopAlways);
  
  me = argv[0];
  hestOptAdd(&hopt, "fr", "eye pos", airTypeDouble, 3, 3, cam->from,
             NULL, "camera eye point");
  hestOptAdd(&hopt, "at", "at pos", airTypeDouble, 3, 3, cam->at,
             "0 0 0", "camera look-at point");
  hestOptAdd(&hopt, "up", "up dir", airTypeDouble, 3, 3, cam->up,
             "0 0 1", "camera pseudo up vector");
  hestOptAdd(&hopt, "rh", NULL, airTypeInt, 0, 0, &(cam->rightHanded), NULL,
             "use a right-handed UVN frame (V points down)");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  cam->neer = -1;
  cam->dist = 0;
  cam->faar = 1;
  cam->atRelative = AIR_TRUE;

  if (limnCameraUpdate(cam)) {
    fprintf(stderr, "%s: trouble:\n%s\n", me, err = biffGet(LIMN));
    free(err);
    return 1;
  }
  
  printf("%s: W2V:\n", me);
  ELL_4M_COPY(mat, cam->W2V);
  ell_4m_print_f(stdout, mat);

  printf("\n");
  printf("%s: V2W:\n", me);
  ELL_4M_COPY(mat, cam->V2W);
  ell_4m_print_f(stdout, mat);

  airMopOkay(mop);
  return 0;
}

