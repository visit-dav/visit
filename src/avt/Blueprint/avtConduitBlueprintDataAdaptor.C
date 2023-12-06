// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtConduitBlueprintDataAdaptor.h>
#include <conduit.hpp>
#include <conduit_blueprint.hpp>
#include <conduit_blueprint_mesh_utils.hpp>
#include "avtConduitBlueprintLogging.h"
#include "avtConduitBlueprintInfoWarningHandler.h"

//-----------------------------------------------------------------------------
// vtk includes
//-----------------------------------------------------------------------------
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCharArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkShortArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkIntArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkLongArray.h>
#include <vtkLongLongArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkUnsignedLongLongArray.h>
#include <vtkFloatArray.h>
#include <vtkCellArray.h>
#include <vtkIdTypeArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkVisItUtility.h>

using namespace conduit;
using namespace mfem;

// ****************************************************************************
//  Method: Initialize
//
//  Purpose: Initialize the conduit blueprint data adaptor.
//
//  Programmer: Justin Privitera
//  Creation:   Mon Aug 22 17:15:06 PDT 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtConduitBlueprintDataAdaptor::Initialize()
{
    SetInfoWarningHandlers();
}

// ****************************************************************************
//  Method: SetInfoWarningHandlers
//
//  Purpose: Set the info and warning handlers for conduit info and warnings.
//
//  Programmer: Justin Privitera
//  Creation:   Mon Aug 22 17:15:06 PDT 2022
//
//  Modifications:
//
// ****************************************************************************
void
avtConduitBlueprintDataAdaptor::SetInfoWarningHandlers()
{
    // these redirect conduit info and warnings to debug 5
    conduit::utils::set_info_handler(
        avtConduitBlueprintInfoWarningHandler::avt_conduit_blueprint_info_handler);
    conduit::utils::set_warning_handler(
        avtConduitBlueprintInfoWarningHandler::avt_conduit_blueprint_warning_handler);
}

// ****************************************************************************
// ****************************************************************************
///
/// VTK Data Adaptor Functions
///
// ****************************************************************************
// ****************************************************************************


// ****************************************************************************
static int
ElementShapeNameToVTKCellType(const std::string &shape_name)
{
    if (shape_name == "point")   return VTK_VERTEX;
    if (shape_name == "line")    return VTK_LINE;
    if (shape_name == "tri")     return VTK_TRIANGLE;
    if (shape_name == "quad")    return VTK_QUAD;
    if (shape_name == "hex")     return VTK_HEXAHEDRON;
    if (shape_name == "tet")     return VTK_TETRA;
    if (shape_name == "wedge")   return VTK_WEDGE;
    if (shape_name == "pyramid") return VTK_PYRAMID;
    AVT_CONDUIT_BP_WARNING("Unsupported Element Shape: " << shape_name);
    return 0;
}

// ****************************************************************************
static int
VTKCellTypeSize(int cell_type)
{
    if (cell_type == VTK_VERTEX)     return 1;
    if (cell_type == VTK_LINE)       return 2;
    if (cell_type == VTK_TRIANGLE)   return 3;
    if (cell_type == VTK_QUAD)       return 4;
    if (cell_type == VTK_HEXAHEDRON) return 8;
    if (cell_type == VTK_TETRA)      return 4;
    if (cell_type == VTK_WEDGE)      return 6;
    if (cell_type == VTK_PYRAMID)    return 5;
    return 0;
}

// ****************************************************************************
std::string
VTKCellTypeToElementShapeName(const int vtk_cell_type)
{
    if (vtk_cell_type == VTK_VERTEX)     return "point";
    if (vtk_cell_type == VTK_LINE)       return "line";
    if (vtk_cell_type == VTK_TRIANGLE)   return "tri";
    if (vtk_cell_type == VTK_QUAD)       return "quad";
    if (vtk_cell_type == VTK_HEXAHEDRON) return "hex";
    if (vtk_cell_type == VTK_VOXEL)      return "hex";
    if (vtk_cell_type == VTK_TETRA)      return "tet";
    if (vtk_cell_type == VTK_WEDGE)      return "wedge";
    if (vtk_cell_type == VTK_PYRAMID)    return "pyramid";

    AVT_CONDUIT_BP_WARNING("Unsupported vtkCellType : " << vtk_cell_type);
    return "";
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
//
// Helpers for creating VTK objects from Blueprint conforming Conduit data.
//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

// ****************************************************************************
//  Modifications:
//
//  Cyrus Harrison, Fri Dec 16 09:38:35 PST 2022
//  Add support for optional indirection array.
//
// ****************************************************************************
template<typename T> void
Blueprint_MultiCompArray_To_VTKDataArray(const Node &n,
                                         int ncomps,
                                         int ntuples,
                                         int *src_idxs,
                                         vtkDataArray *darray)
{
    // vtk reqs us to set number of comps before number of tuples
    if( ncomps == 2) // we need 3 comps for vectors
        darray->SetNumberOfComponents(3);
    else
        darray->SetNumberOfComponents(ncomps);
    // set number of tuples
    darray->SetNumberOfTuples(ntuples);

    // handle multi-component case
    if(n.number_of_children() > 0)
    {
        for(vtkIdType c=0; c < ncomps; c++)
        {
            conduit::DataArray<T> vals_array = n[c].value();;

            for (vtkIdType i = 0; i < ntuples; i++)
            {
                int idx = i;
                if(src_idxs!=nullptr)
                {
                    idx = src_idxs[i];
                }
                darray->SetComponent(i, c, (double) vals_array[idx]);

                if(ncomps == 2)
                {
                    darray->SetComponent(i, 2, 0.0);
                }
            }
        }
    }
    // single array case
    else
    {
        conduit::DataArray<T> vals_array = n.value();
        for (vtkIdType i = 0; i < ntuples; i++)
        {
            int idx = i;
            if(src_idxs!=nullptr)
            {
                idx = src_idxs[i];
            }
            darray->SetComponent(i,0, (double) vals_array[idx]);
        }
    }
}

// ****************************************************************************
// ****************************************************************************
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//                               BLUEPRINT --> VTK
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
//                             Helper functions
// ****************************************************************************

// ****************************************************************************
//  Method: ConduitArrayToVTKDataArray
//
//  Purpose:
//   Constructs a vtkDataArray from a Conduit mcarray.
//
//  Arguments:
//   n:    Blueprint Field Values Node
//
//  Programmer: Cyrus Harrison
//  Creation:   Sat Jul  5 11:38:31 PDT 2014 (?)
//
//  Modifications:
//    Cyrus Harrison, Thu Jan 13 11:14:20 PST 2022
//    Add support for unsigned long and unsigned long long.
// 
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to conduit blueprint data adaptor.
//
//    Cyrus Harrison, Fri Dec 16 09:33:41 PST 2022
//    Added support for an optional indirection array
//
// ****************************************************************************
vtkDataArray *
ConduitArrayToVTKDataArray(const conduit::Node &n,
                           int src_idxs_length = 0,
                           int *src_idxs = NULL)
{
    AVT_CONDUIT_BP_INFO("Creating VTKDataArray from Node");
    vtkDataArray *retval = NULL;


    int nchildren = n.number_of_children();
    int ntuples = 0;
    int ncomps  = 1;


    DataType vals_dtype;

    if(nchildren > 0) // n is a mcarray w/ children that hold the vals
    {
        Node v_info;
        if(!blueprint::mcarray::verify(n,v_info))
        {
            AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException,
                                      "Node is not a mcarray " << v_info.to_yaml());
        }

        // in this case, each child is a component of the array
        ncomps = nchildren;
        // This assumes all children have the same leaf type
        vals_dtype = n[0].dtype();
    }
    else // n is an array, holds the vals
    {
        vals_dtype = n.dtype();
    }

    // get the number of tuples
    ntuples = (int) vals_dtype.number_of_elements();
    
    if(src_idxs_length > 0)
    {
        ntuples = src_idxs_length;
    }

    AVT_CONDUIT_BP_INFO("VTKDataArray num_tuples = " << ntuples << " "
                        << " num_comps = " << ncomps);


    if (vals_dtype.is_unsigned_char())
    {
        retval = vtkUnsignedCharArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_UNSIGNED_CHAR>(n,
                                                                               ncomps,
                                                                               ntuples,
                                                                               src_idxs,
                                                                               retval);
    }
    else if (vals_dtype.is_unsigned_short())
    {
        retval = vtkUnsignedShortArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_UNSIGNED_SHORT>(n,
                                                                                ncomps,
                                                                                ntuples,
                                                                                src_idxs,
                                                                                retval);
    }
    else if (vals_dtype.is_unsigned_int())
    {
        retval = vtkUnsignedIntArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_UNSIGNED_INT>(n,
                                                                              ncomps,
                                                                              ntuples,
                                                                              src_idxs,
                                                                              retval);
    }
    else if (vals_dtype.is_unsigned_long())
    {
        retval = vtkUnsignedLongArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_UNSIGNED_LONG>(n,
                                                                               ncomps,
                                                                               ntuples,
                                                                               src_idxs,
                                                                               retval);
    }
#if CONDUIT_USE_LONG_LONG
    else if (vals_dtype.id() == CONDUIT_NATIVE_UNSIGNED_LONG_LONG_ID)
    {
        retval = vtkUnsignedLongLongArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_UNSIGNED_LONG_LONG>(n,
                                                                                    ncomps,
                                                                                    ntuples,
                                                                                    src_idxs,
                                                                                    retval);
    }
#endif
    else if (vals_dtype.is_char())
    {
        retval = vtkCharArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_CHAR>(n,
                                                                      ncomps,
                                                                      ntuples,
                                                                      src_idxs,
                                                                      retval);

    }
    else if (vals_dtype.is_short())
    {
        retval = vtkShortArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_SHORT>(n,
                                                                       ncomps,
                                                                       ntuples,
                                                                       src_idxs,
                                                                       retval);
    }
    else if (vals_dtype.is_int())
    {
        retval = vtkIntArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_INT>(n,
                                                                     ncomps,
                                                                     ntuples,
                                                                     src_idxs,
                                                                     retval);
    }
    else if (vals_dtype.is_long())
    {
        retval = vtkLongArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_LONG>(n,
                                                                      ncomps,
                                                                      ntuples,
                                                                      src_idxs,
                                                                      retval);
    }
#if CONDUIT_USE_LONG_LONG
    else if (vals_dtype.id() == CONDUIT_NATIVE_LONG_LONG_ID)
    {
        retval = vtkLongLongArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_LONG_LONG>(n,
                                                                              ncomps,
                                                                              ntuples,
                                                                              src_idxs,
                                                                              retval);
    }
#endif
    else if (vals_dtype.is_float())
    {
        retval = vtkFloatArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_FLOAT>(n,
                                                                       ncomps,
                                                                       ntuples,
                                                                       src_idxs,
                                                                       retval);
    }
    else if (vals_dtype.is_double())
    {
        retval = vtkDoubleArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_DOUBLE>(n,
                                                                        ncomps,
                                                                        ntuples,
                                                                        src_idxs,
                                                                        retval);
    }
    else
    {
        AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException,
                                  "Conduit Array to VTK Data Array"
                                  "unsupported data type: " << n.dtype().name());
    }
    return retval;
}


