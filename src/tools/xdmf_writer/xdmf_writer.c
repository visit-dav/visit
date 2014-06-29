#include <xdmf_writer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#ifdef PARALLEL
#include <mpi.h>
#endif
#include <hdf5.h>

#define NX 2

typedef struct XDMFFileParallel
{
    int       type;
    char     *fileName;
    int       nFiles;
    int       nProcs;
    int       iProc;
} XDMFFileParallel;

typedef struct HDFFileParallel
{
    int       type;
    char     *fileName;
    int       nFiles;
    int       nProcs;
    int       iProc;
} HDFFileParallel;

typedef struct XDMFFileSerial
{
    int       type;
    FILE     *file;
} XDMFFileSerial;

typedef struct HDFFileSerial
{
    int       type;
    hid_t     fileId;
} HDFFileSerial;

const char *varTypeToString[2] = {"AttributeType=\"Scalar\"",
                                  "AttributeType=\"Vector\""};
const char *centeringToString[2] = {"Center=\"Cell\"",
                                    "Center=\"Node\""};
const char *dataTypeToString[4] = {"NumberType=\"Float\" Precision=\"4\"",
                                   "NumberType=\"Float\" Precision=\"8\"",
                                   "NumberType=\"Int\" Precision=\"4\"",
                                   "NumberType=\"Char\" Precision=\"1\""};
const char *cellTypeToString[27] = {"TopologyType=\"NoTopology\"",
                                    "TopologyType=\"Polyvertex\"",
                                    "TopologyType=\"Polyline\"",
                                    "TopologyType=\"Polygon\"",
                                    "TopologyType=\"Triangle\"",
                                    "TopologyType=\"Quadrilateral\"",
                                    "TopologyType=\"Tetrahedron\"",
                                    "TopologyType=\"Pyramid\"",
                                    "TopologyType=\"Wedge\"",
                                    "TopologyType=\"Hexahedron\"",
                                    "TopologyType=\"Edge_3\"",
                                    "TopologyType=\"Triangle_3\"",
                                    "TopologyType=\"Triangle_6\"",
                                    "TopologyType=\"Triangle_7\"",
                                    "TopologyType=\"Quadrilateral_6\"",
                                    "TopologyType=\"Quadrilateral_8\"",
                                    "TopologyType=\"Quadrilateral_9\"",
                                    "TopologyType=\"Tetrahedron_10\"",
                                    "TopologyType=\"Pyramid_13\"",
                                    "TopologyType=\"Wedge_12\"",
                                    "TopologyType=\"Wedge_15\"",
                                    "TopologyType=\"Hexahedron_20\"",
                                    "TopologyType=\"Hexahedron_24\"",
                                    "TopologyType=\"Hexahedron_27\"",
                                    "TopologyType=\"Hexahedron_64\"",
                                    "TopologyType=\"Hexahedron_125\"",
                                    "TopologyType=\"Mixed\""};
hid_t dataTypeToHDFType[4];

struct timeval hdf5_create_start;
struct timeval hdf5_create_end;
struct timeval hdf5_put_multi_start;
struct timeval hdf5_put_multi_ghost_created;
struct timeval hdf5_put_multi_write_start;
struct timeval hdf5_put_multi_write_end;
struct timeval hdf5_put_multi_batton_passed;
struct timeval hdf5_close_start;

int timeval_subtract(struct timeval *result, struct timeval *x,
    struct timeval *y);

void XDMFWriteCurvBlock(FILE *xmf, const char *gridFileName,
    const char *varFileName, const char *blockName, const char *coordName,
    int iBlock, int gridDataType, int nVars, char **varNames, int *varTypes,
    int *varCentering, int *varDataTypes, int nDims, int *dims,
    int *baseIndex, int *ghostOffsets);
void XDMFPutUcdGrid(FILE *xmf, const char *gridFileName,
    const char *varFileName, const char *gridName, int iBlock,
    const char *coordName, int coordDataType, int nCoords, int coordDims,
    const char *connectivityName, int cellType, int nCells,
    int connectivityLength, int nVars, char **varNames, int *varTypes,
    int *varCentering, int *varDataTypes);
void HdfWriteCurvMeshBlock(HDFFileParallel *hdfFile, int coordDataType,
    float *coords, int nDims, int *dims);
void HdfWriteCurvVarBlock(HDFFileParallel *hdfFile, int nVars, char **varNames,
    int *varTypes, int *varCentering, int *varDataTypes, void *vars,
    int nDims, int *dims);
void HdfWriteCurvBlock(HDFFileParallel *hdfFile, const char *gridName,
    int gridDataType, float *gridCoords, int nVars, char **varNames,
    int *varTypes, int *varCentering, int *varDataTypes, void *vars,
    int nDims, int *dims);
void HdfWriteUcdMeshBlock(HDFFileParallel *hdfFile, int coordDataType,
    float *coords, int nCoords, int *connectivity, int connectivityLength);
void HdfWriteUcdVarBlock(HDFFileParallel *hdfFile, int nVars, char **varNames,
    int *varTypes, int *varCentering, int *varDataTypes, void *vars,
    int nDims, int nCoords, int nCells);

void DetermineGhostInfo(int nDims, int *dims, int *iBlock, int *nBlocks,
    int *newDims, int *baseIndex, int *ghostOffsets);
void StartExchange3D(int exchangeProc, float *var, int nComps,
    int min[3], int max[3], int dims[3], int recvTag, int sendTag);
void FinishExchange3D(float *var, int nComps, int min[3], int max[3],
    int dims[3]);
void StartSend3D(int exchangeProc, float *var, int nComps,
    int min[3], int max[3], int dims[3], int sendTag);
void StartReceive3D(int exchangeProc, float *var, int nels, int recvTag);
void FinishReceive3D(float *var, int nComps, int min[3], int max[3],
    int dims[3]);
void Create3DNodalGhostData(float *var, int *dims, int nComps, int *iBlock,
    int *nBlocks, float **newVar, int newDims[3]);
void Create3DZonalGhostData(float *var, int *dims, int nComps, int *iBlock,
    int *nBlocks, float **newVar, int newDims[3]);
void Create3DNodalGapData(float *var, int *dims, int nComps, int *iBlock,
    int *nBlocks, float **newVar, int newDims[3]);

