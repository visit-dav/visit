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
#include <vtkWedge.h>
#include <vtkPyramid.h>


//-----------------------------------------------------------------------------
// visit includes
//-----------------------------------------------------------------------------
#include "InvalidVariableException.h"
#include <StringHelpers.h>

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
//
// Modifications:
//  Cyrus Harrison, Mon Oct 28 16:31:26 PDT 2024
//  Added support for wedges (prisms) and pyramids.
//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
std::string
ElementTypeToShapeName(Element::Type element_type)
{
   // Note -- the mapping from Element::Type to string is based on
   //   enum Element::Type { POINT, SEGMENT, TRIANGLE, QUADRILATERAL,
   //                        TETRAHEDRON, HEXAHEDRON, WEDGE, PYRAMID};
   // Note: -- the string names are from conduit's blueprint

   switch (element_type)
   {
      case Element::POINT:          return "point";
      case Element::SEGMENT:        return "line";
      case Element::TRIANGLE:       return "tri";
      case Element::QUADRILATERAL:  return "quad";
      case Element::TETRAHEDRON:    return "tet";
      case Element::HEXAHEDRON:     return "hex";
      case Element::WEDGE:          return "wedge";
      case Element::PYRAMID:        return "pyramid";
   }
   

   return "unknown";
}

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
    if (cell_type == VTK_WEDGE)      return 6;
    if (cell_type == VTK_PYRAMID)    return 5;
    return 0;
}

// ****************************************************************************
//  Method: DiscontinuousRefineMeshToVTK
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
//    Justin Privitera, Wed Dec 17 14:01:55 PST 2025
//    Renamed LegacyRefineMeshToVTK to DiscontinuousRefineMeshToVTK.
//
// ****************************************************************************

