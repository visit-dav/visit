#ifndef VTK_TRIANGULATION_TABLES_H
#define VTK_TRIANGULATION_TABLES_H
#include <visit_vtk_light_exports.h>

// ************************************************************************* //
//                         vtkTriangulationTables.h                          //
// ************************************************************************* //


extern VISIT_VTK_LIGHT_API int tetTriangulationTable[16][7];
extern VISIT_VTK_LIGHT_API int tetVerticesFromEdges[6][2];
extern VISIT_VTK_LIGHT_API int tetTriangleFaces[4][3];

extern VISIT_VTK_LIGHT_API int pyramidTriangulationTable[32][13];
extern VISIT_VTK_LIGHT_API int pyramidVerticesFromEdges[8][2];
extern VISIT_VTK_LIGHT_API int pyramidTriangleFaces[4][3];
extern VISIT_VTK_LIGHT_API int pyramidQuadFaces[1][4];

extern VISIT_VTK_LIGHT_API int wedgeTriangulationTable[64][13];
extern VISIT_VTK_LIGHT_API int wedgeVerticesFromEdges[9][2];
extern VISIT_VTK_LIGHT_API int wedgeTriangleFaces[2][3];
extern VISIT_VTK_LIGHT_API int wedgeQuadFaces[3][4];

extern VISIT_VTK_LIGHT_API int hexTriangulationTable[256][16];
extern VISIT_VTK_LIGHT_API int hexVerticesFromEdges[12][2];
extern VISIT_VTK_LIGHT_API int hexQuadFaces[6][4];

extern VISIT_VTK_LIGHT_API int voxTriangulationTable[256][16];
extern VISIT_VTK_LIGHT_API int voxVerticesFromEdges[12][2];
extern VISIT_VTK_LIGHT_API int voxQuadFaces[6][4];

extern VISIT_VTK_LIGHT_API int triVerticesFromEdges[3][2];
extern VISIT_VTK_LIGHT_API int quadVerticesFromEdges[4][2];
extern VISIT_VTK_LIGHT_API int pixelVerticesFromEdges[4][2];

#endif
