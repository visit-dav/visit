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

#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
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

#if defined(_MSC_VER) || !defined(HAVE_STRTOF) || !defined(HAVE_STRTOF_PROTOTYPE)
#ifndef strtof
#define strtof(f1,f2) ((float)strtod(f1,f2))
#endif
#endif

using     std::string;
using     std::vector;
using     std::min;
using     std::max;

// ****************************************************************************
//  Method: DDCMDHeader constructor
//
//  Purpose:
//      Read the header from the specified file.
//
//  Arguments:
//      fname      The name of the root file.
//      subname    The name of the sub file.
//
//  Programmer: brugger
//  Creation:   Thu Nov 20 10:44:45 PST 2008
//
//  Modifications:
//    Eric Brugger, Fri Dec  5 16:39:53 PST 2008
//    I enhanced the reader to read ascii atom files.
//
// ****************************************************************************

DDCMDHeader::DDCMDHeader(const char *fname, const char *subname)
{
    int           i, headerlength;
    char          string[4096], *header, *h;

    //
    // Open the file and read the header. It is a bunch of lines terminated
    // with a null character.
    //
    int           maxlen;
    FILE         *file;

    sprintf(string, "%s/%s#000000", fname, subname);

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

    object_get(obj, (char*)"datatype", &dataType, STRING, 1, (char*)"NONE");
    object_get(obj, (char*)"lrec", &lRec, INT, 1, (char*)"16");
    object_get(obj, (char*)"endian_key", &endianKey, INT, 1, (char*)"4");
    object_get(obj, (char*)"nrecord", &nRecord, INT, 1, (char*)"1");
    object_get(obj, (char*)"nfiles", &nFiles, INT, 1, (char*)"1");
    object_get(obj, (char*)"nfields", &nFields, INT, 1, (char*)"1");
    object_getv(obj, (char*)"field_names", (void **)&fieldNames, STRING);
    object_getv(obj, (char*)"field_types", (void **)&fieldTypes, STRING);
    fieldSizes = (unsigned*) malloc(nFields*sizeof(int));
    if (strcmp(dataType, "FIXRECORDBINARY") == 0)
    {
        for (i = 0; i < nFields; i++)
        {
            fieldSizes[i] = atoi(fieldTypes[i] + 1);
        }
    }
    else
    {
        for (i = 0; i < nFields; i++)
        {
            fieldSizes[i] = 1;
        }
    }

    object_get(obj, (char*)"h", &hMatrix, DOUBLE, 9, (char*)"1000.0");
    object_get(obj, (char*)"time", &time, DOUBLE, 1, (char*)"0.0");
    object_get(obj, (char*)"loop", &loop, INT, 1, (char*)"0");

    memcpy(&keyLocal, "1234", 4);
    swap = (keyLocal != endianKey);

    nSpecies = object_getv1(obj, (char*)"species", (void**)&speciesNames,
                            STRING);

    //
    // Cgrid specific information.
    //
    object_get(obj, (char*)"nx", &nXFile, INT, 1, (char*)"0");
    object_get(obj, (char*)"ny", &nYFile, INT, 1, (char*)"0");
    object_get(obj, (char*)"nz", &nZFile, INT, 1, (char*)"0");

    //
    // Atom specific information.
    //
    nGroups = object_getv1(obj, (char*)"groups", (void**)&groupNames, STRING);
    nTypes = object_getv1(obj, (char*)"types", (void**)&typeNames, STRING);

    free(obj);

    //
    // Determine whether we have a cgrid or an atom file.
    //
    cgridFile = !(nXFile == 0 && nYFile == 0 && nZFile == 0) ? true : false;

    //
    // Print out the contents of the header to the debug logs.
    //
    debug1 << "dataType=" << dataType << ",nRecord=" << nRecord
           << ",nFields=" << nFields << ",lRec=" << lRec << endl;
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
    debug1 << "nXFile=" << nXFile << ",nYFile=" << nYFile
           << ",nZFile=" << nZFile << endl;
    debug1 << "nSpecies=" << nSpecies << endl;
    for (int i = 0; i < nSpecies; i++)
    {
        debug1 << speciesNames[i];
        if (i < nSpecies - 1) debug1 << ",";
    }
    debug1 << endl;
}

// ****************************************************************************
//  Method: DDCMDHeader destructor
//
//  Purpose:
//      Free up the memory associated with the header.
//
//  Programmer: brugger
//  Creation:   Thu Nov 20 10:44:45 PST 2008
//
// ****************************************************************************

DDCMDHeader::~DDCMDHeader()
{
    //
    // We are using free, since all the memory was allocated with malloc.
    //
    for (int i = 0; i < nFields; i++)
    {
        free(fieldNames[i]);
        free(fieldTypes[i]);
    }
    free(fieldNames);
    free(fieldTypes);
    free(fieldSizes);
    for (int i = 0; i < nSpecies; i++)
        free(speciesNames[i]);
    free(speciesNames);
    for (int i = 0; i < nGroups; i++)
        free(groupNames[i]);
    free(groupNames);
    for (int i = 0; i < nTypes; i++)
        free(typeNames[i]);
    free(typeNames);
};


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
//  Method: avtDDCMDFileFormat::bFieldConvert
//
//  Purpose:
//      Convert a 'b' field type into an unsigned long long.
//
//  Programmer: Eric Brugger
//  Creation:   Tue Jul 15 15:21:50 PDT 2008
//
// ****************************************************************************