XDMFFile *
XdmfParallelCreate(const char *fileName, int nFiles, double time)
{
    XDMFFileParallel *xdmfFile;

    xdmfFile = (XDMFFileParallel *) malloc(sizeof(XDMFFileParallel));

#ifdef PARALLEL
    MPI_Comm_size(MPI_COMM_WORLD, &(xdmfFile->nProcs));
    MPI_Comm_rank(MPI_COMM_WORLD, &(xdmfFile->iProc));
#else
    xdmfFile->nProcs = 1;
    xdmfFile->iProc = 0;
#endif

    xdmfFile->type = 1;
    xdmfFile->fileName = (char  *) malloc(strlen(fileName)+1);
    strcpy (xdmfFile->fileName, fileName);
    xdmfFile->nFiles = nFiles;

    //
    // Only processor 0 creates the file.
    //
    if (xdmfFile->iProc == 0)
    {
        char *fName = (char *) malloc(strlen(xdmfFile->fileName)+4+1);
        sprintf(fName, "%s.xmf", xdmfFile->fileName);

        FILE *xmf = fopen(fName, "w");

        free(fName);

        fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
        fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
        fprintf(xmf, "<Xdmf Version=\"2.0\">\n");
        fprintf(xmf, " <Information Name=\"Time\" Value=\"%g\"/>\n", time);

        fprintf(xmf, " <Domain>\n");

        fclose(xmf);
    }

#ifdef PARALLEL
    //
    // Let processor zero write data.
    //
    int buf[1];
    buf[0] = 1;

    // FIX_ME: This hangs if nProcs < NX (processor 0 doing a blocking
    //         send to itself)
    if ((xdmfFile->iProc % NX == 0) &&
        (xdmfFile->iProc + NX >= xdmfFile->nProcs))
        MPI_Send(buf, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
#endif

    return (XDMFFile *) xdmfFile;
}

void
XdmfPutCurvMultiVar(XDMFFile *xdmfFileIn, const char *gridFileName,
    const char *gridName, int gridDataType, int nVars, char **varNames,
    int *varTypes, int *varCentering, int *varDataTypes, int nDims, int *dims,
    int *iBlock, int *nBlocks)
{
    XDMFFileParallel *xdmfFile = (XDMFFileParallel *) xdmfFileIn;

    int newDims[3];
    int baseIndex[3];
    int ghostOffsets[6];

    DetermineGhostInfo(nDims, dims, iBlock, nBlocks, newDims,
        baseIndex, ghostOffsets);

#ifdef PARALLEL
    //
    // Every Nth processor collects the data from other processors and
    // writes the information.
    //
    if (xdmfFile->iProc % NX == 0)
    {
        //
        // Collect the dimension, base index, and ghost zone offset
        // information from processors iProc through iProc + nCollect - 1.
        //
        int nCollect = (xdmfFile->nProcs - xdmfFile->iProc) < NX ?
                       (xdmfFile->nProcs - xdmfFile->iProc) : NX;

        int *info = (int *) malloc(nCollect * nDims * 4 * sizeof(int));
     
        int i;
        for (i = 0; i < nDims; i++)
        {
            info[i] = newDims[i];
            info[nDims+i] = baseIndex[i];
            info[nDims*2+i*2]   = ghostOffsets[i*2];
            info[nDims*2+i*2+1] = ghostOffsets[i*2+1];
        }
       
        if (nCollect > 1)
        {
            MPI_Request *req =
                (MPI_Request *) malloc((nCollect-1) * sizeof(MPI_Request));
            MPI_Status *statuses =
                (MPI_Status *) malloc((nCollect-1) * sizeof(MPI_Status));

            for (i = 1; i < nCollect; i++)
            {
                MPI_Irecv(&(info[i*nDims*4]), nDims*4, MPI_INT,
                    xdmfFile->iProc+i, 1, MPI_COMM_WORLD, &(req[i-1]));
            }
            MPI_Waitall(nCollect - 1, req, statuses);

            free(req);
            free(statuses);
        }

        //
        // Wait for my turn to write the data.
        //
        MPI_Status status;

        int buf[1];
        buf[0] = 1;

        int iGroup = xdmfFile->iProc / NX;
        int nGroup = (xdmfFile->nProcs + NX - 1) / NX;
        int battonProc = ((iGroup - 1 + nGroup) % nGroup) * NX;
        MPI_Recv(buf, 1, MPI_INT, battonProc, 2, MPI_COMM_WORLD, &status);

        //
        // Write the data.
        //
        char *fName = (char *) malloc(strlen(xdmfFile->fileName)+4+1);
        sprintf(fName, "%s.xmf", xdmfFile->fileName);

        FILE *xmf = fopen(fName, "a");

        free(fName);

        if (xdmfFile->iProc == 0)
            fprintf(xmf, "  <Grid Name=\"%s\" GridType=\"Collection\">\n",
                    gridName);

        char coordName[4];
        if (nDims == 2)
            strcpy(coordName, "XY");
        else
            strcpy(coordName, "XYZ");
        int iBlock;
        for (iBlock = 0; iBlock < nCollect; iBlock++)
        {
            char *gridFile = (char *) malloc(strlen(gridFileName)+5+3+1);
            char *varFile = (char *) malloc(strlen(xdmfFile->fileName)+5+3+1);
            int iFile = (xdmfFile->iProc + iBlock) /
                ((xdmfFile->nProcs + xdmfFile->nFiles - 1) / xdmfFile->nFiles);
            if (xdmfFile->nProcs == 1)
            {
                sprintf(gridFile, "%s.h5", gridFileName);
                sprintf(varFile, "%s.h5", xdmfFile->fileName);
            }
            else
            {
                sprintf(gridFile, "%s_%04d.h5", gridFileName, iFile);
                sprintf(varFile, "%s_%04d.h5", xdmfFile->fileName, iFile);
            }
            XDMFWriteCurvBlock(xmf, gridFile, varFile, "block", coordName,
                                xdmfFile->iProc+iBlock, gridDataType,
                                nVars, varNames, varTypes, varCentering,
                                varDataTypes, nDims,
                                &(info[iBlock*nDims*4]), 
                                &(info[iBlock*nDims*4+nDims]),
                                &(info[iBlock*nDims*4+nDims*2]));
            free(gridFile);
            free(varFile);
        }

        if (xdmfFile->iProc + nCollect == xdmfFile->nProcs)
            fprintf(xmf, "  </Grid>\n");

        fclose(xmf);

        //
        // Let the next processor write the data.
        //
        battonProc = ((iGroup + 1 + nGroup) % nGroup) * NX;
        MPI_Send(buf, 1, MPI_INT, battonProc, 2, MPI_COMM_WORLD);

        free(info);
    }
    else
    {
        int *info = (int *) malloc(nDims * 4 * sizeof(int));
     
        int i;
        for (i = 0; i < nDims; i++)
        {
            info[i] = newDims[i];
            info[nDims+i] = baseIndex[i];
            info[nDims*2+i*2]   = ghostOffsets[i*2];
            info[nDims*2+i*2+1] = ghostOffsets[i*2+1];
        }
       
        int destProc = xdmfFile->iProc - (xdmfFile->iProc % NX);
        MPI_Send(info, nDims*4, MPI_INT, destProc, 1, MPI_COMM_WORLD);

        free(info);
    }
#else
    char *fName = (char *) malloc(strlen(xdmfFile->fileName)+4+1);
    sprintf(fName, "%s.xmf", xdmfFile->fileName);

    FILE *xmf = fopen(fName, "a");

    free(fName);

    fprintf(xmf, "  <Grid Name=\"%s\" GridType=\"Collection\">\n",
            gridName);

    char coordName[4];
    if (nDims == 2)
        strcpy(coordName, "XY");
    else
        strcpy(coordName, "XYZ");

    char *gridFile= (char *) malloc(strlen(gridFileName)+3+1);
    char *varFile= (char *) malloc(strlen(xdmfFile->fileName)+3+1);
    sprintf(gridFile, "%s.h5", gridFileName);
    sprintf(varFile, "%s.h5", xdmfFile->fileName);
    XDMFWriteCurvBlock(xmf, gridFile, varFile, "block", coordName,
                        xdmfFile->iProc, gridDataType,
                        nVars, varNames, varTypes, varCentering,
                        varDataTypes, nDims,
                        newDims, baseIndex, ghostOffsets);
    free(gridFile);
    free(varFile);

    fprintf(xmf, "  </Grid>\n");

    fclose(xmf);
#endif
}

void
XdmfPutUcdMultiVar(XDMFFile *xdmfFileIn, const char *gridFileName,
    const char *gridName, int coordDataType, int nCoords, int coordDims,
    int cellType, int nCells, int connectivityLength, int nVars,
    char **varNames, int *varTypes, int *varCentering, int *varDataTypes,
    int iBlock, int nBlocks)
{
    XDMFFileParallel *xdmfFile = (XDMFFileParallel *) xdmfFileIn;

#ifdef PARALLEL
    //
    // Every Nth processor collects the data from other processors and
    // writes the information.
    //
    if (xdmfFile->iProc % NX == 0)
    {
        //
        // Collect the dimension, base index, and ghost zone offset
        // information from processors iProc through iProc + nCollect - 1.
        //
        int nCollect = (xdmfFile->nProcs - xdmfFile->iProc) < NX ?
                       (xdmfFile->nProcs - xdmfFile->iProc) : NX;

        int *info = (int *) malloc(nCollect * 4 * sizeof(int));
     
        info[0] = nCoords;
        info[1] = nCells;
        info[2] = cellType;
        info[3] = connectivityLength;
       
        if (nCollect > 1)
        {
            MPI_Request *req =
                (MPI_Request *) malloc((nCollect-1) * sizeof(MPI_Request));
            MPI_Status *statuses =
                (MPI_Status *) malloc((nCollect-1) * sizeof(MPI_Status));

            int i;
            for (i = 1; i < nCollect; i++)
            {
                MPI_Irecv(&(info[i*4]), 4, MPI_INT,
                    xdmfFile->iProc+i, 1, MPI_COMM_WORLD, &(req[i-1]));
            }
            MPI_Waitall(nCollect - 1, req, statuses);

            free(req);
            free(statuses);
        }

        //
        // Wait for my turn to write the data.
        //
        MPI_Status status;

        int buf[1];
        buf[0] = 1;

        int iGroup = xdmfFile->iProc / NX;
        int nGroup = (xdmfFile->nProcs + NX - 1) / NX;
        int battonProc = ((iGroup - 1 + nGroup) % nGroup) * NX;
        MPI_Recv(buf, 1, MPI_INT, battonProc, 2, MPI_COMM_WORLD, &status);

        //
        // Write the data.
        //
        char *fName = (char *) malloc(strlen(xdmfFile->fileName)+4+1);
        sprintf(fName, "%s.xmf", xdmfFile->fileName);

        FILE *xmf = fopen(fName, "a");

        free(fName);

        if (xdmfFile->iProc == 0)
            fprintf(xmf, "  <Grid Name=\"%s\" GridType=\"Collection\">\n",
                    gridName);

        char coordName[4];
        if (coordDims == 2)
            strcpy(coordName, "XY");
        else
            strcpy(coordName, "XYZ");
        int iBlock;
        for (iBlock = 0; iBlock < nCollect; iBlock++)
        {
            char *gridFile = (char *) malloc(strlen(gridFileName)+5+3+1);
            char *varFile = (char *) malloc(strlen(xdmfFile->fileName)+5+3+1);
            int iFile = (xdmfFile->iProc + iBlock) /
                ((xdmfFile->nProcs + xdmfFile->nFiles - 1) / xdmfFile->nFiles);
            if (xdmfFile->nProcs == 1)
            {
                sprintf(gridFile, "%s.h5", gridFileName);
                sprintf(varFile, "%s.h5", xdmfFile->fileName);
            }
            else
            {
                sprintf(gridFile, "%s_%04d.h5", gridFileName, iFile);
                sprintf(varFile, "%s_%04d.h5", xdmfFile->fileName, iFile);
            }
            XDMFPutUcdGrid(xmf, gridFile, varFile, "block", 
                           xdmfFile->iProc+iBlock, coordName,
                           coordDataType, info[iBlock*4+0], coordDims,
                           "connectivity", info[iBlock*4+2], info[iBlock*4+1],
                           info[iBlock*4+3], nVars, varNames,
                           varTypes, varCentering, varDataTypes);
            free(gridFile);
            free(varFile);
        }

        if (xdmfFile->iProc + nCollect == xdmfFile->nProcs)
            fprintf(xmf, "  </Grid>\n");

        fclose(xmf);

        //
        // Let the next processor write the data.
        //
        battonProc = ((iGroup + 1 + nGroup) % nGroup) * NX;
        MPI_Send(buf, 1, MPI_INT, battonProc, 2, MPI_COMM_WORLD);

        free(info);
    }
    else
    {
        int *info = (int *) malloc(4 * sizeof(int));
     
        info[0] = nCoords;
        info[1] = nCells;
        info[2] = cellType;
        info[3] = connectivityLength;
       
        int destProc = xdmfFile->iProc - (xdmfFile->iProc % NX);
        MPI_Send(info, 4, MPI_INT, destProc, 1, MPI_COMM_WORLD);

        free(info);
    }
#else
    char *fName = (char *) malloc(strlen(xdmfFile->fileName)+4+1);
    sprintf(fName, "%s.xmf", xdmfFile->fileName);

    FILE *xmf = fopen(fName, "a");

    free(fName);

    fprintf(xmf, "  <Grid Name=\"%s\" GridType=\"Collection\">\n",
            gridName);

    char coordName[4];
    if (coordDims == 2)
        strcpy(coordName, "XY");
    else
        strcpy(coordName, "XYZ");

    char *gridFile= (char *) malloc(strlen(gridFileName)+3+1);
    char *varFile= (char *) malloc(strlen(xdmfFile->fileName)+3+1);
    sprintf(gridFile, "%s.h5", gridFileName);
    sprintf(varFile, "%s.h5", xdmfFile->fileName);
    XDMFPutUcdGrid(xmf, gridFile, varFile, "block", 
                   xdmfFile->iProc+iBlock, coordName,
                   coordDataType, nCoords, coordDims,
                   "connectivity", cellType, nCells, connectivityLength,
                   nVars, varNames, varTypes, varCentering,
                   varDataTypes);
    free(gridFile);
    free(varFile);

    fprintf(xmf, "  </Grid>\n");

    fclose(xmf);
#endif
}

void
XdmfParallelClose(XDMFFile *xdmfFileIn)
{
    XDMFFileParallel *xdmfFile = (XDMFFileParallel *) xdmfFileIn;

    //
    // Only processor 0 closes the file.
    //
    if (xdmfFile->iProc == 0)
    {
#ifdef PARALLEL
        MPI_Status status;

        //
        // Wait for my turn to write the data.
        //
        int buf[1];
        buf[0] = 1;

        int nGroup = (xdmfFile->nProcs + NX - 1) / NX;
        int battonProc = (nGroup - 1) * NX;
        MPI_Recv(buf, 1, MPI_INT, battonProc, 2, MPI_COMM_WORLD, &status);
#endif

        //
        // Write the trailer information.
        //
        char *fName = (char *) malloc(strlen(xdmfFile->fileName)+4+1);
        sprintf(fName, "%s.xmf", xdmfFile->fileName);

        FILE *xmf = fopen(fName, "a");

        free(fName);

        fprintf(xmf, " </Domain>\n");

        fprintf(xmf, "</Xdmf>\n");

        fclose(xmf);
    }

    free(xdmfFile->fileName);
    xdmfFile->fileName = NULL;
    free(xdmfFile);
}

int
timeval_subtract (struct timeval *result, struct timeval *x,
    struct timeval *y)
{
    //
    // Perform the carry for the later subtraction by updating y.
    //
    if (x->tv_usec < y->tv_usec) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    //
    // Compute the time remaining to wait.
    //  tv_usec is certainly positive.
    //
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    //
    // Return 1 if result is negative.
    //
    return x->tv_sec < y->tv_sec;
}

void
XDMFWriteCurvBlock(FILE *xmf, const char *gridFileName,
    const char *varFileName, const char *blockName,
    const char *coordName, int iBlock, int gridDataType, int nVars,
    char **varNames, int *varTypes, int *varCentering, int *varDataTypes,
    int nDims, int *dims, int *baseIndex, int *ghostOffsets)
{
    //
    // Write out the grid meta data.
    //
    if (iBlock == XDMF_NULL_GRID)
        fprintf(xmf, "   <Grid Name=\"%s\" GridType=\"Uniform\">\n",
            blockName);
    else
        fprintf(xmf, "   <Grid Name=\"%s%d\" GridType=\"Uniform\">\n",
            blockName, iBlock);
    fprintf(xmf, "     <Topology TopologyType=\"%dDSMesh\"", nDims);
    if (nDims == 2)
    {
        fprintf(xmf, " NumberOfElements=\"%d %d\"/>\n", dims[1]+1, dims[0]+1);
        if (baseIndex != NULL)
        {
            fprintf(xmf, "     <Information Name=\"BaseIndex\" Value=\"");
            fprintf(xmf, "%d %d\"/>\n", baseIndex[1], baseIndex[0]);
        }
        if (ghostOffsets != NULL)
        {
            fprintf(xmf, "     <Information Name=\"GhostOffsets\" Value=\"");
            fprintf(xmf, "%d %d %d %d\"/>\n", ghostOffsets[2],
                ghostOffsets[3], ghostOffsets[0], ghostOffsets[1]);
        }
        fprintf(xmf, "     <Geometry GeometryType=\"XY\">\n");
        fprintf(xmf, "       <DataItem Dimensions=\"%d 2\" %s Format=\"HDF\">\n",
            (dims[1]+1)*(dims[0]+1), dataTypeToString[gridDataType]);
        if (iBlock == XDMF_NULL_GRID)
            fprintf(xmf, "        %s:/%s\n", gridFileName, coordName);
        else
            fprintf(xmf, "        %s:/%s%d\n", gridFileName, coordName, iBlock);
    }
    else
    {
        fprintf(xmf, " NumberOfElements=\"%d %d %d\"/>\n",
                dims[2]+1, dims[1]+1, dims[0]+1);
        if (baseIndex != NULL)
        {
            fprintf(xmf, "     <Information Name=\"BaseIndex\" Value=\"");
            fprintf(xmf, "%d %d %d\"/>\n",
                baseIndex[2], baseIndex[1], baseIndex[0]);
        }
        if (ghostOffsets != NULL)
        {
            fprintf(xmf, "     <Information Name=\"GhostOffsets\" Value=\"");
            fprintf(xmf, "%d %d %d %d %d %d\"/>\n",
                ghostOffsets[4], ghostOffsets[5], ghostOffsets[2],
                ghostOffsets[3], ghostOffsets[0], ghostOffsets[1]);
        }
        fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
        fprintf(xmf, "       <DataItem Dimensions=\"%d 3\" %s Format=\"HDF\">\n",
            (dims[2]+1)*(dims[1]+1)*(dims[0]+1),
            dataTypeToString[gridDataType]);
        if (iBlock == XDMF_NULL_GRID)
            fprintf(xmf, "        %s:/%s\n", gridFileName, coordName);
        else
            fprintf(xmf, "        %s:/%s%d\n", gridFileName, coordName, iBlock);
    }
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");

    //
    // Write out the variable meta data.
    //
    int iVar;
    for (iVar = 0; iVar < nVars; iVar++)
    { 
        fprintf(xmf, "     <Attribute Name=\"%s\" %s %s>\n",
            varNames[iVar], varTypeToString[varTypes[iVar]],
            centeringToString[varCentering[iVar]]);
        fprintf(xmf, "       <DataItem Dimensions=\"");
        if (nDims == 2)
            if (varCentering[iVar] == XDMF_CELL_CENTER)
                fprintf(xmf, "%d %d", dims[1], dims[0]);
            else
                fprintf(xmf, "%d %d", dims[1]+1, dims[0]+1);
        else
            if (varCentering[iVar] == XDMF_CELL_CENTER)
                fprintf(xmf, "%d %d %d", dims[2], dims[1], dims[0]);
            else
                fprintf(xmf, "%d %d %d", dims[2]+1, dims[1]+1, dims[0]+1);
        if (varTypes[iVar] == XDMF_VECTOR)
            fprintf(xmf, " %d", nDims);
        fprintf(xmf, "\" %s Format=\"HDF\">\n",
            dataTypeToString[varDataTypes[iVar]]);
        if (iBlock == XDMF_NULL_GRID)
            fprintf(xmf, "        %s:/%s\n", varFileName, varNames[iVar]);
        else
            fprintf(xmf, "        %s:/%s%d\n",
                varFileName, varNames[iVar], iBlock);
        fprintf(xmf, "       </DataItem>\n");
        fprintf(xmf, "     </Attribute>\n");
    }

    fprintf(xmf, "   </Grid>\n");
}

void
XDMFPutUcdGrid(FILE *xmf, const char *gridFileName,
    const char *varFileName, const char *gridName, int iGrid,
    const char *coordName, int coordDataType, int nCoords, int coordDims,
    const char *connectivityName, int cellType, int nCells,
    int connectivityLength, int nVars, char **varNames, int *varTypes,
    int *varCentering, int *varDataTypes)
{
    //
    // Write out the grid meta data.
    //
    if (iGrid == XDMF_NULL_GRID)
        fprintf(xmf, "   <Grid Name=\"%s\" GridType=\"Uniform\">\n",
            gridName);
    else
        fprintf(xmf, "   <Grid Name=\"%s%d\" GridType=\"Uniform\">\n",
            gridName, iGrid);
    fprintf(xmf, "     <Topology %s NumberOfElements=\"%d\">\n",
        cellTypeToString[cellType], nCells);
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" %s Format=\"HDF\">\n",
        connectivityLength, dataTypeToString[XDMF_INT]);
    if (iGrid == XDMF_NULL_GRID)
        fprintf(xmf, "        %s:/%s\n", gridFileName, connectivityName);
    else
        fprintf(xmf, "        %s:/%s%d\n", gridFileName, connectivityName, iGrid);
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Topology>\n");
    if (coordDims == 2)
    {
        fprintf(xmf, "     <Geometry GeometryType=\"XY\">\n");
        fprintf(xmf, "       <DataItem Dimensions=\"%d 2\" %s Format=\"HDF\">\n",
            nCoords, dataTypeToString[coordDataType]);
        if (iGrid == XDMF_NULL_GRID)
            fprintf(xmf, "        %s:/%s\n", gridFileName, coordName);
        else
            fprintf(xmf, "        %s:/%s%d\n", gridFileName, coordName, iGrid);
    }
    else
    {
        fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
        fprintf(xmf, "       <DataItem Dimensions=\"%d 3\" %s Format=\"HDF\">\n",
            nCoords, dataTypeToString[coordDataType]);
        if (iGrid == XDMF_NULL_GRID)
            fprintf(xmf, "        %s:/%s\n", gridFileName, coordName);
        else
            fprintf(xmf, "        %s:/%s%d\n", gridFileName, coordName, iGrid);
    }
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");

    //
    // Write out the variable meta data.
    //
    int iVar;
    for (iVar = 0; iVar < nVars; iVar++)
    { 
        fprintf(xmf, "     <Attribute Name=\"%s\" %s %s>\n",
            varNames[iVar], varTypeToString[varTypes[iVar]],
            centeringToString[varCentering[iVar]]);
        fprintf(xmf, "       <DataItem Dimensions=\"");
        if (varCentering[iVar] == XDMF_CELL_CENTER)
            fprintf(xmf, "%d", nCells);
        else
            fprintf(xmf, "%d", nCoords);
        if (varTypes[iVar] == XDMF_VECTOR)
            fprintf(xmf, " %d", coordDims);
        fprintf(xmf, "\" %s Format=\"HDF\">\n",
            dataTypeToString[varDataTypes[iVar]]);
        if (iGrid == XDMF_NULL_GRID)
            fprintf(xmf, "        %s:/%s\n", varFileName, varNames[iVar]);
        else
            fprintf(xmf, "        %s:/%s%d\n",
                varFileName, varNames[iVar], iGrid);
        fprintf(xmf, "       </DataItem>\n");
        fprintf(xmf, "     </Attribute>\n");
    }

    fprintf(xmf, "   </Grid>\n");
}

