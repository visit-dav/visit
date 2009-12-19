/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                            avtCCMFileFormat.C                             //
// ************************************************************************* //

#include <avtCCMFileFormat.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtFacelist.h>
#include <avtVariableCache.h>

#include <Expression.h>
#include <Expression.h>

#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

#include <DebugStream.h>
#include <TimingsManager.h>

#include <set>
#include <deque>
#include <algorithm>

using     std::string;

#ifdef PARALLEL
// If we're running in parallel then we can enable subdivision of single domains.
// This only kicks in when there is a single domain that we want to split among
// all processors.
#include <avtParallel.h>
#define ENABLE_SUBDIVISION
#endif

// Prototypes
ostream &operator << (ostream &os, CCMIOEntity e);
ostream &operator << (ostream &os, const CCMIONode &node);
ostream &operator << (ostream &os, const CCMIOID &id);

// ****************************************************************************
//  Method: avtCCM constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 28 15:00:24 PST 2008
//    Added varsOnSubmesh.
//
// ****************************************************************************

avtCCMFileFormat::avtCCMFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1), varsToFields(), varsOnSubmesh()
{
    ccmOpened = false;
    ccmStateFound = false;
    ccmErr = kCCMIONoErr;
    subdividingSingleMesh = false;
}

// ****************************************************************************
//  Method: avtCCM destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
// ****************************************************************************

avtCCMFileFormat::~avtCCMFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtCCMFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 28 15:00:24 PST 2008
//    Added varsOnSubmesh.
//
// ****************************************************************************

void
avtCCMFileFormat::FreeUpResources(void)
{
    if(ccmOpened)
    {
        ccmOpened = false;
        CCMIOCloseFile(&ccmErr, ccmRoot);
        ccmErr = kCCMIONoErr;
    }
    for (int i = 0; i < originalCells.size(); ++i)
    {
        if (originalCells[i] != NULL)
        {
            originalCells[i]->Delete();
            originalCells[i] = NULL;
        }
    }
    originalCells.clear();
    varsToFields.clear();
    varsOnSubmesh.clear();

    subdividingSingleMesh = false;
}    


// ****************************************************************************
//  Method:  avtCCMFileFormat::CanCacheVariable
//
//  Purpose:
//    Determines if the given variable can be cached above the layer of the
//    plugin. In some cases, particularly where different ghosting is used
//    for different variables, the mesh variable cannot be cached.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 28, 2008 
//
// ***************************************************************************

bool
avtCCMFileFormat::CanCacheVariable(const char *varname)
{
    return varsOnSubmesh.empty();
}

// ****************************************************************************
// Method: avtCCMFileFormat::GetRoot
//
// Purpose: 
//   Opens the file and passes back an id for the "root" node that can be used
//   to read information about the file. We use this instead of ccmRoot so the
//   file can be implicitly opened, if needed.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 6 09:11:07 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

CCMIOID &
avtCCMFileFormat::GetRoot()
{
    const char *mName = "avtCCMFileFormat::GetRoot: ";
    if(!ccmOpened)
    {
        ccmErr = kCCMIONoErr;
        ccmErr = CCMIOOpenFile(NULL, filenames[0], kCCMIORead, &ccmRoot);
        if(ccmErr == kCCMIONoErr)
            ccmOpened = true;
        else
            debug4 << mName << "Could not open CCM file " 
                   << filenames[0] << endl;
    }

    return ccmRoot;
}


// ****************************************************************************
// Method: avtCCMFileFormat::GetState
//
// Purpose: 
//   Gets the 'state' entity.  A 'default' state is tried first, and if
//   not found the first state found is used.  We may want to change this
//   in the future, so that multiple states can be retrieved. 
//
// Programmer: Kathleen Bonnell
// Creation:   September 11, 2007 
//
// Modifications:
//   
// ****************************************************************************

CCMIOID &
avtCCMFileFormat::GetState()
{
    const char *mName = "avtCCMFileFormat::GetState: ";
    if (!ccmStateFound)
    {
        // first try default state
        CCMIOGetState(&ccmErr, GetRoot(), "default", &ccmProblem, &ccmState);
        if (ccmErr == kCCMIONoErr)
        {
            ccmStateFound = true;
        }
        else 
        {
            int i = 0;
            ccmErr = kCCMIONoErr; 
            if (CCMIONextEntity(NULL, GetRoot(), kCCMIOState, &i, &ccmState) 
                == kCCMIONoErr)
            {
                ccmStateFound = true;
                int j = 0;
                CCMIONextEntity(NULL, GetRoot(), kCCMIOProblemDescription, 
                                &j, &ccmProblem);
            }
            else
            {
                debug4 << mName <<  "Could not find state entity." << endl;
            }
        }
    }
    return ccmState;
}


// ****************************************************************************
// Method: avtCCMFileFormat::GetIDsForDomain
//
// Purpose: 
//   Gets nodes for state, processor, vertices, topology and solution that can
//   be used to query attributes for variables and meshes.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 6 09:10:29 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtCCMFileFormat::GetIDsForDomain(int dom, 
    CCMIOID &processor, CCMIOID &vertices, CCMIOID &topology,
    CCMIOID &solution, bool &hasSolution)
{
    const char *mName = "avtCCMFileFormat::GetIDsForDomain: ";

    // Try and get the requested processor.
    int proc = dom;
    bool ret = (
        CCMIONextEntity(NULL, GetState(), kCCMIOProcessor, &proc, &processor) ==
        kCCMIONoErr);
    if(ret)
    {
        hasSolution = true;
        ccmErr = kCCMIONoErr;
        // Try and read the vertices, topology, and solution ids for this 
        // processor.
        CCMIOReadProcessor(&ccmErr, processor, &vertices, &topology, NULL, 
                           &solution);
        if(ccmErr != kCCMIONoErr)
        {
            // That didn't work. (Maybe no solution). See if we can at least 
            // get the vertices and processor.
            ccmErr = kCCMIONoErr;
            CCMIOReadProcessor(&ccmErr, processor, &vertices, &topology, NULL, 
                               NULL);
            if(ccmErr == kCCMIONoErr)
                hasSolution = false;
            else
                ret = false;
        }
    }

    return ret;
}


// ****************************************************************************
//  Method: avtCCMFileFormat::GetFaces
//
//  Purpose: Reads the face info. 
//
//  Arguments:
//    faceID        The ID of the face entity.
//    faceType      The type of faces (internal or boundary).
//    nFaces        How many faces are in the entity. 
//    cellIDMap     Used to map cell IDs to indices
//    vertexIDMap   Used to map vertex IDs to indices
//    minSize       Min num verts in a face
//    maxSize       Max num verts in a face
//    ci            A place to store the face info, must be allocated by
//                  calling method.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 5, 2007 
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar  6 09:21:02 PST 2008
//    Removed unused variable.
//
//    Dave Bremer, Fri Apr  4 16:29:49 PDT 2008
//    Fixed a bug in which cell and vertex ids were mistaken for 1-based
//    indices.
// ****************************************************************************

void
avtCCMFileFormat::GetFaces(CCMIOID faceID, CCMIOEntity faceType,
                           unsigned int nFaces, const IDMap &cellIDMap,
                           const IDMap &vertexIDMap, 
                           int &minSize, int &maxSize, CellInfoVector &ci)
{
    if (faceType != kCCMIOInternalFaces && faceType != kCCMIOBoundaryFaces)
    {
        debug1 << "avtCCMFileFormat::GetFaces encountered an internal error"
                << endl;
        return; 
    }
    int getFacesTimer = visitTimer->StartTimer();
    CCMIOID mapID;
    unsigned int nCells = 0, size = 0;
    //intVector faces;
    intVector faceNodes, faceCells;

    // Determine the size of the faceNodes array, which is of the
    // form n1, v1, v2, ...vn1, n2, v1, v2, ... vn2, )
    CCMIOReadFaces(&ccmErr, faceID, faceType, NULL, &size, NULL,
                   kCCMIOStart, kCCMIOEnd);
    faceNodes.resize(size);
    //faces.resize(nFaces);
    if (faceType == kCCMIOInternalFaces)
        faceCells.resize(nFaces*2);
    else 
        faceCells.resize(nFaces);
    CCMIOReadFaces(&ccmErr, faceID, faceType, &mapID, NULL,
                   &faceNodes[0], kCCMIOStart, kCCMIOEnd);
    CCMIOReadFaceCells(&ccmErr, faceID, faceType, &faceCells[0],
                       kCCMIOStart, kCCMIOEnd);
    //CCMIOReadMap(&ccmErr, mapID, &faces[0], kCCMIOStart, kCCMIOEnd);

    unsigned int pos = 0;
    for (unsigned int i = 0; i < nFaces; ++i)
    {
        FaceInfo newFace;
        //newFace.id = faces[i];
        if (faceType == kCCMIOInternalFaces)
        {
            newFace.cells[0] = faceCells[i*2];
            newFace.cells[1] = faceCells[i*2+1];
        }
        else 
        {
            newFace.cells[0] = faceCells[i];
        }
        int nVerts = faceNodes[pos];

        if (nVerts < minSize)
            minSize = nVerts;
        if (nVerts > maxSize)
            maxSize = nVerts;

        for (unsigned int j = 0; j < nVerts; ++j)
        {
            newFace.nodes.push_back( vertexIDMap.IDtoIndex(faceNodes[pos+1+j]) );
        }
        // cell ids are 1-origin, so must subract 1 to get the
        // correct index into the CellInfoVector
        if (faceType == kCCMIOInternalFaces)
        {
            int  c0 = cellIDMap.IDtoIndex(newFace.cells[0]);
            int  c1 = cellIDMap.IDtoIndex(newFace.cells[1]);

            ci[c0].faceTypes.push_back(1);
            ci[c0].faces.push_back(newFace);
            ci[c1].faceTypes.push_back(2);
            ci[c1].faces.push_back(newFace);
        }
        else 
        {
            int  c0 = cellIDMap.IDtoIndex(newFace.cells[0]);

            ci[c0].faceTypes.push_back(0);
            ci[c0].faces.push_back(newFace);
        }
        pos += faceNodes[pos] +1;
    }
    visitTimer->StopTimer(getFacesTimer, "GetFaces");
}

