/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <avtStructuredDomainBoundaries.h>
#include <avtVariableCache.h>

#include <Expression.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <FileFunctions.h>

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

// Make it easier to keep the spellings consistent.
#define DDCMD_FIXRECORDBINARY "FIXRECORDBINARY"
#define DDCMD_FIXRECORDASCII  "FIXRECORDASCII"

// ****************************************************************************
// Method: ReadString
//
// Purpose: 
//   Read a field as a string.
//
// Arguments:
//   obj : The object.
//   key : The key value to read.
//   default_values : The default value if the key is not present.
//
// Returns:    The value for the key.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct  8 15:36:59 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

static std::string
ReadString(OBJECT *obj, const char *key, const char *default_value)
{
    std::string retval(default_value);
    char *val = NULL;
    object_get(obj, (char*)key, &val, STRING, 1, (char*)default_value);
    if(val != NULL)
    {
        retval = std::string(val);
        free(val);
    }
    return retval;
}

// ****************************************************************************
// Method: ReadStringVector
//
// Purpose: 
//   Read a field as a string vector.
//
// Arguments:
//   obj : The object.
//   key : The key value to read.
//   values : The key values were were read.
//
// Returns:    The number of key values that were read.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct  8 15:36:59 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

static int
ReadStringVector(OBJECT *obj, const char *key, stringVector &values)
{
    values.clear();
    TRY
    {
        char **val = NULL;
        int nvals = object_getv(obj, (char*)key, (void **)&val, STRING);

        if(nvals > 0 && val != NULL)
        {
            for(int i = 0; i < nvals; ++i)
                values.push_back(std::string(val[i]));

            /* Free as one chunk because it looks like it was allocated in one
               chunk in object.C */
           free(val);
        }
    }
    CATCH(InvalidFilesException)
    {
        // Do nothing. We just asked for a key that does not exist in the object
        // and that triggered some exception coding.
    }
    ENDTRY

    return (int)values.size();
}

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
//    Jeremy Meredith, Thu Jan  7 12:25:48 EST 2010
//    Error if we can't open the file.  Stop reading at EOF.
//
//    Brad Whitlock, Mon Oct  8 15:09:45 PDT 2012
//    More initialization and error checking. Switch to stringVector. Split
//    into helper methods.
//
// ****************************************************************************

