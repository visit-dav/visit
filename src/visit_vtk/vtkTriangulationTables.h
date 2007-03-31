#ifndef VTK_TRIANGULATION_TABLES_H
#define VTK_TRIANGULATION_TABLES_H

// ************************************************************************* //
//                         vtkTriangulationTables.h                          //
// ************************************************************************* //


extern int tetTriangulationTable[16][7];
extern int tetVerticesFromEdges[6][2];
extern int tetTriangleFaces[4][3];

extern int pyramidTriangulationTable[32][13];
extern int pyramidVerticesFromEdges[8][2];
extern int pyramidTriangleFaces[4][3];
extern int pyramidQuadFaces[1][4];

extern int wedgeTriangulationTable[64][13];
extern int wedgeVerticesFromEdges[9][2];
extern int wedgeTriangleFaces[2][3];
extern int wedgeQuadFaces[3][4];

extern int hexTriangulationTable[256][16];
extern int hexVerticesFromEdges[12][2];
extern int hexQuadFaces[6][4];

#endif
