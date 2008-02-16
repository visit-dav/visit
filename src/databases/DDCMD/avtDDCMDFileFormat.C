/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtDDCMDFileFormat.C                           //
// ************************************************************************* //

#include <avtDDCMDFileFormat.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <vector>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtIOInformation.h>

#include <Expression.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <Utility.h>

#include <object.h>

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

using     std::string;
using     std::vector;
using     std::min;
using     std::max;

// ****************************************************************************
//  Method: avtDDCMDFileFormat::Convert
//
//  Purpose:
//      Byte swap the data.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
// ****************************************************************************

void
avtDDCMDFileFormat::Convert(void *ptr, int size)
{
    int i,j;
    char *b,save;
    b =(char *) (ptr) ;
    j = size;
    for (i=0;i<size/2;i++)
    {
        --j;
        save = b[i] ;
        b[i] = b[j];
        b[j] = save;
    }
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::DetermineBlockDecomposition
//
//  Purpose:
//      Determine the decomposition of the mesh into blocks. If we are
//      running in serial we break it into 1 block, otherwise we use
//      avtDatabase::ComputeRectilinearDecomposition.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
// ****************************************************************************

void
avtDDCMDFileFormat::DetermineBlockDecomposition()
{
#ifdef PARALLEL
    int nProcs = PAR_Size();
    if (nDims == 2)
    {
        //
        // The arrays are intentionally ordered Y, Z, X because the
        // function does poor decompositions when the X dimension is 1.
        //
        avtDatabase::ComputeRectilinearDecomposition(nDims, nProcs, nYFile,
            nZFile, nXFile, &nYFileBlocks, &nZFileBlocks, &nXFileBlocks);
    }
    else
    {
        avtDatabase::ComputeRectilinearDecomposition(nDims, nProcs, nXFile,
            nYFile, nZFile, &nXFileBlocks, &nYFileBlocks, &nZFileBlocks);
    }
#else
    nXFileBlocks = 1;
    nYFileBlocks = 1;
    nZFileBlocks = 1;
#endif
    nBlocks = nXFileBlocks * nYFileBlocks * nZFileBlocks;
    if (nDims == 2)
    {
        nXMeshBlocks = nYFileBlocks;
        nYMeshBlocks = nZFileBlocks;
        nZMeshBlocks = 1;
    }
    else
    {
        nXMeshBlocks = nXFileBlocks;
        nYMeshBlocks = nYFileBlocks;
        nZMeshBlocks = nZFileBlocks;
    }

    debug1 << "nXFileBlocks=" << nXFileBlocks
           << ",nYFileBlocks=" << nYFileBlocks
           << ",nZFileBlocks=" << nZFileBlocks << endl;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::ExchangeProcessorData
//
//  Purpose:
//      Exchange the data between the processors into spacially contiguous
//      blocks.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
// ****************************************************************************

void
avtDDCMDFileFormat::ExchangeProcessorData()
{
#ifdef PARALLEL
    int rank = PAR_Rank();
    int nProcs = PAR_Size();

    //
    // Determine the number of records to send to each processor.  Each
    // processor gets exactly one block, so the processor number is the
    // same as the block number.
    //
    int *outCharCounts = new int[nProcs];
    for (int i = 0; i < nProcs; i++)
        outCharCounts[i] = 0;
    
    int deltaX = (nXFile + nXFileBlocks - 1) / nXFileBlocks;
    int deltaY = (nYFile + nYFileBlocks - 1) / nYFileBlocks;
    int deltaZ = (nZFile + nZFileBlocks - 1) / nZFileBlocks;

    long   nRecords = nRecordsList[rank];

    char   *data = readProcessorData;

    for (int i = 0; i < nRecords; i++)
    {
        //
        // Byte swap the data if necessary.
        //
        unsigned id = *((unsigned *) (data+labelOffset));
        if (swap) Convert(&id, 4);

        int iX = id / (nYFile * nZFile);
        id %= (nYFile * nZFile);
        int iY = id / nZFile;
        int iZ = id % nZFile;

        int iXBlock = iX / deltaX;
        int iYBlock = iY / deltaY;
        int iZBlock = iZ / deltaZ;
        int iBlock = iXBlock * nYFileBlocks * nZFileBlocks +
                     iYBlock * nZFileBlocks + iZBlock;

        outCharCounts[iBlock] += lRec;
        
        data += lRec;
    }

    debug1 << "outCharCounts=";
    for (int i = 0; i < nProcs; i++)
        debug1 << outCharCounts[i] << "(" << outCharCounts[i] / lRec << "),";
    debug1 << endl;

    //
    // Determine the offsets into the output buffer for each of the processors.
    //
    int *outProcOffsets = new int[nProcs];
    outProcOffsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        outProcOffsets[i] = outProcOffsets[i-1] + outCharCounts[i-1];

    debug1 << "outProcOffsets=";
    for (int i = 0; i < nProcs; i++)
        debug1 << outProcOffsets[i] << "(" << outProcOffsets[i] / lRec << "),";
    debug1 << endl;

    //
    // Transfer the input buffer contents to the output buffer.
    //
    char *outProcessorData = new char[nRecords*lRec];

    data = readProcessorData;

    for (int i = 0; i < nRecords; i++)
    {
        //
        // Byte swap the data if necessary.
        //
        unsigned id = *((unsigned *) (data+labelOffset));
        if (swap) Convert(&id, 4);

        int iX = id / (nYFile * nZFile);
        id %=  (nYFile * nZFile);
        int iY = id / nZFile;
        int iZ = id % nZFile;

        int iXBlock = iX / deltaX;
        int iYBlock = iY / deltaY;
        int iZBlock = iZ / deltaZ;
        int iBlock = iXBlock * nYFileBlocks * nZFileBlocks +
                     iYBlock * nZFileBlocks + iZBlock;

        memcpy(outProcessorData + outProcOffsets[iBlock], data, lRec);

        outProcOffsets[iBlock] += lRec;
        
        data += lRec;
    }

    //
    // Recalculate the offsets into the output buffer for each of the
    // processors since we modified them when we transfered the input
    // buffer to the output buffer.
    //
    outProcOffsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        outProcOffsets[i] = outProcOffsets[i-1] + outCharCounts[i-1];

    //
    // Do an all to all communication so that each processor can determine
    // the amountof data it will recieve from each processor.
    //
    int *inCharCounts = new int[nProcs];
    MPI_Alltoall(outCharCounts, 1, MPI_INT, inCharCounts, 1, MPI_INT,
                 VISIT_MPI_COMM);

    debug1 << "inCharCounts=";
    for (int i = 0; i < nProcs; i++)
        debug1 << inCharCounts[i] << "(" << inCharCounts[i] / lRec << "),";
    debug1 << endl;

    //
    // Determine the offsets into the input buffer for each of the processors.
    //
    int *inProcOffsets = new int[nProcs];
    inProcOffsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        inProcOffsets[i] = inProcOffsets[i-1] + inCharCounts[i-1];

    debug1 << "inProcOffsets=";
    for (int i = 0; i < nProcs; i++)
        debug1 << inProcOffsets[i] << "(" << inProcOffsets[i] / lRec << "),";
    debug1 << endl;

    //
    // Allocate the input buffer to recieve the data from the other processors.
    //
    nInRecords = inProcOffsets[nProcs - 1] + inCharCounts[nProcs - 1];
    inProcessorData = new char[nInRecords];
    
    //
    // Exchange the data among processors.
    //
    MPI_Alltoallv(outProcessorData, outCharCounts, outProcOffsets, MPI_CHAR,
                  inProcessorData, inCharCounts, inProcOffsets, MPI_CHAR,
                  VISIT_MPI_COMM);

    //
    // Free memory. We no longer need the read processor data so it is being
    // freed and inProcessorData is the output of the routine so it is
    // not being freed.
    //
    delete [] outCharCounts;
    delete [] outProcOffsets;
    delete [] outProcessorData;
    delete [] inCharCounts;
    delete [] inProcOffsets;
    delete [] readProcessorData;
    readProcessorData = 0;
#else
    nInRecords = nRecordsList[0] * lRec;
    inProcessorData = readProcessorData;
    readProcessorData = 0;
#endif

    //
    // Free memory.
    //
    delete [] nRecordsList;
    nRecordsList = 0;
    delete [] fileNumberList;
    fileNumberList = 0;
    delete [] fileOffsetList;
    fileOffsetList = 0;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::CopyExchangeDataToBlocks
//
//  Purpose:
//      Copy the data exchanged with the other processors into spacially
//      contiguous blocks.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
// ****************************************************************************

void
avtDDCMDFileFormat::CopyExchangeDataToBlocks()
{
#ifdef PARALLEL
    int rank = PAR_Rank();
#else
    int rank = 0;
#endif

    int deltaX = (nXFile + nXFileBlocks - 1) / nXFileBlocks;
    int deltaY = (nYFile + nYFileBlocks - 1) / nYFileBlocks;
    int deltaZ = (nZFile + nZFileBlocks - 1) / nZFileBlocks;

    int iBlock = rank;
    int iXBlock = iBlock / (nYFileBlocks * nZFileBlocks);
    int iMin = iXBlock * deltaX;
    int iMax = iMin + deltaX < nXFile ? iMin + deltaX : nXFile;

    iBlock = iBlock % (nYFileBlocks * nZFileBlocks);
    int iYBlock = iBlock / nZFileBlocks;
    int jMin = iYBlock * deltaY;
    int jMax = jMin + deltaY < nYFile ? jMin + deltaY : nYFile;

    int iZBlock = iBlock % nZFileBlocks;
    int kMin = iZBlock * deltaZ;
    int kMax = kMin + deltaZ < nZFile ? kMin + deltaZ : nZFile;

    if (nDims == 2)
    {
        nXBlock = jMax - jMin;
        nYBlock = kMax - kMin;
        nZBlock = 1;
    }
    else
    {
        nXBlock = iMax - iMin;
        nYBlock = jMax - jMin;
        nZBlock = kMax - kMin;
    }
    nZonesBlock = (iMax - iMin) * (jMax - jMin) * (kMax - kMin);
       
    int nZones = nZonesBlock;

    varsBlock = new float*[nVars];
    for (int i = 0; i < nVars; i++)
    {
        varsBlock[i] = new float[nZones*nSpecies];
        memset(varsBlock[i], 0, nZones*nSpecies*sizeof(float));
    }

    //
    // Copy the processor data to the blocks.
    //
    long   nRecords = nInRecords / lRec;

    char   *data = inProcessorData;

    for (int i = 0; i < nRecords; i++)
    {
        //
        // Byte swap the data if necessary.
        //
        char *field = data;
        for (int j = 0; j < nFields; j++)
        {
            if (swap && fieldTypes[j][0] != (char)'s') Convert(field, fieldSizes[j]);
            field += fieldSizes[j];
        }

        //
        // Copy the record into the correct zone in the correct block.
        //
        unsigned id = *((unsigned *) (data+labelOffset));
        unsigned iSpecies = *((unsigned *) (data+iSpeciesOffset));

        int iX = id / (nYFile * nZFile);
        id %= (nYFile * nZFile);
        int iY = id / nZFile;
        int iZ = id % nZFile;

        int iXZone, iYZone, iZZone;
        if  (nDims == 2)
        {
            iXZone = iY % deltaY;
            iYZone = iZ % deltaZ;
            iZZone = 0;
        }
        else
        {
            iXZone = iX % deltaX;
            iYZone = iY % deltaY;
            iZZone = iZ % deltaZ;
        }
        int iZone = iZZone * nXBlock * nYBlock +
                    iYZone * nXBlock + iXZone;

        int nZones = nZonesBlock;

        float **vars = varsBlock;
        for (int j = 0; j < nVars; j++)
        {
            float *fvar = vars[j];
            if (varFloat[j])
            {
                fvar[iSpecies*nZones+iZone] +=
                    *((float *) (data+varOffsets[j]));
            }
            else
            {
                fvar[iSpecies*nZones+iZone] +=
                    float(*((unsigned *) (data+varOffsets[j])));
            }
        }
        data += lRec;
    }

    delete [] varOffsets;
    varOffsets = 0;
    delete [] varSizes;
    varSizes = 0;
    delete [] varFloat;
    varFloat = 0;
    delete [] inProcessorData;
    inProcessorData = 0;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::ReadProcessorChunk
//
//  Purpose:
//      Read this processors chunk of the data.  This is everything but
//      the header.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
//  Modifications:
//    Kathleen Bonnell, Thu Oct 11 10:11:20 PDT 2007
//    Add the 'b' designation to fopen commands.
// 
//    Hank Childs, Fri Feb 15 15:58:07 PST 2008
//    Add a check for a valid file name.
//
// ****************************************************************************

void
avtDDCMDFileFormat::ReadProcessorChunk()
{
#ifdef PARALLEL
    int rank = PAR_Rank();
#else
    int rank = 0;
#endif

    char          string[1024];
    char         *buffer;
    FILE         *file;

    long   nRecords   = nRecordsList[rank];
    int    fileNumber = fileNumberList[rank];
    off_t  fileOffset = fileOffsetList[rank];
    
    readProcessorData = new char[nRecords*lRec];

    //
    // Open the first file to read and position the file pointer at the
    // correct spot.
    //
    sprintf(string, "%s/cgrid#%6.6d", fname.c_str(), fileNumber);
    debug1 << "Opening " << string << endl;
    file = fopen(string, "rb");
    if (file == NULL)
    {
        EXCEPTION1(InvalidFilesException, string);
    }
    fseek(file, fileOffset, SEEK_SET);
    debug1 << "fileOffset=" << fileOffset << ",nRecords=" << nRecords << endl;

    //
    // Read this processors chunk of the data.
    //
    buffer = readProcessorData;
    while (nRecords > 0)
    {
        if (feof(file))
        {
            fclose(file);
            fileNumber++;
            if (fileNumber < nFiles)
            {
                sprintf(string, "%s/cgrid#%6.6d", fname.c_str(), fileNumber);
                debug1 << "Opening " << string << endl;
                file = fopen(string, "rb");
                if (file == NULL)
                {
                    EXCEPTION1(InvalidFilesException, string);
                }
            }
        }
        int cnt = fread(buffer, lRec, nRecords, file);
        debug1 << "Reading " << nRecords << " records, read " << cnt << " records" << endl;
        nRecords -= cnt;
        buffer += cnt * lRec;
    } 

    fclose(file);
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::DetermineProcessorReadOffset
//
//  Purpose:
//      Determine the offsets into the file for each processor.  This
//      includes determining the file, the offset into the file and the
//      number of bytes to read.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
// ****************************************************************************

void
avtDDCMDFileFormat::DetermineProcessorReadOffset()
{
#ifdef PARALLEL
    int nProcs = PAR_Size();
#else
    int nProcs = 1;
#endif

    //
    // Determine the size of each file.
    //
    off_t *fileSizeList = new off_t[nFiles];
    off_t fileSize = 0;
    for (int i = 0; i < nFiles; i++)
    {
        char          string[1024];
        VisItStat_t   statbuf;

        sprintf(string, "%s/cgrid#%6.6d", fname.c_str(), i);

        int rc = VisItStat(string, &statbuf);
        fileSizeList[i] = statbuf.st_size; 
        fileSize += fileSizeList[i];
    } 
    fileSize -= headerLength;

    debug1 << "fileSizes=";
    for (int i = 0; i < nFiles; i++)
    {
        debug1 << fileSizeList[i] << ",";
    }
    debug1 << endl;

    //
    // Allocate space for file offset information for each processor.
    //
    nRecordsList   = new long[nProcs];
    fileNumberList = new int[nProcs];
    fileOffsetList = new off_t[nProcs];

    off_t nRecordsTotal = fileSize / lRec;
    off_t nRecordsPerProc = (nRecordsTotal + nProcs - 1) / nProcs;

    int iFile = 0;
    int fileOffset = headerLength;
    for (int i = 0; i < nProcs; i++)
    {
        int nRec = nRecordsPerProc < nRecordsTotal ?
                          nRecordsPerProc : nRecordsTotal;

        nRecordsList[i] = nRec;
        fileNumberList[i] = iFile;
        fileOffsetList[i] = fileOffset;

        nRecordsTotal -= nRec;
        fileOffset += nRec * lRec;
        while (fileOffset > fileSizeList[iFile])
        {
            fileOffset -= fileSizeList[iFile];
            iFile++;
        }
    } 

    delete [] fileSizeList;

    for (int i = 0; i < nProcs; i++)
    {
       debug1 << "nRecords=" << nRecordsList[i] << ",fileNumber=" << fileNumberList[i] << ",fileOffset=" << fileOffsetList[i] << endl;
    }
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::ReadData
//
//  Purpose:
//      Read in the data portion of the cgrid file.  This is everything but
//      the header.  Each processor reads in a portion of the data and then
//      the data is exchanged among the processors into spacially contiguous
//      blocks.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
// ****************************************************************************

void
avtDDCMDFileFormat::ReadData()
{
#ifdef MDSERVER
    return;
#endif 

    DetermineProcessorReadOffset();

    DetermineBlockDecomposition();

    ReadProcessorChunk();

    ExchangeProcessorData();

    CopyExchangeDataToBlocks();
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::ReadHeader
//
//  Purpose:
//      Read the header of the cgrid file.  The file is opened, the header
//      read, and the file is closed again.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
//  Modifications:
//
//    Hank Childs, Fri Feb 15 16:00:20 PST 2008
//    Fix memory leak.
//
// ****************************************************************************

void
avtDDCMDFileFormat::ReadHeader(const char *filename)
{
    int           i, headerlength;
    char          string[4096], *header, *h;

    //
    // Open the file and read the header. It is a bunch of lines terminated
    // with a null character.
    //
    int           maxlen;
    FILE         *file;

    sprintf(string, "%s/cgrid#000000", filename);

    file = fopen(string, "r");
    maxlen = 4096;
    header = string;
    headerlength = 0;
    fgets(header, maxlen - headerlength, file);
    headerlength = strlen(header);
    fgets(h = header + headerlength, maxlen - headerlength, file);
    while(*h != '\n')
    {
        headerlength = strlen(header);
        fgets(h = header + headerlength, maxlen-headerlength, file);
    }
    headerlength = strlen(header);
    fclose(file);

    //
    // Convert any newline characters to null characters.
    //
    for (i = 0; i < headerlength; i++)
        if (header[i] == '\n' ) header[i] = ' ';
    header[i] = '\0';

    headerLength = headerlength;

    //
    // Parse the header.
    //
    OBJECT       *obj;
    unsigned int  endianKey, keyLocal;

    obj = (OBJECT*) malloc(sizeof(OBJECT));
    object_lineparse(header, obj);

    object_get(obj, "lrec", &lRec, INT, 1, "16");
    object_get(obj, "endian_key", &endianKey, INT, 1, "4");
    object_get(obj, "nrecord", &nRecord, INT, 1, "1");
    object_get(obj, "nfiles", &nFiles, INT, 1, "1");
    object_get(obj, "nfields", &nFields, INT, 1, "1");
    object_getv(obj, "field_names", (void **)&fieldNames, STRING);
    object_getv(obj, "field_types", (void **)&fieldTypes, STRING);
    fieldSizes = (unsigned*) malloc(nFields*sizeof(int));
    for (i = 0; i < nFields; i++)
    {
        fieldSizes[i] = atoi(fieldTypes[i] + 1);
    }

    object_get(obj, "h", &hMatrix, DOUBLE, 9, "1000.0");
    object_get(obj, "time", &time, DOUBLE, 1, "0.0");
    object_get(obj, "loop", &loop, INT, 1, "0");

    memcpy(&keyLocal, "1234", 4);
    swap = (keyLocal != endianKey);

    object_get(obj, "nx", &nXFile, INT, 1, "0");
    object_get(obj, "ny", &nYFile, INT, 1, "0");
    object_get(obj, "nz", &nZFile, INT, 1, "0");

    nSpecies = object_getv(obj, "species", (void**) &speciesNames, STRING);

    //
    // Determine the variables to plot.
    //
    nVars = 0;
    int           offset = 0;
    
    labelOffset = -1;
    iSpeciesOffset = -1;
    varOffsets = new int[nFields];
    varSizes   = new int[nFields];
    varFloat   = new bool[nFields];
    for (i = 0; i < nFields; i++)
    {
        if (strcmp(fieldNames[i], "checksum") != 0)
        {
            if (strcmp(fieldNames[i], "label") == 0)
            {
                 labelOffset = offset;
            }
            else if (strcmp(fieldNames[i], "species_index") == 0)
            {
                 iSpeciesOffset = offset;
            }
            else if (strcmp(fieldNames[i], "number_particles") == 0)
            {
                 varNames.push_back("nParticles");
                 varOffsets[nVars] = offset;
                 varSizes[nVars]   = fieldSizes[i];
                 varFloat[nVars]   = fieldTypes[i][0] == 'f' ? true : false;
                 nVars++;
            }
            else
            {
                 varNames.push_back(fieldNames[i]);
                 varOffsets[nVars] = offset;
                 varSizes[nVars]   = fieldSizes[i];
                 varFloat[nVars]   = fieldTypes[i][0] == 'f' ? true : false;
                 nVars++;
            }
        }
        offset += fieldSizes[i];
    }
    if (labelOffset == -1 || iSpeciesOffset == -1)
    {
        EXCEPTION1(InvalidFilesException, fname.c_str());
    }
    for (i = 0 ; i < nVars; i++)
    {
        debug1 << "varName=" << varNames[i] << ",offset=" << varOffsets[i]
               << ",size=" << varSizes[i] << ",float=" << varFloat[i] << endl;
    }

    //
    // Determine the mesh information.
    //
    coordsUnit = "nm";
    float xSize = hMatrix[0] / 10.0; // now in nm
    float ySize = hMatrix[4] / 10.0; // now in nm
    float zSize = hMatrix[8] / 10.0; // now in nm
    if (max(xSize, max(ySize, zSize)) > 1e3)
    {
        xSize /= 1e3;
        ySize /= 1e3;
        zSize /= 1e3;
        coordsUnit = "um";
    }

    //
    // If nX is 1, then we have a 2d mesh.  Map y to x and z to y.
    //
    if (nXFile == 1)
    {
        nDims = 2;
        xMin = -(ySize / 2.0);
        dX   =   ySize / (1.0 * nYFile);
        yMin = -(zSize / 2.0);
        dY   =   zSize / (1.0 * nZFile);
        zMin = -(xSize / 2.0);
        dZ   =   xSize / (1.0 * nXFile);
        nXMesh = nYFile;
        nYMesh = nZFile;
        nZMesh = 1;
    }
    else
    {
        nDims = 3;
        xMin = -(xSize / 2.0);
        dX   =   xSize / (1.0 * nXFile);
        yMin = -(ySize / 2.0);
        dY   =   ySize / (1.0 * nYFile);
        zMin = -(zSize / 2.0);
        dZ   =   zSize / (1.0 * nZFile);
        nXMesh = nXFile;
        nYMesh = nYFile;
        nZMesh = nZFile;
    }

    //
    // Print out the contents of the header to the debug logs.
    //
    debug1 << "nRecord=" << nRecord << ",nFields=" << nFields << ",lRec=" << lRec << endl;
    debug1 << "swap=" << swap << ",loop=" << loop << ",time=" << time << endl;
    debug1 << "fieldSizes=";
    for (int i = 0; i < nFields; i++)
    {
        debug1 << fieldSizes[i];
        if (i < nFields - 1) debug1 << ",";
    }
    debug1 << endl;
    debug1 << "fieldNames=";
    for (int i = 0; i < nFields; i++)
    {
        debug1 << fieldNames[i];
        if (i < nFields - 1) debug1 << ",";
    }
    debug1 << endl;
    debug1 << "fieldTypes=";
    for (int i = 0; i < nFields; i++)
    {
        debug1 << fieldTypes[i];
        if (i < nFields - 1) debug1 << ",";
    }
    debug1 << endl;
    debug1 << "nXFile=" << nXFile << ",nYFile=" << nYFile << ",nZFile=" << nZFile << endl;
    debug1 << "nSpecies=" << nSpecies << endl;
    for (int i = 0; i < nSpecies; i++)
    {
        debug1 << speciesNames[i];
        if (i < nSpecies - 1) debug1 << ",";
    }
    debug1 << endl;
    debug1 << "coordsUnit=" << coordsUnit << endl;
    debug1 << "xMin=" << xMin << ",yMin=" << yMin << ",zMin=" << zMin << endl;
    debug1 << "dX=" << dX << ",dY=" << dY << ",dZ=" << dZ << endl;

    free(obj);
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat constructor
//
//  Programmer: brugger -- generated by xml2avt
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
// ****************************************************************************

avtDDCMDFileFormat::avtDDCMDFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    string name(filename);
    fname = name.substr(0,name.length()-6);

    dataRead = false;
    varsBlock =  0;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat destructor
//
//  Programmer: brugger
//  Creation:   Thu Jan  3 11:11:39 PST 2008
//
// ****************************************************************************

avtDDCMDFileFormat::~avtDDCMDFileFormat()
{
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: brugger -- generated by xml2avt
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
//  Modifications:
//
//    Eric Brugger, Thu Jan  3 11:11:39 PST 2008
//    I modified the routine to set readData to false so that the data
//    would be recreated if necessary by ActivateTimestep.
//
// ****************************************************************************

void
avtDDCMDFileFormat::FreeUpResources(void)
{
    //
    // Only free the block variables in the engine. The pointer will
    // be 0 on the mdserver.
    //
    if (varsBlock != 0)
    {
        for (int i = 0; i < nVars; i++)
        {
            delete [] varsBlock[i];
        }
        delete [] varsBlock;
        varsBlock = 0;
    }

    //
    // Indicate that the data needs to read again.
    //
    dataRead = false;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::GetCycle
//
//  Purpose:
//      Return the current cycle.
//
//  Programmer: brugger
//  Creation:   Wed Sep  5 11:40:55 PDT 2007
//
// ****************************************************************************

int
avtDDCMDFileFormat::GetCycle(void)
{
    return loop;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::GetTime
//
//  Purpose:
//      Return the current time.
//
//  Programmer: brugger
//  Creation:   Wed Sep  5 11:40:55 PDT 2007
//
// ****************************************************************************

double
avtDDCMDFileFormat::GetTime(void)
{
    return time;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::ActivateTimestep
//
//  Purpose: Provides a guarenteed collective entry point for operations
//    that may involve collective parallel communication.
//
//  Programmer: brugger
//  Creation:   Wed Sep  5 11:40:55 PDT 2007
//
// ****************************************************************************

void
avtDDCMDFileFormat::ActivateTimestep(void)
{
    //
    // Return if the data has been read.
    //
    if (dataRead)
        return;

    //
    // Read the header.
    //
    ReadHeader(fname.c_str());

    //
    // Read the data.
    //
    ReadData();

    dataRead = true;
}

// ****************************************************************************
//  Method: avtDDCMDFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: brugger -- generated by xml2avt
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
// ****************************************************************************

void
avtDDCMDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    //
    // Set the mesh information.
    //
    string meshname = "quadmesh";

    avtMeshType mt = AVT_RECTILINEAR_MESH;

    int nblocks = 1;
    int block_origin = 0;
    int spatial_dimension = nDims;
    int topological_dimension = nDims;
    double *extents = NULL;

    AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                      spatial_dimension, topological_dimension);
    md->SetFormatCanDoDomainDecomposition(true);

    //
    // Set the particle variable information (One variable per species
    // and one expression).
    //
    avtCentering cent = AVT_ZONECENT;

    for (int j = 0; j < nSpecies; j++)
    {
        string name = "nParticles_" + string(speciesNames[j]);
        AddScalarVarToMetaData(md, name, meshname, cent);
    }

    Expression expr;
    expr.SetName("nParticles");
    string name = "nParticles_" + string(speciesNames[0]);
    for (int j = 1; j < nSpecies; j++)
    {
        name += " + nParticles_" + string(speciesNames[j]);
    }
    expr.SetDefinition(name);
    expr.SetType(Expression::ScalarMeshVar);
    md->AddExpression(&expr);

    for (int j = 0; j < nSpecies; j++)
    {
        string name = "frac_" + string(speciesNames[j]);
        expr.SetName(name);
        name = "nParticles_" + string(speciesNames[j]) + "/nParticles";
        expr.SetDefinition(name);
        expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&expr);

        name = "mix_" + string(speciesNames[j]);
        expr.SetName(name);
        string frac_name = "frac_" + string(speciesNames[j]);
        name = frac_name + "-" + frac_name + "*" + frac_name;
        expr.SetDefinition(name);
        expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&expr);
    }

    //
    // Set the normal variable information (One variable per species
    // per variable and one expression per variable).
    //
    vector<string> vars;
    vars.push_back("px");
    vars.push_back("py");
    vars.push_back("pz");
    vars.push_back("mass");
    vars.push_back("virial");
    vars.push_back("Kx");
    vars.push_back("Ky");
    vars.push_back("Kz");
    vars.push_back("U");
    for (int i = 0; i < vars.size(); i++)
    {
        for (int j = 0; j < nSpecies; j++)
        {
            string name = vars[i] + "_" + string(speciesNames[j]);
            AddScalarVarToMetaData(md, name, meshname, cent);
        }

        expr.SetName(vars[i]);
        string name = vars[i] + "_" + string(speciesNames[0]);
        for (int j = 1; j < nSpecies; j++)
        {
            name += " + " + vars[i] + "_" + string(speciesNames[j]);
        }
        expr.SetDefinition(name);
        expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&expr);
    }

    //
    // Set the miscellaneous expressions.
    //
    if (nDims == 2)
    {
        expr.SetName("p");
        expr.SetDefinition("{py, pz}");
        expr.SetType(Expression::VectorMeshVar);
        md->AddExpression(&expr);
        expr.SetName("v");
        expr.SetDefinition("{vy,vz}");
        expr.SetType(Expression::VectorMeshVar);
        md->AddExpression(&expr);
    }
    else
    {
        expr.SetName("p");
        expr.SetDefinition("{px, py, pz}");
        expr.SetType(Expression::VectorMeshVar);
        md->AddExpression(&expr);
        expr.SetName("v");
        expr.SetDefinition("{vx, vy,vz}");
        expr.SetType(Expression::VectorMeshVar);
        md->AddExpression(&expr);
        expr.SetName("vx");
        expr.SetDefinition("px/max(mass,1.0)");
        expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&expr);
    }
    expr.SetName("vy");
    expr.SetDefinition("py/max(mass,1.0)");
    expr.SetType(Expression::ScalarMeshVar);
    md->AddExpression(&expr);
    expr.SetName("vz");
    expr.SetDefinition("pz/max(mass,1.0)");
    expr.SetType(Expression::ScalarMeshVar);
    md->AddExpression(&expr);
    expr.SetName("K");
    expr.SetDefinition("Kx+Ky+Kz");
    expr.SetType(Expression::ScalarMeshVar);
    md->AddExpression(&expr);
    expr.SetName("Etotal");
    expr.SetDefinition("K+U");
    expr.SetType(Expression::ScalarMeshVar);
    md->AddExpression(&expr);
    expr.SetName("density");
    expr.SetDefinition("mass/area(quadmesh)");
    expr.SetType(Expression::ScalarMeshVar);
    md->AddExpression(&expr);
    expr.SetName("Temperature");
    expr.SetDefinition("(2.0/3.0)*(11605/(nParticles-1)) * "
        "(K - ( (103.645*magnitude(p)*magnitude(p)) / (2*mass) ) )");
    expr.SetType(Expression::ScalarMeshVar);
    md->AddExpression(&expr);
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: brugger -- generated by xml2avt
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
// ****************************************************************************

vtkDataSet *
avtDDCMDFileFormat::GetMesh(int domain, const char *meshname)
{
    int dims[3] = {1, 1, 1};
    vtkFloatArray *coords[3] = {0, 0, 0};

    //
    // Determine the block indices based on the domain number.
    //
    int deltaX = (nXFile + nXFileBlocks - 1) / nXFileBlocks;
    int deltaY = (nYFile + nYFileBlocks - 1) / nYFileBlocks;
    int deltaZ = (nZFile + nZFileBlocks - 1) / nZFileBlocks;

#ifdef PARALLEL
    int iBlock = PAR_Rank();
#else
    int iBlock = 0;
#endif
    int iXBlock = iBlock / (nYFileBlocks * nZFileBlocks);
    iBlock %= (nYFileBlocks * nZFileBlocks);
    int iYBlock = iBlock / nZFileBlocks;
    int iZBlock = iBlock % nZFileBlocks;

    int iXMin, iXMax, iYMin, iYMax, iZMin, iZMax;
    if (nDims == 2)
    {
        iXMin = iYBlock * deltaY;
        iXMax = iXMin + deltaY < nYFile ? iXMin + deltaY : nYFile;

        iYMin = iZBlock * deltaZ;
        iYMax = iYMin + deltaZ < nZFile ? iYMin + deltaZ : nZFile;

        iZMin = iXBlock * deltaX;
        iZMax = iZMin + deltaX < nXFile ? iZMin + deltaX : nXFile;
    }
    else
    {
        iXMin = iXBlock * deltaX;
        iXMax = iXMin + deltaX < nXFile ? iXMin + deltaX : nXFile;

        iYMin = iYBlock * deltaY;
        iYMax = iYMin + deltaY < nYFile ? iYMin + deltaY : nYFile;

        iZMin = iZBlock * deltaZ;
        iZMax = iZMin + deltaZ < nZFile ? iZMin + deltaZ : nZFile;
    }

    debug1 << "iXMin=" << iXMin << ",iXMax=" << iXMax
           << ",iYMin=" << iYMin << ",iYMax=" << iYMax
           << ",iZMin=" << iZMin << ",iZMax=" << iZMax << endl;

    //
    // Set the dimension information for the mesh.
    //
    dims[0] = iXMax - iXMin + 1;
    dims[1] = iYMax - iYMin + 1;
    dims[2] = iZMax - iZMin + 1;

    //
    // Set the coordinate arrays.
    //
    coords[0] = vtkFloatArray::New();
    coords[0]->SetNumberOfTuples(dims[0]);
    float *xarray = (float *)coords[0]->GetVoidPointer(0);
    for (int i = iXMin; i < iXMax + 1; i++)
    {
        xarray[i-iXMin] = i * dX + xMin;
    } 

    coords[1] = vtkFloatArray::New();
    coords[1]->SetNumberOfTuples(dims[1]);
    float *yarray = (float *)coords[1]->GetVoidPointer(0);
    for (int i = iYMin; i < iYMax + 1; i++)
    {
        yarray[i-iYMin] = i * dY + yMin;
    } 

    if (nDims == 2)
    {
        coords[2] = vtkFloatArray::New();
        coords[2]->SetNumberOfTuples(1);
        coords[2]->SetComponent(0, 0, 0.);
    }
    else
    {
        coords[2] = vtkFloatArray::New();
        coords[2]->SetNumberOfTuples(dims[2]);
        float *zarray = (float *)coords[2]->GetVoidPointer(0);
        for (int i = iZMin; i < iZMax + 1; i++)
        {
            zarray[i-iZMin] = i * dZ + zMin;
        } 
    }
     
    //
    // Create the vtkRectilinearGrid object and set its dimensions
    // and coordinates.
    //
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(dims);
    rgrid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    rgrid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    rgrid->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    return rgrid;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: brugger -- generated by xml2avt
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
// ****************************************************************************

vtkDataArray *
avtDDCMDFileFormat::GetVar(int domain, const char *varname)
{
    if (domain != 0)
        EXCEPTION1(InvalidVariableException, varname);

    vtkFloatArray *rv = 0;

    string var = varname;

    //
    // Determine the variable name and the species name.
    //
    int i;
    for (i = 0; i < var.length() && var[i] != '_'; i++)
        /* do nothing */;
    string varBase = var.substr(0, i);
    string varSpec = var.substr(i+1, var.length() - i - 1);
    
    //
    // Get the index of the variable name and the index of the species name.
    //
    int iVar;

    for (iVar = 0; iVar < varNames.size() && varNames[iVar] != varBase; iVar++)
        /* Do nothing. */;
    if (iVar == varNames.size())
        EXCEPTION1(InvalidVariableException, varname);

    int iSpec;

    for (iSpec = 0; iSpec < nSpecies && strcmp(speciesNames[iSpec], varSpec.c_str()); iSpec++)
        /* Do nothing. */;
    if (iSpec == nSpecies)
        EXCEPTION1(InvalidVariableException, varname);

    //
    // Copy the data array.
    //
    rv = vtkFloatArray::New();

    int ntuples = nZonesBlock;
    rv->SetNumberOfTuples(ntuples);

    float *data = varsBlock[iVar] + iSpec * ntuples;
    i = 0;
    for (int iZ = 0; iZ < nZBlock; iZ++)
    {
        for (int iY = 0; iY < nYBlock; iY++)
        {
            for (int iX = 0; iX < nXBlock; iX++)
            {
                rv->SetTuple1(i, *(data++));
                i++;
            }
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: brugger -- generated by xml2avt
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
// ****************************************************************************

vtkDataArray *
avtDDCMDFileFormat::GetVectorVar(int domain, const char *varname)
{
    //
    // We do not have any vector variables.
    //
    EXCEPTION1(InvalidVariableException, varname);

    return 0;
}
