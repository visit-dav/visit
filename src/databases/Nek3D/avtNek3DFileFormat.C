/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtNek3DFileFormat.C                           //
// ************************************************************************* //

#include <avtNek3DFileFormat.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif

#include <string>
#include <iostream>
#include <fstream>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <Expression.h>

#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h>


using     std::string;
#ifndef STREQUAL
#if defined(_WIN32) 
#  define STREQUAL(a,b)              stricmp(a,b)
#else
#  define STREQUAL(a,b)              strcasecmp(a,b)
#endif
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
// The header becomes 132 chars.  Here are two example headers:
// #std 4  6  6  6   120  240  0.1500E+01  300  1  2XUPT
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
// for each variable in U, P, and any misc scalars
//     for each block
//         for each sample in the block
//             write the sample
// ****************************************************************************









// ****************************************************************************
//  Method: avtNek3D constructor
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
// ****************************************************************************

avtNek3DFileFormat::avtNek3DFileFormat(const char *filename)
    : avtMTMDFileFormat(filename)
{
    version = "1.0";
    bSwapEndian = false;
    fileTemplate = "";
    iFirstTimestep = 1;
    iNumTimesteps = 1;
    bBinary = true;
    iNumOutputDirs = 1;

    iNumBlocks = 1;
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
    iHeaderSize = 84;
    iDim = 3;
    iPrecision = 4;
    aBlockLocs = NULL;

    string tag, buf2;
    char buf[2048];
    ifstream  f(filename);
    int ii, jj;

    // Verify that the 'magic' and version number are right
    f >> tag;
    while (tag[0] == '#')
    {
        f.getline(buf, 2048);
        f >> tag;
    }
    if (STREQUAL("NEK3D", tag.c_str()) != 0)
    {
        EXCEPTION1(InvalidDBTypeException, "Not a Nek3D file." );
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
        EXCEPTION1(InvalidDBTypeException, "Not a Nek3D file." );
    }
    if (version != "1.0")
    {
        EXCEPTION1(InvalidDBTypeException, 
                   "Only Nek3D version 1.0 is supported." );
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
            string  endianness;
            f >> endianness;

            //This tag is deprecated.  There's a float written into each binary file
            //from which endianness can be determined.
            /*
            bool bDataIsBigEndian;

            if (STREQUAL("big", endianness.c_str())==0)
            {
                bDataIsBigEndian = true;
            }
            else if (STREQUAL("little", endianness.c_str())==0)
            {
                bDataIsBigEndian = false;
            }
            else
            {
                EXCEPTION1(InvalidDBTypeException, 
                   "Value following endian: must be \"big\" or \"little\"" );
            }
            int iTest = 1;
            char *pTest = (char *)(&iTest);
            if ((*pTest == 1) && bDataIsBigEndian)
                bSwapEndian = true;
            else if ((*pTest == 0) && !bDataIsBigEndian)
                bSwapEndian = true;
            */
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
            string t;
            f >> t;
            if (STREQUAL("binary", t.c_str())==0)
            {
                bBinary = true;
            }
            else if (STREQUAL("ascii", t.c_str())==0)
            {
                bBinary = false;
            }
            else
            {
                EXCEPTION1(InvalidDBTypeException, 
                   "Value following type: must be \"binary\" or \"ascii\"" );
            }
        }
        else if (STREQUAL("numoutputdirs:", tag.c_str())==0)
        {
            f >> iNumOutputDirs;
        }
        else
        {
            sprintf(buf, "Error parsing file.  Unknown tag %s", tag.c_str());
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

    //Now read the header out of one the files to get block and variable info
    char *blockfilename = new char[ fileTemplate.size() + 64 ];
    if (iNumOutputDirs == 1)
        sprintf(blockfilename, fileTemplate.c_str(), iFirstTimestep);
    else
        sprintf(blockfilename, fileTemplate.c_str(), 0, 0, iFirstTimestep);
    f.open(blockfilename);

    if (iNumOutputDirs == 1)
    {
        f >> iNumBlocks;
        f >> iBlockSize[0];
        f >> iBlockSize[1];
        f >> iBlockSize[2];
    
        f >> buf2;   //skip
        f >> buf2;   //skip
    
        while (1)
        {
            f >> tag;
            if (tag == "X" || tag == "Y" || tag == "Z")
                continue;
            else if (tag == "U")
                bHasVelocity = true;
            else if (tag == "P")
                bHasPressure = true;
            else if (tag == "T")
                bHasTemperature = true;
            else if (tag == "1")
            {
                char c;
                iNumSFields++;
    
                //From this file pos, figure out how long the sequence is.
                //Starting with 1, we'll have a sequence of chars separated by
                //a single space.  "1" or "1 2 3" for example.  This sequence
                //is also the last possible sequence, hence the break after
                //the while loop.
                while (1)
                {
                    f.read(&c, 1);
                    if (c == ' ' && f.peek() != ' ')
                    {
                        f >> tag;
                        if ( atoi(tag.c_str()) == iNumSFields+1 )
                        {
                            iNumSFields++;
                        }
                        else
                            break;
                    }
                    else
                        break;
                }
                break;
            }
            else
                break;
        }
    }
    else
    {
        //Here's are two examples of what I'm parsing:
        //#std 4  6  6  6   120  240  0.1500E+01  300  1  2XUPT
        //#std 4  6  6  6   120  240  0.1500E+01  300  1  2 U T123
        //This example means:  #std is for versioning, 4 bytes per sample, 6x6x6 blocks, 
        //  120 of 240 blocks are in this file, time=1.5, cycle=300, 
        //  this output dir=1, num output dirs=2, XUPT123 are tags that this file has a mesh, 
        //  velocity, pressure, temperature, and 3 misc scalars.
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
        f >> buf2;        //blocks per file
        f >> iNumBlocks;
        /*
        f >> buf2;        //time
        f >> buf2;        //cycle
        f >> buf2;        //MPI rank
        while (f.get() == ' ')  //read past the first char of the next tag
            ;
        f.get();         //skip the mesh code
        */
        //This bypasses some tricky and unnecessary parsing of data
        //I already have.
        f.seekg(77, std::ios_base::beg);

        if (f.get() == 'U')
            bHasVelocity = true;
        if (f.get() == 'P')
            bHasPressure = true;
        if (f.get() == 'T')
            bHasTemperature = true;
        while ((f.get()-'0') == (iNumSFields+1))
            iNumSFields++;
    }
    if (iBlockSize[2] == 1)
        iDim = 2;
    
    if (iNumOutputDirs > 1  &&  iNumBlocks%iNumOutputDirs != 0)
    {
        EXCEPTION1(InvalidDBTypeException, 
            "Parallel Nek reader requires an equal number of blocks per file." );
    }
    iBlocksPerFile = iNumBlocks/iNumOutputDirs;


    if (bBinary)
    {
        // Compute the size of the header.  In all cases so far, the header size
        // for binary files has been 84.
        int iDomainSize, d1, d2, d3, d4, d5;
        GetDomainSizeAndVarOffset(iFirstTimestep, NULL, iDomainSize, d1, d2, d3, d4, d5);
    
        f.seekg( -iBlocksPerFile*iDomainSize*iPrecision, std::ios_base::end );
        iHeaderSize = (int)f.tellg();

        // Determine endianness and whether we need to swap bytes.
        // If this machine's endian matches the file's, the read will 
        // put 6.54321 into this float.
        float test;  
        if (iNumOutputDirs == 1)
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

        // In each parallel file, in the header, there's a table that maps 
        // each local block to a global id which starts at 1.  Here, I make 
        // an inverse map, from a zero-based global id to a proc num and local
        // offset.
        if (iNumOutputDirs > 1)
        {
            f.close();
            aBlockLocs = new int[2*iNumBlocks];
            for (ii = 0; ii < 2*iNumBlocks; ii++)
            {
                aBlockLocs[ii] = -999;
            }
            int *tmpBlocks = new int[iBlocksPerFile];
            for (ii = 0; ii < iNumOutputDirs; ii++)
            {
                sprintf(blockfilename, fileTemplate.c_str(), ii, ii, iFirstTimestep);
                f.open(blockfilename);
                if (!f.is_open())
                {
                    char msg[1024];
                    sprintf(msg, "Could not open file %s.", filename);
                    EXCEPTION1(InvalidDBTypeException, msg);
                }
                f.seekg( 136, std::ios_base::beg );
                f.read( (char *)tmpBlocks, iBlocksPerFile*sizeof(int) );
                f.close();

                for (jj = 0; jj < iBlocksPerFile; jj++)
                {
                    int iBlockID = tmpBlocks[jj]-1;

                    if ( iBlockID < 0  ||  
                         iBlockID >= iNumBlocks ||
                         aBlockLocs[iBlockID*2]   != -999  ||
                         aBlockLocs[iBlockID*2+1] != -999 )
                    {
                        EXCEPTION1(InvalidDBTypeException, 
                                   "Error reading parallel file block IDs.");
                    }
                    aBlockLocs[iBlockID*2  ] = ii;
                    aBlockLocs[iBlockID*2+1] = jj;
                }
            }
            delete[] tmpBlocks;
        }
    }
    else
    {
        iHeaderSize = 80;
    }

    delete[] blockfilename;
    if (f.is_open())
        f.close();
}


// ****************************************************************************
//  Method: avtNek3DFileFormat destructor
//
//  Programmer: David Bremer
//  Creation:   Wed Nov 14 11:35:30 PST 2007
//
// ****************************************************************************

avtNek3DFileFormat::~avtNek3DFileFormat()
{
    FreeUpResources();
    if (aBlockLocs)
    {
        delete[] aBlockLocs;
        aBlockLocs = NULL;
    }
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
avtNek3DFileFormat::GetNTimesteps(void)
{
    return iNumTimesteps;
}


// ****************************************************************************
//  Method: avtNek3DFileFormat::FreeUpResources
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
// ****************************************************************************

void
avtNek3DFileFormat::FreeUpResources(void)
{
    // Closing this file descriptor might not be necessary here.  Maybe can
    // be done in the destructor.  It doesn't matter here, but may be important
    // if we cache the whole mesh in memory.
    if (fdMesh)
    {
        fclose(fdMesh);
        fdMesh = NULL;
    }
    if (fdVar)
    {
        fclose(fdVar);
        fdVar = NULL;
    }
    iCurrTimestep = -999;
    iCurrMeshProc = -999;
    iCurrVarProc  = -999;
    iAsciiMeshFileStart = -999;
    iAsciiCurrFileStart = -999;
}


// ****************************************************************************
//  Method: avtNek3DFileFormat::PopulateDatabaseMetaData
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
// ****************************************************************************

void
avtNek3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int /*timeState*/)
{
    string meshname = "mesh";
    double *extents = NULL;
    AddMeshToMetaData(md, meshname, AVT_CURVILINEAR_MESH, extents, iNumBlocks, 1, iDim, iDim);
    
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
        sprintf(scalarVarName, "s%d", ii+1);
        AddScalarVarToMetaData(md, scalarVarName, meshname, AVT_NODECENT);
    }
}


