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
//                         avtBoxlibFileFormat.C                             //
// ************************************************************************* //


// **************************************************************************//
//  Reader: Boxlib
// 
//  Current format for a .boxlib file is a bit bogus.  There is currently
//  no mechanism for supporting MTMD with changing SILs.  So Boxlib is a STMD.
//  This makes filenames a bit tough (you have to have one .boxlib3d file per
//  timestep).  So the contents of a boxlib file are not used.  They just
//  should be in the same directory as the Header file.
// 
//  Notes:                             
//     There are certain assumptions that this reader currently makes that
//  should be noted if this reader is extended in the future to support
//  the more general Boxlib framework.
//
//   -> Ghost zones are currently not supported, and a warning is issued
//      if a dataset is found to have ghost zones. This is due to lack
//      of sample datasets, and should be removed once appropriate data
//      is avaliable.
//
//   Code is currently in place to check these assumptions where possible,
//   and issue warnings or throw exceptions as appropriate.
//                              
// **************************************************************************//

// It is necessary to define BL_USE_MPI for Boxlib to avoid collision of
// type definitions for MPI_Comm in Boxlib header files with MPI library
#ifdef PARALLEL
#define BL_USE_MPI 1
#endif

#include <avtBoxlibFileFormat.h>

#include <vector>
#include <string>
#include <visitstream.h>
#include <visit-config.h>
#include <snprintf.h>

#include <ctype.h>
#include <stdlib.h>

#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>

#include <avtCallback.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtIntervalTree.h>
#include <avtMaterial.h>
#include <avtParallel.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtStructuredDomainNesting.h>
#include <avtVariableCache.h>

#include <BadDomainException.h>
#include <BadIndexException.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <InvalidDBTypeException.h>

#include <VisMF.H>

using std::vector;
using std::string;

static string GetDirName(const char *path);
static void   EatUpWhiteSpace(ifstream &in);
static int    VSSearch(const vector<string> &, const string &); 

// ****************************************************************************
//  Function: GetCycleFromRootPath
//
//  Programmer: Akira Haddox
//  Creation:   July 25, 2003
//
//  Modifications:
//
//    Mark C. Miller, Tue Nov  8 21:14:00 PST 2005
//    Refactored from constructor 
// ****************************************************************************
static int GetCycleFromRootPath(const std::string &rpath)
{
    int cyc = 0;
    const char *cur = rpath.c_str();
    const char *last = NULL;
    while (cur != NULL)
    {
        cur = strstr(cur, "plt");
        if (cur != NULL)
        {
            last = cur;
            cur = cur+1;
        }
    }
    if (last != NULL)
    {
        cyc = atoi(last + strlen("plt"));
        return cyc;
    }

    return avtFileFormat::INVALID_CYCLE;
}

// ****************************************************************************
//  Constructor:  avtBoxlibFileFormat::avtBoxlibFileFormat
//
//  Arguments:
//    fname      the file name of the .boxlib file
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Nov  6 09:25:17 PST 2003
//    Modified to work for one timestep only.
//
//    Hank Childs, Sun Feb 13 13:53:14 PST 2005
//    Do not require to be in a "plt" directory.
//
//    Hank Childs, Sun Mar  6 16:21:15 PST 2005
//    Add support for GeoDyne material names.
//
//    Hank Childs, Thu Jun 23 14:39:04 PDT 2005
//    Initialize haveReadTimeAndCycle.
//
//    Mark C. Miller, Wed Nov  9 12:35:15 PST 2005
//    Moved code to parse cycle to GetCycleFromRootPath
//
//    Hank Childs, Tue Feb 19 14:20:22 PST 2008
//    Initialize nMaterials.
//
// ****************************************************************************

avtBoxlibFileFormat::avtBoxlibFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
    // The root path is the boxlib name.  This needs to change.
    rootPath = GetDirName(fname);

    cycle = GetCycleFromRootPath(rootPath);

    static const char *t ="";
    timestepPath = t;

    initializedReader = false;
    vf_names_for_materials = false;
    time = 0.;
    haveReadTimeAndCycle = false;
    nMaterials = 0;
}


// ****************************************************************************
//  Destructor:  avtBoxlibFileFormat::~avtBoxlibFileFormat
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
// ****************************************************************************

avtBoxlibFileFormat::~avtBoxlibFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method avtBoxlibFileFormat::GetCycleFromFilename
//
//  Programmer:  Mark C. Miller
//  Creation:    November 8, 2005
// ****************************************************************************
int
avtBoxlibFileFormat::GetCycleFromFilename(const char *f) const
{
    return GetCycleFromRootPath(GetDirName(f));
}

// ****************************************************************************
//  Method: avtBoxlibFileFormat::ActivateTimestep
//
//  Purpose:
//      Calls InitializeReader, which does collective communication.
//
//  Programmer: Hank Childs
//  Creation:   June 23, 2005
//
// ****************************************************************************

void
avtBoxlibFileFormat::ActivateTimestep(void)
{
    InitializeReader();
}


// ****************************************************************************
//  Method: avtBoxlibFileFormat::InitializeReader
//
//  Purpose:
//      Initializes the reader.
//
//  Programmer: Hank Childs
//  Creation:   November 6, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Nov 15 14:23:46 PST 2003
//    Do a better job of parsing material numbers.
//
//    Hank Childs, Tue Nov 18 23:31:00 PST 2003
//    Do not assume that this routine is called only one time (we may call
//    free up resources and then come back to this timestep).
//
//    Hank Childs, Sat Nov 22 08:18:12 PST 2003
//    Calculate the domain nesting information when initializing the reader.
//    This way that information will be available when we re-visit timesteps
//    (because we will get it, the DB will clear it, and, now we will get
//    it again).
//
//    Hank Childs, Sun Mar  6 16:21:15 PST 2005
//    Add support for GeoDyne material names.
//
// ****************************************************************************