vtkDataSet *
avtMFEMDataAdaptor::DiscontinuousRefineMeshToVTK(mfem::Mesh *mesh,
                                                 const int domain,
                                                 const int lod)
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
                case Geometry::PRISM:        ele_cell = vtkWedge::New();      break;
                case Geometry::PYRAMID:      ele_cell = vtkPyramid::New();    break;
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
//    mesh:            MFEM mesh to be refined
//    domain:          domain id
//    lod:             number of refinement steps
//    mesh_ref_method: chosen MFEM mesh refinement method
//    ref_basis_type:  chosen MFEM refinement basis method
//
//  Programmer: Justin Privitera
//  Creation:   Wed Apr 13 13:53:06 PDT 2022
//
// Notes: See DiscontinuousRefineMeshToVTK for the function originally 
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
//    Cyrus Harrison, Fri Mar 10 11:58:33 PST 2023
//    Add original cell ids, so mesh plots render outlines of the
//    high order elements.
// 
//    Justin Privitera, Wed Oct 30 14:18:31 PDT 2024
//    Simplified test for periodic meshes and added comments.
// 
//    Justin Privitera, Wed Dec 17 14:01:55 PST 2025
//    Rewrote this method to handle new LOR options, new basis types, and
//    conversions to low order.
//
// ****************************************************************************
vtkDataSet *
avtMFEMDataAdaptor::RefineMeshToVTK(mfem::Mesh *mesh,
                                    int domain,
                                    int lod,
                                    const meshRefinementMethod mesh_ref_method,
                                    const refinementBasisType ref_basis_type)
{
    // discontinuous LOR -> forces discontinuous refine
    // default LOR -> continuous refine unless mesh is periodic (see below)
    // continuous LOR -> forces continuous refine

    AVT_MFEM_INFO("Creating Refined MFEM Mesh with lod:" << lod);

    if (meshRefinementMethod::Discontinuous_LOR == mesh_ref_method)
    {
        AVT_MFEM_INFO("Using Legacy LOR to refine mesh.");
        return DiscontinuousRefineMeshToVTK(mesh, domain, lod);
    }

    if (mesh && mesh->GetNodalFESpace() && mesh->GetNodalFESpace()->IsDGSpace())
    {
        // This if-test condition was taken from here:
        // https://github.com/orgs/mfem/discussions/4556#discussioncomment-11093211

        // Periodic meshes don't play nicely with the new mesh refinement method.
        // They actually refine just fine, but what is produced is also periodic.
        // We need to disconnect the mesh to view it. In lieu of some algorithm
        // that disconnects the result for us, we choose to fall back to legacy
        // LOR.

        // The problem is, there is no easy way to tell if a mesh is periodic.
        // We know that all periodic meshes are L2, but not all L2 meshes are
        // periodic. So our best bet is to catch all L2 meshes and fall back
        // to legacy LOR.

        if (meshRefinementMethod::Default_LOR == mesh_ref_method)
        {
            AVT_MFEM_INFO("High Order Mesh may be periodic and default "
                          "projection has been selected; falling back to "
                          "Legacy LOR.");
            return DiscontinuousRefineMeshToVTK(mesh, domain, lod);
        }
    }
    else
    {
        AVT_MFEM_INFO("High Order Mesh is not periodic.");
    }

    // mfem::Mesh::MakeRefined does not yet support pyramids
    if(mesh->GetNE() > 0)
    {
        if(mesh->GetElement(0)->GetType() == mfem::Element::PYRAMID)
        {
            AVT_MFEM_EXCEPTION1(InvalidVariableException,
                                "Current MFEM implementation does not support refining Meshes with Pyramids.");
        }
    }

    // refine the mesh
    mfem::Mesh lo_mesh;
    if (refinementBasisType::Gauss_Lobatto_Default == ref_basis_type)
    {
        lo_mesh = mfem::Mesh::MakeRefined(*mesh, lod, mfem::BasisType::GaussLobatto);
    }
    else if (refinementBasisType::Closed_Uniform == ref_basis_type)
    {
        lo_mesh = mfem::Mesh::MakeRefined(*mesh, lod, mfem::BasisType::ClosedUniform);
    }
    else
    {
        AVT_MFEM_EXCEPTION1(InvalidVariableException,
                            "Unknown MFEM LOR refinement basis type: " << ref_basis_type);
    }

    vtkDataSet *res_ds = LowOrderMeshToVTK(&lo_mesh);

    /// ----------------
    // add original cell ids, which associate our refined elements with the
    // original mfem elements.
    /// ----------------
    // NOTE:
    //  This counting pattern follows the implementation of
    // mfem::Mesh::MakeRefined. If its implementation changes significantly,
    // we will have to adapt this logic as well.
    /// ----------------

    int orig_nelems = mesh->GetNE();

    GeometryRefiner refiner;
    if (refinementBasisType::Gauss_Lobatto_Default == ref_basis_type)
    {
        refiner.SetType(BasisType::GetQuadrature1D(mfem::BasisType::GaussLobatto));
    }
    else if (refinementBasisType::Closed_Uniform == ref_basis_type)
    {
        refiner.SetType(BasisType::GetQuadrature1D(mfem::BasisType::ClosedUniform));
    }
    else
    {
        AVT_MFEM_EXCEPTION1(InvalidVariableException,
                            "Unknown MFEM LOR refinement basis type: " << ref_basis_type);
    }

    int lor_nelems = lo_mesh.GetNE();

    vtkUnsignedIntArray *orig_cell_ids = vtkUnsignedIntArray::New();
    orig_cell_ids->SetName("avtOriginalCellNumbers");
    orig_cell_ids->SetNumberOfComponents(2);
    orig_cell_ids->SetNumberOfTuples(lor_nelems);

    unsigned int *orig_cell_ids_ptr = orig_cell_ids->GetPointer(0);

    int orig_cell_ids_idx=0;
    // assoc refined elements with orig element
    for (int el = 0; el < orig_nelems; el++)
    {
       Geometry::Type geom = mesh->GetElementGeometry(el);

       int nvert = Geometry::NumVerts[geom];
       RefinedGeometry &RG = *refiner.Refine(geom, lod);

       for (int j = 0; j < RG.RefGeoms.Size()/nvert; j++)
       {
           orig_cell_ids_ptr[orig_cell_ids_idx] = static_cast<unsigned int>(domain);
           orig_cell_ids_ptr[orig_cell_ids_idx+1] = static_cast<unsigned int>(el);
           orig_cell_ids_idx+=2;
       }
    }

    res_ds->GetCellData()->AddArray(orig_cell_ids);
    orig_cell_ids->Delete();

    return res_ds;
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
    // Need future work to address this.
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
//  Method: QuadratureFunctionMeshToVTK
//
//  Purpose:
//    Constructs a vtkUnstructuredGrid that represents an mfem quad pts mesh.
//
//  Arguments:
//    mesh:        MFEM mesh
//.   order:       Quad Func Order
//
//  Programmer: Cyrus Harrison
//  Creation:   Fri Sep 26 09:16:26 PDT 2025
// 
//  Modifications:
//    Justin Privitera, Wed Dec 17 14:01:55 PST 2025
//    Use refinement basis type.
//
// ****************************************************************************
vtkDataSet *
avtMFEMDataAdaptor::QuadratureFunctionMeshToVTK(mfem::Mesh *mesh,
                                                int order,
                                                const refinementBasisType ref_basis_type)
{
    vtkDataSet *rv = nullptr;
   
    // refine the mesh
    // gauss p points, we use gauss lobatto lor'd mesh with p + 1
    // we use the mfem quad function to get the right numbers

    ///
    // this logic for order and ref_factor is from glviz
    // assume identical order
    // const int order = quad_f->GetIntRule(0).GetOrder()/2; // <-- Gauss-Legendre
    //
    // The order which can be perfectly integated is equal to (2 * p -1) quad points
    // gauss lobatto lor'd mesh with p + 1 == order / 2 +1
    const int qpts_order = order / 2;
    const int ref_factor = qpts_order + 1;
    ///
    
    // Note:
    // mfem::BasisType::ClosedGL is what glviz uses
    mfem::Mesh lo_mesh;
    if (refinementBasisType::Gauss_Lobatto_Default == ref_basis_type)
    {
        lo_mesh = mfem::Mesh::MakeRefined(*mesh, ref_factor, mfem::BasisType::GaussLobatto);
    }
    else if (refinementBasisType::Closed_Uniform == ref_basis_type)
    {
        lo_mesh = mfem::Mesh::MakeRefined(*mesh, ref_factor, mfem::BasisType::ClosedUniform);
    }
    else
    {
        AVT_MFEM_EXCEPTION1(InvalidVariableException,
                            "Unknown MFEM LOR refinement basis type: " << ref_basis_type);
    }
    

    vtkDataSet *res_ds = LowOrderMeshToVTK(&lo_mesh);
    return res_ds;
}

// ****************************************************************************
//  Method: DiscontinuousRefineGridFunctionToVTK
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
//    Justin Privitera, Wed Dec 17 14:01:55 PST 2025
//    Renamed LegacyRefineGridFunctionToVTK to
//    DiscontinuousRefineGridFunctionToVTK.
//
// ****************************************************************************
vtkDataArray *
avtMFEMDataAdaptor::DiscontinuousRefineGridFunctionToVTK(mfem::Mesh *mesh,
                                                         mfem::GridFunction *gf,
                                                         const int lod,
                                                         const bool var_is_nodal)
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

    const int ncomps = gf->VectorDim();

    if(ncomps == 2)
        rv->SetNumberOfComponents(3);
    else
        rv->SetNumberOfComponents(ncomps);

    if (var_is_nodal)
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
//    Justin Privitera, Wed Dec 17 14:01:55 PST 2025
//    Properly handle ncomps.
// 
// ****************************************************************************

vtkDataArray *
avtMFEMDataAdaptor::LowOrderGridFunctionToVTK(mfem::GridFunction *gf)
{
    AVT_MFEM_INFO("Converting Low Order Grid Function To VTK");

    mfem::FiniteElementSpace *fespace = gf->FESpace();
    const int ncomps = fespace->GetVectorDim();
    const int ndofs = fespace->GetNDofs();

    AVT_MFEM_INFO("VTKDataArray num_tuples = " << ndofs << " "
                    << " num_comps = " << ncomps);

    vtkDataArray *retval = vtkDoubleArray::New();
    // vtk reqs us to set number of comps before number of tuples
    retval->SetNumberOfComponents(ncomps == 2 ? 3 : ncomps);
    // set number of tuples
    retval->SetNumberOfTuples(ndofs);

    const double *values = gf->HostRead();

    if (ncomps == 1) // scalar case
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
        int stride = bynodes ? 1 : ncomps;
        int ncomps_stride = bynodes ? ndofs : 1;
        int offset = 0;

        for (int i = 0;  i < ncomps; i ++)
        {
            for (vtkIdType j = 0; j < ndofs; j ++)
            {
                retval->SetComponent(j, i, values[offset + j * stride]);
                if(ncomps == 2)
                {
                    retval->SetComponent(j, 2, 0.0);
                }
            }
            offset += ncomps_stride;
        }
    }

    return retval;
}

