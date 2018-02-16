/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
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
#include "vtkIdTypeArray.h"
#include "vtkPoints.h"
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



//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
//
// Helpers for creating VTK objects from Blueprint conforming Conduit data.
//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

// ****************************************************************************
template<typename T> void 
Blueprint_MulitCompArray_To_VTKDataArray(const Node &n,
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
            BP_PLUGIN_ERROR("Node is not a mcarray " << v_info.to_json());
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
        Blueprint_MulitCompArray_To_VTKDataArray<CONDUIT_NATIVE_UNSIGNED_CHAR>(n,
                                                                               ncomps,
                                                                               ntuples,
                                                                               retval);
    }
    else if (vals_dtype.is_unsigned_short())
    {
        retval = vtkUnsignedShortArray::New();
        Blueprint_MulitCompArray_To_VTKDataArray<CONDUIT_NATIVE_UNSIGNED_SHORT>(n,
                                                                                ncomps,
                                                                                ntuples,
                                                                                retval);
    }
    else if (vals_dtype.is_unsigned_int())
    {
        retval = vtkUnsignedIntArray::New();
        Blueprint_MulitCompArray_To_VTKDataArray<CONDUIT_NATIVE_UNSIGNED_INT>(n,
                                                                              ncomps,
                                                                              ntuples,
                                                                              retval);
    }
    else if (vals_dtype.is_char())
    {
        retval = vtkCharArray::New();
        Blueprint_MulitCompArray_To_VTKDataArray<CONDUIT_NATIVE_CHAR>(n,
                                                                      ncomps,
                                                                      ntuples,
                                                                      retval);

    }
    else if (vals_dtype.is_short())
    {
        retval = vtkShortArray::New();
        Blueprint_MulitCompArray_To_VTKDataArray<CONDUIT_NATIVE_SHORT>(n,
                                                                       ncomps,
                                                                       ntuples,
                                                                       retval);
    }
    else if (vals_dtype.is_int())
    {
        retval = vtkIntArray::New();
        Blueprint_MulitCompArray_To_VTKDataArray<CONDUIT_NATIVE_INT>(n,
                                                                     ncomps,
                                                                     ntuples,
                                                                     retval);
    }
    else if (vals_dtype.is_float())
    {
        retval = vtkFloatArray::New();
        Blueprint_MulitCompArray_To_VTKDataArray<CONDUIT_NATIVE_FLOAT>(n,
                                                                       ncomps,
                                                                       ntuples,
                                                                       retval);
    }
    else if (vals_dtype.is_double())
    {
        retval = vtkDoubleArray::New();
        Blueprint_MulitCompArray_To_VTKDataArray<CONDUIT_NATIVE_DOUBLE>(n,
                                                                        ncomps,
                                                                        ntuples,
                                                                        retval);
    }
    else
    {
        BP_PLUGIN_ERROR("unsupported datatype " << n.dtype().name());
        EXCEPTION2(UnexpectedValueException, "A standard data type", "unknown");
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

    double dx[3];
    dx[0] = n_coords["spacing"].has_child("dx") ? n_coords["spacing/dx"].to_double() : 0;
    dx[1] = n_coords["spacing"].has_child("dy") ? n_coords["spacing/dy"].to_double() : 0;
    dx[2] = n_coords["spacing"].has_child("dz") ? n_coords["spacing/dz"].to_double() : 0;

    double x0[3]; 
    
    x0[0] = n_coords["origin"].has_child("x") ? n_coords["origin/x"].to_double() : 0;
    x0[1] = n_coords["origin"].has_child("y") ? n_coords["origin/y"].to_double() : 0;
    x0[2] = n_coords["origin"].has_child("z") ? n_coords["origin/z"].to_double() : 0;

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
        else if (dt.is_float())
            da = vtkFloatArray::New();
        else if (dt.is_double())
            da = vtkDoubleArray::New();
        else
        {
            BP_PLUGIN_ERROR("unsupported data type " << dt.name());
            EXCEPTION2(UnexpectedValueException, "A standard data type", dt.name());
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
    
    // This old path preserves the original data type, but 
    // creates doubles in the process and does many
    // string comparisons per coord value, we want to avoid that. 
    
    /*
    
    int npts = (int) n_coords_values["x"].dtype().number_of_elements();
    DataType dt = n_coords_values["x"].dtype();
    if(dt.is_char())
        points->SetDataTypeToChar();
    else if (dt.is_unsigned_char())
        points->SetDataTypeToUnsignedChar();
    else if (dt.is_short())
        points->SetDataTypeToShort();
    else if (dt.is_unsigned_short())
        points->SetDataTypeToUnsignedShort();
    else if (dt.is_int())
        points->SetDataTypeToInt();
    else if (dt.is_unsigned_int())
        points->SetDataTypeToUnsignedInt();
    else if (dt.is_float())
        points->SetDataTypeToFloat();
    else if (dt.is_double())
        points->SetDataTypeToDouble();
    else
    {
        BP_PLUGIN_ERROR("unsupported data type " << dt.name());
        EXCEPTION2(UnexpectedValueException, "A standard data type", "unknown");
    }
    points->SetNumberOfPoints(npts);
    
    
    
    for (vtkIdType i = 0; i < npts; i++)
    {
        double x = NodeValueAsDouble(n_coords_values["x"], i);
        double y = n_coords_values.has_child("y") ? NodeValueAsDouble(n_coords_values["y"], i) : 0;
        double z = n_coords_values.has_child("z") ? NodeValueAsDouble(n_coords_values["z"], i) : 0;
        points->SetPoint(i, x, y, z);
    }
    
    */
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

        ida->SetNumberOfTuples(ncells * (csize + 1));
        for (int i = 0 ; i < ncells; i++)
        {
            int_array topo_conn = n_topo["elements/connectivity"].as_int_array();
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
avtBlueprintDataAdaptor::VTK::MeshToVTK(const Node &n_mesh)
{
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
            res = ugrid;
        }
    }
    else
    {
        BP_PLUGIN_ERROR( "expected Coordinate type of \"uniform\", \"rectilinear\", or \"explicit\""
                          << " but found " << n_coords["type"].as_string());
        EXCEPTION2(UnexpectedValueException,
                   "Coordinate type of \"uniform\", \"rectilinear\", or \"explicit\"",
                   n_coords["type"].as_string());
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




// ****************************************************************************
mfem::GridFunction *
avtBlueprintDataAdaptor::MFEM::FieldToMFEM(mfem::Mesh *mesh, 
                                           const Node &field)
{
    BP_PLUGIN_INFO("Creating MFEM GridFunction");
    BP_PLUGIN_INFO("field schema:" << field.schema().to_json());

    // we need basis name to create the proper mfem fec
    string fec_name = field["basis"].as_string();

    // TODO: Create a container class that can hold on to these
    // pointers, so we can clean them up and not leak. 
    
    // TODO: unless the gf takes ownership of the fec or fes, we are leaking
    mfem::FiniteElementCollection *fec = FiniteElementCollection::New(fec_name.c_str());

    // TODO: the gf can't take ownership of the data memory, we are leaking ....
    Node *field_vals= new Node();

    // TODO: USE OREDERING IN CONSTRUCTOR TO AVOID REPACKING
    int vdim = 1;
    if(field["values"].dtype().is_object())
    {
        // for mcarray case, the mfem gf constructor we need to use 
        // requires a contiguous (soa) ordering
        vdim = field["values"].number_of_children();
        blueprint::mcarray::to_contiguous(field["values"],
                                           *field_vals);
    }
    else
    {
        field["values"].compact_to(*field_vals);
    }
    
    // TODO: unless the gf takes ownership of the fec or fes, we are leaking
    mfem::FiniteElementSpace *fes = new FiniteElementSpace(mesh, fec, vdim);

    // TODO: Assumes doubles, mfem only supports doubles but source data 
    // may not be doubles ... 
    double *vals_ptr = NULL;
    if(field["values"].dtype().is_object())
    {
        //the vals are contiguous, we fetch the pointer
        // to the first component in the mcarray
        vals_ptr = field_vals->child_ptr(0)->value();
    }
    else
    {
        vals_ptr = field_vals->value();
    }
    
    mfem::GridFunction *res = new GridFunction(fes,vals_ptr);
    
    return res;
}

// ****************************************************************************
mfem::Mesh *
avtBlueprintDataAdaptor::MFEM::MeshToMFEM(const Node &mesh)
{
    BP_PLUGIN_INFO("Creating MFEM Mesh");
    
    // get the number of dims of the coordset
    const Node n_coordset = mesh["coordsets"][0];
    
    int ndims = n_coordset["values"].number_of_children();

    BP_PLUGIN_INFO("Coordset Dimensions: " << ndims);
    BP_PLUGIN_INFO("Getting Coordset Data");

    // mfem mesh constructor needs coords with interleaved (aos) type ordering
    Node coords_values;
    blueprint::mcarray::to_interleaved(n_coordset["values"],coords_values);

    int num_verts         = coords_values[0].dtype().number_of_elements();
    double *verts_indices = coords_values[0].value();

    BP_PLUGIN_INFO("Getting Mesh Element Data ");
    const Node &mesh_topo = mesh["topologies"][0];
    string mesh_ele_shape = mesh_topo["elements/shape"].as_string();
    mfem::Geometry::Type mesh_geo = ElementShapeNameToMFEMShape(mesh_ele_shape);
    const int *elem_indices = mesh_topo["elements/connectivity"].value();
    int num_mesh_ele        = mesh_topo["elements/connectivity"].dtype().number_of_elements();
    num_mesh_ele            = num_mesh_ele / ConduitElementShapeSize(mesh_ele_shape);


    const int *bndry_indices = NULL;
    int num_bndry_ele        = 0;
    // init to something b/c the mesh constructor will use this for a 
    // table lookup, even if we don't have boundary info.
    mfem::Geometry::Type bndry_geo = mfem::Geometry::POINT;
    
    if( mesh.has_child("boundary") )
    {
        BP_PLUGIN_INFO("Getting Boundary Element Data");
        const Node &bndry_topo = mesh["boundary"];  
        string bndry_ele_shape = bndry_topo["elements/shape"].as_string();
        bndry_geo     = ElementShapeNameToMFEMShape(bndry_ele_shape);
        bndry_indices = bndry_topo["elements/connectivity"].value();    
        num_bndry_ele = bndry_topo["elements/connectivity"].dtype().number_of_elements();
        num_bndry_ele = num_bndry_ele / ConduitElementShapeSize(bndry_ele_shape);
    }
    else
    {
        BP_PLUGIN_INFO("Skipping Boundary Element Data");
    }

    const int *mesh_atts  = NULL;
    const int *bndry_atts = NULL;

    int num_mesh_atts_entires = 0;
    int num_bndry_atts_entires = 0;

    if( mesh.has_child("mesh_attribute") )
    {
        BP_PLUGIN_INFO("Getting Mesh Attribute Data");
        const Node &n_mesh_atts_vals = mesh["mesh_attribute/values"];
        mesh_atts  = n_mesh_atts_vals.value();
        num_mesh_atts_entires = n_mesh_atts_vals.dtype().number_of_elements();
    }
    else
    {
        BP_PLUGIN_INFO("Skipping Mesh Attribute Data");
    }
    
    if( mesh.has_child("boundary_attribute") )
    {
        BP_PLUGIN_INFO("Getting Boundary Attribute Data");
        bndry_atts = mesh["boundary_attribute/values"].value();
        const Node &n_bndry_atts_vals = mesh["boundary_attribute/values"];
        bndry_atts  = n_bndry_atts_vals.value();
        num_bndry_atts_entires = n_bndry_atts_vals.dtype().number_of_elements();
        
    }
    else
    {
        BP_PLUGIN_INFO("Skipping Boundary Attribute Data");
    }

    BP_PLUGIN_INFO("Number of Vertices: " << num_verts  << endl
                   << "Number of Mesh Elements: "    << num_mesh_ele   << endl
                   << "Number of Boundary Elements: " << num_bndry_ele  << endl
                   << "Number of Mesh Attribute Entries: " 
                   << num_mesh_atts_entires << endl
                   << "Number of Boundary Attribute Entries: " 
                   << num_bndry_atts_entires << endl);

    BP_PLUGIN_INFO("Constructing MFEM Mesh Object");
    mfem::Mesh *res = new mfem::Mesh(// from coordset
                                     verts_indices,
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
                                     ndims,
                                     1); // we need this flag

    BP_PLUGIN_INFO("Attaching Nodes Grid Function");
    
    mfem::GridFunction *nodes = FieldToMFEM(res,
                                            mesh["grid_function"]);
    
    res->NewNodes(*nodes);
    
    BP_PLUGIN_INFO("Done creating MFEM Mesh");
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
//   mesh:  MFEM mesh object
//   lod:   number of refinement steps 
//
//  Programmer: Cyrus Harrison
//  Creation:   Sat Jul  5 11:38:31 PDT 2014
//
// Notes: Adapted from avtMFEMFileFormat and MFEM examples.
//
// ****************************************************************************
vtkDataArray *
avtBlueprintDataAdaptor::MFEM::RefineElementColoringToVTK(mfem::Mesh *mesh,
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
    
    //srandom(time(0)); don't seed, may have side effects for other parts of visit
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