void
avtBoxlibFileFormat::InitializeReader(void)
{
    int  i;

    if (initializedReader)
        return;

    initializedReader = true;

    //
    // Read in the time header.  This will set up some of our arrays.
    //
    ReadHeader();

    mfReaders.resize(multifabFilenames.size(), NULL);
    
    //
    // Now that we have the varNames and the multifabFilenames,
    // find which variable is in which file.
    // 
    int count = 0;
    int level = 0;
    for (i = 0; i < multifabFilenames.size(); ++i)
    {
        VisMF *vmf = GetVisMF(i);
        int cnt = vmf->nComp();

        //
        // Determine what type of centering it is.
        //
        const IndexType &ix = vmf->boxArray()[0].ixType();
        bool isNode = true;
        bool isCell = true;

        int k;
        for (k = 0; k < dimension; ++k)
        {
            if (ix.test(k))
                isCell = false;
            else
                isNode = false;
        }

        int type = isNode ? AVT_NODECENT :
                   isCell ? AVT_ZONECENT :
                            AVT_UNKNOWN_CENT;
    
        int j;
        for (j = 0; j < cnt; ++j)
        {
            componentIds[level][j + count] = j;
            fabfileIndex[level][j + count] = i;
            if (level == 0)
                varCentering[j + count] = type;
        }

        count += cnt;
        if (count >= nVars)
        {
            count -= nVars;
            ++level;
        }
    } 

    //
    // varUsedElsewhere is used to denote scalar variables that we don't
    // want to expose as scalar variables, since we are using them elsewhere.
    //
    varUsedElsewhere.clear();
    for (i = 0 ; i < nVars ; i++)
    {
        varUsedElsewhere.push_back(false);
    }

    //
    // Find any materials
    //
    nMaterials = 0;
    for (i = 0; i < nVars; ++i)
    {
        int val = 0;
        if (varNames[i].find("frac") == 0)
        {
            varUsedElsewhere[i] = true;
            int val = atoi(varNames[i].c_str()+4);

            if (val > nMaterials)
                nMaterials = val;
        }
    }
    if (nMaterials == 0)
    {
        for (i = 0; i < nVars; ++i)
        {
            int val = 0;
            if (varNames[i].find("vf_") == 0)
            {
                varUsedElsewhere[i] = true;
                int val = atoi(varNames[i].c_str()+3);

                if (val > nMaterials)
                    nMaterials = val;
                vf_names_for_materials = true;
            }
        }
    }
    
    //
    // Find possible vectors by combining scalars with {x,y,z} prefixes and
    // suffixes.
    // 
    nVectors = 0;
    vectorNames.clear();
    for (i = 0; i < nVars; ++i)
    {
        int id2 = -1;
#if BL_SPACEDIM==3
        int id3 = -1;
#endif
        string needle = varNames[i];
        bool startsWithFirst = false;
        bool foundVector = false;
        if (varNames[i][0] == 'x')
        {
            needle[0] = 'y';
            id2 = VSSearch(varNames, needle);
            if (id2 == -1)
                continue;
            
#if BL_SPACEDIM==3
            needle[0] = 'z';
            id3 = VSSearch(varNames, needle);
            if (id3 == -1)
                continue;
#endif
            
            startsWithFirst = true;
            foundVector = true;
        }
        int lastChar = strlen(needle.c_str())-1;
        if (!foundVector && (varNames[i][lastChar] == 'x'))
        {
            needle[lastChar] = 'y';
            id2 = VSSearch(varNames, needle);
            if (id2 == -1)
                continue;
            
#if BL_SPACEDIM==3
            needle[lastChar] = 'z';
            id3 = VSSearch(varNames, needle);
            if (id3 == -1)
                continue;
#endif

            startsWithFirst = false;
            foundVector = true;
        }

        if (foundVector)
        {
            // Ensure they're all the same type of centering.
            if (varCentering[i] == AVT_UNKNOWN_CENT)
                continue;
            if (varCentering[i] != varCentering[id2])
                continue;
#if BL_SPACEDIM==3
            if (varCentering[i] != varCentering[id3])
                continue;
#endif

            int index = nVectors;
            ++nVectors;
            if (startsWithFirst)
            {
                if (needle.length() > 1)
                    vectorNames.push_back(needle.substr(1, needle.length() - 1));
                else
#if BL_SPACEDIM==2
                    vectorNames.push_back(varNames[i]+varNames[id2]+"_vec");
#elif BL_SPACEDIM==3
                    vectorNames.push_back(varNames[i]+varNames[id2]+varNames[id3]+"_vec");
#endif
            }
            else
            {
                if (needle.length() > 1)
                    vectorNames.push_back(needle.substr(0, needle.length() - 1));
                else
#if BL_SPACEDIM==2
                    vectorNames.push_back(varNames[i]+varNames[id2]+"_vec");
#elif BL_SPACEDIM==3
                    vectorNames.push_back(varNames[i]+varNames[id2]+varNames[id3]+"_vec");
#endif
            }
            vectorCentering.push_back(varCentering[i]);

            vectorComponents.resize(nVectors);
            vectorComponents[index].resize(dimension);
            vectorComponents[index][0] = i;
            varUsedElsewhere[i]   = true;
            vectorComponents[index][1] = id2;
            varUsedElsewhere[id2] = true;
#if BL_SPACEDIM==3
            vectorComponents[index][2] = id3;
            varUsedElsewhere[id3] = true;
#endif
        }
    }

    if (!avtDatabase::OnlyServeUpMetaData())
    {
        CalculateDomainNesting();
    }
}


// ****************************************************************************
//  Method:  avtBoxlibFileFormat::GetGlobalPatchNumber
//
//  Purpose:
//      Gets the global patch number from the level and local patch number.
//
//  Programmer: Hank Childs
//  Creation:   November 6, 2003
//
// ****************************************************************************

int
avtBoxlibFileFormat::GetGlobalPatchNumber(int level, int local_patch) const
{
    if (level < 0 || level >= nLevels)
    {
        EXCEPTION2(BadIndexException, level, nLevels);
    }

    int rv = 0;
    for (int i = 0 ; i < level ; i++)
    {
        rv += patchesPerLevel[i];
    }
    rv += local_patch;

    return rv;
}


// ****************************************************************************
//  Method: avtBoxlibFileFormat::GetLevelAndLocalPatchNumber
//
//  Purpose:
//      Gets the level and local patch number from the global patch number.
//
//  Programmer: Hank Childs
//  Creation:   November 6, 2003
//
// ****************************************************************************

void
avtBoxlibFileFormat::GetLevelAndLocalPatchNumber(int global_patch,
                                            int &level, int &local_patch) const
{
    int tmp = global_patch;
    level = 0;
    while (1)
    {
        if (tmp < patchesPerLevel[level])
        {
            break;
        }
        tmp -= patchesPerLevel[level];
        level++;
    }
    local_patch = tmp;
}


// ****************************************************************************
//  Method:  avtBoxlibFileFormat::GetMesh
//
//  Purpose:
//    Returns the mesh with the given name for the given patch number.
//
//  Arguments:
//    dom        the domain number
//    mesh_name  The name of the mesh.
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Nov  6 09:49:33 PST 2003
//    Removed all notions of timesteps.
//
//    Hank Childs, Wed Feb 18 11:29:59 PST 2004
//    Add a base index.
//
// ****************************************************************************

vtkDataSet *
avtBoxlibFileFormat::GetMesh(int patch, const char *mesh_name)
{
    if (strcmp(mesh_name, "Mesh") != 0)
        EXCEPTION1(InvalidVariableException, mesh_name);

    if (!initializedReader)
        InitializeReader();

    int level, local_patch;
    GetLevelAndLocalPatchNumber(patch, level, local_patch);

    if (level >= nLevels)
    {
        EXCEPTION1(InvalidVariableException, mesh_name);
    }

    if (local_patch >= patchesPerLevel[level])
    {
        EXCEPTION2(BadDomainException, local_patch, patchesPerLevel[level]);
    }

    double lo[BL_SPACEDIM], hi[BL_SPACEDIM], delta[BL_SPACEDIM];
    lo[0] = xMin[patch];
    hi[0] = xMax[patch];
    delta[0] = deltaX[level];

    lo[1] = yMin[patch];
    hi[1] = yMax[patch];
    delta[1] = deltaY[level];

#if BL_SPACEDIM==3
    lo[2] = zMin[patch];
    hi[2] = zMax[patch];
    delta[2] = deltaZ[level];
#endif

    vtkDataSet *rv = CreateGrid(lo, hi, delta);

    //
    // Determine the indices of the mesh within its group.  Add that to the
    // VTK dataset as field data.
    //
    double epsilonX = deltaX[level] / 8.0;
    double epsilonY = deltaY[level] / 8.0;
#if BL_SPACEDIM==3
    double epsilonZ = deltaZ[level] / 8.0;
#endif
    int iStart = ((int) ((xMin[patch]-probLo[0]+epsilonX) 
                       / deltaX[level]));
    int jStart = ((int) ((yMin[patch]-probLo[1]+epsilonY) 
                       / deltaY[level]));
#if BL_SPACEDIM==3
    int kStart = ((int) ((zMin[patch]-probLo[2]+epsilonZ) 
                       / deltaZ[level]));
#endif
    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(3);
    arr->SetValue(0, iStart);
    arr->SetValue(1, jStart);
#if BL_SPACEDIM==3
    arr->SetValue(2, kStart);
#endif
    arr->SetName("base_index");
    rv->GetFieldData()->AddArray(arr);
    arr->Delete();

    return rv;
}