// ****************************************************************************
mfem::GridFunction *
ConvertGridFunctionToScalar(mfem::GridFunction *org_gf,
                            const avtMFEMDataAdaptor::fieldProjectionMethod field_proj_method,
                            const avtMFEMDataAdaptor::refinementBasisType ref_basis_type)
{
    mfem::Mesh *mesh = org_gf->FESpace()->GetMesh();
    const int dim = mesh->Dimension();
    const int p = org_gf->FESpace()->GetMaxElementOrder();

    FiniteElementCollection *new_fec = nullptr;
    if (avtMFEMDataAdaptor::refinementBasisType::Gauss_Lobatto_Default == ref_basis_type)
    {
        if (avtMFEMDataAdaptor::fieldProjectionMethod::Nodal_Projection == field_proj_method)
        {
            new_fec = new H1_FECollection(p, dim, BasisType::GaussLobatto);
        }
        else if (avtMFEMDataAdaptor::fieldProjectionMethod::Zonal_Projection == field_proj_method)
        {
            new_fec = new L2_FECollection(p, dim, BasisType::GaussLobatto);
        }
        else
        {
            AVT_MFEM_EXCEPTION1(InvalidVariableException,
                                "Unknown MFEM LOR field projection type: " << ref_basis_type);
        }
    }
    else if (avtMFEMDataAdaptor::refinementBasisType::Closed_Uniform == ref_basis_type)
    {
        if (avtMFEMDataAdaptor::fieldProjectionMethod::Nodal_Projection == field_proj_method)
        {
            new_fec = new H1_FECollection(p, dim, BasisType::ClosedUniform);
        }
        else if (avtMFEMDataAdaptor::fieldProjectionMethod::Zonal_Projection == field_proj_method)
        {
            new_fec = new L2_FECollection(p, dim, BasisType::ClosedUniform);
        }
        else
        {
            AVT_MFEM_EXCEPTION1(InvalidVariableException,
                                "Unknown MFEM LOR field projection type: " << ref_basis_type);
        }
    }
    else
    {
        AVT_MFEM_EXCEPTION1(InvalidVariableException,
                            "Unknown MFEM LOR refinement basis type: " << ref_basis_type);
    }
    FiniteElementSpace *new_fes =
        new FiniteElementSpace(mesh, new_fec, org_gf->VectorDim());
    GridFunction *new_gf = new GridFunction(new_fes);
    new_gf->MakeOwner(new_fec);

    if (org_gf->FESpace()->FEColl()->GetRangeType(dim) ==
        FiniteElement::RangeType::VECTOR )
    {
       org_gf->ProjectVectorFieldOn(*new_gf);
    }
    else if ( org_gf->FESpace()->GetVectorDim() == 1 )
    {
       mfem::GridFunctionCoefficient gf_cf(org_gf);
       new_gf->ProjectCoefficient(gf_cf);
    }
    else
    {
       mfem::VectorGridFunctionCoefficient gf_cf(org_gf);
       new_gf->ProjectCoefficient(gf_cf);
    }
    return new_gf;
}



