// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtMFEMDataAdaptor.h>
#include <mfem.hpp>

//-----------------------------------------------------------------------------
// vtk includes
//-----------------------------------------------------------------------------
#include <vtkDoubleArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkFloatArray.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkIdTypeArray.h>
#include <vtkLine.h>
#include <vtkTriangle.h>
#include <vtkHexahedron.h>
#include <vtkQuad.h>
#include <vtkTetra.h>

//-----------------------------------------------------------------------------
// visit includes
//-----------------------------------------------------------------------------
#include "InvalidVariableException.h"

#include "avtMFEMLogging.h"

using namespace mfem;

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
    AVT_MFEM_INFO("Warning: Unsupported Element Shape: " << shape_name);
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
//  Method: LegacyRefineMeshToVTK
//
//  Purpose:
//    Constructs a vtkUnstructuredGrid that contains a refined mfem mesh.
//
//  Arguments:
//    mesh:      MFEM mesh to be refined
//    lod:       number of refinement steps
//
//  Programmer: Cyrus Harrison
//  Creation:   Sat Jul  5 11:38:31 PDT 2014
//
// Notes: Adapted from avtMFEMFileFormat and MFEM examples.
//
//  Modifications:
//    Alister Maguire, Wed Jan 15 09:18:05 PST 2020
//    Casting geom to Geometry::Type where appropariate. This is required
//    with the mfem upgrade to 4.0.
// 
//    Justin Privitera, Wed Apr 13 13:53:06 PDT 2022
//    Renamed RefineMeshToVTK to LegacyRefineMeshToVTK.
// 
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to MFEM data adaptor.
//
// ****************************************************************************

