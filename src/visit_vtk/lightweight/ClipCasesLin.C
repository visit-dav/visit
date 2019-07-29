// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "ClipCases.h"

// This file is meant to be read and created by a program other than a
// compiler.  If you must modify it by hand, at least be nice to the 
// parser and don't add anything else to this file or rearrange it.

int numClipCasesLin = 4;

int numClipShapesLin[4] = {
  1,  2,  2,  1 };

int startClipShapesLin[4] = {
  0, 4, 12, 20  };

unsigned char clipShapesLin[] = {
 // Case #0: Unique case #1
  ST_LIN, COLOR0, P0, P1, 
 // Case #1: Unique case #2
  ST_LIN, COLOR0, EA, P1, 
  ST_LIN, COLOR1, P0, EA, 
 // Case #2: (cloned #1)
  ST_LIN, COLOR0, EA, P0, 
  ST_LIN, COLOR1, P1, EA, 
 // Case #3: Unique case #3
  ST_LIN, COLOR1, P0, P1, 
 // Dummy
  0
};

