/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
// avtBlueprintDataAdaptor.C
// ************************************************************************* //

#include "avtBlueprintDataAdaptor.h"

//-----------------------------------------------------------------------------
// vtk includes
//-----------------------------------------------------------------------------
#include "vtkCharArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkShortArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkIntArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkLongArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkLongLongArray.h"
#include "vtkUnsignedLongLongArray.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"

#include "vtkCellArray.h"
#include "vtkCellType.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

#include <vtkCell.h>
#include <vtkLine.h>
#include <vtkTriangle.h>
#include <vtkHexahedron.h>
#include <vtkQuad.h>
#include <vtkTetra.h>
#include <vtkPoints.h>

//-----------------------------------------------------------------------------
// visit includes
//-----------------------------------------------------------------------------
#include "InvalidVariableException.h"
#include "UnexpectedValueException.h"

//-----------------------------------------------------------------------------
// std lib includes
//-----------------------------------------------------------------------------
#include <string>

//-----------------------------------------------------------------------------
// conduit includes
//-----------------------------------------------------------------------------
#include "conduit.hpp"
#include "conduit_blueprint.hpp"

//-----------------------------------------------------------------------------
// mfem includes
//-----------------------------------------------------------------------------
#include "mfem.hpp"


//-----------------------------------------------------------------------------
// bp visit plugin includes
//-----------------------------------------------------------------------------
#include "avtBlueprintLogging.h"

