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


#include "../air.h"

/*
**  0      1       (2)
** bessy <double>
*/
int
main(int argc, char *argv[]) {
  char *me;
  double x;

  me = argv[0];
  if (2 != argc || 1 != sscanf(argv[1], "%lg", &x)) {
    fprintf(stderr, "%s: need one double as argument\n", me);
    exit(1);
  }
  printf("BesselI(0, %g) = %g\n", x, airBesselI0(x));
  printf("log(BesselI(0, %g)) = %g  ?=?  %g\n", x, airLogBesselI0(x),
         log(airBesselI0(x)));
  printf("BesselI(1, %g) = %g\n", x, airBesselI1(x));
  printf("BesselI1By0(%g) = %g  ?=?  %g\n", x,
         airBesselI1By0(x), airBesselI1(x)/airBesselI0(x));
  printf("erfc,erf(%g) = %g  %g\n", x, airErfc(x), airErf(x));
  exit(0);
}