// ****************************************************************************
int
NDIndex(int i, int j, int k, int *offsets, int *strides)
{
    int res = (i + offsets[0]) * strides[0];
    res += (j +offsets[1]) * strides[1];
    res += (k +offsets[2]) * strides[2];
    return res;
}


// ****************************************************************************
void
StructuredTopologyShape(const Node &n_topo, int *shape)
{
    if(n_topo.has_path("elements/dims/i"))
    {
        shape[0] = n_topo["elements/dims/i"].to_value();
    }
    else
    {
        shape[0] = 0;
    }

    if(n_topo.has_path("elements/dims/j"))
    {
        shape[1] = n_topo["elements/dims/j"].to_value();
    }
    else
    {
        shape[1] = 0;
    }

    if(n_topo.has_path("elements/dims/k"))
    {
        shape[2] = n_topo["elements/dims/k"].to_value();
    }
    else
    {
        shape[2] = 0;
    }
}

// ****************************************************************************
void
StructuredTopologyOffsetsAndOffsets(const conduit::Node n_topo,
                                    const int *shape,
                                    int *offsets,
                                    int *strides)
{
    if(n_topo.has_path("elements/dims/offsets"))
    {
        int_accessor off_vals = n_topo["elements/dims/offsets"].value();
        for(int i=0;i<off_vals.dtype().number_of_elements();i++)
        {
            offsets[i] = off_vals[i];
        }
    }
    // Note: default offsets values should be {0,0,0}

    if(n_topo.has_path("elements/dims/strides"))
    {
        int_accessor stride_vals = n_topo["elements/dims/strides"].value();
        for(int i=0;i<stride_vals.dtype().number_of_elements();i++)
        {
            strides[i] = stride_vals[i];
        }
    }
    else // default strides
    {
        strides[0] = 1;
        strides[1] = shape[0];
        strides[2] = shape[1];
    }
}



// ****************************************************************************
vtkPoints *
ExplicitCoordsToVTKPoints(const Node &n_coords, const Node &n_topo)
{

    vtkPoints *points = vtkPoints::New();

    // Identify cases for struct stride
    // shape, offsets, strides
    // points will be walked in i,j,k in logical order

    bool ndstrided  = false;
    int  ele_shape[3]   = {0,0,0};
    int  ele_strides[3] = {0,0,0};
    int  ele_offsets[3] = {0,0,0};

    int  pts_shape[3]   = {0,0,0};
    int  pts_strides[3] = {0,0,0};
    int  pts_offsets[3] = {0,0,0};

    //
    // only support offsets, strides for "structured" topos
    //
    // NOTE: we could also support this uniform or rectilinear
    // but we need more / different logic to detect the logical
    // point dims from the coordset
    if( n_topo["type"].as_string() == "structured" &&
        ( n_topo.has_path("elements/dims/offsets") ||
          n_topo.has_path("elements/dims/strides") ) )
    {
        AVT_CONDUIT_BP_INFO("ExplicitCoordsToVTKPoints:: structured strided case ");
        // for this case, we need to apply 
        // (i,j,k,shape,offsets,strides) --> index
        // to extract the coords

        ndstrided = true;

        // NOTE: This finds the shape and striding details of the elements
        // to construct for vtk, we need to convert this to shape and striding
        // details of points
        StructuredTopologyShape(n_topo,ele_shape);
        StructuredTopologyOffsetsAndOffsets(n_topo,
                                            ele_shape,
                                            ele_offsets,
                                            ele_strides);

        // adj shape
        pts_shape[0] = ele_shape[0]+1;
        pts_shape[1] = ele_shape[1]+1;
        pts_shape[2] = ele_shape[2]+1;

        // NOTE: offsets don't need to be adjusted ??
        pts_offsets[0] = ele_offsets[0];
        pts_offsets[1] = ele_offsets[1];
        pts_offsets[2] = ele_offsets[2];

        // strides *do* need to be adjusted.
        // this is tricky?
        pts_strides[0] = ele_strides[0];
        pts_strides[1] = ele_strides[1];
        pts_strides[2] = ele_strides[2];

        AVT_CONDUIT_BP_INFO("ExplicitCoordsToVTKPoints:: point shape: "
                            << pts_shape[0] << " "
                            << pts_shape[1] << " "
                            << pts_shape[2] );

        AVT_CONDUIT_BP_INFO("ExplicitCoordsToVTKPoints:: point offsets: "
                            << pts_offsets[0] << " "
                            << pts_offsets[1] << " "
                            << pts_offsets[2] );

        AVT_CONDUIT_BP_INFO("ExplicitCoordsToVTKPoints:: point strides: "
                            << pts_strides[0] << " "
                            << pts_strides[1] << " "
                            << pts_strides[2] );
    }

    const Node &n_vals = n_coords["values"];

    int npts =0;

    if(ndstrided)
    {
        npts = pts_shape[0] * pts_shape[1] * pts_shape[2];
    }
    else
    {
        npts = (int) n_vals.child(0).dtype().number_of_elements();
    }

    AVT_CONDUIT_BP_INFO("ExplicitCoordsToVTKPoints:: number of points ="  << npts );

    // TODO: Simplify with double_accessor once
    // conduit allows empty double_accessor construction.
    double_array x_vals;
    double_array y_vals;
    double_array z_vals;

    bool have_y = false;
    bool have_z = false;

    Node n_vals_double;
    std::string coord_sys_type = conduit::blueprint::mesh::utils::coordset::coordsys(n_coords);

    if(coord_sys_type == "cylindrical")
    {
        if(!n_vals["z"].dtype().is_double())
        {
            n_vals["z"].to_double_array(n_vals_double["z"]);
            x_vals = n_vals_double["z"].value();
        }
        else
        {
            x_vals = n_vals["z"].value();
        }

        have_y = true;

        if(!n_vals["r"].dtype().is_double())
        {
            n_vals["r"].to_double_array(n_vals_double["r"]);
            y_vals = n_vals_double["r"].value();
        }
        else
        {
            y_vals = n_vals["r"].value();
        }
    }
    else
    {
        if(!n_vals["x"].dtype().is_double())
        {
            n_vals["x"].to_double_array(n_vals_double["x"]);
            x_vals = n_vals_double["x"].value();
        }
        else
        {
            x_vals = n_vals["x"].value();
        }

        if(n_vals.has_child("y"))
        {
            have_y = true;

            if(!n_vals["y"].dtype().is_double())
            {
                n_vals["y"].to_double_array(n_vals_double["y"]);
                y_vals = n_vals_double["y"].value();
            }
            else
            {
                y_vals = n_vals["y"].value();
            }
        }

        if(n_vals.has_child("z"))
        {
            have_z = true;

            if(!n_vals["z"].dtype().is_double())
            {
                n_vals["z"].to_double_array(n_vals_double["z"]);
                z_vals = n_vals_double["z"].value();
            }
            else
            {
                z_vals = n_vals["z"].value();
            }
        }
    }

    points->SetDataTypeToDouble();
    points->SetNumberOfPoints(npts);


    if(ndstrided) // strided case
    {
        vtkIdType vtk_idx = 0;
        for(int k=0; k < pts_shape[2]; k++)
        {
            for(int j=0; j < pts_shape[1]; j++)
            {
                for(int i=0; i < pts_shape[0]; i++)
                {
                    int bp_idx = NDIndex(i, j, k,
                                         pts_offsets,
                                         pts_strides);

                    double x = x_vals[bp_idx];
                    double y = have_y ? y_vals[bp_idx] : 0;
                    double z = have_z ? z_vals[bp_idx] : 0;
                    points->SetPoint(vtk_idx, x, y, z);
                    vtk_idx++;
                }
            }
        }
    }
    else // default, simplest case
    {
        for (vtkIdType i = 0; i < npts; i++)
        {
            double x = x_vals[i];
            double y = have_y ? y_vals[i] : 0;
            double z = have_z ? z_vals[i] : 0;
            points->SetPoint(i, x, y, z);
        }
    }

    return points;
}

// ****************************************************************************
//  Method: HomogeneousShapeTopologyToVTKCellArray
//
//  Purpose:
//   (outdated?) Translates the blueprint connectivity array to a VTK 
//   connectivity array.
//   Constructs a vtkCell array from a Blueprint topology
//
//  Arguments:
//   n_topo:    Blueprint Topology
//
//  Programmer: Cyrus Harrison
//  Creation:   Sat Jul  5 11:38:31 PDT 2014
//
//  Modifications:
//    Chris Laganella, Fri Nov  5 17:21:05 EDT 2021
//    I fixed a bug where it was copying the entire connectivity array in each
//    iteration of the for loop.
//    
//    Chris Laganella, Thu Jan 13 11:07:26 PST 2022
//    Fix bug where index array could be copied in interloop
// 
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to conduit blueprint data adaptor.
//
// ****************************************************************************