HDFFile *
HdfParallelCreate(const char *fileName, int nFiles)
{
    gettimeofday(&hdf5_create_start, NULL);

    HDFFileParallel *hdfFile;

    hdfFile = (HDFFileParallel *) malloc(sizeof(HDFFileParallel));

#ifdef PARALLEL
    MPI_Comm_size(MPI_COMM_WORLD, &(hdfFile->nProcs));
    MPI_Comm_rank(MPI_COMM_WORLD, &(hdfFile->iProc));
#else
    hdfFile->nProcs = 1;
    hdfFile->iProc = 0;
#endif

    hdfFile->type = 1;
    hdfFile->fileName = (char  *) malloc(strlen(fileName)+1);
    strcpy (hdfFile->fileName, fileName);
    hdfFile->nFiles = nFiles;

    dataTypeToHDFType[0] = H5T_NATIVE_FLOAT;
    dataTypeToHDFType[1] = H5T_NATIVE_DOUBLE;
    dataTypeToHDFType[2] = H5T_NATIVE_INT;
    dataTypeToHDFType[3] = H5T_NATIVE_CHAR; 

    //
    // Only some processors create the file.
    //
    int nProcsPerFile = (hdfFile->nProcs + nFiles - 1) / nFiles;
    int iFile = hdfFile->iProc / nProcsPerFile;

    if (hdfFile->iProc % nProcsPerFile == 0)
    {
        char *fName = (char *) malloc(strlen(fileName)+5+3+1);
        if (hdfFile->nFiles == 1)
            sprintf(fName, "%s.h5", fileName);
        else
            sprintf(fName, "%s_%04d.h5", fileName, iFile);

        hid_t     file_id;
        file_id = H5Fcreate(fName, H5F_ACC_TRUNC, H5P_DEFAULT,
                            H5P_DEFAULT);

        herr_t    status;
        status = H5Fclose(file_id); (void) status;

        free(fName);
    }

    gettimeofday(&hdf5_create_end, NULL);

#ifdef PARALLEL
    //
    // Let the first processor in each group write data.
    //
    int buf[1];
    buf[0] = 1;

    if ((hdfFile->iProc + 1) % nProcsPerFile == 0 ||
        hdfFile->iProc == hdfFile->nProcs - 1)
    {
        int battonProc = iFile * nProcsPerFile;
        MPI_Send(buf, 1, MPI_INT, battonProc, 3, MPI_COMM_WORLD);
    }

#endif

    return (HDFFile *) hdfFile;
}

void
HdfPutCurvMultiMesh(HDFFile *hdfFileIn, int coordDataType, float *coords,
    int nDims, int *dims, int *iBlock, int *nBlocks)
{
    gettimeofday(&hdf5_put_multi_start, NULL);

    HDFFileParallel *hdfFile = (HDFFileParallel *) hdfFileIn;

    //int iProc = hdfFile->iProc;
    //int nProcs = hdfFile->nProcs;
    //int nFiles = hdfFile->nFiles;
    //int nProcPerFile = (nProcs + nFiles - 1) / nFiles;

    int newDims[3];
    float *newCoords = NULL;

    //
    // Create the ghost zone data.
    //
    Create3DNodalGhostData(coords, dims, 3, iBlock, nBlocks,
        &newCoords, newDims);

    gettimeofday(&hdf5_put_multi_ghost_created, NULL);

#ifdef PARALLEL
    int iProc = hdfFile->iProc;
    int nProcs = hdfFile->nProcs;
    int nFiles = hdfFile->nFiles;
    int nProcPerFile = (nProcs + nFiles - 1) / nFiles;
    //
    // Wait for my turn to write the data.
    //
    MPI_Status status;

    int buf[1];
    buf[0] = 1;

    int battonProc;
    if (iProc % nProcPerFile != 0)
    {
        battonProc = iProc - 1;
    }
    else
    {
        int iFile = iProc / nProcPerFile;
        battonProc = (iFile + 1) * nProcPerFile - 1;
        if (battonProc > nProcs - 1)
            battonProc = nProcs - 1;
    }
    MPI_Recv(buf, 1, MPI_INT, battonProc, 3, MPI_COMM_WORLD, &status);
#endif

    gettimeofday(&hdf5_put_multi_write_start, NULL);

    //
    // Write the data.
    //
    HdfWriteCurvMeshBlock(hdfFile, coordDataType, newCoords,
        nDims, newDims);

    gettimeofday(&hdf5_put_multi_write_end, NULL);

#ifdef PARALLEL

    //
    // Let the next processor write the data.
    //
    if ((iProc + 1) % nProcPerFile != 0 && (iProc + 1) < nProcs)
    {
        battonProc = iProc + 1;
    }
    else
    {
        int iFile = iProc / nProcPerFile;
        battonProc = iFile * nProcPerFile;
    }
    MPI_Send(buf, 1, MPI_INT, battonProc, 3, MPI_COMM_WORLD);
#endif

    gettimeofday(&hdf5_put_multi_batton_passed, NULL);

    free(newCoords);
}

void
HdfPutCurvMultiVar(HDFFile *hdfFileIn, int nVars, char **varNames,
    int *varTypes, int *varCentering, int *varDataTypes, void *vars,
    int nDims, int *dims, int *iBlock, int *nBlocks)
{
    gettimeofday(&hdf5_put_multi_start, NULL);

    HDFFileParallel *hdfFile = (HDFFileParallel *) hdfFileIn;

    //int iProc = hdfFile->iProc;
    //int nProcs = hdfFile->nProcs;
    //int nFiles = hdfFile->nFiles;
    //int nProcPerFile = (nProcs + nFiles - 1) / nFiles;

    //
    // Create the ghost zone data.
    //
    int i;
    int newDims[3];

    void **newVars = (void **) malloc(nVars*sizeof(void*));
    for (i = 0; i < nVars; i++)
    {
        int nComps = (varTypes[i] == XDMF_SCALAR) ? 1 : nDims;
        if (varCentering[i] == XDMF_CELL_CENTER)
        {
            Create3DZonalGhostData(((float **)vars)[i], dims, nComps, iBlock,
                nBlocks, (float **)&(newVars[i]), newDims);
        }
        else
        {
            Create3DNodalGhostData(((float **)vars)[i], dims, nComps, iBlock,
                nBlocks, (float **)&(newVars[i]), newDims);
        }
    }

    gettimeofday(&hdf5_put_multi_ghost_created, NULL);

#ifdef PARALLEL
    int iProc = hdfFile->iProc;
    int nProcs = hdfFile->nProcs;
    int nFiles = hdfFile->nFiles;
    int nProcPerFile = (nProcs + nFiles - 1) / nFiles;

    //
    // Wait for my turn to write the data.
    //
    MPI_Status status;

    int buf[1];
    buf[0] = 1;

    int battonProc;
    if (iProc % nProcPerFile != 0)
    {
        battonProc = iProc - 1;
    }
    else
    {
        int iFile = iProc / nProcPerFile;
        battonProc = (iFile + 1) * nProcPerFile - 1;
        if (battonProc > nProcs - 1)
            battonProc = nProcs - 1;
    }
    MPI_Recv(buf, 1, MPI_INT, battonProc, 3, MPI_COMM_WORLD, &status);
#endif

    gettimeofday(&hdf5_put_multi_write_start, NULL);

    //
    // Write the data.
    //
    HdfWriteCurvVarBlock(hdfFile, nVars, varNames, varTypes, varCentering,
        varDataTypes, newVars, nDims, newDims);

    gettimeofday(&hdf5_put_multi_write_end, NULL);

#ifdef PARALLEL
    //
    // Let the next processor write the data.
    //
    if ((iProc + 1) % nProcPerFile != 0 && (iProc + 1) < nProcs)
    {
        battonProc = iProc + 1;
    }
    else
    {
        int iFile = iProc / nProcPerFile;
        battonProc = iFile * nProcPerFile;
    }
    MPI_Send(buf, 1, MPI_INT, battonProc, 3, MPI_COMM_WORLD);
#endif

    gettimeofday(&hdf5_put_multi_batton_passed, NULL);

    for (i = 0; i < nVars; i++)
        free(newVars[i]);
    free(newVars);
}

