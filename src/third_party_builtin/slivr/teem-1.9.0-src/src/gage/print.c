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
_gagePrint_off(FILE *file, gageContext *ctx) {
  int i, fd;
  unsigned int *off;

  fd = 2*ctx->radius;
  off = ctx->off;
  fprintf(stderr, "off[]:\n");
  switch(fd) {
  case 2:
    fprintf(file, "% 6d   % 6d\n", off[6], off[7]);
    fprintf(file, "   % 6d   % 6d\n\n", off[4], off[5]);
    fprintf(file, "% 6d   % 6d\n", off[2], off[3]);
    fprintf(file, "   % 6d   % 6d\n", off[0], off[1]);
    break;
  case 4:
    for (i=3; i>=0; i--) {
      fprintf(file, "% 6d   % 6d   % 6d   % 6d\n", 
              off[12+16*i], off[13+16*i], 
              off[14+16*i], off[15+16*i]);
      fprintf(file, "   % 6d  %c% 6d   % 6d%c   % 6d\n", 
              off[ 8+16*i], (i==1||i==2)?'\\':' ',
              off[ 9+16*i], off[10+16*i], (i==1||i==2)?'\\':' ',
              off[11+16*i]);
      fprintf(file, "      % 6d  %c% 6d   % 6d%c   % 6d\n", 
              off[ 4+16*i], (i==1||i==2)?'\\':' ',
              off[ 5+16*i], off[ 6+16*i], (i==1||i==2)?'\\':' ',
              off[ 7+16*i]);
      fprintf(file, "         % 6d   % 6d   % 6d   % 6d\n", 
              off[ 0+16*i], off[ 1+16*i],
              off[ 2+16*i], off[ 3+16*i]);
      if (i) fprintf(file, "\n");
    }
    break;
  default:
    for (i=0; i<fd*fd*fd; i++) {
      fprintf(file, "  off[% 3d,% 3d,% 3d] = % 6d\n",
              i%fd, (i/fd)%fd, i/(fd*fd), off[i]);
    }
    break;
  }
}

#define PRINT_2(NN,C)                                  \
   fw = fw##NN##C;                                     \
   fprintf(file, " --" #NN "-->% 15.7f   % 15.7f\n", \
          (float)fw[0], (float)fw[1])
#define PRINT_4(NN,C)                                                      \
   fw = fw##NN##C;                                                         \
   fprintf(file, " --" #NN "-->% 15.7f   % 15.7f   % 15.7f   % 15.7f\n", \
          (float)fw[0], (float)fw[1], (float)fw[2], (float)fw[3])
