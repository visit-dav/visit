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

#define XDMF_ZONE_CENTER 0
#define XDMF_NODE_CENTER 1

#define XDMF_FLOAT  0
#define XDMF_DOUBLE 1
#define XDMF_INT    2
#define XDMF_CHAR   3

#define XDMF_NULL_BLOCK -1

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
void XDMF_WRITER_API XdmfParallelClose(XDMFFile *xdmfFile);

HDFFile XDMF_WRITER_API *HdfParallelCreate(const char *fileName, int nFiles);
void XDMF_WRITER_API HdfPutCurvMultiMesh(HDFFile *hdfFile,
    const char *gridName, int gridDataType, float *gridCoords,
    int nDims, int *dims, int *iBlock, int *nBlocks);
void XDMF_WRITER_API HdfPutCurvMultiVar(HDFFile *hdfFile, int nVars,
    char **varNames, int *varTypes, int *varCentering, int *varDataTypes,
    void *vars, int nDims, int *dims, int *iBlock, int *nBlocks);
void XDMF_WRITER_API HdfParallelClose(HDFFile *hdfFile);

XDMFFile XDMF_WRITER_API *XdmfCreate(const char *fileName, double time);
void XDMF_WRITER_API XdmfWriteCurvVar(XDMFFile *file, const char *gridFileName,
    const char *varFileName, const char *gridName, const char *coordName, 
    int gridDataType, int nVars, char **varNames, int *varTypes,
    int *varCentering, int *varDataTypes, int nDims, int *dims);
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
void XDMF_WRITER_API HdfPutCurvMesh(HDFFile *hdfFile, const char *gridCoordName,
    int gridDataType, float *gridCoords, int nDims, int *dims);
void XDMF_WRITER_API HdfPutCurvVar(HDFFile *hdfFile, const char *varName,
    int varType, int varCentering, int varDataType, void *var,
    int nDims, int *dims);
void XDMF_WRITER_API HdfClose(HDFFile *hdfFile);

#endif