void
HdfPutUcdMultiMesh(HDFFile *hdfFileIn, int coordDataType, float *coords,
    int nCoords, int *connectivity, int connectivityLength,
    int iBlock, int nBlocks)
{
    gettimeofday(&hdf5_put_multi_start, NULL);

    HDFFileParallel *hdfFile = (HDFFileParallel *) hdfFileIn;

    //int iProc = hdfFile->iProc;
    //int nProcs = hdfFile->nProcs;
    //int nFiles = hdfFile->nFiles;
    //int nProcPerFile = (nProcs + nFiles - 1) / nFiles;

#ifdef PARALLEL
    int iProc = hdfFile->iProc;
    int nProcs = hdfFile->nProcs;
    int nFiles = hdfFile->nFiles;
    int nProcPerFile = (nProcs + nFiles - 1) / nFiles;

    //
    // Wait for my turn to write the data.
    //
    MPI_Status status;

    int buf[1];
    buf[0] = 1;

    int battonProc;
    if (iProc % nProcPerFile != 0)
    {
        battonProc = iProc - 1;
    }
    else
    {
        int iFile = iProc / nProcPerFile;
        battonProc = (iFile + 1) * nProcPerFile - 1;
        if (battonProc > nProcs - 1)
            battonProc = nProcs - 1;
    }
    MPI_Recv(buf, 1, MPI_INT, battonProc, 3, MPI_COMM_WORLD, &status);
#endif

    gettimeofday(&hdf5_put_multi_write_start, NULL);

    //
    // Write the data.
    //
    HdfWriteUcdMeshBlock(hdfFile, coordDataType, coords,
        nCoords, connectivity, connectivityLength);

    gettimeofday(&hdf5_put_multi_write_end, NULL);

#ifdef PARALLEL
    //
    // Let the next processor write the data.
    //
    if ((iProc + 1) % nProcPerFile != 0 && (iProc + 1) < nProcs)
    {
        battonProc = iProc + 1;
    }
    else
    {
        int iFile = iProc / nProcPerFile;
        battonProc = iFile * nProcPerFile;
    }
    MPI_Send(buf, 1, MPI_INT, battonProc, 3, MPI_COMM_WORLD);
#endif

    gettimeofday(&hdf5_put_multi_batton_passed, NULL);
}

void
HdfPutUcdMultiVar(HDFFile *hdfFileIn, int nVars, char **varNames,
    int *varTypes, int *varCentering, int *varDataTypes, void *vars,
    int nDims, int nCoords, int nCells, int iBlock, int nBlocks)
{
    gettimeofday(&hdf5_put_multi_start, NULL);

    HDFFileParallel *hdfFile = (HDFFileParallel *) hdfFileIn;

    //int iProc = hdfFile->iProc;
    //int nProcs = hdfFile->nProcs;
    //int nFiles = hdfFile->nFiles;
    //int nProcPerFile = (nProcs + nFiles - 1) / nFiles;

#ifdef PARALLEL
    int iProc = hdfFile->iProc;
    int nProcs = hdfFile->nProcs;
    int nFiles = hdfFile->nFiles;
    int nProcPerFile = (nProcs + nFiles - 1) / nFiles;

    //
    // Wait for my turn to write the data.
    //
    MPI_Status status;

    int buf[1];
    buf[0] = 1;

    int battonProc;
    if (iProc % nProcPerFile != 0)
    {
        battonProc = iProc - 1;
    }
    else
    {
        int iFile = iProc / nProcPerFile;
        battonProc = (iFile + 1) * nProcPerFile - 1;
        if (battonProc > nProcs - 1)
            battonProc = nProcs - 1;
    }
    MPI_Recv(buf, 1, MPI_INT, battonProc, 3, MPI_COMM_WORLD, &status);
#endif

    gettimeofday(&hdf5_put_multi_write_start, NULL);

    //
    // Write the data.
    //
    HdfWriteUcdVarBlock(hdfFile, nVars, varNames, varTypes, varCentering,
        varDataTypes, vars, nDims, nCoords, nCells);

    gettimeofday(&hdf5_put_multi_write_end, NULL);

#ifdef PARALLEL
    //
    // Let the next processor write the data.
    //
    if ((iProc + 1) % nProcPerFile != 0 && (iProc + 1) < nProcs)
    {
        battonProc = iProc + 1;
    }
    else
    {
        int iFile = iProc / nProcPerFile;
        battonProc = iFile * nProcPerFile;
    }
    MPI_Send(buf, 1, MPI_INT, battonProc, 3, MPI_COMM_WORLD);
#endif

    gettimeofday(&hdf5_put_multi_batton_passed, NULL);
}

void
HdfParallelClose(HDFFile *hdfFileIn)
{
    gettimeofday(&hdf5_close_start, NULL);

    HDFFileParallel *hdfFile = (HDFFileParallel *) hdfFileIn;

    int iProc = hdfFile->iProc;
    int nProcs = hdfFile->nProcs;
    //int nFiles = hdfFile->nFiles;
    //int nProcPerFile = (nProcs + nFiles - 1) / nFiles;

#ifdef PARALLEL
    //int iProc = hdfFile->iProc;
    //int nProcs = hdfFile->nProcs;
    int nFiles = hdfFile->nFiles;
    int nProcPerFile = (nProcs + nFiles - 1) / nFiles;
    //
    // Wait for batton from the last processor in the group.
    //
    if (iProc % nProcPerFile == 0)
    {
        int iFile = iProc / nProcPerFile;
        int battonProc = (iFile + 1) * nProcPerFile - 1;
        if (battonProc > nProcs - 1)
            battonProc = nProcs - 1;

        MPI_Status status;

        int buf[1];
        buf[0] = 1;

        MPI_Recv(buf, 1, MPI_INT, battonProc, 3, MPI_COMM_WORLD, &status);
    }
#endif

    //
    // Send the timing information back to processor 0.
    //
    struct timeval times[8];

    times[0] = hdf5_create_start;
    times[1] = hdf5_create_end;
    times[2] = hdf5_put_multi_start;
    times[3] = hdf5_put_multi_ghost_created;
    times[4] = hdf5_put_multi_write_start;
    times[5] = hdf5_put_multi_write_end;
    times[6] = hdf5_put_multi_batton_passed;
    times[7] = hdf5_close_start;

    int nBytes = 8 * sizeof(struct timeval);

    struct timeval *allTimes;
    if (iProc == 0)
        allTimes = (struct timeval *) malloc(nProcs * nBytes);
    else
        allTimes = NULL;

#ifdef PARALLEL
    MPI_Gather(times, nBytes, MPI_CHAR, allTimes, nBytes, MPI_CHAR, 0,
               MPI_COMM_WORLD);
#else
    int i;
    for (i = 0; i < nBytes; i++)
        allTimes[i] = times[i];
#endif

    if (iProc == 0)
    {
#ifdef DEBUG
        int i;
        for (i = 0; i < nProcs; i++)
        {
            struct timeval diff;
            double diff2;

            timeval_subtract(&diff, &allTimes[i*8+1], &allTimes[i*8+0]);
            diff2 = (double) diff.tv_sec + (double) diff.tv_usec / 1000000.;
            fprintf(stderr, "%02d: Time to create file: %g\n", i, diff2);

            timeval_subtract(&diff, &allTimes[i*8+3], &allTimes[i*8+2]);
            diff2 = (double) diff.tv_sec + (double) diff.tv_usec / 1000000.;
            fprintf(stderr, "%02d: Time to create ghost data: %g\n", i, diff2);

            timeval_subtract(&diff, &allTimes[i*8+5], &allTimes[i*8+4]);
            diff2 = (double) diff.tv_sec + (double) diff.tv_usec / 1000000.;
            fprintf(stderr, "%02d: Time to write data: %g\n", i, diff2);

            timeval_subtract(&diff, &allTimes[i*8+7], &allTimes[i*8+5]);
            diff2 = (double) diff.tv_sec + (double) diff.tv_usec / 1000000.;
            fprintf(stderr, "%02d: Time to close file: %g\n", i, diff2);
        }

        fprintf(stderr, "\n*********************************************\n\n");

        for (i = 0; i < nProcs; i++)
        {
            fprintf(stderr, "%02d: HdfCreate start: %d, %d\n",
                    i, allTimes[i*8+0].tv_sec, allTimes[i*8+0].tv_usec);
            fprintf(stderr, "%02d: HdfCreate end: %d, %d\n",
                    i, allTimes[i*8+1].tv_sec, allTimes[i*8+1].tv_usec);
            fprintf(stderr, "%02d: HdfPutCurvMultiVar start: %d, %d\n",
                    i, allTimes[i*8+2].tv_sec, allTimes[i*8+2].tv_usec);
            fprintf(stderr, "%02d: HdfPutCurvMultiVar ghost created: %d, %d\n",
                    i, allTimes[i*8+3].tv_sec, allTimes[i*8+3].tv_usec);
            fprintf(stderr, "%02d: HdfPutCurvMultiVar write start: %d, %d\n",
                    i, allTimes[i*8+4].tv_sec, allTimes[i*8+4].tv_usec);
            fprintf(stderr, "%02d: HdfPutCurvMultiVar write end: %d, %d\n",
                    i, allTimes[i*8+5].tv_sec, allTimes[i*8+5].tv_usec);
            fprintf(stderr, "%02d: HdfPutCurvMultiVar batton passed: %d, %d\n",
                    i, allTimes[i*8+6].tv_sec, allTimes[i*8+6].tv_usec);
            fprintf(stderr, "%02d: HdfClose start: %d, %d\n",
                    i, allTimes[i*8+7].tv_sec, allTimes[i*8+7].tv_usec);
        }
#endif

        free(allTimes);
    }

    free(hdfFile->fileName);
    hdfFile->fileName = NULL;
    free(hdfFile);
}

void
HdfWriteCurvMeshBlock(HDFFileParallel *hdfFile, int coordDataType,
    float *coords, int nDims, int *dims)
{
    int iFile = hdfFile->iProc /
        ((hdfFile->nProcs + hdfFile->nFiles - 1) / hdfFile->nFiles);

    hid_t     file_id, dataspace_id, dataset_id;
    hsize_t   /*nvdims,*/ vdims[4];
    herr_t    status;

    char str[1024];
    if (hdfFile->nFiles == 1)
        sprintf(str, "%s.h5", hdfFile->fileName);
    else
        sprintf(str, "%s_%04d.h5", hdfFile->fileName, iFile);

    file_id = H5Fopen(str, H5F_ACC_RDWR, H5P_DEFAULT);

    //
    // Write the grid.
    //
    vdims[0] = (dims[0] + 1) * (dims[1] + 1) * (dims[2] + 1);
    vdims[1] = 3;
    dataspace_id = H5Screate_simple(2, vdims, NULL);

    sprintf(str, "/XYZ%d", hdfFile->iProc);
    dataset_id = H5Dcreate(file_id, str, dataTypeToHDFType[coordDataType],
                           dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, dataTypeToHDFType[coordDataType], H5S_ALL,
                      H5S_ALL, H5P_DEFAULT, coords);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    status = H5Fclose(file_id);

    (void) status;
}

