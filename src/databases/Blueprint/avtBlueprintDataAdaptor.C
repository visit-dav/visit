// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
// avtBlueprintDataAdaptor.C
// ************************************************************************* //


#include "avtBlueprintDataAdaptor.h"

// NOTE: This is from avtblueprint lib
#include "avtConduitBlueprintDataAdaptor.h"
// NOTE: This is from avtmfem lib
#include "avtMFEMDataAdaptor.h"

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
#include "Expression.h"
#include "ExpressionList.h"
#include "UnexpectedValueException.h"

#include "avtMaterial.h"
#include "avtMixedVariable.h"


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
      // not yet supported:
      case Element::WEDGE:       return "unknown";
      
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
