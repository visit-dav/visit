// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "ClipCases.h"

// This file is meant to be read and created by a program other than a
// compiler.  If you must modify it by hand, at least be nice to the 
// parser and don't add anything else to this file or rearrange it.

int numClipCasesVtx = 2;

int numClipShapesVtx[2] = {
  1,  1 };

int startClipShapesVtx[2] = {
  0, 3  };

unsigned char clipShapesVtx[] = {
 // Case #0: Unique case #1
  ST_VTX, COLOR0, P0, 
 // Case #1: Unique case #2
  ST_VTX, COLOR1, P0, 
 // Dummy
  0
};