// ****************************************************************************
//  Method:  avtBoxlibFileFormat::ReadHeader
//
//  Purpose:
//    Reads in the header information for a time step and generate
//    meshes for all patches and levels from that information.
//    Meshes are not generated if populate is set to true.
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Sep  8 16:04:01 PDT 2003
//    While parsing header, a string was returned on the DECs when it was
//    actually EOF, leading to a bad value in the multiFabFilenames.  I put
//    in a check for this and ignored the string in this case.
//
//    Hank Childs, Thu Nov  6 09:49:33 PST 2003
//    Stripped out knowledge of timesteps.  Removed arguments since this is
//    now always called in InitializeReader.
//
//    Hank Childs, Tue Nov 18 23:31:00 PST 2003
//    Do not assume that this routine is called only one time (we may call
//    free up resources and then come back to this timestep).
//
//    Hank Childs, Fri Apr  2 06:45:03 PST 2004
//    g++-3.0.4 does bogus parsing of doubles, so do it ourselves.
//
//    Hank Childs, Wed Jul  7 11:28:30 PDT 2004
//    LBNL refinement ratios are funky, so infer them at a separate spot.
//
//    Hank Childs, Sat Sep 18 08:58:23 PDT 2004
//    Replace commas in variable names with underscores.
//
//    Hank Childs, Thu Jun 23 11:16:52 PDT 2005
//    Have proc. 0 read the Header and then broadcast the info to the other 
//    procs.
//
//    Brad Whitlock, Thu Sep 22 13:34:14 PST 2005
//    Fixed on win32.
//
//    Hank Childs, Thu Feb 21 16:17:07 PST 2008
//    Initialize variables in case parsing fails (Klocwork).
//
// ****************************************************************************

void
avtBoxlibFileFormat::ReadHeader(void)
{
    int failure = 1;
    int success = 0;
    int status = success;

    bool iDoReading = false;
    if (PAR_Rank() == 0)
        iDoReading = true;

    ifstream in;
    string double_tmp;

    string headerFilename = rootPath + timestepPath + SLASH_STRING + "Header";

    if (iDoReading)
        in.open(headerFilename.c_str());

    if (in.fail())
        status = failure;

    BroadcastInt(status);

    if (status == failure)
        EXCEPTION1(InvalidFilesException, headerFilename.c_str());

    int integer=0;
    char buf[1024];
    if (iDoReading)
    {
        // Read in version
        in.getline(buf, 1024);
        // Read in nVars
        in >> integer;

        nVars = integer;
    }

    BroadcastInt(nVars);
    varNames.resize(nVars);
    varCentering.resize(nVars);

   
    int i;
    if (iDoReading)
    {
        EatUpWhiteSpace(in);
        for (i = 0; i < nVars; ++i)
        {
            in.getline(buf, 1024); // Read in var names
    
            // Replace commas with underscores.
            int len = strlen(buf);
            for (int j = 0 ; j < len ; j++)
                if (buf[j] == ',')
                    buf[j] = '_';
    
            varNames[i] = buf;
        }
    }
    BroadcastStringVector(varNames, PAR_Rank());

    // Read in dimension
    if (iDoReading)
        in >> integer;
    BroadcastInt(integer);
    if (dimension != integer)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg),
#if BL_SPACEDIM==2
            "Got dimension=%d! This reader only handles 2D files.", integer);
#elif BL_SPACEDIM==3
            "Got dimension=%d! This reader only handles 3D files.", integer);
#endif
        EXCEPTION1(InvalidDBTypeException,msg);
    }

    // Read in time
    if (iDoReading)
        in >> double_tmp;
    time = atof(double_tmp.c_str());
    BroadcastDouble(time);
    haveReadTimeAndCycle = true;
    if (metadata != NULL)
    {
        metadata->SetTime(timestep, time);
        metadata->SetCycle(timestep, cycle);
    }

    // Read in number of levels for this timestep.
    if (iDoReading)
        in >> nLevels;
    ++nLevels;
    BroadcastInt(nLevels);

    patchesPerLevel.resize(nLevels);
    fabfileIndex.resize(nLevels);
    componentIds.resize(nLevels);

    for (i = 0; i < nLevels; ++i)
    {
        fabfileIndex[i].resize(nVars);
        componentIds[i].resize(nVars);
    }

    // Read the problem size
    if (iDoReading)
    {
        for (i = 0; i < dimension; ++i)
        {
            in >> double_tmp;
            probLo[i] = atof(double_tmp.c_str());
        }
        for (i = 0; i < dimension; ++i)
        {
            in >> double_tmp;
            probHi[i] = atof(double_tmp.c_str());
        }
    }
    BroadcastDouble(probLo[0]);
    BroadcastDouble(probHi[0]);
    BroadcastDouble(probLo[1]);
    BroadcastDouble(probHi[1]);
#if BL_SPACEDIM==3
    BroadcastDouble(probLo[2]);
    BroadcastDouble(probHi[2]);
#endif

    if (iDoReading)
        EatUpWhiteSpace(in);

    // Now finish off the rest of the refinement ratio line.
    int levI;
    if (iDoReading)
    {
        if (nLevels != 1)
            in.getline(buf, 1024);

        // Read in the problem domain for this level
        in.getline(buf, 1024);
        // Read in the levelsteps
        in.getline(buf, 1024);

        // For each level, read in the gridSpacing
        deltaX.clear();
        deltaY.clear();
#if BL_SPACEDIM==3
        deltaZ.clear();
#endif
        for (levI = 0; levI < nLevels; levI++)
        {
            in >> double_tmp;
            deltaX.push_back(atof(double_tmp.c_str()));
            in >> double_tmp;
            deltaY.push_back(atof(double_tmp.c_str()));
#if BL_SPACEDIM==3
            in >> double_tmp;
            deltaZ.push_back(atof(double_tmp.c_str()));
#endif
        }
    }
    BroadcastDoubleVector(deltaX, PAR_Rank());
    BroadcastDoubleVector(deltaY, PAR_Rank());
#if BL_SPACEDIM==3
    BroadcastDoubleVector(deltaZ, PAR_Rank());
#endif

    refinement_ratio.clear();
    for (levI = 1 ; levI < nLevels ; levI++)
    {
        int tmp = (int) (deltaX[levI-1] / (deltaX[levI]*1.01));
        tmp += 1;
        refinement_ratio.push_back(tmp);
    }

    // Read in coord system;
    if (iDoReading)
        in >> integer;
    // Read in width of boundary regions (ghost zones)
    if (iDoReading)
    {
        in >> integer;
        if (integer)
        {
            avtCallback::IssueWarning(
                              "Reader does not currently support ghostzones.");
        }
    }

    // For each level
    xMin.clear();
    xMax.clear();
    yMin.clear();
    yMax.clear();
#if BL_SPACEDIM==3
    zMin.clear();
    zMax.clear();
#endif
    multifabFilenames.clear();
    if (iDoReading)
    {
        for (levI = 0; levI < nLevels; levI++)
        {
            // Read in which level
            int myLevel=0;
            in >> myLevel;
    
            // Read in the number of patches
            int myNPatch=0;
            in >> myNPatch;
            patchesPerLevel[levI] = myNPatch;
    
            // Read in the time (again)
            in >> double_tmp;
            //time = atof(double_tmp.c_str());
    
            // Read in iLevelSteps
            in >> integer;
    
            // For each patch, read the spatial extents.
            for (i = 0; i < myNPatch; ++i)
            {
                in >> double_tmp;
                xMin.push_back(atof(double_tmp.c_str()));
                in >> double_tmp;
                xMax.push_back(atof(double_tmp.c_str()));
                in >> double_tmp;
                yMin.push_back(atof(double_tmp.c_str()));
                in >> double_tmp;
                yMax.push_back(atof(double_tmp.c_str()));
#if BL_SPACEDIM==3
                in >> double_tmp;
                zMin.push_back(atof(double_tmp.c_str()));
                in >> double_tmp;
                zMax.push_back(atof(double_tmp.c_str()));
#endif
            }
    
            EatUpWhiteSpace(in);
            // Read in the MultiFab files (Until we hit an int or eof)
            for (;;)
            {
                if (isdigit(in.peek()) || in.eof() || in.fail())
                    break;
                in.getline(buf, 1024);
                if (strcmp(buf, "") == 0)
                    continue;
                multifabFilenames.push_back(buf);
            }
        }
    }

    BroadcastIntVector(patchesPerLevel, PAR_Rank());
    BroadcastDoubleVector(xMin, PAR_Rank());
    BroadcastDoubleVector(xMax, PAR_Rank());
    BroadcastDoubleVector(yMin, PAR_Rank());
    BroadcastDoubleVector(yMax, PAR_Rank());
