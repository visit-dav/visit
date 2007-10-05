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
//                            avtCCMFileFormat.C                             //
// ************************************************************************* //

#include <avtCCMFileFormat.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
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

using     std::string;



// ****************************************************************************
//  Method: avtCCM constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
// ****************************************************************************

avtCCMFileFormat::avtCCMFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1), varsToFields()
{
    ccmOpened = false;
    ccmStateFound = false;
    ccmErr = kCCMIONoErr;
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
    for (int i = 0; i < originalCells.size(); i++)
    {
        if (originalCells[i] != NULL)
        {
            originalCells[i]->Delete();
            originalCells[i] = NULL;
        }
    }
    originalCells.clear();
    varsToFields.clear();
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
        CCMIOGetState(&ccmErr, GetRoot(), "default", NULL, &ccmState);
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
        // Try and read the vertices, topology, and solution ids for this 
        // processor.
        CCMIOReadProcessor(&ccmErr, processor, &vertices, &topology, NULL, 
                           &solution);
        if(ccmErr != kCCMIONoErr)
        {
            // That didn't work. (Maybe no solution). See if we can at least 
            // get the vertices and processor.
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
//    faceID    The ID of the face entity.
//    faceType  The type of faces (internal or boundary).
//    nFaces    How many faces are in the entity. 
//    fi        A place to store the face info, must be allocated by
//              calling method.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 5, 2007 
//
//  Modifications:
//
// ****************************************************************************

void
avtCCMFileFormat::GetFaces(CCMIOID faceID, CCMIOEntity faceType,
                           unsigned int nFaces, int &minSize, 
                           int &maxSize, CellInfoVector &ci)
{
    if (faceType != kCCMIOInternalFaces && faceType != kCCMIOBoundaryFaces)
    {
        debug1 << "avtCCMFileFormat::GetFaces encountered an internal error"
                << endl;
        return; 
    }
    int getFacesTimer = visitTimer->StartTimer();
    CCMIOID mapID, cellsID;
    unsigned int nCells = 0, size = 0;
    intVector faces, faceNodes, faceCells;

    // Determine the size of the faceNodes array, which is of the
    // form n1, v1, v2, ...vn1, n2, v1, v2, ... vn2, )
    CCMIOReadFaces(&ccmErr, faceID, faceType, NULL, &size, NULL,
                   kCCMIOStart, kCCMIOEnd);
    faceNodes.resize(size);
    faces.resize(nFaces);
    if (faceType == kCCMIOInternalFaces)
        faceCells.resize(nFaces*2);
    else 
        faceCells.resize(nFaces);
    CCMIOReadFaces(&ccmErr, faceID, faceType, &mapID, NULL,
                   &faceNodes[0], kCCMIOStart, kCCMIOEnd);
    CCMIOReadFaceCells(&ccmErr, faceID, faceType, &faceCells[0],
                       kCCMIOStart, kCCMIOEnd);
    CCMIOReadMap(&ccmErr, mapID, &faces[0], kCCMIOStart, kCCMIOEnd);

    unsigned int pos = 0;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        FaceInfo newFace;
        newFace.id = faces[i];
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

        for (unsigned int j = 0; j < nVerts; j++)
        {
            newFace.nodes.push_back(faceNodes[pos+1+j]);
        }
        // cell ids are 1-origin, so must subract 1 to get the
        // correct index into the CellInfoVector
        if (faceType == kCCMIOInternalFaces)
        {
            ci[newFace.cells[0]-1].faceTypes.push_back(1);
            ci[newFace.cells[0]-1].faces.push_back(newFace);
            ci[newFace.cells[1]-1].faceTypes.push_back(2);
            ci[newFace.cells[1]-1].faces.push_back(newFace);
        }
        else 
        {
            ci[newFace.cells[0]-1].faceTypes.push_back(0);
            ci[newFace.cells[0]-1].faces.push_back(newFace);
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
    // Read the simulation title.
    char *title = NULL;
    ccmErr = CCMIOGetTitle(&ccmErr, GetRoot(), &title);
    if(title != NULL)
    {
        md->SetDatabaseComment(title);
        free(title);
    }
#endif

    for (int i = 0; i < nblocks; i++)
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

    // Create a mesh.
    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name = "Mesh";
    mmd->spatialDimension = dims;
    mmd->topologicalDimension = dims;
    mmd->meshType = AVT_UNSTRUCTURED_MESH;
    mmd->numBlocks = nblocks;
    mmd->cellOrigin = 1;
    mmd->nodeOrigin = 1;
    md->Add(mmd);


    // Find variable data
    if (hasSolution)
    {
        CCMIOID field, phase = solution;
        int h = 0, phaseNum = 0, i=0;

        bool oldFile = CCMIONextEntity(NULL, solution, kCCMIOFieldPhase, 
                                       &h, &phase) != kCCMIONoErr;
        h = 0;
        while(oldFile ||
              CCMIONextEntity(NULL, solution, kCCMIOFieldPhase, &h, &phase) ==
                                                                   kCCMIONoErr)
        {
            if (oldFile)
                phase = solution;
            else
            {
                CCMIOGetEntityIndex(NULL, phase, &phaseNum);
            }
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
                int j = 0;

                CCMIOReadField(&ccmErr, field, name, sName, &cdims, &datatype);

                char *usename;
                if (oldFile)
                    usename = name;
                else 
                    usename = sName;

                if (CCMIOReadOptstr(NULL, field, "Units", &usize, NULL) ==
                    kCCMIONoErr)
                {
                    units = new char[usize+1];
                    CCMIOReadOptstr(&ccmErr, field, "Units", NULL, units);
                }
                while (CCMIONextEntity(NULL, field, kCCMIOFieldData, 
                       &j, &fieldData) == kCCMIONoErr)
                {
                    CCMIOReadFieldDataf(&ccmErr, fieldData, &mapID, &type,
                                       NULL, kCCMIOStart, kCCMIOEnd);
                    avtCentering centering = AVT_UNKNOWN_CENT;
                    if (type == kCCMIOVertex)
                    { 
                        centering = AVT_NODECENT;
                    } 
                    else if (type == kCCMIOCell)
                    { 
                        centering = AVT_ZONECENT;
                    } 
                    else if (type == kCCMIOFace)
                    { 
                        debug4 << "Var is face-centered, ignoring for now. " << endl;
                    } 
                    if (centering == AVT_UNKNOWN_CENT)
                        continue;
                    if(cdims==kCCMIOScalar)
                    {
                        avtScalarMetaData *smd = new avtScalarMetaData(usename, 
                            "Mesh", centering);
                        if (units != NULL)
                        {
                            smd->hasUnits = true;
                            smd->units = units;
                        }
                        md->Add(smd);
                        varsToFields[usename] = field;
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
                        md->Add(vmd);
                        varsToFields[usename] = field;
                    }
#if 0
                    else if(cdims==kCCMIOTensor)
                    {
                        avtTensorMetaData *tmd = new avtTensorMetaData(usename, 
                            "Mesh", centering, 9);
                        if (units != NULL)
                        {
                            tmd->hasUnits = true;
                            tmd->units = units;
                        }
                        md->Add(tmd);
                        varsToFields[usename] = field;
                    }
#endif
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
//
// ****************************************************************************

#ifndef MDSERVER
#include <PolygonToTriangles.C>
#endif

vtkDataSet *
avtCCMFileFormat::GetMesh(int dom, const char *meshname)
{
#ifdef MDSERVER
    return 0;
#endif
    vtkUnstructuredGrid *ugrid = NULL;
    CCMIOID processor, vertices, topology, solution;
    bool hasSolution = true;
    unsigned int i, j;

    if(GetIDsForDomain(dom, processor, vertices, topology, solution,
                       hasSolution))
    {
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
        CCMIOID mapID;
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
        if(dims == 2)
        {
            // Read 2D points and convert to 3D, storing into VTK.
            float *pts2d = new float[2 * nnodes];
            CCMIOReadVerticesf(&ccmErr, vertices, &dims, &scale, &mapID, pts2d,
                       0, nnodes);
            float *src = pts2d;
            float *dest = pts;
            for(i = 0; i < nnodes; ++i)
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
        for(i = 0; i < nnodes; ++i)
        {
            pts[0] *= scale;
            pts[1] *= scale;
            pts[2] *= scale;
            pts += 3;
        }

        // Get the topology information
        CCMIOID faceID, cellsID;
        unsigned int nIFaces = 0, nCells = 0, size = 0;
        intVector cells, cellMatType;
        CellInfoVector cellInfo;

        // Read the cells entity
        CCMIOGetEntity(&ccmErr, topology, kCCMIOCells, 0, &cellsID);
        // Read the cells entity size (num cells)
        CCMIOEntitySize(&ccmErr, cellsID, &nCells, NULL);
        cells.resize(nCells);
        cellInfo.resize(nCells);
        cellMatType.resize(nCells);
        // this gets the cell types and the map that stores the cell ids
        CCMIOReadCells(&ccmErr, cellsID, &mapID, &cellMatType[0], 
                       kCCMIOStart, kCCMIOEnd);
        // this reads the cellids from the map.
        CCMIOReadMap(&ccmErr, mapID, &cells[0], kCCMIOStart, kCCMIOEnd);

        int minFaceSize = VTK_LARGE_INTEGER;
        int maxFaceSize = -1;
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
                     minFaceSize, maxFaceSize, cellInfo); 
        }

        // Read the internal faces.
        // Get the InternalFaces entity.
        CCMIOGetEntity(&ccmErr, topology, kCCMIOInternalFaces, 0, &faceID);
        // Get the InternalFaces size (num faces).
        CCMIOEntitySize(&ccmErr, faceID, &nIFaces, NULL);
        
        GetFaces(faceID, kCCMIOInternalFaces, nIFaces, 
                 minFaceSize, maxFaceSize, cellInfo);

        ugrid = vtkUnstructuredGrid::New();

        // Determine cell topology from face lists
        if (minFaceSize <= 4 && maxFaceSize <= 4)
        {
            ugrid->SetPoints(points);
            // We have zoo elements that we can deal with.
            vtkCellArray *cellArray = vtkCellArray::New();
            intVector cellTypes; 
            bool unhandledCellType = false;
            for (i = 0; i < cellInfo.size(); i++)
            {
                CellInfo ci = cellInfo[i]; 
                switch(ci.faces.size())
                {
                    case 4 : 
                        BuildTet(ci, cellArray, cellTypes); 
                        break;
                    case 5 : 
                        {
                        int nNodes = 0;
                        for (j = 0; j < ci.faces.size(); j++)
                        {
                            nNodes += ci.faces[j].nodes.size();
                        }
                        if (nNodes == 16) // Pyramid 
                            BuildPyramid(ci, cellArray, cellTypes);
                        else if (nNodes == 18) // Wedge
                            BuildWedge(ci, cellArray, cellTypes);
                        else
                            unhandledCellType = true; 
                        break;
                        }
                    case 6 : 
                        BuildHex(ci, cellArray, cellTypes); 
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
            TesselateCell(dom, cellInfo, points, ugrid); 
        }
        points->Delete();
    }
    return ugrid;
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
// ****************************************************************************

vtkDataArray *
avtCCMFileFormat::GetVar(int domain, const char *varname)
{
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
        for (unsigned int i = 0 ; i < nvalues ; i++)
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

        unsigned int *oc = ocarray->GetPointer(0);
        rv->SetNumberOfValues(numCells);
        for (unsigned int i = 0 ; i < numCells ; i++)
        {
            rv->SetValue(i, data[oc[i*2+1]]);
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
// ****************************************************************************

vtkDataArray *
avtCCMFileFormat::GetVectorVar(int domain, const char *varname)
{
    VarFieldMap::const_iterator pos = varsToFields.find(varname);
    if (pos == varsToFields.end())
        EXCEPTION1(InvalidVariableException, varname);

    intVector mapData;
    floatVector u, v, w, data;
    CCMIOID scalar;
    CCMIOID field = pos->second;
    CCMIOError err;
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
        for (unsigned int i = 0 ; i < numCells; i++)
        {
            unsigned int id = oc[i*2+1];
            v[i*3+0] = data[id*3+0];
            v[i*3+1] = data[id*3+1];
            v[i*3+2] = data[id*3+2];
        }
    }
    return rv; 
}


void
avtCCMFileFormat::ReadScalar(CCMIOID field, intVector &mapData, 
                             floatVector &data, bool readingVector)

{
    CCMIOSize n;
    CCMIOIndex fmax;
    int j = 0;
    CCMIOID fieldData, mapID;
    CCMIODataLocation type;

    // Read each piece of field data
#if 0
    while (CCMIONextEntity(NULL, field, kCCMIOFieldData, &j, &fieldData)
                                                               == kCCMIONoErr)
#endif
    if (CCMIONextEntity(NULL, field, kCCMIOFieldData, &j, &fieldData)
                                                               == kCCMIONoErr)
    {
        // Figure out how big this data is so we can read it. If we were
        // storing this information permanently we might use a sparse
        // array, in which case we would need to find the maximum ID and
        // make the array that size.
        CCMIOEntitySize(&ccmErr, fieldData, &n, &fmax);
        mapData.resize(n);
        CCMIOReadFieldDataf(&ccmErr, fieldData, &mapID, &type, NULL,
                            kCCMIOStart, kCCMIOEnd);
        CCMIOReadMap(&ccmErr, mapID, &mapData[0], kCCMIOStart, kCCMIOEnd);

        // We are only going to process cell data.  Vertex data would
        // be processed similarly. If your appliation has only one value
        // for boundary data, you would separate the face data into
        // each boundary and combine it together.  If your application
        // stores boundary data on each face you could read it in using
        // similar procedures for the cell and vertex data.  Note that
        // the file may not contain all types of data.
        if (type == kCCMIOCell)
        {
            if (readingVector)
                data.resize(3 * n);
            else
                data.resize(n);
            // If we want double precision we should use
            // CCMIOReadFieldDatad().
            CCMIOReadFieldDataf(&ccmErr, fieldData, &mapID, NULL,
                                &data[0], kCCMIOStart, kCCMIOEnd);
        }
        else if (type == kCCMIOVertex)
        {
            if (readingVector)
                data.resize(3 * n);
            else
                data.resize(n);
            // If we want double precision we should use
            // CCMIOReadFieldDatad().
            CCMIOReadFieldDataf(&ccmErr, fieldData, &mapID, NULL,
                                &data[0], kCCMIOStart, kCCMIOEnd);
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
// ****************************************************************************

void
avtCCMFileFormat::TesselateCell(const int dom, const CellInfoVector &civ, 
                                vtkPoints *points, vtkUnstructuredGrid *ugrid)
{
    unsigned int i, j, k;
    CellInfo  ci;
    vtkPoints *pts = vtkPoints::New();
    pts->Allocate(points->GetNumberOfPoints());
    VertexManager uniqueVerts(pts);
    PolygonToTriangles tess(&uniqueVerts);
    unsigned int oc[2] = {dom, 0};

    originalCells[dom] = vtkUnsignedIntArray::New();
    originalCells[dom]->SetName("avtOriginalCellNumbers");
    originalCells[dom]->SetNumberOfComponents(2);
    originalCells[dom]->Allocate(civ.size()*3);

    for (i = 0; i < civ.size(); i++)
    {
        ci = civ[i];
        int nFaces  = ci.faces.size();
        int nPts = 0;
        double fbounds[6*nFaces];
        for (j = 0; j < nFaces; j++)
        {
            nPts += ci.faces[j].nodes.size();
            fbounds[j*6+0] = VTK_LARGE_FLOAT;
            fbounds[j*6+1] = -VTK_LARGE_FLOAT;
            fbounds[j*6+2] = VTK_LARGE_FLOAT;
            fbounds[j*6+3] = -VTK_LARGE_FLOAT;
            fbounds[j*6+4] = VTK_LARGE_FLOAT;
            fbounds[j*6+5] = -VTK_LARGE_FLOAT;
        }
        intVector nodes;
        double *pt;
        double cbounds[6] = {VTK_LARGE_FLOAT, -VTK_LARGE_FLOAT, 
                             VTK_LARGE_FLOAT, -VTK_LARGE_FLOAT, 
                             VTK_LARGE_FLOAT, -VTK_LARGE_FLOAT};

        int cnt = 0;
        for (j = 0; j < nFaces; j++)
        {
            nodes = ci.faces[j].nodes;
                
            for (k = 0; k < nodes.size(); k++)
            {
                cnt++;
                pt = points->GetPoint(nodes[k]-1);

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
            
        double cc[3];
        double fc[3];
        for (j = 0; j < 3; j++)
            cc[j] = (cbounds[2*j+1]+cbounds[2*j])/2.0; 
        for (j = 0; j < nFaces; j++)
        {
            for (k = 0; k < 3; k++)
                fc[k] = (fbounds[2*k+1+(6*j)]+fbounds[2*k+(6*j)])/2.0; 

            nodes = ci.faces[j].nodes;
            double n[3] = {(cc[0] - fc[0]), (cc[1] - fc[1]), (cc[2] - fc[2])};
            tess.SetNormal(n);
            tess.BeginPolygon();
            tess.BeginContour();
                
            for (k = 0; k < nodes.size(); k++)
            {
                cnt++;
                pt = points->GetPoint(nodes[k]-1);
                tess.AddVertex(pt);
            } // k nodes
            tess.EndContour();
            tess.EndPolygon();
        //}  // j faces
        int centerId = uniqueVerts.GetVertexId(cc);
        vtkIdType verts[4];
        verts[3] = centerId;
        if (tess.GetNumTriangles() > 0)
        {
            for (k = 0; k < tess.GetNumTriangles(); k++)
            {
                int a, b, c;
                tess.GetTriangle(k, a, b, c);
                verts[0] = a; 
                verts[1] = b; 
                verts[2] = c; 
                ugrid->InsertNextCell(VTK_TETRA, 4, verts);
                oc[1] = i; // looping on cells, i = current cell
                ((vtkUnsignedIntArray*)originalCells[dom])->
                    InsertNextTupleValue(oc);
            }
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
//   
// ****************************************************************************

void
avtCCMFileFormat::BuildHex(const CellInfo &ci, vtkCellArray *cellArray, 
                           intVector &cellTypes)
{
    unsigned int i, j, k;
    FaceInfoVector faces = ci.faces;
    intVector uniqueNodes; 
    bool useface;
    bool usedBF = false;
    int nnodes = 0;
    vtkIdList *cellNodes = vtkIdList::New();
      
    for (i = 0; i < faces.size(); i++)
    {
        useface = true;
        int nnodes = faces[i].nodes.size(); 
        if (nnodes != 4)
        {
            return; 
        }
        for (j = 0; useface && j <nnodes; j++)
        {
            useface = (count(uniqueNodes.begin(), uniqueNodes.end(), 
                             faces[i].nodes[j]-1) == 0);
        }
        if (useface)
        {
            if ((ci.faceTypes[i] == 0 && !usedBF) || (ci.faceTypes[i] == 2))
            {
                usedBF = (ci.faceTypes[i] == 0);
                // reorder this face
                for (j = 0; j < nnodes; j++)
                {
                    uniqueNodes.push_back(faces[i].nodes[j]-1);
                }
                cellNodes->InsertNextId(faces[i].nodes[0]-1);
                cellNodes->InsertNextId(faces[i].nodes[3]-1);
                cellNodes->InsertNextId(faces[i].nodes[2]-1);
                cellNodes->InsertNextId(faces[i].nodes[1]-1);
            }
            else
            {
                for (j = 0; j < nnodes; j++)
                {
                    uniqueNodes.push_back(faces[i].nodes[j]-1);
                    cellNodes->InsertNextId(faces[i].nodes[j]-1);
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
//   
// ****************************************************************************

void
avtCCMFileFormat::BuildTet(const CellInfo &ci, vtkCellArray *cellArray, 
                           intVector &cellTypes)
{
    unsigned int i, j;
    FaceInfoVector faces = ci.faces;
    intVector uniqueNodes; 
    bool lastNodeFound = false;
    vtkIdList *cellNodes = vtkIdList::New();

    for (i = 0; i < faces.size(); i++)
    {
        if (faces[i].nodes.size() != 3)
        {
           return;
        }
    }
  
    if (ci.faceTypes[0] != 1)
    { 
        uniqueNodes.push_back(faces[0].nodes[0]-1);  
        uniqueNodes.push_back(faces[0].nodes[2]-1);  
        uniqueNodes.push_back(faces[0].nodes[1]-1);  
        cellNodes->InsertNextId(faces[0].nodes[0]-1);  
        cellNodes->InsertNextId(faces[0].nodes[2]-1);  
        cellNodes->InsertNextId(faces[0].nodes[1]-1);  
    } 
    else
    { 
        uniqueNodes.push_back(faces[0].nodes[0]-1);  
        uniqueNodes.push_back(faces[0].nodes[1]-1);  
        uniqueNodes.push_back(faces[0].nodes[2]-1);  
        cellNodes->InsertNextId(faces[0].nodes[0]-1);  
        cellNodes->InsertNextId(faces[0].nodes[1]-1);  
        cellNodes->InsertNextId(faces[0].nodes[2]-1);  
    } 
      
    for (i = 1; !lastNodeFound && i < faces.size(); i++)
    {
        for (j = 0; !lastNodeFound && j <faces[i].nodes.size(); j++)
        {
           if ((count(uniqueNodes.begin(), uniqueNodes.end(), 
                      faces[i].nodes[j]-1) == 0)) 
           {
               lastNodeFound == true;
               uniqueNodes.push_back(faces[i].nodes[j]-1);
               cellNodes->InsertNextId(faces[i].nodes[j]-1);  
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
// ****************************************************************************

void
avtCCMFileFormat::BuildPyramid(const CellInfo &ci, vtkCellArray *cellArray, 
                               intVector &cellTypes)
{
    unsigned int i, j;
    FaceInfoVector faces = ci.faces;
    intVector uniqueNodes; 
    int baseFace = 0;
    vtkIdList *cellNodes = vtkIdList::New();

    for (i = 0; i < faces.size(); i++)
    {
        if (faces[i].nodes.size() == 4)
        {
            baseFace = i;
            for (j = 0; j < 4; j++)
            {
                uniqueNodes.push_back(faces[i].nodes[j]-1);
                cellNodes->InsertNextId(faces[i].nodes[j]-1);
            }
        }
    }
    int triFace = (baseFace+1) % 4;
    for (i = 0; i < 3; i++) // only 3 nodes in the triangle-face
    {
        if ((count(uniqueNodes.begin(), uniqueNodes.end(), 
             faces[triFace].nodes[i]-1)) == 0)
        {
            cellNodes->InsertNextId(faces[triFace].nodes[i]-1);
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
//   
// ****************************************************************************

void
avtCCMFileFormat::BuildWedge(const CellInfo &ci, vtkCellArray *cellArray, 
                             intVector &cellTypes)
{
    unsigned int i, j;
    FaceInfoVector faces = ci.faces;
    vtkIdList *cellNodes = vtkIdList::New();
    for (i = 0; i < faces.size(); i++)
    {
        if (faces[i].nodes.size() == 3)
        {
            cellNodes->InsertNextId(faces[i].nodes[0]-1);
            cellNodes->InsertNextId(faces[i].nodes[1]-1);
            cellNodes->InsertNextId(faces[i].nodes[2]-1);
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
// ****************************************************************************

avtCCMFileFormat::FaceInfo::FaceInfo()
{
    id = -1;
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
// ****************************************************************************

avtCCMFileFormat::FaceInfo::FaceInfo(const avtCCMFileFormat::FaceInfo &obj)
{
    id = obj.id;
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
// ****************************************************************************

void
avtCCMFileFormat::FaceInfo::operator =(const avtCCMFileFormat::FaceInfo &obj)
{
    id = obj.id;
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


