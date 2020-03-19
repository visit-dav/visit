// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <set>
#include <sstream>
using std::ostringstream;

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkSOADataArrayTemplate.h>
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

#include "SimV2GetMesh.h"
#include "vtkComponentDataArray.h"

//
// Work around a problem with vtkDataWriter that prevents it from correctly writing
// our vtkComponentDataArray-wrapped strided arrays. The vtkDataWriter contains
// code that casts data arrays to concrete VTK-only types (bad assumption) so it 
// can get direct access to contiguous array data (again, bad assumption). 
//
#define VTK_DATA_WRITER_WORKAROUND

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

int
SimV2_GetVTKType(int simv2type)
{
    int ret = VTK_FLOAT;
    switch(simv2type)
    {
    case VISIT_DATATYPE_CHAR:
        ret = VTK_CHAR;
        break;
    case VISIT_DATATYPE_INT:
        ret = VTK_INT;
        break;
    case VISIT_DATATYPE_LONG:
        ret = VTK_LONG;
        break;
    case VISIT_DATATYPE_FLOAT:
        ret = VTK_FLOAT;
        break;
    case VISIT_DATATYPE_DOUBLE:
        ret = VTK_DOUBLE;
        break;
    }

    return ret;
}

static void InitializeCellNumPoints(int *celltype_npts)
{
    celltype_npts[VISIT_CELL_BEAM]  = 2;
    celltype_npts[VISIT_CELL_TRI]   = 3;
    celltype_npts[VISIT_CELL_QUAD]  = 4;
    celltype_npts[VISIT_CELL_TET]   = 4;
    celltype_npts[VISIT_CELL_PYR]   = 5;
    celltype_npts[VISIT_CELL_WEDGE] = 6;
    celltype_npts[VISIT_CELL_HEX]   = 8;
    celltype_npts[VISIT_CELL_POINT] = 1;

    celltype_npts[VISIT_CELL_POLYHEDRON] = 0; // handled specially

    celltype_npts[VISIT_CELL_QUADRATIC_EDGE] = 3;
    celltype_npts[VISIT_CELL_QUADRATIC_TRI]  = 6;
    celltype_npts[VISIT_CELL_QUADRATIC_QUAD] = 8;
    celltype_npts[VISIT_CELL_QUADRATIC_TET] = 10;
    celltype_npts[VISIT_CELL_QUADRATIC_HEX] = 20;
    celltype_npts[VISIT_CELL_QUADRATIC_WEDGE] = 15;
    celltype_npts[VISIT_CELL_QUADRATIC_PYR] = 13;
    celltype_npts[VISIT_CELL_BIQUADRATIC_TRI] = 7;
    celltype_npts[VISIT_CELL_BIQUADRATIC_QUAD] = 9;
    celltype_npts[VISIT_CELL_TRIQUADRATIC_HEX] = 27;
    celltype_npts[VISIT_CELL_QUADRATIC_LINEAR_QUAD] = 6;
    celltype_npts[VISIT_CELL_QUADRATIC_LINEAR_WEDGE] = 12;
    celltype_npts[VISIT_CELL_BIQUADRATIC_QUADRATIC_WEDGE] = 18;
    celltype_npts[VISIT_CELL_BIQUADRATIC_QUADRATIC_HEX] = 24;
}