#if BL_SPACEDIM==3
    BroadcastDoubleVector(zMin, PAR_Rank());
    BroadcastDoubleVector(zMax, PAR_Rank());
#endif
    BroadcastStringVector(multifabFilenames, PAR_Rank());
}


// ****************************************************************************
//  Method:  avtBoxlibFileFormat::CreateGrid
//
//  Purpose:
//    Create a rectilinear grid given bounds and a delta.
//
//  Arguments
//    lo        Lower bound
//    hi        Upper bound
//    delta     Step size in the xyz directions.
//
//  Returns: The grid as a vtkDataSet.
//
//  Programmer:  Akira Haddox
//  Creation:    July 28, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Nov  8 18:47:29 PST 2003
//    Offloaded finding of dimensions to GetDimensions (since other routines
//    also need this functionality and it makes sense to have a single
//    place where we do this).
//
// ****************************************************************************

vtkDataSet *
avtBoxlibFileFormat::CreateGrid(double lo[BL_SPACEDIM], double hi[BL_SPACEDIM], double delta[BL_SPACEDIM]) 
    const
{
    int i;
    int steps[3];
#if BL_SPACEDIM==2
    steps[2] = 1;
#endif
    vtkRectilinearGrid *rg = vtkRectilinearGrid::New();

    GetDimensions(steps, lo, hi, delta);

    rg->SetDimensions(steps);

    vtkFloatArray  *xcoord = vtkFloatArray::New();
    vtkFloatArray  *ycoord = vtkFloatArray::New();
    vtkFloatArray  *zcoord = vtkFloatArray::New();

    xcoord->SetNumberOfTuples(steps[0]);
    ycoord->SetNumberOfTuples(steps[1]);
    zcoord->SetNumberOfTuples(steps[2]);

    float *ptr = xcoord->GetPointer(0);
    for (i = 0; i < steps[0]; ++i)
        ptr[i] = (lo[0] + i * delta[0]);

    ptr = ycoord->GetPointer(0);
    for (i = 0; i < steps[1]; ++i)
        ptr[i] = (lo[1] + i * delta[1]);

    ptr = zcoord->GetPointer(0);
#if BL_SPACEDIM==2
    *ptr = 0.;
#elif BL_SPACEDIM==3
    for (i = 0; i < steps[2]; ++i)
        ptr[i] = (lo[2] + i * delta[2]);
#endif

    rg->SetXCoordinates(xcoord);
    rg->SetYCoordinates(ycoord);
    rg->SetZCoordinates(zcoord);

    xcoord->Delete();
    ycoord->Delete();
    zcoord->Delete();

    return rg;
}


// ****************************************************************************
//  Method: avtBoxlibFileFormat::GetDimensions
//
//  Purpose:
//      Determines the dimensions of a grid based on its extents and step.
//
//  Arguments:
//      dims    The dimensions.  Output.
//      level   The refinement level of the patch.
//      patch   The global patch number.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2003
// 
// ****************************************************************************

void
avtBoxlibFileFormat::GetDimensions(int *dims, int level, int patch) const
{
    double lo[3], hi[3], delta[3];
    lo[0] = xMin[patch];
    hi[0] = xMax[patch];
    delta[0] = deltaX[level];

    lo[1] = yMin[patch];
    hi[1] = yMax[patch];
    delta[1] = deltaY[level];

#if BL_SPACEDIM==3
    lo[2] = zMin[patch];
    hi[2] = zMax[patch];
    delta[2] = deltaZ[level];
#endif

    GetDimensions(dims, lo, hi, delta);
}


// ****************************************************************************
//  Method: avtBoxlibFileFormat::GetDimensions
//
//  Purpose:
//      Determines the dimensions of a grid based on its extents and step.
//
//  Arguments:
//      dims    The dimensions.  Output.
//      lo      The low extent.
//      hi      The high extent.
//      delta   The step.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2003
// 
// ****************************************************************************

void
avtBoxlibFileFormat::GetDimensions(int *dims, double *lo, double *hi, 
                                     double *delta) const
{
    //
    // Because we're reconstructing the dimensions from the deltas, we
    // want to make sure that we don't accidently round down in integer
    // conversion, so we add an appropriate little amount.
    //
    for (int i = 0; i < dimension; ++i)
    {
        double epsilon = delta[i] / 8.0;
        // This expands out to (hi - lo / delta) + epsilon,
        // cast to an int.
        dims[i] = (int) ((hi[i] - lo[i] + epsilon) / delta[i]);
        
        // Increment by one for fencepost point at end.
        ++(dims[i]);
    }
}


// ****************************************************************************
//  Method:  avtBoxlibFileFormat::GetVar
//
//  Purpose:
//    Reads the variable with the given name for the given time step and
//    domain.
//
//  Arguments:
//    patch      the domain number
//    name       the name of the variable to read
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Nov  8 08:59:56 PST 2003
//    Removed notion of timestep.  Also removed dependence on needing
//    rectilinear grid to find dimensions.
//
//    Hank Childs, Tue Nov 18 20:44:48 PST 2003
//    Removed problem size memory leak.
//
//    Hank Childs, Fri Apr  2 10:19:03 PST 2004
//    Added code to sidestep bugginess with VisMF.
//
//    Hank Childs, Wed Jul  7 16:15:36 PDT 2004
//    Account for non 0-origin variables.
//
// ****************************************************************************

vtkDataArray *
avtBoxlibFileFormat::GetVar(int patch, const char *var_name)
{
    if (!initializedReader)
        InitializeReader();

    int level, local_patch;
    GetLevelAndLocalPatchNumber(patch, level, local_patch);

    if (level >= nLevels)
        EXCEPTION2(BadIndexException, level, nLevels);
    if (local_patch >= patchesPerLevel[level])
        EXCEPTION2(BadDomainException, patch, patchesPerLevel[level]);

    int varIndex;
    for (varIndex = 0; varIndex < varNames.size(); ++varIndex)
        if (varNames[varIndex] == var_name)
            break;

    if (varIndex > varNames.size())
        EXCEPTION1(InvalidVariableException, var_name);
    
    int mfIndex = fabfileIndex[level][varIndex];
    int compId = componentIds[level][varIndex];
    
    VisMF *vmf = GetVisMF(mfIndex);
    
    // The VisMF memory management routines are buggy, so we need to do our
    // own management.  Save what we have read so we can free it later.
    clearlist.push_back(mfIndex);
    clearlist.push_back(local_patch);
    clearlist.push_back(compId);

    // Get the data (an FArrayBox)
    FArrayBox fab = vmf->GetFab(local_patch, compId);
    const int *len = fab.length();

    int dims[BL_SPACEDIM];
    GetDimensions(dims, level, patch);

    // Cell based variable. Shift the dimensions.
    if (varCentering[varIndex] == AVT_ZONECENT)
    {
        --dims[0];
        --dims[1];
#if BL_SPACEDIM==3
        --dims[2];
#endif
    }
    
    int xorigin = 0;
    int yorigin = 0;
#if BL_SPACEDIM==3
    int zorigin = 0;
#endif
    if (dims[0] < len[0]-1)
        xorigin = len[0]-1 - dims[0];
    if (dims[1] < len[1]-1)
        yorigin = len[1]-1 - dims[1];
#if BL_SPACEDIM==3
    if (dims[2] < len[2]-1)
        zorigin = len[2]-1 - dims[2];
#endif
    
    vtkFloatArray *farr = vtkFloatArray::New();
#if BL_SPACEDIM==2
    farr->SetNumberOfTuples(dims[0] * dims[1]);
#elif BL_SPACEDIM==3
    farr->SetNumberOfTuples(dims[0] * dims[1] * dims[2]);
#endif

    float *fptr = farr->GetPointer(0);
    
    // We need to index in the same box that the data is defined on,
    // so we need to offset all of our indexes by the lower box corner. 
    IntVect pos;
    const int *offset = fab.box().loVect();
#if BL_SPACEDIM==3
    int x, y, z;
    for (z = 0; z < dims[2]; z++)
    {
        pos[2] = z + offset[2] + zorigin;
#elif BL_SPACEDIM==2
    int x, y;
#endif
        for (y = 0; y < dims[1]; y++)
        {
            pos[1] = y + offset[1] + yorigin;
            for (x = 0; x < dims[0]; x++)
            {
                pos[0] = x + offset[0] + xorigin;
                *(fptr++) = fab(pos);
            }
        }
#if BL_SPACEDIM==3
    }
#endif
    
    fab.clear();
    return farr;
}


