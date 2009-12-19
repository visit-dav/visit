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

#include "gage.h"
#include "privateGage.h"

void
_gageSclIv3Print (FILE *file, gageContext *ctx, gagePerVolume *pvl) {
  gage_t *iv3;
  int i, fd;

  iv3 = pvl->iv3;
  fd = 2*ctx->radius;
  fprintf(file, "iv3[]:\n");
  switch(fd) {
  case 2:
    fprintf(file, "% 10.4f   % 10.4f\n", (float)iv3[6], (float)iv3[7]);
    fprintf(file, "   % 10.4f   % 10.4f\n\n", (float)iv3[4], (float)iv3[5]);
    fprintf(file, "% 10.4f   % 10.4f\n", (float)iv3[2], (float)iv3[3]);
    fprintf(file, "   % 10.4f   % 10.4f\n", (float)iv3[0], (float)iv3[1]);
    break;
  case 4:
    for (i=3; i>=0; i--) {
      fprintf(file, "% 10.4f   % 10.4f   % 10.4f   % 10.4f\n", 
              (float)iv3[12+16*i], (float)iv3[13+16*i], 
              (float)iv3[14+16*i], (float)iv3[15+16*i]);
      fprintf(file, "   % 10.4f  %c% 10.4f   % 10.4f%c   % 10.4f\n", 
              (float)iv3[ 8+16*i], (i==1||i==2)?'\\':' ',
              (float)iv3[ 9+16*i], (float)iv3[10+16*i], (i==1||i==2)?'\\':' ',
              (float)iv3[11+16*i]);
      fprintf(file, "      % 10.4f  %c% 10.4f   % 10.4f%c   % 10.4f\n", 
              (float)iv3[ 4+16*i], (i==1||i==2)?'\\':' ',
              (float)iv3[ 5+16*i], (float)iv3[ 6+16*i], (i==1||i==2)?'\\':' ',
              (float)iv3[ 7+16*i]);
      fprintf(file, "         % 10.4f   % 10.4f   % 10.4f   % 10.4f\n", 
              (float)iv3[ 0+16*i], (float)iv3[ 1+16*i],
              (float)iv3[ 2+16*i], (float)iv3[ 3+16*i]);
      if (i) fprintf(file, "\n");
    }
    break;
  default:
    for (i=0; i<fd*fd*fd; i++) {
      fprintf(file, "  iv3[% 3d,% 3d,% 3d] = % 10.4f\n",
              i%fd, (i/fd)%fd, i/(fd*fd), (float)iv3[i]);
    }
    break;
  }
  return;
}
