#ifndef VTK_TRIANGULATION_TABLES_H
#define VTK_TRIANGULATION_TABLES_H
#include <visit_vtk_exports.h>

// ************************************************************************* //
//                         vtkTriangulationTables.h                          //
// ************************************************************************* //


extern VISIT_VTK_API int tetTriangulationTable[16][7];
extern VISIT_VTK_API int tetVerticesFromEdges[6][2];
extern VISIT_VTK_API int tetTriangleFaces[4][3];

extern VISIT_VTK_API int pyramidTriangulationTable[32][13];
extern VISIT_VTK_API int pyramidVerticesFromEdges[8][2];
extern VISIT_VTK_API int pyramidTriangleFaces[4][3];
extern VISIT_VTK_API int pyramidQuadFaces[1][4];

extern VISIT_VTK_API int wedgeTriangulationTable[64][13];
extern VISIT_VTK_API int wedgeVerticesFromEdges[9][2];
extern VISIT_VTK_API int wedgeTriangleFaces[2][3];
extern VISIT_VTK_API int wedgeQuadFaces[3][4];

extern VISIT_VTK_API int hexTriangulationTable[256][16];
extern VISIT_VTK_API int hexVerticesFromEdges[12][2];
extern VISIT_VTK_API int hexQuadFaces[6][4];

extern VISIT_VTK_API int triVerticesFromEdges[3][2];
extern VISIT_VTK_API int quadVerticesFromEdges[4][2];

#endif