// ****************************************************************************
//  Method: avtCCMFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 28 15:00:24 PST 2008
//    Fixed determination of centering for Vector variables.  Added code to 
//    determine if any variables are defined only on parts of the mesh, by 
//    reading the number of cells, then mapData size for each var.
//
// ****************************************************************************

void
avtCCMFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    const char *mName = "avtCCMFileFormat::PopulateDatabaseMetaData: ";

    // Count the number of processors in the file. 
    // Use that for the number of domains.
    CCMIOID processor;
    int proc = 0;
    int nblocks = 0;
    while (CCMIONextEntity(NULL, GetState(), kCCMIOProcessor, &proc, 
           &processor) == kCCMIONoErr)
    {
        ++nblocks;
    }
    debug4 << mName << "Found " << nblocks << " domains in the file." << endl;
   
#if 0
    // this will be useful for subsetting by the cell type
    int nCellTypes = 0;
    if (CCMIOIsValidEntity(ccmProblem))
    {
        // Determine the number of kCCMIOCellTypes present.
        int i = 0;
        CCMIOID next;
        while(CCMIONextEntity(NULL, ccmProblem, kCCMIOCellType, &i, &next)
              == kCCMIONoErr)
        ++nCellTypes;
    }
#endif


#if 0
    // Read the simulation title.
    char *title = NULL;
    ccmErr = CCMIOGetTitle(&ccmErr, GetRoot(), &title);
    if(title != NULL)
    {
        md->SetDatabaseComment(title);
        free(title);
    }
#endif

    for (unsigned int i = 0; i < nblocks; ++i)
        originalCells.push_back(NULL);
    // Determine the spatial dimensions.
    int dims = 3;
    CCMIOID vertices, topology, solution;
    bool hasSolution = true;
    if(GetIDsForDomain(0, processor, vertices, topology, solution,
                       hasSolution))
    {
        dims = 1 ;
        CCMIOReadVerticesf(&ccmErr, vertices, &dims, NULL, NULL, NULL, 0, 1);
    }
    else
    {
        EXCEPTION1(InvalidFilesException, filenames[0]);
    }

    // If there's just 1 block, read the mesh now and decompose it into
    // more meshes, caching them for later.
    int nDomains = nblocks;
#ifdef ENABLE_SUBDIVISION
    if(nblocks == 1)
    {
        subdividingSingleMesh = true;
        md->SetFormatCanDoDomainDecomposition(true);
    }
#endif

    // Create a mesh.
    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name = "Mesh";
    mmd->spatialDimension = dims;
    mmd->topologicalDimension = dims;
    mmd->meshType = AVT_UNSTRUCTURED_MESH;
    mmd->numBlocks = nDomains;
    mmd->cellOrigin = 1;
    mmd->nodeOrigin = 1;
    md->Add(mmd);

    // Find variable data
    if (hasSolution)
    {
        // Determine number of cells, in order to find out
        // which variables are NOT defined on entire mesh.
        CCMIOError err = kCCMIONoErr;
        CCMIOID cellsID;
        CCMIOSize nCells;
        CCMIOGetEntity(&err, topology, kCCMIOCells, 0, &cellsID);
        CCMIOEntitySize(&err, cellsID, &nCells, NULL);

        debug5 << mName << "Reading variable information" << endl;

        CCMIOID field, phase = solution;
        int h = 0;
        int i = 0;
        bool oldFile = CCMIONextEntity(NULL, solution, kCCMIOFieldPhase, 
                                       &h, &phase) != kCCMIONoErr;

        h = 0;
        while(oldFile ||
              CCMIONextEntity(NULL, solution, kCCMIOFieldPhase, &h, &phase) ==
                                                                   kCCMIONoErr)
        {
            debug5 << mName << "CCMIONextEntity for solution " << solution
                   << " returned h=" << h << ", phase = " << phase << endl;

            if (oldFile)
                phase = solution;
            else
            {
                int phaseNum = 0;
                CCMIOGetEntityIndex(NULL, phase, &phaseNum);
            }

            // This i needs to be set to 0 here because we're asking for the
            // first field from the current phase. Since we're iterating
            // over phases so far, we need to reset i to 0 here to get the
            // first field.
            while(CCMIONextEntity(NULL, phase, kCCMIOField, &i, &field) ==
                  kCCMIONoErr)
            {
                char name[kCCMIOMaxStringLength+1];
                char sName[kCCMIOProstarShortNameLength+1]; 
                char *units = NULL;
                int usize;
                CCMIODataType datatype;
                CCMIODimensionality cdims;
                CCMIOID fieldData, mapID;
                CCMIODataLocation type;
                CCMIOID varField = field;

                debug5 << mName << "CCMIONextEntity for phase " << phase
                       << " returned i=" << i << ", field = " << field << endl;

                // Get the field's name, dims, and data type.
                CCMIOReadField(&ccmErr, field, name, sName, &cdims, &datatype);
                debug5 << mName << "CCMIOReadField for field " << field
                       << " returned name=" << name << ", sName = " 
                       << sName << endl;

                if (cdims == kCCMIOVector)
                {
                    CCMIOID scalar;
                    CCMIOError err = kCCMIONoErr;
                    CCMIOReadMultiDimensionalFieldData(&err, field, 
                                             kCCMIOVectorX, &scalar);
                    if (err == kCCMIONoErr)
                    {
                        // componentized vector, use the X component scalar
                        // in order to retrieve more info about the vector
                        field = scalar;
                    }
                }

                char *usename;
                if (oldFile)
                    usename = name;
                else 
                    usename = sName;

                // Read the field's units
                if (CCMIOReadOptstr(NULL, field, "Units", &usize, NULL) ==
                    kCCMIONoErr)
                {
                    units = new char[usize+1];
                    CCMIOReadOptstr(&ccmErr, field, "Units", NULL, units);
                }

                // Reset j to 0 to get the first field data. We read the 
                // field data to determine the variable centering.
                int j = 0;
                avtCentering centering = AVT_UNKNOWN_CENT;
                CCMIOSize cellVarSize = 0;

                while (CCMIONextEntity(NULL, field, kCCMIOFieldData, 
                       &j, &fieldData) == kCCMIONoErr)
                {
                    debug5 << mName << "CCMIONextEntity for field " << field
                           << " returned kCCMIONoErr, j=" << j 
                           << ",fieldData=" << fieldData << endl;

                    // Read information about the field.
                    CCMIOReadFieldDataf(&ccmErr, fieldData, &mapID, &type,
                                        NULL, kCCMIOStart, kCCMIOEnd);
                    if(ccmErr != kCCMIONoErr)
                    {
                        debug5 << mName << "CCMIOReadFieldDataf failed." 
                               << endl;
                        continue;
                    }

                    // Determine the variable centering.
                    if (type == kCCMIOVertex)
                    { 
                        centering = AVT_NODECENT;
                        debug4 << "Var " << usename << " is node centered" 
                               << endl;
                    } 
                    else if (type == kCCMIOCell)
                    { 
                        centering = AVT_ZONECENT;
                        debug4 << "Var " << usename << " is zone centered" 
                               << endl;
                        CCMIOSize n;
                        CCMIOIndex max;
                        CCMIOEntitySize(&ccmErr, fieldData, &n, &max);
                        cellVarSize += n;
                    } 
                    else if (type == kCCMIOFace)
                    { 
                        debug4 << "Var " << usename << " is face-centered, "
                               << "ignoring for now. " << endl;
                    }
                }

                if (cellVarSize != nCells)
                {
                    varsOnSubmesh.push_back(usename);
                }
                // If we don't have metadata for the variable yet, add it.
                if(varsToFields.find(usename) == varsToFields.end())
                {
                    // Variables with an unsupported centering are tagged 
                    // invalid.
                    bool validVariable = (centering != AVT_UNKNOWN_CENT);

                    if(cdims==kCCMIOScalar)
                    {
                        avtScalarMetaData *smd = new avtScalarMetaData(usename, 
                            "Mesh", centering);
                        if (units != NULL)
                        {
                            smd->hasUnits = true;
                            smd->units = units;
                        }
                        smd->validVariable = validVariable;
                        md->Add(smd);
                        varsToFields[usename] = varField;
                    }
                    else if(cdims==kCCMIOVector)
                    {
                        avtVectorMetaData *vmd = new avtVectorMetaData(usename, 
                            "Mesh", centering, 3);
                        if (units != NULL)
                        {
                            vmd->hasUnits = true;
                            vmd->units = units;
                        }
                        vmd->validVariable = validVariable;
                        md->Add(vmd);
                        varsToFields[usename] = varField;
                    }
                    else if(cdims==kCCMIOTensor)
                    {
                        avtTensorMetaData *tmd = new avtTensorMetaData(usename, 
                            "Mesh", centering, 9);
                        if (units != NULL)
                        {
                            tmd->hasUnits = true;
                            tmd->units = units;
                        }
#if 1
                        // Just make tensor vars display for now. 
                        // Don't plot them.
                        tmd->validVariable = false;
#else
                        tmd->validVariable = validVariable;
#endif
                        md->Add(tmd);
                        varsToFields[usename] = varField;
                    }
                }

                if (units != NULL)
                {
                    delete [] units;
                    units = NULL;
                }
            }
            oldFile = false;
        }
    }
}


// ****************************************************************************
//  Method: avtCCMFileFormat::GetMesh
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
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
//  Modifications:
//    Brad Whitlock, Tue Dec 18 12:57:28 PST 2007
//    Added support for 2D polygonal shapes.
//
//    Kathleen Bonnell, Thu Feb 28 15:00:24 PST 2008
//    If the primary variable (activeVisItVar) is defined only on a portion of
//    the mesh, only retrieve (and tesselate) those cells it is defined upon.
//
//    Dave Bremer, Fri Apr  4 16:29:49 PDT 2008
//    Fixed a bug in which cell and vertex ids were mistaken for 1-based
//    indices.
// 
//    Brad Whitlock, Thu Oct  1 13:36:48 PDT 2009
//    I refactored this routine into helper routines and added support for
//    automatically subdividing a mesh on the fly.
//
// ****************************************************************************