void
HdfWriteCurvVarBlock(HDFFileParallel *hdfFile, int nVars, char **varNames,
    int *varTypes, int *varCentering, int *varDataTypes, void *vars,
    int nDims, int *dims)
{
    int iFile = hdfFile->iProc /
        ((hdfFile->nProcs + hdfFile->nFiles - 1) / hdfFile->nFiles);

    hid_t     file_id, dataspace_id, dataset_id;
    hsize_t   nvdims, vdims[4];
    herr_t    status;

    //
    // Open the file.
    //
    char str[1024];
    if (hdfFile->nFiles == 1)
        sprintf(str, "%s.h5", hdfFile->fileName);
    else
        sprintf(str, "%s_%04d.h5", hdfFile->fileName, iFile);

    file_id = H5Fopen(str, H5F_ACC_RDWR, H5P_DEFAULT);

    //
    // Write the variables.
    //
    int iVar;
    for (iVar = 0; iVar < nVars; iVar++)
    {
        if (varCentering[iVar] == XDMF_CELL_CENTER)
        {
            vdims[0] = dims[2];
            vdims[1] = dims[1];
            vdims[2] = dims[0];
        }
        else
        {
            vdims[0] = dims[2] + 1;
            vdims[1] = dims[1] + 1;
            vdims[2] = dims[0] + 1;
        }
        nvdims = 3;
        if (varTypes[iVar] == XDMF_VECTOR)
        {
            vdims[3] = nDims;
            nvdims++;
        }
        dataspace_id = H5Screate_simple(nvdims, vdims, NULL);

        sprintf(str, "/%s%d", varNames[iVar], hdfFile->iProc);
        dataset_id = H5Dcreate(file_id, str,
            dataTypeToHDFType[varDataTypes[iVar]], dataspace_id,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        status = H5Dwrite(dataset_id, dataTypeToHDFType[varDataTypes[iVar]],
            H5S_ALL, H5S_ALL, H5P_DEFAULT, ((void **)vars)[iVar]);


        status = H5Dclose(dataset_id);

        status = H5Sclose(dataspace_id);
    }

    status = H5Fclose(file_id);

    (void) status;
}

void
HdfWriteCurvBlock(HDFFileParallel *hdfFile, const char *gridName,
    int gridDataType, float *gridCoords, int nVars, char **varNames,
    int *varTypes, int *varCentering, int *varDataTypes, void *vars,
    int nDims, int *dims)
{
    int iFile = hdfFile->iProc /
        ((hdfFile->nProcs + hdfFile->nFiles - 1) / hdfFile->nFiles);

    hid_t     file_id, dataspace_id, dataset_id;
    hsize_t   nvdims, vdims[4];
    herr_t    status;

    char str[1024];
    if (hdfFile->nFiles == 1)
        sprintf(str, "%s.h5", hdfFile->fileName);
    else
        sprintf(str, "%s_%04d.h5", hdfFile->fileName, iFile);

    file_id = H5Fopen(str, H5F_ACC_RDWR, H5P_DEFAULT);

    //
    // Write the grid.
    //
    vdims[0] = (dims[0] + 1) * (dims[1] + 1) * (dims[2] + 1);
    vdims[1] = 3;
    dataspace_id = H5Screate_simple(2, vdims, NULL);

    sprintf(str, "/XYZ%d", hdfFile->iProc);
    dataset_id = H5Dcreate(file_id, str, dataTypeToHDFType[gridDataType],
                           dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, dataTypeToHDFType[gridDataType], H5S_ALL,
                      H5S_ALL, H5P_DEFAULT, gridCoords);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    //
    // Write the variables.
    //
    int iVar;
    for (iVar = 0; iVar < nVars; iVar++)
    {
        if (varCentering[iVar] == XDMF_CELL_CENTER)
        {
            vdims[0] = dims[2];
            vdims[1] = dims[1];
            vdims[2] = dims[0];
        }
        else
        {
            vdims[0] = dims[2] + 1;
            vdims[1] = dims[1] + 1;
            vdims[2] = dims[0] + 1;
        }
        nvdims = 3;
        if (varTypes[iVar] == XDMF_VECTOR)
        {
            vdims[3] = nDims;
            nvdims++;
        }
        dataspace_id = H5Screate_simple(nvdims, vdims, NULL);

        sprintf(str, "/%s%d", varNames[iVar], hdfFile->iProc);
        dataset_id = H5Dcreate(file_id, str,
            dataTypeToHDFType[varDataTypes[iVar]], dataspace_id,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        status = H5Dwrite(dataset_id, dataTypeToHDFType[varDataTypes[iVar]],
            H5S_ALL, H5S_ALL, H5P_DEFAULT, ((void **)vars)[iVar]);


        status = H5Dclose(dataset_id);

        status = H5Sclose(dataspace_id);
    }

    status = H5Fclose(file_id);

    (void) status;
}

void
HdfWriteUcdMeshBlock(HDFFileParallel *hdfFile, int coordDataType,
    float *coords, int nCoords, int *connectivity, int connectivityLength)
{
    int iFile = hdfFile->iProc /
        ((hdfFile->nProcs + hdfFile->nFiles - 1) / hdfFile->nFiles);

    hid_t     file_id, dataspace_id, dataset_id;
    hsize_t   /*nvdims,*/ vdims[4];
    herr_t    status;

    char str[1024];
    if (hdfFile->nFiles == 1)
        sprintf(str, "%s.h5", hdfFile->fileName);
    else
        sprintf(str, "%s_%04d.h5", hdfFile->fileName, iFile);

    file_id = H5Fopen(str, H5F_ACC_RDWR, H5P_DEFAULT);

    //
    // Write the coordinates.
    //
    vdims[0] = nCoords;
    vdims[1] = 3;
    dataspace_id = H5Screate_simple(2, vdims, NULL);

    sprintf(str, "/XYZ%d", hdfFile->iProc);
    dataset_id = H5Dcreate(file_id, str, dataTypeToHDFType[coordDataType],
                           dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, dataTypeToHDFType[coordDataType], H5S_ALL,
                      H5S_ALL, H5P_DEFAULT, coords);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    //
    // Write the connectivity.
    //
    vdims[0] = connectivityLength;
    dataspace_id = H5Screate_simple(1, vdims, NULL);

    sprintf(str, "/connectivity%d", hdfFile->iProc);
    dataset_id = H5Dcreate(file_id, str, dataTypeToHDFType[XDMF_INT],
                           dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, dataTypeToHDFType[XDMF_INT],
                      H5S_ALL, H5S_ALL, H5P_DEFAULT, connectivity);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    status = H5Fclose(file_id);

    (void) status;
}

void
HdfWriteUcdVarBlock(HDFFileParallel *hdfFile, int nVars, char **varNames,
    int *varTypes, int *varCentering, int *varDataTypes, void *vars,
    int nDims, int nCoords, int nCells)
{
    int iFile = hdfFile->iProc /
        ((hdfFile->nProcs + hdfFile->nFiles - 1) / hdfFile->nFiles);

    hid_t     file_id, dataspace_id, dataset_id;
    hsize_t   nvdims, vdims[4];
    herr_t    status;

    //
    // Open the file.
    //
    char str[1024];
    if (hdfFile->nFiles == 1)
        sprintf(str, "%s.h5", hdfFile->fileName);
    else
        sprintf(str, "%s_%04d.h5", hdfFile->fileName, iFile);

    file_id = H5Fopen(str, H5F_ACC_RDWR, H5P_DEFAULT);

    //
    // Write the variables.
    //
    int iVar;
    for (iVar = 0; iVar < nVars; iVar++)
    {
        if (varCentering[iVar] == XDMF_CELL_CENTER)
        {
            vdims[0] = nCells;
        }
        else
        {
            vdims[0] = nCoords;
        }
        nvdims = 1;
        if (varTypes[iVar] == XDMF_VECTOR)
        {
            vdims[1] = nDims;
            nvdims++;
        }
        dataspace_id = H5Screate_simple(nvdims, vdims, NULL);

        sprintf(str, "/%s%d", varNames[iVar], hdfFile->iProc);
        dataset_id = H5Dcreate(file_id, str,
            dataTypeToHDFType[varDataTypes[iVar]], dataspace_id,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        status = H5Dwrite(dataset_id, dataTypeToHDFType[varDataTypes[iVar]],
            H5S_ALL, H5S_ALL, H5P_DEFAULT, ((void **)vars)[iVar]);

        status = H5Dclose(dataset_id);

        status = H5Sclose(dataspace_id);
    }

    status = H5Fclose(file_id);

    (void) status;
}

void
DetermineGhostInfo(int nDims, int *dims, int *iBlock, int *nBlocks,
    int *newDims, int *baseIndex, int *ghostOffsets)
{
    int i;
    for (i = 0; i < nDims; i++)
    {
        newDims[i] = dims[i];
        baseIndex[i] = iBlock[i] * dims[i];
        ghostOffsets[i*2]   = 0;
        ghostOffsets[i*2+1] = 0;
        if (iBlock[i] > 0)
        {
            newDims[i]++;
            ghostOffsets[i*2] = 1;
        }
        if (iBlock[i] < nBlocks[i] - 1)
        {
            newDims[i]++;
            ghostOffsets[i*2+1] = 1;
        }
    }
}

//
// Routines for doing ghost zone data exchange.
//
#ifdef PARALLEL
int nRecv;
int nSend;
MPI_Request recvReq[26];
MPI_Request sendReq[26];
float *recvBufs[26];
float *sendBufs[26];
#endif

void
StartExchange3D(int exchangeProc, float *var, int nComps,
    int min[3], int max[3], int dims[3], int recvTag, int sendTag)
{
#ifdef PARALLEL
    int nels = nComps*(max[0]-min[0])*(max[1]-min[1])*(max[2]-min[2]);

    float *recvBuf = (float *) malloc(nels*sizeof(float));
    recvBufs[nRecv] = recvBuf;
 
    MPI_Irecv(recvBuf, nels, MPI_FLOAT, exchangeProc,
              recvTag, MPI_COMM_WORLD, &(recvReq[nRecv++]));

    float *sendBuf = (float *) malloc(nels*sizeof(float));
    sendBufs[nSend] = sendBuf;

    int ndx = 0;
    int k;
    for (k = min[2]; k < max[2]; k++)
    {
        int j;
        for (j = min[1]; j < max[1]; j++)
        {
            int i;
            for (i = min[0]; i < max[0]; i++)
            {
                int ndx2 = k * dims[0] * dims[1] + j * dims[0] + i;
                int ii;
                for (ii = 0; ii < nComps; ii++)
                {
                    sendBuf[ndx++] = var[ndx2*nComps+ii];
                }
            }
        }
    }

    MPI_Isend(sendBuf, nels, MPI_FLOAT, exchangeProc,
              sendTag, MPI_COMM_WORLD, &(sendReq[nSend++]));
#endif
}

void
FinishExchange3D(float *var, int nComps, int min[3], int max[3], int dims[3])
{
#ifdef PARALLEL
    float *recvBuf = recvBufs[nRecv++];
    float *sendBuf = sendBufs[nSend++];

    int ndx = 0;
    int k;
    for (k = min[2]; k < max[2]; k++)
    {
        int j;
        for (j = min[1]; j < max[1]; j++)
        {
            int i;
            for (i = min[0]; i < max[0]; i++)
            {
                int ii;
                for (ii = 0; ii < nComps; ii++)
                {
                    int ndx2 = k * dims[0] * dims[1] + j * dims[0] + i;
                    var[ndx2*nComps+ii] = recvBuf[ndx++];
                }
            }
        }
    }

    free(sendBuf);
    free(recvBuf);
#endif
}

void
StartSend3D(int exchangeProc, float *var, int nComps,
    int min[3], int max[3], int dims[3], int sendTag)
{
#ifdef PARALLEL
    int nels = nComps*(max[0]-min[0])*(max[1]-min[1])*(max[2]-min[2]);

    float *sendBuf = (float *) malloc(nels*sizeof(float));
    sendBufs[nSend] = sendBuf;

    int ndx = 0;
    int k;
    for (k = min[2]; k < max[2]; k++)
    {
        int j;
        for (j = min[1]; j < max[1]; j++)
        {
            int i;
            for (i = min[0]; i < max[0]; i++)
            {
                int ndx2 = k * dims[0] * dims[1] + j * dims[0] + i;
                int ii;
                for (ii = 0; ii < nComps; ii++)
                {
                    sendBuf[ndx++] = var[ndx2*nComps+ii];
                }
            }
        }
    }

    MPI_Isend(sendBuf, nels, MPI_FLOAT, exchangeProc,
              sendTag, MPI_COMM_WORLD, &(sendReq[nSend++]));
#endif
}

void
StartReceive3D(int exchangeProc, float *var, int nels, int recvTag)
{
#ifdef PARALLEL
    float *recvBuf = (float *) malloc(nels*sizeof(float));
    recvBufs[nRecv] = recvBuf;
 
    MPI_Irecv(recvBuf, nels, MPI_FLOAT, exchangeProc,
              recvTag, MPI_COMM_WORLD, &(recvReq[nRecv++]));
#endif
}

void
FinishReceive3D(float *var, int nComps, int min[3], int max[3], int dims[3])
{
#ifdef PARALLEL
    float *recvBuf = recvBufs[nRecv++];
    float *sendBuf = sendBufs[nSend++];

    int ndx = 0;
    int k;
    for (k = min[2]; k < max[2]; k++)
    {
        int j;
        for (j = min[1]; j < max[1]; j++)
        {
            int i;
            for (i = min[0]; i < max[0]; i++)
            {
                int ii;
                for (ii = 0; ii < nComps; ii++)
                {
                    int ndx2 = k * dims[0] * dims[1] + j * dims[0] + i;
                    var[ndx2*nComps+ii] = recvBuf[ndx++];
                }
            }
        }
    }

    free(sendBuf);
    free(recvBuf);
#endif
}

void
Create3DNodalGhostData(float *var, int *zonalDims, int nComps, int *iBlock,
    int *nBlocks, float **newVar, int newZonalDims[3])
{
    //
    // We are working with 3D data.
    //
    int nDims = 3;

    //
    // Determine the shape of the new variable with ghost zones.
    //
    int mins[3];

    int i;
    for (i = 0; i < nDims; i++)
    {
        mins[i] = 0;
        newZonalDims[i] = zonalDims[i];
        if (iBlock[i] > 0)
        {
            mins[i] = 1;
            newZonalDims[i]++;
        }
        if (iBlock[i] < nBlocks[i] - 1)
        {
            newZonalDims[i]++;
        }
    }

    int dims[3];
    int newDims[3];
    int nVals2 = 1;
    for (i = 0; i < nDims; i++)
    {
        dims[i] = zonalDims[i] + 1;
        newDims[i] = newZonalDims[i] + 1;
        nVals2 *= newDims[i];
    }

    float *var2 = (float *) malloc(nComps*nVals2*sizeof(float));
#ifdef DEBUG
    for (i = 0; i < nComps*nVals2; i++)
        var2[i] = -1.;
#endif

    //
    // Exchange the data with the neighboring processes.
    //
    int min[3], max[3];

#ifdef PARALLEL
    nRecv = 0;
    nSend = 0;
#endif

    if (iBlock[0] < nBlocks[0] - 1)
    {
        //
        // Exchange data with (+1,0,0).
        //
        min[0] = dims[0]-1-1; max[0] = dims[0]-1-1 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]+0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 102, 101);
    }

    if (iBlock[0] > 0)
    {
        //
        // Exchange data with (-1,0,0).
        //
        min[0] = 0 + 1; max[0] = 0 + 1 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-0)*nBlocks[2] +
                           (iBlock[2]-0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 101, 102);
    }

    if (iBlock[1] < nBlocks[1] - 1)
    {
        //
        // Exchange data with (0,+1,0).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = dims[1]-1-1; max[1] = dims[1]-1-1 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 104, 103);
    }

    if (iBlock[1] > 0)
    {
        //
        // Exchange data with (0,-1,0).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = 0 + 1; max[1] = 0 + 1 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]-0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]-0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 103, 104);
    }

    if (iBlock[2] < nBlocks[2] - 1)
    {
        //
        // Exchange data with (0,0,+1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = 0; max[1] = dims[1];
        min[2] = dims[2]-1-1; max[2] = dims[2]-1-1 + 1;
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 106, 105);
    }

    if (iBlock[2] > 0)
    {
        //
        // Exchange data with (0,0,-1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = 0; max[1] = dims[1];
        min[2] = 0 + 1; max[2] = 0 + 1 + 1;
        int exchangeProc = (iBlock[0]-0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-0)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 105, 106);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1))
    {
        //
        // Exchange data with (+1,+1,0).
        //
        min[0] = dims[0]-1-1; max[0] = dims[0]-1-1 + 1;
        min[1] = dims[1]-1-1; max[1] = dims[1]-1-1 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 108, 107);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0))
    {
        //
        // Exchange data with (-1,-1,0).
        //
        min[0] = 0 + 1; max[0] = 0 + 1 + 1;
        min[1] = 0 + 1; max[1] = 0 + 1 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]+0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 107, 108);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (+1,0,+1).
        //
        min[0] = dims[0]-1-1; max[0] = dims[0]-1-1 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = dims[2]-1-1; max[2] = dims[2]-1-1 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 110, 109);
    }

    if ((iBlock[0] > 0) && (iBlock[2] > 0))
    {
        //
        // Exchange data with (-1,0,-1).
        //
        min[0] = 0 + 1; max[0] = 0 + 1 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = 0 + 1; max[2] = 0 + 1 + 1;
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 109, 110);
    }

    if ((iBlock[1] < nBlocks[1] - 1) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (0,+1,+1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = dims[1]-1-1; max[1] = dims[1]-1-1 + 1;
        min[2] = dims[2]-1-1; max[2] = dims[2]-1-1 + 1;
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 112, 111);
    }

    if ((iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Exchange data with (0,-1,-1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = 0 + 1; max[1] = 0 + 1 + 1;
        min[2] = 0 + 1; max[2] = 0 + 1 + 1;
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 111, 112);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] > 0))
    {
        //
        // Exchange data with (+1,-1,0).
        //
        min[0] = dims[0]-1-1; max[0] = dims[0]-1-1 + 1;
        min[1] = 0 + 1; max[1] = 0 + 1 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]+0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 114, 113);
    }

    if ((iBlock[0] > 0) && (iBlock[1] < nBlocks[1] - 1))
    {
        //
        // Exchange data with (-1,+1,0).
        //
        min[0] = 0 + 1; max[0] = 0 + 1 + 1;
        min[1] = dims[1]-1-1; max[1] = dims[1]-1-1 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 113, 114);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[2] > 0))
    {
        //
        // Exchange data with (+1,0,-1).
        //
        min[0] = dims[0]-1-1; max[0] = dims[0]-1-1 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = 0 + 1; max[2] = 0 + 1 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 116, 115);
    }

    if ((iBlock[0] > 0) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (-1,0,+1).
        //
        min[0] = 0 + 1; max[0] = 0 + 1 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = dims[2]-1-1; max[2] = dims[2]-1-1 + 1;
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 115, 116);
    }

    if ((iBlock[1] < nBlocks[1] - 1) && (iBlock[2] > 0))
    {
        //
        // Exchange data with (0,+1,-1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = dims[1]-1-1; max[1] = dims[1]-1-1 + 1;
        min[2] = 0 + 1; max[2] = 0 + 1 + 1;
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 118, 117);
    }

    if ((iBlock[1] > 0) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (0,-1,+1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = 0 + 1; max[1] = 0 + 1 + 1;
        min[2] = dims[2]-1-1; max[2] = dims[2]-1-1 + 1;
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 117, 118);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (+1,+1,+1).
        //
        min[0] = dims[0]-1-1; max[0] = dims[0]-1-1 + 1;
        min[1] = dims[1]-1-1; max[1] = dims[1]-1-1 + 1;
        min[2] = dims[2]-1-1; max[2] = dims[2]-1-1 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 120, 119);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Exchange data with (-1,-1,-1).
        //
        min[0] = 0 + 1; max[0] = 0 + 1 + 1;
        min[1] = 0 + 1; max[1] = 0 + 1 + 1;
        min[2] = 0 + 1; max[2] = 0 + 1 + 1;
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 119, 120);
    }

    if ((iBlock[0] > 0) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (-1,+1,+1).
        //
        min[0] = 0 + 1; max[0] = 0 + 1 + 1;
        min[1] = dims[1]-1-1; max[1] = dims[1]-1-1 + 1;
        min[2] = dims[2]-1-1; max[2] = dims[2]-1-1 + 1;
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 122, 121);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Exchange data with (+1,-1,-1).
        //
        min[0] = dims[0]-1-1; max[0] = dims[0]-1-1 + 1;
        min[1] = 0 + 1; max[1] = 0 + 1 + 1;
        min[2] = 0 + 1; max[2] = 0 + 1 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 121, 122);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] > 0) &&
        (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (+1,-1,+1).
        //
        min[0] = dims[0]-1-1; max[0] = dims[0]-1-1 + 1;
        min[1] = 0 + 1; max[1] = 0 + 1 + 1;
        min[2] = dims[2]-1-1; max[2] = dims[2]-1-1 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 124, 123);
    }

    if ((iBlock[0] > 0) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] > 0))
    {
        //
        // Exchange data with (-1,+1,-1).
        //
        min[0] = 0 + 1; max[0] = 0 + 1 + 1;
        min[1] = dims[1]-1-1; max[1] = dims[1]-1-1 + 1;
        min[2] = 0 + 1; max[2] = 0 + 1 + 1;
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 123, 124);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] > 0))
    {
        //
        // Exchange data with (+1,+1,-1).
        //
        min[0] = dims[0]-1-1; max[0] = dims[0]-1-1 + 1;
        min[1] = dims[1]-1-1; max[1] = dims[1]-1-1 + 1;
        min[2] = 0 + 1; max[2] = 0 + 1 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 126, 125);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (-1,-1,+1).
        //
        min[0] = 0 + 1; max[0] = 0 + 1 + 1;
        min[1] = 0 + 1; max[1] = 0 + 1 + 1;
        min[2] = dims[2]-1-1; max[2] = dims[2]-1-1 + 1;
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 125, 126);
    }

    //
    // Copy data for (0,0,0).
    //
    int ndx = 0;
    int k;
    for (k = 0; k < dims[2]; k++)
    {
        int j;
        for (j = 0; j < dims[1]; j++)
        {
            int i;
            for (i = 0; i < dims[0]; i++)
            {
                int ii;
                for (ii = 0; ii < nComps; ii++)
                {
                    int ndx2 = (k+mins[2]) * (newDims[0]) * (newDims[1]) +
                               (j+mins[1]) * (newDims[0]) + (i+mins[0]);
                    var2[ndx2*nComps+ii] = var[ndx++];
                }
            }
        }
    }