// ****************************************************************************
//  Method: RefineGridFunctionToVTK
//
//  Purpose:
//   Constructs a vtkDataArray that contains a refined mfem mesh field variable.
//
//  Arguments:
//   mesh:              MFEM mesh for the field
//   gf:                MFEM Grid Function for the field
//   lod:               number of refinement steps
//   mesh_ref_method:   chosen MFEM mesh refinement method
//   field_proj_method: chosen MFEM grid function projetion method
//   ref_basis_type:    chosen MFEM refinement basis method
//   cent_change:       records if a centering change has taken place due to the
//                      refinement options
//   var_is_nodal:      a guess if the resulting variable will be nodal or not
//
//  Programmer: Justin Privitera
//  Creation:   Fri May  6 15:23:56 PDT 2022
//
//  Notes: See DiscontinuousRefineGridFunctionToVTK for the function originally 
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
//    Justin Privitera, Wed Oct 30 14:18:31 PDT 2024
//    Simplified test for periodic meshes and added comments.
// 
//    Justin Privitera, Wed Dec 17 14:01:55 PST 2025
//    Rewrote this method to handle new LOR options, new basis types, and
//    conversions to low order.
//
// ****************************************************************************
vtkDataArray *
avtMFEMDataAdaptor::RefineGridFunctionToVTK(mfem::Mesh *mesh,
                                            mfem::GridFunction *gf,
                                            const int lod,
                                            const meshRefinementMethod mesh_ref_method,
                                            const fieldProjectionMethod field_proj_method,
                                            const refinementBasisType ref_basis_type,
                                            avtCentering &cent_change,
                                            bool var_is_nodal)
{
    AVT_MFEM_INFO("Creating Refined MFEM Field with meshRefinementMethod: " << mesh_ref_method);
    AVT_MFEM_INFO("Creating Refined MFEM Field with fieldProjectionMethod: " << field_proj_method);
    AVT_MFEM_INFO("Creating Refined MFEM Field with refinementBasisType: " << ref_basis_type);
    AVT_MFEM_INFO("Creating Refined MFEM Field with lod: " << lod);

    if (meshRefinementMethod::Discontinuous_LOR == mesh_ref_method)
    {
        if (fieldProjectionMethod::Zonal_Projection == field_proj_method)
        {
            AVT_MFEM_EXCEPTION1(InvalidVariableException,
                "Zonal projection together with discontinuous low-order-refinement is not supported.");
        }

        AVT_MFEM_INFO("Using Legacy LOR to refine grid function.");
        cent_change = AVT_NODECENT;
        return DiscontinuousRefineGridFunctionToVTK(mesh, gf, lod, var_is_nodal);
    }

    if (mesh && mesh->GetNodalFESpace() && mesh->GetNodalFESpace()->IsDGSpace())
    {
        // This if-test condition was taken from here:
        // https://github.com/orgs/mfem/discussions/4556#discussioncomment-11093211

        // Periodic meshes don't play nicely with the new mesh refinement method.
        // They actually refine just fine, but what is produced is also periodic.
        // We need to disconnect the mesh to view it. In lieu of some algorithm
        // that disconnects the result for us, we choose to fall back to legacy
        // LOR.

        // The problem is, there is no easy way to tell if a mesh is periodic.
        // We know that all periodic meshes are L2, but not all L2 meshes are
        // periodic. So our best bet is to catch all L2 meshes and fall back
        // to legacy LOR.

        if (meshRefinementMethod::Default_LOR == mesh_ref_method)
        {
            if (fieldProjectionMethod::Zonal_Projection == field_proj_method)
            {
                AVT_MFEM_EXCEPTION1(InvalidVariableException,
                    "Zonal projection together with discontinuous low-order-refinement is not supported.");
            }
            AVT_MFEM_INFO("High Order Mesh may be periodic and default "
                          "refinement has been selected; falling back to "
                          "Legacy LOR.");
            cent_change = AVT_NODECENT;
            return DiscontinuousRefineGridFunctionToVTK(mesh, gf, lod, var_is_nodal);
        }
    }
    else
    {
        AVT_MFEM_INFO("High Order Mesh is not periodic.");
    }

    if(!gf->FESpace())
    {
        AVT_MFEM_EXCEPTION1(InvalidVariableException, 
            "RefineGridFunctionToVTK: high order gf finite element space is null");
    }

    // extract basis type information
    std::string basis(gf->FESpace()->FEColl()->Name());
    bool h1    = basis.find("H1_")   != std::string::npos; // nodal
    bool l2    = basis.find("L2_")   != std::string::npos; // zonal
    bool hdiv  = basis.find("RT_")   != std::string::npos;
    bool hcurl = basis.find("ND_")   != std::string::npos;
    bool nurbs = basis.find("NURBS") != std::string::npos;

    int bases = static_cast<int>(l2) +
                static_cast<int>(h1) +
                static_cast<int>(hdiv) +
                static_cast<int>(hcurl) +
                static_cast<int>(nurbs);

    // go ahead and fall back to var_is_nodal guess
    if (0 == bases)
    {
        if (var_is_nodal)
        {
            h1 = true;
        }
        else
        {
            l2 = true;
        }
        bases = 1;
    }

    // we must enforce only a single basis type
    if (1 != bases)
    {
        AVT_MFEM_EXCEPTION1(InvalidVariableException, 
            "RefineGridFunctionToVTK: grid function may not have multiple basis types. "
            "Unsupported basis type in " << basis);
    }

    // We will need to resolve the default field projection method to either zonal or nodal,
    // so we include another variable to store the resulting field projection method.
    fieldProjectionMethod field_proj_method_to_use = field_proj_method;

    // select projection type and convert gridfunction when necessary
    mfem::GridFunction *gf_to_use = gf;
    bool delete_gf_to_use = false;
    if (fieldProjectionMethod::Default_Projection == field_proj_method)
    {
        if (h1)
        {
            // default h1 -> nodal
            field_proj_method_to_use = fieldProjectionMethod::Nodal_Projection;
        }
        else if (l2)
        {
            // default l2 -> zonal
            field_proj_method_to_use = fieldProjectionMethod::Zonal_Projection;
        }
        else if (hdiv || hcurl)
        {
            // default hdiv, hcurl -> zonal
            field_proj_method_to_use = fieldProjectionMethod::Zonal_Projection;
            gf_to_use = ConvertGridFunctionToScalar(gf, field_proj_method_to_use, ref_basis_type); // Convert to L2
            delete_gf_to_use = true;
        }
        else if (nurbs)
        {
            // default nurbs -> nodal
            field_proj_method_to_use = fieldProjectionMethod::Nodal_Projection;
            gf_to_use = ConvertGridFunctionToScalar(gf, field_proj_method_to_use, ref_basis_type); // Convert to H1
            delete_gf_to_use = true;
        }
    }
    else if (fieldProjectionMethod::Nodal_Projection == field_proj_method ||
             fieldProjectionMethod::Zonal_Projection == field_proj_method)
    {
        field_proj_method_to_use = field_proj_method;
        // whatever refinement method we end up using, we cannot directly refine hdiv, hcurl, and nurbs
        if (hdiv || hcurl || nurbs)
        {
            // we must convert to either h1 or l2 using the specified refinement method
            gf_to_use = ConvertGridFunctionToScalar(gf, field_proj_method_to_use, ref_basis_type);
            delete_gf_to_use = true;
        }
    }
    else
    {
        AVT_MFEM_EXCEPTION1(InvalidVariableException,
                            "Unknown MFEM LOR refinement method: " << field_proj_method_to_use);
    }

    // create the low order grid function
    mfem::FiniteElementCollection *lo_col;
    if (fieldProjectionMethod::Nodal_Projection == field_proj_method_to_use)
    {
        // convert to H1
        lo_col = new mfem::LinearFECollection;
        // node centered
        cent_change = AVT_NODECENT;
    }
    else if (fieldProjectionMethod::Zonal_Projection == field_proj_method_to_use)
    {
        // convert to L2
        lo_col = new mfem::L2_FECollection(0, mesh->Dimension(), 1);
        // element centered
        cent_change = AVT_ZONECENT;
    }
    else
    {
        AVT_MFEM_EXCEPTION1(InvalidVariableException,
                            "Unknown MFEM LOR refinement method: " << field_proj_method_to_use);
    }
    
    // refine the mesh and convert to vtk
    // it would be nice if this was cached somewhere but we will do it again
    mfem::Mesh lo_mesh;
    if (refinementBasisType::Gauss_Lobatto_Default == ref_basis_type)
    {
        lo_mesh = mfem::Mesh::MakeRefined(*mesh, lod, mfem::BasisType::GaussLobatto);
    }
    else if (refinementBasisType::Closed_Uniform == ref_basis_type)
    {
        lo_mesh = mfem::Mesh::MakeRefined(*mesh, lod, mfem::BasisType::ClosedUniform);
    }
    else
    {
        AVT_MFEM_EXCEPTION1(InvalidVariableException,
                            "Unknown MFEM LOR refinement basis type: " << ref_basis_type);
    }
    mfem::FiniteElementSpace lo_fes(&lo_mesh, lo_col, gf_to_use->FESpace()->GetVectorDim());
    mfem::GridFunction lo_gf(&lo_fes);
    // transform the higher order function to a low order function
    // using a matrix free transfer operator
    mfem::OperatorHandle hi_to_lo(mfem::Operator::ANY_TYPE);
    lo_fes.GetTransferOperator(*gf_to_use->FESpace(), hi_to_lo);
    hi_to_lo.Ptr()->Mult(*gf_to_use, lo_gf);

    vtkDataArray *retval = LowOrderGridFunctionToVTK(&lo_gf);

    if (delete_gf_to_use)
    {
        delete gf_to_use;
    }

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
// ****************************************************************************
//  Method: BoundaryAttributeToVTK
//
//  Purpose:
//   Constructs a vtkDataArray that contains the quad points values
//   for a mfem mesh. These are piece wise constant.
//
//  Arguments:
//   qf:      MFEM quad function object
//
//  Programmer: Cyrus Harrison
//  Creation:   Fri Sep 26 09:16:26 PDT 2025
//
//  Modifications:
//
// ****************************************************************************
vtkDataArray *
avtMFEMDataAdaptor::QuadratureFunctionToVTK(mfem::QuadratureFunction *qf)
{
    AVT_MFEM_INFO("Creating MFEM Quadrature Function")

    //mfem::QuadratureSpace *qspace = qf->FESpace();
    const int vdim = qf->GetVDim();
    const int qfsize = qf->Size();
    const int ntuples = qfsize / vdim;

    const double *values = qf->HostRead();

    AVT_MFEM_INFO("VTKDataArray "
                    << " quad function mesh elements: " << ntuples
                    << " vdim: " << vdim << " "
                    << " quad function total size: " << qfsize);

    vtkDataArray *retval = vtkDoubleArray::New();
    // vtk reqs us to set number of comps before number of tuples
    retval->SetNumberOfComponents(vdim == 2 ? 3 : vdim);
    // set number of tuples
    retval->SetNumberOfTuples(ntuples);

    if (vdim == 1) // scalar case
    {
        for (vtkIdType i = 0; i < ntuples; i ++)
        {
            retval->SetComponent(i, 0, (double) values[i]);
        }
    }
    else // vector case
    {
        // NOTE: quad function is strided by vdims
        int idx = 0;
        for (vtkIdType j = 0; j < ntuples; j ++)
        {
            for (int i = 0;  i < vdim; i ++)
            {
                retval->SetComponent(j, i, values[idx]);
                idx++;
            }
            // vtk always wants 3d for vector fields
            if(vdim == 2)
            {
                retval->SetComponent(j, 2, 0.0);
            }
        }

    }

    return retval;
}

// ****************************************************************************
//  Method: CheckBasisStringForQuadratureFunction
//
//  Purpose:
//   Checks if the basis string has a `QF_` prefix, which indicates
//   a QuadratureFunction
//
//  Arguments:
//   basis:     MFEM style basis string
//
//  Programmer: Cyrus Harrison
//  Creation:   Fri Oct 10 15:09:42 PDT 2025
//
//  Modifications:
//
// ****************************************************************************
bool
avtMFEMDataAdaptor::CheckBasisStringForQuadratureFunction(const std::string &basis)
{
    return basis.find("QF_") != std::string::npos;
}

// ****************************************************************************
//  Method: ParseQuadratureFunctionBasisString
//
//  Purpose:
//   Parse Order and VDim details from a Quadrature Function style basis
//   string.
//
//  Arguments:
//   basis:     MFEM style basis string
//   qf_order:  Order result
//   qf_vdim:   VDim Result
//
//  Programmer: Cyrus Harrison
//  Creation:   Fri Oct 10 15:09:42 PDT 2025
//
//  Modifications:
//   Cyrus Harrison, Tue Feb 17 09:46:24 PST 2026
//   Expand the string pattern to support QF_{TYPE}_{ORDER}_{VDIM} style
//
// ****************************************************************************
void
avtMFEMDataAdaptor::ParseQuadratureFunctionBasisString(const std::string &basis,
                                                       int &qf_order,
                                                       int &qf_vdim)
{
    // the pattern used to encode the quad space params is:
    //  QF_{ORDER}_{VDIM}
    // or
    //  QF_{TYPE}_{ORDER}_{VDIM}
    //
    // ORDER is the degree of the polynmials for the quad rule
    // VDIM  is the number of components at each quad point (scalar, vector, etc)
    //
    // Note: Order == 2*(quad points) -1
    //
    // split to parse
    std::vector<std::string> toks = StringHelpers::split(basis,'_');

    // there should be 3 or 4 tokens
    if(toks.size() != 3 && toks.size() != 4)
    {
        //bad qf basis string
        AVT_MFEM_EXCEPTION1(InvalidVariableException,
                            "Invalid quadrature function basis string: " << basis  << std::endl
                            << "Expected: QF_{ORDER}_{VDIM} or QF_{TYPE}_{ORDER}_{VDIM}");
    }

    int order_index = toks.size() - 2;
    int vdim_index  = toks.size() - 1;

    // ORDER
    if(!StringHelpers::StringToInt(toks[order_index],qf_order))
    {
        // error
        AVT_MFEM_EXCEPTION1(InvalidVariableException,
                            "Failed to parse quadrature function order from " << toks[order_index]);
    }
    // VDIM
    if(!StringHelpers::StringToInt(toks[vdim_index],qf_vdim))
    {
        // error
        AVT_MFEM_EXCEPTION1(InvalidVariableException,
                            "Failed to parse quadrature function vdim from " << toks[vdim_index]);
    }
}

// ****************************************************************************
//  Method: GenerateQuadratureFunctionBasisString
//
//  Purpose:
//   Create a Quadrature Function Basis String from a QuadratureFunction
//   object.
//
//  Arguments:
//   qf:     MFEM QuadratureFunction object
//
//  Programmer: Cyrus Harrison
//  Creation:   Fri Oct 10 15:09:42 PDT 2025
//
//  Modifications:
//
// ****************************************************************************
std::string
avtMFEMDataAdaptor::GenerateQuadratureFunctionBasisString(mfem::QuadratureFunction *qf)
{
    return GenerateQuadratureFunctionBasisString(qf->GetSpace()->GetOrder(),
                                                 qf->GetVDim());
}
// ****************************************************************************
//  Method: GenerateQuadratureFunctionBasisString
//
//  Purpose:
//   Create a Quadrature Function Basis String for a given order and vdim.
//
//  Arguments:
//   qf_order:  Order
//   qf_vdim:   VDim
//
//  Programmer: Cyrus Harrison
//  Creation:   Fri Oct 10 15:09:42 PDT 2025
//
//  Modifications:
//
// ****************************************************************************
std::string
avtMFEMDataAdaptor::GenerateQuadratureFunctionBasisString(int qf_order,
                                                          int qf_vdim)
{
    std::ostringstream oss;
    oss << "QF_"<< qf_order << "_" << qf_vdim;
    return oss.str();
}

// ****************************************************************************
//  Method: CheckMeshNameForQuadratureFunctionString
//
//  Purpose:
//   Checks if the mesh name represents a QuadratureFunction mesh.
//
//  Arguments:
//   mesh_name:     Mesh Name
//
//  Programmer: Cyrus Harrison
//  Creation:   Mon Oct 27 14:43:23 PDT 2025
//
//  Modifications:
//
// ****************************************************************************
bool
avtMFEMDataAdaptor::CheckMeshNameForQuadratureFunctionString(const std::string &mesh_name)
{
    return mesh_name.find("_quad_func_o") != std::string::npos;
}

// ****************************************************************************
//  Method: ParseQuadratureFunctionMeshString
//
//  Purpose:
//   Parses the base mesh name and order from a Quadrature Function mesh name
//
//  Arguments:
//   qf_mesh_name:    qf style mesh name
//   base_mesh_name:  parsed base mesh name (output) 
//   qf_order:        parsed quad func order (output)
//
//  Programmer: Cyrus Harrison
//  Creation:   Fri Oct 10 15:09:42 PDT 2025
//
//  Modifications:
//
// ****************************************************************************
void
avtMFEMDataAdaptor::ParseQuadratureFunctionMeshString(const std::string &qf_mesh_name,
                                                      std::string &base_mesh_name,
                                                      int &qf_order)
{
    qf_order = -1; // -1 == not quad points case
    base_mesh_name = ""; // empty == not quad points case

    // check for fetch of quad points mesh
    // {mesh_name}_quad_func_o{order}

    std::string qf_indicator = "_quad_func_o";
    size_t qf_str_idx = qf_mesh_name.find(qf_indicator);
    if(qf_str_idx != std::string::npos)
    {
        // quadrature points are a special variant of the main mfem mesh
        // fetch w/o quad pts suffix
        // extract the order from mesh name
        std::string order_str = qf_mesh_name.substr(qf_str_idx + qf_indicator.size());
        base_mesh_name = qf_mesh_name.substr(0, qf_str_idx);
        // parse order
        if(!StringHelpers::StringToInt(order_str,qf_order))
        {
            // error
            AVT_MFEM_EXCEPTION1(InvalidVariableException,
                                "Failed to parse quadrature function mesh order from "
                                <<order_str);
        }
    }
}

// ****************************************************************************
//  Method: GenerateQuadratureFunctionMeshName
//
//  Purpose:
//   Creates a Quadrature Function mesh name from base mesh and order
//
//  Arguments:
//   base_mesh:  Base mesh name
//   qf_order:   Quadrature Function order
//
//  Programmer: Cyrus Harrison
//  Creation:   Fri Oct 10 15:09:42 PDT 2025
//
//  Modifications:
//
// ****************************************************************************
std::string
avtMFEMDataAdaptor::GenerateQuadratureFunctionMeshName(const std::string &base_mesh,
                                                       int qf_order)
{
    // the mesh name associated is:
    // {base_mesh}_quad_func_o{order}
    std::ostringstream oss;
    oss << base_mesh << "_quad_func_o" << qf_order;
    return oss.str();
}