#ifndef MDSERVER
#include <PolygonToTriangles.C>
#endif

vtkDataSet *
avtCCMFileFormat::GetMesh(int domain, const char *meshname)
{
#ifdef MDSERVER
    return 0;
#else
    // Override domain if we're automatically dividing the data
    int dom = subdividingSingleMesh ? 0 : domain;

    vtkUnstructuredGrid *ugrid = NULL;
    vtkPoints *points = NULL;

    TRY
    {
        // Read the points
        points = ReadPoints(dom, meshname);

        // Read the cell connectivity
        CellInfoVector cellInfo;
        int minFaceSize = VTK_LARGE_INTEGER;
        int maxFaceSize = -1;
        ReadCellInfo(dom, meshname,
                     cellInfo, minFaceSize, maxFaceSize);

        //
        // Convert cellInfo into vtkUnstructuredGrid
        //
        SelectCellsForThisProcessor(cellInfo, points);

        ugrid = vtkUnstructuredGrid::New();

        // Determine cell topology from face lists
        if (minFaceSize == 2 && maxFaceSize == 2)
        {
            // 2D edges that we must assemble into polygons and tessellate into 
            // triangles that VisIt can digest.
            TesselateCells2D(domain, cellInfo, points, ugrid); 
        }
        else if (minFaceSize <= 4 && maxFaceSize <= 4)
        {
#ifdef ENABLE_SUBDIVISION
            // If we're subdividing a single domain on the fly then we create 
            // original cell numbers so we can use them in the GetVar method 
            // to return only the cell values that we selected for this chunk
            // of the mesh.
            unsigned int oc[2] = {dom, 0};
            vtkUnsignedIntArray *origCells = 0;
            if(subdividingSingleMesh)
            {
                int useCount = 0;
                for(unsigned int i = 0; i < cellInfo.size(); ++i)
                    useCount += (cellInfo[i].id != -1) ? 1 : 0;
                origCells = vtkUnsignedIntArray::New();
                origCells->SetName("avtOriginalCellNumbers");
                origCells->SetNumberOfComponents(2);
                origCells->Allocate(useCount * 3);
                originalCells[dom] = origCells;
            }
#endif
            ugrid->SetPoints(points);
            // We have zoo elements that we can deal with.
            vtkCellArray *cellArray = vtkCellArray::New();
            intVector cellTypes; 
            bool unhandledCellType = false;
            for (unsigned int i = 0; i < cellInfo.size(); i++)
            {
                const CellInfo &ci = cellInfo[i]; 
                if(ci.id == -1)
                    continue;
#ifdef ENABLE_SUBDIVISION
                oc[1] = ci.id;
#endif
                switch(ci.faces.size())
                {
                    case 4 : 
                        BuildTet(ci, cellArray, cellTypes);
#ifdef ENABLE_SUBDIVISION
                        if(subdividingSingleMesh)
                            origCells->InsertNextTupleValue(oc);
#endif
                        break;
                    case 5 : 
                        {
                        int nNodes = 0;
                        for (size_t j = 0; j < ci.faces.size(); j++)
                        {
                            nNodes += ci.faces[j].nodes.size();
                        }
                        if (nNodes == 16) // Pyramid 
                        {
                            BuildPyramid(ci, cellArray, cellTypes);
#ifdef ENABLE_SUBDIVISION
                            if(subdividingSingleMesh)
                                origCells->InsertNextTupleValue(oc);
#endif
                        }
                        else if (nNodes == 18) // Wedge
                        {
                            BuildWedge(ci, cellArray, cellTypes);
#ifdef ENABLE_SUBDIVISION
                            if(subdividingSingleMesh)
                                origCells->InsertNextTupleValue(oc);
#endif
                        }
                        else
                            unhandledCellType = true; 
                        break;
                        }
                    case 6 : 
                        BuildHex(ci, cellArray, cellTypes);
#ifdef ENABLE_SUBDIVISION
                        if(subdividingSingleMesh)
                            origCells->InsertNextTupleValue(oc);
#endif
                        break;
                    default : 
                        unhandledCellType = true; 
                        break;
                }
            }
            ugrid->SetCells(&cellTypes[0], cellArray);
            cellArray->Delete();
        }
        else
        {
            TesselateCell(domain, cellInfo, points, ugrid);
        }

        points->Delete();
    }
    CATCHALL
    {
        if(points != NULL)
            points->Delete();
        if(ugrid != NULL)
            ugrid->Delete();
    }
    ENDTRY

    return ugrid;
#endif
}

// ****************************************************************************
// Method: avtCCMFileFormat::ReadPoints
//
// Purpose: 
//   Reads the points associated with a specified domain/mesh.
//
// Arguments:
//   dom         : The domain number.
//   meshname    : The name of the mesh.(for error reporting)
//
// Returns:    The vtkPoints object that contains the points.
//
// Note:       This method was broken out from the original GetMesh routine.
//
// Programmer: Kathleen Bonnell, Brad Whitlock
// Creation:   Thu Oct  1 13:29:03 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

vtkPoints *
avtCCMFileFormat::ReadPoints(int dom, const char *meshname)
{
    CCMIOID processor, vertices, topology, solution;
    bool hasSolution = true;
    if(!GetIDsForDomain(dom, processor, vertices, topology, solution,
                        hasSolution))
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    // Read the size of the vertices
    CCMIOSize nnodes = 0;
    CCMIOEntitySize(&ccmErr, vertices, &nnodes, NULL);
    if(ccmErr != kCCMIONoErr)
    {
        debug4 << "CCMIOEntitySize for vertices failed with error " ;
        debug4 << ccmErr << endl;
        EXCEPTION1(InvalidVariableException, meshname);
    }

    // Read the dimensions of the vertex.
    int dims = 1;
    float scale;
    CCMIOReadVerticesf(&ccmErr, vertices, &dims, NULL, NULL, NULL, 0, 1);
    if(ccmErr != kCCMIONoErr)
    {
        debug4 << "CCMIOReadVertices for first vertex dimensions ";
        debug4 << "failed with error " << ccmErr << endl;
        EXCEPTION1(InvalidVariableException, meshname);
    }

    // Allocate VTK memory.
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(nnodes);
    float *pts = (float *)points->GetVoidPointer(0);

    // Read the data into the VTK points.
    CCMIOID mapID;
    if(dims == 2)
    {
        // Read 2D points and convert to 3D, storing into VTK.
        float *pts2d = new float[2 * nnodes];
        CCMIOReadVerticesf(&ccmErr, vertices, &dims, &scale, &mapID, pts2d,
                   0, nnodes);
        float *src = pts2d;
        float *dest = pts;
        for(int i = 0; i < nnodes; ++i)
        {
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = 0.;
        }
        delete [] pts2d;
    }
    else
    {
        // Read the data directly into the VTK buffer.
        CCMIOReadVerticesf(&ccmErr, vertices, &dims, &scale, &mapID, pts,
                   0, nnodes);
    }

    // Scale the points, according to the scale factor read with the 
    // vertices.
    for(int i = 0; i < nnodes; ++i)
    {
        pts[0] *= scale;
        pts[1] *= scale;
        pts[2] *= scale;
        pts += 3;
    }

    return points;
}

// ****************************************************************************
// Method: avtCCMFileFormat::ReadCellInfo
//
// Purpose: 
//   Reads the cell info associated with a specified domain/mesh.
//
// Arguments:
//   dom         : The domain number.
//   meshname    : The name of the mesh.(for error reporting)
//   cellInfo    : The cell information to populate.
//   minFaceSize : Return value of the min face size.
//   maxFaceSize : Return value of the max face size.
//
// Returns:    
//
// Note:       This method was broken out from the original GetMesh routine.
//             min/maxFaceSize are used to determine whether tesselation is 
//             required.
//
// Programmer: Kathleen Bonnell, Brad Whitlock
// Creation:   Thu Oct  1 13:29:03 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtCCMFileFormat::ReadCellInfo(int dom, const char *meshname,
    CellInfoVector &cellInfo, int &minFaceSize, int &maxFaceSize)
{
    CCMIOID processor, vertices, topology, solution;
    bool hasSolution = true;
    if(!GetIDsForDomain(dom, processor, vertices, topology, solution,
                       hasSolution))
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    // Read the size of the vertices
    CCMIOSize nnodes = 0;
    CCMIOEntitySize(&ccmErr, vertices, &nnodes, NULL);
    if(ccmErr != kCCMIONoErr)
    {
        debug4 << "CCMIOEntitySize for vertices failed with error " ;
        debug4 << ccmErr << endl;
        EXCEPTION1(InvalidVariableException, meshname);
    }

    // Read the dimensions of the vertex and get the mapID
    int dims = 1;
    CCMIOID mapID;
    CCMIOReadVerticesf(&ccmErr, vertices, &dims, NULL, &mapID, NULL, 0, 1);
    if(ccmErr != kCCMIONoErr)
    {
        debug4 << "CCMIOReadVertices for first vertex dimensions ";
        debug4 << "failed with error " << ccmErr << endl;
        EXCEPTION1(InvalidVariableException, meshname);
    }

    // Read the vertex ids
    intVector tmpVertexMap(nnodes);
    CCMIOReadMap(&ccmErr, mapID, &tmpVertexMap[0], kCCMIOStart, kCCMIOEnd);
    IDMap  vertexIDMap;
    vertexIDMap.SetIDs(tmpVertexMap);
    tmpVertexMap.clear();

    // Get the topology information
    CCMIOID faceID, cellsID;
    unsigned int nIFaces = 0, nCells = 0, size = 0;
    intVector cells;
    //intVector cellMatType;

    // Read the cells entity
    CCMIOGetEntity(&ccmErr, topology, kCCMIOCells, 0, &cellsID);
    // Read the cells entity size (num cells)
    CCMIOEntitySize(&ccmErr, cellsID, &nCells, NULL);
    cells.resize(nCells);
    cellInfo.resize(nCells);
    //cellMatType.resize(nCells);
    // this gets the cell types and the map that stores the cell ids
    CCMIOReadCells(&ccmErr, cellsID, &mapID, NULL,
                   kCCMIOStart, kCCMIOEnd);
    // this reads the cellids from the map.
    CCMIOReadMap(&ccmErr, mapID, &cells[0], kCCMIOStart, kCCMIOEnd);
    for (int i = 0; i < nCells; ++i)
        cellInfo[i].id = cells[i];

    IDMap cellIDMap;
    cellIDMap.SetIDs(cells);

    // Read the boundary faces.
    int index = 0;
    int count = 0;
    int nBoundaries = 0;
    while (CCMIONextEntity(NULL, topology, kCCMIOBoundaryFaces, &index, 
                           &faceID) == kCCMIONoErr)
    {
        nBoundaries++;
    }
     
    index = 0;
    while (CCMIONextEntity(NULL, topology, kCCMIOBoundaryFaces, &index, 
                           &faceID) == kCCMIONoErr)
    {
        CCMIOSize nBFaces;
        CCMIOEntitySize(&ccmErr, faceID, &nBFaces, NULL);
        GetFaces(faceID, kCCMIOBoundaryFaces, nBFaces, 
                 cellIDMap, vertexIDMap,
                 minFaceSize, maxFaceSize, cellInfo); 
    }

    // Read the internal faces.
    // Get the InternalFaces entity.
    CCMIOGetEntity(&ccmErr, topology, kCCMIOInternalFaces, 0, &faceID);
    // Get the InternalFaces size (num faces).
    CCMIOEntitySize(&ccmErr, faceID, &nIFaces, NULL);
    
    GetFaces(faceID, kCCMIOInternalFaces, nIFaces, cellIDMap, vertexIDMap,
             minFaceSize, maxFaceSize, cellInfo);


    if (find(varsOnSubmesh.begin(), varsOnSubmesh.end(), activeVisItVar) 
            != varsOnSubmesh.end())
    {
        // need to reduce the number of cells we actually process.
        intVector validCells;
        CellInfoVector vcv;
        GetCellMapData(dom, activeVisItVar, validCells);
          
        for (int i = 0; i < cellInfo.size(); ++i)
        {
            if (find(validCells.begin(), validCells.end(), cellInfo[i].id)
                     != validCells.end())
            {
                vcv.push_back(cellInfo[i]);
            }
        }
        cellInfo = vcv;
    } 

    debug5 << "minFaceSize = " << minFaceSize
           << ", maxFaceSize = " << maxFaceSize << endl;
}