#ifdef PARALLEL
    //
    // Wait for the communication to complete.
    //
    MPI_Status recvStatus[26];
    MPI_Status sendStatus[26];
    MPI_Waitall(nRecv, recvReq, recvStatus);
    MPI_Waitall(nSend, sendReq, sendStatus);

    //
    // Handle the recieved data.
    //
    nRecv = 0;
    nSend = 0;
#endif

    if (iBlock[0] < nBlocks[0] - 1)
    {
        //
        // Handle the data from (+1,0,0).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if (iBlock[0] > 0)
    {
        //
        // Handle the data from (-1,0,0).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if (iBlock[1] < nBlocks[1] - 1)
    {
        //
        // Handle the data from (0,+1,0).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if (iBlock[1] > 0)
    {
        //
        // Handle the data from (0,-1,0).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = 0; max[1] = 0 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if (iBlock[2] < nBlocks[2] - 1)
    {
        //
        // Handle the data from (0,0,+1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if (iBlock[2] > 0)
    {
        //
        // Handle the data from (0,0,-1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1))
    {
        //
        // Handle the data from (+1,+1,0).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0))
    {
        //
        // Handle the data from (-1,-1,0).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (+1,0,+1).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[2] > 0))
    {
        //
        // Handle the data from (-1,0,-1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[1] < nBlocks[1] - 1) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (0,+1,+1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Handle the data from (0,-1,-1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] > 0))
    {
        //
        // Handle the data from (+1,-1,0).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] < nBlocks[1] - 1))
    {
        //
        // Handle the data from (-1,+1,0).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[2] > 0))
    {
        //
        // Handle the data from (+1,0,-1).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (-1,0,+1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[1] < nBlocks[1] - 1) && (iBlock[2] > 0))
    {
        //
        // Handle the data from (0,+1,-1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[1] > 0) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (0,-1,+1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = 0; max[1] = 0 + 1;
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (+1,+1,+1).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Handle the data from (-1,-1,-1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (-1,+1,+1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Handle the data from (+1,-1,-1).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] > 0) &&
        (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (+1,-1,+1).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] > 0))
    {
        //
        // Handle the data from (-1,+1,-1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] > 0))
    {
        //
        // Handle the data from (+1,+1,-1).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (-1,-1,+1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    *newVar = var2;
}

void
Create3DZonalGhostData(float *var, int *dims, int nComps, int *iBlock,
    int *nBlocks, float **newVar, int newDims[3])
{
    //
    // We are working with 3D data.
    //
    int nDims = 3;

    //
    // Determine the shape of the new variable with ghost zones.
    //
    int mins[3];

    int nVals2 = 1;
    int i;
    for (i = 0; i < nDims; i++)
    {
        newDims[i] = dims[i];
        mins[i] = 0;
        if (iBlock[i] > 0)
        {
            mins[i] = 1;
            newDims[i]++;
        }
        if (iBlock[i] < nBlocks[i] - 1)
        {
            newDims[i]++;
        }
        nVals2 *= newDims[i];
    }
        
    float *var2 = (float *) malloc(nComps*nVals2*sizeof(float));
#ifdef DEBUG
    for (i = 0; i < nComps*nVals2; i++)
        var2[i] = -1.;
#endif

    //
    // Exchange the data with the neighboring processes.
    //
    int min[3], max[3];

#ifdef PARALLEL
    nRecv = 0;
    nSend = 0;
#endif

    if (iBlock[0] < nBlocks[0] - 1)
    {
        //
        // Exchange data with (+1,0,0).
        //
        min[0] = dims[0]-1; max[0] = dims[0]-1 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]+0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 102, 101);
    }

    if (iBlock[0] > 0)
    {
        //
        // Exchange data with (-1,0,0).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-0)*nBlocks[2] +
                           (iBlock[2]-0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 101, 102);
    }

    if (iBlock[1] < nBlocks[1] - 1)
    {
        //
        // Exchange data with (0,+1,0).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = dims[1]-1; max[1] = dims[1]-1 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 104, 103);
    }

    if (iBlock[1] > 0)
    {
        //
        // Exchange data with (0,-1,0).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]-0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]-0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 103, 104);
    }

    if (iBlock[2] < nBlocks[2] - 1)
    {
        //
        // Exchange data with (0,0,+1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = 0; max[1] = dims[1];
        min[2] = dims[2]-1; max[2] = dims[2]-1 + 1;
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 106, 105);
    }

    if (iBlock[2] > 0)
    {
        //
        // Exchange data with (0,0,-1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = 0; max[1] = dims[1];
        min[2] = 0; max[2] = 0 + 1;
        int exchangeProc = (iBlock[0]-0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-0)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 105, 106);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1))
    {
        //
        // Exchange data with (+1,+1,0).
        //
        min[0] = dims[0]-1; max[0] = dims[0]-1 + 1;
        min[1] = dims[1]-1; max[1] = dims[1]-1 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 108, 107);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0))
    {
        //
        // Exchange data with (-1,-1,0).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]+0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 107, 108);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (+1,0,+1).
        //
        min[0] = dims[0]-1; max[0] = dims[0]-1 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = dims[2]-1; max[2] = dims[2]-1 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 110, 109);
    }

    if ((iBlock[0] > 0) && (iBlock[2] > 0))
    {
        //
        // Exchange data with (-1,0,-1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = 0; max[2] = 0 + 1;
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 109, 110);
    }

    if ((iBlock[1] < nBlocks[1] - 1) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (0,+1,+1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = dims[1]-1; max[1] = dims[1]-1 + 1;
        min[2] = dims[2]-1; max[2] = dims[2]-1 + 1;
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 112, 111);
    }

    if ((iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Exchange data with (0,-1,-1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = 0 + 1;
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 111, 112);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] > 0))
    {
        //
        // Exchange data with (+1,-1,0).
        //
        min[0] = dims[0]-1; max[0] = dims[0]-1 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]+0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 114, 113);
    }

    if ((iBlock[0] > 0) && (iBlock[1] < nBlocks[1] - 1))
    {
        //
        // Exchange data with (-1,+1,0).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = dims[1]-1; max[1] = dims[1]-1 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+0);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 113, 114);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[2] > 0))
    {
        //
        // Exchange data with (+1,0,-1).
        //
        min[0] = dims[0]-1; max[0] = dims[0]-1 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = 0; max[2] = 0 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 116, 115);
    }

    if ((iBlock[0] > 0) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (-1,0,+1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = dims[2]-1; max[2] = dims[2]-1 + 1;
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 115, 116);
    }

    if ((iBlock[1] < nBlocks[1] - 1) && (iBlock[2] > 0))
    {
        //
        // Exchange data with (0,+1,-1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = dims[1]-1; max[1] = dims[1]-1 + 1;
        min[2] = 0; max[2] = 0 + 1;
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 118, 117);
    }

    if ((iBlock[1] > 0) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (0,-1,+1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = 0; max[1] = 0 + 1;
        min[2] = dims[2]-1; max[2] = dims[2]-1 + 1;
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 117, 118);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (+1,+1,+1).
        //
        min[0] = dims[0]-1; max[0] = dims[0]-1 + 1;
        min[1] = dims[1]-1; max[1] = dims[1]-1 + 1;
        min[2] = dims[2]-1; max[2] = dims[2]-1 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 120, 119);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Exchange data with (-1,-1,-1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = 0 + 1;
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 119, 120);
    }

    if ((iBlock[0] > 0) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (-1,+1,+1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = dims[1]-1; max[1] = dims[1]-1 + 1;
        min[2] = dims[2]-1; max[2] = dims[2]-1 + 1;
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 122, 121);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Exchange data with (+1,-1,-1).
        //
        min[0] = dims[0]-1; max[0] = dims[0]-1 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = 0 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 121, 122);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] > 0) &&
        (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (+1,-1,+1).
        //
        min[0] = dims[0]-1; max[0] = dims[0]-1 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = dims[2]-1; max[2] = dims[2]-1 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 124, 123);
    }

    if ((iBlock[0] > 0) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] > 0))
    {
        //
        // Exchange data with (-1,+1,-1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = dims[1]-1; max[1] = dims[1]-1 + 1;
        min[2] = 0; max[2] = 0 + 1;
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 123, 124);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] > 0))
    {
        //
        // Exchange data with (+1,+1,-1).
        //
        min[0] = dims[0]-1; max[0] = dims[0]-1 + 1;
        min[1] = dims[1]-1; max[1] = dims[1]-1 + 1;
        min[2] = 0; max[2] = 0 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 126, 125);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Exchange data with (-1,-1,+1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = dims[2]-1; max[2] = dims[2]-1 + 1;
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartExchange3D(exchangeProc, var, nComps, min, max, dims, 125, 126);
    }

    //
    // Copy data for (0,0,0).
    //
    int ndx = 0;
    int k;
    for (k = 0; k < dims[2]; k++)
    {
        int j;
        for (j = 0; j < dims[1]; j++)
        {
            int i;
            for (i = 0; i < dims[0]; i++)
            {
                int ii;
                for (ii = 0; ii < nComps; ii++)
                {
                    int ndx2 = (k+mins[2]) * newDims[0] * newDims[1] +
                               (j+mins[1]) * newDims[0] + (i+mins[0]);
                    var2[ndx2*nComps+ii] = var[ndx++];
                }
            }
        }
    }

