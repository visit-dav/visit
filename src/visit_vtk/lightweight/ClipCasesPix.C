#include "ClipCases.h"

// note -- this was converted automatically from the quad cases
// using the clipeditor

// This file is meant to be read and created by a program other than a
// compiler.  If you must modify it by hand, at least be nice to the 
// parser and don't add anything else to this file or rearrange it.

int numClipCasesPix = 16;

int numClipShapesPix[16] = {
  1,  3,  3,  2,  3,  2,  4,  3, // cases 0 - 7
  3,  4,  2,  3,  2,  3,  3,  1  // cases 8 - 15
};

int startClipShapesPix[16] = {
  0, 6, 22, 38, 50, 66, 78, 100, // cases 0 - 7
  116, 132, 154, 166, 182, 194, 210, 226  // cases 8 - 15
};

unsigned char clipShapesPix[] = {
 // Case #0: Unique case #1
  ST_QUA, COLOR0, P0, P1, P3, P2, 
 // Case #1: Unique case #2
  ST_QUA, COLOR0, ED, EA, P1, P2, 
  ST_TRI, COLOR0, P2, P1, P3, 
  ST_TRI, COLOR1, P0, EA, ED, 
 // Case #2: (cloned #1)
  ST_QUA, COLOR0, EA, EB, P3, P0, 
  ST_TRI, COLOR0, P0, P3, P2, 
  ST_TRI, COLOR1, P1, EB, EA, 
 // Case #3: Unique case #3
  ST_QUA, COLOR0, ED, EB, P3, P2, 
  ST_QUA, COLOR1, P0, P1, EB, ED, 
 // Case #4: (cloned #1)
  ST_QUA, COLOR0, EC, ED, P0, P3, 
  ST_TRI, COLOR0, P3, P0, P1, 
  ST_TRI, COLOR1, P2, ED, EC, 
 // Case #5: (cloned #3)
  ST_QUA, COLOR0, EC, EA, P1, P3, 
  ST_QUA, COLOR1, P2, P0, EA, EC, 
 // Case #6: Unique case #4
  ST_TRI, COLOR0, EA, ED, P0, 
  ST_TRI, COLOR0, EC, EB, P3, 
  ST_QUA, COLOR1, P2, P1, EB, EC, 
  ST_QUA, COLOR1, P1, P2, ED, EA, 
 // Case #7: Unique case #5
  ST_TRI, COLOR0, EC, EB, P3, 
  ST_QUA, COLOR1, P2, P1, EB, EC, 
  ST_TRI, COLOR1, P0, P1, P2, 
 // Case #8: (cloned #1)
  ST_QUA, COLOR0, EB, EC, P2, P1, 
  ST_TRI, COLOR0, P1, P2, P0, 
  ST_TRI, COLOR1, P3, EC, EB, 
 // Case #9: (cloned #6)
  ST_TRI, COLOR0, EB, EA, P1, 
  ST_TRI, COLOR0, ED, EC, P2, 
  ST_QUA, COLOR1, P0, P3, EC, ED, 
  ST_QUA, COLOR1, P3, P0, EA, EB, 
 // Case #10: (cloned #3)
  ST_QUA, COLOR0, EA, EC, P2, P0, 
  ST_QUA, COLOR1, P1, P3, EC, EA, 
 // Case #11: (cloned #7)
  ST_TRI, COLOR0, ED, EC, P2, 
  ST_QUA, COLOR1, P0, P3, EC, ED, 
  ST_TRI, COLOR1, P1, P3, P0, 
 // Case #12: (cloned #3)
  ST_QUA, COLOR0, EB, ED, P0, P1, 
  ST_QUA, COLOR1, P3, P2, ED, EB, 
 // Case #13: (cloned #7)
  ST_TRI, COLOR0, EB, EA, P1, 
  ST_QUA, COLOR1, P3, P0, EA, EB, 
  ST_TRI, COLOR1, P2, P0, P3, 
 // Case #14: (cloned #7)
  ST_TRI, COLOR0, EA, ED, P0, 
  ST_QUA, COLOR1, P1, P2, ED, EA, 
  ST_TRI, COLOR1, P3, P2, P1, 
 // Case #15: Unique case #6
  ST_QUA, COLOR1, P0, P1, P3, P2, 
 // Dummy
  0
};