vtkCellArray *
HomogeneousShapeTopologyToVTKCellArray(const Node &n_topo,
                                       int /* npts -- UNUSED */)
{
    vtkCellArray *ca = vtkCellArray::New();
    vtkIdTypeArray *ida = vtkIdTypeArray::New();

    // TODO, I don't think we need this logic any more
    // // Handle empty and point topology
    // if (!n_topo.has_path("elements/connectivity") ||
    //     (n_topo.has_path("elements/shape") &&
    //      n_topo["elements/shape"].as_string() == "point"))
    // {
    //     // TODO, why is this 2 * npts?
    //     ida->SetNumberOfTuples(2*npts);
    //     for (int i = 0 ; i < npts; i++)
    //     {
    //         ida->SetComponent(2*i  , 0, 1);
    //         ida->SetComponent(2*i+1, 0, i);
    //     }
    //     ca->SetCells(npts, ida);
    //     ida->Delete();
    // }
    // else
    // {

        int ctype = ElementShapeNameToVTKCellType(n_topo["elements/shape"].as_string());
        int csize = VTKCellTypeSize(ctype);
        int ncells = n_topo["elements/connectivity"].dtype().number_of_elements() / csize;
        ida->SetNumberOfTuples(ncells * (csize + 1));

        // Extract connectivity as int array, using 'to_int_array' if needed.
        int_array topo_conn;
        Node n_tmp;
        if(n_topo["elements/connectivity"].dtype().is_int())
        {
            topo_conn = n_topo["elements/connectivity"].as_int_array();
        }
        else
        {
            n_topo["elements/connectivity"].to_int_array(n_tmp);
            topo_conn = n_tmp.as_int_array();
        }

        for (int i = 0 ; i < ncells; i++)
        {
            ida->SetComponent((csize+1)*i, 0, csize);
            for (int j = 0; j < csize; j++)
            {
                ida->SetComponent((csize+1)*i+j+1, 0,topo_conn[i*csize+j]);
            }
        }
        ca->SetCells(ncells, ida);
        ida->Delete();
    // }
    return ca;
}

// ****************************************************************************
vtkDataSet *
UniformCoordsToVTKRectilinearGrid(const Node &n_coords)
{
    vtkRectilinearGrid *rectgrid = vtkRectilinearGrid::New();

    AVT_CONDUIT_BP_INFO("UniformCoordsToVTKRectilinearGrid");
    AVT_CONDUIT_BP_INFO(n_coords.to_yaml());

    int nx[3];
    nx[0] = n_coords["dims"].has_child("i") ? n_coords["dims/i"].to_int() : 1;
    nx[1] = n_coords["dims"].has_child("j") ? n_coords["dims/j"].to_int() : 1;
    nx[2] = n_coords["dims"].has_child("k") ? n_coords["dims/k"].to_int() : 1;
    rectgrid->SetDimensions(nx);

    double dx[3] = {1.0,1.0,1.0};
    std::string coord_sys_type = conduit::blueprint::mesh::utils::coordset::coordsys(n_coords);

    if(n_coords.has_child("spacing"))
    {
        const Node &n_spacing = n_coords["spacing"];

        if(coord_sys_type == "cylindrical")
        {
            // note: we always treat z as horz axis for
            // RZ meshes
            if(n_spacing.has_child("dz"))
                dx[0] = n_spacing["dz"].to_double();

            if(n_spacing.has_child("dr"))
                dx[1] = n_spacing["dr"].to_double();
        }
        else // xyz
        {
            if(n_spacing.has_child("dx"))
                dx[0] = n_spacing["dx"].to_double();

            if(n_spacing.has_child("dy"))
                dx[1] = n_spacing["dy"].to_double();

            if(n_spacing.has_child("dz"))
                dx[2] = n_spacing["dz"].to_double();
        }
    }

    double x0[3] = {0.0, 0.0, 0.0};

    if(n_coords.has_child("origin"))
    {
        const Node &n_origin =  n_coords["origin"];

        if(coord_sys_type == "cylindrical")
        {
            if(n_origin.has_child("z"))
                x0[0] = n_origin["z"].to_double();

            if(n_origin.has_child("r"))
                x0[1] = n_origin["r"].to_double();
        }
        else
        {
            if(n_origin.has_child("x"))
                x0[0] = n_origin["x"].to_double();

            if(n_origin.has_child("y"))
                x0[1] = n_origin["y"].to_double();

            if(n_origin.has_child("z"))
                x0[2] = n_origin["z"].to_double();
        }
    }

    for (int i = 0; i < 3; i++)
    {
        vtkDataArray *da = NULL;
        DataType dt = DataType::c_double();

        // we have we origin, we can infer type from it
        if(coord_sys_type == "cylindrical")
        {
            if(n_coords.has_path("origin/z"))
            {
                dt = n_coords["origin"]["z"].dtype();
            }
        }
        else
        {
            if(n_coords.has_path("origin/x"))
            {
                dt = n_coords["origin"]["x"].dtype();
            }
        }

        // since vtk uses the c-native style types
        // only need to check for native types in conduit
        if (dt.is_unsigned_char())
            da = vtkUnsignedCharArray::New();
        else if (dt.is_unsigned_short())
            da = vtkUnsignedShortArray::New();
        else if (dt.is_unsigned_int())
            da = vtkUnsignedIntArray::New();
        else if (dt.is_char())
            da = vtkCharArray::New();
        else if (dt.is_short())
            da = vtkShortArray::New();
        else if (dt.is_int())
            da = vtkIntArray::New();
        else if (dt.is_long())
            da = vtkLongArray::New();
#if CONDUIT_USE_LONG_LONG
        else if (dt.id() == CONDUIT_NATIVE_LONG_LONG_ID)
            da = vtkLongLongArray::New();
#endif
        else if (dt.is_float())
            da = vtkFloatArray::New();
        else if (dt.is_double())
            da = vtkDoubleArray::New();
        else
        {
            AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException,
                                      "Conduit Blueprint to Rectilinear Grid coordinates "
                                      "unsupported data type: " << dt.name());
        }

        da->SetNumberOfTuples(nx[i]);
        double x = x0[i];
        for (int j = 0; j < nx[i]; j++, x += dx[i])
            da->SetComponent(j, 0, x);

        if (i == 0) rectgrid->SetXCoordinates(da);
        if (i == 1) rectgrid->SetYCoordinates(da);
        if (i == 2) rectgrid->SetZCoordinates(da);

        da->Delete();
    }

    return rectgrid;
}

// ****************************************************************************
vtkDataSet *
RectilinearCoordsToVTKRectilinearGrid(const Node &n_coords)
{
    AVT_CONDUIT_BP_INFO("RectilinearCoordsToVTKRectilinearGrid");
    
    vtkRectilinearGrid *rectgrid = vtkRectilinearGrid::New();

    const Node &n_coords_values  = n_coords["values"];

    int dims[3] = {1,1,1};

    std::string coord_sys_type = conduit::blueprint::mesh::utils::coordset::coordsys(n_coords);

    vtkDataArray *coords[3] = {0,0,0};

    AVT_CONDUIT_BP_INFO("coord system type: " << coord_sys_type);

    if(coord_sys_type == "cylindrical")
    {
        dims[0] = n_coords_values["z"].dtype().number_of_elements();
        dims[1] = n_coords_values["r"].dtype().number_of_elements();
        rectgrid->SetDimensions(dims);

        coords[0] = ConduitArrayToVTKDataArray(n_coords_values["z"]);
        coords[1] = ConduitArrayToVTKDataArray(n_coords_values["r"]);
        coords[2] = coords[0]->NewInstance();
        coords[2]->SetNumberOfTuples(1);
        coords[2]->SetComponent(0,0,0);
    }
    else // cartesian
    {
        dims[0] = n_coords_values["x"].dtype().number_of_elements();
        
        if (n_coords_values.has_child("y"))
            dims[1] = n_coords_values["y"].dtype().number_of_elements();
        
        if (n_coords_values.has_child("z"))
            dims[2] = n_coords_values["z"].dtype().number_of_elements();

        rectgrid->SetDimensions(dims);

        coords[0] = ConduitArrayToVTKDataArray(n_coords_values["x"]);
        
        if (n_coords_values.has_child("y"))
            coords[1] = ConduitArrayToVTKDataArray(n_coords_values["y"]);
        else
        {
            coords[1] = coords[0]->NewInstance();
            coords[1]->SetNumberOfTuples(1);
            coords[1]->SetComponent(0,0,0);
        }
        
        if (n_coords_values.has_child("z"))
            coords[2] = ConduitArrayToVTKDataArray(n_coords_values["z"]);
        else
        {
            coords[2] = coords[0]->NewInstance();
            coords[2]->SetNumberOfTuples(1);
            coords[2]->SetComponent(0,0,0);
        }
    }

    rectgrid->SetXCoordinates(coords[0]);
    rectgrid->SetYCoordinates(coords[1]);
    rectgrid->SetZCoordinates(coords[2]);

    coords[0]->Delete();
    coords[1]->Delete();
    coords[2]->Delete();

    return rectgrid;
}

// ****************************************************************************
vtkDataSet *
StructuredTopologyToVTKStructuredGrid(const Node &n_coords,
                                      const Node &n_topo)
{
    vtkStructuredGrid *sgrid = vtkStructuredGrid::New();

    int dims[3];
    dims[0] = n_topo.has_path("elements/dims/i") ? n_topo["elements/dims/i"].to_int()+1 : 1;
    dims[1] = n_topo.has_path("elements/dims/j") ? n_topo["elements/dims/j"].to_int()+1 : 1;
    dims[2] = n_topo.has_path("elements/dims/k") ? n_topo["elements/dims/k"].to_int()+1 : 1;
    sgrid->SetDimensions(dims);

    vtkPoints *points = ExplicitCoordsToVTKPoints(n_coords, n_topo);
    sgrid->SetPoints(points);
    points->Delete();

    return sgrid;
}