#ifdef PARALLEL
    //
    // Wait for the communication to complete.
    //
    MPI_Status recvStatus[26];
    MPI_Status sendStatus[26];
    MPI_Waitall(nRecv, recvReq, recvStatus);
    MPI_Waitall(nSend, sendReq, sendStatus);

    nRecv = 0;
    nSend = 0;
#endif

    //
    // Handle the recieved data.
    //
    if (iBlock[0] < nBlocks[0] - 1)
    {
        //
        // Handle the data from (+1,0,0).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if (iBlock[0] > 0)
    {
        //
        // Handle the data from (-1,0,0).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if (iBlock[1] < nBlocks[1] - 1)
    {
        //
        // Handle the data from (0,+1,0).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if (iBlock[1] > 0)
    {
        //
        // Handle the data from (0,-1,0).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = 0; max[1] = 0 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if (iBlock[2] < nBlocks[2] - 1)
    {
        //
        // Handle the data from (0,0,+1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if (iBlock[2] > 0)
    {
        //
        // Handle the data from (0,0,-1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1))
    {
        //
        // Handle the data from (+1,+1,0).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0))
    {
        //
        // Handle the data from (-1,-1,0).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (+1,0,+1).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[2] > 0))
    {
        //
        // Handle the data from (-1,0,-1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[1] < nBlocks[1] - 1) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (0,+1,+1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Handle the data from (0,-1,-1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] > 0))
    {
        //
        // Handle the data from (+1,-1,0).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] < nBlocks[1] - 1))
    {
        //
        // Handle the data from (-1,+1,0).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[2] > 0))
    {
        //
        // Handle the data from (+1,0,-1).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (-1,0,+1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[1] < nBlocks[1] - 1) && (iBlock[2] > 0))
    {
        //
        // Handle the data from (0,+1,-1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[1] > 0) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (0,-1,+1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = 0; max[1] = 0 + 1;
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (+1,+1,+1).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Handle the data from (-1,-1,-1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (-1,+1,+1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Handle the data from (+1,-1,-1).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] > 0) &&
        (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (+1,-1,+1).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] > 0))
    {
        //
        // Handle the data from (-1,+1,-1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] > 0))
    {
        //
        // Handle the data from (+1,+1,-1).
        //
        min[0] = newDims[0]-1; max[0] = newDims[0]-1 + 1;
        min[1] = newDims[1]-1; max[1] = newDims[1]-1 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Handle the data from (-1,-1,+1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = newDims[2]-1; max[2] = newDims[2]-1 + 1;
        FinishExchange3D(var2, nComps, min, max, newDims);
    }

    *newVar = var2;
}

void
Create3DNodalGapData(float *var, int *dims, int nComps, int *iBlock,
    int *nBlocks, float **newVar, int newDims[3])
{
    //
    // We are working with 3D data.
    //
    int nDims = 3;

    //
    // Determine the shape of the new variable with ghost zones.
    //
    int mins[3];

    int nVals2 = 1;
    int i;
    for (i = 0; i < nDims; i++)
    {
        mins[i] = 0;
        newDims[i] = dims[i];
        if (iBlock[i] > 0)
        {
            mins[i] = 1;
            newDims[i]++;
        }
        nVals2 *= newDims[i];
    }

    float *var2 = (float *) malloc(nComps*nVals2*sizeof(float));
#ifdef DEBUG
    for (i = 0; i < nComps*nVals2; i++)
        var2[i] = -1.;
#endif

    //
    // Exchange the data with the neighboring processes.
    //
    int nels;
    int min[3], max[3];

#ifdef PARALLEL
    nRecv = 0;
    nSend = 0;
#endif

    if (iBlock[0] > 0)
    {
        //
        // Receive data from (-1,0,0).
        //
        nels = nComps * dims[1] * dims[2];
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-0)*nBlocks[2] +
                           (iBlock[2]-0);
        StartReceive3D(exchangeProc, var, nels, 101);
    }

    if (iBlock[0] < nBlocks[0] - 1)
    {
        //
        // Send data to (+1,0,0).
        //
        min[0] = dims[0]-1; max[0] = dims[0]-1 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]+0);
        StartSend3D(exchangeProc, var, nComps, min, max, dims, 101);
    }

    if (iBlock[1] > 0)
    {
        //
        // Receive data from (0,-1,0).
        //
        nels = nComps * dims[0] * dims[2];
        int exchangeProc = (iBlock[0]-0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]-0);
        StartReceive3D(exchangeProc, var, nels, 103);
    }

    if (iBlock[1] < nBlocks[1] - 1)
    {
        //
        // Send data to (0,+1,0).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = dims[1]-1; max[1] = dims[1]-1 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+0);
        StartSend3D(exchangeProc, var, nComps, min, max, dims, 103);
    }

    if (iBlock[2] > 0)
    {
        //
        // Receive data from (0,0,-1).
        //
        nels = nComps * dims[0] * dims[1];
        int exchangeProc = (iBlock[0]-0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-0)*nBlocks[2] +
                           (iBlock[2]-1);
        StartReceive3D(exchangeProc, var, nels, 105);
    }

    if (iBlock[2] < nBlocks[2] - 1)
    {
        //
        // Send data to (0,0,+1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = 0; max[1] = dims[1];
        min[2] = dims[2]-1; max[2] = dims[2]-1 + 1;
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]+1);
        StartSend3D(exchangeProc, var, nComps, min, max, dims, 105);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0))
    {
        //
        // Receive data from (-1,-1,0).
        //
        nels = nComps * dims[2];
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]+0);
        StartReceive3D(exchangeProc, var, nels, 107);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1))
    {
        //
        // Send data to (+1,+1,0).
        //
        min[0] = dims[0]-1; max[0] = dims[0]-1 + 1;
        min[1] = dims[1]-1; max[1] = dims[1]-1 + 1;
        min[2] = 0; max[2] = dims[2];
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+0);
        StartSend3D(exchangeProc, var, nComps, min, max, dims, 107);
    }

    if ((iBlock[0] > 0) && (iBlock[2] > 0))
    {
        //
        // Receive data from (-1,0,-1).
        //
        nels = nComps * dims[1];
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]-1);
        StartReceive3D(exchangeProc, var, nels, 109);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Send data to (+1,0,+1).
        //
        min[0] = dims[0]-1; max[0] = dims[0]-1 + 1;
        min[1] = 0; max[1] = dims[1];
        min[2] = dims[2]-1; max[2] = dims[2]-1 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+0)*nBlocks[2] +
                           (iBlock[2]+1);
        StartSend3D(exchangeProc, var, nComps, min, max, dims, 109);
    }

    if ((iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Receive data from (0,-1,-1).
        //
        nels = nComps * dims[0];
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartReceive3D(exchangeProc, var, nels, 111);
    }

    if ((iBlock[1] < nBlocks[1] - 1) && (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Send data to (0,+1,+1).
        //
        min[0] = 0; max[0] = dims[0];
        min[1] = dims[1]-1; max[1] = dims[1]-1 + 1;
        min[2] = dims[2]-1; max[2] = dims[2]-1 + 1;
        int exchangeProc = (iBlock[0]+0)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartSend3D(exchangeProc, var, nComps, min, max, dims, 111);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Receive data from (-1,-1,-1).
        //
        nels = nComps;
        int exchangeProc = (iBlock[0]-1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]-1)*nBlocks[2] +
                           (iBlock[2]-1);
        StartReceive3D(exchangeProc, var, nels, 119);
    }

    if ((iBlock[0] < nBlocks[0] - 1) && (iBlock[1] < nBlocks[1] - 1) &&
        (iBlock[2] < nBlocks[2] - 1))
    {
        //
        // Send data to (+1,+1,+1).
        //
        min[0] = dims[0]-1; max[0] = dims[0]-1 + 1;
        min[1] = dims[1]-1; max[1] = dims[1]-1 + 1;
        min[2] = dims[2]-1; max[2] = dims[2]-1 + 1;
        int exchangeProc = (iBlock[0]+1)*(nBlocks[2]*nBlocks[1]) +
                           (iBlock[1]+1)*nBlocks[2] +
                           (iBlock[2]+1);
        StartSend3D(exchangeProc, var, nComps, min, max, dims, 119);
    }

    //
    // Copy data for (0,0,0).
    //
    int ndx = 0;
    int k;
    for (k = 0; k < dims[2]; k++)
    {
        int j;
        for (j = 0; j < dims[1]; j++)
        {
            int i;
            for (i = 0; i < dims[0]; i++)
            {
                int ii;
                for (ii = 0; ii < nComps; ii++)
                {
                    int ndx2 = (k+mins[2]) * (newDims[0]) * (newDims[1]) +
                               (j+mins[1]) * (newDims[0]) + (i+mins[0]);
                    var2[ndx2*nComps+ii] = var[ndx++];
                }
            }
        }
    }

#ifdef PARALLEL
    //
    // Wait for the communication to complete.
    //
    MPI_Status recvStatus[26];
    MPI_Status sendStatus[26];
    MPI_Waitall(nRecv, recvReq, recvStatus);
    MPI_Waitall(nSend, sendReq, sendStatus);

    //
    // Handle the recieved data.
    //
    nRecv = 0;
    nSend = 0;
#endif

    if (iBlock[0] > 0)
    {
        //
        // Receive the data from (-1,0,0).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishReceive3D(var2, nComps, min, max, newDims);
    }

    if (iBlock[1] > 0)
    {
        //
        // Receive the data from (0,-1,0).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = 0; max[1] = 0 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishReceive3D(var2, nComps, min, max, newDims);
    }

    if (iBlock[2] > 0)
    {
        //
        // Receive the data from (0,0,-1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = 0; max[2] = 0 + 1;
        FinishReceive3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0))
    {
        //
        // Receive the data from (-1,-1,0).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = mins[2]; max[2] = mins[2] + dims[2];
        FinishReceive3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[2] > 0))
    {
        //
        // Receive the data from (-1,0,-1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = mins[1]; max[1] = mins[1] + dims[1];
        min[2] = 0; max[2] = 0 + 1;
        FinishReceive3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Receive the data from (0,-1,-1).
        //
        min[0] = mins[0]; max[0] = mins[0] + dims[0];
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishReceive3D(var2, nComps, min, max, newDims);
    }

    if ((iBlock[0] > 0) && (iBlock[1] > 0) && (iBlock[2] > 0))
    {
        //
        // Receive the data from (-1,-1,-1).
        //
        min[0] = 0; max[0] = 0 + 1;
        min[1] = 0; max[1] = 0 + 1;
        min[2] = 0; max[2] = 0 + 1;
        FinishReceive3D(var2, nComps, min, max, newDims);
    }

    *newVar = var2;
}