#ifdef ENABLE_SUBDIVISION
// ****************************************************************************
// Method: ComputePatchCenter
//
// Purpose: 
//   Computes the center and bounds of the patch.
//
// Arguments:
//   centers : The centers for each cell in the mesh.
//   patch   : The list of cellids that make up the patch.
//   center  : The calculated center of the patch.
//   bounds  : The calculated bounds of the patch.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct  6 16:12:08 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

static void
ComputePatchCenter(const double *centers, const intVector &patch, double *center, double *bounds)
{
    center[0] = 0.;
    center[1] = 0.;
    center[2] = 0.;
    int nMatches = 0;
    for(int i = 0; i < patch.size(); ++i)
    {
        const double *c = centers + patch[i] * 3;

        // Compute extents of cell centers.
        if(nMatches == 0)
        {
            bounds[0] = bounds[1] = c[0];
            bounds[2] = bounds[3] = c[1];
            bounds[4] = bounds[5] = c[2];
        }
        else
        {
            if(c[0] < bounds[0])
                bounds[0] = c[0];
            if(c[0] > bounds[1])
                bounds[1] = c[0];

            if(c[1] < bounds[2])
                bounds[2] = c[1];
            if(c[1] > bounds[3])
                bounds[3] = c[1];

            if(c[2] < bounds[4])
                bounds[4] = c[2];
            if(c[2] > bounds[5])
                bounds[5] = c[2];
        }

        center[0] += c[0];
        center[1] += c[1];
        center[2] += c[2];
        nMatches++;
    }
    if(nMatches > 0)
    {
        center[0] /= double(nMatches);
        center[1] /= double(nMatches);
        center[2] /= double(nMatches);
    }
}

// ****************************************************************************
// Method: DivideLargestPatch
//
// Purpose: 
//   Divides the largest patch in the patch vector.
//
// Arguments:
//   centers : The centers of all cells in the mesh.
//   patches : The list of all patches.
//
// Returns:    
//
// Note:       This routine modifies the patches vector by splitting 1 of the
//             patches, replacing the split patch with piece0. piece1 is appended
//             to the patch vector.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct  6 16:13:33 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

static void
DivideLargestPatch(const double *centers, std::vector<intVector> &patches)
{
    // Find the index of the largest patch
    int maxIndex = 0;
    for(int i = 1; i < patches.size(); ++i)
        if(patches[i].size() > patches[maxIndex].size())
            maxIndex = i;

    // Compute the center at which we will bisect.
    double center[3], bounds[6];
    ComputePatchCenter(centers, patches[maxIndex], center, bounds);

    // Figure out the longest dimension since that's the dimension we'll bisect.
    double dX = bounds[1] - bounds[0];
    double dY = bounds[3] - bounds[2];
    double dZ = bounds[5] - bounds[4];
    int longestDimension = 2;
    if(dX > dY)
    {
        if(dX > dZ)
            longestDimension = 0;
    }
    else
    {
        if(dY > dZ)
            longestDimension = 1;
    }

    const intVector &patch = patches[maxIndex];
    intVector piece0, piece1;
    for(int j = 0; j < patch.size(); ++j)
    {
        const double *c = centers + patch[j] * 3;
        if(c[longestDimension] > center[longestDimension])
            piece0.push_back(patch[j]);
        else
            piece1.push_back(patch[j]);
    }
    patches[maxIndex] = piece0;
    patches.push_back(piece1);
}
#endif

// ****************************************************************************
// Method: avtCCMFileFormat::SelectCellsForThisProcessor
//
// Purpose: 
//   This routine divides the cells spatially into PAR_Size() different bins
//   and sets all of the ids for the cells in cellInfo to -1 (invalid) unless
//   their bin matches PAR_Rank(). This means that we are marking a subset of
//   the cells in cellInfo as being valid so we can return just a part of the
//   dataset.
//
// Arguments:
//   cellInfo : The vector of cell data.
//   points   : The points used by the cells.
//
// Returns:    
//
// Note:       This method is just used in parallel when we have a single 
//             domain dataset that we want to automatically chunk up under the
//             covers.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  1 13:25:17 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtCCMFileFormat::SelectCellsForThisProcessor(CellInfoVector &cellInfo, vtkPoints *points)
{
#ifdef ENABLE_SUBDIVISION
    if(subdividingSingleMesh)
    {
        // Compute cell centers
        double *centers = new double[cellInfo.size() * 3];
        for(size_t i = 0; i < cellInfo.size(); ++i)
            cellInfo[i].CellCenter(centers + i * 3, points);

        // Start out with all cells in 1 patch
        std::vector<intVector> patches;
        intVector allCells;
        for(size_t i = 0; i < cellInfo.size(); ++i)
            allCells.push_back(i);
        patches.push_back(allCells);

        // Divide the largest patch until we have enough patches.
        while(patches.size() < PAR_Size())
            DivideLargestPatch(centers, patches);

        // Set cellid to -1 unless we're on the patch whose id == PAR_Rank.
        for(size_t p = 0; p < patches.size(); ++p)
        {
            if(p == PAR_Rank())
                continue;

            const intVector &patch = patches[p];
            for(size_t i = 0; i < patch.size(); ++i)
                cellInfo[patch[i]].id = -1;
        }

        delete [] centers;
    }
#endif
}


// ****************************************************************************
//  Method:  avtCCMFileFormat::RegisterVariableList
//
//  Purpose:
//    Records the active variable name so only cells valid for the var
//    will be retrieved during GetMesh calls.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 28, 2008
//
// ***************************************************************************

void
avtCCMFileFormat::RegisterVariableList(const char *primaryVar,
                                       const std::vector<CharStrRef> &)
{
    activeVisItVar = primaryVar;
}


// ****************************************************************************
//  Method:  avtCCMFileFormat::GetCellMapData
//
//  Purpose:
//    Retrieves the map data for a given var, which specifies the cell
//    ids the var is defined upon. 
//
//  Arguments:
//    domain     The required domain.
//    var        The requested variable name.
//    mapData    A place to store the retrieved map data.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 28, 2008
//
// ***************************************************************************

void
avtCCMFileFormat::GetCellMapData(const int domain, const string &var, 
    intVector &mapData)
{
    VarFieldMap::const_iterator pos = varsToFields.find(var.c_str());
    if (pos == varsToFields.end())
        EXCEPTION1(InvalidVariableException, var);

    mapData.clear();
  
    CCMIOSize n;
    CCMIOIndex fmax;
    CCMIOError ferr = kCCMIONoErr;
    int j = 0, cnt = 0;
    CCMIOID fieldData, mapID;
    CCMIODimensionality cdims;
    CCMIOID field = pos->second; 
  
    CCMIOReadField(&ferr, field, NULL, NULL, &cdims, NULL);
    if (cdims == kCCMIOVector)
    {
        CCMIOID scalar;
        ferr = kCCMIONoErr;
        CCMIOReadMultiDimensionalFieldData(&ferr, field, 
                                             kCCMIOVectorX, &scalar);
        if (ferr == kCCMIONoErr)
        {
            // componentized vector, use the X component scalar
            // in order to retrieve more info about the vector
            field = scalar;
        }
    }
    ferr = kCCMIONoErr;
    while(CCMIONextEntity(NULL, field, kCCMIOFieldData, &j, &fieldData)
                                                              == kCCMIONoErr)
    {
        CCMIOEntitySize(&ferr, fieldData, &n, &fmax);
        CCMIOReadFieldDataf(&ferr, fieldData, &mapID, NULL, NULL, 
                            kCCMIOStart, kCCMIOEnd);
        mapData.resize(cnt+n);
        CCMIOReadMap(&ferr, mapID, &mapData[cnt], kCCMIOStart, kCCMIOEnd);
        cnt += n;
    } 
}


