#ifndef SOLID_SPLIT_CASES_H
#define SOLID_SPLIT_CASES_H

// Programmer: Jeremy Meredith
// Date      : August 11, 2003

// Points of original cell (up to 8, for the hex)
#define P0     0
#define P1     1
#define P2     2
#define P3     3
#define P4     4
#define P5     5
#define P6     6
#define P7     7

// Edges of original cell (up to 12, for the hex)
#define EA     20
#define EB     21
#define EC     22
#define ED     23
#define EE     24
#define EF     25
#define EG     26
#define EH     27
#define EI     28
#define EJ     29
#define EK     30
#define EL     31

// New interpolated points (ST_PNT outputs)
#define N0     40
#define N1     41
#define N2     42
#define N3     43

// Shapes
#define ST_TET 100
#define ST_PYR 101
#define ST_WDG 102
#define ST_HEX 103
#define ST_PNT 104

// Colors
#define COLOR0 120
#define COLOR1 121

// Tables
extern int numSolidSplitCasesHex;
extern int numSolidSplitShapesHex[256];
extern int startSolidSplitShapesHex[256];
extern unsigned char solidSplitShapesHex[];

extern int numSolidSplitCasesWdg;
extern int numSolidSplitShapesWdg[64];
extern int startSolidSplitShapesWdg[64];
extern unsigned char solidSplitShapesWdg[];

extern int numSolidSplitCasesPyr;
extern int numSolidSplitShapesPyr[32];
extern int startSolidSplitShapesPyr[32];
extern unsigned char solidSplitShapesPyr[];

extern int numSolidSplitCasesTet;
extern int numSolidSplitShapesTet[16];
extern int startSolidSplitShapesTet[16];
extern unsigned char solidSplitShapesTet[];

#endif