static bool
ValidCellType(int celltype)
{
    return (celltype >= VISIT_CELL_BEAM && celltype <= VISIT_CELL_POLYHEDRON) ||
           (celltype >= VISIT_CELL_QUADRATIC_EDGE && celltype <= VISIT_CELL_BIQUADRATIC_QUADRATIC_HEX);
}

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
            debug1 << "bad Index on first[" << i << "]=" << first[i]
                   << " dims is: " << dims[i] << endl;
            badIndex = true;
        }

        if (last[i] < 0 || last[i] >= dims[i])
        {
            debug1 << "bad Index on last[" << i << "]=" << last[i]
                   << " dims is: " << dims[i] << endl;
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
//   Brad Whitlock, Mon Jun 29 13:40:08 PDT 2015
//   Added strided access.
//
// ****************************************************************************

template <typename Scalar>
inline unsigned char
GhostZoneValue(Scalar value)
{
    unsigned char gz = 0;

    if(value & VISIT_GHOSTCELL_INTERIOR_BOUNDARY)
        avtGhostData::AddGhostZoneType(gz, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
    if(value & VISIT_GHOSTCELL_EXTERIOR_BOUNDARY)
        avtGhostData::AddGhostZoneType(gz, ZONE_EXTERIOR_TO_PROBLEM);
    if(value & VISIT_GHOSTCELL_ENHANCED_CONNECTIVITY)
        avtGhostData::AddGhostZoneType(gz, ENHANCED_CONNECTIVITY_ZONE);
    if(value & VISIT_GHOSTCELL_REDUCED_CONNECTIVITY)
        avtGhostData::AddGhostZoneType(gz, REDUCED_CONNECTIVITY_ZONE);
    if(value & VISIT_GHOSTCELL_BLANK)
        avtGhostData::AddGhostZoneType(gz, ZONE_NOT_APPLICABLE_TO_PROBLEM);
    if(value & VISIT_GHOSTCELL_REFINED_AMR_CELL)
        avtGhostData::AddGhostZoneType(gz, REFINED_ZONE_IN_AMR_GRID);

    return gz;
}

static bool
AddGhostZonesFromArray(vtkDataSet *ds, visit_handle ghostCells)
{
    bool retval = false;

    // Get the ghost cell information
    int owner, dataType, nComps, nTuples = 0, memory, offset, stride;
    void *data = 0;
    if(simv2_VariableData_getArrayData(ghostCells, 0, memory, owner, dataType, 
        nComps, nTuples, offset, stride, data))
    {
        vtkUnsignedCharArray *ghosts = vtkUnsignedCharArray::New();
        ghosts->SetNumberOfTuples(nTuples);
        ghosts->SetName(AVT_GHOST_ZONES_ARRAY);
        unsigned char *dest = (unsigned char *)ghosts->GetVoidPointer(0);
        if(memory == VISIT_MEMORY_CONTIGUOUS)
        {
            if(dataType == VISIT_DATATYPE_CHAR)
            {
                const unsigned char *src = static_cast<const unsigned char *>(data);
                const unsigned char *end = src + nTuples;
                for( ; src < end; src++)
                {
                    *dest++ = GhostZoneValue(*src);
                }

                ds->GetCellData()->AddArray(ghosts);
                retval = true;
            }
            else if(dataType == VISIT_DATATYPE_INT)
            {
                const int *src = static_cast<const int *>(data);
                const int *end = src + nTuples;
                for( ; src < end; src++)
                {
                    *dest++ = GhostZoneValue(*src);
                }

                ds->GetCellData()->AddArray(ghosts);
                retval = true;
            }
        }
        else
        {
            // Use the VTK type to iterate over the data to convert to int.
            int vtkType = SimV2_GetVTKType(dataType);
            vtkComponentDataArray<int> *src = vtkComponentDataArray<int>::New();
            src->SetNumberOfTuples(nTuples);
            src->SetComponentData(0, vtkArrayComponentStride(data, offset, stride, vtkType, false));
            for(vtkIdType i = 0; i < nTuples; ++i)
            {
                int value = static_cast<int>(src->GetTuple1(i));
                *dest++ = GhostZoneValue(value);
            }
            src->Delete();

            ds->GetCellData()->AddArray(ghosts);
            retval = true;
        }
        ghosts->Delete();
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
//   Brad Whitlock, Mon Jun 29 13:40:08 PDT 2015
//   Added strided access.
//
// ****************************************************************************

static bool
AddGhostNodesFromArray(vtkDataSet *ds, visit_handle ghostNodes)
{
    bool retval = false;

    // Get the ghost cell information
    int owner, dataType, nComps, nTuples = 0, memory, offset, stride;
    void *data = 0;
    if(simv2_VariableData_getArrayData(ghostNodes, 0, memory, owner, dataType, 
        nComps, nTuples, offset, stride, data))
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

        if(memory == VISIT_MEMORY_CONTIGUOUS)
        {
            if(dataType == VISIT_DATATYPE_CHAR)
            {
                const unsigned char *src = (const unsigned char *)data;
                const unsigned char *end = src + nTuples;
                for( ; src < end; src++)
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
                }

                ds->GetPointData()->AddArray(ghosts);
                retval = true;
            }
            else if(dataType == VISIT_DATATYPE_INT)
            {
                const int *src = (const int *)data;
                const int *end = src + nTuples;
                for( ; src < end; src++)
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
                }

                ds->GetPointData()->AddArray(ghosts);
                retval = true;
            }
        }
        else
        {
            // Use the VTK type to iterate over the data to convert to int.
            int vtkType = SimV2_GetVTKType(dataType);
            vtkComponentDataArray<int> *src = vtkComponentDataArray<int>::New();
            src->SetNumberOfTuples(nTuples);
            src->SetComponentData(0, vtkArrayComponentStride(data, offset, stride, vtkType, false));
            for(vtkIdType i = 0; i < nTuples; ++i)
            {
                int value = static_cast<int>(src->GetTuple1(i));
                if(value >= 0 && value <= VISIT_GHOSTNODE_FINE_SIDE)
                {
                    *dest++ = gnTypes[value];
                }
                else
                {
                    ghosts->Delete();
                    EXCEPTION1(ImproperUseException, "Invalid ghost node value");
                }
            }
            src->Delete();

            ds->GetPointData()->AddArray(ghosts);
            retval = true;
        }
        ghosts->Delete();
    }

    return retval;
}

// ****************************************************************************
// Method: SimV2_MultiArray_Compatible_With_VTK_SOA
//
// Purpose:
//   Checks whether a Libsim multi-component array has components that are all
//   the same type with the right strides. If so, it would be compatible with
//   VTK's newer SOA data array type.
//
// Arguments:
//   dims             : The number of dimensions
//   c                : The handle to the variable data object.
//
// Returns:    True if the data are compatible with VTK SOA. False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 18 14:17:56 PDT 2018
//
// Modifications:
//
// ****************************************************************************

bool
SimV2_MultiArray_Compatible_With_VTK_SOA(int ndims, visit_handle c, int &dt)
{
    // Check whether the array components are contiguous and of the same type.
    bool compatible = true;
    int dataType0, stride0;
    for(int i = 0; i < ndims; ++i)
    {
        // Get the information from the i'th data array component.
        void *data = NULL;
        int owner, dataType, nComps, nTuples, memory, offset, stride;
        if(simv2_VariableData_getArrayData(c, i, memory, owner, dataType,
           nComps, nTuples, offset, stride, data) == VISIT_ERROR)
        {
            compatible = false;
        }
        else
        {
            if(i == 0)
            {
                dt = dataType0 = dataType;
                stride0 = stride;

                if(dataType == VISIT_DATATYPE_CHAR)
                    compatible = stride == sizeof(char);
                else if(dataType == VISIT_DATATYPE_INT)
                    compatible = stride == sizeof(int);
                else if(dataType == VISIT_DATATYPE_LONG)
                    compatible = stride == sizeof(long);
                else if(dataType == VISIT_DATATYPE_FLOAT)
                    compatible = stride == sizeof(float);
                else if(dataType == VISIT_DATATYPE_DOUBLE)
                    compatible = stride == sizeof(double);
            }
            else
            {
                if(dataType0 != dataType || stride0 != stride)
                    compatible = false;
            }
        }
    }

    return compatible;
}

// ****************************************************************************
// Method: SimV2_MultiArray_To_VTK_SOA
//
// Purpose:
//   If the libsim multi-array data is compatible with VTK SOA then make that
//   type to wrap the data array.
//
// Arguments:
//   dims             : The number of dimensions
//   c                : The handle to the variable data object.
//
// Returns:    A vtkDataArray SOA instance or NULL if it is not compatible.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 18 14:17:56 PDT 2018
//
// Modifications:
//
// ****************************************************************************
vtkDataArray *
SimV2_MultiArray_To_VTK_SOA(int ndims, visit_handle c)
{
    vtkDataArray *arr = NULL;

    // If the data arrays are not compatible with the VTK SOA type then return NULL.
    int dt;
    if(!SimV2_MultiArray_Compatible_With_VTK_SOA(ndims, c, dt))
        return arr;

    void *data = NULL;
    int owner, dataType, nComps, nTuples, memory, offset, stride;

    // It was compatible, make the right return object.
    if(dt == VISIT_DATATYPE_CHAR)
    {
        vtkSOADataArrayTemplate<char> *d = vtkSOADataArrayTemplate<char>::New();
        d->SetNumberOfComponents(ndims);
        for(int i = 0; i < ndims; ++i)
        {
            if(simv2_VariableData_getArrayData(c, i, memory, owner, dataType,
               nComps, nTuples, offset, stride, data) != VISIT_ERROR)
            {
                if(i == 0)
                    d->SetNumberOfTuples(nTuples);
                d->SetArray(i, (char *)data, nTuples, false, owner==VISIT_OWNER_SIM);
                debug5 << "\tAdding char component " << i << ": " << data << endl;
            }
        }
        arr = d;
    }
    else if(dt == VISIT_DATATYPE_INT)
    {
        vtkSOADataArrayTemplate<int> *d = vtkSOADataArrayTemplate<int>::New();
        d->SetNumberOfComponents(ndims);
        for(int i = 0; i < ndims; ++i)
        {
            if(simv2_VariableData_getArrayData(c, i, memory, owner, dataType,
               nComps, nTuples, offset, stride, data) != VISIT_ERROR)
            {
                if(i == 0)
                    d->SetNumberOfTuples(nTuples);
                d->SetArray(i, (int *)data, nTuples, false, owner==VISIT_OWNER_SIM);
                debug5 << "\tAdding int component " << i << ": " << data << endl;
            }
        }
        arr = d;
    }
    else if(dt == VISIT_DATATYPE_LONG)
    {
        vtkSOADataArrayTemplate<long> *d = vtkSOADataArrayTemplate<long>::New();
        d->SetNumberOfComponents(ndims);
        for(int i = 0; i < ndims; ++i)
        {
            if(simv2_VariableData_getArrayData(c, i, memory, owner, dataType,
               nComps, nTuples, offset, stride, data) != VISIT_ERROR)
            {
                if(i == 0)
                    d->SetNumberOfTuples(nTuples);
                d->SetArray(i, (long *)data, nTuples, false, owner==VISIT_OWNER_SIM);
                debug5 << "\tAdding long component " << i << ": " << data << endl;
            }
        }
        arr = d;
    }
    else if(dt == VISIT_DATATYPE_FLOAT)
    {
        vtkSOADataArrayTemplate<float> *d = vtkSOADataArrayTemplate<float>::New();
        d->SetNumberOfComponents(ndims);
        for(int i = 0; i < ndims; ++i)
        {
            if(simv2_VariableData_getArrayData(c, i, memory, owner, dataType,
               nComps, nTuples, offset, stride, data) != VISIT_ERROR)
            {
                if(i == 0)
                    d->SetNumberOfTuples(nTuples);
                d->SetArray(i, (float *)data, nTuples, false, owner==VISIT_OWNER_SIM);
                debug5 << "\tAdding float component " << i << ": " << data << endl;
            }
        }
        arr = d;
    }
    else if(dt == VISIT_DATATYPE_DOUBLE)
    {
        vtkSOADataArrayTemplate<double> *d = vtkSOADataArrayTemplate<double>::New();
        d->SetNumberOfComponents(ndims);
        for(int i = 0; i < ndims; ++i)
        {
            if(simv2_VariableData_getArrayData(c, i, memory, owner, dataType,
               nComps, nTuples, offset, stride, data) != VISIT_ERROR)
            {
                if(i == 0)
                    d->SetNumberOfTuples(nTuples);
                d->SetArray(i, (double *)data, nTuples, false, owner==VISIT_OWNER_SIM);
                debug5 << "\tAdding double component " << i << ": " << data << endl;
            }
        }
        arr = d;
    }

    if(arr != NULL)
    {
        debug5 << "\tSimV2_MultiArray_To_VTK_SOA: zero-copy" << endl;
    }

    return arr;
}

// ****************************************************************************
// Function: SimV2_Create_DataArray3_From_One_MultiArray
//
// Purpose:
//   Create a vtkComponentData with 3 components from a single VariableData
//   object that contains multiple arrays.
//
// Arguments:
//   dims             : The number of dimensions
//   c                : The handle to the variable data object.
//
// Returns:    A vtkComponentDataArray that wraps the data arrays.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 29 15:55:02 PDT 2015
//
// Modifications:
//
// ****************************************************************************
vtkDataArray *
SimV2_Create_DataArray3_From_One_MultiArray(int ndims, visit_handle c)
{
    debug5 << "SimV2_Create_DataArray3_From_One_MultiArray" << endl;

    vtkDataArray *arr = NULL;

    // Try and make a VTK SOA data array from the multi array.
    arr = SimV2_MultiArray_To_VTK_SOA(ndims, c);

    if(arr == NULL)
    {
        debug5 << "\tCould not make VTK SOA, Make VisIt vtkComponentDataArray." << endl;

        // Zero copy - use separate components to build up a view of the components
        vtkComponentDataArray<float> *position = vtkComponentDataArray<float>::New();
        position->SetNumberOfComponents(3);

        bool doNull = false;
        for(int i = 0; i < ndims; ++i)
        {
            static char coordName[3] = {'x', 'y', 'z'};

            // Get the information from the i'th data array component.
            void *data = NULL;
            int owner, dataType, nComps, nTuples, memory, offset, stride;
            if(simv2_VariableData_getArrayData(c, i, memory, owner, dataType,
               nComps, nTuples, offset, stride, data) == VISIT_ERROR)
            {
                position->Delete();
                ostringstream oss;
                oss << "Failed to get data for " << coordName[i];
                EXCEPTION1(ImproperUseException, oss.str().c_str());
            }

            // Store the data array information into the data component.
            if(i == 0)
                position->SetNumberOfTuples(nTuples);
            bool owns = owner == VISIT_OWNER_VISIT;

            debug5 << "\tAdding component " << i << ": data=" << (void*)data
                   << ", offset=" << offset << ", stride=" << stride
                   << ", vtktype=" << SimV2_GetVTKType(dataType)
                   << ", owns=" << (owns?"true":"false") << endl;

            position->SetComponentData(i, vtkArrayComponentStride(data, offset, stride, SimV2_GetVTKType(dataType), owns));
            doNull |= owns;
        }
        // For 2D data, install an empty 3rd component.
        if(ndims == 2)
        {
            debug5 << "\tAdding empty 3rd component." << endl;
            position->SetComponentData(2, vtkArrayComponentStride(NULL, 0, 0, VTK_FLOAT, false));
        }

        // Give up our ownership. VTK will free the data.
        if(doNull)
            simv2_VariableData_nullData(c);

        arr = position;
    }

    return arr;
}

// ****************************************************************************
// Function: SimV2_Create_ComponentDataArray3_From_Three_SingleArray
//
// Purpose:
//   Create a vtkComponentData with 3 components from 3 VariableData
//   objects that contains single arrays.
//
// Arguments:
//   dims             : The number of dimensions
//   x                : The handle to the x coordinate array.
//   y                : The handle to the y coordinate array.
//   z                : The handle to the z coordinate array.
//
// Returns:    A vtkComponentDataArray that wraps the data arrays.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 29 15:55:02 PDT 2015
//
// Modifications:
//
//  Burlen Loring, Sat Sep 12 15:54:59 PDT 2015
//  Add support for ref-counted zero-copy (ie VISIT_OWNER_EX)
//
// ****************************************************************************

static vtkComponentDataArray<float> *
SimV2_Create_ComponentDataArray3_From_Three_SingleArray(int ndims, 
    visit_handle x, visit_handle y, visit_handle z)
{
    debug5 << "SimV2_Create_ComponentDataArray3_From_Three_SingleArray" << endl;
    vtkComponentDataArray<float> *position = vtkComponentDataArray<float>::New();
    position->SetNumberOfComponents(3);
    visit_handle cHandles[3] = {x, y, z};
    for(int i = 0; i < ndims; ++i)
    {
        static char coordName[3] = {'x', 'y', 'z'};

        // Get the information from the data array.
        void *data = NULL;
        int owner, dataType, nComps, nTuples, memory, offset, stride;
        if(simv2_VariableData_getArrayData(cHandles[i], 0, memory, owner, dataType,
           nComps, nTuples, offset, stride, data) == VISIT_ERROR)
        {
            position->Delete();
            ostringstream oss;
            oss << "Failed to get data for " << coordName[i];
            EXCEPTION1(ImproperUseException, oss.str().c_str());
        }

        // Store the data array information into the data component.
        if(i == 0)
            position->SetNumberOfTuples(nTuples);
        bool visit_owns = owner == VISIT_OWNER_VISIT;

        debug5 << "\tAdding component " << i << ": data=" << (void*)data
               << ", offset=" << offset << ", stride=" << stride
               << ", vtktype=" << SimV2_GetVTKType(dataType)
               << ", owns=" << (visit_owns?"true":"false") << endl;

        position->SetComponentData(i, vtkArrayComponentStride(
            data, offset, stride, SimV2_GetVTKType(dataType), visit_owns));

        if (visit_owns)
        {
            // takes ownership
            simv2_VariableData_nullData(cHandles[i]);
        }
        else
        if (owner == VISIT_OWNER_VISIT_EX)
        {
            // zero copy from a ref counted array
            void (*callback)(void*) = NULL;
            void *callbackData = NULL;
            simv2_VariableData_getDeletionCallback(cHandles[i], callback, callbackData);

            if (!callback)
            {
                ostringstream oss;
                oss << "Attempt to use VISIT_OWNER_VISIT_EX without a callback"
                    << " for " << coordName[i] << " coordinates in point mesh";
                EXCEPTION1(ImproperUseException, oss.str().c_str());
            }

            simV2_DeleteEventObserver *observer = simV2_DeleteEventObserver::New();
            observer->Observe(position, callback, callbackData);
            // this is not a leak, the observer is Delete'd after it's
            // invoked.

            // takes ownership
            simv2_VariableData_nullData(cHandles[i]);
        }

    }
    if(ndims == 2)
    {
        debug5 << "\tAdding empty 3rd component." << endl;
        position->SetComponentData(2, vtkArrayComponentStride(NULL, 0, 0, VTK_FLOAT, false));
    }
#if 0
    for(vtkIdType i = 0; i < position->GetNumberOfTuples(); ++i)
    {
        const double *pt = position->GetTuple(i);
        cout << "pt[" << i << "] = " << pt[0] << ", " << pt[1] << ", " << pt[2] << endl;
    }
#endif
    return position;
}

// ****************************************************************************
// Function: SimV2_CreatePoints_From_DataArray3
//
// Purpose:
//   Create a vtkPoints object from a component data array with 3 components.
//
// Arguments:
//   dims             : The number of dimensions
//   additionalPoints : The number of additional points we need to allocate.
//   position         : The data array containing the points.
//
// Returns:    A vtkPoints object that contains the points.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 16 16:00:50 PDT 2015
//
// Modifications:
//
// ****************************************************************************

static vtkPoints *
SimV2_CreatePoints_From_DataArray3(bool forceCopy, int ndims, int additionalPoints,
    vtkDataArray *position)
{
    (void)ndims;

    vtkPoints *points = vtkPoints::New();

    if(/*ndims == 2 || */ forceCopy || additionalPoints > 0)
    {
        debug5 << "SimV2_CreatePoints_From_DataArray3: copy points" << endl;

        // We need to copy so copy the zero-copy form into a new vtkPoints.
        vtkIdType nTuples = position->GetNumberOfTuples();
        points->SetNumberOfPoints(nTuples + additionalPoints);
        for(vtkIdType ptid = 0; ptid < nTuples; ++ptid)
        {
            const double *p = position->GetTuple(ptid);
            points->SetPoint(ptid, p[0], p[1], p[2]);
        }
    }
    else
    {
        debug5 << "SimV2_CreatePoints_From_DataArray3: zero copy" << endl;

        // Zero-copy.
        points->SetData(position);
    }

    // Delete the zero-copy view of the data.
    position->Delete();

    return points;
}

// ****************************************************************************
// Function: SimV2_CreatePoints_Separate
//
// Purpose:
//   Create a vtkPoints object from various VariableData objects.
//
// Arguments:
//   dims             : The number of dimensions
//   x                : The handle to the x coordinate array.
//   y                : The handle to the y coordinate array.
//   z                : The handle to the z coordinate array.
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
//    Brad Whitlock, Mon Jun 29 15:55:02 PDT 2015
//    I added support for strided memory.
//
// ****************************************************************************

static vtkPoints *
SimV2_CreatePoints_Separate(int ndims,
    visit_handle x, visit_handle y, visit_handle z, int additionalPoints, bool forceCopy)
{
    debug5 << "SimV2_CreatePoints_Separate" << endl;
    vtkComponentDataArray<float> *position = SimV2_Create_ComponentDataArray3_From_Three_SingleArray(ndims, x,y,z);
    return SimV2_CreatePoints_From_DataArray3(forceCopy, 3, additionalPoints, position);
}

// ****************************************************************************
// Function: SimV2_CreatePoints_Interleaved_SingleArray
//
// Purpose:
//   Create a vtkPoints object from a single VariableData object that is made of
//   a single array.
//
// Arguments:
//   dims             : The number of dimensions
//   c                : The handle to the coordinate array.
//   additionalPoints : The number of additional points to create.
//
// Returns:    A vtkPoints object that contains the coordinates.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 29 15:55:02 PDT 2015
//
// Modifications:
//
// ****************************************************************************

static vtkPoints *
SimV2_CreatePoints_Interleaved_SingleArray(int ndims, visit_handle c, 
    int additionalPoints, bool forceCopy)
{
    // Get information about the first array.
    int owner, dataType, nComps, nTuples;
    void *data = NULL;
    int ierr = simv2_VariableData_getData(c, owner, dataType, nComps,
                                          nTuples, data);
    if (ierr == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Failed to get data for interleaved coordinates");
    }

    // validate the data type.
    if (!simV2_ValidFloatDataType(dataType))
    {
        EXCEPTION1(ImproperUseException,
                   "Coordinate array must be float or double.\n");
    }

    vtkPoints *points = vtkPoints::New();

    // NOTE: The API prevents strided array access for this case. We can assume still
    //       that the arrays here will be contiguous 1 array with 2-3 components.

    if(ndims == 2)
    {
        debug5 << "SimV2_CreatePoints_Interleaved_SingleArray: copy 2d points to 3d points." << endl;

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
    else if (forceCopy || additionalPoints > 0)
    {
        debug5 << "SimV2_CreatePoints_Interleaved_SingleArray: copy points to buffer with "
               << additionalPoints << " extra elements." << endl;

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
        debug5 << "SimV2_CreatePoints_Interleaved_SingleArray: zero copy." << endl;

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
                void (*callback)(void*) = NULL;
                void *callbackData = NULL;
                simv2_VariableData_getDeletionCallback(c, callback, callbackData);

                if (!callback)
                {
                    ostringstream oss;
                    oss << "Attempt to use VISIT_OWNER_VISIT_EX without a callback"
                        << " for interleaved coordinates in a point based mesh";
                    EXCEPTION1(ImproperUseException, oss.str().c_str());
                 }

                // we observe VTK data array's DeleteEvent and invoke the
                // user provided callback in repsonse. it's the callbacks
                // duty to free the memory.
                pts->SetArray(static_cast<simV2_TT::cppType*>(data), nTuples*3, 1);

                simV2_DeleteEventObserver *observer = simV2_DeleteEventObserver::New();
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

    return points;
}

// ****************************************************************************
// Function: SimV2_CreatePoints_Interleaved_MultiArray
//
// Purpose:
//   Create a vtkPoints object from a single VariableData object that is made of
//   multiple arrays.
//
// Arguments:
//   dims             : The number of dimensions
//   c                : The handle to the coordinate array.
//   additionalPoints : The number of additional points to create.
//   forceCopy        : A flag to force copying.
//
// Returns:    A vtkPoints object that contains the coordinates.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 29 15:55:02 PDT 2015
//
// Modifications:
//
// ****************************************************************************

static vtkPoints *
SimV2_CreatePoints_Interleaved_MultiArray(int ndims, visit_handle c, int additionalPoints,
    bool forceCopy)
{
    // NOTE: Here we have a separate array for each component. Each component may
    //       have a different memory layout or storage type.
    debug5 << "SimV2_CreatePoints_Interleaved_MultiArray" << endl;
    vtkDataArray *position = SimV2_Create_DataArray3_From_One_MultiArray(ndims, c);
    return SimV2_CreatePoints_From_DataArray3(forceCopy, ndims, additionalPoints, position);
}

// ****************************************************************************
// Function: SimV2_CreatePoints_Interleaved
//
// Purpose:
//   Create a vtkPoints object from various VariableData objects.
//
// Arguments:
//   dims             : The number of dimensions
//   c                : The handle to the coordinate array.
//   additionalPoints : The number of additional points to create.
//   forceCopy        : A flag to force copying.
//
// Returns:    A vtkPoints object that contains the coordinates.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 25 11:14:01 PST 2010
//
// Modifications:
//    Brad Whitlock, Mon Jun 29 15:55:02 PDT 2015
//    I added support for strided memory.
//
// ****************************************************************************

static vtkPoints *
SimV2_CreatePoints_Interleaved(int ndims, visit_handle c, int additionalPoints, bool forceCopy)
{
    debug5 << "SimV2_CreatePoints_Interleaved" << endl;

    vtkPoints *points = NULL;
    int nArrs = 1;
    if(simv2_VariableData_getNumArrays(c, &nArrs) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Failed to get number of arrays for interleaved coordinates");
    }

    if(nArrs == 1)
        points = SimV2_CreatePoints_Interleaved_SingleArray(ndims, c, additionalPoints, forceCopy);
    else
        points = SimV2_CreatePoints_Interleaved_MultiArray(ndims, c, additionalPoints, forceCopy);
    return points;
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
//   forceCopy        : A flag to force copying.
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
    int additionalPoints, bool forceCopy)
{
    debug5 << "SimV2_CreatePoints" << endl;
    vtkPoints *points = NULL;

    if(coordMode == VISIT_COORD_MODE_SEPARATE)
        points = SimV2_CreatePoints_Separate(ndims, x, y, z, additionalPoints, forceCopy);
    else
        points = SimV2_CreatePoints_Interleaved(ndims, c, additionalPoints, forceCopy);

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

    bool forceCopy = false;
#ifdef VTK_DATA_WRITER_WORKAROUND
    if(ndims == 2)
    {
        if(coordMode == VISIT_COORD_MODE_SEPARATE)
            forceCopy = true;
        else
        {
            int nArr = 0;
            if(simv2_VariableData_getNumArrays(c, &nArr) == VISIT_OKAY)
            {
                 forceCopy = nArr > 1;
            }
        }
    }
#endif

    // Create the points.
    vtkPoints *points = SimV2_CreatePoints(ndims, coordMode, x, y, z, c, 0, forceCopy);

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
//   Brad Whitlock, Mon Jun 29 17:06:23 PDT 2015
//   Added support for strided memory.
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
    int memory[3]={0,0,0}, stride[3]={1,1,1}, offset[3] = {0,0,0};
    void *data[3] = {0,0,0};
    for(int i = 0; i < ndims; ++i)
    {
        if(simv2_VariableData_getArrayData(cHandles[i], 0, memory[i], owner[i],
            dataType[i], nComps[i], nTuples[i], offset[i], stride[i], data[i]) == VISIT_ERROR)
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
    bool coordError = false;
    for(int i = 0; i < 3; ++i)
    {
        switch (dataType[i])
        {
#define COPYMACRO(index,srcval) dest[index] = srcval;
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
                simV2_TT::cppType *dest = static_cast<simV2_TT::cppType *>(coords[i]->GetVoidPointer(0));
                bool err = true;
                simV2MemoryCopyMacro(COPYMACRO, memory[i], nTuples[i], offset[i], stride[i], simV2_TT::cppType, data[i], err);
                coordError |= err;
            }
            );
#undef COPYMACRO
        }
    }

    if(coordError)
    {
        for (int i=0; i<3; ++i) 
            coords[i]->Delete();
        EXCEPTION1(ImproperUseException,
                   "Problem with coordinates.\n");
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
    int celltype_npts[40];
    InitializeCellNumPoints(celltype_npts);

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
        else if(ValidCellType(celltype))
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
//   Brad Whitlock, Fri Jan  8 15:17:10 PST 2016
//   Add some quadratic cell types.
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
    int connMemory = 0, connOwner = 0, connDataType=0, connNComps=0, connectivityLen=0;
    int connOffset = 0, connStride = 0;
    void *connData = 0;
    if(simv2_VariableData_getArrayData(conn, 0, connMemory, connOwner,
       connDataType, connNComps, connectivityLen, connOffset, connStride, connData) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
             "Could not obtain connectivity data using the provided handle.\n");
    }

    if(connMemory == VISIT_MEMORY_STRIDED)
    {
        EXCEPTION1(ImproperUseException,
             "Connectivity data must be contiguous in memory.\n");
    }

    const int *connectivity = (const int *)connData;

    // Count the polyhedral cells so we can allocate more points
    int normalCellCount = 0, polyhedralCellCount = 0;
    SimV2_UnstructuredMesh_Count_Cells(connectivity, connectivityLen, normalCellCount,
        polyhedralCellCount);

    bool forceCopy = false;
#ifdef VTK_DATA_WRITER_WORKAROUND
    if(ndims == 2)
    {
        if(coordMode == VISIT_COORD_MODE_SEPARATE)
            forceCopy = true;
        else
        {
            int nArr = 0;
            if(simv2_VariableData_getNumArrays(c, &nArr) == VISIT_OKAY)
            {
                 forceCopy = nArr > 1;
            }
        }
    }
#endif

    //
    // Create the points.
    //
    vtkPoints *points = SimV2_CreatePoints(ndims, coordMode, x, y, z, c,
                                           polyhedralCellCount,
                                           forceCopy);
    int nRealPoints = points->GetNumberOfPoints() - polyhedralCellCount;

    //
    // Create the cells.
    //
    int celltype_npts[40];
    InitializeCellNumPoints(celltype_npts);

    int celltype_idtype[40];
    celltype_idtype[VISIT_CELL_BEAM]  = VTK_LINE;
    celltype_idtype[VISIT_CELL_TRI]   = VTK_TRIANGLE;
    celltype_idtype[VISIT_CELL_QUAD]  = VTK_QUAD;
    celltype_idtype[VISIT_CELL_TET]   = VTK_TETRA;
    celltype_idtype[VISIT_CELL_PYR]   = VTK_PYRAMID;
    celltype_idtype[VISIT_CELL_WEDGE] = VTK_WEDGE;
    celltype_idtype[VISIT_CELL_HEX]   = VTK_HEXAHEDRON;
    celltype_idtype[VISIT_CELL_POINT] = VTK_VERTEX;
    celltype_idtype[VISIT_CELL_QUADRATIC_EDGE]          = VTK_QUADRATIC_EDGE;
    celltype_idtype[VISIT_CELL_QUADRATIC_TRI]           = VTK_QUADRATIC_TRIANGLE;
    celltype_idtype[VISIT_CELL_QUADRATIC_QUAD]          = VTK_QUADRATIC_QUAD;
    celltype_idtype[VISIT_CELL_QUADRATIC_TET]           = VTK_QUADRATIC_TETRA;
    celltype_idtype[VISIT_CELL_QUADRATIC_HEX]           = VTK_QUADRATIC_HEXAHEDRON;
    celltype_idtype[VISIT_CELL_QUADRATIC_WEDGE]         = VTK_QUADRATIC_WEDGE;
    celltype_idtype[VISIT_CELL_QUADRATIC_PYR]           = VTK_QUADRATIC_PYRAMID;
    celltype_idtype[VISIT_CELL_BIQUADRATIC_TRI]         = VTK_BIQUADRATIC_TRIANGLE;
    celltype_idtype[VISIT_CELL_BIQUADRATIC_QUAD]        = VTK_BIQUADRATIC_QUAD;
    celltype_idtype[VISIT_CELL_TRIQUADRATIC_HEX]        = VTK_TRIQUADRATIC_HEXAHEDRON;
    celltype_idtype[VISIT_CELL_QUADRATIC_LINEAR_QUAD]   = VTK_QUADRATIC_LINEAR_QUAD;
    celltype_idtype[VISIT_CELL_QUADRATIC_LINEAR_WEDGE]  = VTK_QUADRATIC_LINEAR_WEDGE;
    celltype_idtype[VISIT_CELL_BIQUADRATIC_QUADRATIC_WEDGE] = VTK_BIQUADRATIC_QUADRATIC_WEDGE;
    celltype_idtype[VISIT_CELL_BIQUADRATIC_QUADRATIC_HEX]   = VTK_BIQUADRATIC_QUADRATIC_HEXAHEDRON;


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
    vtkIdType verts[27];
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
        else if(ValidCellType(celltype))
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
        snprintf(tmp, 100, "Cell %d's connectivity contained invalid points or "
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
    vtkPoints *points = SimV2_CreatePoints(ndims, coordMode, x, y, z, c, 0, false);

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