// ****************************************************************************
vtkDataSet *
PointsTopologyToVTKUnstructuredGrid(const Node &n_coords,
                                    const Node &n_topo)
{
    vtkPoints *points = ExplicitCoordsToVTKPoints(n_coords,n_topo);

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    points->Delete();

    int npoints = points->GetNumberOfPoints();

    //
    // Now, add explicit topology
    //
    vtkCellArray *ca = vtkCellArray::New();
    vtkIdTypeArray *ida = vtkIdTypeArray::New();

    ida->SetNumberOfTuples(npoints * 2);
    // Create cell array that ranges from 0 to n-1.
    for (int i = 0; i < npoints; i++)
    {
        ida->SetComponent(2 * i, 0, 1);
        ida->SetComponent(2 * i + 1, 0, i);
    }
    ca->SetCells(npoints, ida);
    ida->Delete();

    ugrid->SetCells(VTK_VERTEX, ca);
    ca->Delete();

    return ugrid;
}

// ****************************************************************************
//  Method: UnstructuredTopologyToVTKUnstructuredGrid
//
//  Purpose:
//   Constructs a vtkUnstructuredGrid from a Blueprint topology and coordset
//
//  Arguments:
//   n_coords:  Blueprint Coordset
//   n_topo:    Blueprint Topology
//
//  Modifications:
//    Justin Privitera Wed Mar 9 2022
//    added logic to check if topology is polyhedral or polygonal, and, if so,
//    transform it to a tetrahedral or triangular topology, respectively.
//
//    Justin Privitera Tue Mar 15 18:01:14 PDT 2022
//    now this function adds original element ids (avtOriginalCellNumbers)
//    to the vtkDataSet it returns, if applicable (if the mesh was polyhedral
//    or polygonal and was transformed; see above modification comment).
//
//    Justin Privitera Wed Mar 23 12:28:02 PDT 2022
//    Added domain as first argument, which is used for orig elem ids.
//
//    Cyrus Harrison, Mon Mar 28 12:14:20 PDT 2022
//    Use conduit version check for polytopal support.
// 
//    Justin Privitera, Mon May 23 20:28:42 PDT 2022
//    Moved the deletion of points to lower in the function.
// 
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to conduit blueprint data adaptor.
// 
//    Justin Privitera, Tue Aug 23 14:40:24 PDT 2022
//    Removed `CONDUIT_HAVE_PARTITION_FLATTEN` check.
//
//    Brad Whitlock, Wed Dec  6 11:36:10 PST 2023
//    Return actual polyhedral and polygonal zones.
//
// ****************************************************************************
vtkDataSet *
UnstructuredTopologyToVTKUnstructuredGrid(int domain,
                                          const Node &n_coords,
                                          const Node &n_topo)
{
    // Make output mesh and add points.
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    vtkPoints *points = ExplicitCoordsToVTKPoints(n_coords, n_topo);
    ugrid->SetPoints(points);
    points->Delete();

    std::string element_shape;
    if (n_topo.has_path("elements/shape"))
        element_shape = n_topo["elements/shape"].as_string();

    if(element_shape == "polyhedral")
    {
        // Cell definitions
        const auto elements_connectivity = n_topo["elements/connectivity"].as_index_t_accessor();
        const auto elements_sizes = n_topo["elements/sizes"].as_index_t_accessor();
        const auto nZones = elements_sizes.number_of_elements();

        // Face definitions
        const auto subelements_connectivity = n_topo["subelements/connectivity"].as_index_t_accessor();
        const auto subelements_sizes = n_topo["subelements/sizes"].as_index_t_accessor();
        const auto subelements_offsets = n_topo["subelements/offsets"].as_index_t_accessor();

        // Original cell numbers
        auto oca = vtkUnsignedIntArray::New();
        oca->SetName("avtOriginalCellNumbers");
        oca->SetNumberOfComponents(2);

        // Iterate the connectivity and add zones/faces.
        conduit::index_t elem_conn_index = 0;
        vtkNew<vtkIdList> faces;
        for(conduit::index_t zi = 0; zi < nZones; zi++)
        {
            conduit::index_t nZoneFaces = elements_sizes[zi];
            for(conduit::index_t zfi = 0; zfi < nZoneFaces; zfi++)
            {
                auto currentFaceId = elements_connectivity[elem_conn_index++];

                // Now, add the face to the zone.
                auto faceSize = subelements_sizes[currentFaceId];
                auto faceOffset = subelements_offsets[currentFaceId];
                faces->InsertNextId(faceSize);
                for(conduit::index_t fi = 0; fi < faceSize; fi++)
                {
                    faces->InsertNextId(subelements_connectivity[faceOffset + fi]);
                }
            }
            ugrid->InsertNextCell(VTK_POLYHEDRON, nZoneFaces, faces->GetPointer(0));
            faces->Reset();

            // Add a tuple to the original cell numbers.
            unsigned int ocdata[2] = {static_cast<unsigned int>(domain), 
                                      static_cast<unsigned int>(zi)};
            oca->InsertNextTypedTuple(ocdata);
        }

        // Add original cell numbers zones to see if that cleans up mesh plot.
        ugrid->GetCellData()->AddArray(oca);
        ugrid->GetCellData()->CopyFieldOn("avtOriginalCellNumbers");
        oca->Delete();
    }
    else if(element_shape == "polygonal")
    {
        const auto elements_sizes = n_topo["elements/sizes"].as_index_t_accessor();
        const auto emin = elements_sizes.min();
        const auto emax = elements_sizes.max();
        if(emin == emax && emin == 3)
        {
            // Add as triangles.
            vtkCellArray *ca = HomogeneousShapeTopologyToVTKCellArray(n_topo, points->GetNumberOfPoints());
            ugrid->SetCells(ElementShapeNameToVTKCellType("triangles"), ca);
            ca->Delete();
        }
        else if(emin == emax && emin == 4)
        {
            // Add as quads
            vtkCellArray *ca = HomogeneousShapeTopologyToVTKCellArray(n_topo, points->GetNumberOfPoints());
            ugrid->SetCells(ElementShapeNameToVTKCellType("quads"), ca);
            ca->Delete();
        }
        else
        {
            // Add as polygons.

            // Original cell numbers
            auto oca = vtkUnsignedIntArray::New();
            oca->SetName("avtOriginalCellNumbers");
            oca->SetNumberOfComponents(2);

            const auto elements_connectivity = n_topo["elements/connectivity"].as_index_t_accessor();
            const auto nZones = elements_sizes.number_of_elements();
            conduit::index_t elem_conn_index = 0;
            vtkNew<vtkIdList> pts;
            for(conduit::index_t zi = 0; zi < nZones; zi++)
            {
                conduit::index_t nZonePoints = elements_sizes[zi];
                for(conduit::index_t zpi = 0; zpi < nZonePoints; zpi++)
                {
                    pts->InsertNextId(elements_connectivity[elem_conn_index++]);
                }
                ugrid->InsertNextCell(VTK_POLYGON, nZonePoints, pts->GetPointer(0));
                pts->Reset();

                // Add a tuple to the original cell numbers.
                unsigned int ocdata[2] = {static_cast<unsigned int>(domain), 
                                          static_cast<unsigned int>(zi)};
                oca->InsertNextTypedTuple(ocdata);
            }

            // Add original cell numbers zones to see if that cleans up mesh plot.
            ugrid->GetCellData()->AddArray(oca);
            ugrid->GetCellData()->CopyFieldOn("avtOriginalCellNumbers");
            oca->Delete();
        }
    }
    else
    {
        //
        // Now, add explicit topology
        //
        vtkCellArray *ca = HomogeneousShapeTopologyToVTKCellArray(n_topo, points->GetNumberOfPoints());
        ugrid->SetCells(ElementShapeNameToVTKCellType(element_shape), ca);
        ca->Delete();
    }

    return ugrid;
}

// ****************************************************************************
//                             API functions
// ****************************************************************************

// ****************************************************************************
//  Method: avtConduitBlueprintDataAdaptor::BlueprintToVTK::MeshToVTK
//
//  Modifications:
//    Justin Privitera, Fri 04 Mar 2022 05:57:49 PM PST
//    Added support for points topology type; see
//    PointsTopologyToVTKUnstructuredGrid as well.
// 
//    Justin Privitera, Wed Mar 23 12:26:31 PDT 2022
//    Added "domain" as first arg of MeshToVTK and passed it to
//    UnstructuredTopologyToVTKUnstructuredGrid.
// 
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to conduit blueprint data adaptor.
//
// ****************************************************************************
vtkDataSet *
avtConduitBlueprintDataAdaptor::BlueprintToVTK::MeshToVTK(int domain,
                                                          const Node &n_mesh)
{
    //NOTE: this assumes one coordset and one topo
    // that is the case for the blueprint plugin, but may not be the case
    // generally if we want to reuse this code.
    AVT_CONDUIT_BP_INFO("BlueprintVTK::MeshToVTKDataSet Begin");

    const Node &n_coords = n_mesh["coordsets"][0];
    const Node &n_topo   = n_mesh["topologies"][0];

    vtkDataSet *res = NULL;

    if (n_coords["type"].as_string() == "uniform")
    {
        AVT_CONDUIT_BP_INFO("BlueprintVTK::MeshToVTKDataSet UniformCoordsToVTKRectilinearGrid");
        res = UniformCoordsToVTKRectilinearGrid(n_coords);
    }
    else if (n_coords["type"].as_string() == "rectilinear")
    {
        AVT_CONDUIT_BP_INFO("BlueprintVTK::MeshToVTKDataSet RectilinearCoordsToVTKRectilinearGrid");
        res = RectilinearCoordsToVTKRectilinearGrid(n_coords);
    }
    else if (n_coords["type"].as_string() == "explicit")
    {
        if (n_topo["type"].as_string() == "structured")
        {
            AVT_CONDUIT_BP_INFO("BlueprintVTK::MeshToVTKDataSet StructuredTopologyToVTKStructuredGrid");
            res = StructuredTopologyToVTKStructuredGrid(n_coords, n_topo);
        }
        else if (n_topo["type"].as_string() == "points")
        {
            AVT_CONDUIT_BP_INFO("BlueprintVTK::MeshToVTKDataSet PointsTopologyToVTKUnstructuredGrid");
            res = PointsTopologyToVTKUnstructuredGrid(n_coords, n_topo);
        }
        else
        {
            AVT_CONDUIT_BP_INFO("BlueprintVTK::MeshToVTKDataSet UnstructuredTopologyToVTKUnstructuredGrid");
            res = UnstructuredTopologyToVTKUnstructuredGrid(domain, n_coords, n_topo);
        }
    }
    else
    {
        AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException,
                                  "expected Coordinate type of \"uniform\", \"rectilinear\", or \"explicit\""
                                  << " but found " << n_coords["type"].as_string());
    }

    AVT_CONDUIT_BP_INFO("BlueprintVTK::MeshToVTKDataSet End");

    return res;
}

