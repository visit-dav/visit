// File      : VisItDataInterface_V1.h
//
// Programmer: Jeremy Meredith
// Date      : March 17, 2005
//

#ifndef VISIT_DATA_INTERFACE_V1_H
#define VISIT_DATA_INTERFACE_V1_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Mesh Types */
#define VISIT_MESHTYPE_RECTILINEAR        0
#define VISIT_MESHTYPE_CURVILINEAR        1
#define VISIT_MESHTYPE_UNSTRUCTURED       2
#define VISIT_MESHTYPE_POINT              3
#define VISIT_MESHTYPE_SURFACE            4
#define VISIT_MESHTYPE_UNKNOWN            5

/* Centering */
#define VISIT_VARCENTERING_NODE           0
#define VISIT_VARCENTERING_ZONE           1

/* Variable Type */
#define VISIT_VARTYPE_MESH                0
#define VISIT_VARTYPE_SCALAR              1
#define VISIT_VARTYPE_VECTOR              2
#define VISIT_VARTYPE_TENSOR              3
#define VISIT_VARTYPE_SYMMETRIC_TENSOR    4
#define VISIT_VARTYPE_MATERIAL            5
#define VISIT_VARTYPE_MATSPECIES          6
#define VISIT_VARTYPE_CURVE               7
#define VISIT_VARTYPE_UNKNOWN             8

typedef struct VisIt_MeshMetaData
{
    const char *name;
 
    int         meshType;  /* MESHTYPE */
    int         topologicalDimension;
    int         spatialDimension;

    int         numBlocks;
    const char *blockTitle;
    const char *blockPieceName;

    int         numGroups;
    const char *groupTitle;
    const char *groupPieceName;
    int        *groupIds;

    const char *units;
} VisIt_MeshMetaData;

typedef struct VisIt_ScalarMetaData
{
    const char *name;
    const char *meshName;

    int         centering;    /* VARCENTERING */
    int         treatAsASCII; /* boolean */
} VisIt_ScalarMetaData;

typedef struct VisIt_MaterialMetaData
{
    const char *name;
    const char *meshName;
    int         numMaterials;
    const char *materialNames[];
} VisIt_MaterialMetaData;

typedef struct VisIt_CurveMetaData
{
    const char *name;

    const char *xUnits;
    const char *yUnits;
} VisIt_CurveMetaData;

typedef struct VisIt_ExpressionMetaData
{
    const char *name;
    const char *definition;
    int         vartype;      /* VARTYPE */
} VisIt_ExpressionMetaData;

typedef struct VisIt_SimulationMetaData
{
    int      currentCycle;
    double   currentTime;

    int numMeshes;
    int numScalars;
    int numMaterials;
    int numCurves;
    int numExpressions;

    VisIt_MeshMetaData       *meshes;
    VisIt_ScalarMetaData     *scalars;
    VisIt_MaterialMetaData   *materials;
    VisIt_CurveMetaData      *curves;
    VisIt_ExpressionMetaData *expressions;
} VisIt_SimulationMetaData;

typedef struct VisIt_CurvilinearMesh
{
    int dims[3];
    int baseIndex[3];
    int firstRealZone[3];
    int lastRealZone[3];
    float *xcoords;
    float *ycoords;
    float *zcoords;
} VisIt_CurvilinearMesh;

typedef struct VisIt_RectilinearMesh
{
    int dims[3];
    int baseIndex[3];
    int firstRealZone[3];
    int lastRealZone[3];
    float *xcoords;
    float *ycoords;
    float *zcoords;
} VisIt_RectilinearMesh;

typedef struct VisIt_UnstructuredMesh
{
    /* FILL IN */
} VisIt_UnstructuredMesh;

typedef struct VisIt_PointMesh
{
    /* FILL IN */
} VisIt_PointMesh;

typedef struct VisIt_MeshData
{
    int                     meshType; /* MESHTYPE */
    VisIt_CurvilinearMesh  *cmesh;
    VisIt_RectilinearMesh  *rmesh;
    VisIt_UnstructuredMesh *umesh;
    VisIt_PointMesh        *pmesh;
} VisIt_MeshData;

typedef struct VisIt_ScalarData
{
    int len;
    float *data;
} VisIt_ScalarData;

typedef struct VisIt_MixedScalarData
{
    int len;
    float *data;
} VisIt_MixedScalarData;

typedef struct VisIt_MaterialData
{
    /* FILL IN */
} VisIt_MaterialData;

typedef struct VisIt_CurveData
{
    int len;
    float *x;
    float *y;
} VisIt_CurveData;

typedef struct VisIt_SimulationCallback
{
    VisIt_SimulationMetaData *(*GetMetaData)();
    VisIt_MeshData           *(*GetMesh)(int,const char*);
    VisIt_MaterialData       *(*GetMaterial)(int,const char*);
    VisIt_ScalarData         *(*GetScalar)(int,const char*);
    VisIt_CurveData          *(*GetCurve)(const char*);
    VisIt_MixedScalarData    *(*GetMixedScalar)(int,const char*);
} VisIt_SimulationCallback;

#ifdef __cplusplus
}
#endif

#endif