// ****************************************************************************
//  Method:  avtBoxlibFileFormat::GetVectorVar
//
//  Purpose:
//    Reads the vector variable with the given name for the given domain.
//
//  Note:  The Generic Database (which calls this function) already handles
//         caching for a single time step.
//
//  Arguments:
//    patch                the patch number
//    var_name             the name of the variable to read
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Nov  8 08:59:56 PST 2003
//    Removed notion of timestep.  Also removed dependence on needing
//    rectilinear grid to find dimensions.
//
//    Hank Childs, Tue Nov 18 20:44:48 PST 2003
//    Removed problem size memory leak.
//
//    Hank Childs, Fri Apr  2 10:19:03 PST 2004
//    Added code to sidestep bugginess with VisMF.
//
//    Hank Childs, Thu Jul  8 14:08:18 PDT 2004
//    Account for non 0-origin variables.
//    
// ****************************************************************************

vtkDataArray *
avtBoxlibFileFormat::GetVectorVar(int patch, const char *var_name)
{
    if (!initializedReader)
        InitializeReader();

    int level, local_patch;
    GetLevelAndLocalPatchNumber(patch, level, local_patch);

    if (level >= nLevels)
        EXCEPTION2(BadIndexException, level, nLevels);
    if (local_patch >= patchesPerLevel[level])
        EXCEPTION2(BadDomainException, patch, patchesPerLevel[level]);

    int vectIndex;
    for (vectIndex = 0; vectIndex < vectorNames.size(); ++vectIndex)
        if (vectorNames[vectIndex] == var_name)
            break;

    if (vectIndex > nVectors)
        EXCEPTION1(InvalidVariableException, var_name);
    
    // Get the data for the components (in FArrayBoxes).
    vector<FArrayBox *> fab(dimension);
    
    int i;
    vector<int> compIdsList;
    vector<VisMF *> vmfList;
    for (i = 0; i < dimension; ++i)
    {
        int varIndex = vectorComponents[vectIndex][i];
        int mfIndex = fabfileIndex[level][varIndex];
        int compId = componentIds[level][varIndex];

        // The VisMF memory management routines are buggy, so we need to do our
        // own management.  Save what we have read so we can free it later.
        clearlist.push_back(mfIndex);
        clearlist.push_back(local_patch);
        clearlist.push_back(compId);

        VisMF *vmf = GetVisMF(mfIndex); 
        fab[i] = new FArrayBox(vmf->GetFab(local_patch, compId));
        compIdsList.push_back(compId);
        vmfList.push_back(vmf);
    }
    
    int dims[BL_SPACEDIM];
    GetDimensions(dims, level, patch);
    
    // Cell based variable.
    if (vectorCentering[vectIndex] == AVT_ZONECENT)
    {
        --dims[0];
        --dims[1];
#if BL_SPACEDIM==3
        --dims[2];
#endif
    }
    
    // Assume all the variables have the same origin.
    int xorigin = 0;
    int yorigin = 0;
#if BL_SPACEDIM==3
    int zorigin = 0;
#endif
    const int *len = fab[0]->length();
    if (dims[0] < len[0]-1)
        xorigin = len[0]-1 - dims[0];
    if (dims[1] < len[1]-1)
        yorigin = len[1]-1 - dims[1];
#if BL_SPACEDIM==3
    if (dims[2] < len[2]-1)
        zorigin = len[2]-1 - dims[2];
#endif

    vtkFloatArray *farr = vtkFloatArray::New();
    farr->SetNumberOfComponents(3);
#if BL_SPACEDIM==2
    farr->SetNumberOfTuples(dims[0] * dims[1]);
#elif BL_SPACEDIM==3
    farr->SetNumberOfTuples(dims[0] * dims[1] * dims[2]);
#endif

    float *fptr = farr->GetPointer(0);
    
    // We need to index in the same box that the data is defined on,
    // so we need to offset all of our indexes by the lower box corner. 
    //
    // We also know the fabs are all defined on the same box, so we
    // only need one offset.
    const int * offset = fab[0]->box().loVect();
    IntVect pos;
#if BL_SPACEDIM==3
    int x, y, z;
    for (z = 0; z < dims[2]; ++z)
    {
        pos[2] = z + offset[2] + zorigin;
#elif BL_SPACEDIM==2
    int x, y;
#endif
        for (y = 0; y < dims[1]; ++y)
        {
            pos[1] = y + offset[1] + yorigin;
            for (x = 0; x < dims[0]; ++x)
            {
                pos[0] = x + offset[0] + xorigin;

                *(fptr++) = (*(fab[0]))(pos);
                *(fptr++) = (*(fab[1]))(pos);
                *(fptr++) = (*(fab[2]))(pos);
            }
        }
#if BL_SPACEDIM==3
    }
#endif
    
    for (i = 0; i < dimension; ++i)
    {
        fab[i]->clear();
        delete fab[i];
    }

    return farr;
}


// ****************************************************************************
//  Method:  avtBoxlibFileFormat::GetVisMF
//
//  Purpose:
//    Get the VisMF multifab reader for a specified timestep and file.
//    If it doesn't exist yet, create it.
//
//  Arguments:
//    ts        The timestep
//    index     The index of the multifab file (into multifabFilenames)
//
//  Returns:    A pointer to the VisMF.
//
//  Programmer:  Akira Haddox
//  Creation:    July 30, 2003
//
//  Modifications:
//    Hank Childs, Thu Nov  6 09:49:33 PST 2003
//    Removed notion of time, since this is now a STMD.
//
//    Brad Whitlock, Thu Sep 22 13:35:57 PST 2005
//    Changed so it works on win32.
//
// ****************************************************************************

VisMF *
avtBoxlibFileFormat::GetVisMF(int index)
{
    if (!mfReaders[index])
    {
        string filename(rootPath + timestepPath + SLASH_STRING +
                        multifabFilenames[index]);
#if defined(_WIN32)
        // If we're on Windows then make some extra passes through the filename
        // so all '\\' characters are changed to '/' or Boxlib will choke.
        std::string::size_type idx;
        while((idx = filename.find("/")) != string::npos)
            filename.replace(idx, 1, "\\");
        while((idx = filename.find("\\\\")) != string::npos)
            filename.replace(idx, 2, "\\");
        while((idx = filename.find("\\")) != string::npos)
            filename.replace(idx, 1, "/");
#endif
        mfReaders[index] = new VisMF(filename.c_str());
    }
    return mfReaders[index];
}


