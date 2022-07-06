/*****************************************************************************
*
* Copyright (c) 2000 - 2022, Lawrence Livermore National Security, LLC
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

#include <avtConduitBlueprintDataAdaptor.h>
#include <conduit.hpp>
#include <conduit_blueprint.hpp>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>

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
    if (shape_name == "point") return VTK_VERTEX;
    if (shape_name == "line")  return VTK_LINE;
    if (shape_name == "tri")   return VTK_TRIANGLE;
    if (shape_name == "quad")  return VTK_QUAD;
    if (shape_name == "hex")   return VTK_HEXAHEDRON;
    if (shape_name == "tet")   return VTK_TETRA;
    BP_PLUGIN_INFO("Warning: Unsupported Element Shape: " << shape_name);
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

    BP_PLUGIN_INFO("Warning: Unsupported vtkCellType : " << vtk_cell_type);
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
template<typename T> void
Blueprint_MultiCompArray_To_VTKDataArray(const Node &n,
                                         int ncomps,
                                         int ntuples,
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
                darray->SetComponent(i, c, (double) vals_array[i]);

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
            darray->SetComponent(i,0, (double) vals_array[i]);
        }
    }
}

// ****************************************************************************
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//                               BLUEPRINT --> VTK
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
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
// ****************************************************************************
vtkDataArray *
ConduitArrayToVTKDataArray(const conduit::Node &n)
{
    BP_PLUGIN_INFO("Creating VTKDataArray from Node");
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
            BP_PLUGIN_EXCEPTION1(InvalidVariableException,
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

    BP_PLUGIN_INFO("VTKDataArray num_tuples = " << ntuples << " "
                    << " num_comps = " << ncomps);

    if (vals_dtype.is_unsigned_char())
    {
        retval = vtkUnsignedCharArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_UNSIGNED_CHAR>(n,
                                                                               ncomps,
                                                                               ntuples,
                                                                               retval);
    }
    else if (vals_dtype.is_unsigned_short())
    {
        retval = vtkUnsignedShortArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_UNSIGNED_SHORT>(n,
                                                                                ncomps,
                                                                                ntuples,
                                                                                retval);
    }
    else if (vals_dtype.is_unsigned_int())
    {
        retval = vtkUnsignedIntArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_UNSIGNED_INT>(n,
                                                                              ncomps,
                                                                              ntuples,
                                                                              retval);
    }
    else if (vals_dtype.is_unsigned_long())
    {
        retval = vtkUnsignedLongArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_UNSIGNED_LONG>(n,
                                                                               ncomps,
                                                                               ntuples,
                                                                               retval);
    }
#if CONDUIT_USE_LONG_LONG
    else if (vals_dtype.id() == CONDUIT_NATIVE_UNSIGNED_LONG_LONG_ID)
    {
        retval = vtkUnsignedLongLongArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_UNSIGNED_LONG_LONG>(n,
                                                                                    ncomps,
                                                                                    ntuples,
                                                                                    retval);
    }
#endif
    else if (vals_dtype.is_char())
    {
        retval = vtkCharArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_CHAR>(n,
                                                                      ncomps,
                                                                      ntuples,
                                                                      retval);

    }
    else if (vals_dtype.is_short())
    {
        retval = vtkShortArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_SHORT>(n,
                                                                       ncomps,
                                                                       ntuples,
                                                                       retval);
    }
    else if (vals_dtype.is_int())
    {
        retval = vtkIntArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_INT>(n,
                                                                     ncomps,
                                                                     ntuples,
                                                                     retval);
    }
    else if (vals_dtype.is_long())
    {
        retval = vtkLongArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_LONG>(n,
                                                                      ncomps,
                                                                      ntuples,
                                                                      retval);
    }
#if CONDUIT_USE_LONG_LONG
    else if (vals_dtype.id() == CONDUIT_NATIVE_LONG_LONG_ID)
    {
        retval = vtkLongLongArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_LONG_LONG>(n,
                                                                              ncomps,
                                                                              ntuples,
                                                                              retval);
    }
#endif
    else if (vals_dtype.is_float())
    {
        retval = vtkFloatArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_FLOAT>(n,
                                                                       ncomps,
                                                                       ntuples,
                                                                       retval);
    }
    else if (vals_dtype.is_double())
    {
        retval = vtkDoubleArray::New();
        Blueprint_MultiCompArray_To_VTKDataArray<CONDUIT_NATIVE_DOUBLE>(n,
                                                                        ncomps,
                                                                        ntuples,
                                                                        retval);
    }
    else
    {
        BP_PLUGIN_EXCEPTION1(InvalidVariableException,
                             "Conduit Array to VTK Data Array"
                             "unsupported data type: " << n.dtype().name());
    }
    return retval;
}

// ****************************************************************************
vtkPoints *
ExplicitCoordsToVTKPoints(const Node &n_coords)
{
    vtkPoints *points = vtkPoints::New();

    const Node &n_vals = n_coords["values"];

    // We always use doubles

    int npts = (int) n_vals["x"].dtype().number_of_elements();

    double_array x_vals;
    double_array y_vals;
    double_array z_vals;

    bool have_y = false;
    bool have_z = false;

    Node n_vals_double;

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


    points->SetDataTypeToDouble();
    points->SetNumberOfPoints(npts);

    //TODO: we could describe the VTK data array via
    // and push the conversion directly into its memory.

    for (vtkIdType i = 0; i < npts; i++)
    {
        double x = x_vals[i];
        double y = have_y ? y_vals[i] : 0;
        double z = have_z ? z_vals[i] : 0;
        points->SetPoint(i, x, y, z);
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

    BP_PLUGIN_INFO(n_coords.to_yaml());

    int nx[3];
    nx[0] = n_coords["dims"].has_child("i") ? n_coords["dims/i"].to_int() : 1;
    nx[1] = n_coords["dims"].has_child("j") ? n_coords["dims/j"].to_int() : 1;
    nx[2] = n_coords["dims"].has_child("k") ? n_coords["dims/k"].to_int() : 1;
    rectgrid->SetDimensions(nx);

    double dx[3] = {1.0,1.0,1.0};
    if(n_coords.has_child("spacing"))
    {
        const Node &n_spacing = n_coords["spacing"];

        if(n_spacing.has_child("dx"))
            dx[0] = n_spacing["dx"].to_double();

        if(n_spacing.has_child("dy"))
            dx[1] = n_spacing["dy"].to_double();

        if(n_spacing.has_child("dz"))
            dx[2] = n_spacing["dz"].to_double();
    }

    double x0[3] = {0.0, 0.0, 0.0};

    if(n_coords.has_child("origin"))
    {
        const Node &n_origin =  n_coords["origin"];

        if(n_origin.has_child("x"))
            x0[0] = n_origin["x"].to_double();

        if(n_origin.has_child("y"))
            x0[1] = n_origin["y"].to_double();

        if(n_origin.has_child("z"))
            x0[2] = n_origin["z"].to_double();

    }

    for (int i = 0; i < 3; i++)
    {
        vtkDataArray *da = NULL;
        DataType dt = DataType::c_double();
        // we have we origin, we can infer type from it
        if(n_coords.has_path("origin/x"))
        {
            dt = n_coords["origin"]["x"].dtype();
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
            BP_PLUGIN_EXCEPTION1(InvalidVariableException,
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
    vtkRectilinearGrid *rectgrid = vtkRectilinearGrid::New();

    const Node &n_coords_values  = n_coords["values"];

    int dims[3] = {1,1,1};

    dims[0] = n_coords_values["x"].dtype().number_of_elements();
    if (n_coords_values.has_child("y"))
        dims[1] = n_coords_values["y"].dtype().number_of_elements();
    if (n_coords_values.has_child("z"))
        dims[2] = n_coords_values["z"].dtype().number_of_elements();
    rectgrid->SetDimensions(dims);

    vtkDataArray *coords[3] = {0,0,0};
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

    vtkPoints *points = ExplicitCoordsToVTKPoints(n_coords);
    sgrid->SetPoints(points);
    points->Delete();

    return sgrid;
}

// ****************************************************************************
vtkDataSet *
PointsTopologyToVTKUnstructuredGrid(const Node &n_coords,
                                    const Node &n_topo)
{
    vtkPoints *points = ExplicitCoordsToVTKPoints(n_coords);

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
// ****************************************************************************
vtkDataSet *
UnstructuredTopologyToVTKUnstructuredGrid(int domain,
                                          const Node &n_coords,
                                          const Node &n_topo)
{
    const Node *coords_ptr = &n_coords;
    const Node *topo_ptr = &n_topo;

    Node res; // Used as a destination for the generate sides call

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    vtkUnsignedIntArray *oca = NULL;

    if (n_topo.has_path("elements/shape"))
    {
        if (n_topo["elements/shape"].as_string() == "polyhedral" || 
            n_topo["elements/shape"].as_string() == "polygonal")
        {
            #if CONDUIT_HAVE_PARTITION_FLATTEN == 1
                Node s2dmap, d2smap, options;
                blueprint::mesh::topology::unstructured::generate_sides(
                    n_topo,
                    res["topologies/" + n_topo.name()],
                    res["coordsets/" + n_topo["coordset"].as_string()],
                    s2dmap,
                    d2smap);

                unsigned_int_accessor values = d2smap["values"].value();

                oca = vtkUnsignedIntArray::New();
                oca->SetName("avtOriginalCellNumbers");
                oca->SetNumberOfComponents(2);

                for (int i = 0; i < values.number_of_elements(); i ++)
                {
                    unsigned int ocdata[2] = {static_cast<unsigned int>(domain), 
                                              static_cast<unsigned int>(values[i])};
                    oca->InsertNextTypedTuple(ocdata);
                }

                coords_ptr = res.fetch_ptr(
                    "coordsets/" + n_topo["coordset"].as_string());
                topo_ptr = res.fetch_ptr("topologies/" + n_topo.name());
            #else
                Node about;
                conduit::about(about);
                BP_PLUGIN_EXCEPTION1(InvalidVariableException,
                    "VisIt Blueprint Plugin requires Conduit >= 0.8.0 to read polytopal meshes."
                    "VisIt was built with Conduit version:"  << about["version"].as_string());
            #endif
        }
    }

    // The coords could be changed in the call above, so this must happen
    // after the conditionals
    vtkPoints *points = ExplicitCoordsToVTKPoints(*coords_ptr);

    ugrid->SetPoints(points);

    if (oca != NULL)
    {
        ugrid->GetCellData()->AddArray(oca);
        ugrid->GetCellData()->CopyFieldOn("avtOriginalCellNumbers");
        oca->Delete();
    }
    
    //
    // Now, add explicit topology
    //
    vtkCellArray *ca = HomogeneousShapeTopologyToVTKCellArray(*topo_ptr, points->GetNumberOfPoints());
    points->Delete();
    ugrid->SetCells(ElementShapeNameToVTKCellType(topo_ptr->fetch("elements/shape").as_string()), ca);
    ca->Delete();

    return ugrid;
}

// ****************************************************************************
//  Method: MeshToVTK
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
// ****************************************************************************
vtkDataSet *
avtConduitBlueprintDataAdaptor::MeshToVTK(int domain,
                                        const Node &n_mesh)
{
    //NOTE: this assumes one coordset and one topo
    // that is the case for the blueprint plugin, but may not be the case
    // generally if we want to reuse this code.
    BP_PLUGIN_INFO("BlueprintVTK::MeshToVTKDataSet Begin");

    const Node &n_coords = n_mesh["coordsets"][0];
    const Node &n_topo   = n_mesh["topologies"][0];

    vtkDataSet *res = NULL;

    if (n_coords["type"].as_string() == "uniform")
    {
        BP_PLUGIN_INFO("BlueprintVTK::MeshToVTKDataSet UniformCoordsToVTKRectilinearGrid");
        res = UniformCoordsToVTKRectilinearGrid(n_coords);
    }
    else if (n_coords["type"].as_string() == "rectilinear")
    {
        BP_PLUGIN_INFO("BlueprintVTK::MeshToVTKDataSet RectilinearCoordsToVTKRectilinearGrid");
        res = RectilinearCoordsToVTKRectilinearGrid(n_coords);
    }
    else if (n_coords["type"].as_string() == "explicit")
    {
        if (n_topo["type"].as_string() == "structured")
        {
            BP_PLUGIN_INFO("BlueprintVTK::MeshToVTKDataSet StructuredTopologyToVTKStructuredGrid");
            res = StructuredTopologyToVTKStructuredGrid(n_coords, n_topo);
        }
        else if (n_topo["type"].as_string() == "points")
        {
            BP_PLUGIN_INFO("BlueprintVTK::MeshToVTKDataSet PointsTopologyToVTKUnstructuredGrid");
            res = PointsTopologyToVTKUnstructuredGrid(n_coords, n_topo);
        }
        else
        {
            BP_PLUGIN_INFO("BlueprintVTK::MeshToVTKDataSet UnstructuredTopologyToVTKUnstructuredGrid");
            res = UnstructuredTopologyToVTKUnstructuredGrid(domain, n_coords, n_topo);
        }
    }
    else
    {
        BP_PLUGIN_EXCEPTION1(InvalidVariableException,
                              "expected Coordinate type of \"uniform\", \"rectilinear\", or \"explicit\""
                              << " but found " << n_coords["type"].as_string());
    }

    BP_PLUGIN_INFO("BlueprintVTK::MeshToVTKDataSet End");

    return res;
}

// ****************************************************************************
vtkDataArray *
avtConduitBlueprintDataAdaptor::FieldToVTK(const conduit::Node &field)
{
    return ConduitArrayToVTKDataArray(field["values"]);
}

// ****************************************************************************
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//                               VTK --> BLUEPRINT
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
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
void vtkUnstructuredToNode(Node &node, vtkUnstructuredGrid *grid, const int dims)
{
  const int npts = grid->GetNumberOfPoints();
  const int nzones = grid->GetNumberOfCells();

  if(nzones == 0)
  {
    BP_PLUGIN_EXCEPTION1(InvalidVariableException,
                         "Unstructured grids must have at least one zone");
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
    BP_PLUGIN_EXCEPTION1(InvalidVariableException,
                         "Only usntructured grids with a single cell type is currently supported");
  }

  const std::string shape_name = VTKCellTypeToElementShapeName(cell_type);
  if(shape_name == "")
  {
    BP_PLUGIN_EXCEPTION1(InvalidVariableException,
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
//  Method: avtBlueprintDataAdapter::VTKFieldNameToBlueprint
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
//
// ****************************************************************************
void avtConduitBlueprintDataAdaptor::VTKFieldNameToBlueprint(const std::string &vtk_name,
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
//  Method: avtBlueprintDataAdapter::VTKFieldsToBlueprint
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
// ****************************************************************************
void avtConduitBlueprintDataAdaptor::VTKFieldsToBlueprint(conduit::Node &node,
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
         BP_PLUGIN_INFO("VTKBlueprint:: converted point field "<<fname);
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

         BP_PLUGIN_INFO("VTKBlueprint:: converted cell field "<<fname);
         VTKDataArrayToNode(node[field_path + "/values"], arr);
    }
  }
}

// ****************************************************************************
//  Method: avtBlueprintDataAdapter::VTKToBlueprint
//
//  Purpose:
//      Takes a vtk data set and converts into a  blueprint mesh.
//
//  Programmer: Matt Larsen
//  Creation:   Feb 15, 2019
//
//  Modifications:
//
// ****************************************************************************
void
avtConduitBlueprintDataAdaptor::VTKToBlueprint(conduit::Node &mesh,
                                               vtkDataSet* dataset,
                                               const int ndims)
{
   std::string coord_path = "coordsets/coords";
   std::string topo_name = "topo";
   std::string topo_path = "topologies/" + topo_name;
   mesh[topo_path + "/coordset"] = "coords";

   if (dataset->GetDataObjectType() == VTK_RECTILINEAR_GRID)
   {
     BP_PLUGIN_INFO("VTKToBlueprint:: Rectilinear");
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
     BP_PLUGIN_INFO("VTKToBlueprint:: StructuredGrid");
     vtkStructuredGrid *grid = (vtkStructuredGrid *) dataset;

     mesh[coord_path + "/type"] = "explicit";
     mesh[topo_path + "/type"] = "structured";

     vtkPoints *vtk_pts = grid->GetPoints();
     vtkPointsToNode(mesh[coord_path + "/values"], vtk_pts, ndims);

     int dims[3];
     grid->GetCellDims(dims);
     BP_PLUGIN_INFO("VTKBlueprint:: StructuredGrid4");
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
     BP_PLUGIN_INFO("VTKToBlueprint:: UntructuredGrid");
     vtkUnstructuredGrid *grid = (vtkUnstructuredGrid *) dataset;

     mesh[coord_path + "/type"] = "explicit";
     mesh[topo_path + "/type"] = "unstructured";

     vtkPoints *vtk_pts = grid->GetPoints();
     vtkPointsToNode(mesh[coord_path + "/values"], vtk_pts, ndims);
     vtkUnstructuredToNode(mesh[topo_path + "/elements"], grid, ndims);
   }

   avtConduitBlueprintDataAdaptor::VTKFieldsToBlueprint(mesh, topo_name, dataset);

}