DDCMDHeader::DDCMDHeader(const std::string &fname, const std::string &subname)
{
    // Initialize some dynamic members in case they don't have values.
    headerLength = 0;
    dataType = std::string(DDCMD_FIXRECORDASCII);
    lRec = nRecord = nFiles = nFields = swap = 0;
    memset(hMatrix, 0, 9 * sizeof(double));
    loop = 0;
    time = 0;
    fieldSizes = NULL;
    nXFile = nYFile = nZFile = 0;

    //
    // Open the file and read the header. It is a bunch of lines terminated
    // with a null character.
    //
    std::string filename(fname + "/" + subname + "#000000");
    char header[4096];
    headerLength = Read(filename, 4096, header);
    if(headerLength <= 0) ///TODO: headerLength < 0 will never be true setting <= 0 as 0 size file is also bad
        EXCEPTION1(InvalidFilesException, fname);

    //
    // Parse the header.
    //
    ParseHeader(header);

    //
    // Print out the contents of the header to the debug logs.
    //
    if(DebugStream::Level1())
        Print(DebugStream::Stream1());
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
//  Modifications:
//    Brad Whitlock, Mon Oct  8 15:38:56 PDT 2012
//    I removed deletion code because I switched to stringVector.
//
// ****************************************************************************

DDCMDHeader::~DDCMDHeader()
{
    //
    // We are using free, since all the memory was allocated with malloc.
    //
    free(fieldSizes);
};

// ****************************************************************************
//  Method: DDCMDHeader::ParseHeader
//
//  Purpose:
//    Read the header from the specified file.
//
//  Arguments:
//    header : The buffer that contains the header to parse.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Nov 20 10:44:45 PST 2008
//
//  Modifications:
//    Brad Whitlock, Tue Oct  9 10:26:39 PDT 2012
//    I moved code here from the constructor.
//
// ****************************************************************************

void
DDCMDHeader::ParseHeader(char *header)
{
    OBJECT       *obj = NULL;
    unsigned int  endianKey, keyLocal;

    obj = (OBJECT*) malloc(sizeof(OBJECT));
    object_lineparse(header, obj);

    dataType = ReadString(obj, "datatype", "NONE");

    object_get(obj, (char*)"lrec", &lRec, INT, 1, (char*)"16");
    object_get(obj, (char*)"endian_key", &endianKey, INT, 1, (char*)"4");
    object_get(obj, (char*)"nrecord", &nRecord, INT, 1, (char*)"0");
    if(nRecord == 0)
    {
        // Also accept "nrecords"
        object_get(obj, (char*)"nrecords", &nRecord, INT, 1, (char*)"0");
    }
    object_get(obj, (char*)"nfiles", &nFiles, INT, 1, (char*)"1");
    object_get(obj, (char*)"nfields", &nFields, INT, 1, (char*)"1");

    ReadStringVector(obj, "field_names", fieldNames);
    ReadStringVector(obj, "field_types", fieldTypes);
    ReadStringVector(obj, "field_units", fieldUnits);

    fieldSizes = (unsigned*) malloc(nFields*sizeof(int));
    if (dataType == std::string("FIXRECORDBINARY"))
    {
        for (unsigned int i = 0; i < nFields; i++)
        {
            fieldSizes[i] = atoi(&fieldTypes[i][1]);
        }
    }
    else
    {
        for (unsigned int i = 0; i < nFields; i++)
        {
            fieldSizes[i] = 1;
        }
    }

    object_get(obj, (char*)"h", &hMatrix, DOUBLE, 9, (char*)"0.0 0.0 0.0 "
                                                            "0.0 0.0 0.0 "
                                                            "0.0 0.0 0.0");
    object_get(obj, (char*)"time", &time, DOUBLE, 1, (char*)"0.0");
    object_get(obj, (char*)"loop", &loop, INT, 1, (char*)"0");

    memcpy(&keyLocal, "1234", 4);
    swap = (keyLocal != endianKey);

    ReadStringVector(obj, "species", speciesNames);

    exeVersion = ReadString(obj, "exe_version", "NONE");

    //
    // Cgrid-specific information.
    //
    object_get(obj, (char*)"nx", &nXFile, INT, 1, (char*)"0");
    object_get(obj, (char*)"ny", &nYFile, INT, 1, (char*)"0");
    object_get(obj, (char*)"nz", &nZFile, INT, 1, (char*)"0");
    if(GetCGridFile())
    {
        double sum = 0.; 
        for(int i = 0; i < 9; ++i)
            sum += hMatrix[i];
        // Assume that hMatrix was not read.
        if(sum == 0.)
        {
            hMatrix[0] = nXFile;
            hMatrix[4] = nYFile;
            hMatrix[8] = nZFile;
        }
    }

    //
    // Atom-specific information.
    //
    ReadStringVector(obj, "groups", groupNames);
    ReadStringVector(obj, "types", typeNames);

    object_free(obj);
}

bool
DDCMDHeader::GetCGridFile() const
{
    //
    // Determine whether we have a cgrid or an atom file.
    //
    bool cgridFile = !(nXFile == 0 && nYFile == 0 && nZFile == 0) ? true : false;
    return cgridFile;
}

// ****************************************************************************
//  Method: DDCMDHeader::Print
//
//  Purpose:
//    Print the object to a stream.
//
//  Arguments:
//    os : The stream 
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Oct  9 10:26:39 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

void
DDCMDHeader::Print(ostream &os) const
{
    os << "DDCMDHeader { " << endl;
    os << "    cgridFile = " << (GetCGridFile()?"true":"false") << endl;
    os << "    isLegacy = " << (IsLegacy()?"true":"false") << endl;
    os << endl;
    os << "    headerLength = " << headerLength << endl;
    os << "    dataType = " << dataType << endl;
    os << "    lRec = " << lRec << endl;
    os << "    nRecord= " << nRecord << endl;
    os << "    nFiles = " << nFiles << endl;
    os << "    nFields = " << nFields << endl;
    os << "    swap = " << swap << endl;
    os << "    hMatrix = {";
    for(int i = 0; i < 9; ++i)
        os << hMatrix[i] << ", ";
    os << "}" << endl;
    os << "    loop = " << loop << endl;
    os << "    time = " << time << endl;
    os << "    fieldNames=";
    for (unsigned int i = 0; i < nFields; i++)
    {
        os << fieldNames[i];
        if (i < nFields - 1) os << ",";
    }
    os << "}" << endl;
    os << "    fieldTypes = {";
    for (unsigned int i = 0; i < nFields; i++)
    {
        os << fieldTypes[i];
        if (i < nFields - 1) os << ",";
    }
    os << "}" << endl;
    os << "    fieldUnits = {";
    for (size_t i = 0; i < fieldUnits.size(); i++)
    {
        os << fieldUnits[i];
        if (i < fieldUnits.size() - 1) os << ",";
    }
    os << "}" << endl;
    os << "    fieldSizes = {";
    for (unsigned int i = 0; i < nFields; i++)
    {
        os << fieldSizes[i];
        if (i < nFields - 1) os << ",";
    }
    os << "}" << endl;
    os << "    nXFile = " << nXFile << endl;
    os << "    nYFile = " << nYFile << endl;
    os << "    nZFile = " << nZFile << endl;
    os << "    speciesNames = {";
    for (size_t i = 0; i < speciesNames.size(); i++)
    {
        os << speciesNames[i];
        if (i < speciesNames.size() - 1) os << ",";
    }
    os << "}" << endl;
    os << "    groupNames = {";
    for (size_t i = 0; i < groupNames.size(); i++)
    {
        os << groupNames[i];
        if (i < groupNames.size() - 1) os << ",";
    }
    os << "}" << endl;
    os << "    typeNames = {";
    for (size_t i = 0; i < typeNames.size(); i++)
    {
        os << typeNames[i];
        if (i < typeNames.size() - 1) os << ",";
    }
    os << "}" << endl;
    os << "    exeVersion = " << exeVersion << endl;
    os << "}" << endl;
}

// ****************************************************************************
// Method: DDCMDHeader::IsLegacy
//
// Purpose: 
//   Return true if the file is a legacy file.
//
// Arguments:
//
// Returns:    True if the file looks like a legacy file. False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct  9 11:33:49 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

bool
DDCMDHeader::IsLegacy() const
{
    // If the file contains "cardioid" then it is NOT a legacy file.
    if(exeVersion.find("cardioid") != std::string::npos)
        return false;

    return exeVersion == "NONE";
}

// ****************************************************************************
// Method: DDCMDHeader::Read
//
// Purpose: 
//   Read the DDCMD header file into a buffer.
//
// Arguments:
//   filename : The name of the file to read.
//   maxlen   : The size of the buffer.
//   header   : The buffer that will contain the header after reading the file.
//
// Returns:    The number of characters read in the buffer.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct  9 10:16:57 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

int
DDCMDHeader::Read(const std::string &filename, int maxlen, char *header)
{
    FILE *file = fopen(filename.c_str(), "r");
    if (!file)
        return -1;

    char* res = NULL; (void) res;
    size_t headerlength = 0;
    res = fgets(header, maxlen - headerlength, file); 
    headerlength = strlen(header);
    char *h = header + headerlength;
    res = fgets(h, maxlen - headerlength, file);
    while(*h != '\n' && !feof(file))
    {
        headerlength = strlen(header);
        res = fgets(h = header + headerlength, maxlen-headerlength, file);
    }
    headerlength = strlen(header);
    fclose(file);

    //
    // Convert any newline characters to spaces.
    //
    size_t i;
    for (i = 0; i < headerlength; i++)
        if (header[i] == '\n' ) header[i] = ' ';
    header[i] = '\0';

    return (int)headerlength;
}

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
//    Brad Whitlock, Thu Oct 11 12:05:53 PDT 2012
//    Code cleanup. Add support for fixed ascii id's.
//
// ****************************************************************************

avtDDCMDFileFormat::DataRecords
avtDDCMDFileFormat::ExchangeProcessorData(const DDCMDHeader *header, 
    avtDDCMDFileFormat::DataRecords &input)
{
    DataRecords output;
#ifdef PARALLEL
    unsigned int lRec = header->GetLRec();
    unsigned int swap = header->GetSwap();
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

    // Define a macro to perform id to iBlock
#define ID_TO_IBLOCK \
            int iX = id / (nYFile * nZFile);\
            int tmp = id % (nYFile * nZFile);\
            int iY = tmp / nZFile;\
            int iZ = tmp % nZFile;\
            int iXBlock = iX / deltaX;\
            int iYBlock = iY / deltaY;\
            int iZBlock = iZ / deltaZ;\
            int iBlock = iXBlock * nYFileBlocks * nZFileBlocks +\
                         iYBlock * nZFileBlocks + iZBlock;

    char *ptr = input.data;
    if (header->GetDataType() == DDCMD_FIXRECORDASCII)
    {
        for (unsigned long i = 0; i < input.nRecords; i++)
        {
            unsigned int id = 0;
            sscanf(ptr, "%u", &id);

            ID_TO_IBLOCK

            outCharCounts[iBlock] += lRec;

            ptr += lRec;
        }
    }
    else if (labelUnsigned)
    {
        for (unsigned long i = 0; i < input.nRecords; i++)
        {
            //
            // Byte swap the data if necessary.
            //
            unsigned id = *((unsigned *) (ptr+labelOffset));
            if (swap) Convert(&id, 4);

            ID_TO_IBLOCK

            outCharCounts[iBlock] += lRec;

            ptr += lRec;
        }
    }
    else
    {
        for (unsigned long i = 0; i < input.nRecords; i++)
        {
            //
            // Byte swap the data if necessary.
            //
            long long id = bFieldConvert((unsigned char *)ptr+labelOffset, labelSize);

            ID_TO_IBLOCK

            outCharCounts[iBlock] += lRec;

            ptr += lRec;
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
    // Reorder the input data records and transfer them to the output buffer.
    //
    char *outProcessorData = new char[input.nRecords*lRec];
    ptr = input.data;
    if (header->GetDataType() == DDCMD_FIXRECORDASCII)
    {
        for (unsigned long i = 0; i < input.nRecords; i++)
        {
            unsigned int id;
            sscanf(ptr, "%u", &id);

            ID_TO_IBLOCK

            memcpy(outProcessorData + outProcOffsets[iBlock], ptr, lRec);

            outProcOffsets[iBlock] += lRec;

            ptr += lRec;
        }
    }
    else if (labelUnsigned)
    {
        for (unsigned long i = 0; i < input.nRecords; i++)
        {
            //
            // Byte swap the data if necessary.
            //
            unsigned id = *((unsigned *) (ptr+labelOffset));
            if (swap) Convert(&id, 4);

            ID_TO_IBLOCK

            memcpy(outProcessorData + outProcOffsets[iBlock], ptr, lRec);

            outProcOffsets[iBlock] += lRec;

            ptr += lRec;
        }
    }
    else
    {
        for (unsigned long i = 0; i < input.nRecords; i++)
        {
            //
            // Byte swap the data if necessary.
            //
            long long id = bFieldConvert((unsigned char *)ptr+labelOffset, labelSize);

            ID_TO_IBLOCK

            memcpy(outProcessorData + outProcOffsets[iBlock], ptr, lRec);

            outProcOffsets[iBlock] += lRec;

            ptr += lRec;
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
    // Allocate the buffer to recieve the data from the other processors.
    //
    long nInRecords = inProcOffsets[nProcs - 1] + inCharCounts[nProcs - 1];
    output.nRecords = nInRecords / lRec;
    output.data = new char[nInRecords];
    
    //
    // Exchange the data among processors.
    //
    MPI_Alltoallv(outProcessorData, outCharCounts, outProcOffsets, MPI_CHAR,
                  output.data, inCharCounts, inProcOffsets, MPI_CHAR,
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
#else
    output = input;
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

    return output;
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
//      input       The unordered data records that we're putting into zonal arrays.
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
//    Brad Whitlock, Mon Oct  8 16:06:33 PDT 2012
//    Code cleanup and improvements that work with ASCII records.
//
// ****************************************************************************

void
avtDDCMDFileFormat::CopyExchangeDataToBlocks(const DDCMDHeader *header, 
    avtDDCMDFileFormat::DataRecords &input)
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
    const stringVector &fieldTypes = header->GetFieldTypes();
    unsigned int  *fieldSizes = header->GetFieldSizes();

    int deltaX = (nXFile + nXFileBlocks - 1) / nXFileBlocks;
    int deltaY = (nYFile + nYFileBlocks - 1) / nYFileBlocks;
    int deltaZ = (nZFile + nZFileBlocks - 1) / nZFileBlocks;

    int iBlock = rank;
    int iXBlock = iBlock / (nYFileBlocks * nZFileBlocks);
    int iMin = iXBlock * deltaX;
    int iMax = iMin + deltaX < (int)nXFile ? iMin + deltaX : (int)nXFile;

    iBlock = iBlock % (nYFileBlocks * nZFileBlocks);
    int iYBlock = iBlock / nZFileBlocks;
    int jMin = iYBlock * deltaY;
    int jMax = jMin + deltaY < (int)nYFile ? jMin + deltaY : (int)nYFile;

    int iZBlock = iBlock % nZFileBlocks;
    int kMin = iZBlock * deltaZ;
    int kMax = kMin + deltaZ < (int)nZFile ? kMin + deltaZ : (int)nZFile;

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
       
    unsigned long nZones = nZonesBlock;

    varValues = new float*[nVars];
    for (int i = 0; i < nVars; i++)
    {
        unsigned int nComponents = max(nSpecies, (unsigned int)1);
        varValues[i] = new float[nZones*nComponents];
        memset(varValues[i], 0, nZones*nComponents*sizeof(float));
    }

#define ID_TO_IZONE \
            int iX = id / (nYFile * nZFile);\
            id %= (nYFile * nZFile);\
            int iY = id / nZFile;\
            int iZ = id % nZFile;\
            int iXZone, iYZone, iZZone;\
            if  (nDims == 2)\
            {\
                iXZone = iY % deltaY;\
                iYZone = iZ % deltaZ;\
                iZZone = 0;\
            }\
            else\
            {\
                iXZone = iX % deltaX;\
                iYZone = iY % deltaY;\
                iZZone = iZ % deltaZ;\
            }\
            unsigned long iZone = iZZone * nXBlock * nYBlock +\
                                  iYZone * nXBlock + iXZone;\
            unsigned long nZones = nZonesBlock;

    //
    // Copy the processor data to the blocks.
    //
    char *data = input.data;
    if(header->GetDataType() == DDCMD_FIXRECORDASCII)
    {
        char **recOffsets = new char*[nFields];
        for (unsigned long i = 0; i < input.nRecords; i++)
        {
            //
            // Determine the start of each field.
            //
            char *rec = data;
            for (unsigned int j = 0; j < nFields; j++)
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
            // Get the id
            //
            unsigned int id = 0, iSpecies = 0;
            sscanf(data, "%u", &id);

            //
            // Copy the record into the correct zone in the correct block.
            //
            ID_TO_IZONE

            for (int j = 0; j < nVars; j++)
            {
                float *fvar = varValues[j];
                switch (varTypes[j])
                {
                  case 'f':
                    fvar[iSpecies*nZones+iZone] += strtof(recOffsets[varOffsets[j]], NULL);
                    break;
                  case 'u':
                    fvar[iSpecies*nZones+iZone] += float(strtol(recOffsets[varOffsets[j]], NULL, 10));
                    break;
                }
            }
            data += lRec;
        }
        delete [] recOffsets;
    }
    else
    {
        for (unsigned long i = 0; i < input.nRecords; i++)
        {
            //
            // Byte swap the data if necessary.
            //
            char *field = data;
            for (unsigned int j = 0; j < nFields; j++)
            {
                if (swap && fieldTypes[j][0] != (char)'b') Convert(field, fieldSizes[j]);
                field += fieldSizes[j];
            }

            //
            // Get the id and iSpecies.
            //
            long long id, iSpecies = 0;
            if (labelUnsigned)
                id = *((unsigned *) (data+labelOffset));
            else
                id = bFieldConvert((unsigned char *)data+labelOffset, labelSize);
            if(nSpecies > 0)
            {
                if (iSpeciesUnsigned)
                    iSpecies = *((unsigned *) (data+iSpeciesOffset));
                else
                    iSpecies = bFieldConvert((unsigned char *)data+iSpeciesOffset, iSpeciesSize);
            }

            //
            // Copy the record into the correct zone in the correct block.
            //
            ID_TO_IZONE

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
    }

    delete [] varOffsets;
    varOffsets = 0;
    delete [] varSizes;
    varSizes = 0;
    delete [] varTypes;
    varTypes = 0;
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
//  Modifications:
//    Cyrus Harrison, Thu Apr 30 14:13:42 PDT 2009
//    Added a fix for atoms files without species data.
//
//    Brad Whitlock, Mon Oct  8 15:42:18 PDT 2012
//    Code cleanup.
//
//    Brad Whitlock, Fri Nov 16 15:17:37 PST 2012
//    Turn coordsBlock into vtkPoints so we don't have to convert to it later.
//
//    Brad Whitlock, Wed Dec  5 16:38:49 PST 2012
//    Use unsigned long instead of int.
//
// ****************************************************************************

void
avtDDCMDFileFormat::CopyAsciiDataToBlocks(const DDCMDHeader *header, 
    avtDDCMDFileFormat::DataRecords &input)
{
    unsigned int   lRec       = header->GetLRec();
    unsigned int   nFields    = header->GetNFields();

    nPoints = input.nRecords;
    char *data = input.data;

    int nSpecies = header->GetNSpecies();
    int nGroups  = header->GetNGroups();
    int nTypes   = header->GetNTypes();
    const stringVector &speciesNames = header->GetSpeciesNames();
    const stringVector &groupNames   = header->GetGroupNames();
    const stringVector &typeNames    = header->GetTypeNames();

    char **recOffsets = new char*[nFields];
    for (unsigned long i = 0; i < nPoints; i++)
    {
        //
        // Determine the start of each field.
        //
        char *rec = data;
        for (unsigned int j = 0; j < nFields; j++)
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

        // Make sure we actually have species & type offsets 
        if (groupOffset != -1 && speciesOffset != -1 && typeOffset!=-1 )
        {
            int iGroup = 0;
            while (iGroup < nGroups &&
                   strcmp(recOffsets[groupOffset], groupNames[iGroup].c_str()))
                iGroup++;
            int iSpecies = 0;
            while (iSpecies < nSpecies &&
                   strcmp(recOffsets[speciesOffset], speciesNames[iSpecies].c_str()))
                iSpecies++;
            int iType = 0;
            while (iType < nTypes &&
                   strcmp(recOffsets[typeOffset], typeNames[iType].c_str()))
                iType++;
            pinfoBlock[i] = iGroup * nSpecies * nTypes +
                            iSpecies * nTypes + iType;
        }
        // moved coords setup out of above conditional
        float pt[3];
        pt[0] = strtof(recOffsets[xOffset], NULL) / coordsScale;
        pt[1] = strtof(recOffsets[yOffset], NULL) / coordsScale;
        pt[2] = strtof(recOffsets[zOffset], NULL) / coordsScale;
        coordsBlock->SetPoint(i, pt);

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
//  Modifications:
//    Brad Whitlock, Thu Oct 11 13:17:08 PDT 2012
//    Pass in the data records that we're operating on.
//
//    Brad Whitlock, Fri Nov 16 15:17:37 PST 2012
//    Turn coordsBlock into vtkPoints so we don't have to convert to it later.
//
// ****************************************************************************

void
avtDDCMDFileFormat::CopyBinaryDataToBlocks(const DDCMDHeader *header, 
    avtDDCMDFileFormat::DataRecords &input)
{
    unsigned int   lRec       = header->GetLRec();
    unsigned int   nFields    = header->GetNFields();
    int            swap       = header->GetSwap();
    const stringVector &fieldTypes = header->GetFieldTypes();
    unsigned int  *fieldSizes = header->GetFieldSizes();

    nPoints = input.nRecords;
    char *data = input.data;

    for (unsigned long i = 0; i < nPoints; i++)
    {
        //
        // Byte swap the data if necessary.
        //
        char *field = data;
        for (unsigned int j = 0; j < nFields; j++)
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

            float pt[3];
            pt[0] = *((float *) (data + xOffset)) / coordsScale;
            pt[1] = *((float *) (data + yOffset)) / coordsScale;
            pt[2] = *((float *) (data + zOffset)) / coordsScale;
            coordsBlock->SetPoint(i, pt);
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
//    Cyrus Harrison, Thu Apr 30 14:13:42 PDT 2009
//    Added a fix for atoms files without species data.
//
//    Brad Whitlock, Mon Oct  8 16:03:11 PDT 2012
//    Use std::string for dataType.
//
//    Brad Whitlock, Fri Nov 16 15:17:37 PST 2012
//    Turn coordsBlock into vtkPoints so we don't have to convert to it later.
//
// ****************************************************************************

void
avtDDCMDFileFormat::CopyDataToBlocks(const DDCMDHeader *header, 
    avtDDCMDFileFormat::DataRecords &input)
{
#ifdef PARALLEL
    int rank = PAR_Rank();
#else
    int rank = 0;
#endif

    nPoints = nRecordsList[rank];

    //
    // Check that the data type is valid.
    //
    if (header->GetDataType() != DDCMD_FIXRECORDBINARY &&
        header->GetDataType() != DDCMD_FIXRECORDASCII)
    {
        debug1 << "Invalid data type: dataType=" << header->GetDataType() << endl;
    }

    //
    // Initialize the storage for the block data.
    //
    if (pinfoOffset != -1 || groupOffset != -1)
    {
        pinfoBlock = new unsigned[nPoints];
    }

    // moved coordsBlock allocate outside of above conditional
    if(coordsBlock != NULL)
    {
        //coordsBlock->Delete();
        debug5 << "coordsBlock != NULL: Do we need to delete it here?" << endl;
    }
    debug5 << "CopyDataToBlocks: Allocating coordinates for " << nPoints << " points" << endl;
    coordsBlock = vtkPoints::New();
    coordsBlock->SetNumberOfPoints(nPoints);

    varValues = new float*[nVars];
    for (int i = 0; i < nVars; i++)
    {
        varValues[i] = new float[nPoints];
    }

    if (header->GetDataType() == DDCMD_FIXRECORDASCII)
        CopyAsciiDataToBlocks(header, input);
    else if (header->GetDataType() == DDCMD_FIXRECORDBINARY)
        CopyBinaryDataToBlocks(header, input);
        
    //
    // Free memory.
    //
    delete [] varOffsets;
    varOffsets = 0;
    delete [] varSizes;
    varSizes = 0;
    delete [] varTypes;
    varTypes = 0;
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
//    Brad Whitlock, Thu Oct 11 14:54:22 PDT 2012
//    Return the data that we read in DataRecords.
//
// ****************************************************************************

avtDDCMDFileFormat::DataRecords
avtDDCMDFileFormat::ReadProcessorChunk(const DDCMDHeader *header,
    const std::string &subname)
{
#ifdef PARALLEL
    int rank = PAR_Rank();
#else
    int rank = 0;
#endif

    unsigned int lRec   = header->GetLRec();
    unsigned int nFiles = header->GetNFiles();

    unsigned long nRecords = nRecordsList[rank];
    unsigned int  fileNumber = fileNumberList[rank];
    unsigned long fileOffset = fileOffsetList[rank];

    DataRecords retval;
    retval.nRecords = nRecords;
    retval.data = new char[nRecords*lRec];

    //
    // Open the first file to read and position the file pointer at the
    // correct spot.
    //
    char string[1024];
    SNPRINTF(string, 1024, "%s/%s#%6.6u", fname.c_str(), subname.c_str(), fileNumber);
    debug1 << "Opening " << string << endl;
    FILE *file = fopen(string, "rb");
    if (file == NULL)
    {
        EXCEPTION1(InvalidFilesException, string);
    }
    fseek(file, fileOffset, SEEK_SET);
    debug1 << "fileOffset=" << fileOffset << ",nRecords=" << nRecords << endl;

    //
    // Read this processors chunk of the data.
    //
    char *buffer = retval.data;
    while (nRecords > 0)
    {
        if (feof(file))
        {
            fclose(file);
            fileNumber++;
            if (fileNumber < nFiles)
            {
                SNPRINTF(string, 1024, "%s/%s#%6.6u", fname.c_str(), subname.c_str(),
                        fileNumber);
                debug1 << "Opening " << string << endl;
                file = fopen(string, "rb");
                if (file == NULL)
                {
                    EXCEPTION1(InvalidFilesException, string);
                }
            }
        }
        size_t cnt = fread(buffer, lRec, nRecords, file);
        debug1 << "Reading " << nRecords << " records, read " << cnt << " records" << endl;
        nRecords -= cnt;
        buffer += cnt * lRec;
    } 

    fclose(file);

    return retval;
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
//    Brad Whitlock, Thu Dec  6 10:50:04 PST 2012
//    Use unsigned long to get the total file size right when the size is
//    really large.
//
// ****************************************************************************

void
avtDDCMDFileFormat::DetermineProcessorReadOffset(const DDCMDHeader *header,
    const std::string &subname)
{
#ifdef PARALLEL
    unsigned int nProcs = PAR_Size();
#else
    unsigned int nProcs = 1;
#endif

    unsigned long headerLength = header->GetHeaderLength();
    unsigned long lRec         = header->GetLRec();
    unsigned int nFiles        = header->GetNFiles();

    //
    // Determine the size of each file.
    //
    unsigned long *fileSizeList = new unsigned long[nFiles];
    unsigned long fileSize = 0;
    for (unsigned int i = 0; i < nFiles; i++)
    {
        char          string[1024];
        FileFunctions::VisItStat_t   statbuf;

        sprintf(string, "%s/%s#%6.6u", fname.c_str(), subname.c_str(), i);

        FileFunctions::VisItStat(string, &statbuf);
        fileSizeList[i] = (unsigned long)(statbuf.st_size); 
        fileSize += fileSizeList[i];
    } 
    fileSize -= headerLength;

    debug1 << "fileSizes=";
    for (unsigned int i = 0; i < nFiles; i++)
    {
        debug1 << fileSizeList[i] << ",";
    }
    debug1 << endl;

    //
    // Allocate space for file offset information for each processor.
    //
    nRecordsList   = new unsigned long[nProcs];
    fileNumberList = new unsigned int[nProcs];
    fileOffsetList = new unsigned long[nProcs];

    unsigned long nRecordsTotal = fileSize / lRec;
    unsigned long nRecordsPerProc = (nRecordsTotal + nProcs - 1) / nProcs;

    unsigned int iFile = 0;
    unsigned long fileOffset = headerLength;
    for (unsigned int i = 0; i < nProcs; i++)
    {
        unsigned long nRec = nRecordsPerProc < nRecordsTotal ?
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

    for (unsigned int i = 0; i < nProcs; i++)
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
//    Brad Whitlock, Mon Oct  8 15:48:07 PDT 2012
//    Use stringVector and print error consistency messages to debug5.
//
// ****************************************************************************

vector<DDCMDHeader*>
avtDDCMDFileFormat::ReadHeader()
{
    const char *mName = "avtDDCMDFileFormat::ReadHeader: ";

    //
    // Read the headers.
    //
    vector<DDCMDHeader*> headers;
    for (size_t i = 0; i < subnames.size(); i++)
        headers.push_back(new DDCMDHeader(fname, subnames[i]));

    //
    // Determine if it is a cgrid file.
    //
    cgridFile = false;
    for (size_t i = 0; i < subnames.size(); i++)
        cgridFile = cgridFile || headers[i]->GetCGridFile();

    //
    // Determine if the file is a legacy file. If any file is NOT a legacy file
    // then treat none as legacy files.
    //
    isLegacy = true;
    for (size_t i = 0; i < subnames.size(); i++)
        isLegacy &= headers[i]->IsLegacy();

    //
    // Check that the files are cnonsistent.
    //
    bool error = false;

    if(isLegacy)
    {
        // just do this check for legacy files.
        if (cgridFile && subnames.size() > 1)
        {
            debug5 << mName << "cgridFile and subnames.size() > 1" << endl;
            error = true;
        }
    }

    for (size_t i = 1; i < subnames.size(); i++)
    {
        if(headers[0]->GetNRecord() != headers[i]->GetNRecord())
        {
            debug5 << mName << "Header 0, " << i << " NRecord values are not equal" << endl;
            error = true;
        }
        if(headers[0]->GetSwap() != headers[i]->GetSwap())
        {
            debug5 << mName << "Header 0, " << i << " Swap values are not equal" << endl;
            error = true;
        }
        if(headers[0]->GetHMatrix()[0] != headers[i]->GetHMatrix()[0])
        {
            debug5 << mName << "Header 0, " << i << " HMatrix[0] values are not equal" << endl;
            error = true;
        }
        if(headers[0]->GetHMatrix()[4] != headers[i]->GetHMatrix()[4])
        {
            debug5 << mName << "Header 0, " << i << " HMatrix[4] values are not equal" << endl;
            error = true;
        }
        if(headers[0]->GetHMatrix()[8] != headers[i]->GetHMatrix()[8])
        {
            debug5 << mName << "Header 0, " << i << " HMatrix[8] values are not equal" << endl;
            error = true;
        }
        if(headers[0]->GetLoop() != headers[i]->GetLoop())
        {
            debug5 << mName << "Header 0, " << i << " Loop values are not equal" << endl;
            error = true;
        }
        if(headers[0]->GetTime() != headers[i]->GetTime())
        {
            debug5 << mName << "Header 0, " << i << " Time values are not equal" << endl;
            error = true;
        }
    }

    //
    // If we had an error, return no headers.
    //
    if (error)
    {
        for (size_t i = 0; i < headers.size(); i++)
            delete headers[i];
        headers.clear();
        debug5 << mName << "Returning empty headers vector." << endl;
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
    for (size_t i = 0; i < headers.size(); i++)
    {
        if (headers[i]->GetNSpecies() > 0 && nSpecies == 0)
        {
            nSpecies = headers[i]->GetNSpecies();
            speciesNames = headers[i]->GetSpeciesNames();
        }
    }
    nGroups = 0;
    for (size_t i = 0; i < headers.size(); i++)
    {
        if (headers[i]->GetNGroups() > 0 && nGroups == 0)
        {
            nGroups = headers[i]->GetNGroups();
            groupNames = headers[i]->GetGroupNames();
        }
    }
    nTypes = 0;
    for (size_t i = 0; i < headers.size(); i++)
    {
        if (headers[i]->GetNTypes() > 0 && nTypes == 0)
        {
            nTypes = headers[i]->GetNTypes();
            typeNames = headers[i]->GetTypeNames();
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
//  Modifications:
//    Brad Whitlock, Mon Oct  8 15:51:41 PDT 2012
//    use stringVector. Add checks for legacy files. Add units.
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
    const stringVector &fieldNames = header->GetFieldNames();
    const stringVector &fieldTypes = header->GetFieldTypes();
    const stringVector &fieldUnits = header->GetFieldUnits();
    unsigned int  *fieldSizes = header->GetFieldSizes();

    //
    // Determine the variables to plot.
    //
    int           offset = 0;
    
    labelOffset = -1;
    iSpeciesOffset = -1;
    varNames.clear();
    varUnits.clear();
    varOffsets = new int[nFields];
    varSizes   = new int[nFields];
    varTypes   = new char[nFields];
    nVars = 0;

    for (unsigned int i = 0; i < nFields; i++)
    {
        if (fieldNames[i] != std::string("checksum"))
        {
            if (fieldNames[i] == std::string("label"))
            {
                labelOffset   = offset;
                labelSize     = fieldSizes[i];
                labelUnsigned = fieldTypes[i][0] == 'u' ? true : false;
            }
            else if (fieldNames[i] == std::string("species_index"))
            {
                iSpeciesOffset   = offset;
                iSpeciesSize     = fieldSizes[i];
                iSpeciesUnsigned = fieldTypes[i][0] == 'u' ? true : false;
            }
            else
            {
                varNames.push_back(fieldNames[i]);
                varOffsets[nVars] = offset;
                varSizes[nVars]   = fieldSizes[i];
                varTypes[nVars]   = fieldTypes[i][0];
                std::string units("1");
                if((unsigned int)i < fieldUnits.size())
                    units = fieldUnits[i];
                varUnits.push_back(units);
                
                nVars++;
            }
        }
        offset += fieldSizes[i];
    }
    if(isLegacy)
    {
        // Just do this check for legacy files.
        if (labelOffset == -1 || iSpeciesOffset == -1)
        {
            EXCEPTION1(InvalidFilesException, fname.c_str());
        }
    }
    for (int i = 0 ; i < nVars; i++)
    {
        debug1 << "varName=" << varNames[i] << ",offset=" << varOffsets[i]
               << ",size=" << varSizes[i] << ",type=" << varTypes[i]
               << ", units=" << varUnits[i] << endl;
    }

    //
    // Determine the mesh information.
    //
    xMin = yMin = zMin = 0.;
    float xExtent = hMatrix[0];
    float yExtent = hMatrix[4];
    float zExtent = hMatrix[8];
    coordsUnit = "m"; // what's the right default unit if none has been given?
    if(isLegacy)
    {
        // Make some assumptions for legacy files.

        coordsUnit = "nm";
        xExtent /= 10.; // now in nm
        yExtent /= 10.; // now in nm
        zExtent /= 10.; // now in nm
        if (max(xExtent, max(yExtent, zExtent)) > 1e3)
        {
            xExtent /= 1e3;
            yExtent /= 1e3;
            zExtent /= 1e3;
            coordsUnit = "um";
        }
    }

    //
    // If nX is 1, then we have a 2d mesh.  Map y to x and z to y.
    //
    if (nXFile == 1)
    {
        nDims = 2;
        if(isLegacy)
        {
            xMin = -(yExtent / 2.0);
            yMin = -(zExtent / 2.0);
            zMin = -(xExtent / 2.0);
        }
        dX   =   yExtent / (1.0 * nYFile);
        dY   =   zExtent / (1.0 * nZFile);
        dZ   =   xExtent / (1.0 * nXFile);
        nXMesh = nYFile;
        nYMesh = nZFile;
        nZMesh = 1;
    }
    else
    {
        nDims = 3;
        if(isLegacy)
        {
            xMin = -(xExtent / 2.0);
            yMin = -(yExtent / 2.0);
            zMin = -(zExtent / 2.0);
        }
        dX   =   xExtent / (1.0 * nXFile);
        dY   =   yExtent / (1.0 * nYFile);
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
//    Brad Whitlock, Thu Oct 11 15:43:47 PDT 2012
//    Add units.
//
// ****************************************************************************

void
avtDDCMDFileFormat::ParseAtomHeader(const DDCMDHeader *header)
{
    unsigned int   nFields    = header->GetNFields();
    const stringVector &fieldNames = header->GetFieldNames();
    const stringVector &fieldTypes = header->GetFieldTypes();
    const stringVector &fieldUnits = header->GetFieldUnits();
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
    varNames.clear();
    varUnits.clear();
    varOffsets = new int[nFields];
    varSizes   = new int[nFields];
    varTypes   = new char[nFields];
    nVars = 0;

    int offset = 0;

    for (unsigned int i = 0; i < header->GetNFields(); i++)
    {
        if (fieldNames[i] != std::string("checksum"))
        {
            if (fieldNames[i] == std::string("class"))
            {
                // Class maps to type.
                typeOffset = offset;
            }
            else if (fieldNames[i] == std::string("type"))
            {
                // Type maps to species.
                speciesOffset = offset;
            }
            else if (fieldNames[i] == std::string("group"))
            {
                // Group maps to group.
                groupOffset = offset;
            }
            else if (fieldNames[i] == std::string("pinfo"))
            {
                pinfoOffset = offset;
                pinfoSize   = fieldSizes[i];
                pinfoType   = fieldTypes[i][0];
            }
            else if (fieldNames[i] == std::string("rx"))
            {
                xOffset = offset;
                xSize   = fieldSizes[i];
                xType   = fieldTypes[i][0];
            }
            else if (fieldNames[i] == std::string("ry"))
            {
                yOffset = offset;
                ySize   = fieldSizes[i];
                yType   = fieldTypes[i][0];
            }
            else if (fieldNames[i] == std::string("rz"))
            {
                zOffset = offset;
                zSize   = fieldSizes[i];
                zType   = fieldTypes[i][0];
            }
            else
            {
                varNames.push_back(fieldNames[i]);
                varOffsets[nVars] = offset;
                varSizes[nVars]   = fieldSizes[i];
                varTypes[nVars]   = fieldTypes[i][0];
                std::string units("1");
                if((unsigned int)i < fieldUnits.size())
                    units = fieldUnits[i];
                varUnits.push_back(units);
                nVars++;
            }
        }
        offset += fieldSizes[i];
    }

    for (int i = 0 ; i < nVars; i++)
    {
        debug1 << "varName=" << varNames[i] << ",offset=" << varOffsets[i]
               << ",size=" << varSizes[i] << ",type=" << varTypes[i]
               << ",units=" << varUnits[i] << endl;
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
//  Modifications:
//    Brad Whitlock, Thu Oct 11 14:57:03 PDT 2012
//    Get and pass data records as arguments rather than class members.
//
// ****************************************************************************

void
avtDDCMDFileFormat::ReadCGridData(const DDCMDHeader *header,
    const std::string &subname)
{
#ifdef MDSERVER
    return;
#else
    DetermineProcessorReadOffset(header, subname);

    DetermineBlockDecomposition();

    DataRecords data = ReadProcessorChunk(header, subname);

    DataRecords thisProcData = ExchangeProcessorData(header, data);

    // If the output of ExchangeProcessorData is not the input then we can free
    // the input.
    if(thisProcData.data != data.data)
        delete [] data.data;

    CopyExchangeDataToBlocks(header, thisProcData);

    delete [] thisProcData.data;

    for (int i = 0; i < nVars; i++)
    {
        varsBlock[nVarsBlock+i] = varValues[i];
    }

    delete [] varValues;
    varValues = 0;
#endif
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
//  Modifications:
//    Brad Whitlock, Thu Oct 11 13:18:02 PDT 2012
//    Pass the data that we operate on.
//
// ****************************************************************************

void
avtDDCMDFileFormat::ReadAtomData(const DDCMDHeader *header,
    const std::string &subname)
{
#ifdef MDSERVER
    return;
#else
    DetermineProcessorReadOffset(header, subname);

    DataRecords data = ReadProcessorChunk(header, subname);

    CopyDataToBlocks(header, data);

    delete [] data.data;

    for (int i = 0; i < nVars; i++)
    {
        varsBlock[nVarsBlock+i] = varValues[i];
    }

    delete [] varValues;
    varValues = 0;
#endif
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
//  Modifications:
//    Brad Whitlock, Mon Oct  8 16:08:44 PDT 2012
//    Support cgrid-style files that have multiple subnames. Add units.
//
// ****************************************************************************

void
avtDDCMDFileFormat::ReadData(vector<DDCMDHeader*> &headers)
{
    int nFields = 0;
    for (size_t i = 0; i < headers.size(); i++)
        nFields += headers[i]->GetNFields();

    nVarsBlock = 0;
    varNamesBlock.clear();
    varUnitsBlock.clear();
#ifndef MDSERVER
    varsBlock = new float*[nFields];
#endif

    if (cgridFile)
    {
//        for (int i = 0; i < headers.size(); i++)
//            headers[i]->Print(cout);

        for (size_t j = 0; j < subnames.size(); j++)
        {
            ParseCGridHeader(headers[j]);

            ReadCGridData(headers[j], subnames[j]);

            for (int i = 0; i < nVars; i++)
            {
                if(subnames.size() > 1)
                    varNamesBlock.push_back(subnames[j] + "/" + varNames[i]);
                else
                    varNamesBlock.push_back(varNames[i]);
                varUnitsBlock.push_back(varUnits[i]);
                nVarsBlock++;
            }

            varNames.clear();
            varUnits.clear();
        }
    }
    else
    {
        for (size_t j = 0; j < subnames.size(); j++)
        {
            ParseAtomHeader(headers[j]);

            ReadAtomData(headers[j], subnames[j]);

            for (int i = 0; i < nVars; i++)
            {
                varNamesBlock.push_back(varNames[i]);
                varUnitsBlock.push_back(varUnits[i]);
                nVarsBlock++;
            }

            varNames.clear();
            varUnits.clear();
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
//  Modifications:
//    Brad Whitlock, Fri Nov 16 15:19:18 PST 2012
//    We've already built the points in the coordBlocks object. Give the
//    coordBlocks object to the unstructured grid. Doing this will increase 
//    the reference count. We don't want the unstructured grid to own the
//    points since we'll keep a reference here so do not delete the coordBlocks.
//
// ****************************************************************************

vtkDataSet *
avtDDCMDFileFormat::GetPointMesh()
{
    //
    // Create the VTK objects and connect them up.
    //
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(this->coordsBlock);
    ugrid->Allocate(nPoints);
    vtkIdType onevertex[1];
    for (unsigned long i = 0 ; i < nPoints; i++)
    {
        onevertex[0] = i;
        ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
    }

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
//  Modifications:
//    Eric Brugger, Thu Mar 12 14:25:26 PDT 2009
//    I modified the routine so that in the case of a 2d mesh it would
//    return 1 for the z dimension instead of 2, which it was erroneously
//    doing before.
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
        iXMax = iXMin + deltaY < (int)nYFile ? iXMin + deltaY : (int)nYFile;

        iYMin = iZBlock * deltaZ;
        iYMax = iYMin + deltaZ < (int)nZFile ? iYMin + deltaZ : (int)nZFile;

        iZMin = 0;
        iZMax = 0;
    }
    else
    {
        iXMin = iXBlock * deltaX;
        iXMax = iXMin + deltaX < (int)nXFile ? iXMin + deltaX : (int)nXFile;

        iYMin = iYBlock * deltaY;
        iYMax = iYMin + deltaY < (int)nYFile ? iYMin + deltaY : (int)nYFile;

        iZMin = iZBlock * deltaZ;
        iZMax = iZMin + deltaZ < (int)nZFile ? iZMin + deltaZ : (int)nZFile;
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
//  Modifications:
//    Brad Whitlock, Fri Nov 16 15:39:31 PST 2012
//    Compute derived values directly in a vtkFloatArray. For variables that
//    we read from the file, wrap them in a vtkFloatArray instead of copying.
//
// ****************************************************************************

vtkDataArray *
avtDDCMDFileFormat::GetPointVar(const char *varname)
{
    std::string var(varname);

    vtkFloatArray *rv = NULL;
    if (var == "species" || var == "group" || var == "type")
    {
        rv = vtkFloatArray::New();
        rv->SetNumberOfTuples(nPoints);
        for (unsigned long i = 0; i < nPoints; i++)
        {
            unsigned pinfo = pinfoBlock[i];
            int ig = pinfo % nGroups;
            pinfo /= nGroups;
            int is = pinfo % nSpecies;
            pinfo /= nSpecies;
            int it = pinfo;
            if (var == "species")
                rv->SetValue(i, float(is));
            else if (var == "group")
                rv->SetValue(i, float(ig));
            else
                rv->SetValue(i, float(it));
        }
    }
    else
    {
        //
        // Get the index of the variable name.
        //
        unsigned int i;
        for (i = 0; i < nVarsBlock && varNamesBlock[i] != varname; i++)
            /* Do nothing. */;
        if (i== nVarsBlock)
            EXCEPTION1(InvalidVariableException, varname);

        // Wrap the existing data as a VTK array. The 1 in SetArray tells VTK
        // not to delete the data.
        rv = vtkFloatArray::New();
        rv->SetNumberOfComponents(1);
        rv->SetArray(varsBlock[i], nPoints, 1);
    }

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
//  Modifications:
//    Brad Whitlock, Tue Oct  9 14:32:21 PDT 2012
//    Don't assume that underscore means that there is a species in the name.
//
// ****************************************************************************

vtkDataArray *
avtDDCMDFileFormat::GetRectilinearVar(const char *varname)
{
    std::string var(varname);

    //
    // Determine the variable name and the species name.
    //
    std::string varBase(var), varSpec;
    std::string::size_type pos = var.rfind("_");
    if(pos != std::string::npos && nSpecies > 0)
    {
        varBase = var.substr(0, pos);
        varSpec = var.substr(pos+1, var.size() - pos - 1);
    }

    //
    // Get the index of the variable name and the index of the species name.
    //
    unsigned int iVar;
    for (iVar = 0; iVar < nVarsBlock; iVar++)
    {
        if(varNamesBlock[iVar] == varBase)
        {
            debug5 << "avtDDCMDFileFormat::GetRectilinearVar: found "
                   << varname << " at index "
                   << iVar << " in varNamesBlock" << endl;
            break;
        }
    }
    if (iVar == nVarsBlock)
        EXCEPTION1(InvalidVariableException, varname);

    unsigned int iSpec;
    for (iSpec = 0; iSpec < nSpecies && speciesNames[iSpec] != varSpec; iSpec++)
        /* Do nothing. */;
    if (nSpecies > 0 && iSpec == nSpecies)
        EXCEPTION1(InvalidVariableException, varname);

    //
    // Copy the data array.
    //
    vtkFloatArray *rv = vtkFloatArray::New();

    unsigned long ntuples = nZonesBlock;
    rv->SetNumberOfTuples(ntuples);

    float *data = varsBlock[iVar] + iSpec * ntuples;
    vtkIdType i = 0;
    for (unsigned int iZ = 0; iZ < nZBlock; iZ++)
    {
        for (unsigned int iY = 0; iY < nYBlock; iY++)
        {
            for (unsigned int iX = 0; iX < nXBlock; iX++)
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
//    Jeremy Meredith, Thu Jan  7 12:28:16 EST 2010
//    Check some array bounds.  Make sure we can open the file.
//
//    Brad Whitlock, Mon Oct  8 16:57:59 PDT 2012
//    I added more member intialization.
//
// ****************************************************************************

avtDDCMDFileFormat::avtDDCMDFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    //
    // Initialization
    //
    nXFileBlocks = nYFileBlocks = nZFileBlocks = nBlocks = 0;

    // Global header information
    cgridFile = false;
    isLegacy = true;
    loop = 0;
    time = 0.;
    nXFile = nYFile = nZFile = 0;
    nSpecies = 0;
    nGroups = 0;
    nTypes = 0;

    // Mesh information
    coordsUnit = std::string("m");
    nDims = 0;
    nXMesh = nYMesh = nZMesh = 0;
    nXMeshBlocks = nYMeshBlocks = nZMeshBlocks = 0;
    xMin = yMin = zMin = 0.f;
    dX = dY = dZ = 0.f;
    coordsScale = 1.f;
    nPoints = 0;

    // Block information
    nXBlock = nYBlock = nZBlock = 0;
    nZonesBlock = 0;
    nVarsBlock = 0;
    varsBlock = NULL;
    coordsBlock = NULL;
    pinfoBlock = NULL;

    // Variable information
    labelOffset = iSpeciesOffset = 0;
    labelSize = iSpeciesSize = 0;
    labelUnsigned = iSpeciesUnsigned = false;
    groupOffset = speciesOffset = typeOffset = 0;
    pinfoOffset = xOffset = yOffset = zOffset = 0;
    pinfoSize = xSize = ySize = zSize = 0;
    pinfoType = xType = yType = zType = 0;
    nVars = 0;
    varValues = NULL;
    varOffsets = NULL;
    varSizes = NULL;
    varTypes = NULL;

    // File information
    dataRead = false;
    nRecordsList = NULL;
    fileNumberList = NULL;
    fileOffsetList = NULL;

    //
    // Store the directory name with the data files.
    //
    string name(filename);
    if (name.length() < 6)
        EXCEPTION2(InvalidFilesException, filename,
                   "Filename not at least 6 characters.");

    fname = name.substr(0,name.length()-6);

    //
    // Read the file to determine the sub file names to read in the
    // directory.  If the file doesn't contain any names assume it is
    // a cgrid file.  This is to maintain backward compatability, where
    // only cgrid files were supported and the file was a zero length
    // file.
    //
    int       maxlen, lbuf=0;
    char      str[1024], *buf, *b;

    FILE *file = fopen(filename, "r");
    if (!file)
        EXCEPTION1(InvalidFilesException, filename);

    str[0] = '\0';
    maxlen = 1024;
    buf = str;
    b = str;
    while (b != NULL && lbuf < 1023)
    {
        lbuf = (int)strlen(buf);
        b = fgets(buf + lbuf, maxlen - lbuf, file);
    }
    if (lbuf >= 1023)
        EXCEPTION2(InvalidFilesException, filename,
                   "First line was more than 1023 bytes.");

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

        object_free(obj);
    }
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
//    Brad Whitlock, Fri Nov 16 15:25:10 PST 2012
//    Delete the coordsBlock.
//
// ****************************************************************************

void
avtDDCMDFileFormat::FreeUpResources(void)
{
    //
    // Only free the block variables in the engine. The pointer will
    // be NULL on the mdserver.
    //
    if (varsBlock != NULL)
    {
        for (unsigned int i = 0; i < nVarsBlock; i++)
        {
            delete [] varsBlock[i];
        }
        delete [] varsBlock;
        varsBlock = NULL;
    }

    if(coordsBlock != NULL)
    {
        coordsBlock->Delete();
        coordsBlock = NULL;
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
    if(!headers.empty())
        ReadData(headers);

    //
    // Free the data.
    //
    for (size_t i = 0; i < headers.size(); i++)
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
//    Eric Brugger, Fri Mar 13 17:07:47 PDT 2009
//    I added rectilinear domain boundaries so that ghost zones would be
//    automatically added when necessary.
//
//    Brad Whitlock, Tue Oct  9 15:19:12 PDT 2012
//    Add mesh units, variables on cgrid files, and guard against no species
//    for cgrid files.
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

        avtMeshMetaData *mmd = new avtMeshMetaData;
        mmd->name = meshname;
        mmd->meshType = AVT_RECTILINEAR_MESH;
        mmd->numBlocks = 1;
        mmd->blockOrigin = 0;
        mmd->cellOrigin = 0;
        mmd->spatialDimension = nDims;
        mmd->topologicalDimension = nDims;
        mmd->blockTitle = "blocks";
        mmd->blockPieceName = "block";
        mmd->xUnits = coordsUnit;
        mmd->yUnits = coordsUnit;
        mmd->zUnits = coordsUnit;
        md->Add(mmd);

        md->SetFormatCanDoDomainDecomposition(true);

        //
        // Create the appropriate domain boundary information.
        //
        if (!avtDatabase::OnlyServeUpMetaData())
        {
            avtRectilinearDomainBoundaries *rdb =
                new avtRectilinearDomainBoundaries(true);

            rdb->SetNumDomains(nBlocks);
            for (int j = 0; j < nBlocks; j++)
            {
                int extents[6];

                int deltaX = (nXFile + nXFileBlocks - 1) / nXFileBlocks;
                int deltaY = (nYFile + nYFileBlocks - 1) / nYFileBlocks;
                int deltaZ = (nZFile + nZFileBlocks - 1) / nZFileBlocks;

                int iBlock = j;
                int iXBlock = iBlock / (nYFileBlocks * nZFileBlocks);
                iBlock %= (nYFileBlocks * nZFileBlocks);
                int iYBlock = iBlock / nZFileBlocks;
                int iZBlock = iBlock % nZFileBlocks;

                int iXMin, iXMax, iYMin, iYMax, iZMin, iZMax;
                if (nDims == 2)
                {
                    iXMin = iYBlock * deltaY;
                    iXMax = iXMin + deltaY < (int)nYFile ? iXMin + deltaY : (int)nYFile;

                    iYMin = iZBlock * deltaZ;
                    iYMax = iYMin + deltaZ < (int)nZFile ? iYMin + deltaZ : (int)nZFile;

                    iZMin = 0;
                    iZMax = 0;
                }
                else
                {
                    iXMin = iXBlock * deltaX;
                    iXMax = iXMin + deltaX < (int)nXFile ? iXMin + deltaX : (int)nXFile;

                    iYMin = iYBlock * deltaY;
                    iYMax = iYMin + deltaY < (int)nYFile ? iYMin + deltaY : (int)nYFile;

                    iZMin = iZBlock * deltaZ;
                    iZMax = iZMin + deltaZ < (int)nZFile ? iZMin + deltaZ : (int)nZFile;
                }

                extents[0] = iXMin;
                extents[1] = iXMax;
                extents[2] = iYMin;
                extents[3] = iYMax;
                extents[4] = iZMin;
                extents[5] = iZMax;

                rdb->SetIndicesForRectGrid(j, extents);
            }
            rdb->CalculateBoundaries();

            void_ref_ptr vr = void_ref_ptr(rdb,
                avtStructuredDomainBoundaries::Destruct);
            cache->CacheVoidRef("any_mesh",
                AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1, vr);
        }

        //
        // Set the variable information.
        //
        Expression expr;
        for (unsigned int i = 0; i < nVarsBlock; i++)
        {
            // For non-legacy files, add the real variable name too.
            if(!isLegacy)
            {
                avtScalarMetaData *smd = new avtScalarMetaData();
                smd->name = varNamesBlock[i];
                smd->meshName = meshname;
                smd->centering = AVT_ZONECENT;
                smd->hasDataExtents = false;
                smd->hasUnits = varUnitsBlock[i] != "1";
                if(smd->hasUnits)
                    smd->units = varUnitsBlock[i];
                md->Add(smd);
            }

            if(nSpecies > 0)
            {
                for (unsigned int j = 0; j < nSpecies; j++)
                {
                    string name = varNamesBlock[i] + "_" + string(speciesNames[j]);
                    AddScalarVarToMetaData(md, name, meshname, AVT_ZONECENT);
                }

                expr.SetName(varNamesBlock[i]);
                string name = varNamesBlock[i] + "_" + string(speciesNames[0]);
                for (unsigned int j = 1; j < nSpecies; j++)
                {
                    name += " + " + varNamesBlock[i] + "_" + string(speciesNames[j]);
                }
                expr.SetDefinition(name);
                expr.SetType(Expression::ScalarMeshVar);
                md->AddExpression(&expr);
            }
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
        /*avtMeshMetaData *mmd = md->GetMesh(meshname);
        if(mmd != NULL)
        {
            mmd->xUnits = coordsUnit;
            mmd->yUnits = coordsUnit;
            mmd->zUnits = coordsUnit;
        }*/

        md->SetFormatCanDoDomainDecomposition(true);

        //
        // Set the variable information.
        //
        for (unsigned int i = 0; i < nVarsBlock; i++)
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
//    Eric Brugger, Wed Nov  3 13:44:17 PDT 2010
//    I added a check on the domain number matching the one in GetVar.
//
// ****************************************************************************

vtkDataSet *
avtDDCMDFileFormat::GetMesh(int domain, const char *meshname)
{
#ifdef PARALLEL
    if (domain != PAR_Rank())
#else
    if (domain != 0)
#endif
        EXCEPTION1(InvalidVariableException, meshname);

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
//    Eric Brugger, Wed Nov  3 13:44:17 PDT 2010
//    I changed the check on the domain number to check that it matched the
//    parallel rank when running in parallel instead of always expecting it
//    to be zero.
//
// ****************************************************************************

vtkDataArray *
avtDDCMDFileFormat::GetVar(int domain, const char *varname)
{
#ifdef PARALLEL
    if (domain != PAR_Rank())
#else
    if (domain != 0)
#endif
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