// ****************************************************************************
//  Method:  avtBoxlibFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//    Returns meta-data about the database.
//
//  Arguments:
//    md         The meta-data structure to populate
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Nov  6 14:08:30 PST 2003
//    Removed notion of time.  Also changed mesh to incorporate all levels.
//    Also fixed a bug with vector centering.
//
//    Hank Childs, Sat Nov  8 10:00:43 PST 2003
//    Do not display scalar variables when they are being used elsewhere
//    (like in a vector or in a material).
//
//    Hank Childs, Sat Nov 22 08:18:12 PST 2003
//    Calculate the domain nesting information when initializing the reader.
//    This way that information will be available when we re-visit timesteps
//    (because we will get it, the DB will clear it, and, now we will get
//    it again).
//
//    Hank Childs, Wed Feb 18 10:46:42 PST 2004
//    Expose all scalar variables whether or not they are used elsewhere.
//
//    Brad Whitlock, Thu Aug 5 15:52:53 PST 2004
//    Prevent VisIt from alphabetizing the variable lists.
//
//    Hank Childs, Mon Feb 14 11:08:13 PST 2005
//    Make materials be 1-indexed.
//
//    Hank Childs, Wed Jan 11 09:40:17 PST 2006
//    Change mesh type to AMR.
//
// ****************************************************************************

void
avtBoxlibFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!initializedReader)
        InitializeReader();

    int totalPatches = 0;
    int level;
    for (level = 0 ; level < nLevels ; level++)
        totalPatches += patchesPerLevel[level];

    // Prevent VisIt from sorting the variables.
    md->SetMustAlphabetizeVariables(false);

    char mesh_name[32] = "Mesh";
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = mesh_name;
    mesh->meshType = AVT_AMR_MESH;
    mesh->numBlocks = totalPatches;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = dimension;
    mesh->topologicalDimension = dimension;
    mesh->hasSpatialExtents = false;
    mesh->blockTitle = "patches";
    mesh->blockPieceName = "patch";
    mesh->numGroups = nLevels;
    mesh->groupTitle = "levels";
    mesh->groupPieceName = "level";
    vector<int> groupIds(totalPatches);
    vector<string> blockPieceNames(totalPatches);
    for (int i = 0 ; i < totalPatches ; i++)
    {
        char tmpName[128];
        int level, local_patch;
        GetLevelAndLocalPatchNumber(i, level, local_patch);
        groupIds[i] = level;
        sprintf(tmpName, "level%d,patch%d", level, local_patch);
        blockPieceNames[i] = tmpName;
    }
    mesh->blockNames = blockPieceNames;
#if BL_SPACEDIM==2
    mesh->meshCoordType = AVT_ZR;
#endif
    md->Add(mesh);
    md->AddGroupInformation(nLevels, totalPatches, groupIds);

    int v;
    for (v = 0; v < nVars; ++v)
    {
        if (varCentering[v] == AVT_UNKNOWN_CENT)
            continue;

        AddScalarVarToMetaData(md, varNames[v], mesh_name, 
                               (avtCentering)varCentering[v]);
    }

    for (v = 0; v < nVectors; ++v)
    {
        AddVectorVarToMetaData(md, vectorNames[v], mesh_name,
                             (avtCentering)vectorCentering[v], dimension);
    }

    if (nMaterials)
    {
        vector<string> mnames(nMaterials);

        string matname;
        matname = "materials";
        
        char str[32];
        for (int m = 0; m < nMaterials; ++m)
        {
            sprintf(str, "mat%d", m+1);
            mnames[m] = str;
        }
        AddMaterialToMetaData(md, matname, mesh_name, nMaterials, mnames);
    }

    if (haveReadTimeAndCycle)
    {
        md->SetTime(timestep, time);
        md->SetCycle(timestep, cycle);
    }
}


// ****************************************************************************
//  Method: avtBoxlibFileFormat::CalculateDomainNesting
//
//  Purpose:
//      Calculates how the patches nest between levels.
//
//  Programmer: Hank Childs
//  Creation:   November 6, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Nov 11 14:00:53 PST 2003
//    Added rectilinear domain boundaries.
//
//    Hank Childs, Sat Nov 22 08:20:15 PST 2003
//    Modify the way indices are determined to account for floating point
//    precision.
//
//    Hank Childs, Wed Dec 10 18:41:45 PST 2003
//    Fix cut-and-paste bug.
//
//    Kathleen Bonnell, Tue Jan 20 17:07:28 PST 2004 
//    Added flag to constructor call of avtRectilinearDomainBoundaries to 
//    indicate that neighbors can be computed from extents.
//
//    Hank Childs, Fri May  9 11:07:14 PDT 2008
//    Speed up calculation of domain nesting.
//
// ****************************************************************************

