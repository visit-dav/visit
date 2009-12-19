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

#include "limn.h"

/*
** vertex, edge, and face numbering, and canonical edge arrangement
**
**     (6)---11---(7)          +----------+           +----------+     
**     /|         /|          /|         /|          /|      \  /|     
**    9 |       10 |         / |  /5/   / |         / |       \/ |     
**   /  6       /  7        /  |    |4|/  |        / \|       /\ |     
** (4)----8---(5)  |       +----------+   |       +----------+   |     
**  |   |      |   |       ||2||      ||3||       |   |  \   |   |     
**  |  (2)---3-|--(3)      |   +------|---+       |   +----\-|---+     
**  4  /       5  /        |  / |1|   |  /        |\ /       |\ /      
**  | 1        | 2         | /   /0/  | /         | /\       | /       
**  |/         |/          |/         |/          |/  \      |/
** (0)----0---(1)          +----------+           +----------+
**                                               canonical edge arrangement
**                                               creates 2 triangular and
**                                               1 hexagonal surface
*/

/*
** the limnContour3DContext's vidx cache uses this numbering
**      (.)--------(.) 
**      /|         /|  
**     4 |        / |  
**    /  |       /  |
**  (.)----3---(.)  |  
**   |   |      |   |  
**   |  (2)-----|--(.) 
**   2  /       |  /   
**   | 1        | /    
**   |/         |/     
**  (X)----0---(.)     
*/

int
_limnContour3DEdge[256] = {
  0x000, 0x013, 0x025, 0x036, 0x04A, 0x059, 0x06F, 0x07C,
  0x08C, 0x09F, 0x0A9, 0x0BA, 0x0C6, 0x0D5, 0x0E3, 0x0F0,
  0x310, 0x303, 0x335, 0x326, 0x35A, 0x349, 0x37F, 0x36C,
  0x39C, 0x38F, 0x3B9, 0x3AA, 0x3D6, 0x3C5, 0x3F3, 0x3E0,
  0x520, 0x533, 0x505, 0x516, 0x56A, 0x579, 0x54F, 0x55C,
  0x5AC, 0x5BF, 0x589, 0x59A, 0x5E6, 0x5F5, 0x5C3, 0x5D0,
  0x630, 0x623, 0x615, 0x606, 0x67A, 0x669, 0x65F, 0x64C,
  0x6BC, 0x6AF, 0x699, 0x68A, 0x6F6, 0x6E5, 0x6D3, 0x6C0,
  0xA40, 0xA53, 0xA65, 0xA76, 0xA0A, 0xA19, 0xA2F, 0xA3C,
  0xACC, 0xADF, 0xAE9, 0xAFA, 0xA86, 0xA95, 0xAA3, 0xAB0,
  0x950, 0x943, 0x975, 0x966, 0x91A, 0x909, 0x93F, 0x92C,
  0x9DC, 0x9CF, 0x9F9, 0x9EA, 0x996, 0x985, 0x9B3, 0x9A0,
  0xF60, 0xF73, 0xF45, 0xF56, 0xF2A, 0xF39, 0xF0F, 0xF1C,
  0xFEC, 0xFFF, 0xFC9, 0xFDA, 0xFA6, 0xFB5, 0xF83, 0xF90,
  0xC70, 0xC63, 0xC55, 0xC46, 0xC3A, 0xC29, 0xC1F, 0xC0C,
  0xCFC, 0xCEF, 0xCD9, 0xCCA, 0xCB6, 0xCA5, 0xC93, 0xC80,
  0xC80, 0xC93, 0xCA5, 0xCB6, 0xCCA, 0xCD9, 0xCEF, 0xCFC,
  0xC0C, 0xC1F, 0xC29, 0xC3A, 0xC46, 0xC55, 0xC63, 0xC70,
  0xF90, 0xF83, 0xFB5, 0xFA6, 0xFDA, 0xFC9, 0xFFF, 0xFEC,
  0xF1C, 0xF0F, 0xF39, 0xF2A, 0xF56, 0xF45, 0xF73, 0xF60,
  0x9A0, 0x9B3, 0x985, 0x996, 0x9EA, 0x9F9, 0x9CF, 0x9DC,
  0x92C, 0x93F, 0x909, 0x91A, 0x966, 0x975, 0x943, 0x950,
  0xAB0, 0xAA3, 0xA95, 0xA86, 0xAFA, 0xAE9, 0xADF, 0xACC,
  0xA3C, 0xA2F, 0xA19, 0xA0A, 0xA76, 0xA65, 0xA53, 0xA40,
  0x6C0, 0x6D3, 0x6E5, 0x6F6, 0x68A, 0x699, 0x6AF, 0x6BC,
  0x64C, 0x65F, 0x669, 0x67A, 0x606, 0x615, 0x623, 0x630,
  0x5D0, 0x5C3, 0x5F5, 0x5E6, 0x59A, 0x589, 0x5BF, 0x5AC,
  0x55C, 0x54F, 0x579, 0x56A, 0x516, 0x505, 0x533, 0x520,
  0x3E0, 0x3F3, 0x3C5, 0x3D6, 0x3AA, 0x3B9, 0x38F, 0x39C,
  0x36C, 0x37F, 0x349, 0x35A, 0x326, 0x335, 0x303, 0x310,
  0x0F0, 0x0E3, 0x0D5, 0x0C6, 0x0BA, 0x0A9, 0x09F, 0x08C,
  0x07C, 0x06F, 0x059, 0x04A, 0x036, 0x025, 0x013, 0x000
};