// ****************************************************************************
vtkDataArray *
avtConduitBlueprintDataAdaptor::BlueprintToVTK::FieldToVTK(
        const conduit::Node &topo,
        const conduit::Node &field)
{
    vtkDataArray * res = nullptr;
    // Handle optional offsets and strides ()
    //
    // only support offsets, strides for fields on  "structured" topos
    //
    // NOTE: we could also support this uniform or rectilinear
    // but we need more / different logic to detect the logical
    // point dims from the coordset
    //
    if( topo["type"].as_string() == "structured" && 
        (field.has_path("offsets") || field.has_path("strides")) )
    {
        AVT_CONDUIT_BP_INFO("FieldToVTK:: structured strided case: ");
        
        bool ele_assoc = false;

        if ( field["association"].as_string() == "element")
        {
             ele_assoc = true;
        }

        int ele_shape[3] = {0,0,0};
        int pts_shape[3] = {0,0,0};
        // to walk the values in the correct order and
        // flatten to vtk, we need the topology and points shapes
        StructuredTopologyShape(topo,ele_shape);

        pts_shape[0] = ele_shape[0]+1;
        pts_shape[1] = ele_shape[1]+1;
        pts_shape[2] = ele_shape[2]+1;

        int strides[3] = {0,0,0};
        int offsets[3] = {0,0,0};

        if(field.has_path("offsets"))
        {
            int_accessor off_vals = field["offsets"].value();
            for(int i=0;i<off_vals.dtype().number_of_elements();i++)
            {
                offsets[i] = off_vals[i];
            }
        }
        // Note: default offsets values should be {0,0,0}

        if(field.has_path("strides"))
        {
            int_accessor stride_vals = field["strides"].value();
            for(int i=0;i<stride_vals.dtype().number_of_elements();i++)
            {
                strides[i] = stride_vals[i];
            }
        }
        else // default strides
        {
            // we need to know if field is vertex or ele assoced
            // to calc proper default strides
            if(ele_assoc)
            {
                strides[0] = 1;
                strides[1] = ele_shape[0];
                strides[2] = ele_shape[1];
            }
            else
            {
                strides[0] = 1;
                strides[1] = pts_shape[0];
                strides[2] = pts_shape[1];
            }
        }
        
        int num_tuples = 0;
        if(ele_assoc)
        {
            num_tuples = ele_shape[0];
            if(ele_shape[1] > 0)
            {
                num_tuples *= ele_shape[1];
            }
            if(ele_shape[2] > 0)
            {
                num_tuples *= ele_shape[2];
            }
        }
        else
        {
            num_tuples = pts_shape[0] * pts_shape[1] * pts_shape[2];
        }


        int *src_shape = NULL;
        if(ele_assoc)
        {
            src_shape = ele_shape;
        }
        else
        {
            src_shape = pts_shape;
        }

        AVT_CONDUIT_BP_INFO("FieldToVTK:: number of tuples: "
                            << num_tuples << std::endl);


        // build indirection array, which will be used
        // to walk the source (conduit data) and put into vtk
        Node src_idxs;
        src_idxs.set(DataType::c_int(num_tuples));
        int *src_idxs_vals = src_idxs.value();
        int idx =0;
        
        
        AVT_CONDUIT_BP_INFO("FieldToVTK:: structured shape: "
                            << src_shape[0] << " "
                            << src_shape[1] << " "
                            << src_shape[2] );

        AVT_CONDUIT_BP_INFO("FieldToVTK:: structured offsets: "
                            << offsets[0] << " "
                            << offsets[1] << " "
                            << offsets[2] );

        AVT_CONDUIT_BP_INFO("FieldToVTK:: structured strides: "
                            << strides[0] << " "
                            << strides[1] << " "
                            << strides[2] );

        for(int k=0; k ==0 || k < src_shape[2]; k++)
        {
            for(int j=0; j==0 || j < src_shape[1]; j++)
            {
                for(int i=0; i < src_shape[0]; i++)
                {
                    src_idxs_vals[idx] = NDIndex(i,j,k,offsets,strides);
                    idx++;
                }
            }
        }

        res = ConduitArrayToVTKDataArray(field["values"],num_tuples,src_idxs_vals);
    }
    else
    {
        res = ConduitArrayToVTKDataArray(field["values"]);
    }
    return res;
}

// ****************************************************************************
vtkDataSet *
avtConduitBlueprintDataAdaptor::BlueprintToVTK::Curve1DToVTK(
        const conduit::Node &coords,
        const conduit::Node &field)
{
    bool elem_assoc = field.has_child("association") && 
        field["association"].as_string() == "element";

    int num_field_vals = field["values"].dtype().number_of_elements();
    if (elem_assoc)
        num_field_vals *= 2;

    vtkRectilinearGrid *rgrid = vtkVisItUtility::Create1DRGrid(num_field_vals, VTK_DOUBLE);
    vtkDoubleArray *vals = vtkDoubleArray::New();
    vals->SetNumberOfComponents(1);
    vals->SetNumberOfTuples(num_field_vals);
    vals->SetName("curve");
    rgrid->GetPointData()->SetScalars(vals);

    vtkDataArray *xs = rgrid->GetXCoordinates();

    double_accessor x_vals = coords["values"][0].value();
    double_accessor y_vals = field["values"].value();

    if (elem_assoc)
    {
        for (vtkIdType i = 0; i < num_field_vals / 2; i ++)
        {
            xs->SetComponent(i * 2, 0, (double) x_vals[i]);
            xs->SetComponent(i * 2 + 1, 0, (double) x_vals[i + 1]);
            vals->SetComponent(i * 2, 0, (double) y_vals[i]);
            vals->SetComponent(i * 2 + 1, 0, (double) y_vals[i]);
        }
    }
    else
    {
        for (vtkIdType i = 0; i < num_field_vals; i ++)
        {
            xs->SetComponent(i, 0, (double) x_vals[i]);
            vals->SetComponent(i, 0, (double) y_vals[i]);
        }
    }

    vals->Delete();

    return rgrid;
}

// ****************************************************************************
// ****************************************************************************
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//                               VTK --> BLUEPRINT
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
//                             Helper functions
// ****************************************************************************

void
CopyComponent64(Node &node, vtkDataArray *da, int component)
{

    int nvals= da->GetNumberOfTuples();
    node.set(DataType::float64(nvals));
    conduit::float64 *vals = node.value();
    for(int i = 0; i < nvals; ++i)
    {
      vals[i] = da->GetComponent(i, component);
    }
}

// ****************************************************************************
void
CopyComponent32(Node &node, vtkDataArray *da, int component)
{

    int nvals= da->GetNumberOfTuples();
    node.set(DataType::float32(nvals));
    conduit::float32 *vals = node.value();
    for(int i = 0; i < nvals; ++i)
    {
      vals[i] = (float)da->GetComponent(i, component);
    }
}

// ****************************************************************************
//  Method: VTKDataArrayToNode
//
//  Purpose:
//      Wraps VTK data as a Conduit field.
//
//  Programmer: Matt Larsen
//  Creation:   2019-02-25
//
//  Modifications:
//
//  Brad Whitlock, Fri Apr  1 13:41:32 PDT 2022
//  Treat scalars specially so we do not make mcarrays out of them.
// 
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to conduit blueprint data adaptor.
//
// ****************************************************************************

void
VTKDataArrayToNode(Node &node, vtkDataArray *arr)
{
    int ncomps = arr->GetNumberOfComponents();

    if(ncomps == 1)
    {
        if(arr->GetDataType() == VTK_DOUBLE)
            CopyComponent64(node, arr, 0);
        else
            CopyComponent32(node, arr, 0);
    }
    else
    {
        for(int i = 0; i < ncomps; ++i)
        {
            std::stringstream ss;
            ss<<"/c"<<i;

            if(arr->GetDataType() == VTK_DOUBLE)
            {
                CopyComponent64(node[ss.str()], arr, i);
            }
            else
            {
                CopyComponent32(node[ss.str()], arr, i);
            }
        }
    }
}

// ****************************************************************************
void
CopyTuple1(Node &node, vtkDataArray *da)
{

    int nvals= da->GetNumberOfTuples();
    node.set(DataType::float64(nvals));
    conduit::float64 *vals = node.value();
    for(int i = 0; i < nvals; ++i)
    {
      vals[i] = da->GetTuple1(i);
    }
}

// ****************************************************************************
void vtkPointsToNode(Node &node, vtkPoints *points, const int dims)
{

  const int num_points = points->GetNumberOfPoints();
  if(points->GetDataType() == VTK_FLOAT)
  {
    float *vtk_ptr = (float *) points->GetVoidPointer(0);
    index_t stride = sizeof(conduit::float32) * 3;
    index_t size = sizeof(conduit::float32);
    node["x"].set_external(DataType::float32(num_points,0,stride), vtk_ptr);
    if(dims > 1)
    {
      node["y"].set_external(DataType::float32(num_points,size,stride), vtk_ptr);
    }
    if(dims > 2)
    {
      node["z"].set_external(DataType::float32(num_points,size*2,stride), vtk_ptr);
    }
  }
  else if(points->GetDataType() == VTK_DOUBLE)
  {
    double *vtk_ptr = (double *) points->GetVoidPointer(0);
    index_t stride = sizeof(conduit::float64) * 3;
    index_t size = sizeof(conduit::float64);
    node["x"].set_external(DataType::float64(num_points,0,stride), vtk_ptr);
    if(dims > 1)
    {
      node["y"].set_external(DataType::float64(num_points,size,stride), vtk_ptr);
    }
    if(dims > 2)
    {
      node["z"].set_external(DataType::float64(num_points,size*2,stride), vtk_ptr);
    }
  }
}