using std::string;
using namespace conduit;
using namespace mfem;


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
                                 "Node is not a mcarray " << v_info.to_json());
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
vtkDataSet *
UniformCoordsToVTKRectilinearGrid(const Node &n_coords)
{
    vtkRectilinearGrid *rectgrid = vtkRectilinearGrid::New();

    BP_PLUGIN_INFO(n_coords.to_json());

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
vtkCellArray *
HomogeneousShapeTopologyToVTKCellArray(const Node &n_topo,
                                       int npts)
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

        int_array topo_conn;
        ida->SetNumberOfTuples(ncells * (csize + 1));
        for (int i = 0 ; i < ncells; i++)
        {
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
UnstructuredTopologyToVTKUnstructuredGrid(const Node &n_coords,
                                          const Node &n_topo)
{
    vtkPoints *points = ExplicitCoordsToVTKPoints(n_coords);

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    points->Delete();

    //
    // Now, add explicit topology
    //
    vtkCellArray *ca = HomogeneousShapeTopologyToVTKCellArray(n_topo, points->GetNumberOfPoints());
    ugrid->SetCells(ElementShapeNameToVTKCellType(n_topo["elements/shape"].as_string()), ca);
    ca->Delete();

    return ugrid;
}


// ****************************************************************************
vtkDataSet *
avtBlueprintDataAdaptor::VTK::MeshToVTK(const Node &n_mesh)
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
        else
        {
            BP_PLUGIN_INFO("BlueprintVTK::MeshToVTKDataSet UnstructuredTopologyToVTKUnstructuredGrid");
            res = UnstructuredTopologyToVTKUnstructuredGrid(n_coords, n_topo);
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
avtBlueprintDataAdaptor::VTK::FieldToVTK(const conduit::Node &field)
{
    return ConduitArrayToVTKDataArray(field["values"]);
}


// ****************************************************************************
// ****************************************************************************
///
/// MFEM Data Adaptor Functions
///
// ****************************************************************************
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
        BP_PLUGIN_WARNING("Unsupported Element Shape: " << shape_name);

    return res;
}


//---------------------------------------------------------------------------//
std::string
ElementTypeToShapeName(Element::Type element_type)
{
   // Adapted from SidreDataCollection

   // Note -- the mapping from Element::Type to string is based on
   //   enum Element::Type { POINT, SEGMENT, TRIANGLE, QUADRILATERAL,
   //                        TETRAHEDRON, HEXAHEDRON };
   // Note: -- the string names are from conduit's blueprint

   switch (element_type)
   {
      case Element::POINT:          return "point";
      case Element::SEGMENT:        return "line";
      case Element::TRIANGLE:       return "tri";
      case Element::QUADRILATERAL:  return "quad";
      case Element::TETRAHEDRON:    return "tet";
      case Element::HEXAHEDRON:     return "hex";
   }

   return "unknown";
}


//---------------------------------------------------------------------------//
mfem::Geometry::Type
ShapeNameToGeomType(const std::string &shape_name)
{
   // Note: must init to something to avoid invalid memory access
   // in the mfem mesh constructor
   mfem::Geometry::Type res = mfem::Geometry::POINT;

   if (shape_name == "point")
   {
      res = mfem::Geometry::POINT;
   }
   else if (shape_name == "line")
   {
      res =  mfem::Geometry::SEGMENT;
   }
   else if (shape_name == "tri")
   {
      res =  mfem::Geometry::TRIANGLE;
   }
   else if (shape_name == "quad")
   {
      res =  mfem::Geometry::SQUARE;
   }
   else if (shape_name == "tet")
   {
      res =  mfem::Geometry::TETRAHEDRON;
   }
   else if (shape_name == "hex")
   {
      res =  mfem::Geometry::CUBE;
   }
   else
   {
       BP_PLUGIN_WARNING("Unsupported Element Shape: " << shape_name);
   }

   return res;
}


//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
// TODO:
// In the future: these methods will be in MFEM's ConduitDataCollection
// we will those, instead of VisIt's own implementation.
//---------------------------------------------------------------------------//
mfem::Mesh *
avtBlueprintDataAdaptor::MFEM::MeshToMFEM(const Node &n_mesh,
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
       BP_PLUGIN_EXCEPTION1(InvalidVariableException,
                            "Expected topology named \""
                            << topo_name << "\" "
                            "(node is missing path \"topologies/"
                            << topo_name << "\")");
   }

   // find coord set

   std::string coords_name = n_mesh["topologies"][topo_name]["coordset"].as_string();


   if(!n_mesh.has_path("coordsets/" + coords_name))
   {
       BP_PLUGIN_EXCEPTION1(InvalidVariableException,
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
avtBlueprintDataAdaptor::MFEM::FieldToMFEM(mfem::Mesh *mesh,
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




//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
//
// Helpers for creating VTK objects from MFEM Objects
//
// These methods all support refinement (sub-divison) of mfem objects
// to create higher resolution low-order VTK data objects to represent the
// high order mfem objects.
//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

// ****************************************************************************
//  Method: RefineMeshToVTK
//
//  Purpose:
//    Constructs a vtkUnstructuredGrid that contains a refined mfem mesh.
//
//  Arguments:
//    mesh:      string with desired mesh name
//    lod:       number of refinement steps
//
//  Programmer: Cyrus Harrison
//  Creation:   Sat Jul  5 11:38:31 PDT 2014
//
// Notes: Adapted from avtMFEMFileFormat and MFEM examples.
//
// ****************************************************************************
vtkDataSet *
avtBlueprintDataAdaptor::MFEM::RefineMeshToVTK(mfem::Mesh *mesh,
                                               int lod)
{
    BP_PLUGIN_INFO("Creating Refined MFEM Mesh with lod:" << lod);

    // create output objects
    vtkUnstructuredGrid *res_ds  = vtkUnstructuredGrid::New();
    vtkPoints           *res_pts = vtkPoints::New();

    int npts=0;
    int neles=0;

    RefinedGeometry *refined_geo;
    DenseMatrix      pmat;

    //
    // find the # of output points and cells at the selected level of
    // refinement (we may want to cache this...)
    //
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        int ele_nverts = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo = GlobGeometryRefiner.Refine(geom, lod, 1);
        npts  += refined_geo->RefPts.GetNPoints();
        neles += refined_geo->RefGeoms.Size() / ele_nverts;
    }

    // create the points for the refined topoloy
    res_pts->Allocate(npts);
    res_pts->SetNumberOfPoints((vtkIdType) npts);

    // create the points for the refined topoloy
    int pt_idx=0;
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        refined_geo = GlobGeometryRefiner.Refine(geom, lod, 1);
        // refined points
        mesh->GetElementTransformation(i)->Transform(refined_geo->RefPts, pmat);
        for (int j = 0; j < pmat.Width(); j++)
        {
            double pt_vals[3]={0.0,0.0,0.0};
            pt_vals[0] = pmat(0,j);
            if (pmat.Height() > 1)
                pt_vals[1] = pmat(1,j);
            if (pmat.Height() > 2)
                pt_vals[2] = pmat(2,j);
            res_pts->InsertPoint(pt_idx,pt_vals);
            pt_idx++;
        }
    }

    res_ds->SetPoints(res_pts);
    res_pts->Delete();
    // create the cells for the refined topology
    res_ds->Allocate(neles);

    pt_idx=0;
    for (int i = 0; i <  mesh->GetNE(); i++)
    {
        int geom       = mesh->GetElementBaseGeometry(i);
        int ele_nverts = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo    = GlobGeometryRefiner.Refine(geom, lod, 1);

        Array<int> &rg_idxs = refined_geo->RefGeoms;

        vtkCell *ele_cell = NULL;
        // rg_idxs contains all of the verts for the refined elements
        for (int j = 0; j < rg_idxs.Size(); )
        {
            switch (geom)
            {
                case Geometry::SEGMENT:      ele_cell = vtkLine::New();       break;
                case Geometry::TRIANGLE:     ele_cell = vtkTriangle::New();   break;
                case Geometry::SQUARE:       ele_cell = vtkQuad::New();       break;
                case Geometry::TETRAHEDRON:  ele_cell = vtkTetra::New();      break;
                case Geometry::CUBE:         ele_cell = vtkHexahedron::New(); break;
            }
            // the are ele_nverts for each refined element
            for (int k = 0; k < ele_nverts; k++, j++)
                ele_cell->GetPointIds()->SetId(k,pt_idx + rg_idxs[j]);

            res_ds->InsertNextCell(ele_cell->GetCellType(),
                                   ele_cell->GetPointIds());
            ele_cell->Delete();
        }

        pt_idx += refined_geo->RefPts.GetNPoints();
   }

   return res_ds;
}

// ****************************************************************************
//  Method: RefineGridFunctionToVTK
//
//  Purpose:
//   Constructs a vtkDataArray that contains a refined mfem mesh field variable.
//
//  Arguments:
//   mesh:      MFEM mesh for the field
//   gf:        MFEM Grid Function for the field
//   lod:       number of refinement steps
//
//  Programmer: Cyrus Harrison
//  Creation:   Sat Jul  5 11:38:31 PDT 2014
//
// Notes: Adapted from avtMFEMFileFormat and MFEM examples.
//
// ****************************************************************************
vtkDataArray *
avtBlueprintDataAdaptor::MFEM::RefineGridFunctionToVTK(mfem::Mesh *mesh,
                                                       mfem::GridFunction *gf,
                                                       int lod)
{
    BP_PLUGIN_INFO("Creating Refined MFEM Field with lod:" << lod);
    int npts=0;
    int neles=0;

    RefinedGeometry *refined_geo;
    Vector           scalar_vals;
    DenseMatrix      vec_vals;
    DenseMatrix      pmat;

    //
    // find the # of output points and cells at the selected level of
    // refinement (we may want to cache this...)
    //
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        int ele_nverts = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo    = GlobGeometryRefiner.Refine(geom, lod, 1);
        npts  += refined_geo->RefPts.GetNPoints();
        neles += refined_geo->RefGeoms.Size() / ele_nverts;
    }

    vtkFloatArray *rv = vtkFloatArray::New();

    int ncomps =gf->VectorDim();

    if(ncomps == 2)
        rv->SetNumberOfComponents(3);
    else
        rv->SetNumberOfComponents(ncomps);

    rv->SetNumberOfTuples(npts);

    double tuple_vals[9];
    int ref_idx=0;
    for (int i = 0; i <  mesh->GetNE(); i++)
    {
        int geom       = mesh->GetElementBaseGeometry(i);
        refined_geo    = GlobGeometryRefiner.Refine(geom, lod, 1);
        if(ncomps == 1)
        {
            gf->GetValues(i, refined_geo->RefPts, scalar_vals, pmat);
            for (int j = 0; j < scalar_vals.Size();j++)
            {
                tuple_vals[0] = scalar_vals(j);
                rv->SetTuple(ref_idx, tuple_vals);
                ref_idx++;
            }
        }
        else
        {
            gf->GetVectorValues(i, refined_geo->RefPts, vec_vals, pmat);
            for (int j = 0; j < vec_vals.Width(); j++)
            {
                tuple_vals[2] = 0.0;
                tuple_vals[0] = vec_vals(0,j);
                tuple_vals[1] = vec_vals(1,j);
                if (vec_vals.Height() > 2)
                    tuple_vals[2] = vec_vals(2,j);
                rv->SetTuple(ref_idx, tuple_vals);
                ref_idx++;
            }
        }
    }

    return rv;
}

// ****************************************************************************
//  Method: RefineElementColoringToVTK
//
//  Purpose:
//   Constructs a vtkDataArray that contains coloring that reflects the orignal
//   finite elements of a mfem mesh.
//
//  Arguments:
//   mesh:        MFEM mesh object
//   domain_id :  domain id, use for rng seed
//   lod:         number of refinement steps
//
//  Programmer: Cyrus Harrison
//  Creation:   Sat Jul  5 11:38:31 PDT 2014
//
// Notes: Adapted from avtMFEMFileFormat and MFEM examples.
//
// ****************************************************************************
vtkDataArray *
avtBlueprintDataAdaptor::MFEM::RefineElementColoringToVTK(mfem::Mesh *mesh,
                                                          int domain_id,
                                                          int lod)
{
    BP_PLUGIN_INFO("Creating Refined MFEM Element Coloring with lod:" << lod);
    int npts=0;
    int neles=0;

    RefinedGeometry *refined_geo;
    Array<int>       coloring;

    //
    // find the # of output points and cells at the selected level of
    // refinement (we may want to cache this...)
    //
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        int ele_nverts = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo    = GlobGeometryRefiner.Refine(geom, lod, 1);
        npts  += refined_geo->RefPts.GetNPoints();
        neles += refined_geo->RefGeoms.Size() / ele_nverts;
    }

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(1);
    rv->SetNumberOfTuples(neles);

    //
    // Use mfem's mesh coloring algo
    //

    // seed using domain id for predictable results
    srand(domain_id);

#ifdef _WIN32
    double a = double(rand()) / (double(RAND_MAX) + 1.);
#else
    double a = double(random()) / (double(RAND_MAX) + 1.);
#endif
    int el0 = (int)floor(a * mesh->GetNE());
    mesh->GetElementColoring(coloring, el0);
    int ref_idx=0;
    // set output array value from generated coloring
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        int nv = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo= GlobGeometryRefiner.Refine(geom, lod, 1);
        for (int j = 0; j < refined_geo->RefGeoms.Size(); j += nv)
        {
             rv->SetTuple1(ref_idx,coloring[i]+1);
             ref_idx++;
        }
   }

   return rv;
}