int
_limnContour3DTriangle[256][19] = {
  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 5, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 5, 2, 1, 4, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 3, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 3, 6, 0, 6, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 5, 2, 0, 0, 1, 2, 2, 1, 3, 1, 3, 6,-1,-1,-1,-1,-1,-1,-1},
  { 6, 4, 3, 4, 2, 3, 4, 5, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 2, 7, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4, 2, 7, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 7, 3, 0, 5, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 7, 3, 5, 5, 3, 1, 5, 1, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 2, 7, 1, 7, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 7, 6, 2, 6, 0, 2, 6, 4, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 6, 1, 7, 7, 1, 0, 7, 0, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 5, 7, 4, 7, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 9, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 9, 8, 1, 8, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 9, 0, 0, 9, 2, 9, 8, 2, 2, 8, 5,-1,-1,-1,-1,-1,-1,-1},
  { 9, 8, 1, 1, 8, 5, 1, 5, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 1, 8, 1, 3, 8, 8, 3, 9, 3, 6, 9,-1,-1,-1,-1,-1,-1,-1},
  { 8, 0, 9, 0, 6, 9, 0, 3, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1, 5, 2, 8, 2, 3, 8, 8, 3, 9, 3, 6, 9,-1,-1,-1,-1},
  { 5, 2, 8, 2, 3, 8, 8, 3, 9, 3, 6, 9,-1,-1,-1,-1,-1,-1,-1},
  { 2, 7, 3, 4, 9, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 9, 8, 1, 8, 0, 2, 7, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 3, 0, 4, 9, 3, 3, 9, 7, 7, 9, 8, 7, 8, 5,-1,-1,-1,-1},
  { 7, 3, 5, 5, 3, 8, 3, 1, 8, 8, 1, 9,-1,-1,-1,-1,-1,-1,-1},
  { 4, 1, 2, 4, 2, 8, 2, 7, 8, 7, 9, 8, 7, 6, 9,-1,-1,-1,-1},
  { 7, 6, 2, 6, 9, 2, 2, 9, 0, 9, 8, 0,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1, 5, 7, 8, 7, 9, 8, 7, 6, 9,-1,-1,-1,-1,-1,-1,-1},
  { 5, 7, 8, 7, 9, 8, 7, 6, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 5, 8,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4, 5, 8,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 2, 8,10, 2, 0, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 4, 2, 2, 4, 8, 2, 8,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 3, 6, 5, 8,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 3, 6, 0, 6, 4, 5, 8,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 3,10, 2, 3, 6,10,10, 6, 8, 8, 6, 1, 8, 1, 0,-1,-1,-1,-1},
  { 6, 4, 3, 4, 8, 3, 3, 8, 2, 8,10, 2,-1,-1,-1,-1,-1,-1,-1},
  { 5, 8, 2, 2, 8, 3, 8,10, 3, 3,10, 7,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4, 5, 8, 2, 2, 8, 3, 8,10, 3, 3,10, 7,-1,-1,-1,-1},
  { 8,10, 0, 0,10, 7, 0, 7, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 8, 1, 1, 8, 3, 8,10, 3, 3,10, 7,-1,-1,-1,-1,-1,-1,-1},
  {10, 7, 6,10, 6, 8, 6, 1, 8, 1, 5, 8, 1, 2, 5,-1,-1,-1,-1},
  {10, 5, 8, 7, 5,10, 7, 0, 5, 7, 6, 0, 6, 4, 0, 0, 2, 5,-1},
  { 6, 1, 7, 7, 1,10, 1, 0,10,10, 0, 8,-1,-1,-1,-1,-1,-1,-1},
  { 4, 8, 6, 6, 8,10, 6,10, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4,10, 5, 4, 9,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 9, 0, 9, 5, 0, 9,10, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 2, 0,10,10, 0, 4,10, 4, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1,10, 2, 1, 9,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 6, 9,10, 6,10, 3,10, 5, 3, 5, 1, 3, 5, 4, 1,-1,-1,-1,-1},
  { 0, 3, 5, 3, 6, 5, 5, 6,10, 6, 9,10,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1, 9,10, 6,10, 3, 6,10, 2, 3,-1,-1,-1,-1,-1,-1,-1},
  { 9,10, 6,10, 3, 6,10, 2, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 7, 9,10, 7, 3, 9, 9, 3, 4, 4, 3, 2, 4, 2, 5,-1,-1,-1,-1},
  { 0, 2, 5, 1, 2, 0, 1, 7, 2, 1, 9, 7, 9,10, 7, 7, 3, 2,-1},
  { 0, 4, 3, 3, 4, 7, 4, 9, 7, 7, 9,10,-1,-1,-1,-1,-1,-1,-1},
  {10, 7, 9, 9, 7, 3, 9, 3, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 2, 5, 1, 5, 4, 6,10, 7, 6, 9,10,-1,-1,-1,-1,-1,-1,-1},
  { 0, 2, 5, 6,10, 7, 6, 9,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1, 6,10, 7, 6, 9,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 6,10, 7, 6, 9,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 6,11, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4, 6,11, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 5, 2, 6,11, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 5, 2, 1, 4, 5, 6,11, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 3,11, 9, 3, 9, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 3, 4, 3, 9, 4, 3,11, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 9, 1, 0, 5, 9, 9, 5,11,11, 5, 2,11, 2, 3,-1,-1,-1,-1},
  { 3,11, 2,11, 9, 2, 2, 9, 5, 9, 4, 5,-1,-1,-1,-1,-1,-1,-1},
  { 7,11, 2, 2,11, 3,11, 9, 3, 3, 9, 6,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4, 7,11, 2, 2,11, 3,11, 9, 3, 3, 9, 6,-1,-1,-1,-1},
  {11, 5, 7,11, 9, 5, 5, 9, 0, 0, 9, 6, 0, 6, 3,-1,-1,-1,-1},
  {11, 9, 6, 7, 6,11, 7, 6, 1, 7, 1, 5, 5, 1, 4, 1, 6, 3,-1},
  { 9, 1,11, 1, 7,11, 1, 2, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 0, 9, 0, 2, 9, 9, 2,11, 2, 7,11,-1,-1,-1,-1,-1,-1,-1},
  { 1, 0, 9, 9, 0,11, 0, 5,11,11, 5, 7,-1,-1,-1,-1,-1,-1,-1},
  { 4, 5, 9, 5,11, 9, 5, 7,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 6,11, 4,11, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  {11, 8, 6, 8, 1, 6, 8, 0, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 5,11, 8, 5, 2,11,11, 2, 6, 6, 2, 0, 6, 0, 4,-1,-1,-1,-1},
  { 1, 6, 2, 2, 6, 5, 6,11, 5, 5,11, 8,-1,-1,-1,-1,-1,-1,-1},
  { 3,11, 1,11, 4, 1,11, 8, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 3,11, 0,11, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1, 3,11, 2,11, 5, 2,11, 8, 5,-1,-1,-1,-1,-1,-1,-1},
  { 3,11, 2,11, 5, 2,11, 8, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 3, 4, 6, 3, 2, 4, 4, 2, 8, 8, 2, 7, 8, 7,11,-1,-1,-1,-1},
  { 7, 3, 2,11, 3, 7,11, 1, 3,11, 8, 1, 8, 0, 1, 1, 6, 3,-1},
  { 0, 6, 3, 0, 4, 6, 5, 7,11, 5,11, 8,-1,-1,-1,-1,-1,-1,-1},
  { 1, 6, 3, 5, 7,11, 5,11, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 2, 4, 2, 7, 4, 4, 7, 8, 7,11, 8,-1,-1,-1,-1,-1,-1,-1},
  {11, 8, 7, 8, 2, 7, 8, 0, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1, 5, 7,11, 5,11, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 5, 7,11, 5,11, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  {10, 5,11, 5, 8,11,11, 8, 6, 8, 9, 6,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4,10, 5,11, 5, 8,11,11, 8, 6, 8, 9, 6,-1,-1,-1,-1},
  { 9, 0, 8, 9, 6, 0, 0, 6, 2, 2, 6,11, 2,11,10,-1,-1,-1,-1},
  { 4, 8, 9, 1, 9, 4, 1, 9,11, 1,11, 2, 2,11,10,11, 9, 6,-1},
  { 8, 9, 1, 8, 1, 5, 1, 3, 5, 3,10, 5, 3,11,10,-1,-1,-1,-1},
  { 4, 8, 9, 0, 8, 4, 0,10, 8, 0, 3,10, 3,11,10,10, 5, 8,-1},
  { 1, 8, 9, 1, 0, 8, 3,11,10, 3,10, 2,-1,-1,-1,-1,-1,-1,-1},
  { 3,11,10, 3,10, 2, 4, 8, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 5, 8, 2, 2, 8, 3, 8, 9, 3, 3, 9, 6, 7,11,10,-1,-1,-1,-1},
  { 0, 1, 4, 5, 8, 2, 2, 8, 3, 8, 9, 3, 3, 9, 6, 7,11,10,-1},
  { 8, 9, 0, 0, 9, 6, 0, 6, 3, 7,11,10,-1,-1,-1,-1,-1,-1,-1},
  { 4, 8, 1, 1, 8, 3, 8, 9, 3, 3, 9, 6, 7,11,10,-1,-1,-1,-1},
  { 9, 1, 8, 1, 5, 8, 1, 2, 5, 7,11,10,-1,-1,-1,-1,-1,-1,-1},
  { 4, 0, 9, 0, 2, 9, 9, 2, 8, 2, 5, 8, 7,11,10,-1,-1,-1,-1},
  { 1, 8, 9, 1, 0, 8, 7,11,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 8, 9, 7,11,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 6,11, 4, 4,11,10, 4,10, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 5, 0,10, 0, 1,10,10, 1,11, 1, 6,11,-1,-1,-1,-1,-1,-1,-1},
  { 2, 0,10,10, 0,11, 0, 4,11,11, 4, 6,-1,-1,-1,-1,-1,-1,-1},
  { 1, 6, 2, 2, 6,11, 2,11,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 3,11, 1,11,10, 1, 1,10, 4,10, 5, 4,-1,-1,-1,-1,-1,-1,-1},
  { 0, 3, 5, 3,10, 5, 3,11,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1, 3,11,10, 3,10, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 3,11,10, 3,10, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 6, 3, 4, 4, 3, 2, 4, 2, 5, 7,11,10,-1,-1,-1,-1,-1,-1,-1},
  { 5, 0, 2, 0, 1, 2, 2, 1, 3, 1, 6, 3, 7,11,10,-1,-1,-1,-1},
  { 0, 6, 3, 0, 4, 6, 7,11,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 6, 3, 7,11,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 2, 5, 1, 5, 4, 7,11,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 2, 5, 7,11,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1, 7,11,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 7,11,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 7,10,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4, 7,10,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 5, 2, 7,10,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 5, 2, 1, 4, 5, 7,10,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 3, 6, 7,10,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 3, 6, 0, 6, 4, 7,10,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 5, 2, 0, 0, 1, 2, 2, 1, 3, 1, 3, 6, 7,10,11,-1,-1,-1,-1},
  { 6, 4, 3, 4, 2, 3, 4, 5, 2, 7,10,11,-1,-1,-1,-1,-1,-1,-1},
  { 3,10,11, 3, 2,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4, 3,10,11, 3, 2,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 5, 3, 3, 5,10, 3,10,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 3, 1,11,11, 1,10, 1, 4,10,10, 4, 5,-1,-1,-1,-1,-1,-1,-1},
  { 1, 2, 6, 2,11, 6, 2,10,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 2,10, 0,10,11, 0, 0,11, 4,11, 6, 4,-1,-1,-1,-1,-1,-1,-1},
  { 5,10, 0, 0,10, 1,10,11, 1, 1,11, 6,-1,-1,-1,-1,-1,-1,-1},
  { 6, 4,11, 4,10,11, 4, 5,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 9, 8, 7,10,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 9, 8, 1, 8, 0, 7,10,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 9, 0, 0, 9, 2, 9, 8, 2, 2, 8, 5, 7,10,11,-1,-1,-1,-1},
  { 9, 8, 1, 1, 8, 5, 1, 5, 2, 7,10,11,-1,-1,-1,-1,-1,-1,-1},
  { 4, 1, 8, 1, 3, 8, 8, 3, 9, 3, 6, 9, 7,10,11,-1,-1,-1,-1},
  { 8, 0, 9, 0, 6, 9, 0, 3, 6, 7,10,11,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1, 5, 2, 8, 2, 3, 8, 8, 3, 9, 3, 6, 9, 7,10,11,-1},
  { 5, 2, 8, 2, 3, 8, 8, 3, 9, 3, 6, 9, 7,10,11,-1,-1,-1,-1},
  { 3,10,11, 3, 2,10, 4, 9, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 9, 8, 1, 8, 0, 3,10,11, 3, 2,10,-1,-1,-1,-1,-1,-1,-1},
  { 4, 9, 8, 0, 8, 4, 0, 8,10, 0,10, 3, 3,10,11,10, 8, 5,-1},
  { 8, 1, 9, 8, 5, 1, 1, 5, 3, 3, 5,10, 3,10,11,-1,-1,-1,-1},
  { 4, 9, 8, 1, 9, 4, 1,11, 9, 1, 2,11, 2,10,11,11, 6, 9,-1},
  { 9, 8, 0, 9, 0, 6, 0, 2, 6, 2,11, 6, 2,10,11,-1,-1,-1,-1},
  { 0, 4, 1,10,11, 5, 5,11, 8,11, 6, 8, 8, 6, 9,-1,-1,-1,-1},
  {10,11, 5, 5,11, 8,11, 6, 8, 8, 6, 9,-1,-1,-1,-1,-1,-1,-1},
  { 5,11, 7, 5, 8,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4, 5,11, 7, 5, 8,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  {11, 7, 8, 8, 7, 2, 8, 2, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 4, 2, 2, 4, 7, 4, 8, 7, 7, 8,11,-1,-1,-1,-1,-1,-1,-1},
  { 1, 3, 6, 5,11, 7, 5, 8,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 3, 6, 0, 6, 4, 5,11, 7, 5, 8,11,-1,-1,-1,-1,-1,-1,-1},
  { 7, 2, 3,11, 3, 7,11, 3, 1,11, 1, 8, 8, 1, 0, 1, 3, 6,-1},
  { 3, 6, 4, 3, 4, 2, 4, 8, 2, 8, 7, 2, 8,11, 7,-1,-1,-1,-1},
  { 3, 2,11,11, 2, 5,11, 5, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4, 3, 2,11,11, 2, 5,11, 5, 8,-1,-1,-1,-1,-1,-1,-1},
  { 0,11, 3, 0, 8,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 3, 1,11,11, 1, 4,11, 4, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 2, 6, 2, 5, 6, 6, 5,11, 5, 8,11,-1,-1,-1,-1,-1,-1,-1},
  { 5, 8,11, 5,11, 2,11, 6, 2, 6, 0, 2, 6, 4, 0,-1,-1,-1,-1},
  {11, 6, 8, 8, 6, 1, 8, 1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4,11, 6, 4, 8,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 9, 5, 5, 9,11, 5,11, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 9, 0, 9,11, 0, 0,11, 5,11, 7, 5,-1,-1,-1,-1,-1,-1,-1},
  { 4, 9, 0, 0, 9, 2, 9,11, 2, 2,11, 7,-1,-1,-1,-1,-1,-1,-1},
  { 9,11, 1, 1,11, 7, 1, 7, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  {11, 6, 9, 7, 6,11, 7, 1, 6, 7, 5, 1, 5, 4, 1, 1, 3, 6,-1},
  {11, 7, 5,11, 5, 9, 5, 0, 9, 0, 6, 9, 0, 3, 6,-1,-1,-1,-1},
  { 0, 4, 1, 7, 2,11, 2, 3,11,11, 3, 9, 3, 6, 9,-1,-1,-1,-1},
  { 7, 2,11, 2, 3,11,11, 3, 9, 3, 6, 9,-1,-1,-1,-1,-1,-1,-1},
  { 3, 2,11,11, 2, 9, 2, 5, 9, 9, 5, 4,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 9, 0, 9, 5, 9,11, 5,11, 2, 5,11, 3, 2,-1,-1,-1,-1},
  { 0, 4, 3, 3, 4, 9, 3, 9,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 3, 9,11, 3, 1, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 2, 5, 1, 5, 4, 6, 9,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 2, 5, 6, 9,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1, 6, 9,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 6, 9,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 6, 7,10, 6,10, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4, 6, 7,10, 6,10, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 5, 2, 6, 7,10, 6,10, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 5, 2, 1, 4, 5, 6, 7,10, 6,10, 9,-1,-1,-1,-1,-1,-1,-1},
  {10, 9, 7, 9, 3, 7, 9, 1, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 3, 4, 3, 7, 4, 4, 7, 9, 7,10, 9,-1,-1,-1,-1,-1,-1,-1},
  { 0, 5, 2, 1, 2, 0, 1, 2, 7, 1, 7, 9, 9, 7,10, 7, 2, 3,-1},
  { 7,10, 9, 7, 9, 3, 9, 4, 3, 4, 2, 3, 4, 5, 2,-1,-1,-1,-1},
  { 9, 6,10,10, 6, 3,10, 3, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4, 9, 6,10,10, 6, 3,10, 3, 2,-1,-1,-1,-1,-1,-1,-1},
  { 0, 5, 3, 3, 5, 6, 5,10, 6, 6,10, 9,-1,-1,-1,-1,-1,-1,-1},
  { 6,10, 9, 6, 3,10,10, 3, 5, 5, 3, 1, 5, 1, 4,-1,-1,-1,-1},
  { 1, 2,10, 1,10, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 2,10, 0,10, 4, 0,10, 9, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 0, 9, 9, 0, 5, 9, 5,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 5,10, 4,10, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 6, 8, 6,10, 8, 6, 7,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 6, 7, 1, 7,10, 1, 1,10, 0,10, 8, 0,-1,-1,-1,-1,-1,-1,-1},
  {10, 8, 5, 7, 5,10, 7, 5, 0, 7, 0, 6, 6, 0, 4, 0, 5, 2,-1},
  {10, 6, 7,10, 8, 6, 6, 8, 1, 1, 8, 5, 1, 5, 2,-1,-1,-1,-1},
  { 4, 1, 8, 1, 3, 8, 8, 3,10, 3, 7,10,-1,-1,-1,-1,-1,-1,-1},
  { 8, 0,10, 0, 7,10, 0, 3, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1, 5, 2, 8, 2, 3, 8, 8, 3,10, 3, 7,10,-1,-1,-1,-1},
  { 5, 2, 8, 2, 3, 8, 8, 3,10, 3, 7,10,-1,-1,-1,-1,-1,-1,-1},
  { 6, 3, 4, 4, 3, 8, 3, 2, 8, 8, 2,10,-1,-1,-1,-1,-1,-1,-1},
  { 3, 2,10, 3,10, 6,10, 8, 6, 8, 1, 6, 8, 0, 1,-1,-1,-1,-1},
  { 0, 6, 3, 0, 4, 6, 5,10, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 6, 3, 5,10, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 2, 4, 2, 8, 4, 2,10, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 2,10, 8, 2, 8, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1, 5,10, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 5,10, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 5, 8, 7, 7, 8, 9, 7, 9, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4, 5, 8, 7, 7, 8, 9, 7, 9, 6,-1,-1,-1,-1,-1,-1,-1},
  { 7, 2, 6, 6, 2, 9, 2, 0, 9, 9, 0, 8,-1,-1,-1,-1,-1,-1,-1},
  { 4, 2, 1, 4, 8, 2, 2, 8, 7, 7, 8, 9, 7, 9, 6,-1,-1,-1,-1},
  { 7, 5, 3, 5, 8, 3, 3, 8, 1, 8, 9, 1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 0, 3, 4, 3, 9, 3, 7, 9, 7, 8, 9, 7, 5, 8,-1,-1,-1,-1},
  { 1, 8, 9, 1, 0, 8, 2, 3, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 2, 3, 7, 4, 8, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 5, 8, 2, 2, 8, 3, 8, 9, 3, 3, 9, 6,-1,-1,-1,-1,-1,-1,-1},
  { 0, 1, 4, 5, 8, 2, 2, 8, 3, 8, 9, 3, 3, 9, 6,-1,-1,-1,-1},
  { 8, 9, 0, 0, 9, 6, 0, 6, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 8, 1, 1, 8, 3, 8, 9, 3, 3, 9, 6,-1,-1,-1,-1,-1,-1,-1},
  { 9, 1, 8, 1, 5, 8, 1, 2, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 0, 9, 0, 2, 9, 9, 2, 8, 2, 5, 8,-1,-1,-1,-1,-1,-1,-1},
  { 1, 8, 9, 1, 0, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 8, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 4, 7, 5, 4, 6, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 6, 7, 1, 7, 0, 1, 7, 5, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 7, 2, 6, 6, 2, 0, 6, 0, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 7, 2, 1, 6, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 7, 5, 3, 5, 1, 3, 5, 4, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 3, 7, 0, 7, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1, 2, 3, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 2, 3, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 6, 3, 4, 4, 3, 2, 4, 2, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 5, 0, 2, 0, 1, 2, 2, 1, 3, 1, 6, 3,-1,-1,-1,-1,-1,-1,-1},
  { 0, 6, 3, 0, 4, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 6, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 2, 5, 1, 5, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 2, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 0, 4, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
};

limnContour3DContext *
limnContour3DContextNew(void) {
  limnContour3DContext *lctx;

  lctx = (limnContour3DContext *)calloc(1, sizeof(limnContour3DContext));
  if (lctx) {
    lctx->nvol = NULL;
    lctx->lowerInside = AIR_FALSE;
    ELL_4M_IDENTITY_SET(lctx->transform);
    lctx->reverse = AIR_FALSE;
    lctx->spanSize = 300;
    lctx->nspanHist = nrrdNew();
    nrrdMaybeAlloc_va(lctx->nspanHist, nrrdTypeInt, 2, 
                      AIR_CAST(size_t, lctx->spanSize), 
                      AIR_CAST(size_t, lctx->spanSize));
    lctx->range = nrrdRangeNew(AIR_NAN, AIR_NAN);
    lctx->sx = 0;
    lctx->sy = 0;
    lctx->sz = 0;
    lctx->vidx = NULL;
    lctx->val = NULL;
    lctx->time = AIR_NAN;
  }  
  return lctx;
}

limnContour3DContext *
limnContour3DContextNix(limnContour3DContext *lctx) {

  if (lctx) {
    lctx->nspanHist = nrrdNuke(lctx->nspanHist);
    lctx->range = nrrdRangeNix(lctx->range);
    lctx->vidx = (int *)airFree(lctx->vidx);
    lctx->val = (double *)airFree(lctx->val);
    airFree(lctx);
  }
  return NULL;
}

int
_limnContour3DReverse(limnContour3DContext *lctx) {
  double det, rot[9];

  ELL_34M_EXTRACT(rot, lctx->transform);
  det = ELL_3M_DET(rot);
  return (!!lctx->lowerInside) ^ (det < 0);
}

int
limnContour3DLowerInsideSet(limnContour3DContext *lctx,
                            int lowerInside) {
  char me[]="limnContour3DLowerInsideSet", err[BIFF_STRLEN];

  if (!lctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  lctx->lowerInside = lowerInside;
  lctx->reverse = _limnContour3DReverse(lctx);
  return 0;
}

int
limnContour3DTransformSet(limnContour3DContext *lctx,
                          const double mat[16]) {
  char me[]="limnContour3DTransformSet", err[BIFF_STRLEN];
  double det;

  if (!( lctx && mat )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  det = ELL_4M_DET(mat);
  if (!det) {
    sprintf(err, "%s: transform had zero determinant", me);
    biffAdd(LIMN, err); return 1;
  }
  ELL_4M_COPY(lctx->transform, mat);
  lctx->reverse = _limnContour3DReverse(lctx);
  return 0;
}

int
limnContour3DVolumeSet(limnContour3DContext *lctx, const Nrrd *nvol) {
  char me[]="limnContour3DVolumeSet", err[BIFF_STRLEN];
  int minI, maxI, *spanHist, sx, sy, sz, ss, si, xi, yi, zi, vi;
  double tmp, min, max, (*lup)(const void *v, size_t I);
  void *data;

  if (!( lctx && nvol )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  if (nrrdCheck(nvol)) {
    sprintf(err, "%s: problem with volume", me);
    biffMove(LIMN, err, NRRD); return 1;
  }
  if (3 != nvol->dim) {
    sprintf(err, "%s: need 3-D volume (not %d-D)", me, nvol->dim);
    biffAdd(LIMN, err); return 1;
  }
  if (nrrdTypeBlock == nvol->type) {
    sprintf(err, "%s: can't isosurface %s type", me, 
            airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(LIMN, err); return 1;
  }

  lctx->nvol = nvol;
  /* set/allocate things related to volume dimensions */
  nrrdRangeSet(lctx->range, nvol, nrrdBlind8BitRangeFalse);
  if (!( lctx->sx == nvol->axis[0].size &&
         lctx->sy == nvol->axis[1].size &&
         lctx->sz == nvol->axis[2].size )) {
    lctx->sx = nvol->axis[0].size;
    lctx->sy = nvol->axis[1].size;
    lctx->sz = nvol->axis[2].size;
    airFree(lctx->vidx);
    airFree(lctx->val);
    lctx->vidx = (int *)calloc(5*lctx->sx*lctx->sy, sizeof(int));
    lctx->val = (double *)calloc(4*(lctx->sx+2)*(lctx->sy+2), sizeof(double));
  }
  /* compute span space histogram */
  ss = lctx->spanSize;
  spanHist = (int*)(lctx->nspanHist->data);
  for (si=0; si<ss*ss; si++) {
    spanHist[si] = 0;
  }
  sx = lctx->sx;
  sy = lctx->sy;
  sz = lctx->sz;
  lup = nrrdDLookup[nvol->type];
  data = nvol->data;
  for (zi=0; zi<sz-1; zi++) {
    for (yi=0; yi<sy-1; yi++) {
      for (xi=0; xi<sx-1; xi++) {
        vi = xi + sx*(yi + sy*zi);
        tmp = lup(data, vi + 0 + 0*sx + 0*sx*sy);
        min = max = tmp;
        tmp = lup(data, vi + 1 + 0*sx + 0*sx*sy);
        min = AIR_MIN(min, tmp);
        max = AIR_MAX(max, tmp);
        tmp = lup(data, vi + 0 + 1*sx + 0*sx*sy);
        min = AIR_MIN(min, tmp);
        max = AIR_MAX(max, tmp);
        tmp = lup(data, vi + 1 + 1*sx + 0*sx*sy);
        min = AIR_MIN(min, tmp);
        max = AIR_MAX(max, tmp);
        tmp = lup(data, vi + 0 + 0*sx + 1*sx*sy);
        min = AIR_MIN(min, tmp);
        max = AIR_MAX(max, tmp);
        tmp = lup(data, vi + 1 + 0*sx + 1*sx*sy);
        min = AIR_MIN(min, tmp);
        max = AIR_MAX(max, tmp);
        tmp = lup(data, vi + 0 + 1*sx + 1*sx*sy);
        min = AIR_MIN(min, tmp);
        max = AIR_MAX(max, tmp);
        tmp = lup(data, vi + 1 + 1*sx + 1*sx*sy);
        min = AIR_MIN(min, tmp);
        max = AIR_MAX(max, tmp);
        minI = airIndex(lctx->range->min, min, lctx->range->max, ss);
        maxI = airIndex(lctx->range->min, max, lctx->range->max, ss);
        spanHist[minI + ss*maxI]++;
      }
    }
  }

  return 0;
}

#define VAL(xx, yy, zz)  (val[4*( (xx) + (yy)*(sx+2) + spi) + (zz+1)])
void
_limnContour3DVoxelGrads(double vgrad[8][3], double *val,
                         int sx, int spi) {
  ELL_3V_SET(vgrad[0],
             VAL( 1,  0,  0) - VAL(-1,  0,  0),
             VAL( 0,  1,  0) - VAL( 0, -1,  0),
             VAL( 0,  0,  1) - VAL( 0,  0, -1));
  ELL_3V_SET(vgrad[1],
             VAL( 2,  0,  0) - VAL( 0,  0,  0),
             VAL( 1,  1,  0) - VAL( 1, -1,  0),
             VAL( 1,  0,  1) - VAL( 1,  0, -1));
  ELL_3V_SET(vgrad[2],
             VAL( 1,  1,  0) - VAL(-1,  1,  0),
             VAL( 0,  2,  0) - VAL( 0,  0,  0),
             VAL( 0,  1,  1) - VAL( 0,  1, -1));
  ELL_3V_SET(vgrad[3],
             VAL( 2,  1,  0) - VAL( 0,  1,  0),
             VAL( 1,  2,  0) - VAL( 1,  0,  0),
             VAL( 1,  1,  1) - VAL( 1,  1, -1));
  ELL_3V_SET(vgrad[4],
             VAL( 1,  0,  1) - VAL(-1,  0,  1),
             VAL( 0,  1,  1) - VAL( 0, -1,  1),
             VAL( 0,  0,  2) - VAL( 0,  0,  0));
  ELL_3V_SET(vgrad[5],
             VAL( 2,  0,  1) - VAL( 0,  0,  1),
             VAL( 1,  1,  1) - VAL( 1, -1,  1),
             VAL( 1,  0,  2) - VAL( 1,  0,  0));
  ELL_3V_SET(vgrad[6],
             VAL( 1,  1,  1) - VAL(-1,  1,  1),
             VAL( 0,  2,  1) - VAL( 0,  0,  1),
             VAL( 0,  1,  2) - VAL( 0,  1,  0));
  ELL_3V_SET(vgrad[7],
             VAL( 2,  1,  1) - VAL( 0,  1,  1),
             VAL( 1,  2,  1) - VAL( 1,  0,  1),
             VAL( 1,  1,  2) - VAL( 1,  1,  0));
}
#undef VAL

int
limnContour3DExtract(limnContour3DContext *lctx,
                     limnObject *cont, double isovalue) {
  char me[]="limnContour3DExtract", err[BIFF_STRLEN];
  int sx, sy, sz, xi, yi, zi, zpi, si, spi, partIdx, vidx[12],
    minI, maxI, valI, ss, *spanHist,
    estVoxNum, estFaceNum, estVertNum;
  double (*lup)(const void *, size_t);
  const void *data;
  int e2v[12][2] = {        /* maps edge index to corner vertex indices */
    {0, 1},  /*  0 */
    {0, 2},  /*  1 */
    {1, 3},  /*  2 */
    {2, 3},  /*  3 */
    {0, 4},  /*  4 */
    {1, 5},  /*  5 */
    {2, 6},  /*  6 */
    {3, 7},  /*  7 */
    {4, 5},  /*  8 */
    {4, 6},  /*  9 */
    {5, 7},  /* 10 */
    {6, 7}   /* 11 */
  };
  double vccoord[8][3] = {  /* vertex corner coordinates */
    {0, 0, 0},  /* 0 */
    {1, 0, 0},  /* 1 */
    {0, 1, 0},  /* 2 */
    {1, 1, 0},  /* 3 */
    {0, 0, 1},  /* 4 */
    {1, 0, 1},  /* 5 */
    {0, 1, 1},  /* 6 */
    {1, 1, 1}   /* 7 */
  };

  if (!( lctx && cont )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  if (!AIR_EXISTS(isovalue)) {
    sprintf(err, "%s: got non-existent isovalue", me);
    biffAdd(LIMN, err); return 1;
  }

  if (!( isovalue > lctx->range->min &&
         isovalue < lctx->range->max )) {
    /* can't have any surface */
    lctx->time = 0;
    return 0;
  }
  
  /* initialize output summary info */
  lctx->voxNum = 0;
  lctx->vertNum = 0;
  lctx->faceNum = 0;
  lctx->time = airTime();

  /* set up dual transform for normals */
  if (lctx->findNormals) {
    double matA[9], matB[9];
    ELL_34M_EXTRACT(matA, lctx->transform);
    ell_3m_inv_d(matB, matA);
    ELL_3M_TRANSPOSE(lctx->normalTransform, matB);
    if (!lctx->lowerInside) {
      ELL_3M_SCALE(lctx->normalTransform, -1, lctx->normalTransform);
    }
  }

  /* copy local variables */
  sx = lctx->sx;
  sy = lctx->sy;
  sz = lctx->sz;
  lup = nrrdDLookup[lctx->nvol->type];
  data = lctx->nvol->data;
  ss = lctx->spanSize;
  spanHist = (int*)(lctx->nspanHist->data);

  /* estimate number of voxels, faces, and vertices involved */
  estVoxNum = 0;
  valI = airIndex(lctx->range->min, isovalue, lctx->range->max, ss);
  for (minI=0; minI<=valI; minI++) {
    for (maxI=valI; maxI<ss; maxI++) {
      estVoxNum += spanHist[minI + ss*maxI];
    }
  }
  estFaceNum = (int)(estVoxNum*2.15);
  estVertNum = (int)(estVoxNum*1.15);
  
  /* start new part, and preset length of face and vert arrays */
  partIdx = limnObjectPartAdd(cont);
  limnObjectFaceNumPreSet(cont, partIdx, estFaceNum);
  limnObjectVertexNumPreSet(cont, partIdx, estVertNum);

  /* initialize per-slice stuff */
  for (yi=0; yi<sy; yi++) {
    for (xi=0; xi<sx; xi++) {
      si = xi + sx*yi;
      spi = (xi+1) + (sx+2)*(yi+1);
      lctx->vidx[0 + 5*si] = -1;
      lctx->vidx[1 + 5*si] = -1;
      lctx->vidx[2 + 5*si] = -1;
      lctx->vidx[3 + 5*si] = -1;
      lctx->vidx[4 + 5*si] = -1;
      lctx->val[0 + 4*spi] = AIR_NAN;
      lctx->val[1 + 4*spi] = lup(data, si + sx*sy*0) - isovalue;
      lctx->val[2 + 4*spi] = lctx->val[1 + 4*spi];
      lctx->val[3 + 4*spi] = lup(data, si + sx*sy*1) - isovalue;
    }
    ELL_4V_COPY(lctx->val + 4*(0    + (sx+2)*(yi+1)),
                lctx->val + 4*(1    + (sx+2)*(yi+1)));
    ELL_4V_COPY(lctx->val + 4*(sx+1 + (sx+2)*(yi+1)),
                lctx->val + 4*(sx   + (sx+2)*(yi+1)));
  }
  for (xi=0; xi<sx+2; xi++) {
    ELL_4V_COPY(lctx->val + 4*(xi + (sx+2)*0),
                lctx->val + 4*(xi + (sx+2)*1));
    ELL_4V_COPY(lctx->val + 4*(xi + (sx+2)*(sy+1)),
                lctx->val + 4*(xi + (sx+2)*sy));
  }

  /* set up vidx */
  /*                X      Y */
  vidx[0]  = 0 + 5*(0 + sx*0);
  vidx[1]  = 1 + 5*(0 + sx*0);
  vidx[2]  = 1 + 5*(1 + sx*0);
  vidx[3]  = 0 + 5*(0 + sx*1);
  vidx[4]  = 2 + 5*(0 + sx*0);
  vidx[5]  = 2 + 5*(1 + sx*0);
  vidx[6]  = 2 + 5*(0 + sx*1);
  vidx[7]  = 2 + 5*(1 + sx*1);
  vidx[8]  = 3 + 5*(0 + sx*0);
  vidx[9]  = 4 + 5*(0 + sx*0);
  vidx[10] = 4 + 5*(1 + sx*0);
  vidx[11] = 3 + 5*(0 + sx*1);

  /* go through all slices */
  for (zi=0; zi<sz-1; zi++) {
    zpi = AIR_MIN(sz-1, zi+2);
    /* shuffle up per-slice info */
    for (yi=0; yi<sy; yi++) {
      for (xi=0; xi<sx; xi++) {
        si = xi + sx*yi;
        spi = (xi+1) + (sx+2)*(yi+1);
        lctx->vidx[0 + 5*si] = lctx->vidx[3 + 5*si];
        lctx->vidx[1 + 5*si] = lctx->vidx[4 + 5*si];
        lctx->vidx[2 + 5*si] = -1;
        lctx->vidx[3 + 5*si] = -1;
        lctx->vidx[4 + 5*si] = -1;
        lctx->val[0 + 4*spi] = lctx->val[1 + 4*spi];
        lctx->val[1 + 4*spi] = lctx->val[2 + 4*spi];
        lctx->val[2 + 4*spi] = lctx->val[3 + 4*spi];
        lctx->val[3 + 4*spi] = lup(data, si + sx*sy*zpi) - isovalue;
      }
      ELL_4V_COPY(lctx->val + 4*(0    + (sx+2)*(yi+1)),
                  lctx->val + 4*(1    + (sx+2)*(yi+1)));
      ELL_4V_COPY(lctx->val + 4*(sx+1 + (sx+2)*(yi+1)),
                  lctx->val + 4*(sx   + (sx+2)*(yi+1)));
    }
    for (xi=0; xi<sx+2; xi++) {
      ELL_4V_COPY(lctx->val + 4*(xi + (sx+2)*0),
                  lctx->val + 4*(xi + (sx+2)*1));
      ELL_4V_COPY(lctx->val + 4*(xi + (sx+2)*(sy+1)),
                  lctx->val + 4*(xi + (sx+2)*sy));
    }
    /* triangulate slice */
    for (yi=0; yi<sy-1; yi++) {
      double vval[8], vgrad[8][3], vert[3], tvertA[4], tvertB[4], ww;
      unsigned char vcase;
      int ti, vi, ei, vi0, vi1, ecase, *tcase;
      unsigned int vii[3];
      for (xi=0; xi<sx-1; xi++) {
        si = xi + sx*yi;
        spi = (xi+1) + (sx+2)*(yi+1);
        /* learn voxel values */
        /*                     X   Y                 Z */
        vval[0] = lctx->val[4*(0 + 0*(sx+2) + spi) + 1];
        vval[1] = lctx->val[4*(1 + 0*(sx+2) + spi) + 1];
        vval[2] = lctx->val[4*(0 + 1*(sx+2) + spi) + 1];
        vval[3] = lctx->val[4*(1 + 1*(sx+2) + spi) + 1];
        vval[4] = lctx->val[4*(0 + 0*(sx+2) + spi) + 2];
        vval[5] = lctx->val[4*(1 + 0*(sx+2) + spi) + 2];
        vval[6] = lctx->val[4*(0 + 1*(sx+2) + spi) + 2];
        vval[7] = lctx->val[4*(1 + 1*(sx+2) + spi) + 2];
        /* determine voxel and edge case */
        vcase = 0;
        for (vi=0; vi<8; vi++) {
          vcase |= (vval[vi] > 0) << vi;
        }
        if (0 == vcase || 255 == vcase) {
          /* no triangles added here */
          continue;
        }
        /* set voxel corner gradients */
        if (lctx->findNormals) {
          _limnContour3DVoxelGrads(vgrad, lctx->val, sx, spi);
        }
        lctx->voxNum++;
        ecase = _limnContour3DEdge[vcase];
        /* create new vertices as needed */
        for (ei=0; ei<12; ei++) {
          if ((ecase & (1 << ei))
              && -1 == lctx->vidx[vidx[ei] + 5*si]) {
            int ovi;
            limnVertex *vtx;
            double tvec[3], grad[3], tlen;
            /* this edge is needed for triangulation,
               and, we haven't already created a vertex for it */
            vi0 = e2v[ei][0];
            vi1 = e2v[ei][1];
            ww = vval[vi0]/(vval[vi0] - vval[vi1]);
            ELL_3V_LERP(vert, ww, vccoord[vi0], vccoord[vi1]);
            ELL_4V_SET(tvertA, vert[0] + xi, vert[1] + yi, vert[2] + zi, 1);
            ELL_4MV_MUL(tvertB, lctx->transform, tvertA);
            ELL_4V_HOMOG(tvertB, tvertB);
            ovi = lctx->vidx[vidx[ei] + 5*si] =
              limnObjectVertexAdd(cont, partIdx,
                                  AIR_CAST(float, tvertB[0]),
                                  AIR_CAST(float, tvertB[1]),
                                  AIR_CAST(float, tvertB[2]));
            if (lctx->findNormals) {
              ELL_3V_LERP(grad, ww, vgrad[vi0], vgrad[vi1]);
              ELL_3MV_MUL(tvec, lctx->normalTransform, grad);
              vtx = cont->vert + ovi;
              ELL_3V_NORM_TT(vtx->worldNormal, float, tvec, tlen);
            }
            lctx->vertNum++;
            /*
            fprintf(stderr, "%s: vert %d (edge %d) of (%d,%d,%d) "
                    "at %g %g %g\n",
                    me, lctx->vidx[vidx[ei] + 5*si], ei, xi, yi, zi,
                    vert[0] + xi, vert[1] + yi, vert[2] + zi);
            */
          }
        }
        /* add triangles */
        ti = 0;
        tcase = _limnContour3DTriangle[vcase];
        while (-1 != tcase[0 + 3*ti]) {
          ELL_3V_SET(vii,
                     lctx->vidx[vidx[tcase[0 + 3*ti]] + 5*si],
                     lctx->vidx[vidx[tcase[1 + 3*ti]] + 5*si],
                     lctx->vidx[vidx[tcase[2 + 3*ti]] + 5*si]);
          if (lctx->reverse) {
            int tmpi;
            tmpi = vii[1]; vii[1] = vii[2]; vii[2] = tmpi;
          }
          limnObjectFaceAdd(cont, partIdx, 0, 3, vii);
          lctx->faceNum++;
          ti++;
        }
      }
    }
  }
  
  /* end time */
  lctx->time = airTime() - lctx->time;
  return 0;
}