vtkDataSet *
avtMFEMDataAdaptor::LegacyRefineMeshToVTK(mfem::Mesh *mesh,
                                          int domain,
                                          int lod)
{
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
        refined_geo = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
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
        refined_geo = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
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

    // Make some original cell ids so we can try to eliminate the mesh lines.
    std::vector<unsigned int> originalCells;
    originalCells.reserve(neles * 2);
    unsigned int udomain = static_cast<unsigned int>(domain);
    bool doOriginalCells = true;
    
    pt_idx=0;

    for (int i = 0; i <  mesh->GetNE(); i++)
    {
        int geom       = mesh->GetElementBaseGeometry(i);
        int ele_nverts = Geometries.GetVertices(geom)->GetNPoints();
        refined_geo    = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
        Array<int> &rg_idxs = refined_geo->RefGeoms;
        vtkCell *ele_cell = NULL;
        // rg_idxs contains all of the verts for the refined elements
        for (int j = 0; j < rg_idxs.Size(); )
        {
            switch (geom)
            {
                case Geometry::SEGMENT:
                    ele_cell = vtkLine::New();
                    doOriginalCells = false;
                    break;
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

            originalCells.push_back(udomain);
            originalCells.push_back(static_cast<unsigned int>(i));
        }
        pt_idx += refined_geo->RefPts.GetNPoints();
    }

    vtkUnsignedIntArray *ocn = vtkUnsignedIntArray::New();
    ocn->SetName("avtOriginalCellNumbers");
    ocn->SetNumberOfComponents(2);
    ocn->SetNumberOfTuples(originalCells.size()/2);
    memcpy(ocn->GetVoidPointer(0), &originalCells[0],
           sizeof(unsigned int) * originalCells.size());
    res_ds->GetCellData()->AddArray(ocn);
    ocn->Delete();

    return res_ds;
}

// ****************************************************************************
//  Method: LowOrderMeshToVTK
//
//  Purpose:
//    Converts a low order MFEM mesh to a VTK unstructured grid.
//
//  Arguments:
//    mesh:         MFEM mesh to be refined
//
//  Programmer: Justin Privitera
//  Creation:   Wed Apr 13 13:53:06 PDT 2022
// 
//  Modifications:
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to MFEM data adaptor.
//
// ****************************************************************************
vtkDataSet *
avtMFEMDataAdaptor::LowOrderMeshToVTK(mfem::Mesh *mesh)
{
    AVT_MFEM_INFO("Converting Low Order Mesh to VTK.");

    int dim = mesh->SpaceDimension();
    if (dim < 1 || dim > 3)
    {
        AVT_MFEM_EXCEPTION1(InvalidVariableException,
                            "invalid mesh dimension " << dim);
    }

    ////////////////////////////////////////////
    // Setup main coordset
    ////////////////////////////////////////////

    int num_vertices = mesh->GetNV();

    vtkPoints *points = vtkPoints::New();
    points->SetDataTypeToDouble();
    points->SetNumberOfPoints(num_vertices);

    double *coords_ptr = mesh->GetVertex(0);
    for (int i = 0; i < num_vertices; i ++)
    {
        double x = coords_ptr[i * 3];
        double y = dim >= 2 ? coords_ptr[i * 3 + 1] : 0;
        double z = dim >= 3 ? coords_ptr[i * 3 + 2] : 0;
        points->SetPoint(i, x, y, z);
    }

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    points->Delete();

    ////////////////////////////////////////////
    // Setup main topo
    ////////////////////////////////////////////

    vtkCellArray *ca = vtkCellArray::New();
    vtkIdTypeArray *ida = vtkIdTypeArray::New();

    int ncells = mesh->GetNE();
    int geom = mesh->GetElementBaseGeometry(0);
    int idxs_per_ele = mfem::Geometry::NumVerts[geom];

    mfem::Element::Type ele_type = static_cast<mfem::Element::Type>(
        mesh->GetElement(0)->GetType());
    std::string ele_shape = ElementTypeToShapeName(ele_type);
    int ctype = ElementShapeNameToVTKCellType(ele_shape);
    int csize = VTKCellTypeSize(ctype);
    ida->SetNumberOfTuples(ncells * (csize + 1));

    // check our assumptions
    if (idxs_per_ele != csize)
    {
        // Note:
        // ncells = mesh->GetNE() * idxs_per_ele / csize
        // but since the latter two are equal, we can safely
        // say ncells = mesh->GetNE()
        AVT_MFEM_EXCEPTION1(InvalidVariableException,
                            "Expected equality of MFEM and VTK layout variables.");
    }

    for (int i = 0; i < ncells; i ++)
    {
        const int *ele_verts = mesh->GetElement(i)->GetVertices();
        ida->SetComponent((csize + 1) * i, 0, csize);
        for (int j = 0; j < csize; j ++)
        {
            ida->SetComponent((csize + 1) * i + j + 1, 0, ele_verts[j]);
        }
    }

    ca->SetCells(ncells, ida);
    ida->Delete();
    ugrid->SetCells(ctype, ca);
    ca->Delete();
    return ugrid;
}

// ****************************************************************************
//  Method: RefineMeshToVTK
//
//  Purpose:
//    Constructs a vtkUnstructuredGrid that contains a refined mfem mesh.
//
//  Arguments:
//    mesh:        MFEM mesh to be refined
//    lod:         number of refinement steps
//    new_refine:  switch for using the new LOR or legacy LOR
//
//  Programmer: Justin Privitera
//  Creation:   Wed Apr 13 13:53:06 PDT 2022
//
// Notes: See LegacyRefineMeshToVTK for the function originally 
//   with this name.
// 
// Modifications:
//    Justin Privitera, Tue Jul 26 13:04:31 PDT 2022
//    Use new makerefined constructor.
// 
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to MFEM data adaptor.
// 
//    Justin Privitera, Tue Oct 18 09:53:50 PDT 2022
//    Added guards to prevent segfault.
//
// ****************************************************************************
vtkDataSet *
avtMFEMDataAdaptor::RefineMeshToVTK(mfem::Mesh *mesh,
                                    int domain,
                                    int lod,
                                    bool new_refine)
{
    AVT_MFEM_INFO("Creating Refined MFEM Mesh with lod:" << lod);

    if (!new_refine)
    {
        AVT_MFEM_INFO("Using Legacy LOR to refine mesh.");
        return LegacyRefineMeshToVTK(mesh, domain, lod);
    }

    // This logic avoids segfaults
    if (mesh)
    {
        if (mesh->GetNodes())
        {
            if (mesh->GetNodes()->FESpace())
            {
                if (mesh->GetNodes()->FESpace()->FEColl())
                {
                    // Check if the mesh is periodic.
                    const L2_FECollection *L2_coll = dynamic_cast<const L2_FECollection *>
                                                     (mesh->GetNodes()->FESpace()->FEColl());
                    if (L2_coll)
                    {
                        AVT_MFEM_INFO("High Order Mesh is periodic; falling back to Legacy LOR.");
                        return LegacyRefineMeshToVTK(mesh, domain, lod);
                    }
                }
            }
        }
    }

    AVT_MFEM_INFO("High Order Mesh is not periodic.");

    // refine the mesh
    mfem::Mesh lo_mesh = mfem::Mesh::MakeRefined(*mesh, lod, mfem::BasisType::GaussLobatto);

    return LowOrderMeshToVTK(&lo_mesh);
}


// ****************************************************************************
//  Method: BoundaryMeshToVTK
//
//  Purpose:
//    Constructs a vtkUnstructuredGrid that represents an mfem boundary mesh.
//
//  Arguments:
//    mesh:        MFEM mesh
//
//  Programmer: Cyrus Harrison
//  Creation:   Thu Mar  2 09:36:49 PST 2023
//
// ****************************************************************************
vtkDataSet *
avtMFEMDataAdaptor::BoundaryMeshToVTK(mfem::Mesh *mesh)
{
    AVT_MFEM_INFO("Creating Boundary MFEM Mesh");

    // guard vs if we have boundary elements
    if (!mesh->HasBoundaryElements())
    {
       return NULL;
    }

    // FIRST: walk the boundary verts to find those used
    // and create a map from main mesh vert idxs to
    // to a compact set of verts used only for the boundary
    int num_mesh_vertices   = mesh->GetNV();
    // boundary is embedded in same space as main mesh
    int bndry_sdim          = mesh->SpaceDimension();
    int num_bndry_ele       = mesh->GetNBE();
    int bndry_geom          = mesh->GetBdrElement(0)->GetType();
    mfem::Element::Type bndry_ele_type = static_cast<mfem::Element::Type>(
        mesh->GetBdrElement(0)->GetType());
    int bndry_idxs_per_ele  = Geometry::NumVerts[bndry_geom];
    int num_bndry_conn_idxs = num_bndry_ele * bndry_idxs_per_ele;

    AVT_MFEM_INFO("Number of Mesh Vertices  = " << num_mesh_vertices);
    AVT_MFEM_INFO("Number of Boundary Elements  = " << num_bndry_ele);

    int *mesh_verts_used_by_bndry = new int[num_mesh_vertices];
    int *mesh_to_bndry_verts_map  = new int[num_mesh_vertices];

    // init mesh_verts_used_by_bndry
    for (int i=0; i < num_mesh_vertices; i++)
    {
        mesh_verts_used_by_bndry[i] = 0;
        mesh_to_bndry_verts_map[i] = -1;
    }

    // walk to tag mesh_verts_used_by_bndry
    for (int i=0; i < num_bndry_ele; i++)
    {
        const Element *bndry_ele = mesh->GetBdrElement(i);
        const int *bndry_ele_verts = bndry_ele->GetVertices();

        for(int j=0;j< bndry_idxs_per_ele;j++)
        {
            mesh_verts_used_by_bndry[bndry_ele_verts[j]] = 1;
        }
    }

    // count # of boundary verts
    int num_boundary_verts = 0;
    for (int i=0; i < num_mesh_vertices; i++)
    {
        num_boundary_verts += mesh_verts_used_by_bndry[i];
    }

    AVT_MFEM_INFO("Number of Boundary Vertices  = " << num_boundary_verts);

    // gen bndry_to_mesh_verts_map and mesh_to_bndry_verts_map
    int *bndry_to_mesh_verts_map = new int[num_boundary_verts];

    for (int i=0; i < num_boundary_verts; i++)
    {
        bndry_to_mesh_verts_map[i] = -1;
    }

    int bndry_idx = 0;
    for (int i=0; i < num_mesh_vertices; i++)
    {
        if(mesh_verts_used_by_bndry[i] == 1)
        {
            bndry_to_mesh_verts_map[bndry_idx] = i;
            mesh_to_bndry_verts_map[i] = bndry_idx;
            bndry_idx+=1;
        }
    }

    // // if you want to debug these maps
    // std::cout << " mesh_verts_used_by_bndry = " << std::endl;
    // for (int i=0; i < num_mesh_vertices; i++)
    // {
    //     std::cout << mesh_verts_used_by_bndry[i] << " ";
    // }
    // std::cout << endl;
    //
    // std::cout << " mesh_to_bndry_verts_map = " << std::endl;
    // for (int i=0; i < num_mesh_vertices; i++)
    // {
    //     std::cout << mesh_to_bndry_verts_map[i] << " ";
    // }
    // std::cout << endl;
    //
    // std::cout << " bndry_to_mesh_verts_map = "<< std::endl;
    // for (int i=0; i < num_boundary_verts; i++)
    // {
    //     std::cout << bndry_to_mesh_verts_map[i] << " ";
    // }
    // std::cout << endl;

    // done with this bookkeeping array
    delete [] mesh_verts_used_by_bndry;

    ////////////////////////////////////////////
    // Setup bndry points
    ////////////////////////////////////////////

    vtkPoints *points = vtkPoints::New();
    points->SetDataTypeToDouble();
    points->SetNumberOfPoints(num_boundary_verts);

    GridFunction *mesh_gf = mesh->GetNodes();

    if(mesh_gf == NULL)
    {
        AVT_MFEM_INFO("Mesh does not have Nodes Grid Function ");
    }
    else
    {
        AVT_MFEM_INFO("Mesh does have Nodes Grid Function ");
    }

    //-------------------------------------------------------//
    // TODO/NOTE: There may be cases where the vertices exist
    // but are not valid, in that case we will need to
    // use the nodes gf to find the right spatial position
    //
    // See related hints (but not the exact recipe we need):
    // https://github.com/mfem/mfem/issues/861
    //-------------------------------------------------------//

    // extract the verts used in the boundary
    for (int i=0; i < num_boundary_verts; i++)
    {
        int vert_id = bndry_to_mesh_verts_map[i];
        // look up the vert from the mesh to bndry vert map
        double *coords_ptr = mesh->GetVertex(vert_id);
        double x = coords_ptr[0];
        double y = bndry_sdim >= 2 ? coords_ptr[1] : 0;
        double z = bndry_sdim >= 3 ? coords_ptr[2] : 0;
        points->SetPoint(i, x, y, z);
    }

    ////////////////////////////////////////////
    // Setup bndry topo
    ////////////////////////////////////////////
    std::string bndry_shape = ElementTypeToShapeName(bndry_ele_type);
    int bndry_ctype = ElementShapeNameToVTKCellType(bndry_shape);
    int bndry_csize = VTKCellTypeSize(bndry_ctype);

    vtkIdTypeArray *ida = vtkIdTypeArray::New();
    ida->SetNumberOfTuples(num_bndry_ele * (bndry_csize + 1));

    for (int i=0; i < num_bndry_ele; i++)
    {
        const Element *bndry_ele = mesh->GetBdrElement(i);
        const int *bndry_ele_verts = bndry_ele->GetVertices();
        ida->SetComponent((bndry_csize + 1) * i, 0, bndry_csize);
        for(int j=0;j< bndry_idxs_per_ele;j++)
        {
            // bndry mesh idx are in terms of the main set of verts
            // map main mesh vert idx to bndry vert idx
            int bndry_vert_idx = mesh_to_bndry_verts_map[bndry_ele_verts[j]];
            ida->SetComponent((bndry_csize + 1) * i + j + 1, 0, bndry_vert_idx);
        }
    }

    // finish setup of vtk objects
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    points->Delete();
    vtkCellArray *ca = vtkCellArray::New();
    ca->SetCells(num_bndry_ele, ida);
    ida->Delete();
    ugrid->SetCells(bndry_ctype, ca);
    ca->Delete();

    // clean up bookkeeping arrays
    delete [] bndry_to_mesh_verts_map;
    delete [] mesh_to_bndry_verts_map;

    return ugrid;
}


// ****************************************************************************
//  Method: LegacyRefineGridFunctionToVTK
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
//  Modifications:
//    Alister Maguire, Wed Jan 15 09:18:05 PST 2020
//    Casting geom to Geometry::Type where appropariate. This is required
//    with the mfem upgrade to 4.0.
// 
//    Justin Privitera, Fri May  6 15:23:56 PDT 2022
//    Renamed RefineGridFunctionToVTK to LegacyRefineGridFunctionToVTK.
// 
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to MFEM data adaptor.
//
// ****************************************************************************
vtkDataArray *
avtMFEMDataAdaptor::LegacyRefineGridFunctionToVTK(mfem::Mesh *mesh,
                                                  mfem::GridFunction *gf,
                                                  int lod,
                                                  bool var_is_nodal)
{
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
        refined_geo    = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
        npts  += refined_geo->RefPts.GetNPoints();
        neles += refined_geo->RefGeoms.Size() / ele_nverts;
    }

    vtkFloatArray *rv = vtkFloatArray::New();

    int ncomps = gf->VectorDim();

    if(ncomps == 2)
        rv->SetNumberOfComponents(3);
    else
        rv->SetNumberOfComponents(ncomps);

    if(var_is_nodal)
        rv->SetNumberOfTuples(npts);
    else
        rv->SetNumberOfTuples(neles);

    double tuple_vals[9];
    int ref_idx=0;
    for (int i = 0; i <  mesh->GetNE(); i++)
    {
        int geom       = mesh->GetElementBaseGeometry(i);
        refined_geo    = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
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
//  Method: LowOrderGridFunctionToVTK
//
//  Purpose:
//   Converts a low order MFEM grid function to a vtkDataArray.
//
//  Arguments:
//   gf:           MFEM Grid Function for the field
//
//  Programmer: Justin Privitera
//  Creation:   Fri May  6 15:23:56 PDT 2022
//
//  Modifications:
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to MFEM data adaptor.
// 
// ****************************************************************************

vtkDataArray *
avtMFEMDataAdaptor::LowOrderGridFunctionToVTK(mfem::GridFunction *gf)
{
    AVT_MFEM_INFO("Converting Low Order Grid Function To VTK");

    mfem::FiniteElementSpace *fespace = gf->FESpace();
    int vdim = fespace->GetVDim();
    int ndofs = fespace->GetNDofs();

    // all supported grid functions coming out of mfem end up being 
    // associated with vertices

    AVT_MFEM_INFO("VTKDataArray num_tuples = " << ndofs << " "
                    << " num_comps = " << vdim);

    vtkDataArray *retval = vtkDoubleArray::New();
    // vtk reqs us to set number of comps before number of tuples
    retval->SetNumberOfComponents(vdim == 2 ? 3 : vdim);
    // set number of tuples
    retval->SetNumberOfTuples(ndofs);

    const double *values = gf->HostRead();

    if (vdim == 1) // scalar case
    {
        for (vtkIdType i = 0; i < ndofs; i ++)
        {
            retval->SetComponent(i, 0, (double) values[i]);
        }
    }
    else // vector case
    {
        // deal with striding of all components
        bool bynodes = fespace->GetOrdering() == mfem::Ordering::byNODES;
        int stride = bynodes ? 1 : vdim;
        int vdim_stride = bynodes ? ndofs : 1;
        int offset = 0;

        for (int i = 0;  i < vdim; i ++)
        {
            for (vtkIdType j = 0; j < ndofs; j ++)
            {
                retval->SetComponent(j, i, values[offset + j * stride]);
                if(vdim == 2)
                {
                    retval->SetComponent(j, 2, 0.0);
                }
            }
            offset += vdim_stride;
        }
    }

    return retval;
}

// ****************************************************************************
//  Method: RefineGridFunctionToVTK
//
//  Purpose:
//   Constructs a vtkDataArray that contains a refined mfem mesh field variable.
//
//  Arguments:
//   mesh:         MFEM mesh for the field
//   gf:           MFEM Grid Function for the field
//   lod:          number of refinement steps
//   new_refine:   switch for using the new LOR or legacy LOR
//
//  Programmer: Justin Privitera
//  Creation:   Fri May  6 15:23:56 PDT 2022
//
//  Notes: See LegacyRefineGridFunctionToVTK for the function originally 
//   with this name.
// 
//  Modifications:
//     Justin Privitera, Fri Jul 22 16:10:43 PDT 2022
//     Added back in the L2 logic, and fixed it.
//     Use new makerefined constructor.
// 
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to MFEM data adaptor.
// 
//    Justin Privitera, Tue Oct 18 09:53:50 PDT 2022
//    Added logic for determining nodal vs zonal vars.
//    Added guards to prevent segfault.
// 
//    Justin Privitera, Wed Oct 19 15:03:26 PDT 2022
//    Cleaned up nodal/zonal logic to match blueprint plugin.
//
// ****************************************************************************
vtkDataArray *
avtMFEMDataAdaptor::RefineGridFunctionToVTK(mfem::Mesh *mesh,
                                            mfem::GridFunction *gf,
                                            int lod,
                                            bool new_refine,
                                            bool var_is_nodal)
{
    AVT_MFEM_INFO("Creating Refined MFEM Field with lod:" << lod);

    if (!new_refine)
    {
        AVT_MFEM_INFO("Using Legacy LOR to refine grid function.");
        return LegacyRefineGridFunctionToVTK(mesh, gf, lod, var_is_nodal);
    }

    // This logic avoids segfaults
    if (mesh)
    {
        if (mesh->GetNodes())
        {
            if (mesh->GetNodes()->FESpace())
            {
                if (mesh->GetNodes()->FESpace()->FEColl())
                {
                    // Check if the mesh is periodic.
                    const L2_FECollection *L2_coll = dynamic_cast<const L2_FECollection *>
                                                     (mesh->GetNodes()->FESpace()->FEColl());
                    if (L2_coll)
                    {
                        AVT_MFEM_INFO("High Order Mesh is periodic; falling back to Legacy LOR.");
                        return LegacyRefineGridFunctionToVTK(mesh, gf, lod, var_is_nodal);
                    }
                }
            }
        }
    }

    AVT_MFEM_INFO("High Order Mesh is not periodic.");

    mfem::FiniteElementSpace *ho_fes = gf->FESpace();
    if(!ho_fes)
    {
        AVT_MFEM_EXCEPTION1(InvalidVariableException, 
            "RefineGridFunctionToVTK: high order gf finite element space is null");
    }
    // create the low order grid function
    mfem::FiniteElementCollection *lo_col;

    // H1 is nodal
    // L2 is zonal

    std::string basis(gf->FESpace()->FEColl()->Name());  
    // we may have more than just L2 or H1 at this point
    bool l2 = basis.find("L2_") != std::string::npos;
    bool h1 = basis.find("H1_") != std::string::npos;
    bool node_centered;
    if (h1 && l2)
    {
        AVT_MFEM_EXCEPTION1(InvalidVariableException, 
            "RefineGridFunctionToVTK: grid function cannot be both H1 and L2");
    }
    else if (!h1 && !l2) // defer
    {
        AVT_MFEM_INFO("WARNING: RefineGridFunctionToVTK: Grid Function is "
                      "neither H1 nor L2. Deferring to arguments to determine "
                      "if grid function is nodal or zonal.");
        node_centered = var_is_nodal; 
        // the only danger is that var_is_nodal has a default value
        // however, the mfem plugin will always pass var_is_nodal, and the 
        // blueprint plugin always produces h1 or l2.
    }
    // This case will override whatever was passed in for var_is_nodal
    else
        node_centered = h1 && !l2;

    if (node_centered != var_is_nodal)
        AVT_MFEM_INFO("WARNING: RefineGridFunctionToVTK: nodal determination mismatch, is var_is_nodal using default value?")

    if (node_centered)
    {
        lo_col = new mfem::LinearFECollection;
    }
    else
    {
        int p = 0; // single scalar
        lo_col = new mfem::L2_FECollection(p, mesh->Dimension(), 1);
    }
    
    // refine the mesh and convert to vtk
    // it would be nice if this was cached somewhere but we will do it again
    mfem::Mesh lo_mesh = mfem::Mesh::MakeRefined(*mesh, lod, mfem::BasisType::GaussLobatto);
    mfem::FiniteElementSpace lo_fes(&lo_mesh, lo_col, ho_fes->GetVDim());
    mfem::GridFunction lo_gf(&lo_fes);
    // transform the higher order function to a low order function somehow
    mfem::OperatorHandle hi_to_lo;
    lo_fes.GetTransferOperator(*ho_fes, hi_to_lo);
    hi_to_lo.Ptr()->Mult(*gf, lo_gf);

    vtkDataArray *retval = LowOrderGridFunctionToVTK(&lo_gf);
    
    delete lo_col;

    return retval;
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
//  Modifications:
//    Alister Maguire, Wed Jan 15 09:18:05 PST 2020
//    Casting geom to Geometry::Type where appropariate. This is required
//    with the mfem upgrade to 4.0.
// 
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to MFEM data adaptor.
//
// ****************************************************************************
vtkDataArray *
avtMFEMDataAdaptor::RefineElementColoringToVTK(mfem::Mesh *mesh,
                                               int domain_id,
                                               int lod)
{
    AVT_MFEM_INFO("Creating Refined MFEM Element Coloring with lod:" << lod);
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
        refined_geo    = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
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
        refined_geo= GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
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
//  Modifications:
//    Alister Maguire, Wed Jan 15 09:18:05 PST 2020
//    Casting geom to Geometry::Type where appropariate. This is required
//    with the mfem upgrade to 4.0.
// 
//    Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//    Moved from blueprint plugin to MFEM data adaptor.
//
// ****************************************************************************
vtkDataArray *
avtMFEMDataAdaptor::RefineElementAttributeToVTK(mfem::Mesh *mesh,
                                                int lod)
{
    AVT_MFEM_INFO("Creating Refined MFEM Element Attribute with lod:" << lod);
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
        refined_geo    = GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
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
        refined_geo= GlobGeometryRefiner.Refine((Geometry::Type)geom, lod, 1);
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
//  Method: BoundaryAttributeToVTK
//
//  Purpose:
//   Constructs a vtkDataArray that contains the "attribute" value
//   for a mfem mesh boundary.
//
//  Arguments:
//   mesh:      MFEM mesh object
//
//  Programmer: Cyrus Harrison
//  Creation:   Thu Mar  2 09:42:34 PST 2023
//
//  Modifications:
//
// ****************************************************************************
vtkDataArray *
avtMFEMDataAdaptor::BoundaryAttributeToVTK(mfem::Mesh *mesh)
{
    AVT_MFEM_INFO("Creating MFEM Boundary Attribute")

    // guard vs if we have boundary elements
    if (!mesh->HasBoundaryElements())
    {
       return NULL;
    }

    int num_bndry_ele = mesh->GetNBE();

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(1);
    rv->SetNumberOfTuples(num_bndry_ele);

    for (int i = 0; i < num_bndry_ele; i++)
    {
        rv->SetTuple1(i,mesh->GetBdrAttribute(i));
    }

    return rv;
}

