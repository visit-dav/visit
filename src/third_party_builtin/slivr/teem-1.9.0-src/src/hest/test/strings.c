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

char *me;

extern void
_airCloPrintStr(FILE *f, int indent, int already, int width, char *_str);

int
main() {
  char line[1025];
  
  strcpy(line, "bingo  \t\t   bob    \t   boasts   \n   bumperstickers  \n  ");
  airOneLinify(line);
  printf("|%s|\n", line);
  
  strcpy(line, " \t  \n  ");
  airOneLinify(line);
  printf("|%s|\n", line);

  strcpy(line, "Director, writor, and editor John Sayles goes happily "
         "against the grain with this tale, set in a fictional Latin "
         "American country and shot almost entirely in Spanish and Indian "
         "dialects.  The story follows a well-to-do physician who has trained "
         "young doctors to work in the countryside among local Mayan Indians. "
         "He now wants to find each of the 'ambassadors of health,' but as "
         "the film unfolds, he comes to realize that a civil war is engulfing "
         "his country and the Indians are practically ensalved.  The\tMen\t"
         "with\tGuns (Los\tHombres\tArmados) have forever left their mark, "
         "too, on "
         "his students.  Sayles based his idea for the film, shot in Mexico, "
         "on the 36-year long civil war in Guatemala, which began in 1960.");
  printf("airStrlen(line) = %d\n", (int)airStrlen(line));

  fprintf(stdout, "This was found on my desk: ");
  _hestPrintStr(stdout, 10, strlen("This was found on my desk: "),
                80, line, AIR_FALSE);

  fprintf(stdout, "This was found on my desk: ");
  _hestPrintStr(stdout, 10, strlen("This was found on my desk: "),
                79, line, AIR_FALSE);

  fprintf(stdout, "This was found on my desk: ");
  _hestPrintStr(stdout, 10, strlen("This was found on my desk: "),
                78, line, AIR_FALSE);

  fprintf(stdout, "This was found on my desk: ");
  _hestPrintStr(stdout, 10, strlen("This was found on my desk: "),
                77, line, AIR_FALSE);

  fprintf(stdout, "This was found on my desk: ");
  _hestPrintStr(stdout, 10, strlen("This was found on my desk: "),
                76, line, AIR_FALSE);
  
  exit(0);
}