void
avtBoxlibFileFormat::CalculateDomainNesting(void)
{
    int level;

    //
    // Calculate some info we will need in the rest of the routine.
    //
    int totalPatches = 0;
    vector<int> levelStart;
    vector<int> levelEnd;
    for (level = 0 ; level < nLevels ; level++)
    {

        levelStart.push_back(totalPatches);
        totalPatches += patchesPerLevel[level];
        levelEnd.push_back(totalPatches);
    }

    //
    // Now that we know the total number of patches, we can allocate the
    // data structure we want to populate.
    //
    avtStructuredDomainNesting *dn = new avtStructuredDomainNesting(
                                          totalPatches, nLevels);

    //
    // Calculate what the refinement ratio is from one level to the next.
    //
    vector<int> rr(BL_SPACEDIM);
    for (level = 0 ; level < nLevels ; level++)
    {
        if (level == 0)
        {
            rr[0] = 1;
            rr[1] = 1;
#if BL_SPACEDIM==3
            rr[2] = 1;
#endif
        }
        else
        {
            rr[0] = refinement_ratio[level-1];
            rr[1] = refinement_ratio[level-1];
#if BL_SPACEDIM==3
            rr[2] = refinement_ratio[level-1];
#endif
        }
        dn->SetLevelRefinementRatios(level, rr);
    }

    //
    // This multiplier will be needed to find out if patches are nested.
    //
    vector<int> multiplier(nLevels);
    multiplier[nLevels-1] = 1;
    for (level = nLevels-2 ; level >= 0 ; level--)
    {
        multiplier[level] = multiplier[level+1]*refinement_ratio[level];
    }
    
    //
    // Calculate the logical extents for each patch.  The indices will be
    // in terms of the indices for the most finely refined level.  That way
    // we can easily compare between the patches to see if they are nested.
    //
    vector<int> logIMin(totalPatches);
    vector<int> logIMax(totalPatches);
    vector<int> logJMin(totalPatches);
    vector<int> logJMax(totalPatches);
#if BL_SPACEDIM==3
    vector<int> logKMin(totalPatches);
    vector<int> logKMax(totalPatches);
#endif
    level = 0;
    avtRectilinearDomainBoundaries *rdb = new avtRectilinearDomainBoundaries(true);
    rdb->SetNumDomains(totalPatches);
    for (int patch = 0 ; patch < totalPatches ; patch++)
    {
        int my_level, local_patch;
        GetLevelAndLocalPatchNumber(patch, my_level, local_patch);

        double epsilonX = deltaX[my_level] / 8.0;
        double epsilonY = deltaY[my_level] / 8.0;
#if BL_SPACEDIM==3
        double epsilonZ = deltaZ[my_level] / 8.0;
#endif
        logIMin[patch] = ((int) ((xMin[patch]-probLo[0]+epsilonX) 
                           / deltaX[my_level])) * multiplier[my_level];
        logIMax[patch] = ((int) ((xMax[patch]-probLo[0]+epsilonX) 
                           / deltaX[my_level])) * multiplier[my_level];
        logJMin[patch] = ((int) ((yMin[patch]-probLo[1]+epsilonY) 
                           / deltaY[my_level])) * multiplier[my_level];
        logJMax[patch] = ((int) ((yMax[patch]-probLo[1]+epsilonY) 
                           / deltaY[my_level])) * multiplier[my_level];
#if BL_SPACEDIM==3
        logKMin[patch] = ((int) ((zMin[patch]-probLo[2]+epsilonZ) 
                           / deltaZ[my_level])) * multiplier[my_level];
        logKMax[patch] = ((int) ((zMax[patch]-probLo[2]+epsilonZ) 
                           / deltaZ[my_level])) * multiplier[my_level];
#endif
        int e[6] = {0,0,0,0,0,0};
        e[0] = logIMin[patch] / multiplier[my_level];
        e[1] = logIMax[patch] / multiplier[my_level];
        e[2] = logJMin[patch] / multiplier[my_level];
        e[3] = logJMax[patch] / multiplier[my_level];
#if BL_SPACEDIM==3
        e[4] = logKMin[patch] / multiplier[my_level];
        e[5] = logKMax[patch] / multiplier[my_level];
#endif
  
        rdb->SetIndicesForAMRPatch(patch, my_level, e);
    }
    rdb->CalculateBoundaries();
    void_ref_ptr vrdb = void_ref_ptr(rdb,
                                   avtStructuredDomainBoundaries::Destruct);
    cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                        timestep, -1, vrdb);

    //
    // Calculate the child patches.
    //
    vector< vector<int> > childPatches(totalPatches);
    for (level = nLevels-1 ; level > 0 ; level--) 
    {
        int prev_level = level-1;
        int coarse_start  = levelStart[prev_level];
        int coarse_end    = levelEnd[prev_level];
        int num_coarse    = coarse_end - coarse_start;
        avtIntervalTree coarse_levels(num_coarse, dimension, false);
        double exts[6] = { 0, 0, 0, 0, 0, 0 };
        for (int i = 0 ; i < num_coarse ; i++)
        {
            exts[0] = logIMin[coarse_start+i];
            exts[1] = logIMax[coarse_start+i];
            exts[2] = logJMin[coarse_start+i];
            exts[3] = logJMax[coarse_start+i];
#if BL_SPACEDIM==3
            exts[4] = logKMin[coarse_start+i];
            exts[5] = logKMax[coarse_start+i];
#endif
            coarse_levels.AddElement(i, exts);
        }
        coarse_levels.Calculate(true);


        int patches_start = levelStart[level];
        int patches_end   = levelEnd[level];
        for (int patch = patches_start ; patch < patches_end ; patch++)
        {
            double min[BL_SPACEDIM];
            double max[BL_SPACEDIM];
            min[0] = logIMin[patch];
            max[0] = logIMax[patch];
            min[1] = logJMin[patch];
            max[1] = logJMax[patch];
#if BL_SPACEDIM==3
            min[2] = logKMin[patch];
            max[2] = logKMax[patch];
#endif
            vector<int> list;
            coarse_levels.GetElementsListFromRange(min, max, list);
            for (int i = 0 ; i < list.size() ; i++)
            {
                int candidate = coarse_start + list[i];
                if (logIMax[patch] < logIMin[candidate])
                    continue;
                if (logIMin[patch] >= logIMax[candidate])
                    continue;
                if (logJMax[patch] < logJMin[candidate])
                    continue;
                if (logJMin[patch] >= logJMax[candidate])
                    continue;
#if BL_SPACEDIM==3
                if (logKMax[patch] < logKMin[candidate])
                    continue;
                if (logKMin[patch] >= logKMax[candidate])
                    continue;
#endif
                childPatches[candidate].push_back(patch);
           }
        }
    }

    //
    // Now that we know the extents for each patch and what its children are,
    // tell the structured domain boundary that information.
    //
    for (int i = 0 ; i < totalPatches ; i++)
    {
        int my_level, local_patch;
        GetLevelAndLocalPatchNumber(i, my_level, local_patch);

        vector<int> logExts(6);
        logExts[0] = logIMin[i] / multiplier[my_level];
        logExts[3] = logIMax[i] / multiplier[my_level] - 1;
        logExts[1] = logJMin[i] / multiplier[my_level];
        logExts[4] = logJMax[i] / multiplier[my_level] - 1;
#if BL_SPACEDIM==2
        logExts[2] = 0;
        logExts[5] = 0;
#elif BL_SPACEDIM==3
        logExts[2] = logKMin[i] / multiplier[my_level];
        logExts[5] = logKMax[i] / multiplier[my_level] - 1;
#endif

        dn->SetNestingForDomain(i, my_level, childPatches[i], logExts);
    }

    //
    // Register this structure with the generic database so that it knows
    // to ghost out the right cells.
    //
    dn->SetNumDimensions(BL_SPACEDIM);
    void_ref_ptr vr = void_ref_ptr(dn, avtStructuredDomainNesting::Destruct);
    cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                        timestep, -1, vr);
}


// ****************************************************************************
//  Method: avtBoxlibFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data specified.
//
//  Arguments:
//      var        The variable of interest.
//      dom        The domain of interest.
//      type       The type of auxiliary data.
//      <unnamed>  The arguments for that type -- not used.
//      df         Destructor function.
//
//  Returns:    The auxiliary data.
//
//  Programmer: Akira Haddox
//  Creation:   August 5, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Nov  8 10:26:29 PST 2003
//    Removed all notion of timesteps.  Moved material code into its own
//    dedicated routine.  Added support for interval trees of spatial extents.
//
// ****************************************************************************
 
void *
avtBoxlibFileFormat::GetAuxiliaryData(const char *var, int dom, 
                                        const char * type, void *,
                                        DestructorFunction &df) 
{
    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
        return GetMaterial(var, dom, type, df);
    else if (strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0)
        return GetSpatialIntervalTree(df);

    return NULL;
}


// ****************************************************************************
//  Method: avtBoxlibFileFormat::GetMaterial
//
//  Purpose:
//      Gets an avtMaterial object for the specified patch
//
//  Notes:      This routine was largely taken from the old GetAuxiliaryData,
//              written by Akira Haddox.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2003
//
//  Modifications:
//
//    Hank Childs, Wed Feb 18 10:19:34 PST 2004
//    Construct material in a more numerically reliable way.
//
//    Hank Childs, Mon Feb 14 11:08:13 PST 2005
//    Make materials be 1-indexed.
//
//    Hank Childs, Sun Mar  6 16:21:15 PST 2005
//    Add support for GeoDyne material names.
//
//    Hank Childs, Tue Feb  5 16:37:58 PST 2008
//    Fix memory leaks.
//
// ****************************************************************************
    
