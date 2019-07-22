// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CLIP_CASES_H
#define CLIP_CASES_H
#include <visit_vtk_light_exports.h>

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
//    Brad Whitlock, Tue Sep 23 09:59:23 PDT 2003
//    Added API so it builds on Windows.
//
//    Jeremy Meredith, Wed Jun 23 15:39:58 PDT 2004
//    Added voxel and pixel cases.  Not output shapes, though.
//
//    Jeremy Meredith, Tue Aug 29 13:52:33 EDT 2006
//    Added line segments and vertexes.
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
#define ST_VTX 106
#define ST_LIN 107
#define ST_PNT 108

// Colors
#define COLOR0  120
#define COLOR1  121
#define NOCOLOR 122

// Tables
extern VISIT_VTK_LIGHT_API int numClipCasesHex;
extern VISIT_VTK_LIGHT_API int numClipShapesHex[256];
extern VISIT_VTK_LIGHT_API int startClipShapesHex[256];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesHex[];

extern VISIT_VTK_LIGHT_API int numClipCasesVox;
extern VISIT_VTK_LIGHT_API int numClipShapesVox[256];
extern VISIT_VTK_LIGHT_API int startClipShapesVox[256];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesVox[];

extern VISIT_VTK_LIGHT_API int numClipCasesWdg;
extern VISIT_VTK_LIGHT_API int numClipShapesWdg[64];
extern VISIT_VTK_LIGHT_API int startClipShapesWdg[64];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesWdg[];

extern VISIT_VTK_LIGHT_API int numClipCasesPyr;
extern VISIT_VTK_LIGHT_API int numClipShapesPyr[32];
extern VISIT_VTK_LIGHT_API int startClipShapesPyr[32];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesPyr[];

extern VISIT_VTK_LIGHT_API int numClipCasesTet;
extern VISIT_VTK_LIGHT_API int numClipShapesTet[16];
extern VISIT_VTK_LIGHT_API int startClipShapesTet[16];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesTet[];

extern VISIT_VTK_LIGHT_API int numClipCasesQua;
extern VISIT_VTK_LIGHT_API int numClipShapesQua[16];
extern VISIT_VTK_LIGHT_API int startClipShapesQua[16];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesQua[];

extern VISIT_VTK_LIGHT_API int numClipCasesPix;
extern VISIT_VTK_LIGHT_API int numClipShapesPix[16];
extern VISIT_VTK_LIGHT_API int startClipShapesPix[16];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesPix[];

extern VISIT_VTK_LIGHT_API int numClipCasesTri;
extern VISIT_VTK_LIGHT_API int numClipShapesTri[8];
extern VISIT_VTK_LIGHT_API int startClipShapesTri[8];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesTri[];

extern VISIT_VTK_LIGHT_API int numClipCasesLin;
extern VISIT_VTK_LIGHT_API int numClipShapesLin[4];
extern VISIT_VTK_LIGHT_API int startClipShapesLin[4];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesLin[];

extern VISIT_VTK_LIGHT_API int numClipCasesVtx;
extern VISIT_VTK_LIGHT_API int numClipShapesVtx[2];
extern VISIT_VTK_LIGHT_API int startClipShapesVtx[2];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesVtx[];

extern VISIT_VTK_LIGHT_API int numClipCasesPoly5;
extern VISIT_VTK_LIGHT_API int numClipShapesPoly5[32];
extern VISIT_VTK_LIGHT_API int startClipShapesPoly5[32];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesPoly5[];

extern VISIT_VTK_LIGHT_API int numClipCasesPoly6;
extern VISIT_VTK_LIGHT_API int numClipShapesPoly6[64];
extern VISIT_VTK_LIGHT_API int startClipShapesPoly6[64];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesPoly6[];

extern VISIT_VTK_LIGHT_API int numClipCasesPoly7;
extern VISIT_VTK_LIGHT_API int numClipShapesPoly7[128];
extern VISIT_VTK_LIGHT_API int startClipShapesPoly7[128];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesPoly7[];

extern VISIT_VTK_LIGHT_API int numClipCasesPoly8;
extern VISIT_VTK_LIGHT_API int numClipShapesPoly8[256];
extern VISIT_VTK_LIGHT_API int startClipShapesPoly8[256];
extern VISIT_VTK_LIGHT_API unsigned char clipShapesPoly8[];

#endif
