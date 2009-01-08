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
//                           avtNek5000FileFormat.C                          //
// ************************************************************************* //

#include <avtNek5000FileFormat.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif
#include <float.h>

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtDatabase.h>
#include <avtIntervalTree.h>
#include <avtIsolevelsSelection.h>
#include <avtNekDomainBoundaries.h>
#include <avtParallel.h>
#include <avtPlaneSelection.h>
#include <avtSpatialBoxSelection.h>
#include <avtVariableCache.h>
#include <snprintf.h>

#include <Expression.h>

#include <DebugStream.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h>

#include <TimingsManager.h>
#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

#define USE_SIMPLE_BLOCK_NUMBERING 1

using     std::string;
#ifndef STREQUAL
#if defined(_WIN32) 
#  define STREQUAL(a,b)              stricmp(a,b)
#else
#  define STREQUAL(a,b)              strcasecmp(a,b)
#endif
#endif

#ifdef WIN32
typedef __int64 int64_t;
#else
#include <inttypes.h>
#endif


// ****************************************************************************
// Notes on the format of each Nek variant--binary/ascii, 3D/2D, serial/parallel
//
// There's one data file per timestep for serial writes, several per timestep for 
// parallel writes.  Usually it has the .fld extension.  And there's a text
// metadata file whose format I made up, which has a template for the filenames,
// first and number of timesteps, etc.
//
// This is the layout of a 3D, binary, serial .fld file.
// 80 char ascii header:
//   num_blocks  block_dim_x  block_dim_y  block_dim_z  time  cycle  var_codes
//
//   The var codes are single characters that indicate the presence or absence
//   of known variables.
//   X Y Z U P T 1 2 3 4   indicates a mesh (X Y Z), velocity vec (U), pressure,
//                         temperature, and 4 misc scalars
//   A space in place of a letter code means that variable is absent.
//   The variables always are written into the file in this order.
//
// After the ascii header the number 6.54321 is written in binary, and can be
// read back to determine endianness of the data.
//
// After this comes the bulk of the data, written as 4-byte floats, tight packed
// with no padding at all.
// for each block
//     for each variable that is present out of X,Y,Z,U,V,W,P,T
//         for each sample in the block
//             write the sample
//
// --------
// If the data is 2D, the layout is the same, except the mesh points and velocity
// have 2 components instead of 3.  In the .fld header, there are still 3 components
// for the block dim, and the third is 1.
//
// --------
// If the data is ascii, the 80-char header is the same, but the 4-byte endian
// tag is not written.  Following the header, one float is written per block,
// but it's info is not used (all zeros in my test examples).
//
// Next comes the bulk of the data:
// for each block
//     for each sample in the block
//         for each variable that is present out of X,Y,Z,U,V,W,P,T
//             write the sample, using 14 bytes including a leading space
//         write a newline character
//
// Because each line takes the same amount of space, the reader has random-access
// to blocks in the file.
//
// --------
// If the data was written in parallel
// The header becomes 132 chars.  Here are three example headers:
// #std 4  6  6  6   120  240  0.1500E+01  300  1  2XUPT
// #std 4  6  6  6   120  240  0.1500E+01  300  1  2 XUPTS03
// #std 4  6  6  6   120  240  0.1500E+01  300  1  2 U T123
// This example means:  #std is for versioning, 4 bytes per sample (could be 8, 
//   for double precision), 6x6x6 blocks, 120 of 240 blocks are in this file, 
//   time=1.5, cycle=300, this output dir=1, num output dirs=2, XUPT123 are 
//   tags that this file has a mesh, velocity, pressure, temperature, and 
//   3 misc scalars.
// 
// Next comes the 4-byte 6.54321 for endian determination.
// Next comes one 4-byte integer per block in the file, which is the global id
// of that block.
// Next the bulk of the data:
//
// if a mesh is present:
// for each block
//     for each variable X,Y,Z
//         for each sample in the block
//             write the sample
//
// if velocity is present:
// for each block
//     for each variable U,V,W
//         for each sample in the block
//             write the sample
//
// for each variable in P, T, and any misc scalars
//     for each block
//         for each sample in the block
//             write the sample
//
//
// ****************************************************************************
// Example of a metadata input file
//
// NEK3D
// version: 1.0
// filetemplate: example.fld%02d  
// firsttimestep: 2                 # first dump here: example.fld02
// numtimesteps: 3                  # number of dumps
// type: binary                     # ascii or binary
//
// 
// NEK3D
// version: 1.0
// filetemplate: example%02d.f%04d  
// firsttimestep: 2                 # first dump here: example??.f0002
// numtimesteps: 3                  # number of dumps
// type: binary                     # ascii or binary
//
//
// NOTE: if the number of printf tokens is > 1 than the multiple fld format
//       is used.
// ****************************************************************************









// ****************************************************************************
//  Method: avtNek5000 constructor
//
//  Programmer: dbremer -- generated by xml2avt
//  Creation:   Fri May 18 16:07:09 PST 2007
//
//  Modifications:
//    Dave Bremer, Tue Sep 11 15:45:51 PDT 2007
//    Added a small mod to support variable-length headers 
//    and absolute paths to data files in the file template.
//
//    Dave Bremer, Wed Nov  7 14:17:19 PST 2007
//    Added support for 2D and ascii files, deprecated the
//    meshcoords tag, and added support for misc scalar fields
//    in addition to pressure and temperature.
//
//    Dave Bremer, Wed Nov 14 15:00:13 PST 2007
//    Added support for the parallel version of the file, added
//    support for comments in the file, and deprecated the endian flag.
//
//    Dave Bremer, Thu Nov 15 16:44:42 PST 2007
//    Added a small fix for the case in which there are more than 9 output dirs
//
//    Dave Bremer, Wed Feb  6 19:12:55 PST 2008
//    Refactored the constructor, moving some functionality into other methods, 
//    and deferring some significant computation. I also rewrote the 
//    constructor for the parallel case, to make one process read the data and
//    broadcast it out, thinking it would help with performance.  It doesn't 
//    make much difference in practice, so the code could be changed to follow 
//    the serial path in all cases.
//
//    Dave Bremer, Fri Jun  6 15:38:45 PDT 2008
//    Added the bParFormat flag allowing the parallel format to be used
//    by a serial code, in which there is only one output dir.
//
//    Dave Bremer, Thu Jun 12 12:59:23 PDT 2008
//    Support varying numbers of blocks per file in the parallel format.
//    Removed the code to read on one proc and broadcast the header, rather
//    than having all procs read it, because it made no performance difference.
//
//    Dave Bremer, Fri Aug  8 12:57:47 PDT 2008
//    Brought in a change from Stefan Kerkemeier to correct an error message.
//
//    Dave Bremer, Mon Aug 11 13:53:18 PDT 2008
//    Brought in a change from Stefan Kerkemeier to change default init values.
// ****************************************************************************

avtNek5000FileFormat::avtNek5000FileFormat(const char *filename)
    : avtMTMDFileFormat(filename)
{
    int t0 = visitTimer->StartTimer();

    version = "";
    bSwapEndian = false;
    fileTemplate = "";
    iFirstTimestep = 1;
    iNumTimesteps = 1;
    bBinary = false;
    iNumOutputDirs = 0;
    bParFormat = false;

    iNumBlocks = 0;
    iBlockSize[0] = 1;
    iBlockSize[1] = 1;
    iBlockSize[2] = 1;

    bHasVelocity = false;
    bHasPressure = false;
    bHasTemperature = false;
    iNumSFields = 0;

    fdMesh = NULL;
    fdVar = NULL;
    iCurrTimestep = -999;
    iCurrMeshProc = -999;
    iCurrVarProc  = -999;
    iAsciiMeshFileStart = -999;
    iAsciiCurrFileStart = -999;
    iHeaderSize = 80;
    iDim = 3;
    iPrecision = 4;
    aBlockLocs = NULL;
    aBlocksPerFile = NULL;

    ParseMetaDataFile(filename);
    ParseNekFileHeader();

    visitTimer->StopTimer(t0, "avtNek5000FileFormat constructor");
}




// ****************************************************************************
//  Method: avtNek5000FileFormat::ParseNekFileHeader
//
//  Purpose:
//      This method is called as part of initialization.  It parses the text
//      file which is a companion to the series of .fld files that make up a
//      dataset.
//
//  Programmer: David Bremer
//  Creation:   Wed Feb  6 19:12:55 PST 2008
//
//  Modifications:
//    Dave Bremer, Thu Mar 20 11:28:05 PDT 2008
//    Changed sprintf to SNPRINTF.
//
//    Dave Bremer, Fri Jun  6 15:38:45 PDT 2008
//    Added the bParFormat flag allowing the parallel format to be used
//    by a serial code, in which there is only one output dir.
//
//    Kathleen Bonnell, Wed Jul 2 09:44:44 PDT 2008 
//    Removed unreferenced variables.
//
//    Dave Bremer, Fri Aug  8 12:57:47 PDT 2008
//    Brought in changes prompted by Stefan Kerkemeier, to check that a 
//    file open succeeded, allow parsing of the S## style of scalar field
//    tag within the serial file header, and read the number of output
//    directories from the parallel file header, allowing users to skip
//    the "numoutputdirs:" tag in the .nek file.
//
// ****************************************************************************