#define PRINT_N(NN,C)                                   \
   fw = fw##NN##C;                                      \
   fprintf(file, " --" #NN "--> \n");                 \
   for (i=0; i<fd; i++)                                 \
     fprintf(file, "     % 5d : % 15.7f\n", i, (float)fw[i])

#define PRINTALL(HOW,C)                                 \
   if (ctx->needK[gageKernel00]) { HOW(00,C); }         \
   if (ctx->needK[gageKernel10]) { HOW(10,C); }         \
   if (ctx->needK[gageKernel11]) { HOW(11,C); }         \
   if (ctx->needK[gageKernel20]) { HOW(20,C); }         \
   if (ctx->needK[gageKernel21]) { HOW(21,C); }         \
   if (ctx->needK[gageKernel22]) { HOW(22,C); }

void
_gagePrint_fslw(FILE *file, gageContext *ctx) {
  int i, fd;
  gage_t *fslx, *fsly, *fslz, *fw,
    *fw000, *fw001, *fw002, 
    *fw100, *fw101, *fw102, 
    *fw110, *fw111, *fw112, 
    *fw200, *fw201, *fw202, 
    *fw210, *fw211, *fw212, 
    *fw220, *fw221, *fw222;

  /* float *p; */

  fd = 2*ctx->radius;
  fslx = ctx->fsl + fd*0;
  fsly = ctx->fsl + fd*1;
  fslz = ctx->fsl + fd*2;
  fw000 = ctx->fw + 0 + fd*(0 + 3*gageKernel00);
  fw001 = ctx->fw + 0 + fd*(1 + 3*gageKernel00);
  fw002 = ctx->fw + 0 + fd*(2 + 3*gageKernel00);
  fw100 = ctx->fw + 0 + fd*(0 + 3*gageKernel10);
  fw101 = ctx->fw + 0 + fd*(1 + 3*gageKernel10);
  fw102 = ctx->fw + 0 + fd*(2 + 3*gageKernel10);
  fw110 = ctx->fw + 0 + fd*(0 + 3*gageKernel11);
  fw111 = ctx->fw + 0 + fd*(1 + 3*gageKernel11);
  fw112 = ctx->fw + 0 + fd*(2 + 3*gageKernel11);
  fw200 = ctx->fw + 0 + fd*(0 + 3*gageKernel20);
  fw201 = ctx->fw + 0 + fd*(1 + 3*gageKernel20);
  fw202 = ctx->fw + 0 + fd*(2 + 3*gageKernel20);
  fw210 = ctx->fw + 0 + fd*(0 + 3*gageKernel21);
  fw211 = ctx->fw + 0 + fd*(1 + 3*gageKernel21);
  fw212 = ctx->fw + 0 + fd*(2 + 3*gageKernel21);
  fw220 = ctx->fw + 0 + fd*(0 + 3*gageKernel22);
  fw221 = ctx->fw + 0 + fd*(1 + 3*gageKernel22);
  fw222 = ctx->fw + 0 + fd*(2 + 3*gageKernel22);

  fprintf(file, "fsl -> fw: \n");
  switch(fd) {
  case 2:
    fprintf(file, "x[]: % 15.7f   % 15.7f\n",
            (float)fslx[0], (float)fslx[1]);
    PRINTALL(PRINT_2, 0);
    fprintf(file, "y[]: % 15.7f   % 15.7f\n",
            (float)fsly[0], (float)fsly[1]);
    PRINTALL(PRINT_2, 1);
    fprintf(file, "z[]: % 15.7f   % 15.7f\n",
            (float)fslz[0], (float)fslz[1]);
    PRINTALL(PRINT_2, 2);
    break;
  case 4:
    fprintf(file, "x[]: % 15.7f  % 15.7f  % 15.7f  % 15.7f\n", 
            (float)fslx[0], (float)fslx[1], (float)fslx[2], (float)fslx[3]);
    PRINTALL(PRINT_4, 0);
    fprintf(file, "y[]: % 15.7f  % 15.7f  % 15.7f  % 15.7f\n", 
            (float)fsly[0], (float)fsly[1], (float)fsly[2], (float)fsly[3]);
    PRINTALL(PRINT_4, 1);
    fprintf(file, "z[]: % 15.7f  % 15.7f  % 15.7f  % 15.7f\n", 
            (float)fslz[0], (float)fslz[1], (float)fslz[2], (float)fslz[3]);
    PRINTALL(PRINT_4, 2);
    break;
  default:
    fprintf(file, "x[]:\n");
    for (i=0; i<fd; i++)
      fprintf(file, "     % 5d : % 15.7f\n", i, (float)fslx[i]);
    PRINTALL(PRINT_N, 0);
    fprintf(file, "y[]:\n");
    for (i=0; i<fd; i++)
      fprintf(file, "     % 5d : % 15.7f\n", i, (float)fsly[i]);
    PRINTALL(PRINT_N, 1);
    fprintf(file, "z[]:\n");
    for (i=0; i<fd; i++)
      fprintf(file, "     % 5d : % 15.7f\n", i, (float)fslz[i]);
    PRINTALL(PRINT_N, 2);
    break;
  }
  return;
}

void
gageQueryPrint(FILE *file, const gageKind *kind, gageQuery query) {
  int ii;

  fprintf(file, "%s query = ...\n", kind->name);
  ii = kind->itemMax+1;
  do {
    ii--;
    if (GAGE_QUERY_ITEM_TEST(query, ii)) {
      fprintf(file, "    %3d: %s\n", ii, airEnumStr(kind->enm, ii));
    }
  } while (ii);
}