XDMFFile *
XdmfCreate(const char *fileName, double time)
{
    XDMFFileSerial *xdmfFile;

    xdmfFile = (XDMFFileSerial *) malloc(sizeof(XDMFFileSerial));

    xdmfFile->type = 2;
    xdmfFile->file = fopen(fileName, "w");

    fprintf(xdmfFile->file, "<?xml version=\"1.0\" ?>\n");
    fprintf(xdmfFile->file, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xdmfFile->file, "<Xdmf Version=\"2.0\">\n");
    fprintf(xdmfFile->file, " <Information Name=\"Time\" Value=\"%g\"/>\n",
            time);

    return (XDMFFile *) xdmfFile;
}

void
XdmfPutUcdGrid(XDMFFile *xdmfFileIn, const char *gridFileName,
    const char *varFileName, const char *gridName, const char *coordName,
    int coordDataType, int nCoords, int coordDims, const char *connectivityName,
    int cellType, int nCells, int connectivityLength, int nVars,
    char **varNames, int *varTypes, int *varCentering, int *varDataTypes)
{
    XDMFFileSerial *xdmfFile = (XDMFFileSerial *) xdmfFileIn;

    if (xdmfFile == NULL)
        return;
    if (xdmfFile->type != 2)
        return;

    fprintf(xdmfFile->file, " <Domain>\n");

    XDMFPutUcdGrid(xdmfFile->file, gridFileName, varFileName, gridName,
        XDMF_NULL_GRID, coordName, coordDataType, nCoords, coordDims,
        connectivityName, cellType, nCells, connectivityLength,
        nVars, varNames, varTypes, varCentering, varDataTypes);

    fprintf(xdmfFile->file, " </Domain>\n");
}

void
XdmfWriteCurvVar(XDMFFile *xdmfFileIn, const char *gridFileName,
    const char *varFileName, const char *gridName, const char *coordName,
    int gridDataType, int nVars, char **varNames, int *varTypes,
    int *varCentering, int *varDataTypes, int nDims, int *dims)
{
    XDMFFileSerial *xdmfFile = (XDMFFileSerial *) xdmfFileIn;

    if (xdmfFile == NULL)
        return;
    if (xdmfFile->type != 2)
        return;

    fprintf(xdmfFile->file, " <Domain>\n");

    XDMFWriteCurvBlock(xdmfFile->file, gridFileName, varFileName, gridName,
        coordName, XDMF_NULL_GRID, gridDataType, nVars, varNames, varTypes,
        varCentering, varDataTypes, nDims, dims, NULL, NULL);

    fprintf(xdmfFile->file, " </Domain>\n");
}

void
XdmfStartMultiBlock(XDMFFile *xdmfFileIn, const char *blockName)
{
    XDMFFileSerial *xdmfFile = (XDMFFileSerial *) xdmfFileIn;

    if (xdmfFile == NULL)
        return;
    if (xdmfFile->type != 2)
        return;

    fprintf(xdmfFile->file, " <Domain>\n");
    fprintf(xdmfFile->file, "  <Grid Name=\"%s\" GridType=\"Collection\">\n",
            blockName);
}

void
XdmfWriteCurvBlock(XDMFFile *xdmfFileIn, const char *fileName,
    const char *blockName, const char *coordName, int iBlock, 
    int gridDataType, int nVars, char **varNames, int *varTypes,
    int *varCentering, int *varDataTypes, int nDims, int *dims,
    int *baseIndex, int *ghostOffsets)
{
    XDMFFileSerial *xdmfFile = (XDMFFileSerial *) xdmfFileIn;

    if (xdmfFile == NULL)
        return;
    if (xdmfFile->type != 2)
        return;

    XDMFWriteCurvBlock(xdmfFile->file, fileName, fileName, blockName,
        coordName, iBlock, gridDataType, nVars, varNames, varTypes,
        varCentering, varDataTypes, nDims, dims, baseIndex, ghostOffsets);
}

void
XdmfEndMultiBlock(XDMFFile *xdmfFileIn)
{
    XDMFFileSerial *xdmfFile = (XDMFFileSerial *) xdmfFileIn;

    if (xdmfFile == NULL)
        return;
    if (xdmfFile->type != 2)
        return;

    fprintf(xdmfFile->file, "  </Grid>\n");
    fprintf(xdmfFile->file, " </Domain>\n");
}

void
XdmfClose(XDMFFile *xdmfFileIn)
{
    XDMFFileSerial *xdmfFile = (XDMFFileSerial *) xdmfFileIn;

    if (xdmfFile == NULL)
        return;
    if (xdmfFile->type != 2)
        return;

    fprintf(xdmfFile->file, "</Xdmf>\n");

    fclose(xdmfFile->file);

    free(xdmfFile);
}

HDFFile *
HdfCreate(const char *fileName)
{
    if (fileName == NULL)
        return NULL;

    dataTypeToHDFType[0] = H5T_NATIVE_FLOAT;
    dataTypeToHDFType[1] = H5T_NATIVE_DOUBLE;
    dataTypeToHDFType[2] = H5T_NATIVE_INT;
    dataTypeToHDFType[3] = H5T_NATIVE_CHAR; 

    HDFFileSerial *hdfFile = (HDFFileSerial *) malloc(sizeof(HDFFileSerial));

    hdfFile->type = 2;
    hdfFile->fileId = H5Fcreate(fileName, H5F_ACC_TRUNC, H5P_DEFAULT,
                                H5P_DEFAULT);

    return (HDFFile *) hdfFile;
}

void
HdfPutCoords(HDFFile *hdfFileIn, const char *coordName,
    int coordDataType, float *coords, int nCoords)
{
    HDFFileSerial *hdfFile = (HDFFileSerial *) hdfFileIn;

    if (hdfFile == NULL)
        return;
    if (hdfFile->type != 2)
        return;
    if (coordName == NULL)
        return;
    if (coordDataType < XDMF_FLOAT || coordDataType > XDMF_CHAR)
        return;
    if (coords == NULL)
        return;
    if (nCoords <= 0)
        return;

    hid_t     dataspace_id, dataset_id;
    hsize_t   vdims[4];
    herr_t    status;

    vdims[0] = nCoords;
    vdims[1] = 3;
    dataspace_id = H5Screate_simple(2, vdims, NULL);

    dataset_id = H5Dcreate(hdfFile->fileId, coordName,
                           dataTypeToHDFType[coordDataType],
                           dataspace_id, H5P_DEFAULT,
                           H5P_DEFAULT, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, dataTypeToHDFType[coordDataType], H5S_ALL,
                      H5S_ALL, H5P_DEFAULT, coords);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    (void) status;
}

void
HdfPutConnectivity(HDFFile *hdfFileIn, const char *connectivityName,
    int connectivityDataType, void *connectivity, int connectivityLength)
{
    HDFFileSerial *hdfFile = (HDFFileSerial *) hdfFileIn;

    if (hdfFile == NULL)
        return;
    if (hdfFile->type != 2)
        return;
    if (connectivityName == NULL)
        return;
    if (connectivityDataType < XDMF_FLOAT || connectivityDataType > XDMF_CHAR)
        return;
    if (connectivity == NULL)
        return;
    if (connectivityLength <= 0)
        return;

    hid_t     dataspace_id, dataset_id;
    hsize_t   vdims[4];
    herr_t    status;

    vdims[0] = connectivityLength;
    dataspace_id = H5Screate_simple(1, vdims, NULL);

    dataset_id = H5Dcreate(hdfFile->fileId, connectivityName,
                           dataTypeToHDFType[connectivityDataType],
                           dataspace_id, H5P_DEFAULT,
                           H5P_DEFAULT, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, dataTypeToHDFType[connectivityDataType],
                      H5S_ALL, H5S_ALL, H5P_DEFAULT, connectivity);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    (void) status;
}

void
HdfPutCurvVar(HDFFile *hdfFileIn, const char *varName, int varType,
    int varCentering, int varDataType, void *var, int nDims, int *dims)
{
    HDFFileSerial *hdfFile = (HDFFileSerial *) hdfFileIn;

    if (hdfFile == NULL)
        return;
    if (hdfFile->type != 2)
        return;
    if (varName == NULL)
        return;
    if (varType < XDMF_SCALAR || varType > XDMF_VECTOR)
        return;
    if (varCentering < XDMF_CELL_CENTER || varCentering > XDMF_NODE_CENTER)
        return;
    if (varDataType < XDMF_FLOAT || varDataType > XDMF_CHAR)
        return;
    if (var == NULL)
        return;
    if (nDims < 1 || nDims > 3)
        return;
    if (dims == NULL)
        return;

    hid_t     dataspace_id, dataset_id;
    hsize_t   nvdims, vdims[4];
    herr_t    status;

    nvdims = nDims;
    if (varCentering == XDMF_CELL_CENTER)
    {
        int i;
        for (i = 0; i < nDims; i++)
            vdims[i] = dims[i];
    }
    else
    {
        int i;
        for (i = 0; i < nDims; i++)
            vdims[i] = (dims[i] + 1);
    }
    if (varType == XDMF_VECTOR)
    {
        vdims[nDims] = nDims;
        nvdims++;
    }

    dataspace_id = H5Screate_simple(nvdims, vdims, NULL);

    dataset_id = H5Dcreate(hdfFile->fileId, varName,
                           dataTypeToHDFType[varDataType],
                           dataspace_id, H5P_DEFAULT,
                           H5P_DEFAULT, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, dataTypeToHDFType[varDataType],
                      H5S_ALL, H5S_ALL, H5P_DEFAULT, var);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    (void) status;
}

void
HdfPutUcdVar(HDFFile *hdfFileIn, const char *varName, int varType,
    int varCentering, int varDataType, void *var, int varDim,
    int nCoords, int nCells)
{
    HDFFileSerial *hdfFile = (HDFFileSerial *) hdfFileIn;

    if (hdfFile == NULL)
        return;
    if (hdfFile->type != 2)
        return;
    if (varName == NULL)
        return;
    if (varType < XDMF_SCALAR || varType > XDMF_VECTOR)
        return;
    if (varCentering < XDMF_CELL_CENTER || varCentering > XDMF_NODE_CENTER)
        return;
    if (varDataType < XDMF_FLOAT || varDataType > XDMF_CHAR)
        return;
    if (var == NULL)
        return;
    if (nCoords <= 0 || nCells <= 0)
        return;

    hid_t     dataspace_id, dataset_id;
    hsize_t   nvdims, vdims[4];
    herr_t    status;

    nvdims = 1;
    if (varCentering == XDMF_CELL_CENTER)
    {
        vdims[0] = nCells;
    }
    else
    {
        vdims[0] = nCoords;
    }
    if (varType == XDMF_VECTOR)
    {
        vdims[1] = varDim;
        nvdims++;
    }

    dataspace_id = H5Screate_simple(nvdims, vdims, NULL);

    dataset_id = H5Dcreate(hdfFile->fileId, varName,
                           dataTypeToHDFType[varDataType],
                           dataspace_id, H5P_DEFAULT,
                           H5P_DEFAULT, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, dataTypeToHDFType[varDataType],
                      H5S_ALL, H5S_ALL, H5P_DEFAULT, var);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    (void) status;
}

void
HdfClose(HDFFile *hdfFileIn)
{
    HDFFileSerial *hdfFile = (HDFFileSerial *) hdfFileIn;

    if (hdfFile == NULL)
        return;
    if (hdfFile->type != 2)
        return;

    herr_t    status;
    status = H5Fclose(hdfFile->fileId);
    (void) status;
    free(hdfFile);
}
