// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_MFEM_DATA_ADAPTOR_H
#define AVT_MFEM_DATA_ADAPTOR_H
#include <avtmfem_exports.h>
#include <mfem.hpp>

#include <avtTypes.h>

#include <ostream>

//-----------------------------------------------------------------------------
// vtk forward decls
//-----------------------------------------------------------------------------
class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
//  Class:  avtMFEMDataAdaptor
//
//  Purpose:
//    Helper methods that refine MFEM meshes and grid functions 
//    to low-order VTK objects.
//
//  Programmer:  Cyrus Harrison
//  Creation:    May 31, 2022
//
//  Modifications to Blueprint Data Adaptor (now deprecated and moved here):
//      Justin Privitera, Mon Apr 11 18:20:19 PDT 2022
//      Added "new_refine" as an arg to RefineMeshToVTK.
// 
//      Justin Privitera, Wed Apr 13 13:49:43 PDT 2022
//      Added LegacyRefineMeshToVTK and LowOrderMeshToVTK to MFEM class.
// 
//      Justin Privitera, Mon May  9 14:35:18 PDT 2022
//      Added LegacyRefineGridFunctionToVTK and LowOrderGridFunctionToVTK to 
//      MFEM class and added "new_refine" as an arg to RefineGridFunctionToVTK.
// 
//  Modifications:
//    Justin Privitera, Tue Aug 23 14:52:34 PDT 2022
//    Added all functions, mostly taken from the blueprint plugin data adaptor.
//
//    Cyrus Harrison, Thu Mar  2 09:36:49 PST 2023
//    Added boundary mesh relater helpers
//
//    Cyrus Harrison Mon Sep 29 13:31:18 PDT 2025
//    Added quadtrature function helpers.
// 
//    Justin Privitera, Wed Dec 17 14:01:55 PST 2025
//    Added LOR enums meshRefinementMethod, fieldProjectionMethod, and
//    refinementBasisType.
//    Renamed LegacyRefineMeshToVTK to DiscontinuousRefineMeshToVTK.
//    Renamed LegacyRefineGridFunctionToVTK to
//    DiscontinuousRefineGridFunctionToVTK.
//    Added LOR enum arguments to the refinement methods.
//    Added printing methods for the enums for debugging.
//
// ****************************************************************************

class AVTMFEM_API avtMFEMDataAdaptor
{
public:
      enum class meshRefinementMethod
      {
          Default_LOR,
          Continuous_LOR,
          Discontinuous_LOR
      };

      enum class fieldProjectionMethod
      {
          Default_Projection,
          Zonal_Projection,
          Nodal_Projection
      };

      enum class refinementBasisType
      {
          Gauss_Lobatto_Default,
          Closed_Uniform
      };

      static vtkDataSet   *DiscontinuousRefineMeshToVTK(mfem::Mesh *mesh,
                                                        const int domain,
                                                        const int lod);

      static vtkDataSet   *LowOrderMeshToVTK(mfem::Mesh *mesh);

      static vtkDataSet   *RefineMeshToVTK(mfem::Mesh *mesh,
                                           int domain,
                                           int lod,
                                           const meshRefinementMethod mesh_ref_method,
                                           const refinementBasisType ref_basis_type);

      static vtkDataSet   *BoundaryMeshToVTK(mfem::Mesh *mesh);
      static vtkDataSet   *QuadratureFunctionMeshToVTK(mfem::Mesh *mesh,
                                                       int order,
                                                       const refinementBasisType ref_basis_type);

      static vtkDataArray *DiscontinuousRefineGridFunctionToVTK(mfem::Mesh *mesh,
                                                                mfem::GridFunction *gf,
                                                                const int lod,
                                                                const bool var_is_nodal);

      static vtkDataArray *LowOrderGridFunctionToVTK(mfem::GridFunction *gf);

      static vtkDataArray *RefineGridFunctionToVTK(mfem::Mesh *mesh,
                                                   mfem::GridFunction *gf,
                                                   const int lod,
                                                   const meshRefinementMethod mesh_ref_method,
                                                   const fieldProjectionMethod field_proj_method,
                                                   const refinementBasisType ref_basis_type,
                                                   avtCentering &cent_change,
                                                   bool var_is_nodal = true);

      static vtkDataArray *RefineElementColoringToVTK(mfem::Mesh *mesh,
                                                      int domain_id,
                                                      int lod);

      static vtkDataArray *RefineElementAttributeToVTK(mfem::Mesh *mesh,
                                                       int lod);

      static vtkDataArray *BoundaryAttributeToVTK(mfem::Mesh *mesh);

      static vtkDataArray *QuadratureFunctionToVTK(mfem::QuadratureFunction *qf);

      // Helpers for Quadrature Function style basis strings
      static bool          CheckBasisStringForQuadratureFunction(const std::string &basis);
      static void          ParseQuadratureFunctionBasisString(const std::string &basis,
                                                              int &qf_order,
                                                              int &qf_vdim);
      static std::string   GenerateQuadratureFunctionBasisString(mfem::QuadratureFunction *qf);
      static std::string   GenerateQuadratureFunctionBasisString(int qf_order,
                                                                 int qf_vdim);

      static bool          CheckMeshNameForQuadratureFunctionString(const std::string &mesh_name);
      static void          ParseQuadratureFunctionMeshString(const std::string &qf_mesh_name,
                                                             std::string &base_mesh_name,
                                                             int &qf_order);
      static std::string   GenerateQuadratureFunctionMeshName(const std::string &base_mesh,
                                                              int qf_order);

};

inline std::ostream& operator<<(std::ostream& os, avtMFEMDataAdaptor::meshRefinementMethod method)
{
    using meshRefinementMethod = avtMFEMDataAdaptor::meshRefinementMethod;
    switch(method)
    {
        case meshRefinementMethod::Default_LOR:       os << "Default_LOR"; break;
        case meshRefinementMethod::Continuous_LOR:    os << "Continuous_LOR"; break;
        case meshRefinementMethod::Discontinuous_LOR: os << "Discontinuous_LOR"; break;
        default:                                      os << "Unknown meshRefinementMethod"; break;
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, avtMFEMDataAdaptor::fieldProjectionMethod method)
{
    using fieldProjectionMethod = avtMFEMDataAdaptor::fieldProjectionMethod;
    switch(method)
    {
        case fieldProjectionMethod::Default_Projection: os << "Default_Projection"; break;
        case fieldProjectionMethod::Zonal_Projection:   os << "Zonal_Projection"; break;
        case fieldProjectionMethod::Nodal_Projection:   os << "Nodal_Projection"; break;
        default:                                        os << "Unknown fieldProjectionMethod"; break;
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, avtMFEMDataAdaptor::refinementBasisType method)
{
    using refinementBasisType = avtMFEMDataAdaptor::refinementBasisType;
    switch(method)
    {
        case refinementBasisType::Gauss_Lobatto_Default: os << "Gauss_Lobatto_Default"; break;
        case refinementBasisType::Closed_Uniform:        os << "Closed_Uniform"; break;
        default:                                         os << "Unknown refinementBasisType"; break;
    }
    return os;
}

#endif