// ****************************************************************************
void vtkUnstructuredToNode(Node &node, 
                           vtkUnstructuredGrid *grid, 
                           const int dims)
{
  const int npts = grid->GetNumberOfPoints();
  const int nzones = grid->GetNumberOfCells();

  if(nzones == 0)
  {
    AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException,
                              "Unstructured grids must have at least one "
                              "zone");
  }

  bool single_shape = true;
  int cell_type = grid->GetCell(0)->GetCellType();
  for(int i = 1; i < nzones; ++i)
  {
    vtkCell *cell = grid->GetCell(i);
    if(cell->GetCellType() != cell_type)
    {
      single_shape = false;
      break;
    }
  }

  if(!single_shape)
  {
    AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException,
                              "Only unstructured grids with a single cell type"
                              " are currently supported");
  }

  const std::string shape_name = VTKCellTypeToElementShapeName(cell_type);
  if(shape_name == "")
  {
    AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException,
                              "Unsupported vtk cell type "<<cell_type);
  }

  node["shape"] = shape_name;

  const int cell_points = grid->GetCell(0)->GetNumberOfPoints();
  node["connectivity"].set(DataType::int32(cell_points * nzones));
  conduit::int32 *conn = node["connectivity"].value();
  // vtk connectivity is in the form npts, p0, p1,..
  // and we need p0, p1, .. so just iterate and copy
  if(cell_type != VTK_VOXEL)
  {
    for(int i = 0; i < nzones; ++i)
    {
      vtkCell *cell = grid->GetCell(i);
      const int offset = i * cell_points;
      for(int c = 0; c < cell_points; ++c)
      {
        conn[offset + c]  = cell->GetPointId(c);
      }
    }
  }
  else
  {
    // We need to reorder the voxel indices to be a hex
    int reorder[8] = {0, 1, 3, 2, 4, 5, 7, 6};
    for(int i = 0; i < nzones; ++i)
    {
      vtkCell *cell = grid->GetCell(i);
      const int offset = i * cell_points;
      for(int c = 0; c < cell_points; ++c)
      {
        int index = reorder[c];
        conn[offset + index]  = cell->GetPointId(c);
      }
    }

  }
}


// ****************************************************************************
void vtkPolyDataToNode(Node &node, 
                       vtkPolyData *grid,
                       int /*ndims*/)
{
  const int npts = grid->GetNumberOfPoints();
  const int nzones = grid->GetNumberOfCells();

  if(nzones == 0)
  {
    AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException,
                              "PolyData grids must have at least one "
                              "zone");
  }

  bool single_shape = true;
  int cell_type = grid->GetCell(0)->GetCellType();
  for(int i = 1; i < nzones; ++i)
  {
    vtkCell *cell = grid->GetCell(i);
    if(cell->GetCellType() != cell_type)
    {
      single_shape = false;
      break;
    }
  }

  if(!single_shape)
  {
    AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException,
                              "Only PolyData grids with a single cell type"
                              " are currently supported");
  }

  const std::string shape_name = VTKCellTypeToElementShapeName(cell_type);
  if(shape_name == "")
  {
    AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException,
                              "Unsupported vtk cell type "<<cell_type);
  }

  node["shape"] = shape_name;

  const int cell_points = grid->GetCell(0)->GetNumberOfPoints();
  node["connectivity"].set(DataType::int32(cell_points * nzones));
  conduit::int32 *conn = node["connectivity"].value();
  // vtk connectivity is in the form npts, p0, p1,..
  // and we need p0, p1, .. so just iterate and copy
  if(cell_type != VTK_VOXEL)
  {
    for(int i = 0; i < nzones; ++i)
    {
      vtkCell *cell = grid->GetCell(i);
      const int offset = i * cell_points;
      for(int c = 0; c < cell_points; ++c)
      {
        conn[offset + c]  = cell->GetPointId(c);
      }
    }
  }
  else
  {
    // We need to reorder the voxel indices to be a hex
    int reorder[8] = {0, 1, 3, 2, 4, 5, 7, 6};
    for(int i = 0; i < nzones; ++i)
    {
      vtkCell *cell = grid->GetCell(i);
      const int offset = i * cell_points;
      for(int c = 0; c < cell_points; ++c)
      {
        int index = reorder[c];
        conn[offset + index]  = cell->GetPointId(c);
      }
    }

  }
}


// ****************************************************************************
//                             API functions
// ****************************************************************************

// ****************************************************************************
//  Method: avtConduitBlueprintDataAdaptor::VTKToBlueprint::VTKFieldNameToBlueprint
//
//  Purpose:
//      Replaces '/''s in input vtk_name with '_''s and stores the output in
//      bp_name. If the field name is "mesh_topo_name/name" then the
//      entire "mesh_topo_name/" is removed from the output.
//
//  Programmer: Chris Laganella
//  Creation:   Fri Nov  5 16:35:09 EDT 2021
//
//  Modifications:
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to conduit blueprint data adaptor.
//
// ****************************************************************************
void avtConduitBlueprintDataAdaptor::VTKToBlueprint::VTKFieldNameToBlueprint(
    const std::string &vtk_name,
    const std::string &topo_name,
    std::string &bp_name)
{
    bp_name = vtk_name;
    int first = bp_name.find('/');
    if(first == bp_name.npos)
    {
        return;
    }

    const std::string mesh_topo = "mesh_" + topo_name;
    if(bp_name.substr(0, first) == mesh_topo)
    {
        bp_name = bp_name.substr(first+1);
    }

    for(char &c : bp_name)
    {
        if(c == '/')
        {
            c = '_';
        }
    }
}

// ****************************************************************************
//  Method: avtConduitBlueprintDataAdaptor::VTKToBlueprint::VTKFieldsToBlueprint
//
//  Purpose:
//      Takes a vtk data set and converts all the fields into blueprint nodes.
//      node is the mesh["fields"] node in the blueprint dataset
//
//  Programmer: Matt Larsen
//  Creation:   Feb 15, 2019
//
//  Modifications:
//
//  Chris Laganella Fri Nov  5 16:51:15 EDT 2021
//  Updated to use VTKFieldNameToBlueprint
// 
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to conduit blueprint data adaptor.
// ****************************************************************************
void avtConduitBlueprintDataAdaptor::VTKToBlueprint::VTKFieldsToBlueprint(
    conduit::Node &node,
    const std::string topo_name,
    vtkDataSet* dataset)
{
  vtkPointData *pd = dataset->GetPointData();
  vtkCellData *cd  = dataset->GetCellData();

  if(pd != NULL)
  {
    for (size_t i = 0 ; i < (size_t)pd->GetNumberOfArrays() ; i++)
    {
         vtkDataArray *arr = pd->GetArray(i);

         // skip special variables
         if (strstr(arr->GetName(), "vtk") != NULL)
             continue;
         // keep fields like avtGhostZones
         std::string fname;
         VTKFieldNameToBlueprint(arr->GetName(), topo_name, fname);
         std::string field_path = "fields/" + fname;
         node[field_path + "/association"] = "vertex";
         node[field_path + "/topology"] = topo_name;

         VTKDataArrayToNode(node[field_path + "/values"], arr);
         AVT_CONDUIT_BP_INFO("VTKBlueprint:: converted point field "<<fname);
    }
  }

  if(cd != NULL)
  {
    for (size_t i = 0 ; i < (size_t)cd->GetNumberOfArrays() ; i++)
    {
         vtkDataArray *arr = cd->GetArray(i);
         // skip special variables
         if (strstr(arr->GetName(), "vtk") != NULL)
             continue;
         // keep fields like avtGhostZones
         std::string fname;
         VTKFieldNameToBlueprint(arr->GetName(), topo_name, fname);
         std::string field_path = "fields/" + fname;
         node[field_path + "/association"] = "element";
         node[field_path + "/topology"] = topo_name;

         AVT_CONDUIT_BP_INFO("VTKBlueprint:: converted cell field "<<fname);
         VTKDataArrayToNode(node[field_path + "/values"], arr);
    }
  }
}

// ****************************************************************************
//  Method: avtConduitBlueprintDataAdaptor::VTKToBlueprint::VTKToBlueprintMesh
//
//  Purpose:
//      Takes a vtk data set and converts into a  blueprint mesh.
//
//  Programmer: Matt Larsen
//  Creation:   Feb 15, 2019
//
//  Modifications:
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to conduit blueprint data adaptor.
//
//    Cyrus Harrison, Tue Dec  6 10:31:54 PST 2022
//    Add support to convert VTK Poly Data

