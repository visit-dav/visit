#ifndef XDMF_WRITER
#define XDMF_WRITER

#if defined(_WIN32)
#  if defined(XDMF_WRITER_EXPORTS) || defined(xdmf_writer_ser_EXPORTS) || defined(xdmf_writer_par_EXPORTS)
#    define XDMF_WRITER_API __declspec(dllexport)
#  else
#    define XDMF_WRITER_API __declspec(dllimport)
#  endif
#  if defined(_MSC_VER)
//   Turn off warning about lack of DLL interface
#    pragma warning(disable:4251)
//   Turn off warning non-dll class is base for dll-interface class.
#    pragma warning(disable:4275)
//   Turn off warning about identifier truncation
#    pragma warning(disable:4786)
#  endif
#else
# if __GNUC__ >= 4 && (defined(XDMF_WRITER_EXPORTS) || defined(xdmf_writer_ser_EXPORTS) || defined(xdmf_writer_par_EXPORTS))
#   define XDMF_WRITER_API __attribute__ ((visibility("default")))
# else
#   define XDMF_WRITER_API /* hidden by default */
# endif
#endif

#define XDMF_SCALAR 0
#define XDMF_VECTOR 1

#define XDMF_CELL_CENTER 0
#define XDMF_NODE_CENTER 1

#define XDMF_FLOAT  0
#define XDMF_DOUBLE 1
#define XDMF_INT    2
#define XDMF_CHAR   3

#define XDMF_NOTOPOLOGY   0
#define XDMF_POLYVERTEX   1
#define XDMF_POLYLINE     2
#define XDMF_POLYGON      3
#define XDMF_TRI          4
#define XDMF_QUAD         5
#define XDMF_TET          6
#define XDMF_PYRAMID      7
#define XDMF_WEDGE        8
#define XDMF_HEX          9
#define XDMF_EDGE_3      10
#define XDMF_TRI_6       11
#define XDMF_TRI_7       12
#define XDMF_QUAD_6      13
#define XDMF_QUAD_8      14
#define XDMF_QUAD_9      15
#define XDMF_TET_10      16
#define XDMF_PYRAMID_13  17
#define XDMF_WEDGE_12    18
#define XDMF_WEDGE_15    19
#define XDMF_WEDGE_18    20
#define XDMF_HEX_20      21
#define XDMF_HEX_24      22
#define XDMF_HEX_27      23
#define XDMF_HEX_64      24
#define XDMF_HEX_125     25
#define XDMF_MIXED       26

#define XDMF_NULL_GRID -1

typedef struct XDMFFile
{
    int       type;
} XDMFFile;

typedef struct HDFFile
{
    int       type;
} HDFFile;

XDMFFile XDMF_WRITER_API *XdmfParallelCreate(const char *fileName,
    int nFiles, double time);
void XDMF_WRITER_API XdmfPutCurvMultiVar(XDMFFile *xdmfFile,
    const char *gridFileName, const char *gridName, int gridDataType,
    int nVars, char **varNames, int *varTypes, int *varCentering,
    int *varDataTypes, int nDims, int *dims, int *iBlock, int *nBlocks);
void XDMF_WRITER_API XdmfPutUcdMultiVar(XDMFFile *xdmfFile,
    const char *gridFileName, const char *gridName, int coordDataType,
    int nCoords, int coordDims, int cellType, int nCells,
    int connectivityLength, int nVars, char **varNames, int *varTypes,
    int *varCentering, int *varDataTypes, int iBlock, int nBlocks);
void XDMF_WRITER_API XdmfParallelClose(XDMFFile *xdmfFile);

HDFFile XDMF_WRITER_API *HdfParallelCreate(const char *fileName, int nFiles);
void XDMF_WRITER_API HdfPutCurvMultiMesh(HDFFile *hdfFile,
    int coordDataType, float *coords, int nDims, int *dims,
    int *iBlock, int *nBlocks);
void XDMF_WRITER_API HdfPutCurvMultiVar(HDFFile *hdfFile, int nVars,
    char **varNames, int *varTypes, int *varCentering, int *varDataTypes,
    void *vars, int nDims, int *dims, int *iBlock, int *nBlocks);
void XDMF_WRITER_API HdfPutUcdMultiMesh(HDFFile *hdfFile,
    int coordDataType, float *coords, int nCoords, int *connectivity,
    int connectivityLength, int iBlock, int nBlocks);
void XDMF_WRITER_API HdfPutUcdMultiVar(HDFFile *hdfFile, int nVars,
    char **varNames, int *varTypes, int *varCentering, int *varDataTypes,
    void *vars, int nDims, int nCoords, int nCells, int iBlock, int nBlocks);
void XDMF_WRITER_API HdfParallelClose(HDFFile *hdfFile);

XDMFFile XDMF_WRITER_API *XdmfCreate(const char *fileName, double time);
void XDMF_WRITER_API XdmfWriteCurvVar(XDMFFile *file, const char *gridFileName,
    const char *varFileName, const char *gridName, const char *coordName, 
    int gridDataType, int nVars, char **varNames, int *varTypes,
    int *varCentering, int *varDataTypes, int nDims, int *dims);
void XDMF_WRITER_API XdmfPutUcdGrid(XDMFFile *file, const char *gridFileName,
    const char *varFileName, const char *gridName, const char *coordName, 
    int coordDataType, int nCoords, int coordDims, const char *connectivityName,
    int cellType, int nCells, int connectivityLength, int nVars,
    char **varNames, int *varTypes, int *varCentering, int *varDataTypes);
void XDMF_WRITER_API XdmfStartMultiBlock(XDMFFile *xdmfFile,
    const char *blockName);
void XDMF_WRITER_API XdmfWriteCurvBlock(XDMFFile *file, const char *fileName,
    const char *blockName, const char *coordName, int iBlock,
    int gridDataType, int nVars, char **varNames, int *varTypes,
    int *varCentering, int *varDataTypes, int nDims, int *dims,
    int *baseIndex, int *ghostOffsets);
void XDMF_WRITER_API XdmfEndMultiBlock(XDMFFile *xdmfFile);
void XDMF_WRITER_API XdmfClose(XDMFFile *xdmfFile);

HDFFile XDMF_WRITER_API *HdfCreate(const char *fileName);
void XDMF_WRITER_API HdfPutCoords(HDFFile *hdfFile, const char *coordName,
    int coordDataType, float *coords, int nCoords);
void XDMF_WRITER_API HdfPutConnectivity(HDFFile *hdfFile,
    const char *connectivityName, int connectivityDataType,
    void *connectivity, int connectivityLength);
void XDMF_WRITER_API HdfPutCurvVar(HDFFile *hdfFile, const char *varName,
    int varType, int varCentering, int varDataType, void *var,
    int nDims, int *dims);
void XDMF_WRITER_API HdfPutUcdVar(HDFFile *hdfFile, const char *varName,
    int varType, int varCentering, int varDataType, void *var,
    int varDim, int nCoords, int nCells);
void XDMF_WRITER_API HdfClose(HDFFile *hdfFile);

#endif
