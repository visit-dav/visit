#include "ClipCases.h"

// Programmer: Jeremy Meredith
// Date      : September 18, 2003

// This file is meant to be read and created by a program other than a
// compiler.  If you must modify it by hand, at least be nice to the 
// parser and don't add anything else to this file or rearrange it.

int numClipCasesTri = 8;

int numClipShapesTri[8] = {
  1,  2,  2,  2,  2,  2,  2,  1  // cases 0 - 7
};

int startClipShapesTri[8] = {
  0, 5, 16, 27, 38, 49, 60, 71  // cases 0 - 7
};

unsigned char clipShapesTri[] = {
 // Case #0: Unique case #1
  ST_TRI, COLOR0, P0, P1, P2, 
 // Case #1: Unique case #2
  ST_QUA, COLOR0, P1, P2, EC, EA, 
  ST_TRI, COLOR1, P0, EA, EC, 
 // Case #2: (cloned #1)
  ST_QUA, COLOR0, P2, P0, EA, EB, 
  ST_TRI, COLOR1, P1, EB, EA, 
 // Case #3: Unique case #3
  ST_TRI, COLOR0, EC, EB, P2, 
  ST_QUA, COLOR1, P0, P1, EB, EC, 
 // Case #4: (cloned #1)
  ST_QUA, COLOR0, P0, P1, EB, EC, 
  ST_TRI, COLOR1, P2, EC, EB, 
 // Case #5: (cloned #3)
  ST_TRI, COLOR0, EB, EA, P1, 
  ST_QUA, COLOR1, P2, P0, EA, EB, 
 // Case #6: (cloned #3)
  ST_TRI, COLOR0, EA, EC, P0, 
  ST_QUA, COLOR1, P1, P2, EC, EA, 
 // Case #7: Unique case #4
  ST_TRI, COLOR1, P0, P1, P2, 
 // Dummy
  0
};