void *
avtBoxlibFileFormat::GetMaterial(const char *var, int patch, 
                                   const char *type, DestructorFunction &df)
{
    if (!initializedReader)
        InitializeReader();

    int level, localPatch;
    GetLevelAndLocalPatchNumber(patch, level, localPatch);
    
    if (level >= nLevels)
        EXCEPTION2(BadIndexException, level, nLevels);

    if (localPatch >= patchesPerLevel[level])
        EXCEPTION2(BadDomainException, localPatch, patchesPerLevel[level]);

    int i;
    vector<string> mnames(nMaterials);
    char str[32];
    for (i = 0; i < nMaterials; ++i)
    {
        sprintf(str, "mat%d", i+1);
        mnames[i] = str;
    }
    
    int dims[3];
    GetDimensions(dims, level, patch);
#if BL_SPACEDIM==2
    int nCells = (dims[0]-1)*(dims[1]-1);
#elif BL_SPACEDIM==3
    int nCells = (dims[0]-1)*(dims[1]-1)*(dims[2]-1);
#endif

    vector<int> material_list(nCells);
    vector<int> mix_mat;
    vector<int> mix_next;
    vector<int> mix_zone;
    vector<float> mix_vf;

    // Get the material fractions
    vector<vtkFloatArray *> floatArrays(nMaterials);
    vector<float *> mats(nMaterials);
    for (i = 1; i <= nMaterials; ++i)
    {
        if (vf_names_for_materials)
            sprintf(str,"vf_%d", i);
        else
            sprintf(str,"frac%d", i);
        floatArrays[i - 1] = (vtkFloatArray *)(GetVar(patch, str));
        mats[i - 1] = floatArrays[i - 1]->GetPointer(0);
    }
         
    // Build the appropriate data structures
    for (i = 0; i < nCells; ++i)
    {
        int j;

        // First look for pure materials
        int nmats = 0;
        int lastMat = -1;
        for (j = 0; j < nMaterials; ++j)
        {
            if (mats[j][i] > 0)
            {
                nmats++;
                lastMat = j;
            }
        }

        if (nmats == 1)
        {
            material_list[i] = lastMat;
            continue;
        }

        // For unpure materials, we need to add entries to the tables.  
        material_list[i] = -1 * (1 + mix_zone.size());
        for (j = 0; j < nMaterials; ++j)
        {
            if (mats[j][i] <= 0)
                continue;
            // For each material that's present, add to the tables
            mix_zone.push_back(i);
            mix_mat.push_back(j);
            mix_vf.push_back(mats[j][i]);
            mix_next.push_back(mix_zone.size() + 1);
        }

        // When we're done, the last entry is a '0' in the mix_next
        mix_next[mix_next.size() - 1] = 0;
    }
    
    // we can now free up the vtkFloatArrays we obtained via GetVar calls
    for (i = 1; i <= nMaterials; ++i)
    {
        if (floatArrays[i - 1] != 0)
            floatArrays[i - 1]->Delete();
    }

    int mixed_size = mix_zone.size();
    avtMaterial * mat = new avtMaterial(nMaterials, mnames, nCells,
                                        &(material_list[0]), mixed_size,
                                        &(mix_mat[0]), &(mix_next[0]),
                                        &(mix_zone[0]), &(mix_vf[0]));
     
    df = avtMaterial::Destruct;
    return (void*) mat;
}


// ****************************************************************************
//  Method: avtBoxlibFileFormat::GetSpatialIntervalTree
//
//  Purpose:
//      Gets the spatial interval tree for all the patches.
//
//  Notes:      This routine was largely taken from the old GetAuxiliaryData,
//              written by Akira Haddox.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2003
//
//  Modifications:
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
// ****************************************************************************
    
void *
avtBoxlibFileFormat::GetSpatialIntervalTree(DestructorFunction &df)
{
    int totalPatches = 0;
    for (int level = 0 ; level < nLevels ; level++)
        totalPatches += patchesPerLevel[level];

    avtIntervalTree *itree = new avtIntervalTree(totalPatches, dimension);

    for (int patch = 0 ; patch < totalPatches ; patch++)
    {
        double bounds[6];
        bounds[0] = xMin[patch];
        bounds[1] = xMax[patch];
        bounds[2] = yMin[patch];
        bounds[3] = yMax[patch];
#if BL_SPACEDIM==3
        bounds[4] = zMin[patch];
        bounds[5] = zMax[patch];
#endif
        itree->AddElement(patch, bounds);
    }
    itree->Calculate(true);

    df = avtIntervalTree::Destruct;

    return ((void *) itree);
}


// ****************************************************************************
//  Method:  avtBoxlibFileFormat::FreeUpResources
//
//  Purpose:
//    Release cached memory used by this reader.
//
//  Programmer:  Akira Haddox
//  Creation:    July 30, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Nov  6 09:49:33 PST 2003
//    Removed all notion of time.
//
//    Hank Childs, Fri Apr  2 10:19:03 PST 2004
//    Added code to sidestep bugginess with VisMF.
//
// ****************************************************************************

static void
SwapEntries(std::vector<int> &clearlist, int t, int s)
{
    int tmp;
    tmp = clearlist[3*t];
    clearlist[3*t] = clearlist[3*s];
    clearlist[3*s] = tmp;
    tmp = clearlist[3*t+1];
    clearlist[3*t+1] = clearlist[3*s+1];
    clearlist[3*s+1] = tmp;
    tmp = clearlist[3*t+2];
    clearlist[3*t+2] = clearlist[3*s+2];
    clearlist[3*s+2] = tmp;
}


void
avtBoxlibFileFormat::FreeUpResources()
{
    int   i, j;

    initializedReader = false;
    
    //
    // If we "clear" the same patch twice, it will cause a crash, so we have
    // to sort out the duplicates (this can happen when we access a variable
    // as a component of a vector and then later as a scalar).
    //
    int num_entries = clearlist.size() / 3;
    for (i = 0 ; i < num_entries ; i++)
    {
        for (j = i+1 ; j < num_entries ; j++)
        {
            if (clearlist[3*i] > clearlist[3*j])
                SwapEntries(clearlist, i, j);
            else if (clearlist[3*i] == clearlist[3*j])
                if (clearlist[3*i+1] > clearlist[3*j+1])
                    SwapEntries(clearlist, i, j);
                else if (clearlist[3*i+1] == clearlist[3*j+1])
                    if (clearlist[3*i+2] > clearlist[3*j+2])
                        SwapEntries(clearlist, i, j);
        }
    }

    //
    // Now call all the correct "clear"s on the VisMFs, being careful not
    // to make an identical call to each VisMF.
    //
    for (i = 0 ; i < num_entries ; i++)
    {
        if (i > 0)
        {
            if ((clearlist[3*i] == clearlist[3*i-3]) &&
                (clearlist[3*i+1] == clearlist[3*i-2]) &&
                (clearlist[3*i+2] == clearlist[3*i-1]))
                continue;
        }
        mfReaders[clearlist[3*i]]->clear(clearlist[3*i+1], clearlist[3*i+2]);
    }
    clearlist.clear();

    for (j = 0; j < mfReaders.size(); j++)
    {
        if (mfReaders[j])
        {
            delete mfReaders[j];
            mfReaders[j] = NULL;
        }
    }
}


// ****************************************************************************
//  Method:  GetDirName
//
//  Purpose:
//    Returns the directory from a full path name
//
//  Arguments:
//    path       the full path name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
//  Modifications:
//    Brad Whitlock, Thu Sep 22 13:36:39 PST 2005
//    Changed for win32.
//
// ****************************************************************************

string 
GetDirName(const char *path)
{
    string dir = "";

    int len = strlen(path);
    const char *last = path + (len-1);
    while (*last != SLASH_CHAR && last > path)
    {
        last--;
    }

    if (*last != SLASH_CHAR)
    {
        return "";
    }

    char str[1024];
    strcpy(str, path);
    str[last-path+1] = '\0';

    return str;
}


// ****************************************************************************
//  Method:  EatUpWhiteSpace
//
//  Purpose:
//    Read from the filestream to skip whitespace.
//
//  Programmer:  Akira Haddox
//  Creation:    July  28, 2003
//
// ****************************************************************************

void 
EatUpWhiteSpace(ifstream &in)
{
    while (isspace(in.peek()))
            in.get();
}


// ***************************************************************************
//  Method: VSSearch
//
//  Purpose:
//    Search a vector of strings.
//
//  Arguments:
//    v     the vector
//    s     the string to search for
//
//  Returns:
//    The index of s, or -1 if not found.
//    
//  Programmer:  Akira Haddox
//  Creation:    July  28, 2003
//
// ***************************************************************************

int VSSearch(const vector<string> &v, const string &s)
{
    int i;
    for (i = 0; i < v.size(); ++i)
        if (v[i] == s)
            return i;
    return -1;
}


