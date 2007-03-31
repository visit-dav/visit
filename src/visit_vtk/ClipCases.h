#ifndef CLIP_CASES_H
#define CLIP_CASES_H

// Programmer: Jeremy Meredith
// Date      : August 11, 2003
//
// Modifications:
//    Jeremy Meredith, Mon Sep 15 17:24:15 PDT 2003
//    Added NOCOLOR.
//
//    Jeremy Meredith, Thu Sep 18 11:29:12 PDT 2003
//    Added quad and triangle cases and output shapes.
//

// Points of original cell (up to 8, for the hex)
// Note: we assume P0 is zero in several places.
// Note: we assume these values are contiguous and monotonic.
#define P0     0
#define P1     1
#define P2     2
#define P3     3
#define P4     4
#define P5     5
#define P6     6
#define P7     7

// Edges of original cell (up to 12, for the hex)
// Note: we assume these values are contiguous and monotonic.
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
// Note: we assume these values are contiguous and monotonic.
#define N0     40
#define N1     41
#define N2     42
#define N3     43

// Shapes
#define ST_TET 100
#define ST_PYR 101
#define ST_WDG 102
#define ST_HEX 103
#define ST_TRI 104
#define ST_QUA 105
#define ST_PNT 106

// Colors
#define COLOR0  120
#define COLOR1  121
#define NOCOLOR 122

// Tables
extern int numClipCasesHex;
extern int numClipShapesHex[256];
extern int startClipShapesHex[256];
extern unsigned char clipShapesHex[];

extern int numClipCasesWdg;
extern int numClipShapesWdg[64];
extern int startClipShapesWdg[64];
extern unsigned char clipShapesWdg[];

extern int numClipCasesPyr;
extern int numClipShapesPyr[32];
extern int startClipShapesPyr[32];
extern unsigned char clipShapesPyr[];

extern int numClipCasesTet;
extern int numClipShapesTet[16];
extern int startClipShapesTet[16];
extern unsigned char clipShapesTet[];

extern int numClipCasesQua;
extern int numClipShapesQua[16];
extern int startClipShapesQua[16];
extern unsigned char clipShapesQua[];

extern int numClipCasesTri;
extern int numClipShapesTri[8];
extern int startClipShapesTri[8];
extern unsigned char clipShapesTri[];

#endif