long long
avtDDCMDFileFormat::bFieldConvert(const unsigned char *ptr, int size)
{
    long long result = 0;

    for (int i = 0; i < size; i++)
    {
        result *= 256;
        result += *(ptr+i);
    }

    return result;
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
//  Arguments:
//      header      The header data for the current file.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
//  Modifications:
//    Eric Brugger, Tue Jul 15 15:21:50 PDT 2008
//    I added support for the 'b' field type.
//
//    Eric Brugger, Thu Nov 20 10:44:45 PST 2008
//    I added the ability to read atom files, which required being able to
//    read multiple files to get all the data.
//
// ****************************************************************************

void
avtDDCMDFileFormat::ExchangeProcessorData(const DDCMDHeader *header)
{
    unsigned int lRec = header->GetLRec();
    unsigned int swap = header->GetSwap();

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

    if (labelUnsigned)
    {
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
    }
    else
    {
        for (int i = 0; i < nRecords; i++)
        {
            //
            // Byte swap the data if necessary.
            //
            long long id = bFieldConvert((unsigned char *)data+labelOffset, labelSize);

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

    if (labelUnsigned)
    {
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
    }
    else
    {
        for (int i = 0; i < nRecords; i++)
        {
            //
            // Byte swap the data if necessary.
            //
            long long id = bFieldConvert((unsigned char *)data+labelOffset, labelSize);

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
//  Arguments:
//      header      The header data for the current file.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
//  Modifications:
//    Eric Brugger, Tue Jul 15 15:21:50 PDT 2008
//    I added support for the 'b' field type.
//
//    Eric Brugger, Thu Nov 20 10:44:45 PST 2008
//    I added the ability to read atom files, which required being able to
//    read multiple files to get all the data.
//
// ****************************************************************************

void
avtDDCMDFileFormat::CopyExchangeDataToBlocks(const DDCMDHeader *header)
{
#ifdef PARALLEL
    int rank = PAR_Rank();
#else
    int rank = 0;
#endif

    unsigned int lRec     = header->GetLRec();
    unsigned int nXFile   = header->GetNXFile();
    unsigned int nYFile   = header->GetNYFile();
    unsigned int nZFile   = header->GetNZFile();
    unsigned int nSpecies = header->GetNSpecies();
    unsigned int nFields  = header->GetNFields();
    int          swap     = header->GetSwap();
    char         **fieldTypes = header->GetFieldTypes();
    unsigned int  *fieldSizes = header->GetFieldSizes();

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

    varValues = new float*[nVars];
    for (int i = 0; i < nVars; i++)
    {
        varValues[i] = new float[nZones*nSpecies];
        memset(varValues[i], 0, nZones*nSpecies*sizeof(float));
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
            if (swap && fieldTypes[j][0] != (char)'b') Convert(field, fieldSizes[j]);
            field += fieldSizes[j];
        }

        //
        // Copy the record into the correct zone in the correct block.
        //
        long long id, iSpecies;
        if (labelUnsigned)
            id = *((unsigned *) (data+labelOffset));
        else
            id = bFieldConvert((unsigned char *)data+labelOffset, labelSize);
        if (iSpeciesUnsigned)
            iSpecies = *((unsigned *) (data+iSpeciesOffset));
        else
            iSpecies = bFieldConvert((unsigned char *)data+iSpeciesOffset, iSpeciesSize);

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

        float **vars = varValues;
        for (int j = 0; j < nVars; j++)
        {
            float *fvar = vars[j];
            switch (varTypes[j])
            {
              case 'f':
                fvar[iSpecies*nZones+iZone] +=
                    *((float *) (data+varOffsets[j]));
                break;
              case 'u':
                fvar[iSpecies*nZones+iZone] +=
                    float(*((unsigned *) (data+varOffsets[j])));
                break;
              case 'b':
                fvar[iSpecies*nZones+iZone] +=
                    float(bFieldConvert((unsigned char *)data+varOffsets[j],
                                        varSizes[j]));
                break;
            }
        }
        data += lRec;
    }

    delete [] varOffsets;
    varOffsets = 0;
    delete [] varSizes;
    varSizes = 0;
    delete [] varTypes;
    varTypes = 0;
    delete [] inProcessorData;
    inProcessorData = 0;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::CopyAsciiDataToBlocks
//
//  Purpose:
//      Copy the ascii data read from the files into contiguous blocks.
//
//  Arguments:
//      header      The header data for the current file.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Dec  5 16:39:53 PST 2008
//
// ****************************************************************************

void
avtDDCMDFileFormat::CopyAsciiDataToBlocks(const DDCMDHeader *header)
{
#ifdef PARALLEL
    int rank = PAR_Rank();
#else
    int rank = 0;
#endif

    unsigned int   lRec       = header->GetLRec();
    unsigned int   nFields    = header->GetNFields();
    int            swap       = header->GetSwap();
    char         **fieldTypes = header->GetFieldTypes();
    unsigned int  *fieldSizes = header->GetFieldSizes();

    nPoints = nRecordsList[rank];

    char *data = readProcessorData;

    int nSpecies = header->GetNSpecies();
    int nGroups  = header->GetNGroups();
    int nTypes   = header->GetNTypes();
    char **speciesNames = header->GetSpeciesNames();
    char **groupNames   = header->GetGroupNames();
    char **typeNames    = header->GetTypeNames();

    char **recOffsets = new char*[nFields];
    for (int i = 0; i < nPoints; i++)
    {
        //
        // Determine the start of each field.
        //
        char *rec = data;
        for (int j = 0; j < nFields; j++)
        {
             while (isspace(*rec))
                 rec++;
             recOffsets[j] = rec;
             while (!isspace(*rec))
                 rec++;
             rec[0] = '\0';
             rec++;
        }

        //
        // Copy the pinfo and coordinate data.
        //
        if (groupOffset != -1)
        {
            int iGroup = 0;
            while (iGroup < nGroups &&
                   strcmp(recOffsets[groupOffset], groupNames[iGroup]))
                iGroup++;
            int iSpecies = 0;
            while (iSpecies < nSpecies &&
                   strcmp(recOffsets[speciesOffset], speciesNames[iSpecies]))
                iSpecies++;
            int iType = 0;
            while (iType < nTypes &&
                   strcmp(recOffsets[typeOffset], typeNames[iType]))
                iType++;
            pinfoBlock[i] = iGroup * nSpecies * nTypes +
                            iSpecies * nTypes + iType;

            coordsBlock[i*3]   = strtof(recOffsets[xOffset], NULL) / coordsScale;
            coordsBlock[i*3+1] = strtof(recOffsets[yOffset], NULL) / coordsScale;
            coordsBlock[i*3+2] = strtof(recOffsets[zOffset], NULL) / coordsScale;
        }

        //
        // Copy the variable information.
        //
        float **vars = varValues;
        for (int j = 0; j < nVars; j++)
        {
            float *fvar = vars[j];
            switch (varTypes[j])
            {
              case 'f':
                fvar[i] = strtof(recOffsets[varOffsets[j]], NULL);
                break;
              case 'u':
                fvar[i] = float(strtol(recOffsets[varOffsets[j]], NULL, 10));
                break;
            }
        }

        data += lRec;
    }

    delete [] recOffsets;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::CopyBinaryDataToBlocks
//
//  Purpose:
//      Copy the binary data read from the files into contiguous blocks.
//
//  Arguments:
//      header      The header data for the current file.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Dec  5 16:39:53 PST 2008
//
// ****************************************************************************

void
avtDDCMDFileFormat::CopyBinaryDataToBlocks(const DDCMDHeader *header)
{
#ifdef PARALLEL
    int rank = PAR_Rank();
#else
    int rank = 0;
#endif

    unsigned int   lRec       = header->GetLRec();
    unsigned int   nFields    = header->GetNFields();
    int            swap       = header->GetSwap();
    char         **fieldTypes = header->GetFieldTypes();
    unsigned int  *fieldSizes = header->GetFieldSizes();

    nPoints = nRecordsList[rank];

    char *data = readProcessorData;

    for (int i = 0; i < nPoints; i++)
    {
        //
        // Byte swap the data if necessary.
        //
        char *field = data;
        for (int j = 0; j < nFields; j++)
        {
            if (swap && fieldTypes[j][0] != (char)'b') Convert(field, fieldSizes[j]);
            field += fieldSizes[j];
        }

        //
        // Copy the pinfo and coordinate information.
        //
        if (pinfoOffset != -1)
        {
            switch (pinfoType)
            {
              case 'f':
                pinfoBlock[i] = unsigned(*((float *) (data+pinfoOffset)));
                break;
              case 'u':
                pinfoBlock[i] = *((unsigned *) (data+pinfoOffset));
                break;
              case 'b':
                pinfoBlock[i] =
                    unsigned(bFieldConvert((unsigned char *)data+pinfoOffset,
                                            pinfoSize));
                break;
            }

            coordsBlock[i*3]   = *((float *) (data + xOffset)) / coordsScale;
            coordsBlock[i*3+1] = *((float *) (data + yOffset)) / coordsScale;
            coordsBlock[i*3+2] = *((float *) (data + zOffset)) / coordsScale;
        }

        //
        // Copy the variable information.
        //
        float **vars = varValues;
        for (int j = 0; j < nVars; j++)
        {
            float *fvar = vars[j];
            switch (varTypes[j])
            {
              case 'f':
                fvar[i] = *((float *) (data+varOffsets[j]));
                break;
              case 'u':
                fvar[i] = float(*((unsigned *) (data+varOffsets[j])));
                break;
              case 'b':
                fvar[i] =
                    float(bFieldConvert((unsigned char *)data+varOffsets[j],
                                        varSizes[j]));
                break;
            }
        }

        data += lRec;
    }
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::CopyDataToBlocks
//
//  Purpose:
//      Copy the data read from the files into contiguous blocks.
//
//  Arguments:
//      header      The header data for the current file.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Nov 20 10:44:45 PST 2008
//
//  Modifications:
//    Eric Brugger, Fri Dec  5 16:39:53 PST 2008
//    I enhanced the reader to read ascii atom files.
//
// ****************************************************************************

void
avtDDCMDFileFormat::CopyDataToBlocks(const DDCMDHeader *header)
{
#ifdef PARALLEL
    int rank = PAR_Rank();
#else
    int rank = 0;
#endif

    char          *dataType   = header->GetDataType();

    nPoints = nRecordsList[rank];

    //
    // Check that the data type is valid.
    //
    if (strcmp(dataType, "FIXRECORDBINARY") != 0 &&
        strcmp(dataType, "FIXRECORDASCII") != 0)
    {
        debug1 << "Invalid data type: dataType=" << dataType << endl;
    }

    //
    // Initialize the storage for the block data.
    //
    if (pinfoOffset != -1 || groupOffset != -1)
    {
        coordsBlock = new float[nPoints*3];
        pinfoBlock = new unsigned[nPoints];
    }
    varValues = new float*[nVars];
    for (int i = 0; i < nVars; i++)
    {
        varValues[i] = new float[nPoints];
    }

    if (strcmp(dataType, "FIXRECORDASCII") == 0)
        CopyAsciiDataToBlocks(header);
    else if (strcmp(dataType, "FIXRECORDBINARY") == 0)
        CopyBinaryDataToBlocks(header);
        
    //
    // Free memory.
    //
    delete [] varOffsets;
    varOffsets = 0;
    delete [] varSizes;
    varSizes = 0;
    delete [] varTypes;
    varTypes = 0;
    delete [] readProcessorData;
    readProcessorData = 0;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::ReadProcessorChunk
//
//  Purpose:
//      Read this processors chunk of the data.  This is everything but
//      the header.
//
//  Arguments:
//      header      The header data for the current file.
//      subname     The name of the sub file.
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
//    Eric Brugger, Thu Nov 20 10:44:45 PST 2008
//    I added the ability to read atom files, which required being able to
//    read multiple files to get all the data.
//
//    Eric Brugger, Fri Jan 23 13:24:17 PST 2009
//    I corrected a bug where the reader only read the first file of a
//    multi-file file.  The problem was that it was picking up the value
//    of nFiles from the base class, which was 1, instead of from the
//    header. I changed the code to set nFiles from the header.
//
// ****************************************************************************

void
avtDDCMDFileFormat::ReadProcessorChunk(const DDCMDHeader *header,
    const char *subname)
{
#ifdef PARALLEL
    int rank = PAR_Rank();
#else
    int rank = 0;
#endif

    unsigned int lRec   = header->GetLRec();
    unsigned int nFiles = header->GetNFiles();

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
    sprintf(string, "%s/%s#%6.6d", fname.c_str(), subname, fileNumber);
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
                sprintf(string, "%s/%s#%6.6d", fname.c_str(), subname,
                        fileNumber);
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
//  Arguments:
//      header      The header data for the current file.
//      subname     The name of the sub file.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
//  Modifications:
//    Eric Brugger, Thu Nov 20 10:44:45 PST 2008
//    I added the ability to read atom files, which required being able to
//    read multiple files to get all the data.
//
//    Eric Brugger, Fri Jan 23 13:24:17 PST 2009
//    I corrected a bug where the reader only read the first file of a
//    multi-file file.  The problem was that it was picking up the value
//    of nFiles from the base class, which was 1, instead of from the
//    header. I changed the code to set nFiles from the header.
//
// ****************************************************************************

void
avtDDCMDFileFormat::DetermineProcessorReadOffset(const DDCMDHeader *header,
    const char *subname)
{
#ifdef PARALLEL
    int nProcs = PAR_Size();
#else
    int nProcs = 1;
#endif

    unsigned int headerLength = header->GetHeaderLength();
    unsigned int lRec         = header->GetLRec();
    bool         cgridFile    = header->GetCGridFile();
    unsigned int nFiles       = header->GetNFiles();

    //
    // Determine the size of each file.
    //
    off_t *fileSizeList = new off_t[nFiles];
    off_t fileSize = 0;
    for (int i = 0; i < nFiles; i++)
    {
        char          string[1024];
        VisItStat_t   statbuf;

        sprintf(string, "%s/%s#%6.6d", fname.c_str(), subname, i);

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
//  Method: avtDDCMDFileFormat::ReadHeader
//
//  Purpose:
//      Return a list of headers associated with the list of files.  The
//      headers are checked for consistency.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
//  Modifications:
//    Hank Childs, Fri Feb 15 16:00:20 PST 2008
//    Fix memory leak.
//
//    Eric Brugger, Tue Jul 15 15:21:50 PDT 2008
//    I added support for the 'b' field type.
//
//    Jeremy Meredith, Thu Aug  7 15:48:21 EDT 2008
//    Assume that object_get won't modify its input char* argument, and
//    cast our string literals appropriately.
//
//    Eric Brugger, Thu Nov 20 10:44:45 PST 2008
//    I added the ability to read atom files, which required being able to
//    read multiple files to get all the data.
//
// ****************************************************************************

vector<DDCMDHeader*>
avtDDCMDFileFormat::ReadHeader()
{
    //
    // Read the headers.
    //
    vector<DDCMDHeader*> headers;
    for (int i = 0; i < subnames.size(); i++)
        headers.push_back(new DDCMDHeader(fname.c_str(), subnames[i].c_str()));

    //
    // Determine if it is a cgrid file.
    //
    cgridFile = false;
    for (int i = 0; i < subnames.size(); i++)
        cgridFile = cgridFile || headers[i]->GetCGridFile();

    //
    // Check that the files are cnonsistent.
    //
    bool error = false;

    if (cgridFile && subnames.size() > 1)
        error = true;

    for (int i = 1; i < subnames.size(); i++)
    {
        error = (headers[0]->GetNRecord() != headers[i]->GetNRecord()) ?
            true : error;
        error = (headers[0]->GetSwap() != headers[i]->GetSwap()) ?
            true : error;
        error = (headers[0]->GetHMatrix()[0] != headers[i]->GetHMatrix()[0]) ?
            true : error;
        error = (headers[0]->GetHMatrix()[4] != headers[i]->GetHMatrix()[4]) ?
            true : error;
        error = (headers[0]->GetHMatrix()[8] != headers[i]->GetHMatrix()[8]) ?
            true : error;
        error = (headers[0]->GetLoop() != headers[i]->GetLoop()) ?
            true : error;
        error = (headers[0]->GetTime() != headers[i]->GetTime()) ?
            true : error;
    }

    //
    // If we had an error, return no headers.
    //
    if (error)
    {
        for (int i = 0; i < headers.size(); i++)
            delete headers[i];
        headers.clear();
        return headers;
    }

    //
    // Store some global information from the headers.
    //
    loop = headers[0]->GetLoop();
    time = headers[0]->GetTime();
    nXFile = headers[0]->GetNXFile();
    nYFile = headers[0]->GetNYFile();
    nZFile = headers[0]->GetNZFile();

    //
    // Determine the species, group, type information. Only one file
    // should have it, so I take it from the first file that has it.
    // Assume that if other files have it, it will be consistent.
    //
    nSpecies = 0;
    for (int i = 0; i < headers.size(); i++)
    {
        if (headers[i]->GetNSpecies() > 0 && nSpecies == 0)
        {
            nSpecies = headers[i]->GetNSpecies();
            speciesNames = new char*[nSpecies];
            for (int j = 0; j < nSpecies; j++)
            {
                speciesNames[j] =
                    new char[strlen(headers[i]->GetSpeciesNames()[j])+1];
                strcpy(speciesNames[j], headers[i]->GetSpeciesNames()[j]);
            }
        }
    }
    nGroups = 0;
    for (int i = 0; i < headers.size(); i++)
    {
        if (headers[i]->GetNGroups() > 0 && nGroups == 0)
        {
            nGroups = headers[i]->GetNGroups();
            groupNames = new char*[nGroups];
            for (int j = 0; j < nGroups; j++)
            {
                groupNames[j] =
                    new char[strlen(headers[i]->GetGroupNames()[j])+1];
                strcpy(groupNames[j], headers[i]->GetGroupNames()[j]);
            }
        }
    }
    nTypes = 0;
    for (int i = 0; i < headers.size(); i++)
    {
        if (headers[i]->GetNTypes() > 0 && nTypes == 0)
        {
            nTypes = headers[i]->GetNTypes();
            typeNames = new char*[nTypes];
            for (int j = 0; j < nTypes; j++)
            {
                typeNames[j] =
                    new char[strlen(headers[i]->GetTypeNames()[j])+1];
                strcpy(typeNames[j], headers[i]->GetTypeNames()[j]);
            }
        }
    }

    return headers;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::ParseCGridHeader
//
//  Purpose:
//      Parse the header for a cgrid file.
//
//  Arguments:
//      header      The header data for the current file.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Nov 20 10:44:45 PST 2008
//
// ****************************************************************************

void
avtDDCMDFileFormat::ParseCGridHeader(const DDCMDHeader *header)
{
    unsigned int  nXFile  = header->GetNXFile();
    unsigned int  nYFile  = header->GetNYFile();
    unsigned int  nZFile  = header->GetNZFile();
    unsigned int  nFields = header->GetNFields();
    const double *hMatrix = header->GetHMatrix();
    char         **fieldNames = header->GetFieldNames();
    char         **fieldTypes = header->GetFieldTypes();
    unsigned int  *fieldSizes = header->GetFieldSizes();

    //
    // Determine the variables to plot.
    //
    int           i;
    int           offset = 0;
    
    labelOffset = -1;
    iSpeciesOffset = -1;
    varNames   = new string[nFields];
    varOffsets = new int[nFields];
    varSizes   = new int[nFields];
    varTypes   = new char[nFields];
    nVars = 0;
    for (i = 0; i < nFields; i++)
    {
        if (strcmp(fieldNames[i], "checksum") != 0)
        {
            if (strcmp(fieldNames[i], "label") == 0)
            {
                labelOffset   = offset;
                labelSize     = fieldSizes[i];
                labelUnsigned = fieldTypes[i][0] == 'u' ? true : false;
            }
            else if (strcmp(fieldNames[i], "species_index") == 0)
            {
                iSpeciesOffset   = offset;
                iSpeciesSize     = fieldSizes[i];
                iSpeciesUnsigned = fieldTypes[i][0] == 'u' ? true : false;
            }
            else
            {
                varNames[nVars]   = fieldNames[i];
                varOffsets[nVars] = offset;
                varSizes[nVars]   = fieldSizes[i];
                varTypes[nVars]   = fieldTypes[i][0];
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
               << ",size=" << varSizes[i] << ",type=" << varTypes[i] << endl;
    }

    //
    // Determine the mesh information.
    //
    coordsUnit = "nm";
    float xExtent = hMatrix[0] / 10.0; // now in nm
    float yExtent = hMatrix[4] / 10.0; // now in nm
    float zExtent = hMatrix[8] / 10.0; // now in nm
    if (max(xExtent, max(yExtent, zExtent)) > 1e3)
    {
        xExtent /= 1e3;
        yExtent /= 1e3;
        zExtent /= 1e3;
        coordsUnit = "um";
    }

    //
    // If nX is 1, then we have a 2d mesh.  Map y to x and z to y.
    //
    if (nXFile == 1)
    {
        nDims = 2;
        xMin = -(yExtent / 2.0);
        dX   =   yExtent / (1.0 * nYFile);
        yMin = -(zExtent / 2.0);
        dY   =   zExtent / (1.0 * nZFile);
        zMin = -(xExtent / 2.0);
        dZ   =   xExtent / (1.0 * nXFile);
        nXMesh = nYFile;
        nYMesh = nZFile;
        nZMesh = 1;
    }
    else
    {
        nDims = 3;
        xMin = -(xExtent / 2.0);
        dX   =   xExtent / (1.0 * nXFile);
        yMin = -(yExtent / 2.0);
        dY   =   yExtent / (1.0 * nYFile);
        zMin = -(zExtent / 2.0);
        dZ   =   zExtent / (1.0 * nZFile);
        nXMesh = nXFile;
        nYMesh = nYFile;
        nZMesh = nZFile;
    }
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::ParseAtomHeader
//
//  Purpose:
//      Parse the header for an atom file.
//
//  Arguments:
//      header      The header data for the current file.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Nov 20 10:44:45 PST 2008
//
//  Modifications:
//    Eric Brugger, Fri Dec  5 16:39:53 PST 2008
//    I enhanced the reader to read ascii atom files.
//
// ****************************************************************************

void
avtDDCMDFileFormat::ParseAtomHeader(const DDCMDHeader *header)
{
    unsigned int   nFields    = header->GetNFields();
    char         **fieldNames = header->GetFieldNames();
    char         **fieldTypes = header->GetFieldTypes();
    unsigned int  *fieldSizes = header->GetFieldSizes();

    //
    // Determine the variables to plot.
    //
    groupOffset   = -1;
    speciesOffset = -1;
    typeOffset    = -1;
    pinfoOffset   = -1;
    xOffset       = -1;
    yOffset       = -1;
    zOffset       = -1;
    varNames   = new string[nFields];
    varOffsets = new int[nFields];
    varSizes   = new int[nFields];
    varTypes   = new char[nFields];
    nVars = 0;

    int offset = 0;

    for (int i = 0; i < header->GetNFields(); i++)
    {
        if (strcmp(fieldNames[i], "checksum") != 0)
        {
            if (strcmp(fieldNames[i], "class") == 0)
            {
                // Class maps to type.
                typeOffset = offset;
            }
            else if (strcmp(fieldNames[i], "type") == 0)
            {
                // Type maps to species.
                speciesOffset = offset;
            }
            else if (strcmp(fieldNames[i], "group") == 0)
            {
                // Group maps to group.
                groupOffset = offset;
            }
            else if (strcmp(fieldNames[i], "pinfo") == 0)
            {
                pinfoOffset = offset;
                pinfoSize   = fieldSizes[i];
                pinfoType   = fieldTypes[i][0];
            }
            else if (strcmp(fieldNames[i], "rx") == 0)
            {
                xOffset = offset;
                xSize   = fieldSizes[i];
                xType   = fieldTypes[i][0];
            }
            else if (strcmp(fieldNames[i], "ry") == 0)
            {
                yOffset = offset;
                ySize   = fieldSizes[i];
                yType   = fieldTypes[i][0];
            }
            else if (strcmp(fieldNames[i], "rz") == 0)
            {
                zOffset = offset;
                zSize   = fieldSizes[i];
                zType   = fieldTypes[i][0];
            }
            else
            {
                varNames[nVars]   = fieldNames[i];
                varOffsets[nVars] = offset;
                varSizes[nVars]   = fieldSizes[i];
                varTypes[nVars]   = fieldTypes[i][0];
                nVars++;
            }
        }
        offset += fieldSizes[i];
    }

    for (int i = 0 ; i < nVars; i++)
    {
        debug1 << "varName=" << varNames[i] << ",offset=" << varOffsets[i]
               << ",size=" << varSizes[i] << ",type=" << varTypes[i] << endl;
    }

    //
    // Determine the mesh information.
    //
    const double *hMatrix = header->GetHMatrix();

    coordsUnit = "nm";
    coordsScale = 10.0;
    float xExtent = hMatrix[0] / 10.0; // now in nm
    float yExtent = hMatrix[4] / 10.0; // now in nm
    float zExtent = hMatrix[8] / 10.0; // now in nm
    if (max(xExtent, max(yExtent, zExtent)) > 1e3)
    {
        xExtent /= 1e3;
        yExtent /= 1e3;
        zExtent /= 1e3;
        coordsUnit = "um";
        coordsScale *= 1e3;
    }

    nDims = 3;
    xMin = -(xExtent / 2.0);
    dX   =   xExtent;
    yMin = -(yExtent / 2.0);
    dY   =   yExtent;
    zMin = -(zExtent / 2.0);
    dZ   =   zExtent;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::ReadCGridData
//
//  Purpose:
//      Read the data from a cgrid file.  Each processor reads in a portion
//      of the data and then the data is exchanged among the processors
//      into spacially contiguous blocks.
//
//  Arguments:
//      header      The header data for the current file.
//      subname     The name of the sub file.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Nov 20 10:44:45 PST 2008
//
// ****************************************************************************

void
avtDDCMDFileFormat::ReadCGridData(const DDCMDHeader *header,
    const string &subname)
{
#ifdef MDSERVER
    return;
#endif
    DetermineProcessorReadOffset(header, subname.c_str());

    DetermineBlockDecomposition();

    ReadProcessorChunk(header, subname.c_str());

    ExchangeProcessorData(header);

    CopyExchangeDataToBlocks(header);

    varsBlock = varValues;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::ReadAtomData
//
//  Purpose:
//      Read the data from an atom file.
//
//  Arguments:
//      header      The header data for the current file.
//      subname     The name of the sub file.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Nov 20 10:44:45 PST 2008
//
// ****************************************************************************

void
avtDDCMDFileFormat::ReadAtomData(const DDCMDHeader *header,
    const string &subname)
{
#ifdef MDSERVER
    return;
#endif
    DetermineProcessorReadOffset(header, subname.c_str());

    ReadProcessorChunk(header, subname.c_str());

    CopyDataToBlocks(header);

    for (int i = 0; i < nVars; i++)
    {
        varsBlock[nVarsBlock+i] = varValues[i];
    }

    delete [] varValues;
    varValues = 0;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::ReadData
//
//  Purpose:
//      Read in the data portion of the file.  This is everything but
//      the header.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
// ****************************************************************************

void
avtDDCMDFileFormat::ReadData(vector<DDCMDHeader*> &headers)
{
    if (cgridFile)
    {
        ParseCGridHeader(headers[0]);

        ReadCGridData(headers[0], subnames[0]);

        nVarsBlock = nVars;
        varNamesBlock = varNames;
    }
    else
    {
        int nFields = 0;
        for (int i = 0; i < headers.size(); i++)
            nFields += headers[i]->GetNFields();

        nVarsBlock = 0;
        varNamesBlock = new string[nFields];
#ifndef MDSERVER
        varsBlock = new float*[nFields];
#endif
        for (int i = 0; i < subnames.size(); i++)
        {
            ParseAtomHeader(headers[i]);

            ReadAtomData(headers[i], subnames[i]);

            for (int i = 0; i < nVars; i++)
            {
                varNamesBlock[nVarsBlock] = varNames[i];
                nVarsBlock++;
            }

            delete [] varNames;
            varNames = 0;
        }
    }

    //
    // Print out the mesh extents to the debug logs.
    //
    debug1 << "coordsUnit=" << coordsUnit << endl;
    debug1 << "xMin=" << xMin << ",yMin=" << yMin << ",zMin=" << zMin << endl;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::GetPointMesh
//
//  Purpose:
//      Get the point mesh for the file.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Nov 20 10:44:45 PST 2008
//
// ****************************************************************************

vtkDataSet *
avtDDCMDFileFormat::GetPointMesh()
{
    vtkPoints *points  = vtkPoints::New();
    points->SetNumberOfPoints(nPoints);
    float *pts = vtkFloatArray::SafeDownCast(points->GetData())->GetPointer(0);

    for (int i = 0; i < nPoints*3; i++)
        pts[i] = coordsBlock[i];

    //
    // Create the VTK objects and connect them up.
    //
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    ugrid->Allocate(nPoints);
    vtkIdType onevertex[1];
    for (int i = 0 ; i < nPoints; i++)
    {
        onevertex[0] = i;
        ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
    }

    points->Delete();

    return ugrid;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::GetRectilinearMesh
//
//  Purpose:
//      Get the rectilinear mesh for the file.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Nov 20 10:44:45 PST 2008
//
// ****************************************************************************

vtkDataSet *
avtDDCMDFileFormat::GetRectilinearMesh()
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
//  Method: avtDDCMDFileFormat::GetPointVar
//
//  Purpose:
//      Get the specified point variable for the file.
//
//  Arguments:
//      varname     The name of the variable to get.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Nov 20 10:44:45 PST 2008
//
// ****************************************************************************

vtkDataArray *
avtDDCMDFileFormat::GetPointVar(const char *varname)
{
    string var = varname;

    float *tmpData = 0;
    float *data = 0;
    if (var == "species" || var == "group" || var == "type")
    {
        tmpData = new float[nPoints];
        data = tmpData;
        for (int i = 0; i < nPoints; i++)
        {
            unsigned pinfo = pinfoBlock[i];
            int ig = pinfo % nGroups;
            pinfo /= nGroups;
            int is = pinfo % nSpecies;
            pinfo /= nSpecies;
            int it = pinfo;
            if (var == "species")
                data[i] = float(is);
            else if (var == "group")
                data[i] = float(ig);
            else
                data[i] = float(it);
        }
    }
    else
    {
        //
        // Get the index of the variable name.
        //
        int i;

        for (i= 0; i < nVarsBlock && varNamesBlock[i] != varname; i++)
            /* Do nothing. */;
        if (i== nVarsBlock)
            EXCEPTION1(InvalidVariableException, varname);
        data = varsBlock[i];
    }

    //
    // Copy the data array.
    //
    vtkFloatArray *rv = vtkFloatArray::New();

    rv->SetNumberOfTuples(nPoints);

    for (int i = 0; i < nPoints; i++)
    {
        rv->SetTuple1(i, *(data++));
    }

    //
    // Free temporary storage.
    //
    if (tmpData != 0)  delete [] tmpData;

    return rv;
}


// ****************************************************************************
//  Method: avtDDCMDFileFormat::GetRectilinearVar
//
//  Purpose:
//      Get the specified rectlinear variable for the file.
//
//  Arguments:
//      varname     The name of the variable to get.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Nov 20 10:44:45 PST 2008
//
// ****************************************************************************

vtkDataArray *
avtDDCMDFileFormat::GetRectilinearVar(const char *varname)
{
    string var = varname;

    //
    // Determine the variable name and the species name.
    //
    int i;
    for (i = var.length() - 1; i > 0 && var[i] != '_'; i--)
        /* do nothing */;
    string varBase = var.substr(0, i);
    string varSpec = var.substr(i+1, var.length() - i - 1);
    
    //
    // Get the index of the variable name and the index of the species name.
    //
    int iVar;

    for (iVar = 0; iVar < nVarsBlock && varNamesBlock[iVar] != varBase; iVar++)
        /* Do nothing. */;
    if (iVar == nVarsBlock)
        EXCEPTION1(InvalidVariableException, varname);

    int iSpec;

    for (iSpec = 0; iSpec < nSpecies && strcmp(speciesNames[iSpec], varSpec.c_str()); iSpec++)
        /* Do nothing. */;
    if (iSpec == nSpecies)
        EXCEPTION1(InvalidVariableException, varname);

    //
    // Copy the data array.
    //
    vtkFloatArray *rv = vtkFloatArray::New();

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
//  Method: avtDDCMDFileFormat constructor
//
//  Programmer: brugger -- generated by xml2avt
//  Creation:   Fri Aug 31 15:27:59 PST 2007
//
//  Modifications:
//    Eric Brugger, Fri Nov 21 12:39:10 PST 2008
//    I modified the routine to get the sub file names from the file
//    instead of assuming it was cgrid.  If there are no sub file names
//    in the file, then it assumes that it is cgrid to maintain backward
//    compatability.
//
// ****************************************************************************

avtDDCMDFileFormat::avtDDCMDFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    //
    // Store the directory name with the data files.
    //
    string name(filename);
    fname = name.substr(0,name.length()-6);

    //
    // Read the file to determine the sub file names to read in the
    // directory.  If the file doesn't contain any names assume it is
    // a cgrid file.  This is to maintain backward compatability, where
    // only cgrid files were supported and the file was a zero length
    // file.
    //
    int       maxlen, lbuf;
    char      str[1024], *buf, *b;
    FILE     *file;

    file = fopen(filename, "r");
    str[0] = '\0';
    maxlen = 1024;
    buf = str;
    b = str;
    while (b != NULL)
    {
        lbuf = strlen(buf);
        b = fgets(buf + lbuf, maxlen - lbuf, file);
    }
    fclose(file);

    if (lbuf == 0)
    {
        subnames.push_back("cgrid");
    }
    else
    {
        int       nFiles;
        char    **fileNames;
        OBJECT   *obj;

        obj = (OBJECT*) malloc(sizeof(OBJECT));
        object_lineparse(buf, obj);

        nFiles = object_getv1(obj, (char*)"files", (void**)&fileNames, STRING);
        if (nFiles == 0)
        {
            subnames.push_back("cgrid");
        }
        else
        {
            for (int i = 0; i < nFiles; i++)
            {
                subnames.push_back(fileNames[i]);
                free(fileNames[i]);
            }
            free(fileNames);
        }

        free(obj);
    }

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
        for (int i = 0; i < nVarsBlock; i++)
        {
            delete [] varsBlock[i];
        }
        delete [] varsBlock;
        varsBlock = 0;
    }

    //
    // Indicate that the data needs to be read again.
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
//  Modifications:
//    Eric Brugger, Thu Nov 20 10:44:45 PST 2008
//    I added the ability to read atom files, which required being able to
//    read multiple files to get all the data.
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
    vector<DDCMDHeader*> headers = ReadHeader();

    //
    // Read the data.
    //
    ReadData(headers);

    //
    // Free the data.
    //
    for (int i = 0; i < headers.size(); i++)
        delete headers[i];

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
//  Modifications:
//    Eric Brugger, Tue Jul 15 15:21:50 PDT 2008
//    I added the stress tensor to the list of variables in the file.
//
//    Eric Brugger, Thu Nov 20 10:44:45 PST 2008
//    I added the ability to read atom files, which required being able to
//    read multiple files to get all the data.
//
// ****************************************************************************

void
avtDDCMDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (cgridFile)
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
        double *extents = 0;

        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                          spatial_dimension, topological_dimension);
        md->SetFormatCanDoDomainDecomposition(true);

        //
        // Set the variable information.
        //
        Expression expr;
        for (int i = 0; i < nVarsBlock; i++)
        {
            for (int j = 0; j < nSpecies; j++)
            {
                string name = varNamesBlock[i] + "_" + string(speciesNames[j]);
                AddScalarVarToMetaData(md, name, meshname, AVT_ZONECENT);
            }

            expr.SetName(varNamesBlock[i]);
            string name = varNamesBlock[i] + "_" + string(speciesNames[0]);
            for (int j = 1; j < nSpecies; j++)
            {
                name += " + " + varNamesBlock[i] + "_" + string(speciesNames[j]);
            }
            expr.SetDefinition(name);
            expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&expr);
        }
    }
    else
    {
        //
        // Set the mesh information.
        //
        string meshname = "atoms";

        avtMeshType mt = AVT_POINT_MESH;

        int nblocks = 1;
        int block_origin = 0;
        int spatial_dimension = nDims;
        int topological_dimension = nDims;
        double extents[6];
        extents[0] = xMin;
        extents[1] = xMin + dX;
        extents[2] = yMin;
        extents[3] = yMin + dY;
        extents[4] = zMin;
        extents[5] = zMin + dZ;

        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                          spatial_dimension, topological_dimension);
        md->SetFormatCanDoDomainDecomposition(true);

        //
        // Set the variable information.
        //
        for (int i = 0; i < nVarsBlock; i++)
        {
            AddScalarVarToMetaData(md, varNamesBlock[i], meshname, AVT_ZONECENT);
        }

        //
        // Set the variables incoded in pinfo.
        //
        if (nSpecies > 1)
            AddScalarVarToMetaData(md, "species", meshname, AVT_ZONECENT);
        if (nGroups > 1)
            AddScalarVarToMetaData(md, "group", meshname, AVT_ZONECENT);
        if (nTypes > 1)
            AddScalarVarToMetaData(md, "type", meshname, AVT_ZONECENT);
    }
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
//  Modifications:
//    Eric Brugger, Thu Nov 20 10:44:45 PST 2008
//    I added the ability to read atom files, which required being able to
//    read multiple files to get all the data.
//
// ****************************************************************************

vtkDataSet *
avtDDCMDFileFormat::GetMesh(int domain, const char *meshname)
{
    if (cgridFile)
        return GetRectilinearMesh();
    else
        return GetPointMesh();
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
//  Modifications:
//    Eric Brugger, Tue Jul 15 15:21:50 PDT 2008
//    I changed the loop that searches for an underscore in the variable name
//    to split the name into a variable name and a species name to search
//    from the end of the name backwards so that it can handle variable
//    names with underscores.
//
//    Eric Brugger, Thu Nov 20 10:44:45 PST 2008
//    I added the ability to read atom files, which required being able to
//    read multiple files to get all the data.
//
// ****************************************************************************

vtkDataArray *
avtDDCMDFileFormat::GetVar(int domain, const char *varname)
{
    if (domain != 0)
        EXCEPTION1(InvalidVariableException, varname);

    if (cgridFile)
        return GetRectilinearVar(varname);
    else
        return GetPointVar(varname);
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