// ****************************************************************************
//  Method: RefineElementAttributeToVTK
//
//  Purpose:
//   Constructs a vtkDataArray that contains the refined "attribute" value
//   for finite elements in a mfem mesh.
//
//  Arguments:
//   mesh:      MFEM mesh object
//   lod:       number of refinement steps
//
//  Programmer: Cyrus Harrison
//  Creation:   Sat Jul  5 11:38:31 PDT 2014
//
// ****************************************************************************
vtkDataArray *
avtBlueprintDataAdaptor::MFEM::RefineElementAttributeToVTK(mfem::Mesh *mesh,
                                                           int lod)
{
    BP_PLUGIN_INFO("Creating Refined MFEM Element Attribute with lod:" << lod);
    int npts=0;
    int neles=0;

    RefinedGeometry *refined_geo;
    Array<int>       coloring;

    //
    // find the # of output points and cells at the selected level of
    // refinement (we may want to cache this...)
    //
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        int ele_nverts = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo    = GlobGeometryRefiner.Refine(geom, lod, 1);
        npts  += refined_geo->RefPts.GetNPoints();
        neles += refined_geo->RefGeoms.Size() / ele_nverts;
    }

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(1);
    rv->SetNumberOfTuples(neles);

    // set output array value from the mfem mesh's "Attribue" field
    int ref_idx=0;
    for (int i = 0; i < mesh->GetNE(); i++)
    {
        int geom = mesh->GetElementBaseGeometry(i);
        int nv = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo= GlobGeometryRefiner.Refine(geom, lod, 1);
        int attr = mesh->GetAttribute(i);
        for (int j = 0; j < refined_geo->RefGeoms.Size(); j += nv)
        {
             rv->SetTuple1(ref_idx,attr);
             ref_idx++;
        }
   }
   return rv;
}

// ****************************************************************************
// ****************************************************************************
///
/// VTK Data to Blueprint Adaptor Functions
///
// ****************************************************************************
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
void VTKDataArrayToNode(Node &node, vtkDataArray *arr)
{
    int ncomps = arr->GetNumberOfComponents();

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
// ****************************************************************************
void avtBlueprintDataAdaptor::BP::VTKFieldsToBlueprint(conduit::Node &node,
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
         std::string fname = arr->GetName();
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
         std::string fname = arr->GetName();
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
avtBlueprintDataAdaptor::BP::VTKToBlueprint(conduit::Node &mesh,
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

   avtBlueprintDataAdaptor::BP::VTKFieldsToBlueprint(mesh, topo_name, dataset);

}