// ****************************************************************************
void
avtConduitBlueprintDataAdaptor::VTKToBlueprint::VTKToBlueprintMesh(
    conduit::Node &mesh,
    vtkDataSet* dataset,
    const int ndims)
{
    std::string coord_path = "coordsets/coords";
    std::string topo_name = "topo";
    std::string topo_path = "topologies/" + topo_name;
    mesh[topo_path + "/coordset"] = "coords";

    if (dataset->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        AVT_CONDUIT_BP_INFO("VTKToBlueprint:: Rectilinear");
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) dataset;

        mesh[coord_path+ "/type"] = "rectilinear";
        mesh[topo_path + "/type"] = "rectilinear";

        if(ndims > 0)
        {
            int dimx = rgrid->GetXCoordinates()->GetNumberOfTuples();
            CopyTuple1(mesh[coord_path+ "/values/x"], rgrid->GetXCoordinates());
            mesh[topo_path+ "/elements/dims/i"] = dimx;
        }
        if(ndims > 1)
        {
            int dimy = rgrid->GetYCoordinates()->GetNumberOfTuples();
            CopyTuple1(mesh[coord_path + "/values/y"], rgrid->GetYCoordinates());
            mesh[topo_path+ "/elements/dims/j"] = dimy;
        }
        if(ndims > 2)
        {
            int dimz = rgrid->GetZCoordinates()->GetNumberOfTuples();
            CopyTuple1(mesh[coord_path + "/values/z"], rgrid->GetZCoordinates());
            mesh[topo_path+ "/elements/dims/k"] = dimz;
        }
    }
    else if (dataset->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        AVT_CONDUIT_BP_INFO("VTKToBlueprint:: StructuredGrid");
        vtkStructuredGrid *grid = (vtkStructuredGrid *) dataset;

        mesh[coord_path + "/type"] = "explicit";
        mesh[topo_path + "/type"] = "structured";

        vtkPoints *vtk_pts = grid->GetPoints();
        vtkPointsToNode(mesh[coord_path + "/values"], vtk_pts, ndims);

        int dims[3];
        grid->GetCellDims(dims);
        AVT_CONDUIT_BP_INFO("VTKBlueprint:: StructuredGrid4");
        if(ndims > 0)
        {
            mesh[topo_path+ "/elements/dims/i"] = dims[0];
        }
        if(ndims > 1)
        {
            mesh[topo_path+ "/elements/dims/j"] = dims[1];
        }
        if(ndims > 2)
        {
            mesh[topo_path+ "/elements/dims/k"] = dims[2];
        }
    }
    else if (dataset->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        AVT_CONDUIT_BP_INFO("VTKToBlueprint:: UnstructuredGrid");
        vtkUnstructuredGrid *grid = (vtkUnstructuredGrid *) dataset;

        mesh[coord_path + "/type"] = "explicit";
        mesh[topo_path + "/type"] = "unstructured";

        vtkPoints *vtk_pts = grid->GetPoints();
        vtkPointsToNode(mesh[coord_path + "/values"], vtk_pts, ndims);
        vtkUnstructuredToNode(mesh[topo_path + "/elements"], grid, ndims);
    }
    else if(dataset->GetDataObjectType() == VTK_POLY_DATA)
    {
        AVT_CONDUIT_BP_INFO("VTKToBlueprint:: PolyData");
        vtkPolyData *grid = (vtkPolyData *) dataset;

        mesh[coord_path + "/type"] = "explicit";
        mesh[topo_path + "/type"] = "unstructured";

        vtkPoints *vtk_pts = grid->GetPoints();
        vtkPointsToNode(mesh[coord_path + "/values"], vtk_pts, ndims);
        vtkPolyDataToNode(mesh[topo_path + "/elements"], grid, ndims);
    }
    else
    {
        AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException,
                                  "Unsupported VTK Data Set type " << 
                                  dataset->GetDataObjectType() );
    }

    avtConduitBlueprintDataAdaptor::VTKToBlueprint::VTKFieldsToBlueprint(mesh,
                                                                    topo_name,
                                                                    dataset);

}

// ****************************************************************************
// ****************************************************************************
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//                               MFEM --> BLUEPRINT
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
//                             Helper functions
// ****************************************************************************

// ****************************************************************************
int
ConduitElementShapeSize(const std::string &shape_name)
{
    int res = 0;
    if (shape_name == "point")      res = 1;
    else if (shape_name == "line")  res = 2;
    else if (shape_name == "tri")   res = 3;
    else if (shape_name == "quad")  res = 4;
    else if (shape_name == "hex")   res = 8;
    else if (shape_name == "tet")   res = 4;
    return res;
}

// ****************************************************************************
mfem::Geometry::Type
ElementShapeNameToMFEMShape(const std::string &shape_name)
{
    // init to somethign to avoid invalid memory access
    // in the mfem mesh constructor
    mfem::Geometry::Type res = mfem::Geometry::POINT;
    if(shape_name == "point")
        res = mfem::Geometry::POINT;
    else if(shape_name == "line")
        res =  mfem::Geometry::SEGMENT;
    else if(shape_name == "tri")
        res =  mfem::Geometry::TRIANGLE;
    else if(shape_name == "quad")
        res =  mfem::Geometry::SQUARE;
    else if(shape_name == "tet")
        res =  mfem::Geometry::TETRAHEDRON;
    else if(shape_name == "hex")
        res =  mfem::Geometry::CUBE;
    else
        AVT_CONDUIT_BP_WARNING("Unsupported Element Shape: " << shape_name);

    return res;
}


//---------------------------------------------------------------------------//
mfem::Geometry::Type
ShapeNameToGeomType(const std::string &shape_name)
{
   // Note: must init to something to avoid invalid memory access
   // in the mfem mesh constructor
   mfem::Geometry::Type res = mfem::Geometry::POINT;

   if (shape_name == "point")
      res = mfem::Geometry::POINT;
   else if (shape_name == "line")
      res =  mfem::Geometry::SEGMENT;
   else if (shape_name == "tri")
      res =  mfem::Geometry::TRIANGLE;
   else if (shape_name == "quad")
      res =  mfem::Geometry::SQUARE;
   else if (shape_name == "tet")
      res =  mfem::Geometry::TETRAHEDRON;
   else if (shape_name == "hex")
      res =  mfem::Geometry::CUBE;
   else
       AVT_CONDUIT_BP_WARNING("Unsupported Element Shape: " << shape_name);

   return res;
}