// ****************************************************************************
//  Method: avtNek3DFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
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
// ****************************************************************************

vtkDataSet *
avtNek3DFileFormat::GetMesh(int /*timestate*/, int domain, const char */*meshname*/)
{
    vtkStructuredGrid    *grid   = vtkStructuredGrid::New();
    vtkPoints            *points  = vtkPoints::New();
    const int             nPts = iBlockSize[0]*iBlockSize[1]*iBlockSize[2];
    int ii;

    grid->SetPoints(points);
    points->Delete();
    grid->SetDimensions(iBlockSize);
    points->SetNumberOfPoints( nPts );

    float *pts = (float *)points->GetVoidPointer(0);

    UpdateCyclesAndTimes();   //This call also finds which timesteps have a mesh.

    if (fdMesh == NULL || 
        (iNumOutputDirs > 1 && aBlockLocs[domain*2] != iCurrMeshProc))
    {
        if (fdMesh)
            fclose(fdMesh);

        char *meshfilename = new char[ fileTemplate.size() + 256 ];

        if (iNumOutputDirs == 1)
        {
            sprintf(meshfilename, fileTemplate.c_str(), iTimestepsWithMesh[0]);
        }
        else
        {
            iCurrMeshProc = aBlockLocs[domain*2];
            sprintf(meshfilename, fileTemplate.c_str(), 
                    iCurrMeshProc, iCurrMeshProc, iTimestepsWithMesh[0]);
        }
        fdMesh = fopen(meshfilename, "rb");
        if (!fdMesh)
            EXCEPTION1(InvalidFilesException, meshfilename);
        delete [] meshfilename;

        if (!bBinary)
            iAsciiMeshFileStart = FindAsciiDataStart(fdMesh);
    }

    if (iNumOutputDirs > 1)
        domain = aBlockLocs[domain*2 + 1];

    int nFloatsInDomain = 0, nBytesInDomain = 0, nBytesInSample = 0, d1, d2, d3;
    GetDomainSizeAndVarOffset(iTimestepsWithMesh[0], NULL, nFloatsInDomain, 
                              nBytesInDomain, nBytesInSample, d1, d2, d3);

    if (bBinary)
    {
        //In the parallel format, the whole mesh comes before all the vars.
        if (iNumOutputDirs > 1)
            nFloatsInDomain = iDim*iBlockSize[0]*iBlockSize[1]*iBlockSize[2];

        if (iPrecision == 4)
        {
            float *tmppts = new float[nPts*iDim];
            fseek(fdMesh, iHeaderSize + nFloatsInDomain*sizeof(float)*domain, SEEK_SET);
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
            fseek(fdMesh, iHeaderSize + nFloatsInDomain*sizeof(double)*domain, SEEK_SET);
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
            fseek(fdMesh, iAsciiMeshFileStart + domain*nBytesInDomain + ii*nBytesInSample, SEEK_SET);
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
    return grid;
}


// ****************************************************************************
//  Method: avtNek3DFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
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
// ****************************************************************************

vtkDataArray *
avtNek3DFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    int iTimestep = timestate + iFirstTimestep;
    const int nPts = iBlockSize[0]*iBlockSize[1]*iBlockSize[2];
    int ii;

    if (iTimestep != iCurrTimestep || 
        (iNumOutputDirs > 1 && aBlockLocs[domain*2] != iCurrVarProc))
    {
        if (fdVar)
            fclose(fdVar);

        char *filename = new char[ fileTemplate.size() + 256 ];
        if (iNumOutputDirs == 1)
        {
            sprintf(filename, fileTemplate.c_str(), iTimestep);
        }
        else
        {
            iCurrVarProc = aBlockLocs[domain*2];
            sprintf(filename, fileTemplate.c_str(), 
                    iCurrVarProc, iCurrVarProc, iTimestep);
        }
        fdVar = fopen(filename, "rb");
        if (!fdVar)
            EXCEPTION1(InvalidFilesException, filename);
        delete[] filename;

        iCurrTimestep = iTimestep;
        if (!bBinary)
            iAsciiCurrFileStart = FindAsciiDataStart(fdVar);
    }

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(nPts);
    float *var = (float *)rv->GetVoidPointer(0);

    int nFloatsInDomain = 0, nBytesInDomain = 0, nBytesInSample = 0, 
        iBinaryOffset = 0, iAsciiOffset = 0, iHasMesh = 0;

    GetDomainSizeAndVarOffset(iTimestep, varname, nFloatsInDomain, 
                              nBytesInDomain, nBytesInSample, 
                              iBinaryOffset, iAsciiOffset, iHasMesh);
    if (iNumOutputDirs > 1)
        domain = aBlockLocs[domain*2 + 1];

    if (bBinary)
    {
        int filepos;
        if (iNumOutputDirs == 1)
            filepos = iHeaderSize + (nFloatsInDomain*domain + iBinaryOffset)*sizeof(float);
        else
        {
            // This assumes uvw for all fields comes after the mesh as [block0: 216u 216v 216w]...
            // then p or t as   [block0: 216p][block1: 216p][block2: 216p]...
            if (strcmp(varname+2, "velocity") == 0)
            {
                filepos  = iHeaderSize;                                  //header
                filepos += iHasMesh*iBlocksPerFile*nPts*iDim*iPrecision; //mesh
                filepos += domain*nPts*iDim*iPrecision;                  //start of block
                filepos += (varname[0] - 'x')*nPts*iPrecision;           //position within block
            }
            else
                filepos = iHeaderSize + iBlocksPerFile*iBinaryOffset*iPrecision + //the header, mesh, vel if present,
                          domain*nPts*iPrecision;
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
            fseek(fdVar, iAsciiCurrFileStart + domain*nBytesInDomain + ii*nBytesInSample + iAsciiOffset, SEEK_SET);
            fscanf(fdVar, " %f", var);
            var++;
        }
    }
    return rv;
}


// ****************************************************************************
//  Method: avtNek3DFileFormat::GetVectorVar
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
// ****************************************************************************

vtkDataArray *
avtNek3DFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
    int iTimestep = timestate + iFirstTimestep;
    const int nPts = iBlockSize[0]*iBlockSize[1]*iBlockSize[2];
    int ii;

    if (iTimestep != iCurrTimestep || 
        (iNumOutputDirs > 1 && aBlockLocs[domain*2] != iCurrVarProc))
    {
        if (fdVar)
            fclose(fdVar);

        char *filename = new char[ fileTemplate.size() + 256 ];
        if (iNumOutputDirs == 1)
        {
            sprintf(filename, fileTemplate.c_str(), iTimestep);
        }
        else
        {
            iCurrVarProc = aBlockLocs[domain*2];
            sprintf(filename, fileTemplate.c_str(), 
                    iCurrVarProc, iCurrVarProc, iTimestep);
        }
        fdVar = fopen(filename, "rb");
        if (!fdVar)
            EXCEPTION1(InvalidFilesException, filename);
        delete[] filename;

        iCurrTimestep = iTimestep;
        if (!bBinary)
            iAsciiCurrFileStart = FindAsciiDataStart(fdVar);
    }

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(3);
    rv->SetNumberOfTuples(nPts);
    float *pts = (float *)rv->GetVoidPointer(0);

    int nFloatsInDomain = 0, nBytesInDomain = 0, nBytesInSample = 0, 
        iBinaryOffset = 0, iAsciiOffset = 0, dummy;

    GetDomainSizeAndVarOffset(iTimestep, varname, nFloatsInDomain, 
                              nBytesInDomain, nBytesInSample, 
                              iBinaryOffset, iAsciiOffset, dummy);
    if (iNumOutputDirs > 1)
        domain = aBlockLocs[domain*2 + 1];

    if (bBinary)
    {
        int filepos;
        if (iNumOutputDirs == 1)
            filepos = iHeaderSize + (nFloatsInDomain*domain + iBinaryOffset)*sizeof(float);
        else
        {
            //This assumes [block 0: 216u 216v 216w][block 1: 216u 216v 216w]...[block n: 216u 216v 216w]
            filepos = iHeaderSize + iBlocksPerFile*iBinaryOffset*iPrecision + //the header and mesh if one exists
                      domain*nPts*iDim*iPrecision;
        }
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
            fseek(fdVar, filepos, SEEK_SET);
            fread(tmppts, sizeof(double), nPts*iDim, fdVar);

            if (bSwapEndian)
                ByteSwap64(tmppts, nPts*iDim);
    
            for (ii = 0 ; ii < nPts ; ii++)
            {
                if (iDim == 3)
                {
                    pts[ii*3 + 0] = (double)tmppts[ii];
                    pts[ii*3 + 1] = (double)tmppts[ii+nPts];
                    pts[ii*3 + 2] = (double)tmppts[ii+nPts+nPts];
                }
                else
                {
                    pts[ii*3 + 0] = (double)tmppts[ii];
                    pts[ii*3 + 1] = (double)tmppts[ii+nPts];
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
            fseek(fdVar, iAsciiCurrFileStart + domain*nBytesInDomain + ii*nBytesInSample + iAsciiOffset, SEEK_SET);
            if (iDim == 3)
            {
                fscanf(fdVar, " %f %f %f", pts, pts+1, pts+2);
            }
            else
            {
                fscanf(fdVar, " %f %f", pts, pts+1);
                pts[2] = 0.0f;
            }
            pts += 3;
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtNek3DFileFormat::GetCycles
//
//  Purpose:
//      Return the cycle number for each timestep
//
//  Programmer: Dave Bremer
//  Creation:   May 21, 2007
//
// ****************************************************************************

void           
avtNek3DFileFormat::GetCycles(std::vector<int> &outCycles)
{
    UpdateCyclesAndTimes();
    outCycles = aCycles;
}


// ****************************************************************************
//  Method: avtNek3DFileFormat::GetTimes
//
//  Purpose:
//      Return the simulation time for each timestep
//
//  Programmer: Dave Bremer
//  Creation:   May 21, 2007
//
// ****************************************************************************

void           
avtNek3DFileFormat::GetTimes(std::vector<double> &outTimes)
{
    UpdateCyclesAndTimes();
    outTimes = aTimes;
}


// ****************************************************************************
//  Method: avtNek3DFileFormat::UpdateCyclesAndTimes
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
// ****************************************************************************

void
avtNek3DFileFormat::UpdateCyclesAndTimes()
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
    for (ii = 0 ; ii < iNumTimesteps ; ii++)
    {
        t = 0.0;
        c = 0;

        if (iNumOutputDirs == 1)
        {
            sprintf(meshfilename, fileTemplate.c_str(), iFirstTimestep+ii);
            f.open(meshfilename);
            f >> dummy >> dummy >> dummy >> dummy >> t >> c >> v;  //skip #blocks and block size
        }
        else
        {
            sprintf(meshfilename, fileTemplate.c_str(), 0, 0, iFirstTimestep+ii);
            f.open(meshfilename);
            f >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy;
            f >> t >> c >> dummy;
            while (f.get() == ' ')  //read past the first char of the next tag
                ;
            v = f.get();
        }
        f.close();

        aTimes[ii] = t;
        aCycles[ii] = c;

        // If this file contains a mesh, the first variable codes after the 
        // cycle number will be X Y
        if (v == "X")
            iTimestepsWithMesh.push_back(iFirstTimestep+ii);
    }
    delete[] meshfilename;
}


// ****************************************************************************
//  Method: avtNek3DFileFormat::GetDomainSizeAndVarOffset
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
// ****************************************************************************

void
avtNek3DFileFormat::GetDomainSizeAndVarOffset(int iTimestep, const char *var, 
                                              int &outDomSizeInFloats, 
                                              int &outDomSizeInBytes, 
                                              int &outSampleSizeInBytes, 
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

    outDomSizeInFloats   = nFloatsPerSample * nPts;       //For binary
    outSampleSizeInBytes = 14*nFloatsPerSample + 1;       //For ascii
    outDomSizeInBytes    = outSampleSizeInBytes * nPts;   //For ascii

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
//  Method: avtNek3DFileFormat::ByteSwap32
//
//  Purpose:
//      Swap endian for an array of 32-bit entities
//
//  Programmer: Dave Bremer
//  Creation:   May 21, 2007
//
// ****************************************************************************

void
avtNek3DFileFormat::ByteSwap32(void *aVals, int nVals)
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
//  Method: avtNek3DFileFormat::ByteSwap64
//
//  Purpose:
//      Swap endian for an array of 64-bit entities
//
//  Programmer: Dave Bremer
//  Creation:   Nov 8, 2007
//
// ****************************************************************************

void
avtNek3DFileFormat::ByteSwap64(void *aVals, int nVals)
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
//  Method: avtNek3DFileFormat::FindAsciiDataStart
//
//  Purpose:
//      If the data is ascii format, there is a certain amount of deprecated
//      data to skip at the beginning of each file.  This method tells where to
//      seek to, to read data for the first block.
//
//  Programmer: Dave Bremer
//  Creation:   Wed Nov  7 14:17:19 PST 2007
//
// ****************************************************************************

int
avtNek3DFileFormat::FindAsciiDataStart(FILE *fd)
{
    //Skip the header, then read a float for each block.  Then skip beyond the
    //newline character and return the current position.
    fseek(fd, iHeaderSize, SEEK_SET);
    for (int ii = 0 ; ii < iNumBlocks ; ii++)
    {
        float dummy;
        fscanf(fd, " %f", &dummy);
    }
    char tmp[256];
    fgets(tmp, 255, fd);
    return ftell(fd);
}