void           
avtNek5000FileFormat::ParseMetaDataFile(const char *filename)
{
    string tag;
    char buf[2048];
    ifstream  f(filename);
    int ii;

    // Verify that the 'magic' and version number are right
    f >> tag;
    while (tag[0] == '#')
    {
        f.getline(buf, 2048);
        f >> tag;
    }
    if (STREQUAL("NEK3D", tag.c_str()) != 0)
    {
        EXCEPTION1(InvalidDBTypeException, "Not a Nek5000 file." );
    }
    f >> tag;
    while (tag[0] == '#')
    {
        f.getline(buf, 2048);
        f >> tag;
    }
    f >> version;
    if (STREQUAL("version:", tag.c_str()) != 0)
    {
        EXCEPTION1(InvalidDBTypeException, "Not a Nek5000 file." );
    }
    if (version != "1.0")
    {
        EXCEPTION1(InvalidDBTypeException, 
                   "Only Nek5000 version 1.0 is supported." );
    }

    // Process a tag at a time until all lines have been read
    while (f.good())
    {
        f >> tag;
        if (f.eof()) {
            f.clear();
            break;
        }

        if (tag[0] == '#')
        {
            f.getline(buf, 2048);
            continue;
        }

        if (STREQUAL("endian:", tag.c_str())==0)
        {
            //This tag is deprecated.  There's a float written into each binary file
            //from which endianness can be determined.
            string  dummy_endianness;
            f >> dummy_endianness;
        }
        else if (STREQUAL("filetemplate:", tag.c_str())==0)
        {
            f >> fileTemplate;
        }
        else if (STREQUAL("firsttimestep:", tag.c_str())==0)
        {
            f >> iFirstTimestep;
        }
        else if (STREQUAL("numtimesteps:", tag.c_str())==0)
        {
            f >> iNumTimesteps;
        }
        else if (STREQUAL("meshcoords:", tag.c_str())==0)
        {
            //This tag is now deprecated.  The same info can be discovered by
            //this reader while it scans all the headers for time and cycle info.

            int nStepsWithCoords;
            f >> nStepsWithCoords;
            
            for (ii = 0 ; ii<nStepsWithCoords ; ii++)
            {
                int step;
                f >> step;
                //iTimestepsWithMesh.push_back(step);
            }
        }
        else if (STREQUAL("type:", tag.c_str())==0)
        {
            //This tag can be deprecated, because the type can be determined
            //from the header 
            string t;
            f >> t;
            if (STREQUAL("binary", t.c_str())==0)
            {
                bBinary = true;
            }
            else if (STREQUAL("binary6", t.c_str())==0)
            {
                bBinary = true;
                bParFormat = true;
            }
            else if (STREQUAL("ascii", t.c_str())==0)
            {
                bBinary = false;
            }
            else
            {
                EXCEPTION1(InvalidDBTypeException, 
                    "Value following type: \"ascii\" or \"binary\" or \"binary6\"" );
            }
        }
        else if (STREQUAL("numoutputdirs:", tag.c_str())==0)
        {
            //This tag can be deprecated, because the number of output dirs 
            //can be determined from the header, and the parallel nature of the
            //file can be inferred from the number of printf tokens in the template.
            //This reader scans the headers for the number of fld files
            f >> iNumOutputDirs;
            if (iNumOutputDirs > 1)
                bParFormat = true;
        }
        else
        {
            SNPRINTF(buf, 2048, "Error parsing file.  Unknown tag %s", tag.c_str());
            EXCEPTION1(InvalidDBTypeException, buf);
        }
    }

    //Do a little consistency checking before moving on
    if (fileTemplate == "")
    {
        EXCEPTION1(InvalidDBTypeException, 
            "A tag called filetemplate: must be specified" );
    }
    f.close();

    // make the file template, which is normally relative to the file being opened, 
    // into an absolute path
    if (fileTemplate[0] != '/')
    {
        for (ii = strlen(filename)-1 ; ii >= 0 ; ii--)
        {
            if (filename[ii] == '/' || filename[ii] == '\\')
            {
                fileTemplate.insert(0, filename, ii+1);
                break;
            }
        }
    }
    if (ii == -1)
    {
#ifdef _WIN32
        _getcwd(buf, 512);
#else
        getcwd(buf, 512);
#endif
        strcat(buf, "/");
        fileTemplate.insert(0, buf, strlen(buf));
    }

#ifdef _WIN32
    for (ii = 0 ; ii < fileTemplate.size() ; ii++)
    {
        if (fileTemplate[ii] == '/')
            fileTemplate[ii] = '\\';
    }
#endif
    if (f.is_open())
        f.close();
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::ParseNekFileHeader
//
//  Purpose:
//      This method is called as part of initialization.  Some of the file
//      metadata is written in the header of each timestep, and this method
//      reads and parses that metadata.
//
//  Programmer: David Bremer
//  Creation:   Wed Feb  6 19:12:55 PST 2008
//
//  Modified:
//    Dave Bremer, Thu Mar 20 11:28:05 PDT 2008
//    Use the GetFileName method.
//
//    Dave Bremer, Fri Jun  6 15:38:45 PDT 2008
//    Added the bParFormat flag allowing the parallel format to be used
//    by a serial code, in which there is only one output dir.
//
//    Dave Bremer, Thu Jun 12 12:59:23 PDT 2008
//    Support varying numbers of blocks per file in the parallel format, and
//    support a new format for the field tags.
//
//    Dave Bremer, Fri Jun 13 18:10:46 PDT 2008
//    Small change to be more robust about finding the location of the field tags.
//
//    Kathleen Bonnell, Wed Jul 2 09:44:44 PDT 2008 
//    Removed unreferenced variables.
//
//    Dave Bremer, Mon Aug 11 13:53:18 PDT 2008
//    Brought in a change from Stefan Kerkemeier to determine if a file is binary
//    by looking for the endian-ness tag.  Added my own change to field tag 
//    parsing, to parse the same way between serial and parallel binary files.
//
// ****************************************************************************

void
avtNek5000FileFormat::ParseNekFileHeader()
{
    string buf2, tag;

    //Now read the header out of one the files to get block and variable info
    char *blockfilename = new char[ fileTemplate.size() + 64 ];

    GetFileName(iFirstTimestep, 0, blockfilename, fileTemplate.size() + 64);
    ifstream  f(blockfilename);

    if (!f.is_open())
    {
        char msg[1024];
        SNPRINTF(msg, 1024, "Could not open file %s.", filename);
        EXCEPTION1(InvalidFilesException, msg);
    }

    // Determine the type (ascii or binary)
    // Parallel type is determined by the number of tokens in the file template
    // and is always binary
    if (!bParFormat)
    {
        float test;
        f.seekg (80, ios::beg);

        f.read((char *)(&test), 4);
        if (test > 6.5 && test < 6.6)
            bBinary = true;
        else
        {
            ByteSwap32(&test, 1);
            if (test > 6.5 && test < 6.6)
                bBinary = true;
        } 
        f.seekg (0, ios::beg);
    } 

    //iHeaderSize no longer includes the size of the block index metadata, for the 
    //parallel format, since this now can vary per file.
    if (bBinary && bParFormat)
        iHeaderSize = 136;
    else if (bBinary && !bParFormat)
        iHeaderSize = 84;
    else
        iHeaderSize = 80;


    if (!bParFormat)
    {
        f >> iNumBlocks;
        f >> iBlockSize[0];
        f >> iBlockSize[1];
        f >> iBlockSize[2];
    
        f >> buf2;   //skip
        f >> buf2;   //skip
    
        ParseFieldTags(f);
    }
    else
    {
        //Here's are some examples of what I'm parsing:
        //#std 4  6  6  6   120  240  0.1500E+01  300  1  2XUPT
        //#std 4  6  6  6   120  240  0.1500E+01  300  1  2 U T123
        //This example means:  #std is for versioning, 4 bytes per sample,  
        //  6x6x6 blocks, 120 of 240 blocks are in this file, time=1.5, 
        //  cycle=300, this output dir=1, num output dirs=2, XUPT123 are 
        //  tags that this file has a mesh, velocity, pressure, temperature, 
        //  and 3 misc scalars.
        //
        //A new revision of the binary header changes the way tags are
        //represented.  Line 2 above would be represented as
        //#std 4  6  6  6   120  240  0.1500E+01  300  1  2UTS03
        //The spaces between tags are removed, and instead of representing
        //scalars as 123, they use S03, allowing more than 9 total.
        f >> tag;
        if (tag != "#std")
        {
            EXCEPTION1(InvalidDBTypeException, 
                "Error reading the header.  Expected it to start with #std" );
        }
        f >> iPrecision;
        f >> iBlockSize[0];
        f >> iBlockSize[1];
        f >> iBlockSize[2];
        f >> buf2;  //blocks per file
        f >> iNumBlocks;

        //This bypasses some tricky and unnecessary parsing of data
        //I already have.  
        //6.13.08  No longer works...
        //f.seekg(77, std::ios_base::beg);
        f >> buf2;  //time
        f >> buf2;  //cycle
        f >> buf2;  //directory num of this file

        //I do this to skip the num directories token, because it may abut 
        //the field tags without a whitespace separator.
        while (f.peek() == ' ')
            f.get();

        //The number of output dirs comes next, but may abut the field tags
        iNumOutputDirs = 0;
        while (f.peek() >= '0' && f.peek() <= '9')
        {
            iNumOutputDirs *= 10;
            iNumOutputDirs += (f.get() - '0');
        }

        ParseFieldTags(f);

    }

    if (iBlockSize[2] == 1)
        iDim = 2;
    
    if (bBinary)
    {
        // Determine endianness and whether we need to swap bytes.
        // If this machine's endian matches the file's, the read will 
        // put 6.54321 into this float.

        float test;  
        if (!bParFormat)
        {
            f.seekg( 80, std::ios_base::beg );
            f.read((char *)(&test), 4);
        }
        else
        {
            f.seekg( 132, std::ios_base::beg );
            f.read((char *)(&test), 4);
        }
        if (test > 6.5 && test < 6.6)
            bSwapEndian = false;
        else
        {
            ByteSwap32(&test, 1);
            if (test > 6.5 && test < 6.6)
                bSwapEndian = true;
            else
            {
                EXCEPTION1(InvalidDBTypeException, 
                    "Error reading file, while trying to determine endianness." );
            }
        }
    }

    // Now that we know iNumBlocks, we can set up what the "typical elements"
    // are for each processor.
    int rank   = PAR_Rank();
    int nprocs = PAR_Size();
    int elements_per_proc = iNumBlocks / nprocs;
    int one_extra_until = iNumBlocks % nprocs;
    int my_num_elements = elements_per_proc + (rank < one_extra_until ? 1 : 0);
    cachableElementMin = elements_per_proc*rank + (rank < one_extra_until ? rank : one_extra_until);
    cachableElementMax = cachableElementMin + my_num_elements;

    delete[] blockfilename;

}



// ****************************************************************************
//  Method: avtNek5000FileFormat::ParseFieldTags
//
//  Purpose:
//      Parses the characters in a binary Nek header file that indicate which
//  fields are present.  Tags can be separated by 0 or more spaces.  Parsing
//  stops when an unknown character is encountered, or when the file pointer
//  moves beyond this->iHeaderSize characters.
//
//  X or X Y Z indicate a mesh.  Ignored here, UpdateCyclesAndTimes picks this up.
//  U indicates velocity
//  P indicates pressure
//  T indicates temperature
//  1 2 3 or S03  indicate 3 other scalar fields
//
//  Programmer: David Bremer
//  Creation:   Mon Aug 11 13:53:18 PDT 2008
//
//  Modified:
// ****************************************************************************

void avtNek5000FileFormat::ParseFieldTags(ifstream &f)
{
    while (f.tellg() < iHeaderSize)
    {
        char c = f.get();
        if (c == ' ')
            continue;
        if (c == 'X' || c == 'Y' || c == 'Z')
            continue;
        else if (c == 'U')
            bHasVelocity = true;
        else if (c == 'P')
            bHasPressure = true;
        else if (c == 'T')
            bHasTemperature = true;
        else if (c == '1')
        {
            while (f.tellg() < iHeaderSize)
            {
                while (f.peek() == ' ')
                    f.get();
                if ((f.peek()-'0') == (iNumSFields+1))
                {
                    iNumSFields++;
                    f.get();
                }
                else
                    break;
            }
        }
        else if (c == 'S')
        {
            while (f.peek() == ' ')
                f.get();
            char digit1 = f.get();
            while (f.peek() == ' ')
                f.get();
            char digit2 = f.get();

            iNumSFields = (digit1-'0')*10 + (digit2-'0');
        }
        else
            break;
    }
}




// ****************************************************************************
//  Method: avtNek5000FileFormat::ReadBlockLocations
//
//  Purpose:
//      For the parallel binary format, there is a mapping from global block 
//      index to filenumber/local offset, which is distributed through the
//      files.  This method reads that information.
//
//  Programmer: David Bremer
//  Creation:   Wed Feb  6 19:12:55 PST 2008
//
//  Modified:
//    Dave Bremer, Thu Mar 20 11:28:05 PDT 2008
//    Use the GetFileName method.  Changed sprintf to SNPRINTF.
//
//    Dave Bremer, Tue May 13 19:51:04 CDT 2008
//    Use a simpler block numbering scheme, to provide faster I/O because of
//    more coherent file access.
//
//    Dave Bremer, Fri Jun  6 15:38:45 PDT 2008
//    Added the bParFormat flag allowing the parallel format to be used
//    by a serial code, in which there is only one output dir.
//
//    Dave Bremer, Thu Jun 12 12:59:23 PDT 2008
//    Support varying numbers of blocks per file in the parallel format.
// 
//    Kathleen Bonnell, Wed Jul 2 09:44:44 PDT 2008 
//    Removed unreferenced variables.
//
// ****************************************************************************
void
avtNek5000FileFormat::ReadBlockLocations()
{
    // In each parallel file, in the header, there's a table that maps 
    // each local block to a global id which starts at 1.  Here, I make 
    // an inverse map, from a zero-based global id to a proc num and local
    // offset.

    if (!bBinary || !bParFormat || aBlockLocs != NULL)
        return;

    int ii, jj;
    aBlockLocs = new int[2*iNumBlocks];
    aBlocksPerFile = new int[iNumOutputDirs];

    int t0 = visitTimer->StartTimer();

    int iRank = 0, nProcs = 1;
#ifdef PARALLEL
    MPI_Comm_rank(VISIT_MPI_COMM, &iRank);
    MPI_Comm_size(VISIT_MPI_COMM, &nProcs);
#endif

    ifstream f;
    for (ii = 0; ii < 2*iNumBlocks; ii++)
    {
        aBlockLocs[ii] = 0;
    }
    for (ii = 0; ii < iNumOutputDirs; ii++)
    {
        aBlocksPerFile[ii] = 0;
    }
    char *blockfilename = new char[ fileTemplate.size() + 64 ];
    int *tmpBlocks = new int[iNumBlocks];

    for (ii = iRank; ii < iNumOutputDirs; ii+=nProcs)
    {
        int t0 = visitTimer->StartTimer();
        GetFileName(iFirstTimestep, ii, blockfilename, fileTemplate.size() + 64);
        f.open(blockfilename);
        visitTimer->StopTimer(t0, "avtNek5000FileFormat constructor, time to open a file");
        if (!f.is_open())
        {
            char msg[1024];
            SNPRINTF(msg, 1024, "Could not open file %s.", filename);
            EXCEPTION1(InvalidDBTypeException, msg);
        }

        int tmp1, tmp2, tmp3, tmp4;
        f.seekg( 5, std::ios_base::beg );  //seek past the #std
        f >> tmp1 >> tmp2 >> tmp3 >> tmp4 >> aBlocksPerFile[ii];

#ifndef USE_SIMPLE_BLOCK_NUMBERING
        f.seekg( 136, std::ios_base::beg );
        f.read( (char *)tmpBlocks, aBlocksPerFile[ii]*sizeof(int) );
        if (bSwapEndian)
            ByteSwap32(tmpBlocks, aBlocksPerFile[ii]);

        for (jj = 0; jj < aBlocksPerFile[ii]; jj++)
        {
            int iBlockID = tmpBlocks[jj]-1;

            if ( iBlockID < 0  ||  
                 iBlockID >= iNumBlocks ||
                 aBlockLocs[iBlockID*2]   != 0  ||
                 aBlockLocs[iBlockID*2+1] != 0 )
            {
                EXCEPTION1(InvalidDBTypeException, 
                            "Error reading parallel file block IDs.");
            }
            aBlockLocs[iBlockID*2  ] = ii;
            aBlockLocs[iBlockID*2+1] = jj;
        }
#endif
        f.close();
    }
    delete[] blockfilename;
    delete[] tmpBlocks;

#ifdef PARALLEL
    int *aTmpBlocksPerFile = new int[iNumOutputDirs];

    MPI_Allreduce(aBlocksPerFile, aTmpBlocksPerFile, iNumOutputDirs, 
                  MPI_INT, MPI_BOR, VISIT_MPI_COMM);
    delete[] aBlocksPerFile;
    aBlocksPerFile = aTmpBlocksPerFile;
#endif

    //Do a sanity check
    int sum = 0;

    for (ii = 0; ii < iNumOutputDirs; ii++)
        sum += aBlocksPerFile[ii];

    if (sum != iNumBlocks)
    {
        EXCEPTION1(InvalidDBTypeException, 
                   "Sum of blocks per file does not equal total number of blocks");
    }

#ifdef USE_SIMPLE_BLOCK_NUMBERING
    //fill in aBlockLocs ...
    int *p = aBlockLocs;
    for (jj = 0; jj < iNumOutputDirs; jj++)
    {
        for (ii = 0; ii < aBlocksPerFile[jj]; ii++)
        {
            *p++ = jj;
            *p++ = ii;
        }
    }
#else
#ifdef PARALLEL
    int *aTmpBlockLocs = new int[2*iNumBlocks];

    MPI_Allreduce(aBlockLocs, aTmpBlockLocs, 2*iNumBlocks, 
                  MPI_INT, MPI_BOR, VISIT_MPI_COMM);
    delete[] aBlockLocs;
    aBlockLocs = aTmpBlockLocs;
#endif
#endif

    visitTimer->StopTimer(t0, "avtNek5000FileFormat  reading block locations");
}



// ****************************************************************************
//  Method: avtNek5000FileFormat destructor
//
//  Programmer: David Bremer
//  Creation:   Wed Nov 14 11:35:30 PST 2007
//
//  Modifications:
//
//    Dave Bremer, Thu Jun 12 12:59:23 PDT 2008
//    Delete the array specifying the number of blocks per file.
//
//    Hank Childs, Wed Jan  7 18:24:01 CST 2009
//    Clear out the cache.
//
// ****************************************************************************

avtNek5000FileFormat::~avtNek5000FileFormat()
{
    FreeUpResources();
    if (aBlockLocs)
    {
        delete[] aBlockLocs;
        aBlockLocs = NULL;
    }
    if (aBlocksPerFile)
    {
        delete[] aBlocksPerFile;
        aBlocksPerFile = NULL;
    }

    std::map<PointerKey, float *>::iterator it;
    for (it = cachedData.begin() ; it != cachedData.end() ; it++)
        delete [] it->second;
    std::map<int, avtIntervalTree *>::iterator it2;
    for (it2 = boundingBoxes.begin() ; it2 != boundingBoxes.end() ; it2++)
        delete [] it2->second;
    std::map<PointerKey, avtIntervalTree *>::iterator it3;
    for (it3 = dataExtents.begin() ; it3 != dataExtents.end() ; it3++)
        delete [] it3->second;
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: dbremer -- generated by xml2avt
//  Creation:   Fri May 18 16:07:09 PST 2007
//
// ****************************************************************************

int
avtNek5000FileFormat::GetNTimesteps(void)
{
    return iNumTimesteps;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: dbremer -- generated by xml2avt
//  Creation:   Fri May 18 16:07:09 PST 2007
//
//  Modifications:
//
//    Hank Childs, Wed Jan  7 18:22:47 CST 2009
//    Make sure we set the name of the open files to a bad value to ensure we
//    don't blindly use a closed file descriptor.
//
// ****************************************************************************

void
avtNek5000FileFormat::FreeUpResources(void)
{
    // Closing this file descriptor might not be necessary here.  Maybe can
    // be done in the destructor.  It doesn't matter here, but may be important
    // if we cache the whole mesh in memory.
    if (fdMesh)
    {
        fclose(fdMesh);
        curOpenMeshFile = "<none>";
        fdMesh = NULL;
    }
    if (fdVar)
    {
        fclose(fdVar);
        curOpenVarFile = "<none>";
        fdVar = NULL;
    }
    iCurrTimestep = -999;
    iCurrMeshProc = -999;
    iCurrVarProc  = -999;
    iAsciiMeshFileStart = -999;
    iAsciiCurrFileStart = -999;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: dbremer -- generated by xml2avt
//  Creation:   Fri May 18 16:07:09 PST 2007
//
//  Modifications:
//    Dave Bremer, Wed Nov  7 14:17:19 PST 2007
//    Added support for 2D and files, and added support for misc scalar fields
//    in addition to pressure and temperature.
//
//    Dave Bremer, Wed Nov 14 15:00:13 PST 2007
//    Changed block numbering from 0 based to 1 based.
//
//    Dave Bremer, Fri Jan 18 16:21:34 PST 2008
//    Added domain boundary metadata.
//
//    Dave Bremer, Thu Mar 20 11:28:05 PDT 2008
//    Changed sprintf to SNPRINTF.
//
//    Hank Childs, Thu Dec 18 10:15:50 CST 2008
//    Indicate that the format now does its own domain decomposition.
//
// ****************************************************************************

void
avtNek5000FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int /*timeState*/)
{
    string meshname = "mesh";
    double *extents = NULL;
    AddMeshToMetaData(md, meshname, AVT_CURVILINEAR_MESH, extents, 1, 1, iDim, iDim);
    md->SetFormatCanDoDomainDecomposition(true);
    
    if (bHasPressure)
        AddScalarVarToMetaData(md, "pressure", meshname, AVT_NODECENT);
    
    if (bHasTemperature)
        AddScalarVarToMetaData(md, "temperature", meshname, AVT_NODECENT);

    if (bHasVelocity)
    {
        AddVectorVarToMetaData(md, "velocity", meshname, AVT_NODECENT, iDim);

        AddScalarVarToMetaData(md, "x_velocity", meshname, AVT_NODECENT);
        AddScalarVarToMetaData(md, "y_velocity", meshname, AVT_NODECENT);

        if (iDim == 3)
            AddScalarVarToMetaData(md, "z_velocity", meshname, AVT_NODECENT);
    }
    int ii;
    for (ii = 0; ii < iNumSFields; ii++)
    {
        char scalarVarName[32];
        SNPRINTF(scalarVarName, 32, "s%d", ii+1);
        AddScalarVarToMetaData(md, scalarVarName, meshname, AVT_NODECENT);
    }
/*
    // HANK: need to find a solution for abutting elements.
    if (!avtDatabase::OnlyServeUpMetaData())
    {
        avtNekDomainBoundaries *db = new avtNekDomainBoundaries;
        db->SetDomainInfo(iNumBlocks, iBlockSize);

        void_ref_ptr vr = void_ref_ptr(db, avtNekDomainBoundaries::Destruct);
        cache->CacheVoidRef("any_mesh",
                       AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1, vr);
    }
 */
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  
//      Applies some caching because the mesh is the same for all time slices.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2008
//
//  Notes:      timestate argument is ignored, in favor of the data member
//              "timestepToUseForMesh"
//
//  Modifications:
//
//    Hank Childs, Thu Dec 18 07:35:16 PST 2008
//    Change routine to assemble an unstructured mesh from the many curvilinear 
//    grids being read in.
//
// ****************************************************************************

vtkDataSet *
avtNek5000FileFormat::GetMesh(int /* timestate */, int domain, const char * /*meshname*/)
{
    int  i;

    int t1 = visitTimer->StartTimer();
    const int num_elements = myElementList.size();
    vector<float *> ptlist(num_elements);
    for (i = 0 ; i < num_elements ; i++)
    {
        int element  = myElementList[i];
        int timestep = timestepToUseForMesh;

        PointerKey key;
        key.var = "points";
        key.element = element;
        key.timestep = timestep;
    
        std::map<PointerKey, float *, KeyCompare>::iterator it;
        it = cachedData.find(key);
        float *pts = NULL;
        if (it == cachedData.end())
        {
            pts = ReadPoints(element, timestep);
            // Only cache the elements that are likely to occur on this proc.
            if (cachableElementMin <= element && element < cachableElementMax)
                cachedData.insert(std::pair<PointerKey, float *>(key, pts));
        }
        else
            pts = it->second;
        ptlist[i] = pts;
    }
    visitTimer->StopTimer(t1, "Reading points from file");

    int t2 = visitTimer->StartTimer();
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    vtkPoints *pts = vtkPoints::New();
    int pts_per_element = iBlockSize[0]*iBlockSize[1];
    if (iDim == 3)
        pts_per_element *= iBlockSize[2];
    pts->SetNumberOfPoints(pts_per_element * num_elements);
    float *pts_ptr = (float *) pts->GetVoidPointer(0);
    for (i = 0 ; i < num_elements ; i++)
    {
        memcpy(pts_ptr, ptlist[i], pts_per_element*3*sizeof(float));
        pts_ptr += pts_per_element*3;
    }
    ugrid->SetPoints(pts);
    pts->Delete();

    int hexes_per_element = (iBlockSize[0]-1)*(iBlockSize[1]-1);
    if (iDim == 3)
        hexes_per_element *= (iBlockSize[2]-1);
    int total_hexes = hexes_per_element*num_elements;
    int total_size = (iDim == 3 ? 9*total_hexes : 5*total_hexes);

    vtkIdTypeArray *nlist = vtkIdTypeArray::New();
    nlist->SetNumberOfValues(total_size);
    vtkIdType *nl = nlist->GetPointer(0);

    vtkUnsignedCharArray *cellTypes = vtkUnsignedCharArray::New();
    cellTypes->SetNumberOfValues(total_hexes);
    unsigned char *ct = cellTypes->GetPointer(0);

    vtkIdTypeArray *cellLocations = vtkIdTypeArray::New();
    cellLocations->SetNumberOfValues(total_hexes);
    int *cl = cellLocations->GetPointer(0);

    int hexes_so_far = 0;
    int elements_so_far = 0;
    for (i = 0 ; i < num_elements ; i++)
    {
        int pt_start = pts_per_element * elements_so_far;
        for (int ii = 0 ; ii < iBlockSize[0]-1 ; ii++)
        {
            for (int jj = 0 ; jj < iBlockSize[1]-1 ; jj++)
            {
                if (iDim == 2)
                {
                    *nl++ = 4;
                    *nl++ = jj*(iBlockSize[0]) + ii + pt_start;
                    *nl++ = jj*(iBlockSize[0]) + ii+1 + pt_start;
                    *nl++ = (jj+1)*(iBlockSize[0]) + ii+1 + pt_start;
                    *nl++ = (jj+1)*(iBlockSize[0]) + ii + pt_start;
                    *ct++ = VTK_QUAD;
                    *cl++ = 5*(hexes_so_far++);
                }
                else
                {
                    for (int kk = 0 ; kk < iBlockSize[2]-1 ; kk++)
                    {
                        *nl++ = 8;
                        *nl++ = kk*(iBlockSize[1])*(iBlockSize[0]) + jj*(iBlockSize[0]) + ii + pt_start;
                        *nl++ = kk*(iBlockSize[1])*(iBlockSize[0]) + jj*(iBlockSize[0]) + ii+1 + pt_start;
                        *nl++ = kk*(iBlockSize[1])*(iBlockSize[0]) + (jj+1)*(iBlockSize[0]) + ii+1 + pt_start;
                        *nl++ = kk*(iBlockSize[1])*(iBlockSize[0]) + (jj+1)*(iBlockSize[0]) + ii + pt_start;
                        *nl++ = (kk+1)*(iBlockSize[1])*(iBlockSize[0]) + jj*(iBlockSize[0]) + ii + pt_start;
                        *nl++ = (kk+1)*(iBlockSize[1])*(iBlockSize[0]) + jj*(iBlockSize[0]) + ii+1 + pt_start;
                        *nl++ = (kk+1)*(iBlockSize[1])*(iBlockSize[0]) + (jj+1)*(iBlockSize[0]) + ii+1 + pt_start;
                        *nl++ = (kk+1)*(iBlockSize[1])*(iBlockSize[0]) + (jj+1)*(iBlockSize[0]) + ii + pt_start;
                        *ct++ = VTK_HEXAHEDRON;
                        *cl++ = 9*(hexes_so_far++);
                    }
                }
            }
        }
        elements_so_far++;
    }

    vtkCellArray *cells = vtkCellArray::New();
    cells->SetCells(total_hexes, nlist);
    nlist->Delete();

    ugrid->SetCells(cellTypes, cellLocations, cells);
    cellTypes->Delete();
    cellLocations->Delete();
    cells->Delete();
    visitTimer->StopTimer(t2, "Building unstructured mesh");
    return ugrid;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::ReadPoints
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
//
//  Programmer: dbremer -- generated by xml2avt
//  Creation:   Fri May 18 16:07:09 PST 2007
//
//  Modifications:
//    Dave Bremer, Wed Jul 18 16:53:29 PDT 2007
//    Fixed fopen call on windows.
//
//    Dave Bremer, Tue Sep 11 15:45:51 PDT 2007
//    Added a small mod to support variable-length headers 
//
//    Dave Bremer, Wed Nov  7 14:17:19 PST 2007
//    Added support for 2D and ascii files, added a call to 
//    UpdateCyclesAndTimes(), which also figures out which
//    files have a mesh.
//
//    Dave Bremer, Wed Nov 14 15:00:13 PST 2007
//    Added support for the parallel version of the file.
//
//    Dave Bremer, Thu Nov 15 16:44:42 PST 2007
//    Small fix for ascii format in case windows-style CRLF is used.
//
//    Kathleen Bonnell, Wed Nov 28 09:43:34 PST 2007 
//    Added space in args list between 'const char *' and '/*meshname*/'.
// 
//    Dave Bremer, Thu Mar 20 11:28:05 PDT 2008
//    Use the GetFileName method.
//
//    Dave Bremer, Fri Jun  6 15:38:45 PDT 2008
//    Added the bParFormat flag allowing the parallel format to be used
//    by a serial code, in which there is only one output dir.
//
//    Dave Bremer, Thu Jun 12 12:59:23 PDT 2008
//    Fix a bug reading files larger than 2Gb
//
//    Hank Childs, Sat Nov  8 14:33:30 PST 2008
//    Renamed to "ReadPoints".  The new "GetMesh" method now does caching
//    across time slices.
//
//    Hank Childs, Thu Dec 18 10:54:27 CST 2008
//    Refactored method to only serve up points.
//
// ****************************************************************************

float *
avtNek5000FileFormat::ReadPoints(int element, int timestep)
{
    int  ii;

    int nPts = iBlockSize[0]*iBlockSize[1];
    if (iDim == 3)
        nPts *= iBlockSize[2];
    float *pts = new float[3*nPts];

    UpdateCyclesAndTimes();   //This call also finds which timesteps have a mesh.
    ReadBlockLocations();

    if (fdMesh == NULL || (bParFormat && aBlockLocs[element*2] != iCurrMeshProc))
    {
        char *meshfilename = new char[ fileTemplate.size() + 64 ];

        iCurrMeshProc = 0;
        if (bParFormat)
            iCurrMeshProc = aBlockLocs[element*2];

        GetFileName(iTimestepsWithMesh[0], iCurrMeshProc, 
                    meshfilename, fileTemplate.size() + 64);

        if (curOpenMeshFile != meshfilename)
        {
            if (fdMesh)
                fclose(fdMesh);

            fdMesh = fopen(meshfilename, "rb");
            curOpenMeshFile = meshfilename;
            if (!fdMesh)
                EXCEPTION1(InvalidFilesException, meshfilename);
        }
        delete [] meshfilename;

        if (!bBinary)
            FindAsciiDataStart(fdMesh, iAsciiMeshFileStart, iAsciiMeshFileLineLen);
    }

    if (bParFormat)
        element = aBlockLocs[element*2 + 1];

    int nFloatsInDomain = 0, d1, d2, d3;
    GetDomainSizeAndVarOffset(iTimestepsWithMesh[0], NULL, nFloatsInDomain, 
                              d1, d2, d3);

    int64_t iRealHeaderSize = iHeaderSize + (bParFormat ? aBlocksPerFile[iCurrMeshProc]*sizeof(int) : 0);

    if (bBinary)
    {
        //In the parallel format, the whole mesh comes before all the vars.
        if (bParFormat)
            nFloatsInDomain = iDim*iBlockSize[0]*iBlockSize[1]*iBlockSize[2];

        if (iPrecision == 4)
        {
            float *tmppts = new float[nPts*iDim];
            fseek(fdMesh, iRealHeaderSize + (int64_t)nFloatsInDomain*sizeof(float)*element, SEEK_SET);
            fread(tmppts, sizeof(float), nPts*iDim, fdMesh);
            if (bSwapEndian)
                ByteSwap32(tmppts, nPts*iDim);
    
            for (ii = 0 ; ii < nPts ; ii++)
            {
                if (iDim == 3)
                {
                    pts[ii*3 + 0] = tmppts[ii];
                    pts[ii*3 + 1] = tmppts[ii+nPts];
                    pts[ii*3 + 2] = tmppts[ii+nPts+nPts];
                }
                else
                {
                    pts[ii*3 + 0] = tmppts[ii];
                    pts[ii*3 + 1] = tmppts[ii+nPts];
                    pts[ii*3 + 2] = 0.0f;
                }
            }
            delete [] tmppts;
        }
        else
        {
            double *tmppts = new double[nPts*iDim];
            fseek(fdMesh, iRealHeaderSize + (int64_t)nFloatsInDomain*sizeof(double)*element, SEEK_SET);
            fread(tmppts, sizeof(double), nPts*iDim, fdMesh);
            if (bSwapEndian)
                ByteSwap64(tmppts, nPts*iDim);
    
            for (ii = 0 ; ii < nPts ; ii++)
            {
                if (iDim == 3)
                {
                    pts[ii*3 + 0] = (float)tmppts[ii];
                    pts[ii*3 + 1] = (float)tmppts[ii+nPts];
                    pts[ii*3 + 2] = (float)tmppts[ii+nPts+nPts];
                }
                else
                {
                    pts[ii*3 + 0] = (float)tmppts[ii];
                    pts[ii*3 + 1] = (float)tmppts[ii+nPts];
                    pts[ii*3 + 2] = 0.0;
                }
            }
            delete [] tmppts;
        }
    }
    else
    {
        for (ii = 0 ; ii < nPts ; ii++)
        {
            fseek(fdMesh, (int64_t)iAsciiMeshFileStart + 
                          (int64_t)element*iAsciiMeshFileLineLen*nPts + 
                          (int64_t)ii*iAsciiMeshFileLineLen, SEEK_SET);
            if (iDim == 3)
            {
                fscanf(fdMesh, " %f %f %f", pts, pts+1, pts+2);
            }
            else
            {
                fscanf(fdMesh, " %f %f", pts, pts+1);
                pts[2] = 0.0f;
            }
            pts += 3;
        }
    }
    return pts;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::GetVar
//
//  Purpose:
//      Implements the virtual method for avtFileFormat to read a scalar
//      variable from the file.
//
//  Programmer: Hank Childs
//  Creation:   December 19, 2008
//
// ****************************************************************************

vtkDataArray *
avtNek5000FileFormat::GetVar(int timestep, int domain, const char *varname)
{
    int t1 = visitTimer->StartTimer();

    const int num_elements = myElementList.size();
    int pts_per_element = iBlockSize[0]*iBlockSize[1];
    if (iDim == 3)
        pts_per_element *= iBlockSize[2];
    int nPts = num_elements*pts_per_element;

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(nPts);
    float *varptr = (float *)rv->GetVoidPointer(0);

    for (int i = 0 ; i < num_elements ; i++)
    {
        int element = myElementList[i];

        PointerKey key;
        key.var = varname;
        key.element = element;
        key.timestep = timestep;
    
        std::map<PointerKey, float *, KeyCompare>::iterator it;
        it = cachedData.find(key);
        float *v = NULL;
        if (it == cachedData.end())
        {
            v = ReadVar(timestep, element, varname);
            // Only cache the elements that are likely to occur on this proc.
            if (cachableElementMin <= element && element < cachableElementMax)
                cachedData.insert(std::pair<PointerKey, float *>(key, v));
        }
        else
            v = it->second;
        memcpy(varptr, v, pts_per_element*sizeof(float));
        varptr += pts_per_element;
    }

    visitTimer->StopTimer(t1, "Reading variable from file");
    return rv;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::ReadVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      element    The Nek element ID.
//      varname    The name of the variable requested.
//
//  Programmer: dbremer -- generated by xml2avt
//  Creation:   Fri May 18 16:07:09 PST 2007
//
//  Modifications:
//    Dave Bremer, Wed Jul 18 16:53:29 PDT 2007
//    Fixed fopen call on windows.
//
//    Dave Bremer, Tue Sep 11 15:45:51 PDT 2007
//    Added a small mod to support variable-length headers 
//
//    Dave Bremer, Wed Nov  7 14:17:19 PST 2007
//    Added support for 2D and ascii files
//
//    Dave Bremer, Wed Nov 14 15:00:13 PST 2007
//    Added support for the parallel version of the file.
//
//    Dave Bremer, Thu Nov 15 16:44:42 PST 2007
//    Small fix for ascii format in case windows-style CRLF is used.
//
//    Dave Bremer, Thu Mar 20 11:28:05 PDT 2008
//    Use the GetFileName method.
//
//    Dave Bremer, Fri Jun  6 15:38:45 PDT 2008
//    Added the bParFormat flag allowing the parallel format to be used
//    by a serial code, in which there is only one output dir.
//
//    Dave Bremer, Thu Jun 12 12:59:23 PDT 2008
//    Fix a bug reading files larger than 2Gb
//
//    Hank Childs, Fri Dec 19 10:38:23 CST 2008
//    Renamed method to ReadVar
//
// ****************************************************************************

float *
avtNek5000FileFormat::ReadVar(int timestate, int element, const char *varname)
{
    int iTimestep = timestate + iFirstTimestep;
    const int nPts = iBlockSize[0]*iBlockSize[1]*iBlockSize[2];
    float *var = new float[nPts];
    int ii;
    ReadBlockLocations();

    if (iTimestep != iCurrTimestep || (bParFormat && aBlockLocs[element*2] != iCurrVarProc))
    {
        if (fdVar)
            fclose(fdVar);

        char *filename = new char[ fileTemplate.size() + 64 ];

        iCurrVarProc = 0;
        if (bParFormat)
            iCurrVarProc = aBlockLocs[element*2];

        GetFileName(iTimestep, iCurrVarProc, 
                    filename, fileTemplate.size() + 64);

        fdVar = fopen(filename, "rb");
        if (!fdVar)
            EXCEPTION1(InvalidFilesException, filename);
        delete[] filename;

        iCurrTimestep = iTimestep;
        if (!bBinary)
            FindAsciiDataStart(fdVar, iAsciiCurrFileStart, iAsciiCurrFileLineLen);
    }

    int nFloatsInDomain = 0, iBinaryOffset = 0, iAsciiOffset = 0, iHasMesh = 0;

    GetDomainSizeAndVarOffset(iTimestep, varname, nFloatsInDomain, 
                              iBinaryOffset, iAsciiOffset, iHasMesh);

    if (bParFormat)
        element = aBlockLocs[element*2 + 1];

    int iRealHeaderSize = iHeaderSize + (bParFormat ? aBlocksPerFile[iCurrVarProc]*sizeof(int) : 0);

    if (bBinary)
    {
        int64_t filepos;
        if (!bParFormat)
            filepos = (int64_t)iRealHeaderSize + ((int64_t)nFloatsInDomain*element + iBinaryOffset)*sizeof(float);
        else
        {
            // This assumes uvw for all fields comes after the mesh as [block0: 216u 216v 216w]...
            // then p or t as   [block0: 216p][block1: 216p][block2: 216p]...
            if (strcmp(varname+2, "velocity") == 0)
            {
                filepos  = (int64_t)iRealHeaderSize +                              //header
                           (int64_t)iHasMesh*aBlocksPerFile[iCurrVarProc]*nPts*iDim*iPrecision + //mesh
                           (int64_t)element*nPts*iDim*iPrecision +                  //start of block
                           (int64_t)(varname[0] - 'x')*nPts*iPrecision;            //position within block
            }
            else
                filepos = (int64_t)iRealHeaderSize + 
                          (int64_t)aBlocksPerFile[iCurrVarProc]*iBinaryOffset*iPrecision + //the header, mesh, vel if present,
                          (int64_t)element*nPts*iPrecision;
        }
        if (iPrecision==4)
        {
            fseek(fdVar, filepos, SEEK_SET);
            fread(var, sizeof(float), nPts, fdVar);
            if (bSwapEndian)
                ByteSwap32(var, nPts);
        }
        else
        {
            double *tmp = new double[nPts];

            fseek(fdVar, filepos, SEEK_SET);
            fread(tmp, sizeof(double), nPts, fdVar);
            if (bSwapEndian)
                ByteSwap64(tmp, nPts);

            for (ii = 0 ; ii < nPts ; ii++)
                var[ii] = (float)tmp[ii];

            delete[] tmp;
        }
    }
    else
    {
        for (ii = 0 ; ii < nPts ; ii++)
        {
            fseek(fdVar, (int64_t)iAsciiCurrFileStart + 
                         (int64_t)element*iAsciiCurrFileLineLen*nPts + 
                         (int64_t)ii*iAsciiCurrFileLineLen + 
                         (int64_t)iAsciiOffset, SEEK_SET);
            fscanf(fdVar, " %f", var);
            var++;
        }
    }

    return var;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::GetVectorVar
//
//  Purpose:
//      Implements the virtual method for avtFileFormat to read a vector
//      variable from the file.
//
//  Programmer: Hank Childs
//  Creation:   January 5, 2009
//
// ****************************************************************************

vtkDataArray *
avtNek5000FileFormat::GetVectorVar(int timestep, int domain, const char *varname)
{
    if (strcmp(varname, "velocity") != 0)
        EXCEPTION1(InvalidVariableException, varname);

    int t1 = visitTimer->StartTimer();

    const int num_elements = myElementList.size();
    int pts_per_element = iBlockSize[0]*iBlockSize[1];
    if (iDim == 3)
        pts_per_element *= iBlockSize[2];
    int nPts = num_elements*pts_per_element;

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(3);
    rv->SetNumberOfTuples(nPts);
    float *varptr = (float *)rv->GetVoidPointer(0);

    for (int i = 0 ; i < num_elements ; i++)
    {
        int element = myElementList[i];

        PointerKey key;
        key.var = varname;
        key.element = element;
        key.timestep = timestep;
    
        std::map<PointerKey, float *, KeyCompare>::iterator it;
        it = cachedData.find(key);
        float *v = NULL;
        if (it == cachedData.end())
        {
            v = ReadVar(timestep, element, varname);
            // Only cache the elements that are likely to occur on this proc.
            if (cachableElementMin <= element && element < cachableElementMax)
                cachedData.insert(std::pair<PointerKey, float *>(key, v));
        }
        else
            v = it->second;
        memcpy(varptr, v, 3*pts_per_element*sizeof(float));
        varptr += pts_per_element;
    }

    visitTimer->StopTimer(t1, "Reading variable from file");
    return rv;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: dbremer -- generated by xml2avt
//  Creation:   Fri May 18 16:07:09 PST 2007
//
//  Modifications:
//    Dave Bremer, Wed Jul 18 16:53:29 PDT 2007
//    Fixed fopen call on windows.
//
//    Dave Bremer, Tue Sep 11 15:45:51 PDT 2007
//    Added a small mod to support variable-length headers 
//
//    Dave Bremer, Wed Nov  7 14:17:19 PST 2007
//    Added support for 2D and ascii files
//
//    Dave Bremer, Wed Nov 14 15:00:13 PST 2007
//    Added support for the parallel version of the file.
//
//    Dave Bremer, Thu Nov 15 16:44:42 PST 2007
//    Small fix for ascii format in case windows-style CRLF is used.
//
//    Dave Bremer, Thu Mar 20 11:28:05 PDT 2008
//    Use the GetFileName method.
//
//    Dave Bremer, Fri Jun  6 15:38:45 PDT 2008
//    Added the bParFormat flag allowing the parallel format to be used
//    by a serial code, in which there is only one output dir.
//
//    Dave Bremer, Thu Jun 12 12:59:23 PDT 2008
//    Fix a bug reading files larger than 2Gb
//  
//    Hank Childs, Mon Jan  5 12:27:50 CST 2009
//    Renamed to ReadVelocity and changed method to only read the data and NOT
//    set up the VTK data structure.
//
// ****************************************************************************

float *
avtNek5000FileFormat::ReadVelocity(int timestate, int domain)
{
    int iTimestep = timestate + iFirstTimestep;
    const int nPts = iBlockSize[0]*iBlockSize[1]*iBlockSize[2];
    int ii;
    ReadBlockLocations();

    float *var = new float[nPts*3];

    if (iTimestep != iCurrTimestep || (bParFormat && aBlockLocs[domain*2] != iCurrVarProc))
    {
        if (fdVar)
            fclose(fdVar);

        char *filename = new char[ fileTemplate.size() + 64 ];
        iCurrVarProc = 0;
        if (bParFormat)
            iCurrVarProc = aBlockLocs[domain*2];

        GetFileName(iTimestep, iCurrVarProc, 
                    filename, fileTemplate.size() + 64);

        fdVar = fopen(filename, "rb");
        if (!fdVar)
            EXCEPTION1(InvalidFilesException, filename);
        delete[] filename;

        iCurrTimestep = iTimestep;
        if (!bBinary)
            FindAsciiDataStart(fdVar, iAsciiCurrFileStart, iAsciiCurrFileLineLen);
    }

    int nFloatsInDomain = 0, iBinaryOffset = 0, iAsciiOffset = 0, dummy;

    GetDomainSizeAndVarOffset(iTimestep, "velocity", nFloatsInDomain, 
                              iBinaryOffset, iAsciiOffset, dummy);
    if (bParFormat)
        domain = aBlockLocs[domain*2 + 1];

    int iRealHeaderSize = iHeaderSize + (bParFormat ? aBlocksPerFile[iCurrVarProc]*sizeof(int) : 0);

    if (bBinary)
    {
        int64_t filepos;
        if (!bParFormat)
            filepos = (int64_t)iRealHeaderSize + (int64_t)(nFloatsInDomain*domain + iBinaryOffset)*sizeof(float);
        else
            //This assumes [block 0: 216u 216v 216w][block 1: 216u 216v 216w]...[block n: 216u 216v 216w]
            filepos = (int64_t)iRealHeaderSize + 
                      (int64_t)aBlocksPerFile[iCurrVarProc]*iBinaryOffset*iPrecision + //the header and mesh if one exists
                      (int64_t)domain*nPts*iDim*iPrecision;
        if (iPrecision == 4)
        {
            float *tmppts = new float[nPts*iDim];

            fseek(fdVar, filepos, SEEK_SET);
            fread(tmppts, sizeof(float), nPts*iDim, fdVar);

            if (bSwapEndian)
                ByteSwap32(tmppts, nPts*iDim);
    
            for (ii = 0 ; ii < nPts ; ii++)
            {
                if (iDim == 3)
                {
                    var[ii*3 + 0] = tmppts[ii];
                    var[ii*3 + 1] = tmppts[ii+nPts];
                    var[ii*3 + 2] = tmppts[ii+nPts+nPts];
                }
                else
                {
                    var[ii*3 + 0] = tmppts[ii];
                    var[ii*3 + 1] = tmppts[ii+nPts];
                    var[ii*3 + 2] = 0.0f;
                }
            }
            delete [] tmppts;
        }
        else
        {
            double *tmppts = new double[nPts*iDim];
            fseek(fdVar, filepos, SEEK_SET);
            fread(tmppts, sizeof(double), nPts*iDim, fdVar);

            if (bSwapEndian)
                ByteSwap64(tmppts, nPts*iDim);
    
            for (ii = 0 ; ii < nPts ; ii++)
            {
                if (iDim == 3)
                {
                    var[ii*3 + 0] = (double)tmppts[ii];
                    var[ii*3 + 1] = (double)tmppts[ii+nPts];
                    var[ii*3 + 2] = (double)tmppts[ii+nPts+nPts];
                }
                else
                {
                    var[ii*3 + 0] = (double)tmppts[ii];
                    var[ii*3 + 1] = (double)tmppts[ii+nPts];
                    var[ii*3 + 2] = 0.0;
                }
            }
            delete [] tmppts;
        }
    }
    else
    {
        for (ii = 0 ; ii < nPts ; ii++)
        {
            fseek(fdVar, (int64_t)iAsciiCurrFileStart + 
                         (int64_t)domain*iAsciiCurrFileLineLen*nPts + 
                         (int64_t)ii*iAsciiCurrFileLineLen + 
                         (int64_t)iAsciiOffset, SEEK_SET);
            if (iDim == 3)
            {
                fscanf(fdVar, " %f %f %f", var, var+1, var+2);
            }
            else
            {
                fscanf(fdVar, " %f %f", var, var+1);
                var[2] = 0.0f;
            }
            var += 3;
        }
    }

    return var;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::GetCycles
//
//  Purpose:
//      Return the cycle number for each timestep
//
//  Programmer: Dave Bremer
//  Creation:   May 21, 2007
//
// ****************************************************************************

void           
avtNek5000FileFormat::GetCycles(std::vector<int> &outCycles)
{
    UpdateCyclesAndTimes();
    outCycles = aCycles;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::GetTimes
//
//  Purpose:
//      Return the simulation time for each timestep
//
//  Programmer: Dave Bremer
//  Creation:   May 21, 2007
//
// ****************************************************************************

void           
avtNek5000FileFormat::GetTimes(std::vector<double> &outTimes)
{
    UpdateCyclesAndTimes();
    outTimes = aTimes;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::GetFileName
//
//  Purpose:
//      Create a filename from the template and other info.
//
//  Programmer: Dave Bremer
//  Creation:   March 19, 2008
//
//  Modifications:
//    Dave Bremer, Fri Jun  6 15:38:45 PDT 2008
//    Added the bParFormat flag allowing the parallel format to be used
//    by a serial code, in which there is only one output dir.
//
//    Dave Bremer, Fri Aug  8 12:57:47 PDT 2008
//    Brought in a change from Stefan Kerkemeier to infer that the parallel/binary
//    format is in use if 2 or 3 printf tokens are in the file template.
// ****************************************************************************

void
avtNek5000FileFormat::GetFileName(int timestep, int pardir, char *outFileName, int bufSize)
{
    int nPrintfTokens = 0;
    int ii;

    for (ii = 0; ii < fileTemplate.size()-1; ii++)
    {
        if (fileTemplate[ii] == '%' && fileTemplate[ii+1] != '%')
            nPrintfTokens++;
    }

    if (nPrintfTokens > 1) 
    {
        bBinary = true;
        bParFormat = true;
    }

    if (!bParFormat && nPrintfTokens != 1)
    {
        EXCEPTION1(ImproperUseException, 
            "The filetemplate tag must receive only one printf token for serial Nek files.");
    }
    else if (bParFormat && (nPrintfTokens < 2 || nPrintfTokens > 3))
    {
        EXCEPTION1(ImproperUseException, 
            "The filetemplate tag must receive either 2 or 3 printf tokens for parallel Nek files.");
    }
    int len;
    if (!bParFormat)
        len = SNPRINTF(outFileName, bufSize, fileTemplate.c_str(), timestep);
    else if (nPrintfTokens == 2)
        len = SNPRINTF(outFileName, bufSize, fileTemplate.c_str(), pardir, timestep);
    else
        len = SNPRINTF(outFileName, bufSize, fileTemplate.c_str(), pardir, pardir, timestep);

    if (len >= bufSize)
        EXCEPTION1(ImproperUseException, 
            "An internal buffer was too small to hold a file name.");
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::UpdateCyclesAndTimes
//
//  Purpose:
//      Read the header for each timestep, and store the cycle number and 
//      simulation time for each.
//
//  Programmer: Dave Bremer
//  Creation:   May 21, 2007
//
//  Modifications:
//    Dave Bremer, Wed Nov  7 14:17:19 PST 2007
//    While scanning the headers, also figure out which files contain a mesh,
//    allowing one less tag in the .nek3d file.
//
//    Dave Bremer, Wed Nov 14 15:00:13 PST 2007
//    Added support for the parallel version of the file.
//
//    Dave Bremer, Thu Mar 20 11:28:05 PDT 2008
//    Use the GetFileName method.
//
//    Dave Bremer, Fri Jun  6 15:38:45 PDT 2008
//    Added the bParFormat flag allowing the parallel format to be used
//    by a serial code, in which there is only one output dir.
//
//    Dave Bremer, Mon Jun 16 18:22:43 PDT 2008
//    Small change to be more robust about finding the location of the field tags.
// ****************************************************************************

void
avtNek5000FileFormat::UpdateCyclesAndTimes()
{
    if (aCycles.size() > 0)
        return;

    aTimes.resize(iNumTimesteps);
    aCycles.resize(iNumTimesteps);

    ifstream f;
    char dummy[64];
    double t;
    int    c;
    string v;
    char *meshfilename = new char[ fileTemplate.size() + 64 ];

    int ii;
    iTimestepsWithMesh.push_back(iFirstTimestep);
    for (ii = 0 ; ii < iNumTimesteps ; ii++)
    {
        t = 0.0;
        c = 0;
        aTimes[ii] = t;
        aCycles[ii] = c;
        continue;

        GetFileName(iFirstTimestep+ii, 0, meshfilename, fileTemplate.size() + 64);
        f.open(meshfilename);

        if (!bParFormat)
        {
            f >> dummy >> dummy >> dummy >> dummy >> t >> c >> v;  //skip #blocks and block size
        }
        else
        {
            f >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy;
            f >> t >> c >> dummy;

            //I do this to skip the num directories token, because it may abut 
            //the field tags without a whitespace separator.
            while (f.peek() == ' ')
                f.get();
            while (f.peek() >= '0' && f.peek() <= '9')
                f.get();
            
            char tmpTags[32];
            f.read(tmpTags, 32);
            tmpTags[31] = '\0';

            v = tmpTags;
        }
        f.close();

        aTimes[ii] = t;
        aCycles[ii] = c;

        // If this file contains a mesh, the first variable codes after the 
        // cycle number will be X Y
        if (v.find("X") != std::string::npos)
            iTimestepsWithMesh.push_back(iFirstTimestep+ii);
    }
    delete[] meshfilename;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::GetDomainSizeAndVarOffset
//
//  Purpose:
//      For the given timestep, return the size of each domain, and the offset
//      of a given variable within each domain, in number of floats.
//
//  Programmer: Dave Bremer
//  Creation:   May 21, 2007
//
//  Modifications:
//    Dave Bremer, Wed Nov  7 14:17:19 PST 2007
//    Return sizes and offsets needed if the data is in ascii format.
//
//    Dave Bremer, Thu Nov 15 16:44:42 PST 2007
//    Moved some size computations to FindAsciiDataStart, to deal with 
//    windows-style CRLF
// ****************************************************************************

void
avtNek5000FileFormat::GetDomainSizeAndVarOffset(int iTimestep, const char *var, 
                                              int &outDomSizeInFloats, 
                                              int &outVarOffsetBinary,
                                              int &outVarOffsetAscii,
                                              int &outTimestepHasMesh )
{
    int ii;
    outTimestepHasMesh = 0;

    UpdateCyclesAndTimes();   //Needs to call this to update iTimestepsWithMesh

    for (ii = 0 ; ii < iTimestepsWithMesh.size() ; ii++)
    {
        if (iTimestepsWithMesh[ii] == iTimestep)
        {
            outTimestepHasMesh = 1;
            break;
        }
    }
    const int nPts = iBlockSize[0]*iBlockSize[1]*iBlockSize[2];

    int nFloatsPerSample = 0;
    if (outTimestepHasMesh)
        nFloatsPerSample += iDim;
    if (bHasVelocity)
        nFloatsPerSample += iDim;
    if (bHasPressure)
        nFloatsPerSample += 1;
    if (bHasTemperature)
        nFloatsPerSample += 1;
    nFloatsPerSample += iNumSFields;

    outDomSizeInFloats = nFloatsPerSample * nPts;

    if (var)
    {
        int iNumPrecedingFloats = 0;
        if (STREQUAL(var, "velocity")==0  || 
            STREQUAL(var, "x_velocity")==0)
        {
            if (outTimestepHasMesh)
                iNumPrecedingFloats += iDim;
        }
        else if (STREQUAL(var, "y_velocity")==0)
        {
            if (outTimestepHasMesh)
                iNumPrecedingFloats += iDim;

            iNumPrecedingFloats += 1;
        }
        else if (STREQUAL(var, "z_velocity")==0)
        {
            if (outTimestepHasMesh)
                iNumPrecedingFloats += iDim;

            iNumPrecedingFloats += 2;
        }
        else if (STREQUAL(var, "pressure")==0)
        {
            if (outTimestepHasMesh)
                iNumPrecedingFloats += iDim;
            if (bHasVelocity)
                iNumPrecedingFloats += iDim;
        }
        else if (STREQUAL(var, "temperature")==0)
        {
            if (outTimestepHasMesh)
                iNumPrecedingFloats += iDim;
            if (bHasVelocity)
                iNumPrecedingFloats += iDim;
            if (bHasPressure)
                iNumPrecedingFloats += 1;
        }
        else if (var[0] == 's')
        {
            if (outTimestepHasMesh)
                iNumPrecedingFloats += iDim;
            if (bHasVelocity)
                iNumPrecedingFloats += iDim;
            if (bHasPressure)
                iNumPrecedingFloats += 1;
            if (bHasTemperature)
                iNumPrecedingFloats += 1;

            int iSField = atoi(var+1);
            //iSField should be between 1..iNumSFields, inclusive
            iNumPrecedingFloats += iSField-1;
        }
        outVarOffsetBinary = nPts*iNumPrecedingFloats;
        outVarOffsetAscii  = 14*iNumPrecedingFloats;
    }
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::ByteSwap32
//
//  Purpose:
//      Swap endian for an array of 32-bit entities
//
//  Programmer: Dave Bremer
//  Creation:   May 21, 2007
//
// ****************************************************************************

void
avtNek5000FileFormat::ByteSwap32(void *aVals, int nVals)
{
    char *v = (char *)aVals;
    char tmp;
    for (int ii = 0 ; ii < nVals ; ii++, v+=4)
    {
        tmp = v[0]; v[0] = v[3]; v[3] = tmp;
        tmp = v[1]; v[1] = v[2]; v[2] = tmp;
    }
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::ByteSwap64
//
//  Purpose:
//      Swap endian for an array of 64-bit entities
//
//  Programmer: Dave Bremer
//  Creation:   Nov 8, 2007
//
// ****************************************************************************

void
avtNek5000FileFormat::ByteSwap64(void *aVals, int nVals)
{
    char *v = (char *)aVals;
    char tmp;
    for (int ii = 0 ; ii < nVals ; ii++, v+=8)
    {
        tmp = v[0]; v[0] = v[7]; v[7] = tmp;
        tmp = v[1]; v[1] = v[6]; v[6] = tmp;
        tmp = v[2]; v[2] = v[5]; v[5] = tmp;
        tmp = v[3]; v[3] = v[4]; v[4] = tmp;
    }
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::FindAsciiDataStart
//
//  Purpose:
//      If the data is ascii format, there is a certain amount of deprecated
//      data to skip at the beginning of each file.  This method tells where to
//      seek to, to read data for the first block.
//
//  Programmer: Dave Bremer
//  Creation:   Wed Nov  7 14:17:19 PST 2007
//
//  Modifications:
//    Dave Bremer, Thu Nov 15 16:44:42 PST 2007
//    Calculate line length, in case windows-style CRLF is used.
// ****************************************************************************

void
avtNek5000FileFormat::FindAsciiDataStart(FILE *fd, int &outDataStart, int &outLineLen)
{
    //Skip the header, then read a float for each block.  Then skip beyond the
    //newline character and return the current position.
    fseek(fd, iHeaderSize, SEEK_SET);
    for (int ii = 0 ; ii < iNumBlocks ; ii++)
    {
        float dummy;
        fscanf(fd, " %f", &dummy);
    }
    char tmp[1024];
    fgets(tmp, 1023, fd);
    outDataStart = ftell(fd);

    fgets(tmp, 1023, fd);
    outLineLen = ftell(fd) - outDataStart;
}



// ****************************************************************************
//  Method: avtNek5000FileFormat::GetAuxiliaryData
//
//  Purpose:
//      Get the auxiliary data.
// 
//  Notes:
//      Everything outside this plugin considers the data to be a single block.
//      So we will return interval trees for a single block.  This is basically
//      to communicate bounds type information to places like the contour
//      (for the n-levels option) and the slice (for the percent for the origin
//      option).
//
//  Programmer: Hank Childs
//  Creation:   January 5, 2009
//
// ****************************************************************************

void *
avtNek5000FileFormat::GetAuxiliaryData(const char *var, int timestep,
                                     int  /*domain*/, const char *type, void *,
                                     DestructorFunction &df)
{
    void *rv = NULL;

    if (strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0)
    {
        if (strcmp(var, "mesh") != 0)
        {
            EXCEPTION1(InvalidVariableException, var);
        }

        avtIntervalTree *internalTree = GetBoundingBoxIntervalTree(
                                                      timestepToUseForMesh);
        if (internalTree != NULL)
        {
            double bounds[6];
            internalTree->GetExtents(bounds);
            avtIntervalTree *externalTree = new avtIntervalTree(1, 3);
            externalTree->AddElement(0, bounds);
            externalTree->Calculate(true);
            rv = (void *) externalTree;
            df = avtIntervalTree::Destruct;
        }
    }
    else
    {
        avtIntervalTree *internalTree=GetDataExtentsIntervalTree(
                                                             timestep,var);
        if (internalTree != NULL)
        {
            double extents[2];
            internalTree->GetExtents(extents);
            avtIntervalTree *externalTree = new avtIntervalTree(1, 1);
            externalTree->AddElement(0, extents);
            externalTree->Calculate(true);
            rv = (void *) externalTree;
            df = avtIntervalTree::Destruct;
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::GetBoundingBoxIntervalTree
//
//  Purpose:
//      Return the interval tree containing the bounding boxes for each 
//      element.
//
//  Notes:
//     In the Nek5000 file, if there is meta-data, it is stored at the end of 
//     the file as:
//        [element 1 X min, max]
//        [element 1 Y min, max]
//        [element 1 Z min, max]
//        ..
//        [element N X min, max]
//        [element N Y min, max]
//        [element N Z min, max]
//        [element 1 U min, max]
//        [element 1 V min, max]
//        [element 1 W min, max]
//        ..
//        [element N U min, max]
//        [element N V min, max]
//        [element N W min, max]
//        [element 1 P min, max]
//        ..
//        [element N P min, max]
//        [element 1 T min, max]
//        ..
//        [element N T min, max]
//        [element 1 S1 min, max]
//        ..
//        [element N S1 min, max]
//        ..
//        [element 1 Sk min, max]
//        ..
//        [element N Sk min, max]
//
//  Programmer: Dave Bremer
//  Creation:   Wed Apr 23 18:12:50 PDT 2008
//
//  Modifications:
//    Dave Bremer, Fri Jun  6 15:38:45 PDT 2008
//    Added the bParFormat flag allowing the parallel format to be used
//    by a serial code, in which there is only one output dir.
//
//    Dave Bremer, Thu Jun 12 12:59:23 PDT 2008
//    Support varying numbers of blocks per file in the parallel format.
//    Fixed a bug with block numbering.  When USE_SIMPLE_BLOCK_NUMBERING
//    was on, the spatial bounds were matched to the wrong blocks.
//
//    Hank Childs, Thu Dec 18 06:38:51 PST 2008
//    Renamed method as this is now an internal routine and not part associated
//    with the virtual method GetAuxiliaryData.
//    
// ****************************************************************************

avtIntervalTree *
avtNek5000FileFormat::GetBoundingBoxIntervalTree(int timestep)
{
    int t1 = visitTimer->StartTimer();
    std::map<int,avtIntervalTree*>::const_iterator fit = boundingBoxes.find(timestep);
    if (fit != boundingBoxes.end())
        return fit->second;

    if (!bBinary || !bParFormat)
        return NULL;

    ReadBlockLocations();

    int nFloatsPerDomain = 0, d1, d2, d3;
    GetDomainSizeAndVarOffset(timestep, NULL, 
                              nFloatsPerDomain, d1, d2, d3 );
    
    int iRank = 0, nProcs = 1;
#ifdef PARALLEL
    MPI_Comm_rank(VISIT_MPI_COMM, &iRank);
    MPI_Comm_size(VISIT_MPI_COMM, &nProcs);
#endif
    
    ifstream f;
    int ii, jj;

    int errorReadingData = 0;

    char  *blockfilename = new char[ fileTemplate.size() + 64 ];

    float *bounds = new float[iNumBlocks*6];
    for (ii = 0; ii < iNumBlocks*6; ii++)
        bounds[ii] = 0.0f;

    for (ii = iRank; ii < iNumOutputDirs; ii+=nProcs)
    {
        int64_t iFileSizeWithoutMetaData = 136 
                + sizeof(int)*aBlocksPerFile[ii] 
                + ((int64_t)nFloatsPerDomain)*sizeof(float)*((int64_t)aBlocksPerFile[ii]);

        int64_t iMDSize = (nFloatsPerDomain * 2 * sizeof(float) * aBlocksPerFile[ii]) / 
                    (iBlockSize[0]*iBlockSize[1]*iBlockSize[2]);

        GetFileName(timestep, ii, blockfilename, fileTemplate.size() + 64);
        f.open(blockfilename);
        if (!f.is_open())
        {
            errorReadingData = 1;
            break;
        }
        f.seekg( 0, std::ios_base::end );
        int64_t iFileSize = f.tellg();
        if (iFileSize < iFileSizeWithoutMetaData+iMDSize)
        {
            errorReadingData = 1;
            break;
        }

#ifdef USE_SIMPLE_BLOCK_NUMBERING
        int nPrecedingBlocks = 0;
        for (jj = 0; jj < ii; jj++)
            nPrecedingBlocks += aBlocksPerFile[jj];

        f.seekg( iFileSizeWithoutMetaData, std::ios_base::beg );

        f.read( (char *)(bounds + nPrecedingBlocks*6), aBlocksPerFile[ii]*6*sizeof(float) );
        if (bSwapEndian)
            ByteSwap32(bounds + nPrecedingBlocks*6, aBlocksPerFile[ii]*6);
#else
        int   *tmpBlocks = new int[aBlocksPerFile[ii]];
        float *tmpBounds = new float[aBlocksPerFile[ii]*6];

        f.seekg( 136, std::ios_base::beg );
        f.read( (char *)tmpBlocks, aBlocksPerFile[ii]*sizeof(int) );
        if (bSwapEndian)
            ByteSwap32(tmpBlocks, aBlocksPerFile[ii]);

        for (jj = 0; jj < aBlocksPerFile[ii]; jj++)
            tmpBlocks[jj]--;
        f.seekg( iFileSizeWithoutMetaData, std::ios_base::beg );
        f.read( (char *)tmpBounds, aBlocksPerFile[ii]*6*sizeof(float) );
        if (bSwapEndian)
            ByteSwap32(tmpBounds, aBlocksPerFile[ii]*6);

        for (jj = 0; jj < aBlocksPerFile[ii]; jj++)
            memcpy(bounds + tmpBlocks[jj]*6, tmpBounds+jj*6, 6*sizeof(float));

        delete[] tmpBlocks;
        delete[] tmpBounds;
#endif
        f.close();
    }
    delete[] blockfilename;

#ifdef PARALLEL
    //See if any proc had a read error.
    int  anyErrorReadingData = 0;
    MPI_Allreduce(&errorReadingData, &anyErrorReadingData, 1, 
                  MPI_INT, MPI_BOR, VISIT_MPI_COMM);
    errorReadingData = anyErrorReadingData;
#endif
    if (errorReadingData)
        return NULL;

#ifdef PARALLEL
        float *mergedBounds = new float[iNumBlocks*6];
    
        MPI_Allreduce(bounds, mergedBounds, iNumBlocks*6, 
                      MPI_FLOAT, MPI_SUM, VISIT_MPI_COMM);
        delete[] bounds;
        bounds = mergedBounds;
#endif
    avtIntervalTree *itree = new avtIntervalTree(iNumBlocks, 3);

    for (ii = 0; ii < iNumBlocks; ii++)
    {
        double b[6];
        b[0] = (double)bounds[ii*6];
        b[1] = (double)bounds[ii*6+1];
        b[2] = (double)bounds[ii*6+2];
        b[3] = (double)bounds[ii*6+3];
        b[4] = (double)bounds[ii*6+4];
        b[5] = (double)bounds[ii*6+5];
        itree->AddElement(ii, b);
    }
    itree->Calculate(true);
    delete[] bounds;

    boundingBoxes[timestep] = itree;
    visitTimer->StopTimer(t1, "Reading interval tree");
    return itree;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::GetDataExtentsIntervalTree
//
//  Purpose:
//      Return the interval tree containing the data extents for each 
//      element.
//
//  Notes:
//     In the Nek5000 file, if there is meta-data, it is stored at the end of 
//     the file as:
//        [element 1 X min, max]
//        [element 1 Y min, max]
//        [element 1 Z min, max]
//        ..
//        [element N X min, max]
//        [element N Y min, max]
//        [element N Z min, max]
//        [element 1 U min, max]
//        [element 1 V min, max]
//        [element 1 W min, max]
//        ..
//        [element N U min, max]
//        [element N V min, max]
//        [element N W min, max]
//        [element 1 P min, max]
//        ..
//        [element N P min, max]
//        [element 1 T min, max]
//        ..
//        [element N T min, max]
//        [element 1 S1 min, max]
//        ..
//        [element N S1 min, max]
//        ..
//        [element 1 Sk min, max]
//        ..
//        [element N Sk min, max]
//
//  Programmer: Hank Childs
//  Creation:   December 18, 2008
//    
// ****************************************************************************

avtIntervalTree *
avtNek5000FileFormat::GetDataExtentsIntervalTree(int timestep, const char *var)
{
    PointerKey key;
    key.var = var;
    key.element = -1;
    key.timestep = timestep;
    std::map<PointerKey,avtIntervalTree*, KeyCompare>::const_iterator fit = dataExtents.find(key);
    if (fit != dataExtents.end())
        return fit->second;

    ReadBlockLocations();

    bool foundVar = false;
    bool doVelocityVar = false;
    int  velocityComp = -1;
    bool doPressureVar = false;
    bool doTemperatureVar = false;
    bool doSFieldVar = false;
    int  sComp = -1;
    int numVars = 0;
    if (bHasPressure)
    {
        if (strcmp(var, "pressure") == 0)
            doPressureVar = true;
        numVars++;
    }
    if (bHasTemperature)
    {
        if (strcmp(var, "temperature") == 0)
            doTemperatureVar = true;
        numVars++;
    }
    if (bHasVelocity)
    {
        if (strcmp(var, "x_velocity") == 0)
        {
            doVelocityVar = true;
            velocityComp = 0;
        }
        if (strcmp(var, "y_velocity") == 0)
        {
            doVelocityVar = true;
            velocityComp = 1;
        }
        if (strcmp(var, "z_velocity") == 0)
        {
            doVelocityVar = true;
            velocityComp = 2;
        }
        numVars += iDim;
    }
    numVars += iNumSFields;
    for (int ss = 0; ss < iNumSFields; ss++)
    {
        char sfieldname[256];
        sprintf(sfieldname, "s%d", ss+1);
        if (strcmp(var, sfieldname) == 0)
        {
            doSFieldVar = true;
            sComp = ss;
        }
    }

    if (!doVelocityVar && !doPressureVar && !doTemperatureVar && !doSFieldVar)
    {
        return NULL;
    }

    if (!bBinary || !bParFormat)
        return NULL;

    int nFloatsPerDomain = 0, d1, d2, d3;
    GetDomainSizeAndVarOffset(timestep, NULL, 
                              nFloatsPerDomain, d1, d2, d3 );

    int iRank = 0, nProcs = 1;
#ifdef PARALLEL
    MPI_Comm_rank(VISIT_MPI_COMM, &iRank);
    MPI_Comm_size(VISIT_MPI_COMM, &nProcs);
#endif
    
    ifstream f;
    int ii, jj;

    int errorReadingData = 0;

    char  *blockfilename = new char[ fileTemplate.size() + 64 ];

    float *bounds = new float[iNumBlocks*2];
    for (ii = 0; ii < iNumBlocks*2; ii++)
        bounds[ii] = 0.0f;

    for (ii = iRank; ii < iNumOutputDirs; ii+=nProcs)
    {
        int64_t iFileSizeWithoutMetaData = 136 
                + sizeof(int)*aBlocksPerFile[ii] 
                + ((int64_t)nFloatsPerDomain)*sizeof(float)*((int64_t)aBlocksPerFile[ii]);

        int64_t iBBSize = 2*iDim * sizeof(float) * aBlocksPerFile[ii];
        int64_t iDESize = 2 * sizeof(float) * aBlocksPerFile[ii] * numVars;
        int64_t iMDSize = iBBSize + iDESize;

        GetFileName(timestep, ii, blockfilename, fileTemplate.size() + 64);
        f.open(blockfilename);
        if (!f.is_open())
        {
            errorReadingData = 1;
            break;
        }
        f.seekg( 0, std::ios_base::end );
        int64_t iFileSize = f.tellg();
        if (iFileSize != iFileSizeWithoutMetaData+iMDSize)
        {
            iBBSize = 0;
            iMDSize = iDESize;
            if (iFileSize != iFileSizeWithoutMetaData+iMDSize)
            {
                errorReadingData = 1;
                break;
            }
        }

#ifdef USE_SIMPLE_BLOCK_NUMBERING
        int nPrecedingBlocks = 0;
        for (jj = 0; jj < ii; jj++)
            nPrecedingBlocks += aBlocksPerFile[jj];

        if (doVelocityVar)
        {
            f.seekg(iFileSizeWithoutMetaData+iBBSize, std::ios_base::beg);
            float *tmp = new float[aBlocksPerFile[ii]*6];
            f.read( (char *)(tmp), aBlocksPerFile[ii]*6*sizeof(float) );
            for (int jj = 0 ; jj < aBlocksPerFile[ii] ; jj++)
            {
                bounds[nPrecedingBlocks*2 + 2*jj] = tmp[6*jj+2*velocityComp];
                bounds[nPrecedingBlocks*2 + 2*jj+1] = tmp[6*jj+2*velocityComp+1];
            }
            delete [] tmp;
        }
        else
        {
            int varIndex = -1;
            if (doPressureVar)
                varIndex = (bHasVelocity ? iDim : 0); 
            else if (doTemperatureVar)
                varIndex = (bHasVelocity ? iDim : 0) + (bHasPressure ? 1 : 0);
            else
                varIndex = (bHasVelocity ? iDim : 0) + (bHasPressure ? 1 : 0) + 
                       (bHasTemperature ? 1 : 0) + sComp;
            int64_t offsetForDE = varIndex*2*sizeof(float)*aBlocksPerFile[ii];
            f.seekg(iFileSizeWithoutMetaData+iBBSize+offsetForDE, std::ios_base::beg);
            f.read( (char *)(bounds + nPrecedingBlocks*2), aBlocksPerFile[ii]*2*sizeof(float) );
        }

        if (bSwapEndian)
            ByteSwap32(bounds + nPrecedingBlocks*2, aBlocksPerFile[ii]*2);
#else
      IMPLEMENT THIS LATER!
#endif
        f.close();
    }
    delete[] blockfilename;

#ifdef PARALLEL
    //See if any proc had a read error.
    int  anyErrorReadingData = 0;
    MPI_Allreduce(&errorReadingData, &anyErrorReadingData, 1, 
                  MPI_INT, MPI_BOR, VISIT_MPI_COMM);
    errorReadingData = anyErrorReadingData;
#endif
    if (errorReadingData)
        return NULL;

#ifdef PARALLEL
    float *mergedBounds = new float[iNumBlocks*2];

    MPI_Allreduce(bounds, mergedBounds, iNumBlocks*2, 
                  MPI_FLOAT, MPI_SUM, VISIT_MPI_COMM);
    delete[] bounds;
    bounds = mergedBounds;
#endif
    avtIntervalTree *itree = new avtIntervalTree(iNumBlocks, 1);

    for (ii = 0; ii < iNumBlocks; ii++)
    {
        double b[2];
        b[0] = (double)bounds[ii*2];
        b[1] = (double)bounds[ii*2+1];
        itree->AddElement(ii, b);
    }
    itree->Calculate(true);
    delete[] bounds;

    dataExtents.insert(std::pair<PointerKey, avtIntervalTree *>(key, itree));
    return itree;
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::RegisterDataSelections
//
//  Purpose:
//      Looks at the registered data selections and decides which elements
//      to use.
//
//  Programmer: Hank Childs
//  Creation:   December 18, 2008
//
// ****************************************************************************

void
avtNek5000FileFormat::RegisterDataSelections(
                               const std::vector<avtDataSelection_p> &selList,
                               std::vector<bool> *selectionsApplied)
{
    int  i;

    vector<vector<int> > domainsToUse(selList.size());

    // Special logic for the case where there are no matches ...
    // otherwise it would appear like we didn't match the selection
    // and needed to use all elements.
    bool noMatches = false;

    int t1 = visitTimer->StartTimer();
    for (int i = 0 ; i < selList.size() ; i++)
    {
        if (string(selList[i]->GetType()) == "Spatial Box Data Selection")
        {
            avtSpatialBoxSelection *sel=(avtSpatialBoxSelection*)*(selList[i]);

            double mins[3], maxs[3];
            sel->GetMins(mins);
            sel->GetMaxs(maxs);
            avtSpatialBoxSelection::InclusionMode imode =
                sel->GetInclusionMode();

            avtIntervalTree *itree = 
                              GetBoundingBoxIntervalTree(timestepToUseForMesh);
            if (itree == NULL)
                continue;

            itree->GetElementsListFromRange(mins, maxs, domainsToUse[i]);
            if (domainsToUse[i].size() == 0)
                noMatches = true;
        }
        if (string(selList[i]->GetType()) == "Plane Selection")
        {
            avtPlaneSelection *sel = (avtPlaneSelection *) *(selList[i]);

            double normal[3], origin[3];
            sel->GetNormal(normal);
            sel->GetOrigin(origin);
            avtIntervalTree *itree = 
                              GetBoundingBoxIntervalTree(timestepToUseForMesh);
            if (itree == NULL)
                continue;

            double D = normal[0]*origin[0] + normal[1]*origin[1] +
                     normal[2]*origin[2];
            itree->GetElementsList(normal, D, domainsToUse[i]);
            if (domainsToUse[i].size() == 0)
                noMatches = true;
        }
        if (string(selList[i]->GetType()) == "Isolevels Selection")
        {
            avtIsolevelsSelection *sel = (avtIsolevelsSelection*)*(selList[i]);

            std::string var = sel->GetVariable();
            std::vector<double> isolevels = sel->GetIsolevels();

            avtIntervalTree *itree = 
                              GetDataExtentsIntervalTree(curTimestep, 
                                                         var.c_str());
            if (itree == NULL)
                continue;

            double eqn[1] = { 1. };
            std::vector<std::vector<int> > elemsForLevelJ(isolevels.size());
            for (int j = 0 ; j < isolevels.size() ; j++)
            {
                itree->GetElementsList(eqn, isolevels[j], elemsForLevelJ[j]);
            }
            CombineElementLists(elemsForLevelJ, domainsToUse[i], true);
            if (domainsToUse[i].size() == 0)
                noMatches = true;
        }
    }
    visitTimer->StopTimer(t1, "Getting element lists for each selection");

    vector<int> finalElementList;
    bool useAllElements;
    if (noMatches)
    {
        useAllElements = false;
        // Note that finalElementList will be empty, which is what we want.
    }
    else
    {
        bool haveSelection = false;
        for (int i = 0 ; i < domainsToUse.size() ; i++)
            if (domainsToUse[i].size() > 0)
                haveSelection = true;

        if (haveSelection)
        {
            CombineElementLists(domainsToUse, finalElementList, false);
            useAllElements = false;
        }
        else
        {
            useAllElements = true;
        }
    }

    int t2 = visitTimer->StartTimer();
    int rank   = PAR_Rank();
    int nprocs = PAR_Size();

    int nelements = (useAllElements ? iNumBlocks : finalElementList.size());
    int elements_per_proc = nelements / nprocs;
    int one_extra_until = nelements % nprocs;
    int my_num_elements = elements_per_proc + (rank < one_extra_until ? 1 : 0);
    int my_start = elements_per_proc*rank + (rank < one_extra_until ? rank : one_extra_until);
    int my_end = my_start + my_num_elements;
    myElementList.resize(my_num_elements);
    for (i = my_start ; i < my_end ; i++)
    {
        myElementList[i-my_start] = (useAllElements ? i : finalElementList[i]);
    }
    if (!useAllElements)
        std::sort(myElementList.begin(), myElementList.end());
    visitTimer->StopTimer(t2, "Assigning elements to processors");
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::CombineElementLists
//
//  Purpose:
//      Combine lists of elements, either by taking their union or their
//      intersection.
//
//  Programmer: Hank Childs
//  Creation:   January 6, 2009
//
// ****************************************************************************

void
avtNek5000FileFormat::CombineElementLists(
                      const std::vector<std::vector<int> > &lists,
                      std::vector<int> &outlist, bool doUnion)
{
    int t1 = visitTimer->StartTimer();

    int  i, j;

    if (lists.size() == 1)
    {
        outlist = lists[0];
        return;
    }

    if (doUnion)
    {
        std::vector<bool> useElements(iNumBlocks, false);
        for (i = 0 ; i < lists.size() ; i++)
        {
            for (j = 0 ; j < lists[i].size() ; j++)
            {
                useElements[lists[i][j]] = true;
            }
        }
        
        int numOn = 0;
        for (i = 0 ; i < iNumBlocks ; i++)
            if (useElements[i])
                numOn++;
        outlist.resize(numOn);
        int ctr = 0;
        for (i = 0 ; i < iNumBlocks ; i++)
            if (useElements[i])
                outlist[ctr++] = i;
    }
    else
    {
        std::vector<int> numHits(iNumBlocks, 0);
        int numSelections = 0;
        for (i = 0 ; i < lists.size() ; i++)
        {
            if (lists[i].size() == 0)
                continue;
            numSelections++;
            for (j = 0 ; j < lists[i].size() ; j++)
                numHits[lists[i][j]]++;
        }
        
        int numOn = 0;
        for (i = 0 ; i < iNumBlocks ; i++)
            if (numHits[i] == numSelections)
                numOn++;
        outlist.resize(numOn);
        int ctr = 0;
        for (i = 0 ; i < iNumBlocks ; i++)
            if (numHits[i] == numSelections)
                outlist[ctr++] = i;
    }

    visitTimer->StopTimer(t1, "Combining element lists from data selections");
}


// ****************************************************************************
//  Method: avtNek5000FileFormat::ActivateTimestep
//
//  Purpose:
//      Tells the reader which timestep should be used.
//
//  Programmer: Hank Childs
//  Creation:   December 18, 2008
//
//  Modifications:
//
//    Hank Childs, Wed Jan  7 16:54:58 CST 2009
//    Delete cached data when we go to a new time slice.
//
// ****************************************************************************

void
avtNek5000FileFormat::ActivateTimestep(int ts)
{
    std::map<PointerKey, float *>::iterator it;
    std::map<PointerKey, float *, KeyCompare> new_cachedData;
    for (it = cachedData.begin() ; it != cachedData.end() ; it++)
    {
        if (it->first.timestep != ts && it->first.var != "points")
            delete [] it->second;
        else
            new_cachedData[it->first] = it->second;
    }
    cachedData = new_cachedData;

    std::map<PointerKey, avtIntervalTree *>::iterator it2;
    std::map<PointerKey, avtIntervalTree *, KeyCompare> new_dataExtents;
    for (it2 = dataExtents.begin() ; it2 != dataExtents.end() ; it2++)
    {
        if (it2->first.timestep != ts)
            delete it2->second;
        else
            new_dataExtents[it2->first] = it2->second;
    }
    dataExtents = new_dataExtents;

    curTimestep = ts;
    timestepToUseForMesh = 0;
}