// ****************************************************************************
//  Method: avtCCMFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 28 15:00:24 PST 2008
//    avtOriginalCellNumbers array now contains CCM cellid, so ensure that
//    the data array is indexed-into correctly, by mapping the original cell
//
//    Brad Whitlock, Thu Oct  1 13:08:35 PDT 2009
//    Set domain to 0 if we're subdividing a single mesh.
//
// ****************************************************************************

vtkDataArray *
avtCCMFileFormat::GetVar(int domain, const char *varname)
{
    const char *mName = "avtCCMFileFormat::GetVar: ";
    // Override domain if we're automatically dividing the data
    domain = subdividingSingleMesh ? 0 : domain;

    VarFieldMap::const_iterator pos = varsToFields.find(varname);
    if (pos == varsToFields.end())
        EXCEPTION1(InvalidVariableException, varname);

    intVector mapData;
    floatVector data;
    ReadScalar(pos->second, mapData, data);

    if (ccmErr != kCCMIONoErr)
        EXCEPTION1(InvalidVariableException, varname);
       
    unsigned int nvalues = data.size();
 
    vtkFloatArray *rv = vtkFloatArray::New();
    if (originalCells[domain] == NULL)
    {
        rv->SetNumberOfValues(nvalues);
        for (unsigned int i = 0 ; i < nvalues ; ++i)
        {
            rv->SetValue(i, data[i]);
        }
    }
    else 
    {
        // We've tesselated, and have more cells than nvalues.  Need to 
        // duplicate values for all cells that share the same original 
        // cell number!
        vtkUnsignedIntArray *ocarray = 
            vtkUnsignedIntArray::SafeDownCast(originalCells[domain]);
        int numCells = ocarray->GetNumberOfTuples();

        debug4 << mName << "numCells = " << numCells << endl;
        unsigned int *oc = ocarray->GetPointer(0);
        rv->SetNumberOfValues(numCells);
        std::map<int, int> cellIdMap;
        for (unsigned int i = 0; i < mapData.size(); i++)
        {
            cellIdMap[mapData[i]] = i;
        }

        for (unsigned int i = 0 ; i < numCells ; i++)
        {
            rv->SetValue(i, data[cellIdMap[oc[i*2+1]]]);
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtCCMFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 28 15:00:24 PST 2008
//    avtOriginalCellNumbers array now contains CCM cellid, so enusre that
//    the data array is indexed-into correctly, by mapping the original cell
//    id through an cellIdMap of valid cell ids (obtained from MapData).
//
//    Dave Bremer, Fri Apr 11 16:49:45 PDT 2008
//    Initialize the err variable.
//
//    Brad Whitlock, Thu Oct  1 13:08:35 PDT 2009
//    Set domain to 0 if we're subdividing a single mesh.
//
// ****************************************************************************

vtkDataArray *
avtCCMFileFormat::GetVectorVar(int domain, const char *varname)
{
    // Override domain if we're automatically dividing the data
    domain = subdividingSingleMesh ? 0 : domain;

    VarFieldMap::const_iterator pos = varsToFields.find(varname);
    if (pos == varsToFields.end())
        EXCEPTION1(InvalidVariableException, varname);

    intVector mapData;
    floatVector u, v, w, data;
    CCMIOID scalar;
    CCMIOID field = pos->second;
    CCMIOError err = kCCMIONoErr;

    CCMIOReadMultiDimensionalFieldData(&err, field, kCCMIOVectorX, &scalar);

    if (err == kCCMIOVersionErr)
    {
        // If we are reading an older version of the file,
        // where vectors are stored as vectors, not components,
        // we need to call CCMIOReadFieldData*(), which is
        // all that ReadScalar() does.
        err = kCCMIONoErr;
        ReadScalar(field, mapData, data, true);
    }
    else
    {
        ReadScalar(scalar, mapData, u);
        CCMIOReadMultiDimensionalFieldData(&err, field, kCCMIOVectorY, &scalar);
        ReadScalar(scalar, mapData, v);
        CCMIOReadMultiDimensionalFieldData(&err, field, kCCMIOVectorZ, &scalar);
        ReadScalar(scalar, mapData, w);
        data.resize(3 * u.size());
        for (unsigned int k = 0;  k < u.size();  ++k)
        {
            data[3 * k    ] = u[k];
            data[3 * k + 1] = v[k];
            data[3 * k + 2] = w[k];
        }
    }

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(3);
    if (originalCells[domain] == NULL)
    {
        unsigned int nvalues = data.size();
        rv->SetNumberOfTuples(nvalues/3);
        float *v = rv->WritePointer(0, nvalues);
        for (unsigned int i = 0 ; i < nvalues ; i++)
        {
            v[i] = data[i];
        }
    }
    else 
    {
        // We've tesselated, and have more cells than nvalues.  Need to 
        // duplicate values for all cells that share the same original 
        // cell number!
        vtkUnsignedIntArray *ocarray = 
            vtkUnsignedIntArray::SafeDownCast(originalCells[domain]);
        int numCells = ocarray->GetNumberOfTuples();

        unsigned int *oc = ocarray->GetPointer(0);
        rv->SetNumberOfTuples(numCells);
        float *v = rv->WritePointer(0, numCells*3);
      
        std::map<int, int> cellIdMap;
        for (unsigned int i = 0; i < mapData.size(); ++i)
        {
            cellIdMap[mapData[i]] = i;
        }

        for (unsigned int i = 0 ; i < numCells; ++i)
        {
            unsigned int id = cellIdMap[oc[i*2+1]];
            v[i*3+0] = data[id*3+0];
            v[i*3+1] = data[id*3+1];
            v[i*3+2] = data[id*3+2];
        }
    }
    return rv; 
}


// ***************************************************************************
//  Method: avtCCMFileFormat::ReadScalar
//
//  Purpose:
//    Reads scalar data from the file.
//
//  Arguments:
//    field      The ID of the field to read. 
//    mapData    A place to store data specifying which cells the scalar 
//               is defined upon.
//    data       A place to store the scalar data.
//    readingVector       Indicates if an old-style vector is being read.
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 28 15:00:24 PST 2008
//    Loop through field entities, as a var may be delineated by cell types,
//    thus being represented by multiple 'fields'.  Correctly resize mapData 
//    and data each time through the loop.  Combined code for Cell and Node 
//    data as they were exactly the same.
//
// ***************************************************************************

void
avtCCMFileFormat::ReadScalar(CCMIOID field, intVector &mapData, 
                             floatVector &data, bool readingVector)

{
    const char *mName = "avtCCMFileFormat::ReadScalar: ";
    CCMIOSize n;
    CCMIOIndex fmax;
    int j = 0, cnt = 0;
    CCMIOID fieldData, mapID;
    CCMIODataLocation type;

    mapData.clear();
    data.clear();
    // Read each piece of field data
    // The fields may be delineated by cell types, so there may be 
    // multiple fields for the same variable.
    while (CCMIONextEntity(NULL, field, kCCMIOFieldData, &j, &fieldData)
                                                               == kCCMIONoErr)
    {
        // Figure out how big this data is so we can read it. If we were
        // storing this information permanently we might use a sparse
        // array, in which case we would need to find the maximum ID and
        // make the array that size.
        CCMIOEntitySize(&ccmErr, fieldData, &n, &fmax);
        CCMIOReadFieldDataf(&ccmErr, fieldData, &mapID, &type, NULL,
                            kCCMIOStart, kCCMIOEnd);

        if (type == kCCMIOCell || type == kCCMIOVertex)
        {
            mapData.resize(cnt +n);
            CCMIOReadMap(&ccmErr, mapID, &mapData[cnt], kCCMIOStart, kCCMIOEnd);
            if (readingVector)
                data.resize(cnt +(3 * n));
            else
                data.resize(cnt + n);
            if (type == kCCMIOCell)
                debug4 << mName << "Reading cell data n= " << n << endl;
            else 
                debug4 << mName << "Reading node data n= " << n << endl;
            // If we want double precision we should use
            // CCMIOReadFieldDatad().
            CCMIOReadFieldDataf(&ccmErr, fieldData, &mapID, NULL,
                                &data[cnt], kCCMIOStart, kCCMIOEnd);
            cnt += n;
        }
#if 0
        else if (type == kCCMIOFace)
        {
            debug3 << "\tReadScalar found type kCCMIOFace" << endl;
        }
#endif

        if (ccmErr != kCCMIONoErr)
            debug1 << "  Error reading scalar data " << ccmErr << endl; 
    }
}

// ****************************************************************************
//  Method: avtCCMFileFormat::TesselateCell
//
//  Purpose:
//      
//     
//    
//
//  Arguments:
//    civ       Contains cell information. 
//    points    The points comprising the dataset.
//    ugrid     The unstructured grid we are building. 
//    
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 1, 2007 
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 28 15:00:24 PST 2008
//    Use cellid stored in CellInfo in avtOriginalCellNumbers array.
//
//    Kathleen Bonnell, Thu Mar  6 09:21:02 PST 2008 
//    Change fbounds to doubleVector to get around compiler problem on Windows. 
//
//    Dave Bremer, Fri Apr  4 16:29:49 PDT 2008
//    Fixed a bug in which cell and vertex ids were mistaken for 1-based
//    indices.
//
// ****************************************************************************

void
avtCCMFileFormat::TesselateCell(const int domain, const CellInfoVector &civ, 
    vtkPoints *points, vtkUnstructuredGrid *ugrid)
{
#ifndef MDSERVER
    int dom = subdividingSingleMesh ? 0 : domain;

    const char *mName = "avtCCMFileFormat::TesselateCell: ";
    unsigned int i, j, k;
    unsigned int tetCount = 0;
    vtkPoints *pts = vtkPoints::New();
    pts->Allocate(points->GetNumberOfPoints());
    VertexManager uniqueVerts(pts);
    ccmPolygonToTriangles tess(&uniqueVerts);
    unsigned int oc[2] = {dom, 0};
    
    int useCount = 0;
    for(i = 0; i < civ.size(); ++i)
         useCount += (civ[i].id != -1) ? 1 : 0;

    originalCells[dom] = vtkUnsignedIntArray::New();
    originalCells[dom]->SetName("avtOriginalCellNumbers");
    originalCells[dom]->SetNumberOfComponents(2);
    originalCells[dom]->Allocate(useCount * 3);

    for (i = 0; i < civ.size(); ++i)
    {
        const CellInfo &ci = civ[i];
        if(ci.id == -1)
            continue;

        oc[1] = ci.id;
        int nFaces  = ci.faces.size();
        int nPts = 0;
        doubleVector fbounds;
        for (j = 0; j < nFaces; ++j)
        {
            nPts += ci.faces[j].nodes.size();
            fbounds.push_back(VTK_LARGE_FLOAT);
            fbounds.push_back(-VTK_LARGE_FLOAT);
            fbounds.push_back(VTK_LARGE_FLOAT);
            fbounds.push_back(-VTK_LARGE_FLOAT);
            fbounds.push_back(VTK_LARGE_FLOAT);
            fbounds.push_back(-VTK_LARGE_FLOAT);
        }
        double *pt;
        double cbounds[6] = {VTK_LARGE_FLOAT, -VTK_LARGE_FLOAT, 
                             VTK_LARGE_FLOAT, -VTK_LARGE_FLOAT, 
                             VTK_LARGE_FLOAT, -VTK_LARGE_FLOAT};

        int cnt = 0;
        for (j = 0; j < nFaces; ++j)
        {
            const intVector &nodes = ci.faces[j].nodes;
                
            for (k = 0; k < nodes.size(); ++k)
            {
                cnt++;
                pt = points->GetPoint(nodes[k]);

                if (pt[0] < cbounds[0])
                    cbounds[0] = pt[0];
                if (pt[0] > cbounds[1])
                    cbounds[1] = pt[0];
                if (pt[1] < cbounds[2])
                    cbounds[2] = pt[1];
                if (pt[1] > cbounds[3])
                    cbounds[3] = pt[1];
                if (pt[2] < cbounds[4])
                    cbounds[4] = pt[2];
                if (pt[2] > cbounds[5])
                    cbounds[5] = pt[2];

                if (pt[0] < fbounds[j*6+0])
                    fbounds[j*6+0] = pt[0];
                if (pt[0] > fbounds[j*6+1])
                    fbounds[j*6+1] = pt[0];
                if (pt[1] < fbounds[j*6+2])
                    fbounds[j*6+2] = pt[1];
                if (pt[1] > fbounds[j*6+3])
                    fbounds[j*6+3] = pt[1];
                if (pt[2] < fbounds[j*6+4])
                    fbounds[j*6+4] = pt[2];
                if (pt[2] > fbounds[j*6+5])
                    fbounds[j*6+5] = pt[2];
            } // k nodes
        } // j faces
            
        double cc[3] = {0.,0.,0.};
        double fc[3] = {0.,0.,0.};
        for (j = 0; j < 3; ++j)
            cc[j] = (cbounds[2*j+1]+cbounds[2*j])/2.0; 
        int centerId = uniqueVerts.GetVertexId(cc);

        for (j = 0; j < nFaces; ++j)
        {
            // Find the face center
            const intVector &nodes = ci.faces[j].nodes;
            double fc[3] = {0.,0.,0.};
            for (k = 0; k < 3; ++k)
                fc[k] = (fbounds[2*k+1+(6*j)]+fbounds[2*k+(6*j)])/2.0; 

            // Tesselate the face
            double n[3] = {(cc[0] - fc[0]), (cc[1] - fc[1]), (cc[2] - fc[2])};
            tess.SetNormal(n);
            tess.BeginPolygon();
            tess.BeginContour();
            for (k = 0; k < nodes.size(); ++k)
            {
                cnt++;
                pt = points->GetPoint(nodes[k]);
                tess.AddVertex(pt);
            } // k nodes
            tess.EndContour();
            tess.EndPolygon();

            // Make a tet for each triangle in the face to the cell center.
            vtkIdType verts[4];
            verts[3] = centerId;
            if (tess.GetNumTriangles() > 0)
            {
                for (k = 0; k < tess.GetNumTriangles(); ++k)
                {
                    int a, b, c;
                    tess.GetTriangle(k, a, b, c);
                    verts[0] = a; 
                    verts[1] = b; 
                    verts[2] = c; 
                    ugrid->InsertNextCell(VTK_TETRA, 4, verts);
                    ((vtkUnsignedIntArray*)originalCells[dom])->
                        InsertNextTupleValue(oc);
                }
                tetCount += tess.GetNumTriangles();
            }
            // prepare for next cell
            tess.ClearTriangles();
        } // end face
    }
    pts->Squeeze();

    ugrid->SetPoints(pts);
    pts->Delete();
    ugrid->GetCellData()->AddArray(originalCells[dom]);
    ugrid->GetCellData()->CopyFieldOn("avtOriginalCellNumbers");

    debug4 << mName << "Input number of polyhedral cells: " << civ.size() 
           << endl;
    debug4 << mName << "Output tetrahedral cells: " << tetCount << endl;
#endif
}

// ****************************************************************************
// Method: avtCCMFileFormat::TesselateCells2D
//
// Purpose: 
//   Adds the 2D cells to the unstructured grid, tessellating them as needed.
//
// Arguments:
//   dom    : The domain number
//   civ    : The cell information vector.
//   points : The points that we'll allocate during tessellation.
//   ugrid  : The return unstructured grid object.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 12:54:56 PST 2007
//
// Modifications:
//   Kathleen Bonnell, Thu Feb 28 15:00:24 PST 2008
//   Use cellid stored in CellInfo in avtOriginalCellNumbers array.
//   
//   Kathleen Bonnell, Thu Mar  6 09:21:02 PST 2008 
//   Remove unused variables.
//
//   Dave Bremer, Fri Apr  4 16:29:49 PDT 2008
//   Fixed a bug in which cell and vertex ids were mistaken for 1-based
//   indices.
//   
// ****************************************************************************

typedef std::pair<int,int> edge_pair;

void
avtCCMFileFormat::TesselateCells2D(const int domain, const CellInfoVector &civ, 
    vtkPoints *points, vtkUnstructuredGrid *ugrid)
{
#ifndef MDSERVER
    int dom = subdividingSingleMesh ? 0 : domain;

    unsigned int i, k;
    vtkPoints *pts = vtkPoints::New();
    pts->Allocate(points->GetNumberOfPoints());
    VertexManager uniqueVerts(pts);
    ccmPolygonToTriangles tess(&uniqueVerts);
    unsigned int oc[2] = {dom, 0};

    int useCount = 0;
    for(i = 0; i < civ.size(); ++i)
         useCount += (civ[i].id != -1) ? 1 : 0;

    originalCells[dom] = vtkUnsignedIntArray::New();
    originalCells[dom]->SetName("avtOriginalCellNumbers");
    originalCells[dom]->SetNumberOfComponents(2);
    originalCells[dom]->Allocate(useCount*3);

    const double n[3] = {0., 0., 1.};

    for (i = 0; i < civ.size(); ++i)
    {
        const CellInfo &ci = civ[i];
        if(ci.id == -1)
            continue;

        oc[1] = ci.id;
        tess.SetNormal(n);
        tess.BeginPolygon();

        // We need to sort the edges around the polygon so we can tessellate
        // by adding the first vertex of each edge to define the contour. We
        // could do a different approach making n-1 triangles for the number
        // of edges in the shape but this way supports concave polygons whereas
        // the other approach does not.

        // Put all of the line segments in a pool of free edges.
        std::set<edge_pair> freeEdges;
        for (int f = 0; f < ci.faces.size(); ++f)
        {
             edge_pair e01(ci.faces[f].nodes[0], ci.faces[f].nodes[1]);
             freeEdges.insert(e01);
        }

        while(freeEdges.size() > 0)
        {
            std::deque<int> shape;

            // Get seed edge and remove it from the pool
            edge_pair currentEdge;
            if(freeEdges.begin() != freeEdges.end())
            {
                currentEdge = *freeEdges.begin();
                freeEdges.erase(freeEdges.begin());
            }
            shape.push_back(currentEdge.first);
            shape.push_back(currentEdge.second);

            // Now, look for edges that contain either of the points in
            // the current edge.
            bool found;
            do
            {
                found = false;
                for(std::set<edge_pair>::iterator pos = freeEdges.begin();
                    pos != freeEdges.end() && !found; ++pos)
                {
                    if(currentEdge.first == pos->first)
                    {
                        currentEdge.first = pos->second;
                        shape.push_front(pos->second);
                        freeEdges.erase(pos);
                        found = true;
                    }
                    else if(currentEdge.first == pos->second)
                    {
                        currentEdge.first = pos->first;
                        shape.push_front(pos->first);
                        freeEdges.erase(pos);
                        found = true;
                    }
                    else if(currentEdge.second == pos->first)
                    {
                        currentEdge.second = pos->second;
                        shape.push_back(pos->second);
                        freeEdges.erase(pos);
                        found = true;
                    }
                    else if(currentEdge.second == pos->second)
                    {
                        currentEdge.second = pos->first;
                        shape.push_back(pos->first);
                        freeEdges.erase(pos);
                        found = true;
                    }
                }
            } while(found);

            if(shape.size() > 2)
            {
                tess.BeginContour();
                for(int v = 0; v < shape.size(); ++v)
                    tess.AddVertex(points->GetPoint(shape[v]));
                tess.EndContour();
            }
        }
        tess.EndPolygon();

        vtkIdType verts[4];
        if (tess.GetNumTriangles() > 0)
        {
            for (k = 0; k < tess.GetNumTriangles(); ++k)
            {
                int a, b, c;
                tess.GetTriangle(k, a, b, c);
                verts[0] = a; 
                verts[1] = b; 
                verts[2] = c; 
                ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);

                ((vtkUnsignedIntArray*)originalCells[dom])->
                    InsertNextTupleValue(oc);
            }
        }

        // prepare for next cell
        tess.ClearTriangles();
    }

    pts->Squeeze();
    ugrid->SetPoints(pts);
    pts->Delete();
    ugrid->GetCellData()->AddArray(originalCells[dom]);
    ugrid->GetCellData()->CopyFieldOn("avtOriginalCellNumbers");
#endif
}

// ****************************************************************************
// Method: avtCCMFileFormat::BuildHex
//
// Purpose: 
//   Creates a VTK hex cell from CCM faces.
//
// Programmer: Kathleen Bonnell 
// Creation:   October 1, 2007 
//
// Modifications:
//   Kathleen Bonnell, Thu Mar  6 09:21:02 PST 2008 
//   Remove unused variables.
//
//   Dave Bremer, Fri Apr  4 16:29:49 PDT 2008
//   Fixed a bug in which cell and vertex ids were mistaken for 1-based
//   indices.
//   
// ****************************************************************************

void
avtCCMFileFormat::BuildHex(const CellInfo &ci, vtkCellArray *cellArray, 
                           intVector &cellTypes)
{
    unsigned int i, j;
    const FaceInfoVector &faces = ci.faces;
    intVector uniqueNodes; 
    bool useface;
    bool usedBF = false;
    int nnodes = 0;
    vtkIdList *cellNodes = vtkIdList::New();
      
    for (i = 0; i < faces.size(); ++i)
    {
        useface = true;
        int nnodes = faces[i].nodes.size(); 
        if (nnodes != 4)
        {
            return; 
        }
        for (j = 0; useface && j <nnodes; ++j)
        {
            useface = (count(uniqueNodes.begin(), uniqueNodes.end(), 
                             faces[i].nodes[j]) == 0);
        }
        if (useface)
        {
            if ((ci.faceTypes[i] == 0 && !usedBF) || (ci.faceTypes[i] == 2))
            {
                usedBF = (ci.faceTypes[i] == 0);
                // reorder this face
                for (j = 0; j < nnodes; ++j)
                {
                    uniqueNodes.push_back(faces[i].nodes[j]);
                }
                cellNodes->InsertNextId(faces[i].nodes[0]);
                cellNodes->InsertNextId(faces[i].nodes[3]);
                cellNodes->InsertNextId(faces[i].nodes[2]);
                cellNodes->InsertNextId(faces[i].nodes[1]);
            }
            else
            {
                for (j = 0; j < nnodes; ++j)
                {
                    uniqueNodes.push_back(faces[i].nodes[j]);
                    cellNodes->InsertNextId(faces[i].nodes[j]);
                }
            }
        }
    }
    cellArray->InsertNextCell(cellNodes);
    cellTypes.push_back(VTK_HEXAHEDRON);
    cellNodes->Delete();
}


// ****************************************************************************
// Method: avtCCMFileFormat::BuildTet
//
// Purpose: 
//   Creates a VTK tet cell from CCM faces.
//
// Programmer: Kathleen Bonnell 
// Creation:   October 1, 2007 
//
// Modifications:
//   Kathleen Bonnell, Thu Mar  6 09:21:02 PST 2008
//   Fix '==' '=' mix-up.
//
//   Dave Bremer, Fri Apr  4 16:29:49 PDT 2008
//   Fixed a bug in which cell and vertex ids were mistaken for 1-based
//   indices.
//   
// ****************************************************************************

void
avtCCMFileFormat::BuildTet(const CellInfo &ci, vtkCellArray *cellArray, 
                           intVector &cellTypes)
{
    unsigned int i, j;
    const FaceInfoVector &faces = ci.faces;
    intVector uniqueNodes; 
    bool lastNodeFound = false;
    vtkIdList *cellNodes = vtkIdList::New();

    for (i = 0; i < faces.size(); ++i)
    {
        if (faces[i].nodes.size() != 3)
        {
           return;
        }
    }
  
    if (ci.faceTypes[0] != 1)
    { 
        uniqueNodes.push_back(faces[0].nodes[0]);  
        uniqueNodes.push_back(faces[0].nodes[2]);  
        uniqueNodes.push_back(faces[0].nodes[1]);  
        cellNodes->InsertNextId(faces[0].nodes[0]);  
        cellNodes->InsertNextId(faces[0].nodes[2]);  
        cellNodes->InsertNextId(faces[0].nodes[1]);  
    } 
    else
    { 
        uniqueNodes.push_back(faces[0].nodes[0]);  
        uniqueNodes.push_back(faces[0].nodes[1]);  
        uniqueNodes.push_back(faces[0].nodes[2]);  
        cellNodes->InsertNextId(faces[0].nodes[0]);  
        cellNodes->InsertNextId(faces[0].nodes[1]);  
        cellNodes->InsertNextId(faces[0].nodes[2]);  
    } 
      
    for (i = 1; !lastNodeFound && i < faces.size(); ++i)
    {
        for (j = 0; !lastNodeFound && j <faces[i].nodes.size(); ++j)
        {
           if ((count(uniqueNodes.begin(), uniqueNodes.end(), 
                      faces[i].nodes[j]) == 0)) 
           {
               lastNodeFound = true;
               uniqueNodes.push_back(faces[i].nodes[j]);
               cellNodes->InsertNextId(faces[i].nodes[j]);  
           }
        }
            
    }
    cellArray->InsertNextCell(cellNodes);
    cellTypes.push_back(VTK_TETRA);
}


// ****************************************************************************
// Method: avtCCMFileFormat::BuildPyramid
//
// Purpose: 
//   Creates a VTK pyramid cell from CCM faces.
//
// Programmer: Kathleen Bonnell 
// Creation:   October 1, 2007 
//
// Modifications:
//
//   Dave Bremer, Fri Apr  4 16:29:49 PDT 2008
//   Fixed a bug in which cell and vertex ids were mistaken for 1-based
//   indices.
//   
// ****************************************************************************

void
avtCCMFileFormat::BuildPyramid(const CellInfo &ci, vtkCellArray *cellArray, 
                               intVector &cellTypes)
{
    unsigned int i, j;
    const FaceInfoVector &faces = ci.faces;
    intVector uniqueNodes; 
    int baseFace = 0;
    vtkIdList *cellNodes = vtkIdList::New();

    for (i = 0; i < faces.size(); ++i)
    {
        if (faces[i].nodes.size() == 4)
        {
            baseFace = i;
            for (j = 0; j < 4; ++j)
            {
                uniqueNodes.push_back(faces[i].nodes[j]);
                cellNodes->InsertNextId(faces[i].nodes[j]);
            }
        }
    }
    int triFace = (baseFace+1) % 4;
    for (i = 0; i < 3; ++i) // only 3 nodes in the triangle-face
    {
        if ((count(uniqueNodes.begin(), uniqueNodes.end(), 
             faces[triFace].nodes[i])) == 0)
        {
            cellNodes->InsertNextId(faces[triFace].nodes[i]);
            break;
        }
    }

    cellArray->InsertNextCell(cellNodes);
    cellTypes.push_back(VTK_PYRAMID);
}


// ****************************************************************************
// Method: avtCCMFileFormat::BuildWedge
//
// Purpose: 
//   Creates a VTK wedge cell from CCM faces.
//
// Programmer: Kathleen Bonnell 
// Creation:   October 1, 2007 
//
// Modifications:
//   Kathleen Bonnell, Thu Mar  6 09:21:02 PST 2008 
//   Remove unused variables.
//
//   Dave Bremer, Fri Apr  4 16:29:49 PDT 2008
//   Fixed a bug in which cell and vertex ids were mistaken for 1-based
//   indices.
//   
// ****************************************************************************

void
avtCCMFileFormat::BuildWedge(const CellInfo &ci, vtkCellArray *cellArray, 
                             intVector &cellTypes)
{
    unsigned int i;
    const FaceInfoVector &faces = ci.faces;
    vtkIdList *cellNodes = vtkIdList::New();
    for (i = 0; i < faces.size(); ++i)
    {
        if (faces[i].nodes.size() == 3)
        {
            cellNodes->InsertNextId(faces[i].nodes[0]);
            cellNodes->InsertNextId(faces[i].nodes[1]);
            cellNodes->InsertNextId(faces[i].nodes[2]);
        }
    }
    cellArray->InsertNextCell(cellNodes);
    cellTypes.push_back(VTK_WEDGE);
}






//
// avtCCMFileFormat::FaceInfo class
//

// ****************************************************************************
// Method: avtCCMFileFormat::FaceInfo::FaceInfo
//
// Purpose: 
//   Constructor
//
// Programmer: Kathleen Bonnell 
// Creation:   September 6, 2007 
//
// Modifications:
//
//   Dave Bremer, Fri Apr  4 16:29:49 PDT 2008
//   Removed unused face id.
//   
// ****************************************************************************

avtCCMFileFormat::FaceInfo::FaceInfo()
{
    //id = -1;
    cells[0] = -1;
    cells[1] = -1;
}

// ****************************************************************************
// Method: avtCCMFileFormat::FaceInfo::FaceInfo
//
// Purpose: 
//   Copy Constructor
//
// Programmer: Kathleen Bonnell 
// Creation:   September 6, 2007 
//
// Modifications:
//
//   Dave Bremer, Fri Apr  4 16:29:49 PDT 2008
//   Removed unused face id.
//   
// ****************************************************************************

avtCCMFileFormat::FaceInfo::FaceInfo(const avtCCMFileFormat::FaceInfo &obj)
{
    //id = obj.id;
    cells[0] = obj.cells[0];
    cells[1] = obj.cells[1]; 
    nodes = obj.nodes;
}


// ****************************************************************************
// Method: avtCCMFileFormat::FaceInfo::~FaceInfo
//
// Purpose: 
//   Destructor
//
// Programmer: Kathleen Bonnell
// Creation:   September 6, 2007 
//
// Modifications:
//   
// ****************************************************************************

avtCCMFileFormat::FaceInfo::~FaceInfo()
{
}


// ****************************************************************************
// Method: avtCCMFileFormat::FaceInfo::operator =
//
// Purpose: 
//   Assignment operator 
//
// Programmer: Kathleen Bonnell 
// Creation:   September 6, 2007 
//
// Modifications:
//
//   Dave Bremer, Fri Apr  4 16:29:49 PDT 2008
//   Removed unused face id.
//   
// ****************************************************************************

void
avtCCMFileFormat::FaceInfo::operator =(const avtCCMFileFormat::FaceInfo &obj)
{
    //id = obj.id;
    cells[0] = obj.cells[0];
    cells[1] = obj.cells[1]; 
    nodes = obj.nodes;
}

//
// avtCCMFileFormat::CellInfo class
//

// ****************************************************************************
// Method: avtCCMFileFormat::CellInfo::CellInfo
//
// Purpose: 
//   Constructor
//
// Programmer: Kathleen Bonnell 
// Creation:   September 6, 2007 
//
// Modifications:
//   
// ****************************************************************************

avtCCMFileFormat::CellInfo::CellInfo()
{
    id = -1;
}

// ****************************************************************************
// Method: avtCCMFileFormat::CellInfo::CellInfo
//
// Purpose: 
//   Copy Constructor
//
// Programmer: Kathleen Bonnell 
// Creation:   September 6, 2007 
//
// Modifications:
//   
// ****************************************************************************

avtCCMFileFormat::CellInfo::CellInfo(const avtCCMFileFormat::CellInfo &obj)
{
    id = obj.id;
    faceTypes = obj.faceTypes;
    faces = obj.faces;
}


// ****************************************************************************
// Method: avtCCMFileFormat::CellInfo::~CellInfo
//
// Purpose: 
//   Destructor
//
// Programmer: Kathleen Bonnell
// Creation:   September 6, 2007 
//
// Modifications:
//   
// ****************************************************************************

avtCCMFileFormat::CellInfo::~CellInfo()
{
}


// ****************************************************************************
// Method: avtCCMFileFormat::CellInfo::operator =
//
// Purpose: 
//   Assignment operator 
//
// Programmer: Kathleen Bonnell 
// Creation:   September 6, 2007 
//
// Modifications:
//   
// ****************************************************************************

void
avtCCMFileFormat::CellInfo::operator =(const avtCCMFileFormat::CellInfo &obj)
{
    id = obj.id;
    faceTypes = obj.faceTypes;
    faces = obj.faces;
}

// ****************************************************************************
// Method: avtCCMFileFormat::CellInfo::CellCenter
//
// Purpose: 
//   Determines the cell center from its nodes.
//
// Arguments:
//   center : The returned center.
//   pts    : The global points array that stores all of the nodes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  1 14:02:37 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtCCMFileFormat::CellInfo::CellCenter(double *center, vtkPoints *pts) const
{
    int npts = 0;
    double c[3] = {0.,0.,0.};
    for(int i = 0; i < faces.size(); ++i)
    {
        for(int j = 0; j < faces[i].nodes.size(); ++j, ++npts)
        {
            double *pt = pts->GetPoint(faces[i].nodes[j]);
            c[0] += pt[0];
            c[1] += pt[1];
            c[2] += pt[2];
        }
    }
    center[0] = c[0] / double(npts);
    center[1] = c[1] / double(npts);
    center[2] = c[2] / double(npts);
}

// ****************************************************************************
// Method: avtCCMFileFormat::CellInfo::UseNodes
//
// Purpose: 
//   Sets a true value into a domain-node-sized bool array for each node that
//   the cell uses.
//
// Arguments:
//   pts : The array containing the values for whether a node is used.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  1 14:03:22 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtCCMFileFormat::CellInfo::UseNodes(bool *pts) const
{
    for(int i = 0; i < faces.size(); ++i)
        for(int j = 0; j < faces[i].nodes.size(); ++j)
            pts[faces[i].nodes[j]] = true;
}

// ****************************************************************************
// Method: operator << (ostream &os, CCMIOEntity e)
//
// Purpose: 
//   Prints a CCMIOEntity
//
// Arguments:
//   os : The stream to which we'll print.
//   e  : The value to print.
//
// Returns:    The input stream.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 15:16:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ostream &
operator << (ostream &os, CCMIOEntity e)
{
    switch(e)
    {
    case kCCMIOBoundaryRegion:
        os << "kCCMIOBoundaryRegion"; break;
    case kCCMIOFieldData:
        os << "kCCMIOFieldData"; break;
    case kCCMIOFieldPhase:
        os << "kCCMIOFieldPhase"; break;
    case kCCMIOInternalFaces:
        os << "kCCMIOInternalFaces"; break;
    case kCCMIOMaxEntity:
        os << "kCCMIOMaxEntity"; break;
    case kCCMIOProblemDescription:
        os << "kCCMIOProblemDescription"; break;
    case kCCMIOReferenceData:
        os << "kCCMIOReferenceData"; break;
    case kCCMIOMap:
        os << "kCCMIOMap"; break;
    case kCCMIONull:
        os << "kCCMIONull"; break;
    case kCCMIOTopology:
        os << "kCCMIOTopology"; break;
    case kCCMIOVertices:
        os << "kCCMIOVertices"; break;
    case kCCMIOBoundaryFaces :
        os << "kCCMIOBoundaryFaces "; break;
    case kCCMIOCellType:
        os << "kCCMIOCellType"; break;
    case kCCMIOCells:
        os << "kCCMIOCells"; break;
    case kCCMIOField:
        os << "kCCMIOField"; break;
    case kCCMIOFieldSet :
        os << "kCCMIOFieldSet "; break;
    case kCCMIOInterfaces:
        os << "kCCMIOInterfaces"; break;
    case kCCMIOLagrangianData :
        os << "kCCMIOLagrangianData "; break;
    case kCCMIOModelConstants :
        os << "kCCMIOModelConstants "; break;
    case kCCMIOProcessor :
        os << "kCCMIOProcessor "; break;
    case kCCMIOProstarSet:
        os << "kCCMIOProstarSet"; break;
    case kCCMIORestart :
        os << "kCCMIORestart "; break;
    case kCCMIORestartData:
        os << "kCCMIORestartData"; break;
    case kCCMIOState :
        os << "kCCMIOState "; break;
    }

    return os;
}

// ****************************************************************************
// Method: operator << (ostream &os, const CCMIONode &node)
//
// Purpose: 
//   Prints a CCMIONode
//
// Arguments:
//   os : The stream to which we'll print.
//   e  : The value to print.
//
// Returns:    The input stream.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 15:16:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ostream &
operator << (ostream &os, const CCMIONode &node)
{
    _CCMIONode *n = (_CCMIONode *)&node;
    os << "(node=" << n->node << ", parent=" << n->parent << ")";
    return os;
}

// ****************************************************************************
// Method: operator << (ostream &os, const CCMIOID &e)
//
// Purpose: 
//   Prints a CCMIOID
//
// Arguments:
//   os : The stream to which we'll print.
//   e  : The value to print.
//
// Returns:    The input stream.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 15:16:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ostream &
operator << (ostream &os, const CCMIOID &id)
{
    os << "(root=" << id.root
       << ", node=" << id.node 
       << ", id=" << id.id 
       << ", type=" << id.type 
       << ", version=" << id.version << ")";
    return os;
}




// ****************************************************************************
// Method: avtCCMFileFormat::IDMap::IDMap
//
// Purpose: 
//   Constructor
//
// Programmer: Dave Bremer
// Creation:   Fri Apr  4 16:29:49 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

avtCCMFileFormat::IDMap::IDMap()
{
    bSequential = false;
    bReverseMap = false; 
    iFirstElem = 0;
    numIDs = 0;
}


// ****************************************************************************
// Method: avtCCMFileFormat::IDMap::SetIDs
//
// Purpose: 
//   Analyze a set of IDs and build a structure for mapping them back 
//   to the index corresponding to their position in v.
//
// Programmer: Dave Bremer
// Creation:   Fri Apr  4 16:29:49 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void 
avtCCMFileFormat::IDMap::SetIDs(const intVector &v)
{
    numIDs = v.size();

    bSequential = true;
    bReverseMap = false;
    iFirstElem = v[0];

    int min = v[0], max = v[0];

    int ii;
    for (ii = 1; ii < v.size(); ii++)
    {
        if (v[ii-1]+1 != v[ii])
            bSequential = false;
        if (v[ii] < min)
            min = v[ii];
        if (v[ii] > max)
            max = v[ii];
    }

    //Don't bother copying data in, in this case.
    if (bSequential)
        return;

    if (max-min+1 <= v.size()*2)
    {
        bReverseMap = true;
        iFirstElem = min;

        ids.resize(max-min+1, -1);
        for (ii = 0; ii < v.size(); ii++)
        {
            ids[v[ii]-iFirstElem] = ii;
        }
    }
    else
    {
        bReverseMap = false;

        ids.resize(v.size()*2);
    
        for (ii = 0; ii < v.size(); ii++)
        {
            ids[ii*2]   = v[ii];
            ids[ii*2+1] = ii;
        }
        qsort( &ids[0], v.size(), sizeof(int)*2, avtCCMFileFormat::IDMap::compare);
    }
}


// ****************************************************************************
// Method: avtCCMFileFormat::IDMap::compare
//
// Purpose: 
//   compare function for qsort
//
// Programmer: Dave Bremer
// Creation:   Fri Apr  4 16:29:49 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int 
avtCCMFileFormat::IDMap::compare(const void *a, const void *b)
{
    return ( *((int *)a) - *((int *)b) );
}


// ****************************************************************************
// Method: avtCCMFileFormat::IDMap::IDtoIndex
//
// Purpose: 
//   Map an id to an array index.  Return -1 if the id is not in the map.
//
// Programmer: Dave Bremer
// Creation:   Fri Apr  4 16:29:49 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int  
avtCCMFileFormat::IDMap::IDtoIndex(int id) const
{
    if (bSequential)
    {
        int r = id - iFirstElem;
        if (r >= 0 && r < numIDs)
            return r;
    }
    else if (bReverseMap)
    {
        if (id-iFirstElem >= 0 && id-iFirstElem < ids.size())
            return ids[id-iFirstElem];
    }
    else
    {
        int min = 0, max = ids.size()/2 - 1;
        int mid = (min+max)/2;
    
        while (min <= max)
        {
            if (id < ids[mid*2])
            {
                max = mid-1;
                mid = (min+max)/2;
            }
            else if (id > ids[mid*2])
            {
                min = mid+1;
                mid = (min+max)/2;
            }
            else
            {
                return ids[mid*2+1];
            }
        }
    }
    return -1;
}