// ****************************************************************************
//                             API functions
// ****************************************************************************

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
// TODO:
// In the future: these methods will be in MFEM's ConduitDataCollection
// we will those, instead of VisIt's own implementation.
//---------------------------------------------------------------------------//
mfem::Mesh *
avtConduitBlueprintDataAdaptor::BlueprintToMFEM::MeshToMFEM(
    const Node &n_mesh,
    const std::string &topology_name)
{
   bool zero_copy = true;
   // n_conv holds converted data (when necessary for mfem api)
   // if n_conv is used ( !n_conv.dtype().empty() ) we
   // now that some data allocation was necessary, so we
   // can't return a mesh that zero copies the conduit data
   Node n_conv;

   // we need to find the topology and its coordset.
   //

   std::string topo_name = topology_name;
   // if topo name is not set, look for first topology
   if (topo_name == "")
   {
       topo_name = n_mesh["topologies"].schema().child_name(0);
   }

   if(!n_mesh.has_path("topologies/" + topo_name))
   {
       AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException,
                            "Expected topology named \""
                            << topo_name << "\" "
                            "(node is missing path \"topologies/"
                            << topo_name << "\")");
   }

   // find coord set

   std::string coords_name = n_mesh["topologies"][topo_name]["coordset"].as_string();


   if(!n_mesh.has_path("coordsets/" + coords_name))
   {
       AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException,
                            "Expected coordinate set named \""
                            << coords_name << "\" "
                            << "(node is missing path \"coordsets/"
                            << coords_name << "\")")
   }

   const Node &n_coordset = n_mesh["coordsets"][coords_name];
   const Node &n_coordset_vals = n_coordset["values"];

   // get the number of dims of the coordset
   int ndims = n_coordset_vals.number_of_children();

   // get the number of points
   int num_verts = n_coordset_vals[0].dtype().number_of_elements();
   // get vals for points
   const double *verts_ptr = NULL;

   // the mfem mesh constructor needs coords with interleaved (aos) type
   // ordering, even for 1d + 2d we always need 3 doubles b/c it uses
   // Array<Vertex> and Vertex is a pod of 3 doubles. we check for this
   // case, if we don't have it we convert the data

   if (ndims == 3 &&
       n_coordset_vals[0].dtype().is_double() &&
       blueprint::mcarray::is_interleaved(n_coordset_vals) )
   {
      // already interleaved mcarray of 3 doubles,
      // return ptr to beginning
      verts_ptr = n_coordset_vals[0].value();
   }
   else
   {
      Node n_tmp;
      // check all vals, if we don't have doubles convert
      // to doubles
      NodeConstIterator itr = n_coordset_vals.children();
      while (itr.has_next())
      {
         const Node &c_vals = itr.next();
         std::string c_name = itr.name();

         if ( c_vals.dtype().is_double() )
         {
            // zero copy current coords
            n_tmp[c_name].set_external(c_vals);

         }
         else
         {
            // convert
            c_vals.to_double_array(n_tmp[c_name]);
         }
      }

      // check if we need to add extra dims to get
      // proper interleaved array
      if (ndims < 3)
      {
         // add dummy z
         n_tmp["z"].set(DataType::c_double(num_verts));
      }

      if (ndims < 2)
      {
         // add dummy y
         n_tmp["y"].set(DataType::c_double(num_verts));
      }

      Node &n_conv_coords_vals = n_conv["coordsets"][coords_name]["values"];
      blueprint::mcarray::to_interleaved(n_tmp,
                                         n_conv_coords_vals);
      verts_ptr = n_conv_coords_vals[0].value();
   }


   const Node &n_mesh_topo    = n_mesh["topologies"][topo_name];
   std::string mesh_ele_shape = n_mesh_topo["elements/shape"].as_string();

   mfem::Geometry::Type mesh_geo = ShapeNameToGeomType(mesh_ele_shape);
   int num_idxs_per_ele = Geometry::NumVerts[mesh_geo];

   const Node &n_mesh_conn = n_mesh_topo["elements/connectivity"];

   const int *elem_indices = NULL;
   // mfem requires ints, we could have int64s, etc convert if necessary
   if (n_mesh_conn.dtype().is_int() &&
       n_mesh_conn.is_compact() )
   {
      elem_indices = n_mesh_topo["elements/connectivity"].value();
   }
   else
   {
      Node &n_mesh_conn_conv= n_conv["topologies"][topo_name]["elements/connectivity"];
      n_mesh_conn.to_int_array(n_mesh_conn_conv);
      elem_indices = n_mesh_conn_conv.value();
   }

   int num_mesh_ele        =
      n_mesh_topo["elements/connectivity"].dtype().number_of_elements();
   num_mesh_ele            = num_mesh_ele / num_idxs_per_ele;


   const int *bndry_indices = NULL;
   int num_bndry_ele        = 0;
   // init to something b/c the mesh constructor will use this for a
   // table lookup, even if we don't have boundary info.
   mfem::Geometry::Type bndry_geo = mfem::Geometry::POINT;

   if ( n_mesh_topo.has_child("boundary_topology") )
   {
      std::string bndry_topo_name = n_mesh_topo["boundary_topology"].as_string();

      // We encountered a case were a mesh specified the boundary
      // topology, but the boundary topology was was omitted from the blueprint
      // index, so it's data could not be obtained.
      //
      // This guard prevents an error in that case, allowing the mesh to be
      // created without boundary info

      if(n_mesh["topologies"].has_child(bndry_topo_name))
      {

         const Node &n_bndry_topo    = n_mesh["topologies"][bndry_topo_name];
         std::string bndry_ele_shape = n_bndry_topo["elements/shape"].as_string();

         bndry_geo = ShapeNameToGeomType(bndry_ele_shape);
         int num_idxs_per_bndry_ele = Geometry::NumVerts[mesh_geo];

         const Node &n_bndry_conn = n_bndry_topo["elements/connectivity"];

         // mfem requires ints, we could have int64s, etc convert if necessary
         if ( n_bndry_conn.dtype().is_int() &&
              n_bndry_conn.is_compact())
         {
            bndry_indices = n_bndry_conn.value();
         }
         else
         {
            Node &(n_bndry_conn_conv) = n_conv["topologies"][bndry_topo_name]["elements/connectivity"];
            n_bndry_conn.to_int_array(n_bndry_conn_conv);
            bndry_indices = (n_bndry_conn_conv).value();

         }

         num_bndry_ele =
            n_bndry_topo["elements/connectivity"].dtype().number_of_elements();
         num_bndry_ele = num_bndry_ele / num_idxs_per_bndry_ele;
      }
   }
   else
   {
      // Skipping Boundary Element Data
   }

   const int *mesh_atts  = NULL;
   const int *bndry_atts = NULL;

   int num_mesh_atts_entires = 0;
   int num_bndry_atts_entires = 0;

   // the attribute fields could have several names
   // for the element attributes check for first occurrence of field with
   // name containing "_attribute", that doesn't contain "boundary"
   std::string main_att_name = "";

   const Node &n_fields = n_mesh["fields"];
   NodeConstIterator itr = n_fields.children();

   while ( itr.has_next() && main_att_name == "" )
   {
      itr.next();
      std::string fld_name = itr.name();
      if ( fld_name.find("boundary")   == std::string::npos &&
           fld_name.find("_attribute") != std::string::npos )
      {
         main_att_name = fld_name;
      }
   }

   if ( main_att_name != "" )
   {
      const Node &n_mesh_atts_vals = n_fields[main_att_name]["values"];

      // mfem requires ints, we could have int64s, etc convert if necessary
      if (n_mesh_atts_vals.dtype().is_int() &&
          n_mesh_atts_vals.is_compact() )
      {
         mesh_atts = n_mesh_atts_vals.value();
      }
      else
      {
         Node &n_mesh_atts_vals_conv = n_conv["fields"][main_att_name]["values"];
         n_mesh_atts_vals.to_int_array(n_mesh_atts_vals_conv);
         mesh_atts = n_mesh_atts_vals_conv.value();
      }

      num_mesh_atts_entires = n_mesh_atts_vals.dtype().number_of_elements();
   }
   else
   {
      // Skipping Mesh Attribute Data
   }

   // for the boundary attributes check for first occurrence of field with
   // name containing "_attribute", that also contains "boundary"
   std::string bnd_att_name = "";
   itr = n_fields.children();

   while ( itr.has_next() && bnd_att_name == "" )
   {
      itr.next();
      std::string fld_name = itr.name();
      if ( fld_name.find("boundary")   != std::string::npos &&
           fld_name.find("_attribute") != std::string::npos )
      {
         bnd_att_name = fld_name;
      }
   }

   if ( bnd_att_name != "" )
   {
      // Info: "Getting Boundary Attribute Data"
      const Node &n_bndry_atts_vals =n_fields[bnd_att_name]["values"];

      // mfem requires ints, we could have int64s, etc convert if necessary
      if ( n_bndry_atts_vals.dtype().is_int() &&
           n_bndry_atts_vals.is_compact())
      {
         bndry_atts = n_bndry_atts_vals.value();
      }
      else
      {
         Node &n_bndry_atts_vals_conv = n_conv["fields"][bnd_att_name]["values"];
         n_bndry_atts_vals.to_int_array(n_bndry_atts_vals_conv);
         bndry_atts = n_bndry_atts_vals_conv.value();
      }

      num_bndry_atts_entires = n_bndry_atts_vals.dtype().number_of_elements();

   }
   else
   {
      // Skipping Boundary Attribute Data
   }

   // Info: "Number of Vertices: " << num_verts  << endl
   //         << "Number of Mesh Elements: "    << num_mesh_ele   << endl
   //         << "Number of Boundary Elements: " << num_bndry_ele  << endl
   //         << "Number of Mesh Attribute Entries: "
   //         << num_mesh_atts_entires << endl
   //         << "Number of Boundary Attribute Entries: "
   //         << num_bndry_atts_entires << endl);

   // Construct MFEM Mesh Object with externally owned data
   // Note: if we don't have a gf, we need to provide the proper space dim
   //       if nodes gf is attached later, it resets the space dim based
   //       on the gf's fes.
   Mesh *mesh = new Mesh(// from coordset
      const_cast<double*>(verts_ptr),
      num_verts,
      // from topology
      const_cast<int*>(elem_indices),
      mesh_geo,
      // from mesh_attribute field
      const_cast<int*>(mesh_atts),
      num_mesh_ele,
      // from boundary topology
      const_cast<int*>(bndry_indices),
      bndry_geo,
      // from boundary_attribute field
      const_cast<int*>(bndry_atts),
      num_bndry_ele,
      ndims, // dim
      ndims); // space dim

   // Attach Nodes Grid Function, if it exists
   if (n_mesh_topo.has_child("grid_function"))
   {
      std::string nodes_gf_name = n_mesh_topo["grid_function"].as_string();

      // fetch blueprint field for the nodes gf
      const Node &n_mesh_gf = n_mesh["fields"][nodes_gf_name];
      // create gf
      mfem::GridFunction *nodes = FieldToMFEM(mesh,
                                              n_mesh_gf);
      // attach to mesh
      mesh->NewNodes(*nodes,true);
   }


   if (zero_copy && !n_conv.dtype().is_empty())
   {
      //Info: "Cannot zero-copy since data conversions were necessary"
      zero_copy = false;
   }

   Mesh *res = NULL;

   if (zero_copy)
   {
      res = mesh;
   }
   else
   {
      // the mesh above contains references to external data, to get a
      // copy independent of the conduit data, we use:
      res = new Mesh(*mesh,true);
      delete mesh;
   }

   return res;
}


//---------------------------------------------------------------------------//
mfem::GridFunction *
avtConduitBlueprintDataAdaptor::BlueprintToMFEM::FieldToMFEM(
    mfem::Mesh *mesh,
    const Node &n_field)
{
    bool zero_copy = true;

   // n_conv holds converted data (when necessary for mfem api)
   // if n_conv is used ( !n_conv.dtype().empty() ) we
   // know that some data allocation was necessary, so we
   // can't return a gf that zero copies the conduit data
   Node n_conv;

   const double *vals_ptr = NULL;

   int vdim = 1;

   Ordering::Type ordering = Ordering::byNODES;

   if (n_field["values"].dtype().is_object())
   {
      vdim = n_field["values"].number_of_children();

      // need to check that we have doubles and
      // cover supported layouts

      if ( n_field["values"][0].dtype().is_double() )
      {
         // check for contig
         if (n_field["values"].is_contiguous())
         {
            // conduit mcarray contig  == mfem byNODES
            vals_ptr = n_field["values"].child(0).value();
         }
         // check for interleaved
         else if (blueprint::mcarray::is_interleaved(n_field["values"]))
         {
            // conduit mcarray interleaved == mfem byVDIM
            ordering = Ordering::byVDIM;
            vals_ptr = n_field["values"].child(0).value();
         }
         else
         {
            // for mcarray generic case --  default to byNODES
            // and provide values w/ contiguous (soa) ordering
            blueprint::mcarray::to_contiguous(n_field["values"],
                                              n_conv["values"]);
            vals_ptr = n_conv["values"].child(0).value();
         }
      }
      else // convert to doubles and use contig
      {
         Node n_tmp;
         // check all vals, if we don't have doubles convert
         // to doubles
         NodeConstIterator itr = n_field["values"].children();
         while (itr.has_next())
         {
            const Node &c_vals = itr.next();
            std::string c_name = itr.name();

            if ( c_vals.dtype().is_double() )
            {
               // zero copy current coords
               n_tmp[c_name].set_external(c_vals);

            }
            else
            {
               // convert
               c_vals.to_double_array(n_tmp[c_name]);
            }
         }

         // for mcarray generic case --  default to byNODES
         // and provide values w/ contiguous (soa) ordering
         blueprint::mcarray::to_contiguous(n_tmp,
                                           n_conv["values"]);
         vals_ptr = n_conv["values"].child(0).value();
      }
   }
   else
   {
      if (n_field["values"].dtype().is_double() &&
          n_field["values"].is_compact())
      {
         vals_ptr = n_field["values"].value();
      }
      else
      {
         n_field["values"].to_double_array(n_conv["values"]);
         vals_ptr = n_conv["values"].value();
      }
   }

   if (zero_copy && !n_conv.dtype().is_empty())
   {
      //Info: "Cannot zero-copy since data conversions were necessary"
      zero_copy = false;
   }

   // we need basis name to create the proper mfem fec
   std::string fec_name = n_field["basis"].as_string();

   GridFunction *res = NULL;
   mfem::FiniteElementCollection *fec = FiniteElementCollection::New(
                                           fec_name.c_str());
   mfem::FiniteElementSpace *fes = new FiniteElementSpace(mesh,
                                                          fec,
                                                          vdim,
                                                          ordering);

   if (zero_copy)
   {
      res = new GridFunction(fes,const_cast<double*>(vals_ptr));
   }
   else
   {
      // copy case, this constructor will alloc the space for the GF data
      res = new GridFunction(fes);
      // create an mfem vector that wraps the conduit data
      Vector vals_vec(const_cast<double*>(vals_ptr),fes->GetVSize());
      // copy values into the result
      (*res) = vals_vec;
   }

   // TODO: I believe the GF already has ownership of fes, so this should be all
   // we need to do to avoid leaking objs created here?
   res->MakeOwner(fec);

   return res;
}
