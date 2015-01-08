/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <set>
#include <sstream>
#include <snprintf.h>
using std::ostringstream;

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

#include <vtkCSGGrid.h>

#include <VisItDataInterface_V2.h>
#include <VisItInterfaceTypes_V2.h>

#include <avtGhostData.h>
#include <avtPolygonToTrianglesTesselator.h>
#include <avtPolyhedralSplit.h>

#include <DebugStream.h>
#include <ImproperUseException.h>



#include <simv2_CSGMesh.h>
#include <simv2_CurvilinearMesh.h>
#include <simv2_PointMesh.h>
#include <simv2_RectilinearMesh.h>
#include <simv2_UnstructuredMesh.h>
#include <simv2_VariableData.h>
#include <simv2_TypeTraits.hxx>
#include <simv2_DeleteEventObserver.h>

static const char *AVT_GHOST_ZONES_ARRAY = "avtGhostZones";
static const char *AVT_GHOST_NODES_ARRAY = "avtGhostNodes";

// ****************************************************************************
//  Function:  GetQuadGhostZones
//
//  Purpose:  Add ghost zone information to a dataset.
//
//  Note:  stolen from the Silo file format method of the same name
//
// ****************************************************************************
static void
GetQuadGhostZones(int nnodes, int ndims,
                  int *dims, int *min_index, int *max_index,
                  vtkDataSet *ds)
{
    //
    //  Determine if we have ghost points
    //
    int first[3];
    int last[3];
    bool ghostPresent = false;
    bool badIndex = false;
    for (int i = 0; i < 3; i++)
    {
        first[i] = (i < ndims ? min_index[i] : 0);
        last[i]  = (i < ndims ? max_index[i] : 0);

        if (first[i] < 0 || first[i] >= dims[i])
        {
            debug1 << "bad Index on first[" << i << "] dims is: "
                   << dims[i] << endl;
            badIndex = true;
        }

        if (last[i] < 0 || last[i] >= dims[i])
        {
            debug1 << "bad Index on last[" << i << "] dims is: "
                   << dims[i] << endl;
            badIndex = true;
        }

        if (first[i] != 0 || last[i] != dims[i] -1)
        {
            ghostPresent = true;
        }
    }

    //
    //  Create the ghost zones array if necessary
    //
    if (ghostPresent && !badIndex)
    {
        bool *ghostPoints = new bool[nnodes];
        //
        // Initialize as all ghost levels
        //
        for (int ii = 0; ii < nnodes; ii++)
            ghostPoints[ii] = true;

        //
        // Set real values
        //
        for (int k = first[2]; k <= last[2]; k++)
            for (int j = first[1]; j <= last[1]; j++)
                for (int i = first[0]; i <= last[0]; i++)
                {
                    int index = k*dims[1]*dims[0] + j*dims[0] + i;
                    ghostPoints[index] = false;
                }

        //
        //  okay, now we have ghost points, but what we really want
        //  are ghost cells ... convert:  if all points associated with
        //  cell are 'real' then so is the cell.
        //
        unsigned char realVal = 0;
        unsigned char ghostVal = 0;
        avtGhostData::AddGhostZoneType(ghostVal,
                                       DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
        int ncells = ds->GetNumberOfCells();
        vtkIdList *ptIds = vtkIdList::New();
        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName(AVT_GHOST_ZONES_ARRAY);
        ghostCells->Allocate(ncells);

        for (int i = 0; i < ncells; i++)
        {
            ds->GetCellPoints(i, ptIds);
            bool ghost = false;
            for (int idx = 0; idx < ptIds->GetNumberOfIds(); idx++)
                ghost |= ghostPoints[ptIds->GetId(idx)];

            if (ghost)
                ghostCells->InsertNextValue(ghostVal);
            else
                ghostCells->InsertNextValue(realVal);

        }
        ds->GetCellData()->AddArray(ghostCells);
        delete [] ghostPoints;
        ghostCells->Delete();
        ptIds->Delete();

        vtkIntArray *realDims = vtkIntArray::New();
        realDims->SetName("avtRealDims");
        realDims->SetNumberOfValues(6);
        realDims->SetValue(0, first[0]);
        realDims->SetValue(1, last[0]);
        realDims->SetValue(2, first[1]);
        realDims->SetValue(3, last[1]);
        realDims->SetValue(4, first[2]);
        realDims->SetValue(5, last[2]);
        ds->GetFieldData()->AddArray(realDims);
        ds->GetFieldData()->CopyFieldOn("avtRealDims");
        realDims->Delete();

        ds->GetInformation()->Set(
            vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);
    }
}

// ****************************************************************************
// Function: AddGhostZonesFromArray
//
// Purpose:
//   Converts a mesh-sized array of ghost cell values into an avtGhostZones
//   array that we add to the dataset.
//
// Arguments:
//   ds         : The dataset to which we're adding ghost cells.
//   ghostCells : The source array that contains the ghost cell types.
//
// Returns:    True if the avtGhostZones array was added; False otherwise.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 11 14:15:15 PDT 2011
//
// Modifications:
//   Brad Whitlock, Thu Sep 27 17:24:15 PDT 2012
//   Add ghost zones such that they can have multiple designations.
//
// ****************************************************************************

static bool
AddGhostZonesFromArray(vtkDataSet *ds, visit_handle ghostCells)
{
    bool retval = false;

    // Get the ghost cell information
    int owner, dataType, nComps, nTuples = 0;
    void *data = 0;
    if(simv2_VariableData_getData(ghostCells, owner, dataType, nComps, nTuples, data))
    {
        vtkUnsignedCharArray *ghosts = vtkUnsignedCharArray::New();
        ghosts->SetNumberOfTuples(nTuples);
        ghosts->SetName(AVT_GHOST_ZONES_ARRAY);
        unsigned char *dest = (unsigned char *)ghosts->GetVoidPointer(0);
        if(dataType == VISIT_DATATYPE_CHAR)
        {
            const unsigned char *src = (const unsigned char *)data;
            const unsigned char *end = src + nTuples;
            for( ; src < end; src++)
            {
                unsigned char gz = 0;

                if(*src & VISIT_GHOSTCELL_INTERIOR_BOUNDARY)
                    avtGhostData::AddGhostZoneType(gz, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
                if(*src & VISIT_GHOSTCELL_EXTERIOR_BOUNDARY)
                    avtGhostData::AddGhostZoneType(gz, ZONE_EXTERIOR_TO_PROBLEM);
                if(*src & VISIT_GHOSTCELL_ENHANCED_CONNECTIVITY)
                    avtGhostData::AddGhostZoneType(gz, ENHANCED_CONNECTIVITY_ZONE);
                if(*src & VISIT_GHOSTCELL_REDUCED_CONNECTIVITY)
                    avtGhostData::AddGhostZoneType(gz, REDUCED_CONNECTIVITY_ZONE);
                if(*src & VISIT_GHOSTCELL_BLANK)
                    avtGhostData::AddGhostZoneType(gz, ZONE_NOT_APPLICABLE_TO_PROBLEM);
                if(*src & VISIT_GHOSTCELL_REFINED_AMR_CELL)
                    avtGhostData::AddGhostZoneType(gz, REFINED_ZONE_IN_AMR_GRID);

                *dest++ = gz;
            }

            ds->GetCellData()->AddArray(ghosts);
            retval = true;
        }
        else if(dataType == VISIT_DATATYPE_INT)
        {
            const int *src = (const int *)data;
            const int *end = src + nTuples;
            for( ; src < end; src++)
            {
                unsigned char gz = 0;

                if(*src & VISIT_GHOSTCELL_INTERIOR_BOUNDARY)
                    avtGhostData::AddGhostZoneType(gz, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
                if(*src & VISIT_GHOSTCELL_EXTERIOR_BOUNDARY)
                    avtGhostData::AddGhostZoneType(gz, ZONE_EXTERIOR_TO_PROBLEM);
                if(*src & VISIT_GHOSTCELL_ENHANCED_CONNECTIVITY)
                    avtGhostData::AddGhostZoneType(gz, ENHANCED_CONNECTIVITY_ZONE);
                if(*src & VISIT_GHOSTCELL_REDUCED_CONNECTIVITY)
                    avtGhostData::AddGhostZoneType(gz, REDUCED_CONNECTIVITY_ZONE);
                if(*src & VISIT_GHOSTCELL_BLANK)
                    avtGhostData::AddGhostZoneType(gz, ZONE_NOT_APPLICABLE_TO_PROBLEM);
                if(*src & VISIT_GHOSTCELL_REFINED_AMR_CELL)
                    avtGhostData::AddGhostZoneType(gz, REFINED_ZONE_IN_AMR_GRID);

                *dest++ = gz;
            }

            ds->GetCellData()->AddArray(ghosts);
            retval = true;
        }
        else
        {
            ghosts->Delete();
        }
    }

    return retval;
}

// ****************************************************************************
// Function: AddGhostNodesFromArray
//
// Purpose:
//   Converts a mesh-sized array of ghost node values into an avtGhostNodes
//   array that we add to the dataset.
//
// Arguments:
//   ds         : The dataset to which we're adding ghost nodes.
//   ghostNodes : The source array that contains the ghost node types.
//
// Returns:    True if the avtGhostNodes array was added; False otherwise.
//
// Note:
//
// Programmer: William T. Jones
// Creation:   Fri Jan 20 17:45:02 EDT 2012
//
// Modifications:
//
// ****************************************************************************

static bool
AddGhostNodesFromArray(vtkDataSet *ds, visit_handle ghostNodes)
{
    bool retval = false;

    // Get the ghost node information
    int owner, dataType, nComps, nTuples = 0;
    void *data = 0;
    if(simv2_VariableData_getData(ghostNodes, owner, dataType, nComps, nTuples, data))
    {
        unsigned char gnTypes[5] = {0,0,0,0,0};
        avtGhostData::AddGhostNodeType(gnTypes[VISIT_GHOSTNODE_INTERIOR_BOUNDARY],     DUPLICATED_NODE);
        avtGhostData::AddGhostNodeType(gnTypes[VISIT_GHOSTNODE_BLANK],                 NODE_NOT_APPLICABLE_TO_PROBLEM);
        avtGhostData::AddGhostNodeType(gnTypes[VISIT_GHOSTNODE_COARSE_SIDE],           NODE_IS_ON_COARSE_SIDE_OF_COARSE_FINE_BOUNDARY);
        avtGhostData::AddGhostNodeType(gnTypes[VISIT_GHOSTNODE_FINE_SIDE],             NODE_IS_ON_FINE_SIDE_OF_COARSE_FINE_BOUNDARY);

        vtkUnsignedCharArray *ghosts = vtkUnsignedCharArray::New();
        ghosts->SetNumberOfTuples(nTuples);
        ghosts->SetName(AVT_GHOST_NODES_ARRAY);
        unsigned char *dest = (unsigned char *)ghosts->GetVoidPointer(0);
        if(dataType == VISIT_DATATYPE_CHAR)
        {
            const unsigned char *src = (const unsigned char *)data;
            const unsigned char *end = src + nTuples;
            while(src < end)
            {
                if(*src <= VISIT_GHOSTNODE_FINE_SIDE)
                {
                    *dest++ = gnTypes[*src];
                }
                else
                {
                    ghosts->Delete();
                    EXCEPTION1(ImproperUseException, "Invalid ghost node value");
                }

                src++;
            }

            ds->GetPointData()->AddArray(ghosts);
            retval = true;
        }
        else if(dataType == VISIT_DATATYPE_INT)
        {
            const int *src = (const int *)data;
            const int *end = src + nTuples;
            while(src < end)
            {
                if(*src >= 0 && *src <= VISIT_GHOSTNODE_FINE_SIDE)
                {
                    *dest++ = gnTypes[*src];
                }
                else
                {
                    ghosts->Delete();
                    EXCEPTION1(ImproperUseException, "Invalid ghost node value");
                }

                src++;
            }

            ds->GetPointData()->AddArray(ghosts);
            retval = true;
        }
        else
        {
            ghosts->Delete();
        }
    }

    return retval;
}

// ****************************************************************************
// Function: SimV2_CreatePoints
//
// Purpose:
//   Create a vtkPoints object from various VariableData objects.
//
// Arguments:
//   dims             : The number of dimensions
//   coordMode        : The coordinate mode: separate or interleaved.
//   x                : The handle to the x coordinate array.
//   y                : The handle to the y coordinate array.
//   z                : The handle to the z coordinate array.
//   c                : The handle to the c coordinate array.
//   additionalPoints : The number of additional points to create.
//
// Returns:    A vtkPoints object that contains the coordinates.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 25 11:14:01 PST 2010
//
// Modifications:
//   Brad Whitlock, Mon Nov 22 16:46:15 PST 2010
//   Fix cut & paste error that caused z coordinates to be zero for doubles.
//
//   Burlen Loring, Fri Feb 21 13:44:36 PST 2014
//   * fix bug where VTK was told to delete the data when he really should
//     have saved it
//   * enable zero-copy
//   * template refactor
//   * make error handling consistent(throw exceptions) because callers don't
//     check for null return
//
//    Kathleen Biagas, Mon May 19 11:38:05 PDT 2014
//    When calling SetArray, include nComps in the size argument.
//
// ****************************************************************************

static vtkPoints *
SimV2_CreatePoints(int ndims, int coordMode,
    visit_handle x, visit_handle y, visit_handle z, visit_handle c,
    int additionalPoints)
{
    vtkPoints *points = NULL;

    if(coordMode == VISIT_COORD_MODE_SEPARATE)
    {
        // get array's and properties, note: properties are guaranteed to
        // be the same for all
        visit_handle cHandles[3] = {x, y, z};
        void *data[3] = {0,0,0};
        int owner, dataType, nComps, nTuples;
        for(int i = 0; i < ndims; ++i)
        {
            if(simv2_VariableData_getData(cHandles[i], owner, dataType,
                nComps, nTuples, data[i]) == VISIT_ERROR)
            {
                char coordName[3] = {'x', 'y', 'z'};
                ostringstream oss;
                oss << "Failed to get data for " << coordName[i];
                EXCEPTION1(ImproperUseException, oss.str().c_str());
            }
        }

        // validate the data type.
        if (!simV2_ValidFloatDataType(dataType))
        {
            EXCEPTION1(ImproperUseException,
                "Coordinate arrays must be float or double.\n");
        }

        // Interleave the coordinate arrays as VTK requires.
        points = vtkPoints::New();
        if(ndims == 2)
        {
            switch (dataType)
            {
            simV2FloatTemplateMacro(
                points->SetDataType(simV2_TT::vtkEnum);
                points->SetNumberOfPoints(nTuples+additionalPoints);
                simV2_TT::cppType *pPts = static_cast<simV2_TT::cppType*>(points->GetVoidPointer(0));

                simV2_TT::cppType *pX = static_cast<simV2_TT::cppType*>(data[0]);
                simV2_TT::cppType *pY = static_cast<simV2_TT::cppType*>(data[1]);

                for(int i = 0; i < nTuples; ++i,pPts+=3)
                {
                    pPts[0] = pX[i];
                    pPts[1] = pY[i];
                    pPts[2] = simV2_TT::cppType();
                }
                );
            }
        }
        else
        {
            switch (dataType)
            {
            simV2FloatTemplateMacro(
                points->SetDataType(simV2_TT::vtkEnum);
                points->SetNumberOfPoints(nTuples+additionalPoints);
                simV2_TT::cppType *pPts = static_cast<simV2_TT::cppType*>(points->GetVoidPointer(0));

                simV2_TT::cppType *pX = static_cast<simV2_TT::cppType*>(data[0]);
                simV2_TT::cppType *pY = static_cast<simV2_TT::cppType*>(data[1]);
                simV2_TT::cppType *pZ = static_cast<simV2_TT::cppType*>(data[2]);

                for(int i = 0; i < nTuples; ++i,pPts+=3)
                {
                    pPts[0] = pX[i];
                    pPts[1] = pY[i];
                    pPts[2] = pZ[i];
                }
                );
            }
        }
    }
    else if(coordMode == VISIT_COORD_MODE_INTERLEAVED)
    {
        int owner, dataType, nComps, nTuples;
        void *data = NULL;
        void (*callback)(void*) = NULL;
        void *callbackData = NULL;

        int ierr = simv2_VariableData_getDataEx(c, owner, dataType, nComps,
                                     nTuples, data, callback, callbackData);

        if (ierr == VISIT_ERROR)
        {
            EXCEPTION1(ImproperUseException,
                "Failed to get data for interleaved coordinates");
            return NULL;
        }

        // validate the data type.
        if (!simV2_ValidFloatDataType(dataType))
        {
            EXCEPTION1(ImproperUseException,
                "Coordinate array must be float or double.\n");
        }

        points = vtkPoints::New();
        if(ndims == 2)
        {
            // make it 3d and copy
            switch (dataType)
            {
            simV2FloatTemplateMacro(
                points->SetDataType(simV2_TT::vtkEnum);
                points->SetNumberOfPoints(nTuples+additionalPoints);
                simV2_TT::cppType *pPts = static_cast<simV2_TT::cppType*>(points->GetVoidPointer(0));
                simV2_TT::cppType *pData = static_cast<simV2_TT::cppType*>(data);

                for(int i = 0; i < nTuples; ++i, pPts+=3, pData+=2)
                {
                    pPts[0] = pData[0];
                    pPts[1] = pData[1];
                    pPts[2] = simV2_TT::cppType();
                }
                );
            }
        }
        else if ( (additionalPoints > 0) || (owner == VISIT_OWNER_COPY) )
        {
            // copy into vtk data array
            switch (dataType)
            {
            simV2FloatTemplateMacro(
                points->SetDataType(simV2_TT::vtkEnum);
                points->SetNumberOfPoints(nTuples+additionalPoints);
                memcpy(points->GetVoidPointer(0), data, 3*nTuples*sizeof(simV2_TT::cppType));
                );
            }
        }
        else
        {
            // zero-copy, VTK uses the pointer rather than making
            // a copy. this is inherently risky, especially for VISIT_OWNER_SIM
            // with VISIT_OWNER_VISIT_EX the sim is expected to hold a
            // reference until his callback is invoked by the vtk array delete
            // event observer
            switch (dataType)
            {
            simV2FloatTemplateMacro(
                simV2_TT::vtkType *pts = simV2_TT::vtkType::New();
                pts->SetNumberOfComponents(3);

                if (owner == VISIT_OWNER_VISIT_EX)
                {
                    // we observe VTK data array's DeleteEvent and invoke the
                    // user provided callback in repsonse. it's the callbacks
                    // duty to free the memory.
                    pts->SetArray(static_cast<simV2_TT::cppType*>(data), 
                                  nTuples*3, 1);

                    simV2_DeleteEventObserver *observer = 
                        simV2_DeleteEventObserver::New();
                    observer->Observe(pts, callback, callbackData);
                    // this is not a leak, the observer is Delete'd after it's
                    // invoked.
                }
                else
                {
                    // VTK assumes ownership for VISIT_OWNER_VISIT. For
                    // VISIT_OWNER_SIM the sim must ensure that data persists
                    // while VTK is using it
                    pts->SetArray(static_cast<simV2_TT::cppType*>(data),
                         nTuples*3, ((owner==VISIT_OWNER_VISIT)?0:1));

                }
                points->SetData(pts);
                pts->Delete();
                );
            }
            // give up our ownership, VTK will free the data
            simv2_VariableData_nullData(c);
        }
    }

    return points;
}

// ****************************************************************************
// Method: SimV2_GetMesh_Curvilinear
//
// Purpose:
//   Builds a VTK curvilinear mesh from the simulation mesh.
//
// Arguments:
//   cmesh : The simulation mesh.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 24 16:41:39 PST 2010
//
// Modifications:
//   Brad Whitlock, Thu Aug 11 13:14:59 PDT 2011
//   I added support for ghost zones from an array.
//
//   William T. Jones, Fri Jan 20 17:55:27 EDT 2012
//   I added support for ghost nodes from an array.
//
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_Curvilinear(visit_handle h)
{
    if (h == VISIT_INVALID_HANDLE)
        return NULL;

    // Obtain the mesh data from the opaque object.
    int ndims = 0, dims[3]={0,0,0}, coordMode = 0;
    int minRealIndex[3]={0,0,0}, maxRealIndex[3]={0,0,0}, baseIndex[3]={0,0,0};
    visit_handle x,y,z,c;
    if(simv2_CurvilinearMesh_getCoords(h, &ndims, dims, &coordMode,
                                       &x,&y,&z,&c) == VISIT_ERROR ||
       simv2_CurvilinearMesh_getRealIndices(h, minRealIndex,
                                       maxRealIndex) == VISIT_ERROR ||
       simv2_CurvilinearMesh_getBaseIndex(h, baseIndex) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
            "Could not obtain mesh data using the provided handle.\n");
        return NULL;
    }

    // Create the points.
    vtkPoints *points = SimV2_CreatePoints(ndims, coordMode, x, y, z, c, 0);

    // Create the VTK objects and connect them up.
    vtkStructuredGrid *sgrid   = vtkStructuredGrid::New();
    sgrid->SetPoints(points);
    points->Delete();

    // Tell the grid what its dimensions are and populate the points array.
    sgrid->SetDimensions(dims);

    bool doQuadGhosts = true;
    visit_handle ghostCells;
    if(simv2_CurvilinearMesh_getGhostCells(h, &ghostCells) == VISIT_OKAY)
    {
        if(ghostCells != VISIT_INVALID_HANDLE)
            doQuadGhosts = !AddGhostZonesFromArray(sgrid, ghostCells);
    }
    if(doQuadGhosts)
    {
        GetQuadGhostZones(dims[0]*dims[1]*dims[2], ndims,
                 dims, minRealIndex, maxRealIndex, sgrid);
    }

    visit_handle ghostNodes;
    if(simv2_CurvilinearMesh_getGhostNodes(h, &ghostNodes) == VISIT_OKAY)
    {
        if(ghostNodes != VISIT_INVALID_HANDLE)
            AddGhostNodesFromArray(sgrid, ghostNodes);
    }

    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(3);
    arr->SetValue(0, baseIndex[0]);
    arr->SetValue(1, baseIndex[1]);
    arr->SetValue(2, baseIndex[2]);
    arr->SetName("base_index");
    sgrid->GetFieldData()->AddArray(arr);
    arr->Delete();

    return sgrid;
}

// ****************************************************************************
// Method: SimV2_GetMesh_Rectilinear
//
// Purpose:
//   Builds a VTK rectilinear mesh from the simulation mesh.
//
// Arguments:
//  rmesh : The simulation mesh.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 25 11:57:18 PST 2010
//
// Modifications:
//   Brad Whitlock, Fri Jan 14 01:49:00 PST 2011
//   Make all components the same type in 2D or bad things happen.
//
//   Brad Whitlock, Thu Aug 11 13:17:33 PDT 2011
//   Add support for ghost cells from an array.
//
//   William T. Jones, Fri Jan 20 17:55:27 EDT 2012
//   I added support for ghost nodes from an array.
//
//   Burlen Loring, Fri Feb 21 13:44:36 PST 2014
//   * template refactor
//
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_Rectilinear(visit_handle h)
{
    if (h == VISIT_INVALID_HANDLE)
        return NULL;

    // Obtain the mesh data from the opaque object.
    int ndims = 0;
    int minRealIndex[3]={0,0,0}, maxRealIndex[3]={0,0,0}, baseIndex[3]={0,0,0};
    visit_handle x,y,z;
    if(simv2_RectilinearMesh_getCoords(h, &ndims, &x, &y, &z) == VISIT_ERROR ||
       simv2_RectilinearMesh_getRealIndices(h, minRealIndex, maxRealIndex) == VISIT_ERROR ||
       simv2_RectilinearMesh_getBaseIndex(h, baseIndex) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
            "Could not obtain mesh data using the provided handle.\n");
    }

    // Obtain the coordinate data from the opaque objects.
    visit_handle cHandles[3] = {x, y, z};
    int owner[3]={0,0,0}, dataType[3]={0,0,0}, nComps[3]={1,1,1}, nTuples[3] = {0,0,1};
    void *data[3] = {0,0,0};
    for(int i = 0; i < ndims; ++i)
    {
        if(simv2_VariableData_getData(cHandles[i], owner[i],
            dataType[i], nComps[i], nTuples[i], data[i]) == VISIT_ERROR)
        {
            EXCEPTION1(ImproperUseException,
                "Could not obtain mesh data using the provided handle.\n");
        }

        // validate the data type.
        if (!simV2_ValidFloatDataType(dataType[i]))
        {
            EXCEPTION1(ImproperUseException,
                "Coordinate arrays must be float or double.\n");
        }
    }

    if (ndims == 2)
        dataType[2] = dataType[0];

    // Create the VTK objects and connect them up.
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(nTuples);

    // Populate the coordinates.
    vtkDataArray *coords[3] = {0,0,0};
    for(int i = 0; i < 3; ++i)
    {
        switch (dataType[i])
        {
        simV2FloatTemplateMacro(
            coords[i] = simV2_TT::vtkType::New();
            if ((ndims == 2) && (i == 2))
            {
                // make 3d for VTK
                coords[i]->SetNumberOfTuples(1);
                coords[i]->SetComponent(0, 0, 0);
            }
            else
            {
                coords[i]->SetNumberOfTuples(nTuples[i]);
                memcpy(coords[i]->GetVoidPointer(0), data[i],
                        nTuples[i]*sizeof(simV2_TT::cppType));
            }
            );
        }
    }

    rgrid->SetXCoordinates(coords[0]);
    rgrid->SetYCoordinates(coords[1]);
    rgrid->SetZCoordinates(coords[2]);
    
    for (int i=0; i<3; ++i) 
        coords[i]->Delete();

    bool doQuadGhosts = true;
    visit_handle ghostCells;
    if(simv2_RectilinearMesh_getGhostCells(h, &ghostCells) == VISIT_OKAY)
    {
        if(ghostCells != VISIT_INVALID_HANDLE)
            doQuadGhosts = !AddGhostZonesFromArray(rgrid, ghostCells);
    }
    if(doQuadGhosts)
    {
        GetQuadGhostZones(nTuples[0]*nTuples[1]*nTuples[2],
                          ndims,
                          nTuples,
                          minRealIndex,
                          maxRealIndex,
                          rgrid);
    }

    visit_handle ghostNodes;
    if(simv2_RectilinearMesh_getGhostNodes(h, &ghostNodes) == VISIT_OKAY)
    {
        if(ghostNodes != VISIT_INVALID_HANDLE)
            AddGhostNodesFromArray(rgrid, ghostNodes);
    }

    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(3);
    arr->SetValue(0, baseIndex[0]);
    arr->SetValue(1, baseIndex[1]);
    arr->SetValue(2, baseIndex[2]);
    arr->SetName("base_index");
    rgrid->GetFieldData()->AddArray(arr);
    arr->Delete();

    return rgrid;
}

// ****************************************************************************
// Method: SimV2_UnstructuredMesh_Count_Cells
//
// Purpose:
//   Examine the connectivity and count the number of regular cells and the
//   number of polyhedral cells.
//
// Arguments:
//   umesh           : The unstructured mesh
//   normalCellCount : The count of non-polyhedral cells.
//   polyCount       : The number of polyhedral cells
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 18 16:57:18 PST 2010
//
// Modifications:
//
// ****************************************************************************

static void
SimV2_UnstructuredMesh_Count_Cells(const int *connectivity, int connectivityLen,
    int &normalCellCount, int &polyCount)
{
    int celltype_npts[10];
    celltype_npts[VISIT_CELL_BEAM]  = 2;
    celltype_npts[VISIT_CELL_TRI]   = 3;
    celltype_npts[VISIT_CELL_QUAD]  = 4;
    celltype_npts[VISIT_CELL_TET]   = 4;
    celltype_npts[VISIT_CELL_PYR]   = 5;
    celltype_npts[VISIT_CELL_WEDGE] = 6;
    celltype_npts[VISIT_CELL_HEX]   = 8;

    polyCount = 0;
    normalCellCount = 0;

    const int *cell = connectivity;
    const int *end = connectivity + connectivityLen;
    while(cell < end)
    {
        int celltype = *cell++;
        if(celltype == VISIT_CELL_POLYHEDRON)
        {
            int nfaces = *cell++;
            for(int i = 0; i < nfaces; ++i)
            {
                int npts = *cell++;
                cell += npts;
            }
            polyCount++;
        }
        else if(celltype >= VISIT_CELL_BEAM && celltype <= VISIT_CELL_POLYHEDRON)
        {
            cell += celltype_npts[celltype];
            normalCellCount++;
        }
        else
        {
            EXCEPTION1(ImproperUseException,
                       "An invalid cell type was provided in the unstructured "
                       "mesh connectivity.\n");
        }
    }
}

// ****************************************************************************
// Method: SimV2_Add_PolyhedralCell
//
// Purpose:
//   Append the current polyhedral cell to the mesh as tets and pyramids while
//   updating the input cellptr to the location of the next polyhedral face.
//
// Arguments:
//   ugrid   : The unstructured grid to which we're adding a cell.
//   cellptr : The pointer to the face connectivity.
//   nnodes  : The number of original nodes.
//   phIndex : The index of the polyhedral zone.
//
// Returns:    Return the number of cells that the polyhedral cell was broken
//             into.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 18 16:59:12 PST 2010
//
// Modifications:
//
//   Cyrus Harrison, Tue Oct  9 13:21:32 PDT 2012
//   Use tess2 via new avtPolygonToTrianglesTesselator class.
//
//   Kathleen Biagas, Thu Jun 26 14:14:37 MST 2014
//   Send the face normal to the tesselator.
//
// ****************************************************************************

static int
SimV2_Add_PolyhedralCell(vtkUnstructuredGrid *ugrid, const int **cellptr,
    int nnodes, int phIndex, avtPolyhedralSplit *polyhedralSplit)
{
    const int *cell = *cellptr;

    // cell points at the number of faces in the cell at this point.
    int nfaces = *cell++;
    // Iterate over the faces and get a list of unique points
    std::set<int> uniquePointIds;
    for(int i = 0; i < nfaces; ++i)
    {
        int nPointsInFace = *cell++;
        for(int j = 0; j < nPointsInFace; ++j)
            uniquePointIds.insert(*cell++);
    }

    // Come up with a center point and store it.
    double pt[3] = {0.,0.,0.}, center[3] = {0.,0.,0.};
    vtkPoints *points = ugrid->GetPoints();
    polyhedralSplit->AppendPolyhedralNode((int)uniquePointIds.size());
    for(std::set<int>::const_iterator it = uniquePointIds.begin();
        it != uniquePointIds.end(); ++it)
    {
        polyhedralSplit->AppendPolyhedralNode(*it);
        points->GetPoint(*it, pt);
        center[0] += pt[0];
        center[1] += pt[1];
        center[2] += pt[2];
    }
    double m = 1. / double(uniquePointIds.size());
    center[0] *= m;
    center[1] *= m;
    center[2] *= m;
    vtkIdType phCenter = nnodes + phIndex;
    points->SetPoint(phCenter, center);

    // Now, iterate over the faces, adding solid cells for them
    int splitCount = 0;
    cell = *cellptr + 1;
    vtkIdType verts[5];
    for(int i = 0; i < nfaces; ++i)
    {
        int nPointsInFace = *cell++;
        if(nPointsInFace == 3)
        {
            // Add a tet
            verts[0] = cell[2];
            verts[1] = cell[1];
            verts[2] = cell[0];
            verts[3] = phCenter;
            ugrid->InsertNextCell(VTK_TETRA, 4, verts);
            splitCount++;
        }
        else if(nPointsInFace == 4)
        {
            // Add a pyramid
            verts[0] = cell[3];
            verts[1] = cell[2];
            verts[2] = cell[1];
            verts[3] = cell[0];
            verts[4] = phCenter;
            ugrid->InsertNextCell(VTK_PYRAMID, 5, verts);
            splitCount++;
        }
        else if(nPointsInFace > 4)
        {
            // Find the face center so we can determine a proxy for a normal.
            double fc[3] = {0., 0., 0.};
            for(int j = 0; j < nPointsInFace; ++j)
            {
                points->GetPoint(cell[j], pt);
                fc[0] += pt[0];
                fc[1] += pt[1];
                fc[2] += pt[2];
            }
            fc[0] /= double(nPointsInFace);
            fc[1] /= double(nPointsInFace);
            fc[2] /= double(nPointsInFace);

            double n[3] = {0.,0.,0.};
            n[0] = center[0] - fc[0];
            n[1] = center[1] - fc[1];
            n[2] = center[2] - fc[2];

            // Tesselate the shape into triangles and add tets. We create
            // a tessellator each time so we can add the face's points to
            // it. This should cause the points to be in the same order as
            // they are in the face.

            vtkPoints *localPts = vtkPoints::New();
            localPts->Allocate(nPointsInFace);
            int *local2Global = new int[nPointsInFace];
            avtPolygonToTrianglesTesselator tess(localPts);
            tess.SetNormal(n);
            tess.BeginContour();
            for(int j = 0; j < nPointsInFace; ++j)
            {
                local2Global[j] = cell[j];
                double *pptr = points->GetPoint(local2Global[j]);
                tess.AddContourVertex(pptr);
            }
            tess.EndContour();

            int ntris = tess.Tessellate();
            for(int t = 0; t < ntris; ++t)
            {
                int a=0,b=0,c=0;
                tess.GetTriangleIndices(t, a, b, c);
                verts[0] = local2Global[a];
                verts[1] = local2Global[b];
                verts[2] = local2Global[c];
                verts[3] = phCenter;
                ugrid->InsertNextCell(VTK_TETRA, 4, verts);
                splitCount++;
            }

            localPts->Delete();
            delete [] local2Global;
        }

        cell += nPointsInFace;
    }

    *cellptr = cell;

    return splitCount;
}

// ****************************************************************************
// Method: SimV2_GetMesh_Unstructured
//
// Purpose:
//   Builds a VTK unstructured mesh from the simulation mesh.
//
// Arguments:
//   umesh   : The simulation mesh.
//   h       : The handle that references the real mesh data.
//   phSplit : Return a polyhedral split object if we had polyhedra.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 25 14:01:25 PST 2010
//
// Modifications:
//   Brad Whitlock, Wed Sep  1 09:44:34 PDT 2010
//   I fixed an off by 1 that was pointed out.
//
//   Brad Whitlock, Tue Oct 26 16:26:39 PDT 2010
//   I changed the interface to avtPolyhedralSplit.
//
//   Brad Whitlock, Thu Aug 11 13:54:49 PDT 2011
//   I added support for ghost zones from an array.
//
//   William T. Jones, Fri Jan 20 17:55:27 EDT 2012
//   I added support for ghost nodes from an array.
//
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_Unstructured(int domain, visit_handle h, avtPolyhedralSplit **phSplit)
{
    if (h == VISIT_INVALID_HANDLE)
        return NULL;

    //
    // Obtain the mesh data from the opaque object.
    //
    int ndims = 0, nzones = 0, coordMode = 0, firstRealZone=0, lastRealZone=0;
    visit_handle x,y,z,c,conn;
    if(simv2_UnstructuredMesh_getCoords(h, &ndims, &coordMode, &x, &y, &z, &c) == VISIT_ERROR ||
       simv2_UnstructuredMesh_getConnectivity(h, &nzones, &conn) == VISIT_ERROR ||
       simv2_UnstructuredMesh_getRealIndices(h, &firstRealZone, &lastRealZone) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain mesh data using the provided handle.\n");
    }

    //
    // Get the connectivity
    //
    int connOwner = 0, connDataType=0, connNComps=0, connectivityLen=0;
    void *connData = 0;
    if(simv2_VariableData_getData(conn, connOwner,
       connDataType, connNComps, connectivityLen, connData) == VISIT_ERROR)
    {
         EXCEPTION1(ImproperUseException,
             "Could not obtain connectivity data using the provided handle.\n");
    }
    const int *connectivity = (const int *)connData;

    // Count the polyhedral cells so we can allocate more points
    int normalCellCount = 0, polyhedralCellCount = 0;
    SimV2_UnstructuredMesh_Count_Cells(connectivity, connectivityLen, normalCellCount,
        polyhedralCellCount);

    //
    // Create the points.
    //
    vtkPoints *points = SimV2_CreatePoints(ndims, coordMode, x, y, z, c,
                                           polyhedralCellCount);
    int nRealPoints = points->GetNumberOfPoints() - polyhedralCellCount;

    //
    // Create the cells.
    //
    int celltype_npts[10];
    celltype_npts[VISIT_CELL_BEAM]  = 2;
    celltype_npts[VISIT_CELL_TRI]   = 3;
    celltype_npts[VISIT_CELL_QUAD]  = 4;
    celltype_npts[VISIT_CELL_TET]   = 4;
    celltype_npts[VISIT_CELL_PYR]   = 5;
    celltype_npts[VISIT_CELL_WEDGE] = 6;
    celltype_npts[VISIT_CELL_HEX]   = 8;

    int celltype_idtype[10];
    celltype_idtype[VISIT_CELL_BEAM]  = VTK_LINE;
    celltype_idtype[VISIT_CELL_TRI]   = VTK_TRIANGLE;
    celltype_idtype[VISIT_CELL_QUAD]  = VTK_QUAD;
    celltype_idtype[VISIT_CELL_TET]   = VTK_TETRA;
    celltype_idtype[VISIT_CELL_PYR]   = VTK_PYRAMID;
    celltype_idtype[VISIT_CELL_WEDGE] = VTK_WEDGE;
    celltype_idtype[VISIT_CELL_HEX]   = VTK_HEXAHEDRON;

    vtkUnstructuredGrid  *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    ugrid->Allocate(normalCellCount + 6 * polyhedralCellCount);
    points->Delete();

    *phSplit = 0;
    avtPolyhedralSplit *polyhedralSplit = 0;
    if(polyhedralCellCount > 0)
    {
        polyhedralSplit = new avtPolyhedralSplit;
    }

    // Iterate over the connectivity and add the appropriate cell types
    int numCells = 0;
    int phIndex = 0;
    const int *cell = connectivity;
    const int *end = cell + connectivityLen;
    vtkIdType verts[8];
    bool noConnectivityError = true;
    while(cell < end && numCells < nzones && noConnectivityError)
    {
        int celltype = *cell++;

        if(celltype == VISIT_CELL_POLYHEDRON)
        {
            // Add a polyhedral cell as a collection of smaller normal cells.
            int nsplits = SimV2_Add_PolyhedralCell(ugrid, &cell, nRealPoints,
                phIndex, polyhedralSplit);
            polyhedralSplit->AppendCellSplits(numCells, nsplits);
            phIndex++;
        }
        else if(celltype >= VISIT_CELL_BEAM && celltype <= VISIT_CELL_HEX)
        {
            // Add a normal cell
            int vtktype = celltype_idtype[celltype];
            int nelempts = celltype_npts[celltype];
            for (int j=0; j<nelempts; j++)
            {
                verts[j] = *cell++;
                noConnectivityError &= (verts[j] >= 0 && verts[j] < nRealPoints);
            }
            ugrid->InsertNextCell(vtktype, nelempts, verts);
        }
        else
        {
            // bad cell type
            noConnectivityError = false;
        }

        ++numCells;
    }

    if (!noConnectivityError)
    {
        delete polyhedralSplit;
        ugrid->Delete();
        char tmp[100];
        SNPRINTF(tmp, 100, "Cell %d's connectivity contained invalid points or "
            "an invalid cell type.", numCells-1);
        EXCEPTION1(ImproperUseException, tmp);
    }
    if (numCells != nzones)
    {
        delete polyhedralSplit;
        ugrid->Delete();
        EXCEPTION1(ImproperUseException,
                   "Number of zones and length of connectivity "
                   "array did not match!");
    }

    //
    // Add the ghost zones to the mesh.
    //
    if (firstRealZone == 0 && lastRealZone == 0 )
    {
        debug5 << "Cannot tell if ghost zones are present because "
               << "min_index & max_index are both zero!" << endl;
    }
    else if (firstRealZone < 0 || firstRealZone >= numCells ||
             lastRealZone  < 0 || lastRealZone  >= numCells ||
             firstRealZone > lastRealZone)
    {
        // bad min or max index
        debug5 << "Invalid min/max index for determining ghost zones:  "
               << "\n\tnumCells: " << numCells
               << "\n\tfirstRealZone: " << firstRealZone
               << "\n\tlastRealZone: " << lastRealZone << endl;
    }
    else
    {
        bool doGhosts = true;
        visit_handle ghostCells;
        if(simv2_UnstructuredMesh_getGhostCells(h, &ghostCells) == VISIT_OKAY)
        {
            if(ghostCells != VISIT_INVALID_HANDLE)
            {
                bool addedGhosts = AddGhostZonesFromArray(ugrid, ghostCells);

                // Split the ghost zones array if we split the cells.
                vtkDataArray *ghostZones = ugrid->GetCellData()->GetArray(AVT_GHOST_ZONES_ARRAY);
                if(ghostZones != 0 && polyhedralCellCount > 0)
                {
                    ugrid->GetCellData()->RemoveArray(AVT_GHOST_ZONES_ARRAY);

                    vtkDataArray *phgz = polyhedralSplit->ExpandDataArray(ghostZones, true);
                    ghostZones->Delete();
                    ugrid->GetCellData()->AddArray(phgz);
                }

                doGhosts = !addedGhosts;
            }
        }

        if (doGhosts && (firstRealZone != 0 || lastRealZone != numCells -1))
        {
            vtkUnsignedCharArray *ghostZones = vtkUnsignedCharArray::New();
            ghostZones->SetNumberOfTuples(numCells);
            unsigned char *gvals = ghostZones->GetPointer(0);
            unsigned char val = 0;
            avtGhostData::AddGhostZoneType(val,
                DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
            for (int i = 0; i < firstRealZone; i++)
                gvals[i] = val;
            for (int i = firstRealZone; i <= lastRealZone; i++)
                gvals[i] = 0;
            for (int i = lastRealZone+1; i < numCells; i++)
                gvals[i] = val;

            if(polyhedralCellCount > 0)
            {
                vtkDataArray *phgz = polyhedralSplit->ExpandDataArray(ghostZones, true);
                ghostZones->Delete();
                ghostZones = (vtkUnsignedCharArray *)phgz;
            }

            ghostZones->SetName(AVT_GHOST_ZONES_ARRAY);
            ugrid->GetCellData()->AddArray(ghostZones);
            ghostZones->Delete();
            ugrid->GetInformation()->Set(
                vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);
        }
    }

    if(polyhedralCellCount > 0)
    {
        vtkDataArray *originalCells = polyhedralSplit->CreateOriginalCells(domain,
            normalCellCount);
        ugrid->GetCellData()->AddArray(originalCells);
        ugrid->GetCellData()->CopyFieldOn("avtOriginalCellNumbers");

        // Return the polyhedral split object.
        *phSplit = polyhedralSplit;
    }

    visit_handle ghostNodes;
    if(simv2_UnstructuredMesh_getGhostNodes(h, &ghostNodes) == VISIT_OKAY)
    {
        if(ghostNodes != VISIT_INVALID_HANDLE)
        {
            AddGhostNodesFromArray(ugrid, ghostNodes);
        }
    }

    return ugrid;
}

// ****************************************************************************
// Method: SimV2_GetMesh_Point
//
// Purpose:
//   Builds a VTK point mesh from the simulation mesh.
//
// Arguments:
//   pmesh : The simulation mesh.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 25 14:08:14 PST 2010
//
// Modifications:
//
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_Point(visit_handle h)
{
    if (h == VISIT_INVALID_HANDLE)
        return NULL;

    //
    // Obtain the mesh data from the opaque object.
    //
    int ndims = 0, coordMode = 0;
    visit_handle x,y,z,c;
    if(simv2_PointMesh_getCoords(h, &ndims, &coordMode, &x, &y, &z, &c) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain mesh data using the provided handle.\n");
    }

    //
    // Create the points.
    //
    vtkPoints *points = SimV2_CreatePoints(ndims, coordMode, x, y, z, c, 0);

    //
    // Add point cells
    //
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    points->Delete();
    int npts = points->GetNumberOfPoints();
    ugrid->Allocate(points->GetNumberOfPoints());
    vtkIdType onevertex[1];
    for (int i=0; i<npts; i++)
    {
        onevertex[0] = i;
        ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
    }

    return ugrid;
}

// ****************************************************************************
// Method: SimV2_GetMesh_CSG
//
// Purpose:
//    Builds a VTK csg mesh from the simulation mesh.
//
// Arguments:
//   csgm : The simulation mesh.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:       Adapted from the Silo reader.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 25 14:32:06 PST 2010
//
// Modifications:
//
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_CSG(visit_handle h)
{
    if(h == VISIT_INVALID_HANDLE)
        return NULL;

    //
    // Obtain the mesh data from the opaque object.
    //
    visit_handle typeflags, leftids, rightids, zonelist, bndtypes, bndcoeffs;
    double min_extents[3]={0.,0.,0.}, max_extents[3]={0.,0.,0.};
    if(simv2_CSGMesh_getRegions(h, &typeflags, &leftids, &rightids) == VISIT_ERROR ||
       simv2_CSGMesh_getZonelist(h, &zonelist) == VISIT_ERROR ||
       simv2_CSGMesh_getBoundaryTypes(h, &bndtypes) == VISIT_ERROR ||
       simv2_CSGMesh_getBoundaryCoeffs(h, &bndcoeffs) == VISIT_ERROR ||
       simv2_CSGMesh_getExtents(h, min_extents, max_extents) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain mesh data using the provided handle.\n");
    }

    //
    // Get the data from the opaque arrays.
    //
    int typeflags_owner=0, typeflags_dataType=0, typeflags_nComps=0, typeflags_nTuples=0;
    void *typeflags_data = 0;
    if(simv2_VariableData_getData(typeflags, typeflags_owner, typeflags_dataType,
       typeflags_nComps, typeflags_nTuples, typeflags_data) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain typeflags for CSG mesh.\n");
    }

    int leftids_owner=0, leftids_dataType=0, leftids_nComps=0, leftids_nTuples=0;
    void *leftids_data = 0;
    if(simv2_VariableData_getData(leftids, leftids_owner, leftids_dataType,
       leftids_nComps, leftids_nTuples, leftids_data) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain leftids for CSG mesh.\n");
    }

    int rightids_owner=0, rightids_dataType=0, rightids_nComps=0, rightids_nTuples=0;
    void *rightids_data = 0;
    if(simv2_VariableData_getData(rightids, rightids_owner, rightids_dataType,
       rightids_nComps, rightids_nTuples, rightids_data) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain rightids for CSG mesh.\n");
    }

    int zonelist_owner=0, zonelist_dataType=0, zonelist_nComps=0, zonelist_nTuples=0;
    void *zonelist_data = 0;
    if(simv2_VariableData_getData(zonelist, zonelist_owner, zonelist_dataType,
       zonelist_nComps, zonelist_nTuples, zonelist_data) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain zonelist for CSG mesh.\n");
    }

    int bndtypes_owner=0, bndtypes_dataType=0, bndtypes_nComps=0, bndtypes_nTuples=0;
    void *bndtypes_data = 0;
    if(simv2_VariableData_getData(bndtypes, bndtypes_owner, bndtypes_dataType,
       bndtypes_nComps, bndtypes_nTuples, bndtypes_data) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain bndtypes for CSG mesh.\n");
    }

    int bndcoeffs_owner=0, bndcoeffs_dataType=0, bndcoeffs_nComps=0, bndcoeffs_nTuples=0;
    void *bndcoeffs_data = 0;
    if(simv2_VariableData_getData(bndcoeffs, bndcoeffs_owner, bndcoeffs_dataType,
       bndcoeffs_nComps, bndcoeffs_nTuples, bndcoeffs_data) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain bndcoeffs for CSG mesh.\n");
    }

    //
    // Create the VTK objects and connect them up.
    //
    vtkCSGGrid *csggrid = vtkCSGGrid::New();

    double minX = -10.0, minY = -10.0, minZ = -10.0;
    double maxX =  10.0, maxY =  10.0, maxZ =  10.0;
    // set bounds *before* anything else
    if (!((min_extents[0] == 0.0 && max_extents[0] == 0.0 &&
           min_extents[1] == 0.0 && max_extents[1] == 0.0 &&
           min_extents[2] == 0.0 && max_extents[2] == 0.0) ||
          (min_extents[0] == -DBL_MAX && max_extents[0] == DBL_MAX &&
           min_extents[1] == -DBL_MAX && max_extents[1] == DBL_MAX &&
           min_extents[2] == -DBL_MAX && max_extents[2] == DBL_MAX)))
    {
        minX = min_extents[0];
        maxX = max_extents[0];
        minY = min_extents[1];
        maxY = max_extents[1];
        minZ = min_extents[2];
        maxZ = max_extents[2];
    }
    csggrid->SetBounds(minX, maxX, minY, maxY, minZ, maxZ);

    if (bndcoeffs_dataType == VISIT_DATATYPE_DOUBLE)
        csggrid->AddBoundaries(bndtypes_nTuples, (const int *)bndtypes_data,
                               bndcoeffs_nTuples, (const double*)bndcoeffs_data);
    else
        csggrid->AddBoundaries(bndtypes_nTuples, (const int *)bndtypes_data,
                               bndcoeffs_nTuples, (const float*)bndcoeffs_data);

    csggrid->AddRegions(leftids_nTuples,
        (const int *)leftids_data, (const int *)rightids_data,
        (const int *)typeflags_data, 0, 0);

    csggrid->AddZones(zonelist_nTuples, (const int *)zonelist_data);

    return csggrid;
}
